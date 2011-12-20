#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "shunting-yard.h"

/* Number of characters to allocate for floats (including the decimal point) */
#define FLOAT_LENGTH 8

const int op_order_len = 2;
const char *op_order[] = {"*/", "+-"};

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    stack *operands = stack_alloc();
    stack *operators = stack_alloc();

    /* Loop through expression */
    int token_pos = -1;
    char *operator;
    for (int i = 0; i <= strlen(str); ++i) {
        if (str[i] == ' ') continue;

        char char_str[] = {str[i], '\0'};   /* convert char to char* */

        /* Operands */
        if (strpbrk(char_str, "1234567890.")) {
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
            if (!stack_is_empty(operators)
                    && compare_operators(stack_top(operators), char_str)) {
                operator = stack_pop(operators);
                apply_operator(operator, operands);
                free(operator);
            }

            stack_push(operators, char_str);
        } else if (str[i] != '\0' && str[i] != '\n')
            printf("unrecognized character \"%c\" at column %d\n", str[i], i);

        if (str[i] == '\n') break;
    }

    /* End of string - apply any remaining operators on the stack */
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
    double val2 = strtod_unalloc(stack_pop(operands));
    double val1 = strtod_unalloc(stack_pop(operands));

    double result;
    switch (operator[0]) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/': result = val1 / val2; break;
    }
    stack_push_unalloc(operands, num_to_str(result));
}

/**
 * Compares the precedence of two operators.
 *
 * @param op1 First operator.
 * @param op2 Second operator.
 * @return 0 for the first operator, 1 for the second.
 */
int compare_operators(char *op1, char *op2) {
    int op1_rank = -1;
    int op2_rank = -1;

    /* Loop through operator order and compare */
    for (int i = 0; i < op_order_len; ++i) {
        if (strpbrk(op1, op_order[i])) op1_rank = i;
        if (strpbrk(op2, op_order[i])) op2_rank = i;
    }

    return op1_rank < op2_rank;
}

/**
 * Convert a number to a character string, for adding to the stack.
 */
char *num_to_str(double num) {
    char *str = malloc(FLOAT_LENGTH);
    snprintf(str, FLOAT_LENGTH, "%g", num);

    return str;
}

/**
 * Wrapper around strtod() that also frees the string that was converted.
 */
double strtod_unalloc(char *str) {
    double num = strtod(str, NULL);
    free(str);
    return num;
}
