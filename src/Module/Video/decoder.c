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

/* ******* GLOBAL VARIABLE ******* */
/* *** FFMPEG *** */
static AVCodecContext* video_codec_context    = NULL;
static enum AVPixelFormat hw_pix_fmt          = AV_PIX_FMT_DRM_PRIME;
AVBufferRef *hw_device_ctx = NULL;

/* ******* STATIC FUNCTIONS ******* */
static ERROR_T sVideoContext_Init(int width, int height, int framerate, void* extradata, int extra_len);


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
    if (ret < ERROR_OK)
    {
        printf("Error setting data to a packet %d\n", ret);
        ERROR_StatusCheck(BRIK_STATUS_NOT_OK, "Error setting data to a packet");
    }

    av_packet->pts = packet->avpacket.timestamp;

    ret = avcodec_send_packet(video_codec_context, av_packet);
    if (ret < ERROR_OK)
    {
        //printf("INVAL %d, AGAIN %d, NOMEM %d, EOF %d\n",  AVERROR(EINVAL), AVERROR(EAGAIN), AVERROR(ENOMEM), AVERROR_EOF);
        printf("Error Sending a packet for decoding %d\n", ret);

        //ERROR_StatusCheck(BRIK_STATUS_NOT_OK, "failed sending packet!!");
    }

#if false // MODULE BACK TRACING.
    ERROR_SystemLog("\n\n- - - - - - DECODER SEND :: FRAME(LAW) - - - - - \n");

    printf("set packet size = %d\n", av_packet->size);
    printf("set packet data = %p\n", av_packet->data);

    ERROR_SystemLog("\n\n- - - - - - - - - - - - - - - - - - - - - - - - - \n");
#endif

    return ret;
}


ERROR_T MODULE_Decoder_Receive(frame_data_t* frameData)
{
    ERROR_T ret    = ERROR_OK;
    ERROR_T retMsg = ERROR_OK;
    frame_data_msg_t * message;

    if((frameData->frame == NULL) || (frameData->hw_frame == NULL) || (video_codec_context == NULL))
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Not Initialized params.");
    }

    while(true)
    {
        ret = avcodec_receive_frame(video_codec_context, frameData->frame);

        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            Module_FrameHandler_BufferFree(frameData);

            return ERROR_OK;
        }
        else if (ret < 0)
        {
            Module_FrameHandler_BufferFree(frameData);

            return ERROR_OK; //DEBUG
        }

        if(frameData->frame->format == hw_pix_fmt)
        {
            /* retrieve data from GPU to CPU */
            if ((ret = av_hwframe_transfer_data(frameData->hw_frame, frameData->frame, 0)) < 0)
            {
                Module_FrameHandler_BufferFree(frameData);
                ERROR_StatusCheck(BRIK_STATUS_DECODE_ERROR, "Error transferring the data to system memory");
            }

            frameData->target_frame = frameData->hw_frame;
        }
        else
        {
            frameData->target_frame = frameData->frame;
        }


        message = malloc(sizeof(frame_data_msg_t));
        if (message == NULL)
        {
            ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Failed to allocate frame handler message: FH_MSG_TYPE_VIDEO_UPDATE.");
        }

        message->frameData = frameData;

        message->packet     = NULL;
        message->payload    = NULL;

        retMsg = MODULE_FrameHandler_SendMessage((void*)message, FH_MSG_TYPE_VIDEO_UPDATE);
        if(retMsg != ERROR_OK)
        {
            printf("Error while sending video stop message to frame handler: %d\n", retMsg);
        }

        return retMsg;
    }

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



