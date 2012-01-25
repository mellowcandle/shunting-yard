/*
 * Copyright 2011, 2012 Brian Marshall. All rights reserved.
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

/* Terminal width (for error output excerpts) */
#define TERM_WIDTH 80

/* Return types */
#define SUCCESS_EQ           -1
#define SUCCESS               0
#define ERROR_SYNTAX          1
#define ERROR_SYNTAX_STACK    2
#define ERROR_SYNTAX_OPERAND  3
#define ERROR_RIGHT_PAREN     4
#define ERROR_LEFT_PAREN      5
#define ERROR_UNRECOGNIZED    6
#define ERROR_NO_INPUT        7
#define ERROR_FUNC_UNDEF      8
#define ERROR_FUNC_NOARGS     9
#define ERROR_VAR_UNDEF       10

/* Stack flags */
#define FLAG_NONE       0
#define FLAG_BOOL_TRUE  1

/* For calls to error() with an unknown column number */
#define NO_COL_NUM -1

/* Size of string to hold a hexadecimal representation of double precision
 * floating point numbers */
#define DOUBLE_STR_LEN 32
/* Number of digits before showing scientific notation */
#define MIN_E_DIGITS   12

/* Convenience functions */
#define is_numeric(c) ((c >= '0' && c <= '9') || c == '.')
#define is_alpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define is_operand(c) (is_numeric(c) || is_alpha(c))
#define is_operator(c) (c != '\0' && strchr("+-*/%=^!", c) != NULL)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
