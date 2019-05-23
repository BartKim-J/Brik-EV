/**
 * @file video_handler.c
 * @author Ben
 * @date 4 Tue 2019
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

/* ******* STATIC DEFINE ******* */
typedef struct threadqueue THQ;

/* ******* GLOBAL VARIABLE ******* */
/* *** SYSTEM *** */
static pthread_t thread_vh;
static THQ       queue_vh;

/* ******* STATIC FUNCTIONS ******* */

/* *** FFMPEG HANDLE VIDEO MESSAGE *** */
static ERROR_T handle_video_codec(CodecDataPacket* packet, void* payload);
static ERROR_T handle_video_data(AVPacketPacket* packet, void* payload);
static ERROR_T handle_video_stop(void);

/* *** THREAD *** */
static void*   thread_VideoHandler(void *arg);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_VideoHandler_Init(void)
{
    ERROR_T ret = ERROR_OK;

    // init data queue for the thread
    ret = thread_queue_init(&queue_vh);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    ret = pthread_create(&thread_vh, NULL, thread_VideoHandler, NULL);

    return ret;
}
ERROR_T MODULE_VideoHandler_Destroy(void)
{
    ERROR_T ret = ERROR_OK;

    thread_queue_cleanup(&queue_vh, true);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    pthread_exit(&thread_vh);

    return ret;
}

ERROR_T MODULE_VideoHandler_SendMessage(void* msg, VH_MSG_T message_type)
{
    return thread_queue_add(&queue_vh, msg, (long)message_type);
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Thread
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static void* thread_VideoHandler(void *arg)
{
    ERROR_T ret_queue = ERROR_OK;

    struct threadmsg message;

    video_data_msg_t* video_msg = NULL;
    SDL_Event event;

    MODULE_Image_LoadImages();

    MODULE_Image_UpdateImage(INTRO_IMAGE);

    while(true)
    {
        // get queued data
        ret_queue = thread_queue_get(&queue_vh, NULL, &message);
        if(ret_queue != 0)
        {
            printf("Failed to get video thread queue msg, %d\n", ret_queue);
            continue;
        }

        video_msg = (video_data_msg_t*)(message.data);

        // identify the type of packet if codec -> handle extradata and (re)init video decoder
        switch(message.msgtype)
        {
            case VH_MSG_TYPE_VIDEO_CODEC:
                handle_video_codec(video_msg->packet, video_msg->payload);
                break;

            case VH_MSG_TYPE_VIDEO_DATA:
                handle_video_data(video_msg->packet, video_msg->payload);
                break;

            case VH_MSG_TYPE_VIDEO_CONNECT:
                break;

            case VH_MSG_TYPE_VIDEO_DISCONNECT:
                handle_video_stop();

                MODULE_Image_UpdateImage(INTRO_IMAGE);
                break;

            default:
                ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Invalid video pakcet.");
                break;
        }

        free(video_msg);

        while( SDL_PollEvent( &event ) )
        {
            /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
            switch( event.type )
            {
                case SDL_QUIT:
                    MODULE_Image_CleanImages();
                    SDL_Quit();
                    break;

                default:
                    break;
            }
        }
    }

    return ERROR_OK;
}

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *
  *  Thread Handle Functions.
  *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static ERROR_T handle_video_codec(CodecDataPacket* packet, void* payload)
{
    unsigned short sps_len = 0;
    unsigned char* p_extra = (unsigned char*)payload;

    int32_t width_sps = 0;
    int32_t height_sps = 0;

    MODULE_Display_Clean();

    ERROR_SystemLog("\n\n- - - - PACKET RECIVE :: VIDEO(CODEC) - - - -\n\n");

    printf("Frame Dimension from packet->video.width = %d, packet->video_height = %d\n", packet->video.width, packet->video.height);

    sps_len = (p_extra[6] << 8) | p_extra[7];

    printf("\nextradata = \n");
    for (int i = 0; i< packet->hdr.payloadSize; i++)
    {
        printf("%02X ", p_extra[i]);
    }
    printf("\n\n");

    printf("length sps = %d\n", sps_len);

    sps_parse(&p_extra[9], sps_len - 1, &width_sps, &height_sps);

    if(width_sps != packet->video.width || height_sps != packet->video.height)
    {
        // Let's suppose extradata has correct data
        printf("Frame dimension from PACKET_VIDEO_CODEC is different to dimensions from extradata\n");
        printf("packet: %d x %d, extradata(sps): %d x %d\n", packet->video.width, packet->video.height, width_sps, height_sps);
        printf("Using resulotion from SPS\n\n\n");
        printf("TODO: FIX Resolution calculation\n\n\n");
    }

    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - \n");

    if(MODULE_Decoder_Init(packet, payload, packet->hdr.payloadSize) < 0)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to init video decoder.");
    }

    free(packet);
    free(payload);

    return ERROR_OK;
}

static ERROR_T handle_video_data(AVPacketPacket* packet, void* payload)
{
    ERROR_T ret = ERROR_OK;

    AVFrame *frame = NULL;      // Must allocaate with "av_frame_alloc()
    AVFrame *hw_frame = NULL;   // Must allocaate with "av_frame_alloc()
    unsigned char * data = (unsigned char*)payload;

    ERROR_SystemLog("  \n- - - - - PACKET RECEIVE :: VIDEO(DATA) - - - - -\n");
    for (int i = 0; i < 40; i++)
    {
        printf("%02X ", data[i]);
    }
    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");

    /* av frame value allocate & init */
    frame    = av_frame_alloc();
    hw_frame = av_frame_alloc();

    if(frame == NULL || hw_frame == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame.");
    }

    MODULE_Decoder_Write(packet, payload);

    MODULE_Decoder_Receive(frame, hw_frame, thread_queue_length(&queue_vh));

    av_frame_free(&hw_frame);
    av_frame_free(&frame);

    free(packet);
    free(payload);

    return ret;
}

static ERROR_T handle_video_stop(void)
{
    int status = ERROR_OK;

    thread_queue_cleanup(&queue_vh, true);
    if(status != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    status = thread_queue_init(&queue_vh);
    if(status != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    return status;
}

