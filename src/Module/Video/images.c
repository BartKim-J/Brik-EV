/**
 * @file images.c
 * @author bato
 * @date 23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
/* ******* INCLUDE ******* */
#include "brik_api.h"
#include "images.h"

/* ******* GLOBAL VARIABLE ******* */
/* *** IMAGES *** */
static AVFrame* imageResource[IMAGE_COUNT];

/* ******* STATIC DEFINE ******* */
#define IMAGE_BRIK_INTRO_PATH            "/brik/brik_ev_c/lib/images/Brik_intro.png"
#define IMAGE_BRIK_ERROR_PATH            "/brik/brik_ev_c/lib/images/Brik_error.png"

/* ******* STATIC FUNCTIONS ******* */
static AVFrame* sOpenImage(const char *filename);
static ERROR_T  sSaveImage(AVFrame *pFrame, int width, int height, int iFrame);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_Image_UpdateImage(imageResoure_t imgIndex)
{
    ERROR_T ret = ERROR_OK;

    if(imageResource[imgIndex] == NULL)
    {
        MODULE_Image_CleanImages();
        MODULE_Image_LoadImages();
    }

    MODULE_Display_Clean();

    switch(imgIndex)
    {
        case INTRO_IMAGE:
                MODULE_Display_Init_Overlay(imageResource[INTRO_IMAGE]->width, imageResource[INTRO_IMAGE]->height, imageResource[INTRO_IMAGE]->format, 0);

                ret = MODULE_Display_Update(imageResource[INTRO_IMAGE]);
            break;
        case ERROR_IMAGE:
                MODULE_Display_Init_Overlay(imageResource[ERROR_IMAGE]->width, imageResource[ERROR_IMAGE]->height, imageResource[INTRO_IMAGE]->format, 0);

                ret = MODULE_Display_Update(imageResource[ERROR_IMAGE]);
            break;
        default:

            break;
    }


    return ret;
}



ERROR_T MODULE_Image_LoadImages(void)
{
    ERROR_T ret = ERROR_OK;
    imageResoure_t imgIndex = 0;

    for(imgIndex = 0; imgIndex < IMAGE_COUNT; imgIndex++)
    {
        switch(imgIndex)
        {
            case INTRO_IMAGE:
                    // Intro Image
                    if(imageResource[INTRO_IMAGE] == NULL)
                    {
                        imageResource[INTRO_IMAGE] = sOpenImage(IMAGE_BRIK_INTRO_PATH);
                        if(imageResource[INTRO_IMAGE] == NULL)
                        {
                            ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Couldn't open intro image file.");
                        }

                        //sSaveImage(imageResource[INTRO_IMAGE], imageResource[INTRO_IMAGE]->width, imageResource[INTRO_IMAGE]->height, 0);
                    }
                break;
            case ERROR_IMAGE:
                    // Intro Image
                    if(imageResource[ERROR_IMAGE] == NULL)
                    {
                        imageResource[ERROR_IMAGE] = sOpenImage(IMAGE_BRIK_ERROR_PATH);
                        if(imageResource[ERROR_IMAGE] == NULL)
                        {
                            ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Couldn't open intro image file.");
                        }

                        //sSaveImage(imageResource[INTRO_IMAGE], imageResource[INTRO_IMAGE]->width, imageResource[INTRO_IMAGE]->height, 0);
                    }
                break;
            default:
                break;
        }
    }

    return ret;
}

ERROR_T MODULE_Image_CleanImages(void)
{
    ERROR_T ret = ERROR_OK;
    imageResoure_t imgIndex = 0;

    for(imgIndex = 0; imgIndex < IMAGE_COUNT; imgIndex++)
    {
        if(imageResource[imgIndex] == NULL)
        {
            av_free(imageResource[imgIndex]);
            imageResource[imgIndex] = NULL;
        }
    }

    return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static AVFrame* sOpenImage(const char *filename)
{
    ERROR_T         videoStream = ERROR_NOT_OK;
    bool            frameFinished = false;

    AVFormatContext *pFormatCtx;       // open file info

    AVPacket        packet;            // encoded packet from open file

    AVCodecContext  *pCodecCtx = NULL; // decoder info
    AVCodec         *pCodec    = NULL; // codec

    AVFrame         *pFrame    = NULL; // open file frame
    AVFrame         *retFrame  = NULL; // return frame

    int             size       = 0;    // frame buffer size
    uint8_t         *frameData = NULL; // frame buffer

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

    /* select the video stream */
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (videoStream < ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Cannot find a video stream in the input file");
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
    pFrame   = av_frame_alloc();
    retFrame = av_frame_alloc();
    if((pFrame == NULL) || (retFrame == NULL))
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to alloc frame memory.");
    }

    // Setup new frame info
    retFrame->format = AV_PIX_FMT_YUV420P;
    retFrame->width  = pCodecCtx->width;
    retFrame->height = pCodecCtx->height;

    // Determine required buffer size and allocate buffer
    size        = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    frameData   = (uint8_t*)malloc(size);

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(retFrame->data, \
                         retFrame->linesize, \
                         frameData,
                         retFrame->format, \
                         retFrame->width, \
                         retFrame->height,\
                         1);

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

                sws_scale(img_convert_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, \
                                    retFrame->height, retFrame->data, retFrame->linesize);
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return retFrame;
}

static ERROR_T sSaveImage(AVFrame *pFrame, int width, int height, int iFrame)
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

