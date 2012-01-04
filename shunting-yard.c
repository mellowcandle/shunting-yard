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
#include <math.h>
#include "config.h"
#include "stack.h"
#include "shunting-yard.h"

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

        char chr_str[] = {str[i], '\0'};   /* convert char to char* */
        char prev_chr = i > 0 ? str[i - 1] : 0;

        /* Operands */
        if (is_operand(str[i])) {
            if (token_pos == -1) token_pos = i;
            continue;
        } else if (token_pos != -1) { /* end of operand */
            stack_push_unalloc(operands, substr(str, token_pos, i - token_pos));
            token_pos = -1;
        }

        /* Operators */
        if (is_operator(str[i])) {
            /* Apply one operator already on the stack if:
             *     1. It's of higher precedence
             *     2. We aren't inside a paren
             *     3. The current operator and the stack operator are either
             *        both unary or both binary
             */

            bool unary = is_unary(str[i], prev_chr);
            if (!stack_is_empty(operators) && !paren_depth
                    && unary == stack_top_item(operators)->flags
                    && compare_operators(stack_top(operators), chr_str)) {
                if (!apply_operator(stack_pop_char(operators), unary,
                            operands)) {
                    error(ERROR_SYNTAX, i, str[i]);
                    return EXIT_FAILURE;
                }
            }

            stack_push(operators, chr_str, unary);
        }
        /* Parentheses */
        else if (str[i] == '(') {
            stack_push(operators, chr_str, 0);
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

                if (!apply_operator(stack_pop_char(operators), false,
                            operands)) {
                    /* TODO: accurate column number (currently is just the col
                     * num of the right paren) */
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
    stack_item *operator;
    while (!stack_is_empty(operators)) {
        operator = stack_pop_item(operators);
        if (!apply_operator(operator->val[0], operator->flags, operands)) {
            error(ERROR_SYNTAX_STACK, NO_COL_NUM, operator->val[0]);
            return EXIT_FAILURE;
        }
        stack_free_item(operator);
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
 * @return true on success, false on failure.
 */
bool apply_operator(char operator, bool unary, stack *operands) {
    /* Check for underflow, as it indicates a syntax error */
    if (stack_is_empty(operands))
        return false;

    double result;
    double val2 = strtod_unalloc(stack_pop(operands));

    /* Handle unary operators */
    if (unary) {
        switch (operator) {
            case '+':
                /* values are already assumed positive */
                stack_push_unalloc(operands, num_to_str(val2));
                return true;
            case '-':
                result = -val2;
                stack_push_unalloc(operands, num_to_str(result));
                return true;
            case '!':
                result = tgamma(val2 + 1);
                stack_push_unalloc(operands, num_to_str(result));
                return true;
        }

        return false;   /* unknown operator */
    }

    /* Check for underflow again before we pop another operand */
    if (stack_is_empty(operands))
        return false;

    double val1 = strtod_unalloc(stack_pop(operands));
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

/**
 * Return a substring.
 */
char *substr(char *str, int start, int len) {
    char *substr = malloc(len + 1);
    memcpy(substr, str + start, len);
    substr[len] = '\0';

    return substr;
}

/**
 * Check if an operator is unary.
 */
bool is_unary(char operator, char prev_chr) {
    return is_operator(prev_chr) || prev_chr == 0 || (is_operand(prev_chr)
            && operator == '!');
}
