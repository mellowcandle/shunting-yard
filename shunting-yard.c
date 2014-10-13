// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.h"
#include "config.h"
#include "stack.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

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

// Inserts an operand (number literal or constant) at the top of the stack.
//
// This will also recognize function identifiers and add them to the function
// stack instead. TODO: Fix this parsing quirk.
static Status push_operand(const char *string, int length, Stack **operands,
        Stack **functions);

static Status apply_operator(const Operator *operator, Stack **operands);
static Status apply_unary_operator(char op, Stack **operands);
static Status apply_stack_operators(Stack **operators, Stack **operands,
        const Operator *new_operator);
static Status apply_function(const char *func, Stack **args);
static bool is_unary(char op, char prev_chr);

// Returns a matching operator.
static const Operator *get_operator(char symbol, bool unary);

// Frees a pointer to a double and returns its value.
static double free_double(const double *pointer);

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
                status = push_operand(expression + token_pos, i - token_pos,
                        &operands, &functions);
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
            status = push_operand(expression + token_pos, i - token_pos,
                    &operands, &functions);
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
                const char *function = stack_pop(&functions);
                status = apply_function(function, &operands);
                free((void *)function);

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
        *result = free_double(stack_pop(&operands));

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

Status push_operand(const char *string, int length, Stack **operands,
        Stack **functions) {
    while (length > 0 && string[length - 1] == ' ')
        length--;  // Strip whitespace from the end.
    char token[length + 1];
    snprintf(token, length + 1, "%s", string);

    double *operand = calloc(1, sizeof (double));
    if (is_alpha(token[0])) {
        if (strcasecmp(token, "e") == 0)
            *operand = M_E;
        else if (strcasecmp(token, "pi") == 0)
            *operand = M_PI;
        else if (strcasecmp(token, "tau") == 0)
            *operand = M_PI * 2;
        else if (string[length] == '(') {
            // This operand is a function identifier instead.
            stack_push(functions, strdup(token));
            return SUCCESS;
        } else
            return ERROR_UNDEFINED_CONSTANT;
    } else {
        char *end = NULL;
        *operand = strtod(token, &end);

        // If not all of the token is converted, the rest is invalid.
        if (token + length != end)
            return ERROR_SYNTAX;
    }

    stack_push(operands, operand);
    return SUCCESS;
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

    double *result = calloc(1, sizeof (double));
    double val2 = free_double(stack_pop(operands));
    // Check for underflow again before we pop another operand
    if (*operands == NULL)
        return ERROR_SYNTAX;
    double val1 = free_double(stack_pop(operands));

    Status status = SUCCESS;
    switch (operator->symbol) {
        case '+': *result = val1 + val2; break;
        case '-': *result = val1 - val2; break;
        case '*': *result = val1 * val2; break;
        case '/': *result = val1 / val2; break;
        case '%': *result = fmod(val1, val2); break;
        case '^': *result = pow(val1, val2); break;
        case '=':
            if (0 == abs(val1 - val2)) {
                status = SUCCESS_EQUAL;
                *result = val1;  // operator returns original value
            } else {
                status = SUCCESS_NOT_EQUAL;
                *result = 0.0;
            }
            break;
        default: return ERROR_UNRECOGNIZED;  // unknown operator
    }

    stack_push(operands, result);
    return status;
}

/**
 * Apply a unary operator to the stack.
 */
Status apply_unary_operator(char op, Stack **operands) {
    double *result = calloc(1, sizeof (double));
    double val = free_double(stack_pop(operands));

    switch (op) {
        case '+': *result = val; break;  // values are assumed positive
        case '-': *result = -val; break;
        case '!': *result = tgamma(val + 1); break;
        default: return ERROR_UNRECOGNIZED;  // unknown operator
    }

    stack_push(operands, result);
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
    double arg = free_double(stack_pop(operands));
    double *result = calloc(1, sizeof (double));

    if (0 == strcasecmp(func, "abs"))
        *result = abs(arg);
    else if (0 == strcasecmp(func, "sqrt"))
        *result = sqrt(arg);
    else if (0 == strcasecmp(func, "ln"))
        *result = log(arg);
    else if (0 == strcasecmp(func, "lb"))
        *result = log2(arg);
    else if (0 == strcasecmp(func, "lg") || 0 == strcasecmp(func, "log"))
        *result = log10(arg);
    else if (0 == strcasecmp(func, "cos"))
        *result = cos(arg);
    else if (0 == strcasecmp(func, "sin"))
        *result = sin(arg);
    else if (0 == strcasecmp(func, "tan"))
        *result = tan(arg);
    else    /* unknown function */
        return ERROR_UNDEFINED_FUNCTION;

    stack_push(operands, result);
    return SUCCESS;
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

const Operator *get_operator(char symbol, bool unary) {
    for (size_t i = 0; i < sizeof OPERATORS / sizeof (Operator); i++) {
        if (OPERATORS[i].symbol == symbol && OPERATORS[i].unary == unary)
            return &OPERATORS[i];
    }
    return NULL;
}

double free_double(const double *pointer) {
    double value = *pointer;
    free((void *)pointer);
    return value;
}
