// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "config.h"
#include "shunting-yard.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

bool sy_quiet = false;  // suppress error output when true

struct Operator {
    char symbol;
    int precedence;
    bool unary;
};

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

// Returns a matching operator.
static const Operator *get_operator(char symbol, bool unary) {
    for (size_t i = 0; i < sizeof OPERATORS / sizeof (Operator); i++) {
        if (OPERATORS[i].symbol == symbol && OPERATORS[i].unary == unary)
            return &OPERATORS[i];
    }
    return NULL;
}

/**
 * Parse a string and do the calculations. In the event of an error, will set
 * errno to an error code and return zero.
 */
double shunting_yard(char *str) {
    errno = 0;
    double result = 0;
    Stack *operands = NULL;
    Stack *operators = NULL;
    Stack *functions = NULL;

    // Loop variables
    int token_pos = -1;
    int paren_depth = 0;
    char prev_chr = '\0';
    const char *operand;

    // Variables used only for error() - not required for parsing
    int error_type = 0;
    int paren_pos = -1;

    // Loop through expression
    for (size_t i = 0; i <= strlen(str); ++i) {
        if (str[i] == ' ') continue;

        // Operands
        if (is_operand(str[i])) {
            if (token_pos == -1)
                token_pos = i;
            else if (is_alpha(str[i]) && is_numeric(prev_chr)) {
                // Parse expressions like "2a"
                if (!push_operand(str, token_pos, i, &operands))
                    goto exit;
                token_pos = i;

                // Emulate encountering a "*" operator, since "2a" implies
                // "2*a"
                if (!apply_stack_operators(&operators, &operands,
                        get_operator('*', false))) {
                    error(ERROR_SYNTAX, i, str);
                    goto exit;
                }
                stack_push(&operators, get_operator('*', false));
            } else if (is_numeric(str[i]) && is_alpha(prev_chr)) {
                // "a2" instead of "2a" is invalid
                error(ERROR_SYNTAX, i, str);
                goto exit;
            }

            goto skip;
        } else if (token_pos != -1) {   // end of operand
            if (!push_operand(str, token_pos, i, &operands))
                goto exit;
            token_pos = -1;
        }

        // Operators
        if (is_operator(str[i])) {
            bool unary = is_unary(str[i], prev_chr);

            // Apply any lower precedence operators on the stack first
            if (!apply_stack_operators(&operators, &operands,
                    get_operator(str[i], unary))) {
                error(ERROR_SYNTAX, i, str);
                goto exit;
            }

            // Push current operator
            stack_push(&operators, get_operator(str[i], unary));
        }
        // Parentheses
        else if (str[i] == '(') {
            // Check if this paren is starting a function
            if (is_operand(prev_chr))
                stack_push(&functions, stack_pop(&operands));

            stack_push(&operators, get_operator(str[i], false));
            ++paren_depth;
            if (paren_depth == 1) paren_pos = i;
        } else if (str[i] == ')') {
            if (!paren_depth) {
                error(ERROR_RIGHT_PAREN, i, str);
                goto exit;
            }

            // Pop and apply operators until we reach the left paren
            while (operators != NULL) {
                if (((const Operator *)stack_top(operators))->symbol == '(') {
                    stack_pop(&operators);
                    --paren_depth;
                    break;
                }

                if (!apply_operator(stack_pop(&operators), &operands)) {
                    // TODO: accurate column number (currently is just the col
                    // num of the right paren)
                    error(ERROR_SYNTAX, i, str);
                    goto exit;
                }
            }

            // Check if this is the end of a function
            if (functions != NULL) {
                operand = stack_pop(&functions);
                error_type = apply_function(operand, &operands);
                free((void *)operand);

                if (error_type != SUCCESS) {
                    // TODO: accurate column number
                    error(error_type, i, str);
                    goto exit;
                }
            }
        }
        // Unknown character
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

    // End of string - apply any remaining operators on the stack
    while (operators != NULL) {
        if (!apply_operator(stack_pop(&operators), &operands)) {
            error(ERROR_SYNTAX_STACK, NO_COL_NUM, str);
            goto exit;
        }
    }

    // Save the final result
    if (operands == NULL)
        error(ERROR_NO_INPUT, NO_COL_NUM, str);
    else
        result = strtod_unalloc(stack_pop(&operands));

exit:
    // Free memory and return
    while (operands != NULL)
        free((void *)stack_pop(&operands));
    while (operators != NULL)
        stack_pop(&operators);
    while (functions != NULL)
        free((void *)stack_pop(&functions));
    return result;
}

/**
 * Push an operand onto the stack and substitute any constants.
 */
bool push_operand(char *str, int pos_a, int pos_b, Stack **operands) {
    char *operand = rtrim(substr(str, pos_a, pos_b - pos_a));

    // Syntax check. Error if one of the following is true:
    //     1. Operand ONLY contains "."
    //     2. Operand contains a space
    //     3. Operand contains more than one "."
    if (strcmp(operand, ".") == 0 || strchr(operand, ' ') != NULL
            || strchr(operand, '.') != strrchr(operand, '.')) {
        error(ERROR_SYNTAX_OPERAND, pos_a, str);
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
            error(ERROR_CONST_UNDEF, pos_a, str);
            goto error;
        }
    }

    stack_push(operands, operand);
    return true;

error:
    free(operand);
    return false;
}

