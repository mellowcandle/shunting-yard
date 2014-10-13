CC = gcc
SOURCES = shunting-yard.c stack.c
calc_SOURCES = $(SOURCES) calc.c
tests_SOURCES = $(SOURCES) tests.c
DEPS = shunting-yard.h stack.h
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -D_BSD_SOURCE
LDFLAGS = -lm

.PHONY: all debug clean

all: calc

debug: CFLAGS += -O0 -g
debug: calc

clean:
	rm -f calc tests

calc: $(DEPS) $(calc_SOURCES)
	$(CC) $(CFLAGS) -o $@ -O2 $(calc_SOURCES) $(LDFLAGS)

tests: $(DEPS) $(tests_SOURCES)
	$(CC) $(CFLAGS) -o $@ $(tests_SOURCES) $(LDFLAGS) -lcunit
