/**
 * @file decoder.c
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
#include "decoder.h"

/* ******* STATIC DEFINE ******* */
/* ******* FLAGS ******* */
#define __NEW_FFMPEG__
#define __RK_HW_DECODER__
#define OPEN_OVERLAY_ON_KEY_FRAME

/* ******* CODEC ID ******* */
#define AV_CODEC_ID_H264_RKMPP           "h264_rkmpp"
#define AV_CODEC_ID_H264_BRIK            "h264"

/* *** IMAGES *** */
#define UNALLOCATED_RESOLUTION           0
#define UNALLOCATED_FORMAT               0

/* *** FRAME *** */
#define TARGET_MOBILE_FPS               30 // frame per second
#define TARGET_DESKTOP_FPS              15

#define TARGET_FPD                       5

#define TARGET_SKIP_FRAME_MAX            3

/* ******* GLOBAL VARIABLE ******* */
/* *** FFMPEG *** */
static AVCodecContext* video_codec_context    = NULL;
static enum AVPixelFormat hw_pix_fmt          = AV_PIX_FMT_DRM_PRIME;
AVBufferRef *hw_device_ctx = NULL;

/* *** FRAME *** */
static int prevFrame_Width  = UNALLOCATED_RESOLUTION;
static int prevFrame_Height = UNALLOCATED_RESOLUTION;

/* ******* STATIC FUNCTIONS ******* */
static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len);
static ERROR_T sFrameUpdate(AVFrame *frame, int frameQueue);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_Decoder_Init(CodecDataPacket* packet, void* extradata, int extra_len)
{
    MODULE_Decoder_Uninit();

    return sVideoContext_Init(packet->video.width, packet->video.height, packet->video.framerate,
                                extradata,  extra_len);
}
ERROR_T MODULE_Decoder_Uninit(void)
{
    if (video_codec_context != NULL)
    {
        avcodec_free_context(&video_codec_context);
        video_codec_context = NULL;
    }

    prevFrame_Width  = UNALLOCATED_RESOLUTION;
    prevFrame_Height = UNALLOCATED_RESOLUTION;

    return ERROR_OK;
}

ERROR_T MODULE_Decoder_Write(AVPacketPacket* packet, void* payload)
{
    ERROR_T ret = ERROR_OK;

    AVPacket* av_packet = NULL;


    if((packet == NULL) || (payload == NULL) || (video_codec_context == NULL))
    {
        ERROR_StatusCheck(BRIK_STATUS_INVALID_PARAM ,"Not Initialized params..");
    }

    /* av packet allocate && init */
    av_packet = av_packet_alloc();
    if(av_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate AV Packet.");
    }

    ret = av_packet_from_data(av_packet, payload, packet->hdr.payloadSize);
    {
        if (ret < ERROR_OK)
        {
            printf("Error setting data to a packet %d\n", ret);

            return ret;
        }
    }

    av_packet->pts = packet->avpacket.timestamp;

    ret = avcodec_send_packet(video_codec_context, av_packet);
    if (ret < ERROR_OK)
    {
        printf("INVAL %d, AGAIN %d, NOMEM %d, EOF %d\n",  AVERROR(EINVAL), AVERROR(EAGAIN), AVERROR(ENOMEM), AVERROR_EOF);
        printf("Error Sending a packet for decoding %d\n", ret);

        ERROR_StatusCheck(BRIK_STATUS_NOT_OK, "failed sending packet!!");
    }

    ERROR_SystemLog("\n\n- - - - - - DECODER SEND :: FRAME(LAW) - - - - - \n");

    printf("set packet size = %d\n", av_packet->size);
    printf("set packet data = %p\n", av_packet->data);

    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");

    return ret;
}


ERROR_T MODULE_Decoder_Receive(AVFrame *frame, AVFrame *hw_frame, int frameQueue)
{
    ERROR_T ret = ERROR_OK;

    AVFrame*        tmp_frame = NULL;  // Just for pointing frame.

    if((frame == NULL) || (hw_frame == NULL) || (video_codec_context == NULL))
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
            // during dcoding Skipping.
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
        sFrameUpdate(tmp_frame, frameQueue);
    }

    return ret;
}

static ERROR_T sFrameUpdate(AVFrame *frame, int frameQueue)
{
    ERROR_T ret = ERROR_OK;

    static uint32_t            fpd = 0;
    static uint32_t            fps = 0;

    static bool           skipFlag = 0;
    static uint32_t       skipCnt  = 0;
    static uint32_t       skipMax  = 0;


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
    fpd = frameQueue;

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

    av_opt_set();

    avcodec_flush_buffers(ctx);

    return ret;
}



