// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.h"

#include <stdio.h>
#include <stdlib.h>

static void show_error(Status status);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s EXPRESSION...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        double result = 0.0;
        Status status = shunting_yard(argv[i], &result);
        if (status != OK) {
            show_error(status);
            return status;
        } else
            printf("%.14g\n", result);
    }
    return EXIT_SUCCESS;
}

void show_error(Status status) {
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
    fprintf(stderr, "%s\n", message);
}
