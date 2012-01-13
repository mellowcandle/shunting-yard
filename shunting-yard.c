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
#include <strings.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "shunting-yard.h"

/* Global variables */
bool sy_quiet = false;  /* suppress error output when true */
static const int op_order_len = 5;
static const char *op_order[] = {"^", "*/", "+-%", "=", "("};

/**
 * Parse a string and do the calculations. In the event of an error, will set
 * errno to an error code and return zero.
 */
double shunting_yard(char *str) {
    double result = 0;
    stack *operands = stack_alloc();
    stack *operators = stack_alloc();
    stack *functions = stack_alloc();
    stack_item *item;

    /* Loop variables */
    int token_pos = -1;
    int paren_depth = 0;
    char prev_chr = '\0';
    char *operand;

    /* Variables used only for error() - not required for parsing */
    int error_type = 0;
    int paren_pos = -1;

    /* Loop through expression */
    for (int i = 0; i <= strlen(str); ++i) {
        if (str[i] == ' ') continue;
        char chr_str[] = {str[i], '\0'};   /* convert char to char* */

        /* Operands */
        if (is_operand(str[i])) {
            if (token_pos == -1)
                token_pos = i;
            else if (is_alpha(str[i]) && is_numeric(prev_chr)) {
                /* Parse expressions like "2a" */
                if (!push_operand(str, token_pos, i, operands))
                    goto exit;
                token_pos = i;

                /* Emulate encountering a "*" operator, since "2a" implies "2*a"
                 */
                if (!apply_stack_operators("*", false, operands, operators)) {
                    error(ERROR_SYNTAX, i, str);
                    goto exit;
                }
                stack_push(operators, "*", false);
            } else if (is_numeric(str[i]) && is_alpha(prev_chr)) {
                /* "a2" instead of "2a" is invalid */
                error(ERROR_SYNTAX, i, str);
                goto exit;
            }

            goto skip;
        } else if (token_pos != -1) {   /* end of operand */
            if (!push_operand(str, token_pos, i, operands))
                goto exit;
            token_pos = -1;
        }

        /* Operators */
        if (is_operator(str[i])) {
            bool unary = is_unary(str[i], prev_chr);

            /* Apply any lower precedence operators on the stack first */
            if (!apply_stack_operators(chr_str, unary, operands, operators)) {
                error(ERROR_SYNTAX, i, str);
                goto exit;
            }

            /* Push current operator */
            stack_push(operators, chr_str, unary);
        }
        /* Parentheses */
        else if (str[i] == '(') {
            /* Check if this paren is starting a function */
            if (is_operand(prev_chr))
                stack_push_unalloc(functions, stack_pop(operands), FLAG_NONE);

            stack_push(operators, chr_str, 0);
            ++paren_depth;
            if (paren_depth == 1) paren_pos = i;
        } else if (str[i] == ')') {
            if (!paren_depth) {
                error(ERROR_RIGHT_PAREN, i, str);
                goto exit;
            }

            /* Pop and apply operators until we reach the left paren */
            while (!stack_is_empty(operators)) {
                if (stack_top(operators)[0] == '(') {
                    stack_pop_char(operators);
                    --paren_depth;
                    break;
                }

                item = stack_pop_item(operators);
                if (!apply_operator(item->val[0], item->flags, operands)) {
                    /* TODO: accurate column number (currently is just the col
                     * num of the right paren) */
                    error(ERROR_SYNTAX, i, str);
                    goto exit;
                }
                stack_free_item(item);
            }

            /* Check if this is the end of a function */
            if (!stack_is_empty(functions)) {
                operand = stack_pop(functions);
                error_type = apply_function(operand, operands);
                free(operand);

                if (error_type != SUCCESS) {
                    /* TODO: accurate column number */
                    error(error_type, i, str);
                    goto exit;
                }
            }
        }
        /* Unknown character */
        else if (str[i] != '\0' && str[i] != '\n') {
            error(ERROR_UNRECOGNIZED, i, str);
            goto exit;
        }

skip:
        if (str[i] == '\n') break;
        prev_chr = str[i];
    }

    if (paren_depth) {
        error(ERROR_LEFT_PAREN, paren_pos, str);
        goto exit;
    }

    /* End of string - apply any remaining operators on the stack */
    while (!stack_is_empty(operators)) {
        item = stack_pop_item(operators);
        if (!apply_operator(item->val[0], item->flags, operands)) {
            error(ERROR_SYNTAX_STACK, NO_COL_NUM, str);
            goto exit;
        }
        stack_free_item(item);
    }

    /* Save the final result */
    if (stack_is_empty(operands))
        error(ERROR_NO_INPUT, NO_COL_NUM, str);
    else {
        item = stack_top_item(operands);

        /* Convert equations into a boolean result */
        if (errno == SUCCESS_EQ)
            result = (item->flags == FLAG_BOOL_TRUE) ? 1 : 0;
        else
            result = strtod(item->val, NULL);
    }

    /* Free memory and return */
exit:
    stack_free(operands);
    stack_free(operators);
    stack_free(functions);
    return result;
}

