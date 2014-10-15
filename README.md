This is a small command-line calculator that implements the [shunting-yard
algorithm][1] to parse input. It's written in C and depends only on the C
standard library.

Building from source:

    $ autoreconf -i
    $ ./configure
    $ make

Using the calculator:

    $ ./calc 1+2^3-4*5
    -11
    $ ./calc '3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3'
    3.0001220703125
    $ ./calc 'sin(pi)'
    0

Multiple shell arguments are evaluated separately:

    $ ./calc 1+2 '3+4 + 5+6' 7+8
    3
    18
    15

A number of common math functions and constants are supported: abs(), sqrt(),
ln(), lb(), lg() or log(), cos(), sin(), tan(), pi, tau, and e.

[1]: http://en.wikipedia.org/wiki/Shunting-yard_algorithm
