// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.h"
#include "config.h"
#include "stack.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

typedef struct {
    char symbol;
    int precedence;
    bool unary;
} Operator;

static const Operator OPERATORS[] = {
    {'!', 1, true},
    {'^', 2, false},
    {'+', 3, true},
    {'-', 3, true},
    {'*', 4, false},
    {'/', 4, false},
    {'%', 4, false},
    {'+', 5, false},
    {'-', 5, false},
    {'=', 6, false},
    {'(', 7, false},
    {')', 7, false}
};

static Status push_operand(const char *str, int pos_a, int pos_b,
        Stack **operands);
static Status apply_operator(const Operator *operator, Stack **operands);
static Status apply_unary_operator(char op, Stack **operands);
static Status apply_stack_operators(Stack **operators, Stack **operands,
        const Operator *new_operator);
static Status apply_function(const char *func, Stack **args);

// Returns a matching operator.
static const Operator *get_operator(char symbol, bool unary);

Status shunting_yard(const char *expression, double *result,
        int *error_column) {
    Status status = SUCCESS;
    Stack *operands = NULL;
    Stack *operators = NULL;
    Stack *functions = NULL;

    int token_pos = -1;
    int paren_pos = -1;
    int paren_depth = 0;
    int error_index = -1;
    char previous_c = '\0';
    const char *operand = NULL;

    for (size_t i = 0; i <= strlen(expression); i++) {
        char c = expression[i];
        error_index = i;
        if (c == ' ') continue;

        // Operands
        if (is_operand(c)) {
            if (token_pos == -1)
                token_pos = i;
            else if (is_alpha(c) && is_numeric(previous_c)) {
                // Parse expressions like "2a"
                status = push_operand(expression, token_pos, i, &operands);
                if (status > SUCCESS) {
                    error_index = token_pos;
                    goto exit;
                }
                token_pos = i;

                // Emulate encountering a "*" operator, since "2a" implies
                // "2*a"
                status = apply_stack_operators(&operators, &operands,
                        get_operator('*', false));
                if (status > SUCCESS)
                    goto exit;
                stack_push(&operators, get_operator('*', false));
            } else if (is_numeric(c) && is_alpha(previous_c)) {
                // "a2" instead of "2a" is invalid
                status = ERROR_SYNTAX;
                goto exit;
            }

            goto skip;
        } else if (token_pos != -1) {   // end of operand
            status = push_operand(expression, token_pos, i, &operands);
            if (status > SUCCESS) {
                error_index = token_pos;
                goto exit;
            }
            token_pos = -1;
        }

        // Operators
        if (is_operator(c)) {
            bool unary = is_unary(c, previous_c);

            // Apply any lower precedence operators on the stack first
            status = apply_stack_operators(&operators, &operands,
                    get_operator(c, unary));
            if (status > SUCCESS)
                goto exit;

            // Push current operator
            stack_push(&operators, get_operator(c, unary));
        }
        // Parentheses
        else if (c == '(') {
            // Check if this paren is starting a function
            if (is_operand(previous_c))
                stack_push(&functions, stack_pop(&operands));

            stack_push(&operators, get_operator(c, false));
            ++paren_depth;
            if (paren_depth == 1) paren_pos = i;
        } else if (c == ')') {
            if (!paren_depth) {
                status = ERROR_RIGHT_PARENTHESIS;
                goto exit;
            }

            // Pop and apply operators until we reach the left paren
            while (operators != NULL) {
                if (((const Operator *)stack_top(operators))->symbol == '(') {
                    stack_pop(&operators);
                    --paren_depth;
                    break;
                }

                status = apply_operator(stack_pop(&operators), &operands);
                if (status > SUCCESS) {
                    // TODO: accurate column number (currently is just the col
                    // num of the right paren)
                    goto exit;
                }
            }

            // Check if this is the end of a function
            if (functions != NULL) {
                operand = stack_pop(&functions);
                status = apply_function(operand, &operands);
                free((void *)operand);

                if (status > SUCCESS) {
                    // TODO: accurate column number
                    goto exit;
                }
            }
        }
        // Unknown character
        else if (c != '\0' && c != '\n') {
            status = ERROR_UNRECOGNIZED;
            goto exit;
        }

skip:
        if (c == '\n') break;
        previous_c = c;
    }

    if (paren_depth) {
        error_index = paren_pos;
        status = ERROR_LEFT_PARENTHESIS;
        goto exit;
    }

    // End of string - apply any remaining operators on the stack
    while (operators != NULL) {
        status = apply_operator(stack_pop(&operators), &operands);
        if (status > SUCCESS)
            goto exit;
    }

    // Save the final result
    if (operands == NULL)
        status = ERROR_NO_INPUT;
    else
        *result = strtod_unalloc(stack_pop(&operands));

exit:
    while (operands != NULL)
        free((void *)stack_pop(&operands));
    while (operators != NULL)
        stack_pop(&operators);
    while (functions != NULL)
        free((void *)stack_pop(&functions));

    if (error_column != NULL)
        *error_column = error_index + 1;
    return status;
}

/**
 * Push an operand onto the stack and substitute any constants.
 */
