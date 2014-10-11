// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "stack.h"

#include <stdlib.h>
#include <string.h>

stack *stack_alloc() {
    stack *list = malloc(sizeof (stack));
    memset(list, 0, sizeof (stack));
    return list;
}

void stack_push(stack *list, char *val, short int flags) {
    list->current = malloc(sizeof (stack_item));
    list->current->val = strdup(val);
    list->current->flags = flags;
    list->current->next = (struct stack_item *)list->top;
    list->top = list->current;
}

void stack_push_unalloc(stack *list, char *val, short int flags) {
    stack_push(list, val, flags);
    free(val);
}

/**
 * Pop from the stack.
 */
char *stack_pop(stack *list) {
    stack_item *item = stack_pop_item(list);
    char *val = strdup(item->val);
    stack_free_item(item);
    return val;
}

/**
 * Pop from the stack, and return the entire stack_item (including value and
 * flags).
 */
stack_item *stack_pop_item(stack *list) {
    if (stack_is_empty(list))
        return NULL;

    stack_item *item = list->top;
    list->top = (stack_item *)list->top->next;
    return item;
}

/**
 * Pop from the stack, but only return a single character, so memory doesn't
 * have to be freed.
 */
char stack_pop_char(stack *list) {
    char *val = stack_pop(list);
    char val_char = val[0];
    free(val);
    return val_char;
}

/**
 * Return the value of the item from the top of the stack without popping.
 */
char *stack_top(stack *list) {
    return list->top->val;
}

/**
 * Return the item from the top of the stack without popping.
 */
stack_item *stack_top_item(stack *list) {
    return list->top;
}

bool stack_is_empty(stack *list) {
    stack_item *p = list->top;
    return (bool)(p == NULL);
}

void stack_free(stack *list) {
    stack_item *p = list->top;
    while (p != NULL) {
        p = (stack_item *)p->next;
        free(stack_pop(list));
    }
    free(list);
}

/**
 * Free an item from the stack after popping it.
 */
void stack_free_item(stack_item *item) {
    free(item->val);
    free(item);
}
