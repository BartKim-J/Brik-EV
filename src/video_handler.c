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
#define TARGET_MOBILE_FPS               30 // frame per second
#define TARGET_DESKTOP_FPS              15

#define TARGET_FPD                      10

#define TARGET_SKIP_FRAME_MAX            5
#define TARGET_SKIP_FRAME_MAX_MOBILE     2

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
static AVBSFContext* bitstream_filter_context = NULL;

static enum AVPixelFormat hw_pix_fmt          = AV_PIX_FMT_DRM_PRIME;

AVBufferRef *hw_device_ctx = NULL;

/* ******* STATIC FUNCTIONS ******* */
/* *** FFMPEG DECODER & CONTEXT INIT *** */
static ERROR_T sDeCoder_Init(CodecDataPacket* data, void* extradata, int extra_len);
static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len);

/* *** FFMPEG DECODE FRAME( WITH OVERLAY UPDATE ) *** */
static ERROR_T sDecoder_sendPacket(AVPacketPacket* packet, void* payload);
static ERROR_T sDecoder_receiveFrame(AVFrame *frame, AVFrame *hw_frame);
static ERROR_T sDecoder_updateOveray(AVFrame *frame);

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
                SDL_Quit();
                break;

              default:
                break;
            }
        }
    }
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
        sDecoder_updateOveray(tmp_frame);
    }

    return ret;
}

static ERROR_T sDecoder_updateOveray(AVFrame *frame)
{
    ERROR_T ret = ERROR_OK;

    static uint32_t prevWidth  = 0;
    static uint32_t prevHeight = 0;

    static uint32_t            fpd = 0;
    static uint32_t            fps = 0;

    static bool           skipFlag = 0;
    static uint32_t       skipCnt  = 0;
    static uint32_t       skipMax  = 0;

#ifdef OPEN_OVERLAY_ON_KEY_FRAME
    if (frame->key_frame)
    {
        if((prevWidth != frame->width) || (prevHeight != frame->height))
        {
#ifdef __RK_HW_DECODER__
            dh_display_init_video_overlay(frame->width, frame->height, frame->format, 0);
#else
            dh_display_init_video_overlay(frame->width, frame->height, AV_PIX_FMT_YUV420P, 0);
#endif
            prevWidth  = frame->width;
            prevHeight = frame->height;

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

            dh_display_decoded_frame(frame);
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
        dh_display_decoded_frame(frame);
    }

    fps = dh_display_get_FPS();
    fpd = thread_queue_length(&queue_vh);

    // Data Log
    ERROR_SystemLog("\n\n- - - - DECODER RECEIVE :: FRAME(DECODED) - - - -\n");
    printf("Pixel Format: [ %3d ] Key Frame: [ %d ] Resolution [ %dx%d ] preview Rsolution [ %dx%d ]\n", \
            frame->format, frame->key_frame, frame->width, frame->height, prevWidth, prevHeight);

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

    int use_sps_resolution = 0;

    dh_display_clean();

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
        use_sps_resolution = 1;
        printf("Frame dimension from PACKET_VIDEO_CODEC is different to dimensions from extradata\n");
        printf("packet: %d x %d, extradata(sps): %d x %d\n", packet->video.width, packet->video.height, width_sps, height_sps);
        printf("Using resulotion from SPS\n\n\n");
        printf("TODO: FIX Resolution calculation\n\n\n");
    }

    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - \n");

#ifndef OPEN_OVERLAY_ON_KEY_FRAME

    if (use_sps_resolution == 1)
    {
        dh_display_init_video_overlay(width_sps, height_sps, AV_PIX_FMT_YUV420P, 0);
    }
    else
    {
        dh_display_init_video_overlay(packet->video.width, packet->video.height, AV_PIX_FMT_YUV420P, 0);
    }
#endif
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

    dh_display_clean();

    return status;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len)
{
    ERROR_T ret = ERROR_OK;

    AVCodec        *codec = NULL;
    AVCodecContext *ctx;
    AVBSFContext   *annexb;

    const AVBitStreamFilter *bsf;
    enum AVHWDeviceType hw_type;

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

#if 0
    hw_type = av_hwdevice_find_type_by_name("drm"); ///dev/dri/card0
    if (hw_type == AV_HWDEVICE_TYPE_NONE)
    {
        fprintf(stderr, "Device type is not supported.\n");
        fprintf(stderr, "Available device types:");
        while((hw_type = av_hwdevice_iterate_types(hw_type)) != AV_HWDEVICE_TYPE_NONE)
            fprintf(stderr, " %s", av_hwdevice_get_type_name(hw_type));
        fprintf(stderr, "\n");
        return ERROR_NOT_OK;
    }

    ctx->get_format  = get_hw_format;

    if ((ret = av_hwdevice_ctx_create(&hw_device_ctx, hw_type, NULL, NULL, 0)) < 0)
    {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return ret;
    }

    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

#endif

    // Open
    if (avcodec_open2(ctx, codec, NULL) < 0)
    {
        avcodec_free_context(&ctx);

        av_log(NULL, AV_LOG_ERROR, "Failed to open codec. %s\n", codec->name);

        return ERROR_NOT_OK;
    }

#if 0 //def __RK_HW_DECODER__
    bsf = av_bsf_get_by_name("h264_mp4toannexb");

    if (annexb == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to get bsf.");
        return -1;
    }

    if ((ret = av_bsf_alloc(bsf, &annexb)) < 0)
    {
        return ret;
    }

    if ((ret = avcodec_parameters_from_context(annexb->par_in, ctx)) < 0)
    {
        return ret;
    }

    ret = av_bsf_init(annexb);

    if (ret < 0)
    {
        return ret;
    }

    bitstream_filter_context = annexb;

#endif
    video_codec_context = ctx;

    avcodec_flush_buffers(ctx);

    return ret;
}

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++)
    {
        if(*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");

    return AV_PIX_FMT_NONE;
}
