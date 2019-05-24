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
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <errno.h>

/* *** Linux System *** */
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/reboot.h>

/* *** TCP/IP - Socket *** */
#include <netinet/in.h>
#include <netinet/ip.h>

/* *** Thread *** */
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
#include "threadqueue.h"
#include "error_handler.h"

/* *** Module *** */
// Display
#include "display.h"

// Packet
#include "connect_mgmt.h"

#include "packets.h"
#include "packet_handler.h"

// Video
#include "images.h"
#include "decoder.h"

#include "video_handler.h"







#endif
