CC = $(shell basename `which clang || echo gcc`)
CFLAGS = -std=c99 -D_XOPEN_SOURCE=500 -lm -o calc
FILES = calc.c shunting-yard.c stack.c

all:
	$(CC) $(CFLAGS) -O2 $(FILES)

debug:
	$(CC) $(CFLAGS) -O0 -g $(FILES)
