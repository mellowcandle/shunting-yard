// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

// Terminal width (for error output excerpts)
#define TERM_WIDTH 80

// Return types
#define SUCCESS_NOT_EQUAL    -2
#define SUCCESS_EQUAL        -1
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
#define ERROR_CONST_UNDEF     10

// For calls to error() with an unknown column number
#define NO_COL_NUM -1

// Size of string to hold a hexadecimal representation of double precision
// floating point numbers
#define DOUBLE_STR_LEN 32
// Number of digits before showing scientific notation
#define MIN_E_DIGITS   12

// Convenience functions
#define is_numeric(c) ((c >= '0' && c <= '9') || c == '.')
#define is_alpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define is_operand(c) (is_numeric(c) || is_alpha(c))
#define is_operator(c) (c != '\0' && strchr("+-*/%=^!", c) != NULL)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
