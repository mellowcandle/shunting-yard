#include <CUnit/Basic.h>
#include "shunting-yard.h"

#define SY_ASSERT(a, b) CU_ASSERT(a == shunting_yard(b)); CU_ASSERT(0 == errno)

void test_addition() {
    SY_ASSERT(4, "2+2");
    SY_ASSERT(4, "2  +  2");
    SY_ASSERT(13, "3 + (5 + 1 + (2 + 2))");
    SY_ASSERT(42, "1+2+4+8+16 + 11");
    SY_ASSERT(4.2, "2.1+2.1");
}

void test_subtraction() {
    SY_ASSERT(4, "8-4");
    SY_ASSERT(5, "15-10");
    SY_ASSERT(6, "27-10-11");
    SY_ASSERT(-16, "-5-11");
    SY_ASSERT(-1.6, "2-3.6");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        goto error;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("shunting yard", NULL, NULL);
    if (NULL == pSuite)
        goto cleanup;

    /* Add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    if ((NULL == CU_add_test(pSuite, "addition", test_addition)) ||
            (NULL == CU_add_test(pSuite, "subtraction", test_subtraction)))
        goto cleanup;

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
cleanup:
    CU_cleanup_registry();
error:
    return CU_get_error();
}
