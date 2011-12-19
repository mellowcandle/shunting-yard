#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "shunting-yard.h"

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    stack *operands = stack_alloc();
    stack *operators = stack_alloc();

    stack_free(operands);
    stack_free(operators);
    free(str);
    return EXIT_SUCCESS;
}

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
