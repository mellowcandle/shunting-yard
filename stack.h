typedef struct {
    char val;
    struct stack_item *next;
} stack_item;

typedef struct {
    stack_item *current;
    stack_item *top;
} stack;

void stack_push(stack *list, char val);
char stack_pop(stack *list);
void stack_display(stack *list);
