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
    char *val = strdup(list->top->val);
    stack_item *p = list->top;

    list->top = (stack_item *)list->top->next;
    free(p->val);
    free(p);

    return val;
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
