/*
 * Copyright 2012 Brian Marshall. All rights reserved.
 * Based on CUnit example code: <http://cunit.sourceforge.net/example.html>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    /* Add the tests to the suite (run in order) */
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
