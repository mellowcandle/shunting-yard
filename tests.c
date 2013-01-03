/*
 * Copyright 2012, 2013 Brian Marshall. All rights reserved.
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

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CUnit/Basic.h"
#include "shunting-yard.h"

const double float_precision = 0.000000000001;

#define SY_ASSERT(a, b) CU_ASSERT_DOUBLE_EQUAL(shunting_yard(b), a, \
        float_precision); CU_ASSERT(errno <= SUCCESS); errno = 0
#define SY_E_ASSERT(a, b) shunting_yard(b); CU_ASSERT_EQUAL(errno, a); errno = 0

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
    SY_ASSERT(1.14159265359, "pi%2");
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
}

void test_function() {
    SY_ASSERT(32, "abs(-32)");
    SY_ASSERT(12, "abs(-5-7)");
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
    SY_ASSERT(1, "2=2");
    SY_ASSERT(0, "1=2");
    SY_ASSERT(1, "0=0");
    SY_ASSERT(1, "(2=2)");
    SY_ASSERT(1, "2=2=2");
    SY_ASSERT(0, "2=1=2");
    SY_ASSERT(1, "5+3=2+6=10-2");
    SY_ASSERT(0, "5+3=1+6=10-2");
    SY_ASSERT(1, "(2+3)=(1+4)=5");
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
}

void test_error() {
    SY_E_ASSERT(ERROR_SYNTAX, "a2");
    SY_E_ASSERT(ERROR_SYNTAX, "2**2");
    SY_E_ASSERT(ERROR_SYNTAX_STACK, "*1");
    SY_E_ASSERT(ERROR_SYNTAX_OPERAND, "2*.");
    SY_E_ASSERT(ERROR_SYNTAX_OPERAND, "2*2 3");
    SY_E_ASSERT(ERROR_SYNTAX_OPERAND, "2*2.3.4");
    SY_E_ASSERT(ERROR_RIGHT_PAREN, "(2+2))");
    SY_E_ASSERT(ERROR_LEFT_PAREN, "(2+2");
    SY_E_ASSERT(ERROR_UNRECOGNIZED, "2+&3");
    SY_E_ASSERT(ERROR_NO_INPUT, "");
    SY_E_ASSERT(ERROR_NO_INPUT, "       ");
    SY_E_ASSERT(ERROR_FUNC_UNDEF, "foo(2)");
    SY_E_ASSERT(ERROR_FUNC_NOARGS, "sqrt()");
    SY_E_ASSERT(ERROR_VAR_UNDEF, "foo");
}

int main() {
    /* Suppress error output from shunting_yard() */
    sy_quiet = true;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        goto error;

    /* Add a suite to the registry */
    CU_pSuite pSuite = CU_add_suite("shunting yard", NULL, NULL);
    if (NULL == pSuite)
        goto cleanup;

    /* Add the tests to the suite (run in order) */
    if ((NULL == CU_add_test(pSuite, "addition", test_add)) ||
            (NULL == CU_add_test(pSuite, "subtraction", test_subtract)) ||
            (NULL == CU_add_test(pSuite, "multiplication", test_multiply)) ||
            (NULL == CU_add_test(pSuite, "division", test_divide)) ||
            (NULL == CU_add_test(pSuite, "modulus", test_mod)) ||
            (NULL == CU_add_test(pSuite, "exponents", test_exponent)) ||
            (NULL == CU_add_test(pSuite, "factorials", test_factorial)) ||
            (NULL == CU_add_test(pSuite, "functions", test_function)) ||
            (NULL == CU_add_test(pSuite, "variables", test_variable)) ||
            (NULL == CU_add_test(pSuite, "equality", test_equal)) ||
            (NULL == CU_add_test(pSuite, "order of operations", test_order)) ||
            (NULL == CU_add_test(pSuite, "error handling", test_error)))
        goto cleanup;

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
cleanup:
    CU_cleanup_registry();
error:
    return CU_get_error();
}
