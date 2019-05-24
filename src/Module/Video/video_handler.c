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
#include "video_handler.h"

/* ******* STATIC DEFINE ******* */
typedef struct threadqueue THQ;
typedef struct frameManager {
    bool isOccupied;
    AVFrame *frame;
    AVFrame *hw_frame;
} frameManager_t;

#define FRAME_BUFFER_MAX 35
/* ******* GLOBAL VARIABLE ******* */
/* *** SYSTEM *** */
static pthread_t thread_vh;
static THQ       queue_vh;

static pthread_t thread_fh;
static THQ       queue_fh;

frameManager_t frameManager[FRAME_BUFFER_MAX];

/* ******* STATIC FUNCTIONS ******* */
/* *** FFMPEG HANDLE VIDEO MESSAGE *** */
static ERROR_T handle_video_codec(CodecDataPacket* packet, void* payload);
static ERROR_T handle_video_data(AVPacketPacket* packet, void* payload);

/* *** THREAD *** */
static void    thread_FrameHandler_Cleanup(void *arg);
static void*   thread_FrameHandler(void *arg);

static void    thread_VideoHandler_Cleanup(void *arg);
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

    // init data queue for the thread
    ret = thread_queue_init(&queue_fh);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    ret = pthread_create(&thread_fh, NULL, thread_VideoHandler, NULL);

    return ret;
}

ERROR_T MODULE_VideoHandler_Destroy(void)
{
    ERROR_T ret  = ERROR_OK;
    void*   tret = NULL;

    thread_queue_cleanup(&queue_vh, true);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    ret = MODULE_Decoder_Uninit();
    //ret = MODULE_Image_CleanImages();

    ret = pthread_cancel(thread_vh);
    if (ret != ERROR_OK)
    {
        ERROR_SystemLog("Brik Failed to try cancle thread. \n\n");
    }


    ret = pthread_join(thread_vh, &tret);
    if (ret != ERROR_NOT_OK)
    {
        if (tret == PTHREAD_CANCELED)
            ERROR_SystemLog("Brik Done Video Handler Thread Clenaup. \n\n");

        else
            ERROR_SystemLog("Brik Failed Video Handler Thread Clenaup. \n\n");
    }

    return ret;
}

ERROR_T MODULE_VideoHandler_SendMessage(void* msg, VH_MSG_T message_type)
{
    return thread_queue_add(&queue_vh, msg, (long)message_type);
}

ERROR_T MODULE_FrameHandler_SendMessage(void* msg, FH_MSG_T message_type)
{
    return thread_queue_add(&queue_vh, msg, (long)message_type);
}


/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Thread
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static void thread_FrameHandler_Cleanup(void *arg)
{
    ERROR_SystemLog("Brik Start Frame Handler Thread Clenaup. \n\n");

    frame_data_msg_t* video_msg = arg;

    if(video_msg != NULL)
    {
        free(video_msg);
        video_msg = NULL;
    }
}
static void* thread_FrameHandler(void *arg)
{
    ERROR_T ret_queue = ERROR_OK;

    struct threadmsg message;

    frame_data_msg_t* frame_msg = NULL;

    // Thread Cleanup
    pthread_cleanup_push(thread_VideoHandler_Cleanup, frame_msg);

    while(true)
    {
        // get queued data
        ret_queue = thread_queue_get(&queue_fh, NULL, &message);
        if(ret_queue != 0)
        {
            printf("Failed to get video thread queue msg, %d\n", ret_queue);
            continue;
        }

        frame_msg = (frame_data_msg_t*)(message.data);

        // identify the type of packet if codec -> handle extradata and (re)init video decoder
        switch(message.msgtype)
        {
            case FH_MSG_TYPE_VIDEO_UPDATE:
                av_frame_free(&frameManager[index].hw_frame);
                av_frame_free(&frameManager[index].frame);
                break;

            case FH_MSG_TYPE_VIDEO_STOP:
                break;

            default:
                ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Invalid video pakcet.");
                break;
        }

        free(frame_msg);
    }

    pthread_cleanup_pop(0);

    return ERROR_OK;
}

static void thread_VideoHandler_Cleanup(void *arg)
{
    ERROR_SystemLog("Brik Start Video Handler Thread Clenaup. \n\n");

    video_data_msg_t* video_msg = arg;

    if(video_msg != NULL)
    {
        free(video_msg);
        video_msg = NULL;
    }
}

static void* thread_VideoHandler(void *arg)
{
    ERROR_T ret_queue = ERROR_OK;

    struct threadmsg message;

    video_data_msg_t* video_msg = NULL;
    SDL_Event event;

    // Thread Cleanup
    pthread_cleanup_push(thread_VideoHandler_Cleanup, video_msg);

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
                //@note Decoder Init In handle_video_codec
                handle_video_codec(video_msg->packet, video_msg->payload);
                break;

            case VH_MSG_TYPE_VIDEO_DATA:
                handle_video_data(video_msg->packet, video_msg->payload);
                break;

            case VH_MSG_TYPE_VIDEO_CONNECT:
                break;

            case VH_MSG_TYPE_VIDEO_DISCONNECT:
                MODULE_Image_UpdateImage(INTRO_IMAGE);
                MODULE_Decoder_Uninit();
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
                    SDL_Quit();
                    break;

                default:
                    break;
            }
        }
    }

    pthread_cleanup_pop(0);

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

    int             index = 0;
    unsigned char * data       = (unsigned char*)payload;

    ERROR_SystemLog("  \n- - - - - PACKET RECEIVE :: VIDEO(DATA) - - - - -\n");
    for (int i = 0; i < 40; i++)
    {
        printf("%02X ", data[i]);
    }
    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");

    MODULE_Decoder_Write(packet, payload);


    for(index = 0; index < FRAME_BUFFER_MAX; index++)
    {
        if(!frameManager[index].isOccupied)
        {
            /* av frame value allocate & init */
            frameManager[index].frame    = av_frame_alloc();
            frameManager[index].hw_frame = av_frame_alloc();

            if(frameManager[index].frame == NULL || frameManager[index].hw_frame == NULL)
            {
                ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame.");
            }

        }
    }

    MODULE_Decoder_Receive(frameManager[index].frame, frameManager[index].hw_frame, thread_queue_length(&queue_vh));

    free(packet);
    free(payload);

    return ret;
}
