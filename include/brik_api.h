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

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#ifdef SDL2
#include <SDL2/SDL.h>
#else // SDL1
#include <SDL/SDL.h>
#include <libswscale/swscale.h>
#endif

/* Type */
#include "packets.h"

// brik
#include "brik_utils.h"
#include "brik_error.h"

// brik system
#include "sys/threadqueue.h"

// packet handler
#include "packet_handler.h"
// connection manager
#include "connect_mgmt.h"
// video_handler
#include "video_handler.h"
// display handler
#include "display_handler.h"



#endif
