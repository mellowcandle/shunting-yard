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
bool push_operand(char *str, int pos_a, int pos_b, stack *operands);
bool apply_operator(op_t *op, stack *operands);
bool apply_unary_operator(char op, stack *operands);
bool apply_stack_operators(char op, bool unary, stack *operands,
        stack *operators);
int apply_function(char *func, stack *args);
int compare_operators(op_t *op1, op_t *op2);
char *num_to_str(double num);
double strtod_unalloc(char *str);
void error(int type, int col_num, char *str);
char *substr(char *str, int start, int len);
bool is_unary(char op, char prev_chr);
char *trim_double(double num);
char *rtrim(char *str);
op_t *get_op(char op, bool unary);
