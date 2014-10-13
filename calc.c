// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.h"

#include <stdio.h>
#include <stdlib.h>

static void show_error(Status status, int expression_number, int column);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s EXPRESSION...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        double result = 0.0;
        int error_column = 0;
        Status status = shunting_yard(argv[i], &result, &error_column);

        if (status > SUCCESS) {
            show_error(status, i, error_column);
            return status;
        }
        if (status == SUCCESS_EQUAL)
            printf("True\n");
        else if (status == SUCCESS_NOT_EQUAL)
            printf("False\n");
        else
            printf("%.16g\n", result);
    }
    return EXIT_SUCCESS;
}

void show_error(Status status, int expression_number, int column) {
    char *message = NULL;
    switch (status) {
        case ERROR_SYNTAX:
            message = "Syntax error";
            break;
        case ERROR_OPEN_PARENTHESIS:
            message = "Missing parenthesis";
            break;
        case ERROR_CLOSE_PARENTHESIS:
            message = "Extra parenthesis";
            break;
        case ERROR_UNRECOGNIZED:
            message = "Unknown character";
            break;
        case ERROR_NO_INPUT:
            message = "Empty expression";
            break;
        case ERROR_UNDEFINED_FUNCTION:
            message = "Unknown function";
            break;
        case ERROR_FUNCTION_ARGUMENTS:
            message = "Missing function arguments";
            break;
        case ERROR_UNDEFINED_CONSTANT:
            message = "Unknown constant";
            break;
        default:
            message = "Unknown error";
    }
    if (column)
        fprintf(stderr, "%s (expression %d, column %d)\n",
                message, expression_number, column);
    else
        fprintf(stderr, "%s (expression %d)\n", message, expression_number);
}
