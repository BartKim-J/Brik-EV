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

/* ******* FLAGS ******* */
#define __NEW_FFMPEG__
#define __RK_HW_DECODER__
#define OPEN_OVERLAY_ON_KEY_FRAME

/* ******* STATIC DEFINE ******* */
/* *** FRAME *** */
#define TARGET_MOBILE_FPS               30 // frame per second
#define TARGET_DESKTOP_FPS              15

#define TARGET_FPD                      10

#define TARGET_SKIP_FRAME_MAX            5
#define TARGET_SKIP_FRAME_MAX_MOBILE     2

/* *** IMAGES *** */
#define UNALLOCATED_RESOLUTION           0
#define UNALLOCATED_FORMAT               0

#define IMAGE_BRIK_INTRO_PATH            "/brik/brik_ev_c/lib/images/Brik_intro.png"

/* ******* CODEC ID ******* */
#define AV_CODEC_ID_H264_RKMPP           "h264_rkmpp"
#define AV_CODEC_ID_H264_BRIK            "h264"

typedef struct threadqueue THQ;

/* ******* GLOBAL VARIABLE ******* */
/* *** SYSTEM *** */
static pthread_t thread_vh;
static THQ       queue_vh;

/* *** FFMPEG *** */
static AVCodecContext* video_codec_context    = NULL;

static enum AVPixelFormat hw_pix_fmt          = AV_PIX_FMT_DRM_PRIME;

AVBufferRef *hw_device_ctx = NULL;

/* *** IMAGES *** */
static uint32_t prevFrame_Width  = UNALLOCATED_RESOLUTION;
static uint32_t prevFrame_Height = UNALLOCATED_RESOLUTION;
AVFrame* introImage = NULL;

/* ******* STATIC FUNCTIONS ******* */
/* *** FFMPEG DECODER & CONTEXT INIT *** */
static ERROR_T sDeCoder_Init(CodecDataPacket* data, void* extradata, int extra_len);
static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len);

/* *** FFMPEG DECODE FRAME( WITH OVERLAY UPDATE ) *** */
static ERROR_T sDecoder_sendPacket(AVPacketPacket* packet, void* payload);
static ERROR_T sDecoder_receiveFrame(AVFrame *frame, AVFrame *hw_frame);
static ERROR_T sDecoder_updateOverlay(AVFrame *frame);

/* *** FFMPEG IMAGE *** */
static ERROR_T sDisplay_LoadImages(void);

static ERROR_T sDisplay_IntroImage(void);

static ERROR_T sDisplay_cleanImageCache(void);

static AVFrame* sOpenImage(const char *filename);
static ERROR_T  sSaveFrame(AVFrame *pFrame, int width, int height, int iFrame);

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

    // avcodec registration for video handler
#ifndef __NEW_FFMPEG__
    //avcodec_register_all();
