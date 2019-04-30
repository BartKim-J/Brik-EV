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

/* *** Linux System *** */
#include <sys/socket.h>
#include <sys/types.h>

/* *** TCP/IP - Socket *** */
#include <netinet/in.h>
#include <netinet/ip.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* *** Renderer *** */
#ifdef SDL2
#include <SDL2/SDL.h>
#else // SDL1
#include <SDL/SDL.h>
#endif

#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

/* *** SYSTEM *** */
#include "sys/threadqueue.h"
#include "packets.h"

#include "brik_utils.h"
#include "error_handler.h"

/* *** MODULE *** */
// Packet Handler
#include "packet_handler.h"

// Connection Manager
#include "connect_mgmt.h"

// Video Handler
#include "video_handler.h"

// Display
#include "display_handler.h"





#endif
