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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "shunting-yard.h"

char *join_argv(int count, char *src[]);

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    double result = shunting_yard(str);
    free(str);

    if (errno == SUCCESS_EQ) {  /* equations (with "=") */
        printf("%s\n", result ? "true" : "false");
        return EXIT_SUCCESS;
    } else if (errno != SUCCESS)
        return EXIT_FAILURE;

    char *result_str = trim_double(result);
    printf("%s\n", result_str);
    free(result_str);

    return EXIT_SUCCESS;
}

/**
 * Concatenate all the arguments passed to the program.
 */
char *join_argv(int count, char *src[]) {
    /* Allocate a buffer for the full string */
    int len = 0;
    for (int i = 0; i < count; ++i)
        len += strlen(src[i]) + 1;

    /* Concatenate the arguments */
    char *str = calloc(count, len + 1);
    for (int i = 1; i < count; ++i) {
        if (i > 1) strcat(str, " ");
        strcat(str, src[i]);
    }

    return str;
}
