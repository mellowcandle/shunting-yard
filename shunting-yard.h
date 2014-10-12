// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "stack.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

extern bool sy_quiet;

typedef struct Operator Operator;

double shunting_yard(char *str);
bool push_operand(char *str, int pos_a, int pos_b, Stack **operands);
bool apply_operator(const Operator *operator, Stack **operands);
bool apply_unary_operator(char op, Stack **operands);
bool apply_stack_operators(Stack **operators, Stack **operands,
        const Operator *new_operator);
int apply_function(const char *func, Stack **args);
char *num_to_str(double num);
double strtod_unalloc(const char *str);
void error(int type, int col_num, char *str);
char *substr(char *str, int start, size_t len);
bool is_unary(char op, char prev_chr);
char *trim_double(double num);
char *rtrim(char *str);