/**
 * Apply an operator to the top 2 operands on the stack.
 */
bool apply_operator(const Operator *operator, Stack **operands) {
    // Check for null op or underflow, as it indicates a syntax error
    if (operator == NULL || *operands == NULL)
        return false;

    if (operator->unary)
        return apply_unary_operator(operator->symbol, operands);

    double result;
    double val2 = strtod_unalloc(stack_pop(operands));
    // Check for underflow again before we pop another operand
    if (*operands == NULL)
        return false;
    double val1 = strtod_unalloc(stack_pop(operands));

    switch (operator->symbol) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/': result = val1 / val2; break;
        case '%': result = fmod(val1, val2); break;
        case '^': result = pow(val1, val2); break;
        case '=':
            if (0 == abs(val1 - val2)) {
                errno = SUCCESS_EQUAL;
                result = val1;  // operator returns original value
            } else {
                errno = SUCCESS_NOT_EQUAL;
                result = 0;
            }
            break;
        default: return false;  // unknown operator
    }

    stack_push(operands, num_to_str(result));
    return true;
}

/**
 * Apply a unary operator to the stack.
 */
bool apply_unary_operator(char op, Stack **operands) {
    double result;
    double val = strtod_unalloc(stack_pop(operands));

    switch (op) {
        case '+': result = val; break;  // values are assumed positive
        case '-': result = -val; break;
        case '!': result = tgamma(val + 1); break;
        default: return false;  // unknown operator
    }

    stack_push(operands, num_to_str(result));
    return true;
}

/**
 * Apply one or more operators currently on the stack.
 */
bool apply_stack_operators(Stack **operators, Stack **operands,
        const Operator *new_operator) {
    if (new_operator == NULL)
        return false;

    // Loop through the operator stack and apply operators until we reach one
    // that's of lower precedence (with different rules for unary operators)
    while (*operators != NULL) {
        if (((const Operator *)stack_top(*operators))->precedence >
                new_operator->precedence || new_operator->unary)
            break;
        if (!apply_operator(stack_pop(operators), operands))
            return false;
    }
    return true;
}

/**
 * Apply a function with arguments.
 */
int apply_function(const char *func, Stack **operands) {
    // Function arguments can't be void
    if (*operands == NULL)
        return ERROR_FUNC_NOARGS;

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
        return ERROR_FUNC_UNDEF;

    stack_push(operands, num_to_str(result));
    return SUCCESS;
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
 * Outputs an error.
 */
void error(int type, int col_num, char *str) {
    errno = type;
    if (sy_quiet) return;   // suppress error output

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
            strcat(error_str, "empty expression");
            break;
        case ERROR_FUNC_UNDEF:
            strcat(error_str, "undefined function");
            break;
        case ERROR_FUNC_NOARGS:
            strcat(error_str, "function requires arguments");
            break;
        case ERROR_CONST_UNDEF:
            strcat(error_str, "undefined constant");
            break;
        default:
            strcat(error_str, "unknown error");
    }

    // Output excerpt and column marker
    if (col_num != NO_COL_NUM) {
        strcat(error_str, ": ");

        ++col_num;  // width variables below start at 1, so this should too
        int total_width = TERM_WIDTH;
        int msg_width = (int)strlen(error_str);
        int avail_width = MIN(total_width - msg_width, (int)strlen(str) * 2);
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
char *substr(char *str, int start, size_t len) {
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