#endif

    // init data queue for the thread
    ret = thread_queue_init(&queue_vh);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video queue.");
    }

    ret = pthread_create(&thread_vh, NULL, thread_VideoHandler, NULL);

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
    ERROR_T ret_queue = 0;

    struct threadmsg message;

    video_data_msg_t* video_msg;
    SDL_Event event;

    sDisplay_LoadImages();
    sDisplay_IntroImage();

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
        if (message.msgtype == VH_MSG_TYPE_VIDEO_CODEC)
        {
            handle_video_codec(video_msg->packet, video_msg->payload);
        }
        else if (message.msgtype == VH_MSG_TYPE_VIDEO_DATA)
        {
            handle_video_data(video_msg->packet, video_msg->payload);
        }
        else if (message.msgtype == VH_MSG_TYPE_VIDEO_STOP)
        {
            handle_video_stop();
        }
        else
        {
            ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Invalid video pakcet.");
        }

        free(video_msg);

        while( SDL_PollEvent( &event ) )
        {
            /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
            switch( event.type )
            {
                case SDL_QUIT:
                    sDisplay_cleanImageCache();
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
 *  Decoder Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static ERROR_T sDeCoder_Init(CodecDataPacket* packet, void* extradata, int extra_len)
{
    if (video_codec_context != NULL)
    {
        avcodec_free_context(&video_codec_context);
        video_codec_context = NULL;
    }

    return sVideoContext_Init(packet->video.width, packet->video.height, packet->video.framerate,
                                extradata,  extra_len);
}

static ERROR_T sDecoder_sendPacket(AVPacketPacket* packet, void* payload)
{
    ERROR_T ret = ERROR_OK;

    AVPacket* av_packet = NULL;

    /* av packet allocate && init */
    av_packet = av_packet_alloc();
    if(av_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate AV Packet.");
    }

    ret = av_packet_from_data(av_packet, payload, packet->hdr.payloadSize);
    {
        if (ret < 0)
        {
            printf("Error setting data to a packet %d\n", ret);

            return ret;
        }
    }

    av_packet->pts = packet->avpacket.timestamp;

    ret = avcodec_send_packet(video_codec_context, av_packet);
    if (ret < 0)
    {
        printf("INVAL %d, AGAIN %d, NOMEM %d, EOF %d\n",  AVERROR(EINVAL), AVERROR(EAGAIN), AVERROR(ENOMEM), AVERROR_EOF);
        printf("Error Sending a packet for decoding %d\n", ret);
    }
    
    ERROR_SystemLog("\n\n- - - - - - DECODER SEND :: FRAME(LAW) - - - - - \n");

    printf("set packet size = %d\n", av_packet->size);
    printf("set packet data = %p\n", av_packet->data);

    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");

    return ret;
}


static ERROR_T sDecoder_receiveFrame(AVFrame *frame, AVFrame *hw_frame)
{
    ERROR_T ret = ERROR_OK;

    AVFrame*        tmp_frame = NULL;  // Just for pointing frame.

    if(frame == NULL || hw_frame == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Not Initialized params.");
    }

    while(ret >= 0)
    {
        ret = avcodec_receive_frame(video_codec_context, frame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // Frame not yet.
            return ret;
        }
        else if (ret < 0)
        {
            ERROR_StatusCheck(BRIK_STATUS_DECODE_ERROR, "Error during decoding");
        }

        if (frame->format == hw_pix_fmt)
        {
            /* retrieve data from GPU to CPU */
            if ((ret = av_hwframe_transfer_data(hw_frame, frame, 0)) < 0)
            {
                fprintf(stderr, "Error transferring the data to system memory\n");

                return ret;
            }

            tmp_frame = hw_frame;
        }
        else
        {
            tmp_frame = frame;
        }

        // if get frame updateOveray
        sDecoder_updateOverlay(tmp_frame);
    }

    return ret;
}

static ERROR_T sDecoder_updateOverlay(AVFrame *frame)
{
    ERROR_T ret = ERROR_OK;

    static uint32_t            fpd = 0;
    static uint32_t            fps = 0;

    static bool           skipFlag = 0;
    static uint32_t       skipCnt  = 0;
    static uint32_t       skipMax  = 0;

#ifdef OPEN_OVERLAY_ON_KEY_FRAME
    if (frame->key_frame)
    {
        if((prevFrame_Width != frame->width) || (prevFrame_Height != frame->height))
        {
#ifdef __RK_HW_DECODER__
            MODULE_Display_Init_Overlay(frame->width, frame->height, frame->format, 0);
#else
            MODULE_Display_Init_Overlay(frame->width, frame->height, AV_PIX_FMT_YUV420P, 0);
#endif
            prevFrame_Width  = frame->width;
            prevFrame_Height = frame->height;

            skipCnt    = 0;
            skipFlag   = false;
            skipMax    = 0;
        }
    }
#endif

    // fps control.
    if(fpd > TARGET_FPD) // if delayed FPD.
    {
        if(frame->width < frame->height) // and when resolution is Mobile size.
        {
            skipMax = TARGET_SKIP_FRAME_MAX_MOBILE;
        }
        else // and when resolution is Desktop size
        {
            // max_skip frame is TARGET_SKIP_FRAME_MAX.
            skipMax = ((fpd / 5) <= TARGET_SKIP_FRAME_MAX) ? (fpd / 5): TARGET_SKIP_FRAME_MAX;
        }

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
    fpd = thread_queue_length(&queue_vh);

    // Data Log
    ERROR_SystemLog("\n\n- - - - DECODER RECEIVE :: FRAME(DECODED) - - - -\n");
    printf("Pixel Format: [ %3d ] Key Frame: [ %d ] Resolution [ %dx%d ] preview Rsolution [ %dx%d ]\n", \
            frame->format, frame->key_frame, frame->width, frame->height, prevFrame_Width, prevFrame_Height);

    printf("\n[%7d] frame(s) decoded\n", video_codec_context->frame_number);
    printf("\n[%3d]FPS\n", fps);
    printf("\n[%3d]FPD\n", fpd);
    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");

    return ret;
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

    if (sDeCoder_Init(packet, payload, packet->hdr.payloadSize) < 0)
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

    if (video_codec_context == NULL)
    {
        ERROR_SystemLog("Video codec is not initialized.");

        return ERROR_NOT_OK;
    }

    /* av frame value allocate & init */
    frame    = av_frame_alloc();
    hw_frame = av_frame_alloc();

    if(frame == NULL || hw_frame == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame.");
    }

    ret = sDecoder_sendPacket(packet, payload);

    ret = sDecoder_receiveFrame(frame, hw_frame);

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

    // Resolution Info Uninit
    prevFrame_Width  = UNALLOCATED_RESOLUTION;
    prevFrame_Height = UNALLOCATED_RESOLUTION;

    sDisplay_IntroImage();

    return status;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static ERROR_T sDisplay_LoadImages(void)
{
    ERROR_T ret = ERROR_OK;

    if(introImage == NULL)
    {
        introImage = sOpenImage(IMAGE_BRIK_INTRO_PATH);
        if(introImage == NULL)
        {
            ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Couldn't open intro image file.");
        }

        sSaveFrame(introImage, introImage->width, introImage->height, 0);
    }

    return ret;
}
static ERROR_T sDisplay_IntroImage(void)
{
    ERROR_T ret = ERROR_OK;

    MODULE_Display_Init_Overlay(introImage->width, introImage->height, introImage->format, 0);

    ret = MODULE_Display_Update(introImage);

    return ret;
}

static ERROR_T sDisplay_cleanImageCache(void)
{
    ERROR_T ret = ERROR_OK;

    // intro Image
    av_free(introImage);

    // add.. other images

    return ret;
}

static AVFrame* sOpenImage(const char *filename)
{
    ERROR_T         videoStream = ERROR_NOT_OK;
    bool            frameFinished = false;

    AVFormatContext *pFormatCtx;
    int             i = 0;

    AVPacket        packet;

    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec    = NULL;
    AVFrame         *pFrame    = NULL;
    AVFrame         *retFrame  = NULL;

    int             numBytes  = 0;
    uint8_t         *buffer   = NULL;

    pFormatCtx = avformat_alloc_context();

    // Open video file
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL)!=0)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Couldn't open file");
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Couldn't find stream information");
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename, false);

    // Find the first video stream
    for(i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    if(videoStream == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Didn't find a video stream");
    }


    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Codec not found");
    }

    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Founded a codec but Could not open.");
    }

    // Allocate video frame
    pFrame = av_frame_alloc();
    if(pFrame == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to alloc frame memory.");
    }

    // Allocate an AVFrame structure
    retFrame = av_frame_alloc();
    if(retFrame == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to alloc frame memory.");
    }

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    buffer   = malloc(numBytes);

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)retFrame, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

    // Read frames
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        // Is this a packet from the video stream?
        if(packet.stream_index == videoStream)
        {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if(frameFinished)
            {
                static struct SwsContext *img_convert_ctx;

                // Convert the image into YUV format that SDL uses
                if(img_convert_ctx == NULL)
                {
                    img_convert_ctx = sws_getContext(
                                            pCodecCtx->width,
                                            pCodecCtx->height,
                                            pCodecCtx->pix_fmt,
                                            pCodecCtx->width,
                                            pCodecCtx->height,
                                            AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC,
                                            NULL, NULL, NULL);

                    if(img_convert_ctx == NULL)
                    {
                        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"annot initialize the conversion context!");
                    }
                }

                sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, retFrame->data, retFrame->linesize);
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    retFrame->width     = pFrame->width;
    retFrame->height    = pFrame->height;
    retFrame->format    = AV_PIX_FMT_YUV420P;

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return retFrame;
}

