CC = $(shell basename `which clang || echo gcc`)
CFLAGS = -std=c99 -D_BSD_SOURCE -lm
FILES = shunting-yard.c stack.c

all:
	$(CC) $(CFLAGS) -o calc -O2 $(FILES) calc.c

debug:
	$(CC) $(CFLAGS) -o calc -O0 -g $(FILES) calc.c

test:
	$(CC) $(CFLAGS) -lcunit -o tests $(FILES) tests.c
	@./tests

clean:
	rm -f calc tests
