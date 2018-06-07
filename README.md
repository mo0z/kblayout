# kblayout
Simple keyboard layout indicator. GNU/Linux

Build:

    gcc -Wall -pedantic -std=c99 -O3 -s -lX11 -lXft `pkg-config --cflags freetype2` kblayout.c -o kblayout
