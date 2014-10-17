// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.
//
// Based on CUnit example code: <http://cunit.sourceforge.net/example.html>.

#include "../src/shunting-yard.h"

#define _XOPEN_SOURCE 700
#include <CUnit/Basic.h>
#include <stdlib.h>

#define SY_ASSERT(expected, expression) \
        SY_ASSERT_STATUS(OK, expression); \
        CU_ASSERT_DOUBLE_EQUAL(expected, result, 10e-11)

#define SY_ASSERT_STATUS(expected, expression) \
        CU_ASSERT(shunting_yard(expression, &result) == expected)

static double result = 0.0;

static void test_addition() {
    SY_ASSERT(4, "2+2");
    SY_ASSERT(4, "2  +  2");
    SY_ASSERT(4, "2+2.");
    SY_ASSERT(13, "3 + (5 + 1 + (2 + 2))");
    SY_ASSERT(42, "1+2+4+8+16 + 11");
    SY_ASSERT(4.2, "2.1+2.1");
}

static void test_subtraction() {
    SY_ASSERT(4, "8-4");
    SY_ASSERT(5, "15-10");
    SY_ASSERT(28, "27 - (10 - 11)");
    SY_ASSERT(-16, "-5-11");
    SY_ASSERT(1.6, "-(2-3.6)");
    SY_ASSERT(-12, "(-5-7)");
}

static void test_multiplication() {
    SY_ASSERT(26, "13 * 2");
    SY_ASSERT(6.4, "3.2*2");
    SY_ASSERT(55, "20*2*1.375");
    SY_ASSERT(-9, "0.75*((2*-4)*1.5)");
    SY_ASSERT(13.5, "27*0.5");
}

static void test_division() {
    SY_ASSERT(0.5, "1/2");
    SY_ASSERT(0.555, "3.885 / 7");
    SY_ASSERT(70, "(140/2)/0.5/2");
    SY_ASSERT(47, "((517/4)/2/.25/.25)/22");
    SY_ASSERT(86, "2987898/34743");
}

static void test_modulus() {
    SY_ASSERT(4, "10 % 6");
    SY_ASSERT(2, "2+3 % 3");
    SY_ASSERT(9, "6*5%21");
    SY_ASSERT(10, "10%11");
    SY_ASSERT(0, "5 %5");
    SY_ASSERT(2.7, "5.7%3");
    SY_ASSERT(1.1415926535898, "pi%2");
}

static void test_exponentiation() {
    SY_ASSERT(9, "3^2");
    SY_ASSERT(0.01, "10^-2");
    SY_ASSERT(16, "4^2");
    SY_ASSERT(256, "2^8");
    SY_ASSERT(390625, "5^(2^3)");
}

static void test_factorials() {
    SY_ASSERT(1, "1!");
    SY_ASSERT(2, "2!");
    SY_ASSERT(6, "3!");
    SY_ASSERT(24, "4!");
    SY_ASSERT(120, "5!");
    SY_ASSERT(7, "3!+1");
}

static void test_functions() {
    SY_ASSERT(32, "abs(-32)");
    SY_ASSERT(12, "abs(-5-7)");
    SY_ASSERT(1.1, "abs(-1.1)");
    SY_ASSERT(10, "sqrt(100)");
    SY_ASSERT(10, "SqRt(100)");
    SY_ASSERT(10, "sqrt(sqrt(10000))");
    SY_ASSERT(30, "sqrt(sqrt(10000) + 800)");
    SY_ASSERT(42, "42 * cos(0)");
    SY_ASSERT(-1, "(sin(0)*cos(0)*40*tan(0))-1");
    SY_ASSERT(1, "log(10)");
    SY_ASSERT(1, "lOG(10)");
    SY_ASSERT(3, "lb(8)");
    SY_ASSERT(1, "ln(e)");
    SY_ASSERT(1, "Ln(E)");
    SY_ASSERT(42, "log(10^42)");
    SY_ASSERT(123, "lb(2^123)");
}

static void test_variables() {
    SY_ASSERT(-1, "cos(pi)");
    SY_ASSERT(0, "tan(pi)");
    SY_ASSERT(0, "Tan(PI)");
    SY_ASSERT(1, "cos(tau)");
    SY_ASSERT(1, "COS(TAU)");
    SY_ASSERT(1, "cos(2pi)");
    SY_ASSERT(1, "((2pi/tau)+(10pi))/(1+10pi)");
}

static void test_precedence() {
    SY_ASSERT(10, "6/3*5");
    SY_ASSERT(12, "6+3*2");
    SY_ASSERT(-100, "-10^2");
    SY_ASSERT(100, "(-10)^2");
    SY_ASSERT(101, "10^2+1");
    SY_ASSERT(1.01, "10^-2+1");
    SY_ASSERT(0.99, "-10^-2+1");
    SY_ASSERT(0.02, "10^-2*2");
    SY_ASSERT(20, "2+6/2*5+10/3-2/6");
    SY_ASSERT(1000000, "10^3!");
    SY_ASSERT(M_PI / 100, "10^-2pi");
    SY_ASSERT(256, "2^2^3");
}

static void test_errors() {
    SY_ASSERT_STATUS(ERROR_SYNTAX, "2+*2");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "2**2");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "*1");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "2*.");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "2*2 3");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "2*2.3.4");
    SY_ASSERT_STATUS(ERROR_SYNTAX, "pi2");
    SY_ASSERT_STATUS(ERROR_OPEN_PARENTHESIS, "(2+2");
    SY_ASSERT_STATUS(ERROR_OPEN_PARENTHESIS, "(2+2)+(2+2");
    SY_ASSERT_STATUS(ERROR_CLOSE_PARENTHESIS, "(2+2))");
    SY_ASSERT_STATUS(ERROR_UNRECOGNIZED, "2+&3");
    SY_ASSERT_STATUS(ERROR_NO_INPUT, "");
    SY_ASSERT_STATUS(ERROR_NO_INPUT, "       ");
    SY_ASSERT_STATUS(ERROR_UNDEFINED_FUNCTION, "foo(2)");
    SY_ASSERT_STATUS(ERROR_FUNCTION_ARGUMENTS, "sqrt()");
    SY_ASSERT_STATUS(ERROR_UNDEFINED_CONSTANT, "foo");
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
