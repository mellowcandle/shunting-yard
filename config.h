/* Number of characters to allocate for floats (including the decimal point) */
#define FLOAT_LENGTH 8

/* Error types */
#define ERROR_SYNTAX       1
#define ERROR_SYNTAX_STACK 2
#define ERROR_RIGHT_PAREN  3
#define ERROR_LEFT_PAREN   4
#define ERROR_UNRECOGNIZED 5

/* For calls to error() with an unknown column number */
#define NO_COL_NUM -2

/* Convenience functions */
#define is_operand(c) (c >= '0' && c <= '9')
#define is_operator(c) (c == '+' || c == '-' || c == '*' || c == '/' \
        || c == '^')
