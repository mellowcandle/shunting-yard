#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "shunting-yard.h"

int main(int argc, char *argv[]) {
    const char *str = join_argv(argc, argv);

    stack *operands = malloc(sizeof(stack));
    stack *operators = malloc(sizeof(stack));

    return EXIT_SUCCESS;
}

char *join_argv(int count, char *src[]) {
    /* Allocate a buffer for the full string */
    int len = 0;
    for (int i = 0; i < count; ++i)
        len += strlen(src[i]) + 1;

    /* Concatenate the arguments */
    char *str = malloc(len + 1);
    for (int i = 1; i < count; ++i) {
        strcat(str, " ");
        strcat(str, src[i]);
    }

    return &str[1];
}
