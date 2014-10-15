// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

typedef enum {
    SUCCESS_NOT_EQUAL = -2,
    SUCCESS_EQUAL,
    SUCCESS,
    ERROR_SYNTAX,
    ERROR_OPEN_PARENTHESIS,
    ERROR_CLOSE_PARENTHESIS,
    ERROR_UNRECOGNIZED,
    ERROR_NO_INPUT,
    ERROR_UNDEFINED_FUNCTION,
    ERROR_FUNCTION_ARGUMENTS,
    ERROR_UNDEFINED_CONSTANT
} Status;

// Parses a mathematical expression and computes the result.
//
// Returns `<= SUCCESS` if successful, or `> SUCCESS` if an error occurred.
Status shunting_yard(const char *expression, double *result);
