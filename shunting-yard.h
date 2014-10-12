// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <stdbool.h>
#include <errno.h>
#include "config.h"
#include "stack.h"

enum op_types { OP_BINARY  = 1 << 0, OP_UNARY = 1 << 1, OP_PREFIX = 1 << 2,
                OP_POSTFIX = 1 << 3, OP_NONE  = 1 << 4 };
typedef struct {
    char op;
    short int prec;
    enum op_types type;
} const op_t;

extern bool sy_quiet;

double shunting_yard(char *str);
bool push_operand(char *str, int pos_a, int pos_b, Stack **operands);
bool apply_operator(op_t *op, Stack **operands);
bool apply_unary_operator(char op, Stack **operands);
bool apply_stack_operators(char op, bool unary, Stack **operands,
        Stack **operators);
int apply_function(const char *func, Stack **args);
int compare_operators(op_t *op1, op_t *op2);
char *num_to_str(double num);
double strtod_unalloc(const char *str);
void error(int type, int col_num, char *str);
char *substr(char *str, int start, size_t len);
bool is_unary(char op, char prev_chr);
char *trim_double(double num);
char *rtrim(char *str);
op_t *get_op(char op, bool unary);
