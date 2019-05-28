/**
 * @file packet_video.c
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
#include "packet_video.h"

/* ******* STATIC FUNCTIONS ******* */
/* *** Packet Payload *** */
static void packet_video_codec(int connection_client, void* packet, void* payload);
static void packet_video_data(int connection_client, void* packet, void* payload);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LOCAL_PacketHandler_Video(int connection_client, void* packet, void* payload)
{
    Packet* received_video_packet;
    int32_t packet_type;

    received_video_packet = (Packet*)packet;
    packet_type = received_video_packet->hdr.type;

    switch(packet_type)
    {
        case PACKET_VIDEO_CODEC:
            packet_video_codec(connection_client, packet, payload);
            break;
        case PACKET_VIDEO_PACKET:
            packet_video_data(connection_client, packet, payload);
            break;
        case PACKET_CURSOR:
            //TODO: Handle Cursor data, Not supported by Airplay receiver
            break;
        case PACKET_CURSOR_POS:
            //TODO: Move cursor sprite, Not supported by Airplay receiver
            break;
        default:
            printf("Invalid video packet type\n");
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void packet_video_codec(int connection_client, void* packet, void* payload)
{
    CodecDataPacket* received_codec_packet;
    void* extradata;
    void* codec_packet;

/*
    int32_t video_codec_type;
    int32_t video_codec_width;
    int32_t video_codec_height;
    int32_t video_codec_framerate;
*/

    video_data_msg_t* message;
    int msg_ret = 0;

    received_codec_packet = (CodecDataPacket*)packet;

    //extradata = payload;
    codec_packet = malloc(sizeof(CodecDataPacket));
    if (codec_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate codec packet.");
    }
    memcpy(codec_packet, received_codec_packet, sizeof(CodecDataPacket));

    extradata = malloc(received_codec_packet->hdr.payloadSize);
    if (extradata == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate extradata to pass.");
    }
    memcpy(extradata, payload, received_codec_packet->hdr.payloadSize);

/*
    video_codec_type = received_codec_packet->video.codecType;   // Only support VIDEO_CODEC_H264 for now
    video_codec_width = received_codec_packet->video.width;
    video_codec_height = received_codec_packet->video.height;
    video_codec_framerate = received_codec_packet->video.framerate;  // Not used for now
*/

    // Need to parse SPS from extradata to get video resolution?
    // check if real resolution is okay.

    // let the video handler thread know the new video setting.


    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video handler message.");
    }

    message->packet = codec_packet;
    message->payload = extradata;

    printf("sending video codec codec_packet = %p, extradata = %p\n", codec_packet, extradata);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_CODEC);
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video_codec message to video handler: %d\n", msg_ret);
    }
}

static void packet_video_data(int connection_client, void* packet, void* payload)
{
    AVPacketPacket* received_video_data;
    video_data_msg_t* message;
    int msg_ret = 0;

    void* framedata;
    void* video_packet;

    received_video_data = (AVPacketPacket*)packet;

    // decode here and display from video thread? or just pass whole payload to video thread?
    if (received_video_data->hdr.payloadSize < 4)
    {
        printf("Invalid h.264 packet. Init seq is not included\n");
        return;
    }

    video_packet = malloc(sizeof(AVPacketPacket));
    if (video_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate AVPacket video packet.");
    }
    memcpy(video_packet, received_video_data, sizeof(AVPacketPacket));

    framedata = malloc(received_video_data->hdr.payloadSize);
    if (framedata == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame data.");
    }
    memcpy(framedata, payload, received_video_data->hdr.payloadSize);

    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video handler message.");
    }

    message->packet = video_packet;
    message->payload = framedata;

    //printf("sending video data message packet = %p, payload = %p\n", video_packet, framedata);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_DATA);
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video_codec message to video handler: %d\n", msg_ret);
    }
}


