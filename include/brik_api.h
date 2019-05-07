/**
 * @file brik_api.h
 * @author bato
 * @date 23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __BRIK_API_H_
#define __BRIK_API_H_

/* *** Standard *** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

/* *** Linux System *** */
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

/* *** TCP/IP - Socket *** */
#include <netinet/in.h>
#include <netinet/ip.h>

#include <pthread.h>
#include <unistd.h>

/* *** SDL *** */
#ifdef SDL2
#include <SDL2/SDL.h>
#else // SDL1
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#endif

/* *** AV Library *** */
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>


/* *** Brik *** */
#include "brik_utils.h"

/* *** System *** */
#include "System/threadqueue.h"
#include "error_handler.h"

/* *** Module *** */
// Packet Handler
#include "packets.h"
#include "packet_handler.h"

// Connection Manager
#include "connect_mgmt.h"

// Video Handler
#include "video_handler.h"

// Display
#include "display.h"





#endif
