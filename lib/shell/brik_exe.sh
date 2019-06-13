#!/bin/bash

# Airplay server
# Mobile
/brik/brik_ev_c/airplay_server/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 640 -lqh 360 &

# Desktop
#/brik/airplay_server/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 1280 -lqh 720 &

# Brik
export SDL_NOMOUSE=1
export SDL_FBDEV=/dev/fb0
export SDL_VIDEODRIVER=fbcon
/brik/brik_ev_c/bin/build/BrikEVC_exe &
