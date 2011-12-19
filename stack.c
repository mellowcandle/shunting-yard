#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

void stack_push(stack *list, char val) {
    list->current = malloc(sizeof(stack_item));
    list->current->val = val;
    list->current->next = (struct stack_item *)list->top;
    list->top = list->current;
}

char stack_pop(stack *list) {
    char val = list->top->val;
    list->top = (stack_item *)list->top->next;

    return val;
}

void stack_display(stack *list) {
    stack_item *pos = list->top;
    while (pos != NULL) {
        printf("%c ", pos->val);
        pos = (stack_item *)pos->next;
    }

    printf("\n");
}
