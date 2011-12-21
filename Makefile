CC = clang
CFLAGS = -std=gnu99 -lm -o shunting-yard
FILES = shunting-yard.c stack.c

all:
	$(CC) $(CFLAGS) -O2 $(FILES)

debug:
	$(CC) $(CFLAGS) -O0 -g $(FILES)
