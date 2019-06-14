#!/bin/bash
# debug(gdb) #
SDL_NOMOUSE=1 SDL_FBDEV=/dev/fb0 SDL_VIDEODRIVER=fbcon gdb /brik/brik_ev_c/bin/build/BrikEVC_exe
