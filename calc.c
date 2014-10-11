// Copyright 2011 - 2013 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shunting-yard.h"

char *join_argv(int count, char *src[]);

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    double result = shunting_yard(str);
    free(str);

    if (errno == SUCCESS_EQ) {  // equations (with "=")
        printf("%s\n", result ? "true" : "false");
        return EXIT_SUCCESS;
    } else if (errno != SUCCESS)
        return errno;

    char *result_str = trim_double(result);
    printf("%s\n", result_str);
    free(result_str);

    return EXIT_SUCCESS;
}

/**
 * Concatenate all the arguments passed to the program.
 */
char *join_argv(int count, char *src[]) {
    // Allocate a buffer for the full string
    int len = 0;
    for (int i = 0; i < count; ++i)
        len += strlen(src[i]) + 1;

    // Concatenate the arguments
    char *str = calloc(count, len + 1);
    for (int i = 1; i < count; ++i) {
        if (i > 1) strcat(str, " ");
        strcat(str, src[i]);
    }

    return str;
}
