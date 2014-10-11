CC = gcc
SOURCES = shunting-yard.c stack.c
DEPS = shunting-yard.h stack.h
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -D_BSD_SOURCE
LDFLAGS = -lm

.PHONY: all debug clean

all: calc

debug: CFLAGS += -O0 -g
debug: calc

clean:
	rm -f calc tests

calc: SOURCES += calc.c
calc: $(DEPS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -O2 $(SOURCES) $(LDFLAGS)

tests: SOURCES += tests.c
tests: $(DEPS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS) -lcunit
