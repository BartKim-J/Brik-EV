/**
 * @file display_sdl.c
 * @author Ben
 * @date  7 Mar 2019
 * @brief
 */
/* ******* INCLUDE ******* */
#ifdef DISPLAY_TYPE_SDL
#include "brik_api.h"
#include "display_sdl.h"

/* ******* STATIC DEFINE ******* */
// How many frames time values to keep
// The higher the value the smoother the result is...
// Don't make it 0 or less :)
#define FRAME_VALUES 30

/* ******* GLOBAL VARIABLE ******* */
static pthread_mutex_t mutex_sdl = PTHREAD_MUTEX_INITIALIZER;

static uint32_t frametimes[FRAME_VALUES]; // An array to store frame times:
static uint32_t frametimelast;            // Last calculated SDL_GetTicks
static uint32_t framecount;               // total frames rendered

static float    framespersecond;

static moduleImageViewer_t moduleImageViewer = {NULL, NULL, NULL, {0,0,0,0}, 0, 0, 0, 0};

/* ******* STATIC FUNCTIONS ******* */
/* *** Image *** */

/* *** Screen *** */
static ERROR_T sScreenClean(moduleImageViewer_t* module);
static ERROR_T sScreenUpdate(moduleImageViewer_t* module, AVFrame* av_frame);

/* *** FPS *** */
static void sFPS_Init(void);
static void sFPS_Update(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_Display_Init(void)
{
    ERROR_T ret = ERROR_OK;

    const SDL_VideoInfo* info_display;
    printf("MODULE INIT => Display handler.\n");

    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        printf("SDL Video already initialized\n");
        return ERROR_OK;
    }

    /* SDL INIT */
    ret = SDL_Init(SDL_INIT_VIDEO);
    if(ret != ERROR_OK)
    {
        printf("Unable to initialize SDL: %s", SDL_GetError());
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Not Initialized params.");
    }

    ret = pthread_mutex_init(&mutex_sdl, NULL);
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize mutex.");
    }

    info_display = SDL_GetVideoInfo();

    moduleImageViewer.screen_w = info_display->current_w;
    moduleImageViewer.screen_h = info_display->current_h;

    printf("screen resolution %d::%d\n", info_display->current_w, info_display->current_h);

    /* Set Video Mode */
    moduleImageViewer.screen = SDL_SetVideoMode(info_display->current_w, info_display->current_h,
                                32, SDL_FULLSCREEN | SDL_HWSURFACE | SDL_HWACCEL /*| SDL_DOUBLEBUF */);
    if(moduleImageViewer.screen == NULL)
    {
        printf("Failed to allocate Video screen surface\n");
    }

    SDL_ShowCursor(SDL_FALSE);

    /* Clean Before Window Screen */
    ret = sScreenClean(&moduleImageViewer);

    sFPS_Init();

    return ret;
}

ERROR_T MODULE_Display_Init_Overlay(int width, int height, uint32_t format, uint32_t dstformat)
{
    ERROR_T ret = ERROR_OK;

    if((width == 0) || (height == 0))
    {
        return ERROR_NOT_OK;
    }

    MODULE_Display_Clean();

    pthread_mutex_lock(&mutex_sdl);

    printf("%s, Screen Overlay Init, %d x %d, format %d\n", __FUNCTION__, width, height, format);

    if(moduleImageViewer.screen == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"video screen is not initialized.");
    }

    if(moduleImageViewer.overlay != NULL)
    {
        SDL_FreeYUVOverlay(moduleImageViewer.overlay);
        moduleImageViewer.overlay = NULL;
    }

    if(moduleImageViewer.sws_ctx != NULL)
    {
        sws_freeContext(moduleImageViewer.sws_ctx);
        moduleImageViewer.sws_ctx = NULL;
    }

    moduleImageViewer.overlay = SDL_CreateYUVOverlay(width, height,
                                                    SDL_YV12_OVERLAY, moduleImageViewer.screen);
    if(moduleImageViewer.overlay == NULL)
    {
        printf("Failed to allocate video overlay: %s\n", SDL_GetError());
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video overlay.");
    }

    //init scaler
    moduleImageViewer.sws_ctx = sws_getContext(width,
                                               height,
                                               format,
                                               width,
                                               height,
                                               AV_PIX_FMT_YUV420P,
                                               //SWS_BILINEAR,
                                               SWS_FAST_BILINEAR,
                                               NULL,
                                               NULL,
                                               NULL
                                               );

    if(moduleImageViewer.sws_ctx == NULL)
    {
        printf("Failed to allocate Video Scaler: %s\n", SDL_GetError());
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate Video Scaler.");
    }

    // calculate overlay ratio, pos & size
    if (moduleImageViewer.screen_w * height / width > height)     // vertical fit
    {
        moduleImageViewer.rect_overlay_dst.y = 0;
        moduleImageViewer.rect_overlay_dst.h = moduleImageViewer.screen_h;
        moduleImageViewer.rect_overlay_dst.w = moduleImageViewer.rect_overlay_dst.h * width / height;
        moduleImageViewer.rect_overlay_dst.x = (moduleImageViewer.screen_w - moduleImageViewer.rect_overlay_dst.w) / 2;
    }
    else //horizontal fit
    {
        moduleImageViewer.rect_overlay_dst.x = 0;
        moduleImageViewer.rect_overlay_dst.w = moduleImageViewer.screen_w;
        moduleImageViewer.rect_overlay_dst.h = moduleImageViewer.rect_overlay_dst.w * height / width;
        moduleImageViewer.rect_overlay_dst.y = (moduleImageViewer.screen_h - moduleImageViewer.rect_overlay_dst.h) / 2;
    }

    printf("overlay dimension calculated: %d x %d\n\n", moduleImageViewer.rect_overlay_dst.w, moduleImageViewer.rect_overlay_dst.h);

    pthread_mutex_unlock(&mutex_sdl);

    return ret;
}

