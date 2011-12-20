#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "shunting-yard.h"

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    stack *operands = stack_alloc();
    stack *operators = stack_alloc();

    /* Loop through expression */
    int token_pos = -1;
    for (int i = 0; i <= strlen(str); ++i) {
        if (str[i] == ' ') continue;

        char char_str[] = {str[i], '\0'};   /* convert char to char* */

        /* Operands */
        if (strpbrk(char_str, "1234567890")) {
            if (token_pos == -1) token_pos = i;
            continue;
        } else if (token_pos != -1) { /* end of operand */
            stack_push_unalloc(operands,
                    strndup(str + token_pos, i - token_pos));
            token_pos = -1;
        }

        /* Operators */
        if (strpbrk(char_str, "+-*/")) {
            /* TODO: Check for operators on the stack and pop/apply them */
            stack_push(operators, char_str);
        } else if (str[i] != '\0' && str[i] != '\n')
            printf("unrecognized character \"%c\" at column %d\n", str[i], i);

        if (str[i] == '\n') break;
    }

    /* End of string - apply any remaining operators on the stack */
    char *operator;
    while (!stack_is_empty(operators)) {
        operator = stack_pop(operators);
        apply_operator(operator, operands);
        free(operator);
    }

    /* Display the result
     * TODO: Format this correctly and check for well-formedness rather than
     * lazily displaying the stack
     */
    stack_display(operands);

    /* Free memory and exit */
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

/**
 * Apply an operator to the top 2 operands on the stack.
 *
 * @param operator Operator to use (e.g., +, -, /, *).
 * @param operands Operands stack.
 */
void apply_operator(char *operator, stack *operands) {
    char *val2_ = stack_pop(operands);
    char *val1_ = stack_pop(operands);
    int val2 = atoi(val2_);
    int val1 = atoi(val1_);
    free(val2_);
    free(val1_);

    int result;
    switch (operator[0]) {
        case '+':
            result = val1 + val2;
            stack_push_unalloc(operands, num_to_str(result));
            break;

        /* TODO: more operator support! */
    }
}

/**
 * Calculate the number of digits/character in a base 10 integer.
 *
 * @param num
 * @return Length of number.
 */
int num_digits(int num) {
    int len = 0;
    while (num) {
        ++len;
        num /= 10;
    }

    return len;
}

/**
 * Convert a number to a character string, for adding to the stack.
 *
 * @param num
 * @return Stringified number.
 */
char *num_to_str(int num) {
    char *str = malloc(num_digits(num) + 1);
    snprintf(str, num_digits(num) + 1, "%d", num);

    return str;
}
