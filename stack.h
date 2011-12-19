#include <stdbool.h>

typedef struct {
    char *val;
    struct stack_item *next;
} stack_item;

typedef struct {
    stack_item *current;
    stack_item *top;
} stack;

stack *stack_alloc();
void stack_push(stack *list, char *val);
void stack_push_unalloc(stack *list, char *val);
char *stack_pop(stack *list);
void stack_display(stack *list);
bool stack_is_empty(stack *list);
void stack_free(stack *list);