ERROR_T MODULE_Display_Clean(void)
{
    return sScreenClean(&moduleImageViewer);
}

ERROR_T MODULE_Display_Update(AVFrame* av_frame)
{
    return sScreenUpdate(&moduleImageViewer, av_frame);
}


ERROR_T MODULE_Display_Destroy(void)
{
    SDL_FreeYUVOverlay(moduleImageViewer.overlay);
    moduleImageViewer.overlay = NULL;


    SDL_FreeSurface(moduleImageViewer.screen);
    moduleImageViewer.screen = NULL;

    SDL_Quit();

    return ERROR_OK;
}

float MODULE_Display_FPS(void)
{
    return framespersecond;
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Screen
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static ERROR_T sScreenUpdate(moduleImageViewer_t* module, AVFrame* av_frame)
{
    ERROR_T ret = ERROR_OK;

    AVFrame frameTarget;
    SDL_Overlay* overlaybuffer = NULL;
    SDL_Overlay* bitmap        = NULL;
    overlaybuffer = SDL_CreateYUVOverlay(moduleImageViewer.overlay->w, moduleImageViewer.overlay->h,
                                                      SDL_YV12_OVERLAY, moduleImageViewer.screen);
    if(overlaybuffer == NULL)
    {
        printf("Failed to allocate video overlay: %s\n", SDL_GetError());
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video overlay.");
    }

    bitmap = overlaybuffer;

    SDL_LockYUVOverlay(bitmap);

#if true
    frameTarget.data[0] = bitmap->pixels[0];
    frameTarget.data[1] = bitmap->pixels[2];
    frameTarget.data[2] = bitmap->pixels[1];

    frameTarget.linesize[0] = bitmap->pitches[0];
    frameTarget.linesize[1] = bitmap->pitches[2];
    frameTarget.linesize[2] = bitmap->pitches[1];

    sws_scale(module->sws_ctx, (uint8_t const * const *)av_frame->data,
              av_frame->linesize, 0, av_frame->height,
              frameTarget.data, frameTarget.linesize);

#else
    bitmap->pixels[0] = av_frame->data[0];
    bitmap->pixels[1] = av_frame->data[2];
    bitmap->pixels[2] = av_frame->data[1];

    bitmap->pitches[0] = av_frame->linesize[0];
    bitmap->pitches[1] = av_frame->linesize[2];
    bitmap->pitches[2] = av_frame->linesize[1];

#endif

    SDL_UnlockYUVOverlay(bitmap);

    SDL_DisplayYUVOverlay(bitmap, &(module->rect_overlay_dst));

    if(overlaybuffer != NULL)
     {
         SDL_FreeYUVOverlay(overlaybuffer);
         overlaybuffer = NULL;
     }

    sFPS_Update();


    return ret;
}

static ERROR_T sScreenClean(moduleImageViewer_t* module)
{
    ERROR_T ret = ERROR_OK;

    SDL_Rect rect_fill = {0, 0, 0, 0};

    pthread_mutex_lock(&mutex_sdl);

    rect_fill.x = 0;
    rect_fill.y = 0;
    rect_fill.w = module->screen_w;
    rect_fill.h = module->screen_h;

    uint32_t white = SDL_MapRGB(module->screen->format, 35, 39, 42);
    /* Clear */
    //SDL_FillRect(module->screen, 0, 0);
    //SDL_Flip(module->screen);
    SDL_FillRect(module->screen, &rect_fill, white);

    pthread_mutex_unlock(&mutex_sdl);

    return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  FPS
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static void sFPS_Init(void)
{
    // Set all frame times to 0ms.
    memset(frametimes, 0, sizeof(frametimes));
    framecount = 0;
    framespersecond = 0;
    frametimelast = SDL_GetTicks();
}

static void sFPS_Update(void)
{
    uint32_t frametimesindex = 0;
    uint32_t getticks = 0;
    uint32_t count = 0;
    uint32_t i = 0;

    // frametimesindex is the position in the array. It ranges from 0 to FRAME_VALUES.
    // This value rotates back to 0 after it hits FRAME_VALUES.
    frametimesindex = framecount % FRAME_VALUES;

    // store the current time
    getticks = SDL_GetTicks();

    // save the frame time value
    frametimes[frametimesindex] = getticks - frametimelast;

    // save the last frame time for the next fpsthink
    frametimelast = getticks;

    // increment the frame count
    framecount++;

    // Work out the current framerate

    // The code below could be moved into another function if you don't need the value every frame.

    // I've included a test to see if the whole array has been written to or not. This will stop
    // strange values on the first few (FRAME_VALUES) frames.
    if (framecount < FRAME_VALUES)
    {
        count = framecount;
    }
    else
    {
        count = FRAME_VALUES;
    }

    // add up all the values and divide to get the average frame time.
    framespersecond = 0;
    for (i = 0; i < count; i++)
    {
        framespersecond += frametimes[i];
    }

    framespersecond /= count;

    // now to make it an actual frames per second value...
    framespersecond = 1000.f / framespersecond;
}

#endif
