// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.
//
// Based on CUnit example code: <http://cunit.sourceforge.net/example.html>.

#include "../src/shunting-yard.h"

#include <CUnit/Basic.h>
#include <stdlib.h>

#define ASSERT_RESULT(expression, expected) \
        ASSERT_STATUS(expression, OK); \
        CU_ASSERT_DOUBLE_EQUAL(result, expected, 10e-11)

#define ASSERT_STATUS(expression, expected) \
        CU_ASSERT(shunting_yard(expression, &result) == expected)

static double result = 0.0;

static void test_addition() {
    ASSERT_RESULT("2+2", 4);
    ASSERT_RESULT("2  +  2", 4);
    ASSERT_RESULT("2+2.", 4);
    ASSERT_RESULT("3 + (5 + 1 + (2 + 2))", 13);
    ASSERT_RESULT("1+2+4+8+16 + 11", 42);
    ASSERT_RESULT("2.1+2.1", 4.2);
}

static void test_subtraction() {
    ASSERT_RESULT("8-4", 4);
    ASSERT_RESULT("15-10", 5);
    ASSERT_RESULT("27 - (10 - 11)", 28);
    ASSERT_RESULT("-5-11", -16);
    ASSERT_RESULT("-(2-3.6)", 1.6);
    ASSERT_RESULT("(-5-7)", -12);
}

static void test_multiplication() {
    ASSERT_RESULT("13 * 2", 26);
    ASSERT_RESULT("3.2*2", 6.4);
    ASSERT_RESULT("20*2*1.375", 55);
    ASSERT_RESULT("0.75*((2*-4)*1.5)", -9);
    ASSERT_RESULT("27*0.5", 13.5);
}

static void test_division() {
    ASSERT_RESULT("1/2", 0.5);
    ASSERT_RESULT("3.885 / 7", 0.555);
    ASSERT_RESULT("(140/2)/0.5/2", 70);
    ASSERT_RESULT("((517/4)/2/.25/.25)/22", 47);
    ASSERT_RESULT("2987898/34743", 86);
}

static void test_modulus() {
    ASSERT_RESULT("10 % 6", 4);
    ASSERT_RESULT("2+3 % 3", 2);
    ASSERT_RESULT("6*5%21", 9);
    ASSERT_RESULT("10%11", 10);
    ASSERT_RESULT("5 %5", 0);
    ASSERT_RESULT("5.7%3", 2.7);
    ASSERT_RESULT("pi%2", 1.1415926535898);
}

static void test_exponentiation() {
    ASSERT_RESULT("3^2", 9);
    ASSERT_RESULT("10^-2", 0.01);
    ASSERT_RESULT("4^2", 16);
    ASSERT_RESULT("2^8", 256);
    ASSERT_RESULT("5^(2^3)", 390625);
}

static void test_factorials() {
    ASSERT_RESULT("1!", 1);
    ASSERT_RESULT("2!", 2);
    ASSERT_RESULT("3!", 6);
    ASSERT_RESULT("4!", 24);
    ASSERT_RESULT("5!", 120);
    ASSERT_RESULT("3!+1", 7);
}

static void test_functions() {
    ASSERT_RESULT("abs(-32)", 32);
    ASSERT_RESULT("abs(-5-7)", 12);
    ASSERT_RESULT("abs(-1.1)", 1.1);
    ASSERT_RESULT("sqrt(100)", 10);
    ASSERT_RESULT("SqRt(100)", 10);
    ASSERT_RESULT("sqrt(sqrt(10000))", 10);
    ASSERT_RESULT("sqrt(sqrt(10000) + 800)", 30);
    ASSERT_RESULT("42 * cos(0)", 42);
    ASSERT_RESULT("(sin(0)*cos(0)*40*tan(0))-1", -1);
    ASSERT_RESULT("log(10)", 1);
    ASSERT_RESULT("lOG(10)", 1);
    ASSERT_RESULT("lb(8)", 3);
    ASSERT_RESULT("ln(e)", 1);
    ASSERT_RESULT("Ln(E)", 1);
    ASSERT_RESULT("log(10^42)", 42);
    ASSERT_RESULT("lb(2^123)", 123);
}

