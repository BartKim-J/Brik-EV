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
#include "frame_handler.h"

/* ******* STATIC DEFINE ******* */
typedef struct threadqueue THQ;

/* *** IMAGES *** */
#define UNALLOCATED_RESOLUTION           0
#define UNALLOCATED_FORMAT               0


/* *** FRAME *** */
#define TARGET_MOBILE_FPS               60 // frame per second
#define TARGET_DESKTOP_FPS              30

#define TARGET_FPD                       5

#define TARGET_SKIP_FRAME_MAX            3

#define FRAME_BUFFER_MAX                50

/* ******* GLOBAL VARIABLE ******* */
static pthread_t       thread_fh;
static THQ             queue_fh;
static pthread_mutex_t mutex_fh = PTHREAD_MUTEX_INITIALIZER;

/* *** FRAME *** */
static int prevFrame_Width  = UNALLOCATED_RESOLUTION;
static int prevFrame_Height = UNALLOCATED_RESOLUTION;

frame_data_t frameBuffer[FRAME_BUFFER_MAX];
int          allocatedFrame;

/* ******* STATIC FUNCTIONS ******* */
/* *** THREAD *** */
static void    thread_FrameHandler_Cleanup(void *arg);
static void*   thread_FrameHandler(void *arg);

/* *** FRAME *** */
static ERROR_T  sFrameBuffer_Cleanup(void);
static ERROR_T  sFrameBuffer_Display(frame_data_t* frameData);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ERROR_T MODULE_FrameHandler_Init(void)
{
    ERROR_T ret = ERROR_OK;

    allocatedFrame = 0;

    // init data queue for the thread
    ret = thread_queue_init(&queue_fh);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize frame queue.");
    }

    ret = pthread_create(&thread_fh, NULL, thread_FrameHandler, NULL);

    ret = pthread_mutex_init(&mutex_fh, NULL);
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize mutex.");
    }

    return ret;
}

ERROR_T MODULE_FrameHandler_Destroy(void)
{
    ERROR_T ret  = ERROR_OK;
    void*   tret = NULL;

    thread_queue_cleanup(&queue_fh, true);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize frame queue.");
    }

    ret = pthread_cancel(thread_fh);
    if (ret != ERROR_OK)
    {
        ERROR_SystemLog("Brik Failed to try cancle thread. \n\n");
    }

    ret = pthread_join(thread_fh, &tret);
    if (ret != ERROR_NOT_OK)
    {
        if (tret == PTHREAD_CANCELED)
            ERROR_SystemLog("Brik Done Frame Handler Thread Clenaup. \n\n");

        else
            ERROR_SystemLog("Brik Failed Frame Handler Thread Clenaup. \n\n");
    }

    sFrameBuffer_Cleanup();

    return ret;
}

frame_data_t* Module_FrameHandler_BufferAlloc(void)
{
    int             index = 0;
    frame_data_t*   retFrame = NULL;

    pthread_mutex_lock(&mutex_fh);

    if(allocatedFrame < (FRAME_BUFFER_MAX - 1))
    {

        for(index = 0; index < FRAME_BUFFER_MAX; index++)
         {
             if(!frameBuffer[index].isOccupied)
             {
                 /* av frame value allocate & init */
                 frameBuffer[index].isOccupied    = true;
                 frameBuffer[index].frame         = av_frame_alloc();
                 frameBuffer[index].hw_frame      = av_frame_alloc();

                 frameBuffer[index].target_frame  = NULL;

                 if((frameBuffer[index].frame == NULL) || (frameBuffer[index].hw_frame == NULL))
                 {
                     ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame.");
                 }

                 allocatedFrame++;

                 retFrame = &frameBuffer[index];

                 break;
             }
         }
    }
    else
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_OK ,"Frame Buffer Over Index.");
    }

    pthread_mutex_unlock(&mutex_fh);

    return retFrame;
}

ERROR_T Module_FrameHandler_BufferFree(frame_data_t* frameData)
{
    if(frameData == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Invalied frame.");
    }

    pthread_mutex_lock(&mutex_fh);

    if(allocatedFrame > 0)
    {

        if(frameData->isOccupied)
        {
            frameData->isOccupied    = false;

            av_frame_free(&frameData->hw_frame);
            av_frame_free(&frameData->frame);

            frameData->frame         = NULL;
            frameData->hw_frame      = NULL;
            frameData->target_frame  = NULL;

            allocatedFrame--;
        }
    }
    else
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_OK ,"Frame Buffer Under Index.");
    }

    pthread_mutex_unlock(&mutex_fh);

    return ERROR_OK;
}

