/**
 * @file packets_audio.h
 * @author Ben
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __PACKET_AUDIO_H_
#define __PACKET_AUDIO_H_

extern void LOCAL_PacketHandler_Audio(int connection_client, void* packet, void* payload);
extern void LOCAL_PacketHandler_Backchannel(int connection_client, void* packet, void* payload);

#endif
