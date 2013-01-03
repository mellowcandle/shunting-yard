This is a small program I made to improve my C skills. It's a working
command-line calculator written from scratch that implements the [shunting-yard
algorithm][1] to parse input.

Compiling should be as simple as running `make` with the C standard library
development headers installed. You need CUnit headers to run the unit tests,
though (`make test`).

Usage is simple:

    $ ./calc 2 + 2
    4
    $ ./calc 2^3
    8

A number of common math functions and constants are supported: abs(), sqrt(),
ln(), lb(), lg() or log(), cos(), sin(), tan(), pi, tau, and e.

[1]: http://en.wikipedia.org/wiki/Shunting-yard_algorithm
