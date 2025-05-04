#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORDS 1000
#define MAX_LEN 64
#define MAX_TEXT_LEN 10000
#define INF 1000000
#define DAMPING_FACTOR 0.85

typedef struct Edge {
    int to;
    int weight;
    struct Edge* next;
} Edge;

typedef struct Node {
    char word[MAX_LEN];
    Edge* head;
} Node;

Node graph[MAX_WORDS];
int nodeCount = 0;
double pageRank[MAX_WORDS];

int findNodeIndex(const char* word) {
    for (int i = 0; i < nodeCount; i++) {
        if (strcmp(graph[i].word, word) == 0) return i;
    }
    return -1;
}

int addNode(const char* word) {
    int idx = findNodeIndex(word);
    if (idx != -1) return idx;
    strcpy(graph[nodeCount].word, word);
    graph[nodeCount].head = NULL;
    return nodeCount++;
}

void addEdge(int from, int to) {
    Edge* cur = graph[from].head;
    while (cur) {
        if (cur->to == to) {
            cur->weight++;
            return;
        }
        cur = cur->next;
    }
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->to = to;
    newEdge->weight = 1;
    newEdge->next = graph[from].head;
    graph[from].head = newEdge;
}

void buildGraph(const char* text) {
    char cleaned[MAX_TEXT_LEN];
    int idx = 0;
    for (int i = 0; text[i]; i++) {
        if (isalpha(text[i]) || isspace(text[i]))
            cleaned[idx++] = tolower(text[i]);
        else
            cleaned[idx++] = ' ';
    }
    cleaned[idx] = '\0';

    char* token = strtok(cleaned, " ");
    char prev[MAX_LEN] = "";
    while (token) {
        if (strlen(token) == 0) {
            token = strtok(NULL, " ");
            continue;
        }
        int curIdx = addNode(token);
        if (strlen(prev)) {
            int prevIdx = addNode(prev);
            addEdge(prevIdx, curIdx);
        }
        strcpy(prev, token);
        token = strtok(NULL, " ");
    }
}

void showDirectedGraph() {
    printf("Directed Graph:\n");
    for (int i = 0; i < nodeCount; i++) {
        printf("%s -> ", graph[i].word);
        Edge* e = graph[i].head;
        while (e) {
            printf("%s(%d) ", graph[e->to].word, e->weight);
            e = e->next;
        }
        printf("\n");
    }
}

char* queryBridgeWords(const char* word1, const char* word2) {
    static char result[512];
    result[0] = '\0';
    int idx1 = findNodeIndex(word1);
    int idx2 = findNodeIndex(word2);
    if (idx1 == -1 || idx2 == -1) {
        sprintf(result, "No %s or %s in the graph!", word1, word2);
        return result;
    }

    Edge* e = graph[idx1].head;
    int found = 0;
    strcat(result, "The bridge words from ");
    strcat(result, word1);
    strcat(result, " to ");
    strcat(result, word2);
    strcat(result, " are: ");
    while (e) {
        Edge* mid = graph[e->to].head;
        while (mid) {
            if (mid->to == idx2) {
                strcat(result, graph[e->to].word);
                strcat(result, ", ");
                found = 1;
                break;
            }
            mid = mid->next;
        }
        e = e->next;
    }
    if (!found) return "No bridge words from given words!";
    result[strlen(result) - 2] = '\0'; // remove last comma
    return result;
}

