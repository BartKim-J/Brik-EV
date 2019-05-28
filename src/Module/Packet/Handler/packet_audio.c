/**
 * @file packet_audio.c
 * @author Ben
 * @date 5 Tue 2019
 * @brief
 *
 * @bug sw decoder not working maybe sdl option.
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
/* ******* INCLUDE ******* */
#include "brik_api.h"
#include "packet_audio.h"

/* ******* STATIC FUNCTIONS ******* */
/* *** Packet Payload *** */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LOCAL_PacketHandler_Audio(int connection_client, void* packet, void* payload)
{
    //TODO: handle audio packets
}

void LOCAL_PacketHandler_Backchannel(int connection_client, void* packet, void* payload)
{
    //TODO: Handle backchannel packets Not for Airplay
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
