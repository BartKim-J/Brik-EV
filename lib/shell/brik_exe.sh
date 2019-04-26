#!/bin/bash

# Airplay server
# Mobile
LD_LIBRARY_PATH=.:/home/brik/brik_ev_c/airplay_server/bin/lib/ /home/brik/brik_ev_c/airplay_server/bin/Airplay-ConnectivityForwarder -hqw 1600 -hqh 900 -lqw 1280 -lqh 720 &

# Desktop
#sudo LD_LIBRARY_PATH=.:/home/brik/airplay_server/bin/lib/ /home/brik/airplay_server/bin/Airplay-ConnectivityForwarder -hqw 1920 -hqh 1080 -lqw 1280 -lqh 720 &

# Brik
export SDL_NOMOUSE=1
export SDL_FBDEV=/dev/fb0
export SDL_VIDEODRIVER=fbcon
/brik/brik_ev_c/bin/build/BrikEVC_exe
