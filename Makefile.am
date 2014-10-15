bin_PROGRAMS = calc
calc_SOURCES = src/calc.c src/shunting-yard.c src/shunting-yard.h src/stack.c \
               src/stack.h
calc_LDADD = -lm

check_PROGRAMS = tests/shunting-yard
tests_shunting_yard_SOURCES = src/shunting-yard.c src/shunting-yard.h \
                              src/stack.c src/stack.h tests/shunting-yard.c
tests_shunting_yard_LDADD = -lm -lcunit

TESTS = $(check_PROGRAMS)