/**
 * Push an operand onto the stack and substitute any variables.
 */
bool push_operand(char *str, int pos_a, int pos_b, stack *operands) {
    char *operand = rtrim(substr(str, pos_a, pos_b - pos_a));

    /* Syntax check. Error if one of the following is true:
     *     1. Operand ONLY contains "."
     *     2. Operand contains a space
     *     3. Operand contains more than one "."
     */
    if (strcmp(operand, ".") == 0 || strchr(operand, ' ') != NULL
            || strchr(operand, '.') != strrchr(operand, '.')) {
        error(ERROR_SYNTAX_OPERAND, pos_a, str);
        return false;
    }

    /* Substitute variables */
    if (is_alpha(operand[0])) {
        if (0 == strcasecmp(operand, "e"))
            operand = num_to_str(M_E);
        else if (0 == strcasecmp(operand, "pi"))
            operand = num_to_str(M_PI);
        else if (0 == strcasecmp(operand, "tau"))
            operand = num_to_str(2 * M_PI);
        else if (str[pos_b] != '(') {  /* unknown variable */
            error(ERROR_VAR_UNDEF, pos_a, str);
            return false;
        }
    }

    stack_push_unalloc(operands, operand, FLAG_NONE);
    return true;
}

/**
 * Apply an operator to the top 2 operands on the stack.
 */
bool apply_operator(char operator, bool unary, stack *operands) {
    /* Check for underflow, as it indicates a syntax error */
    if (stack_is_empty(operands))
        return false;

    /* Apply an unary operator */
    if (unary)
        return apply_unary_operator(operator, operands);

    short int flags = FLAG_NONE;
    double result;
    double val2 = strtod_unalloc(stack_pop(operands));
    /* Check for underflow again before we pop another operand */
    if (stack_is_empty(operands))
        return false;
    double val1 = strtod_unalloc(stack_pop(operands));

    switch (operator) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/': result = val1 / val2; break;
        case '%': result = fmod(val1, val2); break;
        case '^': result = pow(val1, val2); break;
        case '=':
            /* Indicate that output is now a boolean instead of a number */
            errno = SUCCESS_EQ;

            if (0 == abs(val1 - val2)) {
                result = val1;  /* operator returns original value */
                /* This is used instead of simply typecasting the result into a
                 * bool later on, because that would cause "0=0" to return
                 * false */
                flags = FLAG_BOOL_TRUE;
            } else
                result = 0;

            break;
        default: return false;  /* unknown operator */
    }

    stack_push_unalloc(operands, num_to_str(result), flags);
    return true;
}

/**
 * Apply an unary operator to the stack.
 */
bool apply_unary_operator(char operator, stack *operands) {
    double result;
    double val = strtod_unalloc(stack_pop(operands));

    switch (operator) {
        case '+': result = val; break;  /* values are assumed positive */
        case '-': result = -val; break;
        case '!': result = tgamma(val + 1); break;
        default: return false;  /* unknown operator */
    }

    stack_push_unalloc(operands, num_to_str(result), FLAG_NONE);
    return true;
}

/**
 * Apply one or more operators currently on the stack.
 */
bool apply_stack_operators(char *op, bool unary, stack *operands,
        stack *operators) {
    /* Loop through the operator stack and apply operators until we
     * reach one that's of lower precedence (with different rules for
     * unary operators) */
    stack_item *item;
    while (!stack_is_empty(operators)) {
        if (!compare_operators(stack_top(operators),
                    stack_top_item(operators)->flags, op, unary))
            break;

        item = stack_pop_item(operators);
        if (!apply_operator(item->val[0], item->flags, operands))
            return false;
        stack_free_item(item);
    }

    return true;
}


/**
 * Apply a function with arguments.
 */
int apply_function(char *func, stack *operands) {
    /* Function arguments can't be void */
    if (stack_is_empty(operands))
        return ERROR_FUNC_NOARGS;

    /* Pop the last operand from the stack and use it as the argument. (Only
     * functions with exactly one argument are allowed.) */
    double arg = strtod_unalloc(stack_pop(operands));
    double result;

    if (0 == strcasecmp(func, "abs"))
        result = abs(arg);
    else if (0 == strcasecmp(func, "sqrt"))
        result = sqrt(arg);
    else if (0 == strcasecmp(func, "ln"))
        result = log(arg);
    else if (0 == strcasecmp(func, "lb"))
        result = log2(arg);
    else if (0 == strcasecmp(func, "lg") || 0 == strcasecmp(func, "log"))
        result = log10(arg);
    else if (0 == strcasecmp(func, "cos"))
        result = cos(arg);
    else if (0 == strcasecmp(func, "sin"))
        result = sin(arg);
    else if (0 == strcasecmp(func, "tan"))
        result = tan(arg);
    else    /* unknown function */
        return ERROR_FUNC_UNDEF;

    stack_push_unalloc(operands, num_to_str(result), FLAG_NONE);
    return SUCCESS;
}

