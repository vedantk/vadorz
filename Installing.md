# Note #
This page describes how to build vadorz. If you just want to play the game, please see the Downloads tab.

# Introduction #
  1. You need the ncurses library (if you are using Linux/BSD/+nix) or the pdcurses library (if you are using Windows).
  1. You need a modern C compiler.
  1. Download the source from SVN, or online.

# Checking Out #
In order to get the code from SVN (the very latest builds):
  * `$ svn co http://vadorz.googlecode.com/svn/trunk/ vadorz`
  * `$ cd vadorz/`

# Compiling #
For gcc/ncurses:
> `$ gcc vadorz.c -O2 -lncurses -o vadorz`

For pdcurses: use -lcurses

# Running #
To play:
> `$ ./vadorz`
or
> `$ xterm -e ./vadorz`

Enjoy!