Status push_operand(const char *str, int pos_a, int pos_b, Stack **operands) {
    char *operand = rtrim(substr(str, pos_a, pos_b - pos_a));
    Status status = SUCCESS;

    // Syntax check. Error if one of the following is true:
    //     1. Operand ONLY contains "."
    //     2. Operand contains a space
    //     3. Operand contains more than one "."
    if (strcmp(operand, ".") == 0 || strchr(operand, ' ') != NULL
            || strchr(operand, '.') != strrchr(operand, '.')) {
        status = ERROR_SYNTAX;
        goto error;
    }

    // Substitute constants
    if (is_alpha(operand[0])) {
        if (0 == strcasecmp(operand, "e"))
            operand = num_to_str(M_E);
        else if (0 == strcasecmp(operand, "pi"))
            operand = num_to_str(M_PI);
        else if (0 == strcasecmp(operand, "tau"))
            operand = num_to_str(2 * M_PI);
        else if (str[pos_b] != '(') {  // unknown constant
            status = ERROR_UNDEFINED_CONSTANT;
            goto error;
        }
    }

    stack_push(operands, operand);
    return status;

error:
    free(operand);
    return status;
}

/**
 * Apply an operator to the top 2 operands on the stack.
 */
Status apply_operator(const Operator *operator, Stack **operands) {
    // Check for null op or underflow, as it indicates a syntax error
    if (operator == NULL || *operands == NULL)
        return ERROR_SYNTAX;

    if (operator->unary)
        return apply_unary_operator(operator->symbol, operands);

    double result;
    double val2 = strtod_unalloc(stack_pop(operands));
    // Check for underflow again before we pop another operand
    if (*operands == NULL)
        return ERROR_SYNTAX;
    double val1 = strtod_unalloc(stack_pop(operands));

    Status status = SUCCESS;
    switch (operator->symbol) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/': result = val1 / val2; break;
        case '%': result = fmod(val1, val2); break;
        case '^': result = pow(val1, val2); break;
        case '=':
            if (0 == abs(val1 - val2)) {
                status = SUCCESS_EQUAL;
                result = val1;  // operator returns original value
            } else {
                status = SUCCESS_NOT_EQUAL;
                result = 0;
            }
            break;
        default: return ERROR_UNRECOGNIZED;  // unknown operator
    }

    stack_push(operands, num_to_str(result));
    return status;
}

/**
 * Apply a unary operator to the stack.
 */
Status apply_unary_operator(char op, Stack **operands) {
    double result;
    double val = strtod_unalloc(stack_pop(operands));

    switch (op) {
        case '+': result = val; break;  // values are assumed positive
        case '-': result = -val; break;
        case '!': result = tgamma(val + 1); break;
        default: return ERROR_UNRECOGNIZED;  // unknown operator
    }

    stack_push(operands, num_to_str(result));
    return SUCCESS;
}

/**
 * Apply one or more operators currently on the stack.
 */
Status apply_stack_operators(Stack **operators, Stack **operands,
        const Operator *new_operator) {
    if (new_operator == NULL)
        return ERROR_SYNTAX;

    // Loop through the operator stack and apply operators until we reach one
    // that's of lower precedence (with different rules for unary operators)
    while (*operators != NULL) {
        if (((const Operator *)stack_top(*operators))->precedence >
                new_operator->precedence || new_operator->unary)
            break;
        Status status = apply_operator(stack_pop(operators), operands);
        if (status > SUCCESS)
            return status;
    }
    return SUCCESS;
}

/**
 * Apply a function with arguments.
 */
Status apply_function(const char *func, Stack **operands) {
    // Function arguments can't be void
    if (*operands == NULL)
        return ERROR_FUNCTION_ARGUMENTS;

    // Pop the last operand from the stack and use it as the argument. (Only
    // functions with exactly one argument are allowed.)
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
        return ERROR_UNDEFINED_FUNCTION;

    stack_push(operands, num_to_str(result));
    return SUCCESS;
}

const Operator *get_operator(char symbol, bool unary) {
    for (size_t i = 0; i < sizeof OPERATORS / sizeof (Operator); i++) {
        if (OPERATORS[i].symbol == symbol && OPERATORS[i].unary == unary)
            return &OPERATORS[i];
    }
    return NULL;
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
double strtod_unalloc(const char *str) {
    double num = strtod(str, NULL);
    free((void *)str);
    return num;
}

/**
 * Return a substring.
 */
char *substr(const char *str, int start, size_t len) {
    char *substr = malloc(len + 1);
    memcpy(substr, str + start, len);
    substr[len] = '\0';

    return substr;
}

/**
 * Check if an operator is unary.
 */
bool is_unary(char op, char prev_chr) {
    // Special case for postfix unary operators
    if (prev_chr == '!' && op != '!')
        return false;

    // Left paren counts as an operand for prefix operators, and right paren
    // counts for postfix operators
    return is_operator(prev_chr) || prev_chr == '\0' || prev_chr == '('
        || ((is_operand(prev_chr) || prev_chr == ')') && op == '!');
}

/**
 * Trim whitespace from the end of a string.
 */
char *rtrim(char *str) {
    char *end = str + strlen(str);
    while (isspace(*--end));
    *(end + 1) = '\0';
    return str;
}
