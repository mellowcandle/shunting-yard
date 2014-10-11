// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <stdbool.h>

typedef struct {
    char *val;
    short int flags;
    struct stack_item *next;
} stack_item;

typedef struct {
    stack_item *current;
    stack_item *top;
} stack;

stack *stack_alloc(void);
void stack_push(stack *list, char *val, short int flags);
void stack_push_unalloc(stack *list, char *val, short int flags);
char *stack_pop(stack *list);
stack_item *stack_pop_item(stack *list);
char stack_pop_char(stack *list);
char *stack_top(stack *list);
stack_item *stack_top_item(stack *list);
bool stack_is_empty(stack *list);
void stack_free(stack *list);
void stack_free_item(stack_item *item);
