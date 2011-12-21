CC = clang
CFLAGS = -std=c99 -D_XOPEN_SOURCE=500 -lm -o shunting-yard
FILES = shunting-yard.c stack.c

all:
	$(CC) $(CFLAGS) -O2 $(FILES)

debug:
	$(CC) $(CFLAGS) -O0 -g $(FILES)
