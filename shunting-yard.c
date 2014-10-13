// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.h"
#include "stack.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define IS_NUMBER(c) ((c) == '.' || isdigit((c)))
#define IS_OPERAND(c) (isalpha((c)) || IS_NUMBER((c)))
#define IS_OPERATOR(c) ((c) != '\0' && strchr("!^+-*/%=", (c)) != NULL)

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

// Pushes an operand (number literal or constant) to the stack.
//
// This also recognizes function identifiers and adds them to the function stack
// instead. TODO: Fix this parsing quirk.
static Status push_operand(const char *string, int length, Stack **operands,
                           Stack **functions);

// Applies an operator to the top one or two operands, depending on if the
// operator is unary or binary.
static Status apply_operator(const Operator *operator, Stack **operands);

// Applies a unary operator to the top operand.
static Status apply_unary_operator(const Operator *operator, Stack **operands);

// When a new operator is encountered, applies previous operators in the stack
// that have a higher precedence.
static Status apply_stack_operators(const Operator *new_operator,
                                    Stack **operands, Stack **operators);

// Applies a function to the top operand.
static Status apply_function(const char *function, Stack **operands);

// Returns true if an operator is unary, using the previous character for
// context.
static bool is_unary_operator(char symbol, char previous);

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

    for (size_t i = 0; i <= strlen(expression); i++) {
        char c = expression[i];
        if (isspace(c))
            continue;
        char previous = (i == 0 ? '\0' : expression[i - 1]);
        error_index = i;

        // Operands:
        if (IS_OPERAND(c)) {
            if (token_pos == -1)
                token_pos = i;
            else if (isalpha(c) && IS_NUMBER(previous)) {
                // Handle implicit multiplication in the form of "2x".
                status = push_operand(expression + token_pos, i - token_pos,
                                      &operands, &functions);
                if (status > SUCCESS) {
                    error_index = token_pos;
                    goto exit;
                }
                token_pos = i;

                const Operator *operator = get_operator('*', false);
                status = apply_stack_operators(operator, &operands, &operators);
                if (status > SUCCESS)
                    goto exit;
                stack_push(&operators, operator);
            } else if (isalpha(previous) && IS_NUMBER(c)) {
                // Number literals following an identifier ("x2") are invalid.
                status = ERROR_SYNTAX;
                goto exit;
            }
            continue;
        } else if (token_pos != -1) {
            // End of this operand token.
            status = push_operand(expression + token_pos, i - token_pos,
                                  &operands, &functions);
            if (status > SUCCESS) {
                error_index = token_pos;
                goto exit;
            }
            token_pos = -1;
        }

        // Operators:
        if (IS_OPERATOR(c)) {
            const Operator *operator = get_operator(
                c, is_unary_operator(c, previous));

            // Apply previous operators before pushing this one.
            status = apply_stack_operators(operator, &operands, &operators);
            if (status > SUCCESS)
                goto exit;
            stack_push(&operators, operator);
        }
        // Parentheses:
        else if (c == '(') {
            stack_push(&operators, get_operator(c, false));
            paren_depth++;
            if (paren_depth == 1)
                paren_pos = i;
        } else if (c == ')') {
            if (!paren_depth) {
                status = ERROR_CLOSE_PARENTHESIS;
                goto exit;
            }

            // Apply operators until the initial open parenthesis is found.
            while (operators != NULL) {
                const Operator *operator = stack_pop(&operators);
                if (operator->symbol == '(') {
                    paren_depth--;
                    break;
                }
                status = apply_operator(operator, &operands);
                if (status > SUCCESS)
                    goto exit;  // TODO: More accurate error column number.
            }

            // Apply a function if there is one.
            if (functions != NULL) {
                const char *function = stack_pop(&functions);
                status = apply_function(function, &operands);
                free((void *)function);
                if (status > SUCCESS)
                    goto exit;  // TODO: More accurate error column number.
            }
        }
        // Unknown character:
        else if (c != '\0' && c != '\n') {
            status = ERROR_UNRECOGNIZED;
            goto exit;
        }
    }

    if (paren_depth) {
        error_index = paren_pos;
        status = ERROR_OPEN_PARENTHESIS;
        goto exit;
    }

    // Apply all remaining operators.
    while (operators != NULL) {
        status = apply_operator(stack_pop(&operators), &operands);
        if (status > SUCCESS)
            goto exit;
    }

    // Get the final result.
    if (operands == NULL)
        status = ERROR_NO_INPUT;
    else
        *result = free_double(stack_pop(&operands));