static ERROR_T sSaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile = NULL;
    int  y = 0;

    char szFilename[32] = {0, };


    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if(pFile == NULL)
    {
        return ERROR_NOT_OK;
    }

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y = 0; y < height; y++)
    {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    }

    // Close file
    fclose(pFile);

    return ERROR_OK;
}

static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len)
{
    ERROR_T ret = ERROR_OK;

    AVCodec        *codec = NULL;
    AVCodecContext *ctx;

    av_log_set_level(AV_LOG_DEBUG);

    // Codec
    // h.264 only for now.. TODO: Identify Codec
#ifdef __RK_HW_DECODER__
    codec = avcodec_find_decoder_by_name(AV_CODEC_ID_H264_RKMPP);
#else
    codec = avcodec_find_decoder_by_name(AV_CODEC_ID_H264_BRIK);
#endif
    if (codec == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to find h264 (rkmpp) FFmpeg decoder.");
        ERROR_StatusCheck(BRIK_STATUS_NOT_SUPPORTED ,"Failed to find h264 (rkmpp) FFmpeg decoder.");
    }

    // Context
    ctx = avcodec_alloc_context3(codec);
    if (ctx == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate codec context.\n");
        return ERROR_NOT_OK;
    }

    // Extra data
    printf("extradata = %p, size = %d\n", extradata, extra_len);
    if (extradata && (extra_len != 0))
    {
        ctx->extradata = (uint8_t *)av_memdup(extradata, extra_len);
        ctx->extradata_size = extra_len;
    }

    // Open
    if (avcodec_open2(ctx, codec, NULL) < 0)
    {
        avcodec_free_context(&ctx);

        av_log(NULL, AV_LOG_ERROR, "Failed to open codec. %s\n", codec->name);

        return ERROR_NOT_OK;
    }

    video_codec_context = ctx;

    avcodec_flush_buffers(ctx);

    return ret;
}

