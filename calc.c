// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "config.h"
#include "shunting-yard.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s EXPRESSION...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        double result = shunting_yard(argv[i]);
        if (errno > SUCCESS)
            return errno;
        if (errno == SUCCESS_EQUAL)
            printf("True\n");
        else if (errno == SUCCESS_NOT_EQUAL)
            printf("False\n");
        else {
            char *result_string = trim_double(result);
            printf("%s\n", result_string);
            free(result_string);
        }
    }
    return EXIT_SUCCESS;
}