/**
 * Compares the precedence of two operators.
 */
int compare_operators(char *op1, bool op1_unary, char *op2, bool op2_unary) {
    int op1_rank = -1;
    int op2_rank = -1;

    /* Loop through operator order and compare */
    for (int i = 0; i < op_order_len; ++i) {
        if (strpbrk(op1, op_order[i])) op1_rank = i;
        if (strpbrk(op2, op_order[i])) op2_rank = i;
    }

    /* Confusingly, unary "-" operators are a special case: -10^2 is evaluated
     * as -(10^2), but -10*2 is evaluated as (-10)*2. However, this only applies
     * when it's in the op1 position - if it's in op2, as in 10^-2, then
     * standard unary order is in effect */
    if (op1_unary && 0 == strcmp(op1, "-"))
        --op1_rank;

                                   /* unary operators have special precedence */
    return op1_rank <= op2_rank && (!op2_unary);
}

/**
 * Convert a number to a character string, for adding to the stack.
 */
char *num_to_str(double num) {
    char *str = malloc(DOUBLE_STR_LEN);
    snprintf(str, DOUBLE_STR_LEN, "%a", num);

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
void error(int type, int col_num, char *str) {
    errno = type;
    if (sy_quiet) return;   /* suppress error output */

    char error_str[TERM_WIDTH] = "Error: ";
    switch (type) {
        case ERROR_SYNTAX:
        case ERROR_SYNTAX_STACK:
        case ERROR_SYNTAX_OPERAND:
            strcat(error_str, "malformed expression");
            break;
        case ERROR_RIGHT_PAREN:
            strcat(error_str, "mismatched right parenthesis");
            break;
        case ERROR_LEFT_PAREN:
            strcat(error_str, "mismatched (unclosed) left parenthesis");
            break;
        case ERROR_UNRECOGNIZED:
            strcat(error_str, "unrecognized character");
            break;
        case ERROR_NO_INPUT:
            fprintf(stderr, "This is a calculator - provide some math!\n");
            return;
        case ERROR_FUNC_UNDEF:
            strcat(error_str, "undefined function");
            break;
        case ERROR_FUNC_NOARGS:
            strcat(error_str, "function requires arguments");
            break;
        case ERROR_VAR_UNDEF:
            strcat(error_str, "undefined variable");
            break;
        default:
            strcat(error_str, "unknown error");
    }

    /* Output excerpt and column marker */
    if (col_num != NO_COL_NUM) {
        strcat(error_str, ": ");

        ++col_num;  /* width variables below start at 1, so this should too */
        int total_width = TERM_WIDTH;
        int msg_width = (int)strlen(error_str);
        int avail_width = MIN(total_width - msg_width, strlen(str) + 1);
        int substr_start = MAX(col_num - avail_width / 2, 0);

        char *excerpt = substr(str, substr_start, avail_width);
        fprintf(stderr, "%s%s\n", error_str, excerpt);
        fprintf(stderr, "%*c\n", msg_width + col_num - substr_start, '^');
        free(excerpt);
    } else
        fprintf(stderr, "%s\n", error_str);
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
    /* Special case for postfix unary operators */
    if (prev_chr == '!' && operator != '!')
        return false;

    /* Left paren counts as an operand for prefix operators, and right paren
     * counts for postfix operators */
    return is_operator(prev_chr) || prev_chr == '\0' || prev_chr == '('
        || ((is_operand(prev_chr) || prev_chr == ')') && operator == '!');
}

/**
 * Remove trailing zeroes from a double and return it as a string.
 */
char *trim_double(double num) {
    char *str = malloc(DECIMAL_DIG + 1);
    snprintf(str, DECIMAL_DIG + 1,
            num >= pow(10, MIN_E_DIGITS) ? "%.*e" : "%.*f", MIN_E_DIGITS, num);

    for (int i = strlen(str) - 1; i > 0; --i) {
        if (str[i] == '.') str[i] = '\0';
        if (str[i] != '0') break;

        str[i] = '\0';
    }

    return str;
}

/**
 * Trim whitespace from the end of a string.
 */
char *rtrim(char *str)
{
    char *end = str + strlen(str);
    while (isspace(*--end));
    *(end + 1) = '\0';
    return str;
}