char* generateNewText(const char* inputText) {
    static char result[1024];
    result[0] = '\0';

    char text[MAX_TEXT_LEN];
    strcpy(text, inputText);
    for (int i = 0; text[i]; i++)
        text[i] = tolower(isalpha(text[i]) ? text[i] : ' ');

    char* tokens[256];
    int count = 0;

    char* token = strtok(text, " ");
    while (token) {
        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    for (int i = 0; i < count - 1; i++) {
        strcat(result, tokens[i]);
        strcat(result, " ");
        int idx1 = findNodeIndex(tokens[i]);
        int idx2 = findNodeIndex(tokens[i + 1]);
        if (idx1 == -1 || idx2 == -1) continue;

        Edge* e = graph[idx1].head;
        while (e) {
            Edge* mid = graph[e->to].head;
            while (mid) {
                if (mid->to == idx2) {
                    strcat(result, graph[e->to].word);
                    strcat(result, " ");
                    break;
                }
                mid = mid->next;
            }
            e = e->next;
        }
    }
    strcat(result, tokens[count - 1]);
    return result;
}

char* calcShortestPath(const char* word1, const char* word2) {
    static char result[512];
    result[0] = '\0';

    int start = findNodeIndex(word1);
    int end = findNodeIndex(word2);
    if (start == -1 || end == -1) {
        sprintf(result, "No %s or %s in graph!", word1, word2);
        return result;
    }

    int dist[MAX_WORDS], prev[MAX_WORDS], visited[MAX_WORDS] = {0};
    for (int i = 0; i < nodeCount; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int i = 0; i < nodeCount; i++) {
        int u = -1, minD = INF;
        for (int j = 0; j < nodeCount; j++) {
            if (!visited[j] && dist[j] < minD) {
                minD = dist[j];
                u = j;
            }
        }
        if (u == -1) break;
        visited[u] = 1;

        Edge* e = graph[u].head;
        while (e) {
            if (dist[e->to] > dist[u] + e->weight) {
                dist[e->to] = dist[u] + e->weight;
                prev[e->to] = u;
            }
            e = e->next;
        }
    }

    if (dist[end] == INF) return "Words are not connected.";
    char path[512] = "";
    int trace[100], len = 0;
    for (int v = end; v != -1; v = prev[v])
        trace[len++] = v;
    for (int i = len - 1; i >= 0; i--) {
        strcat(path, graph[trace[i]].word);
        if (i > 0) strcat(path, " -> ");
    }
    sprintf(result, "Path: %s | Number of steps: %d", path, len - 1);
    return result;
}

double calPageRank(const char* word) {
    int idx = findNodeIndex(word);
    if (idx == -1) return 0.0;

    double pr[MAX_WORDS], tmp[MAX_WORDS];
    for (int i = 0; i < nodeCount; i++) pr[i] = 1.0 / nodeCount;

    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < nodeCount; i++) tmp[i] = (1 - DAMPING_FACTOR) / nodeCount;

        for (int i = 0; i < nodeCount; i++) {
            Edge* e = graph[i].head;
            int outDegree = 0;
            for (Edge* p = e; p; p = p->next) outDegree++;
            for (Edge* p = e; p; p = p->next) {
                tmp[p->to] += DAMPING_FACTOR * pr[i] / outDegree;
            }
        }
        for (int i = 0; i < nodeCount; i++) pr[i] = tmp[i];
    }
    return pr[idx];
}

char* randomWalk() {
    static char result[512];
    result[0] = '\0';

    srand((unsigned int)time(NULL));
    int visited[MAX_WORDS] = {0};  // 只记录访问的节点，而不是访问每对节点

    int start = rand() % nodeCount;
    int current = start;
    strcat(result, graph[current].word);

    int steps = 0; // 限制最大步数
    int maxSteps = 50; // 最大步数

    while (steps < maxSteps) {
        Edge* e = graph[current].head;
        if (!e) break;  // 如果没有出边，停止

        int edgeCount = 0;
        for (Edge* p = e; p; p = p->next) edgeCount++;
        int r = rand() % edgeCount;

        Edge* selected = e;
        while (r--) selected = selected->next;

        if (visited[selected->to]) break;  // 如果节点已经访问过，结束

        visited[selected->to] = 1;  // 标记节点已访问
        current = selected->to;

        strcat(result, " -> ");
        strcat(result, graph[current].word);

        steps++;  // 增加步数
    }

    return result;
}


int main() {
    char fileName[256];
    printf("Enter file name: ");
    scanf("%s", fileName);

    FILE* f = fopen(fileName, "r");
    if (!f) {
        printf("File not found.\n");
        return 1;
    }

    char text[MAX_TEXT_LEN];
    fread(text, 1, MAX_TEXT_LEN, f);
    fclose(f);

    buildGraph(text);

    while (1) {
        int choice;
        char w1[MAX_LEN], w2[MAX_LEN], input[MAX_TEXT_LEN];
        printf("\n1. Show Graph\n2. Query Bridge Words\n3. Generate New Text\n4. Shortest Path\n5. PageRank\n6. Random Walk\n0. Exit\n>> ");
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                showDirectedGraph();
                break;
            case 2:
                printf("Enter word1 and word2: ");
                scanf("%s %s", w1, w2);
                printf("%s\n", queryBridgeWords(w1, w2));
                break;
            case 3:
                printf("Enter text: ");
                fgets(input, MAX_TEXT_LEN, stdin);
                printf("Result: %s\n", generateNewText(input));
                break;
            case 4:
                printf("Enter word1 and word2: ");
                scanf("%s %s", w1, w2);
                printf("%s\n", calcShortestPath(w1, w2));
                break;
            case 5:
                printf("Enter word: ");
                scanf("%s", w1);
                printf("PageRank: %.4lf\n", calPageRank(w1));
                break;
            case 6:
                printf("Random Walk: %s\n", randomWalk());
                break;
            case 0:
                return 0;
        }
    }
}
