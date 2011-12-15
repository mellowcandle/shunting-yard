all:
	clang -O2 -o shunting-yard shunting-yard.c

debug:
	clang -O0 -g -o shunting-yard shunting-yard.c
