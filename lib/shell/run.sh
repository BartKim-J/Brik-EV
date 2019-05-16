#!/bin/bash
export SDL_NOMOUSE=1
export SDL_FBDEV=/dev/fb0
export SDL_VIDEODRIVER=fbcon
/brik/brik_ev_c/bin/build/BrikEVC_exe
