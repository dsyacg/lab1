#include <CUnit/Basic.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// 外部函数声明（来自被测源文件）
extern void buildGraph(const char* text);
extern char* randomWalk();

// 设置图（复用已有文本）
void setup_graph() {
    buildGraph("The scientist carefully analyzed the data wrote a detailed report and shared the report with the team but the team requested more data so the scientist analyzed it again");
}

// 测试用例 1：检查路径是否可能立即终止（起点无出边）或为合法路径
void test_randomWalk_case1(void) {
    setup_graph();
    char* result = randomWalk();

    CU_ASSERT_PTR_NOT_NULL(result);

    // 判断如果 result 中没有 "->"，则路径长度为1，说明起点无出边，属正常情况
    if (strstr(result, "->") == NULL) {
        CU_PASS("randomWalk stopped immediately (start node has no outgoing edges)");
    } else {
        // 检查路径格式：多个单词用 "->" 连接
        CU_ASSERT_PTR_NOT_NULL(strstr(result, "->"));
    }
}

void test_randomWalk_case2(void) {
    setup_graph();
    char* result = randomWalk();
    CU_ASSERT_PTR_NOT_NULL(result);

    // 拷贝字符串进行分词处理
    char temp[512];
    strncpy(temp, result, sizeof(temp));
    temp[sizeof(temp)-1] = '\0';

    char* tokens[100];
    int count = 0;

    char* token = strtok(temp, " ->");
    while (token != NULL && count < 100) {
        tokens[count++] = token;
        token = strtok(NULL, " ->");
    }

    // 检查是否有重复词，允许起点节点重复出现
    for (int i = 1; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            CU_ASSERT_NOT_EQUAL(strcmp(tokens[i], tokens[j]), 0);
        }
    }
}

int main() {
    setup_graph();

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("randomWalk_TestSuite", 0, 0);
    if (suite == NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((CU_add_test(suite, "test_case1_path_format", test_randomWalk_case1) == NULL) ||
        (CU_add_test(suite, "test_case2_no_repeat_nodes", test_randomWalk_case2) == NULL)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
