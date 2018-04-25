# kblayout
Simple keyboard layout indicator. GNU/Linux

Compile with: 

    gcc -Wall -pedantic -std=c99 -Os -s -lX11 -lXft `pkg-config --cflags freetype2` kblayout.c -o kblayout
