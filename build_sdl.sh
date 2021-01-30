#!/bin/bash

gcc -O3 -ftree-vectorize -o maze_sdl main.c draw.c ui.c -lSDL2 -lSDL2_gfx
