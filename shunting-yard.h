// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    SUCCESS_NOT_EQUAL = -2,
    SUCCESS_EQUAL,
    SUCCESS,
    ERROR_SYNTAX,
    ERROR_RIGHT_PARENTHESIS,
    ERROR_LEFT_PARENTHESIS,
    ERROR_UNRECOGNIZED,
    ERROR_NO_INPUT,
    ERROR_UNDEFINED_FUNCTION,
    ERROR_FUNCTION_ARGUMENTS,
    ERROR_UNDEFINED_CONSTANT
} Status;

// Parses a mathematical expression and computes the result.
//
// Returns `<= SUCCESS` if successful, or `> SUCCESS` if an error occurred. If
// `error_column` is not NULL, it will be set to the error's column number in
// the expression.
Status shunting_yard(const char *expression, double *result, int *error_column);

char *num_to_str(double num);
double strtod_unalloc(const char *str);
char *substr(const char *str, int start, size_t len);
bool is_unary(char op, char prev_chr);
char *rtrim(char *str);
