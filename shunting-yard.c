#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "stack.h"
#include "shunting-yard.h"

/* Number of characters to allocate for floats (including the decimal point) */
#define FLOAT_LENGTH 8
/* Error types */
#define ERROR_SYNTAX       1
#define ERROR_SYNTAX_STACK 2
#define ERROR_RIGHT_PAREN  3
#define ERROR_LEFT_PAREN   4
#define ERROR_UNRECOGNIZED 5
/* For calls to error() with an unknown column number */
#define NO_COL_NUM -2

const int op_order_len = 3;
const char *op_order[] = {"^", "*/", "+-"};

int main(int argc, char *argv[]) {
    char *str = join_argv(argc, argv);
    stack *operands = stack_alloc();
    stack *operators = stack_alloc();

    /* Loop through expression */
    int token_pos = -1;
    int paren_depth = 0;
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
        if (strpbrk(char_str, "+-*/^")) {
            /* Apply an operator already on the stack if it's of higher
             * precedence, as long as we aren't inside a paren */
            if (!stack_is_empty(operators) && !paren_depth
                    && compare_operators(stack_top(operators), char_str)) {
                if (!apply_operator(stack_pop_char(operators), operands)) {
                    error(ERROR_SYNTAX, i, str[i]);
                    return EXIT_FAILURE;
                }
            }

            stack_push(operators, char_str);
        }
        /* Parentheses */
        else if (str[i] == '(') {
            stack_push(operators, char_str);
            ++paren_depth;
        } else if (str[i] == ')') {
            if (!paren_depth) {
                error(ERROR_RIGHT_PAREN, i, str[i]);
                return EXIT_FAILURE;
            }

            /* Pop and apply operators until we reach the left paren */
            while (!stack_is_empty(operators)) {
                if (stack_top(operators)[0] == '(') {
                    stack_pop_char(operators);
                    --paren_depth;
                    break;
                }

                if (!apply_operator(stack_pop_char(operators), operands)) {
                    error(ERROR_SYNTAX, i, str[i]);
                    return EXIT_FAILURE;
                }
            }
        }
        /* Unknown character */
        else if (str[i] != '\0' && str[i] != '\n')
            error(ERROR_UNRECOGNIZED, i, str[i]);

        if (str[i] == '\n') break;
    }

    if (paren_depth) {
        error(ERROR_LEFT_PAREN, NO_COL_NUM, '(');
        return EXIT_FAILURE;
    }

    /* End of string - apply any remaining operators on the stack */
    char operator;
    while (!stack_is_empty(operators)) {
        operator = stack_pop_char(operators);
        if (!apply_operator(operator, operands)) {
            error(ERROR_SYNTAX_STACK, NO_COL_NUM, operator);
            return EXIT_FAILURE;
        }
    }

    /* Display the result
     * TODO: Format this correctly and check for well-formedness rather than
     * lazily displaying the stack */
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
 * @return true on success, false on failure
 */
bool apply_operator(char operator, stack *operands) {
    /* Check for underflow, as it indicates a syntax error */
    if (stack_is_empty(operands) || !operands->top->next)
        return false;

    double val2 = strtod_unalloc(stack_pop(operands));
    double val1 = strtod_unalloc(stack_pop(operands));

    double result;
    switch (operator) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/': result = val1 / val2; break;
        case '^': result = pow(val1, val2); break;
    }
    stack_push_unalloc(operands, num_to_str(result));

    return true;
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

/**
 * Outputs an error.
 */
void error(int type, int col_num, char chr) {
    ++col_num;  /* bump col_num from zero-indexed to one-indexed for display */
    switch (type) {
        case ERROR_SYNTAX:
            printf("syntax error at column %d with \"%c\"\n", col_num, chr);
            break;
        case ERROR_SYNTAX_STACK:
            printf("syntax error at column (unknown) with \"%c\"\n", chr);
            break;
        case ERROR_RIGHT_PAREN:
            printf("mismatched \"%c\" character at column %d\n", chr, col_num);
            break;
        case ERROR_LEFT_PAREN:
            printf("one or more unclosed \"%c\" detected\n", chr);
            break;
        case ERROR_UNRECOGNIZED:
            printf("unrecognized character \"%c\" at column %d ignored\n", chr,
                    col_num);
            break;
        default:
            printf("unknown error at column %d with \"%c\"\n", col_num, chr);
    }
}