ERROR_T MODULE_FrameHandler_SendMessage(void* msg, FH_MSG_T message_type)
{
    return thread_queue_add(&queue_fh, msg, (long)message_type);
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Thread
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static void thread_FrameHandler_Cleanup(void *arg)
{
    ERROR_SystemLog("Brik Start Frame Handler Thread Clenaup. \n\n");

    frame_data_msg_t* frame_msg = arg;

    if(frame_msg != NULL)
    {
        free(frame_msg);
        frame_msg = NULL;
    }
}
static void* thread_FrameHandler(void *arg)
{
    ERROR_T ret_queue = ERROR_OK;

    struct threadmsg message;

    frame_data_msg_t* frame_msg = NULL;

    // Thread Cleanup
    pthread_cleanup_push(thread_FrameHandler_Cleanup, frame_msg);

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
                sFrameBuffer_Display(frame_msg->frameData);
                Module_FrameHandler_BufferFree(frame_msg->frameData);
                break;

            case FH_MSG_TYPE_VIDEO_STOP:
                sFrameBuffer_Cleanup();

                prevFrame_Width  = UNALLOCATED_RESOLUTION;
                prevFrame_Height = UNALLOCATED_RESOLUTION;

                MODULE_Image_UpdateImage(INTRO_IMAGE);
                break;

            default:
                sFrameBuffer_Cleanup();
                ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Invalid video pakcet.");
                break;
        }

        free(frame_msg);
    }

    pthread_cleanup_pop(0);

    return ERROR_OK;
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static ERROR_T sFrameBuffer_Cleanup(void)
{
    ERROR_T ret = ERROR_OK;
    int   index = 0;

    for(index = 0; index < FRAME_BUFFER_MAX; index++)
     {
         if(frameBuffer[index].isOccupied)
         {
             Module_FrameHandler_BufferFree(&frameBuffer[index]);
         }
     }

    return ret;
}

static ERROR_T sFrameBuffer_Display(frame_data_t* frameData)
{
    ERROR_T ret = ERROR_OK;

    static uint32_t            fpd = 0;
    static uint32_t            fps = 0;

    static bool           skipFlag = 0;
    static uint32_t       skipCnt  = 0;
    static uint32_t       skipMax  = 0;

    AVFrame* frame = frameData->target_frame;

    if((frame->width == 0) || (frame->height == 0))
    {
        return ERROR_NOT_OK;
    }

    if((frame->width != prevFrame_Width) || (frame->height != prevFrame_Height))
    {

        MODULE_Display_Init_Overlay(frame->width, frame->height, frame->format, 0);

        prevFrame_Width  = frame->width;
        prevFrame_Height = frame->height;

        skipCnt    = 0;
        skipFlag   = false;
        skipMax    = 0;
    }

    // fps control.
    if(fpd > TARGET_FPD) // if delayed FPD.
    {

        // max_skip frame is TARGET_SKIP_FRAME_MAX.
        skipMax = ((fpd / TARGET_SKIP_FRAME_MAX) <= TARGET_SKIP_FRAME_MAX) ? (fpd / 1): TARGET_SKIP_FRAME_MAX;


        if(!skipFlag)
        {
            skipFlag = true;

            MODULE_Display_Update(frame);
        }
        else
        {
            skipCnt++;

            if(skipCnt >= skipMax)
            {
                skipCnt  = 0;
                skipFlag = false;
            }
        }
    }
    else // if not delayed FPD
    {
        // always frame updated.
        MODULE_Display_Update(frame);
    }

    fps = MODULE_Display_FPS();
    fpd = MODULE_VideoHandler_FPD();

#if false // MODULE BACK TRACING.
    // Data Log
    ERROR_SystemLog("\n\n- - - - DECODER RECEIVE :: FRAME(DECODED) - - - -\n");
    printf("Pixel Format: [ %3d ] Key Frame: [ %d ] Resolution [ %dx%d ] preview Rsolution [ %dx%d ]\n", \
            frame->format, frame->key_frame, frame->width, frame->height, prevFrame_Width, prevFrame_Height);

    printf("\n[%3d]FPS\n", fps);
    printf("\n[%3d]FPD\n", fpd);
    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");
#endif

    return ret;
}