exit:
    // Free elements in the stacks, except for operators, which are static.
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
    while (length > 0 && isspace(string[length - 1]))
        length--;  // Strip whitespace from the end.
    char token[length + 1];
    snprintf(token, length + 1, "%s", string);

    double x = 0.0;
    if (isalpha(token[0])) {
        if (strcasecmp(token, "e") == 0)
            x = M_E;
        else if (strcasecmp(token, "pi") == 0)
            x = M_PI;
        else if (strcasecmp(token, "tau") == 0)
            x = M_PI * 2;
        else if (string[length] == '(') {
            // This operand is a function identifier instead.
            stack_push(functions, strdup(token));
            return SUCCESS;
        } else
            return ERROR_UNDEFINED_CONSTANT;
    } else {
        char *end = NULL;
        x = strtod(token, &end);

        // If not all of the token is converted, the rest is invalid.
        if (token + length != end)
            return ERROR_SYNTAX;
    }

    double *operand = malloc(sizeof (double));
    *operand = x;
    stack_push(operands, operand);
    return SUCCESS;
}

Status apply_operator(const Operator *operator, Stack **operands) {
    if (operator == NULL || *operands == NULL)
        return ERROR_SYNTAX;
    if (operator->unary)
        return apply_unary_operator(operator, operands);

    double y = free_double(stack_pop(operands));
    if (*operands == NULL)
        return ERROR_SYNTAX;
    double x = free_double(stack_pop(operands));
    double *result = calloc(1, sizeof (double));

    Status status = SUCCESS;
    switch (operator->symbol) {
        case '+':
            *result = x + y;
            break;
        case '-':
            *result = x - y;
            break;
        case '*':
            *result = x * y;
            break;
        case '/':
            *result = x / y;
            break;
        case '%':
            *result = fmod(x, y);
            break;
        case '^':
            *result = pow(x, y);
            break;
        case '=':
            if (abs(x - y) == 0) {
                status = SUCCESS_EQUAL;
                *result = x;
            } else
                status = SUCCESS_NOT_EQUAL;
            break;
        default:
            free(result);
            return ERROR_UNRECOGNIZED;
    }
    stack_push(operands, result);
    return status;
}

Status apply_unary_operator(const Operator *operator, Stack **operands) {
    double x = free_double(stack_pop(operands));
    double *result = calloc(1, sizeof (double));
    switch (operator->symbol) {
        case '+':
            *result = +x;
            break;
        case '-':
            *result = -x;
            break;
        case '!':
            *result = tgamma(x + 1);
            break;
        default:
            return ERROR_UNRECOGNIZED;
    }
    stack_push(operands, result);
    return SUCCESS;
}

Status apply_stack_operators(const Operator *new_operator, Stack **operands,
                             Stack **operators) {
    if (new_operator == NULL)
        return ERROR_SYNTAX;

    while (*operators != NULL) {
        // Stop when an operator with lower precedence is found.
        if (((const Operator *)stack_top(*operators))->precedence >
                new_operator->precedence || new_operator->unary)
            break;

        Status status = apply_operator(stack_pop(operators), operands);
        if (status > SUCCESS)
            return status;
    }
    return SUCCESS;
}

Status apply_function(const char *function, Stack **operands) {
    if (*operands == NULL)
        return ERROR_FUNCTION_ARGUMENTS;
    double x = free_double(stack_pop(operands));
    double *result = calloc(1, sizeof (double));

    if (strcasecmp(function, "abs") == 0)
        *result = abs(x);
    else if (strcasecmp(function, "sqrt") == 0)
        *result = sqrt(x);
    else if (strcasecmp(function, "ln") == 0)
        *result = log(x);
    else if (strcasecmp(function, "lb") == 0)
        *result = log2(x);
    else if (strcasecmp(function, "lg") == 0 ||
             strcasecmp(function, "log") == 0)
        *result = log10(x);
    else if (strcasecmp(function, "cos") == 0)
        *result = cos(x);
    else if (strcasecmp(function, "sin") == 0)
        *result = sin(x);
    else if (strcasecmp(function, "tan") == 0)
        *result = tan(x);
    else
        return ERROR_UNDEFINED_FUNCTION;

    stack_push(operands, result);
    return SUCCESS;
}

// Treat open parentheses as part of the operand for prefix operators, and close
// parentheses for postfix operators.
bool is_unary_operator(char symbol, char previous) {
    // Handle postfix unary operators such as '!'.
    if (symbol == '!' && (IS_OPERAND(previous) || previous == ')'))
        return true;
    if (symbol != '!' && previous == '!')
        return false;

    return IS_OPERATOR(previous) || previous == '\0' || previous == '(';
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
