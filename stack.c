/*
 * Copyright 2011 Brian Marshall. All rights reserved.
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
#include <stdbool.h>
#include "stack.h"

stack *stack_alloc() {
    stack *list = malloc(sizeof(stack));
    memset(list, 0, sizeof(stack));
    return list;
}

void stack_push(stack *list, char *val) {
    list->current = malloc(sizeof(stack_item));
    list->current->val = strdup(val);
    list->current->next = (struct stack_item *)list->top;
    list->top = list->current;
}

void stack_push_unalloc(stack *list, char *val) {
    stack_push(list, val);
    free(val);
}

char *stack_pop(stack *list) {
    if (stack_is_empty(list)) {
        printf("Stack underflow\n");
        return NULL;
    }

    char *val = strdup(list->top->val);
    stack_item *p = list->top;

    list->top = (stack_item *)list->top->next;
    free(p->val);
    free(p);

    return val;
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

char *stack_top(stack *list) {
    return list->top->val;
}

void stack_display(stack *list) {
    stack_item *p = list->top;
    while (p != NULL) {
        printf("%s ", p->val);
        p = (stack_item *)p->next;
    }

    printf("\n");
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
