char *join_argv(int count, char *src[]);
void apply_operator(char *operator, stack *operands);
int compare_operators(char *op1, char *op2);
int num_digits(double num);
char *num_to_str(double num);
double strtod_unalloc(char *str);
