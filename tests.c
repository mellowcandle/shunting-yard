// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.
//
// Based on CUnit example code: <http://cunit.sourceforge.net/example.html>.

#include "shunting-yard.h"

#define _XOPEN_SOURCE 700
#include <CUnit/Basic.h>

#define SY_ASSERT(expected, expression) \
        SY_ASSERT_STATUS(SUCCESS, expression); \
        CU_ASSERT_DOUBLE_EQUAL(expected, result, 10e-11)

#define SY_ASSERT_STATUS(expected, expression) \
        CU_ASSERT(shunting_yard(expression, &result) == expected)

static double result = 0.0;

void test_add() {
    SY_ASSERT(4, "2+2");
    SY_ASSERT(4, "2  +  2");
    SY_ASSERT(4, "2+2.");
    SY_ASSERT(13, "3 + (5 + 1 + (2 + 2))");
    SY_ASSERT(42, "1+2+4+8+16 + 11");
    SY_ASSERT(4.2, "2.1+2.1");
}

void test_subtract() {
    SY_ASSERT(4, "8-4");
    SY_ASSERT(5, "15-10");
    SY_ASSERT(28, "27 - (10 - 11)");
    SY_ASSERT(-16, "-5-11");
    SY_ASSERT(1.6, "-(2-3.6)");
    SY_ASSERT(-12, "(-5-7)");
}

void test_multiply() {
    SY_ASSERT(26, "13 * 2");
    SY_ASSERT(6.4, "3.2*2");
    SY_ASSERT(55, "20*2*1.375");
    SY_ASSERT(-9, "0.75*((2*-4)*1.5)");
    SY_ASSERT(13.5, "27*0.5");
}

void test_divide() {
    SY_ASSERT(0.5, "1/2");
    SY_ASSERT(0.555, "3.885 / 7");
    SY_ASSERT(70, "(140/2)/0.5/2");
    SY_ASSERT(47, "((517/4)/2/.25/.25)/22");
    SY_ASSERT(86, "2987898/34743");
}

void test_mod() {
    SY_ASSERT(4, "10 % 6");
    SY_ASSERT(2, "2+3 % 3");
    SY_ASSERT(9, "6*5%21");
    SY_ASSERT(10, "10%11");
    SY_ASSERT(0, "5 %5");
    SY_ASSERT(2.7, "5.7%3");
    SY_ASSERT(1.1415926535898, "pi%2");
}

void test_exponent() {
    SY_ASSERT(9, "3^2");
    SY_ASSERT(0.01, "10^-2");
    SY_ASSERT(16, "4^2");
    SY_ASSERT(256, "2^8");
    SY_ASSERT(390625, "5^(2^3)");
}

void test_factorial() {
    SY_ASSERT(1, "1!");
    SY_ASSERT(2, "2!");
    SY_ASSERT(6, "3!");
    SY_ASSERT(24, "4!");
    SY_ASSERT(120, "5!");
    SY_ASSERT(7, "3!+1");
}

void test_function() {
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

void test_variable() {
    SY_ASSERT(-1, "cos(pi)");
    SY_ASSERT(0, "tan(pi)");
    SY_ASSERT(0, "Tan(PI)");
    SY_ASSERT(1, "cos(tau)");
    SY_ASSERT(1, "COS(TAU)");
    SY_ASSERT(1, "cos(2pi)");
    SY_ASSERT(1, "((2pi/tau)+(10pi))/(1+10pi)");
}

void test_equal() {
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "2=2");
    SY_ASSERT_STATUS(SUCCESS_NOT_EQUAL, "1=2");
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "0=0");
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "(2=2)");
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "2=2=2");
    SY_ASSERT_STATUS(SUCCESS_NOT_EQUAL, "2=1=2");
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "5+3=2+6=10-2");
    SY_ASSERT_STATUS(SUCCESS_NOT_EQUAL, "5+3=1+6=10-2");
    SY_ASSERT_STATUS(SUCCESS_EQUAL, "(2+3)=(1+4)=5");
    SY_ASSERT_STATUS(SUCCESS_NOT_EQUAL, "1 = 1.1");
    SY_ASSERT_STATUS(SUCCESS_NOT_EQUAL, "1 = 1.9");
}

void test_order() {
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

void test_error() {
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
        goto error;
    CU_pSuite suite = CU_add_suite("shunting yard", NULL, NULL);
    if (!suite)
        goto cleanup;

    if (!CU_add_test(suite, "addition", test_add) ||
            !CU_add_test(suite, "subtraction", test_subtract) ||
            !CU_add_test(suite, "multiplication", test_multiply) ||
            !CU_add_test(suite, "division", test_divide) ||
            !CU_add_test(suite, "modulus", test_mod) ||
            !CU_add_test(suite, "exponents", test_exponent) ||
            !CU_add_test(suite, "factorials", test_factorial) ||
            !CU_add_test(suite, "functions", test_function) ||
            !CU_add_test(suite, "variables", test_variable) ||
            !CU_add_test(suite, "equality", test_equal) ||
            !CU_add_test(suite, "order of operations", test_order) ||
            !CU_add_test(suite, "error handling", test_error))
        goto cleanup;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
cleanup:
    CU_cleanup_registry();
error:
    return CU_get_error();
}