static void test_variables() {
    ASSERT_RESULT("cos(pi)", -1);
    ASSERT_RESULT("tan(pi)", 0);
    ASSERT_RESULT("Tan(PI)", 0);
    ASSERT_RESULT("cos(tau)", 1);
    ASSERT_RESULT("COS(TAU)", 1);
    ASSERT_RESULT("cos(2pi)", 1);
    ASSERT_RESULT("((2pi/tau)+(10pi))/(1+10pi)", 1);
}

static void test_precedence() {
    ASSERT_RESULT("6/3*5", 10);
    ASSERT_RESULT("6+3*2", 12);
    ASSERT_RESULT("-10^2", -100);
    ASSERT_RESULT("(-10)^2", 100);
    ASSERT_RESULT("10^2+1", 101);
    ASSERT_RESULT("10^-2+1", 1.01);
    ASSERT_RESULT("-10^-2+1", 0.99);
    ASSERT_RESULT("10^-2*2", 0.02);
    ASSERT_RESULT("2+6/2*5+10/3-2/6", 20);
    ASSERT_RESULT("10^3!", 1000000);
    ASSERT_RESULT("10^-2pi", M_PI / 100);
    ASSERT_RESULT("2^2^3", 256);
}

static void test_errors() {
    ASSERT_STATUS("2+*2", ERROR_SYNTAX);
    ASSERT_STATUS("2**2", ERROR_SYNTAX);
    ASSERT_STATUS("*1", ERROR_SYNTAX);
    ASSERT_STATUS("2*.", ERROR_SYNTAX);
    ASSERT_STATUS("2*2 3", ERROR_SYNTAX);
    ASSERT_STATUS("2*2.3.4", ERROR_SYNTAX);
    ASSERT_STATUS("pi2", ERROR_SYNTAX);
    ASSERT_STATUS("(2+2", ERROR_OPEN_PARENTHESIS);
    ASSERT_STATUS("(2+2)+(2+2", ERROR_OPEN_PARENTHESIS);
    ASSERT_STATUS("(2+2))", ERROR_CLOSE_PARENTHESIS);
    ASSERT_STATUS("2+&3", ERROR_UNRECOGNIZED);
    ASSERT_STATUS("", ERROR_NO_INPUT);
    ASSERT_STATUS("       ", ERROR_NO_INPUT);
    ASSERT_STATUS("foo(2)", ERROR_UNDEFINED_FUNCTION);
    ASSERT_STATUS("sqrt()", ERROR_FUNCTION_ARGUMENTS);
    ASSERT_STATUS("foo", ERROR_UNDEFINED_CONSTANT);
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    unsigned int tests_failed = 0;
    CU_pSuite suite = CU_add_suite("Shunting Yard", NULL, NULL);
    if (!suite)
        goto exit;

    if (!CU_add_test(suite, "addition", test_addition) ||
            !CU_add_test(suite, "subtraction", test_subtraction) ||
            !CU_add_test(suite, "multiplication", test_multiplication) ||
            !CU_add_test(suite, "division", test_division) ||
            !CU_add_test(suite, "modulus", test_modulus) ||
            !CU_add_test(suite, "exponentiation", test_exponentiation) ||
            !CU_add_test(suite, "factorials", test_factorials) ||
            !CU_add_test(suite, "functions", test_functions) ||
            !CU_add_test(suite, "variables", test_variables) ||
            !CU_add_test(suite, "operator precedence", test_precedence) ||
            !CU_add_test(suite, "error handling", test_errors))
        goto exit;

    CU_basic_set_mode(CU_BRM_NORMAL);
    CU_basic_run_tests();
    tests_failed = CU_get_number_of_tests_failed();
exit:
    CU_cleanup_registry();
    return tests_failed ? EXIT_FAILURE : CU_get_error();
}
