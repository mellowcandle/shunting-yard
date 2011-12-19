CC = clang
FILES = shunting-yard.c stack.c

all:
	$(CC) -O2 -o shunting-yard $(FILES)

debug:
	$(CC) -O0 -g -o shunting-yard $(FILES)
