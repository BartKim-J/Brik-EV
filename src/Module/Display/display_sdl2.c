/**
 * @file display_sdl2.c
 * @author Ben
 * @date  7 Mar 2019
 * @brief
 */
#ifdef DISPLAY_TYPE_SDL2
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

moduleImageViewer_t moduleImageViewer;

/* ******* STATIC FUNCTIONS ******* */
/* *** FPS *** */
static void sFPS_Init(void);
static void sFPS_Update(void);

//from YUV frame
static int clean_window(moduleImageViewer_t* module);
static int update_texture(moduleImageViewer_t* module, AVFrame* av_frame);
static int update_frame(moduleImageViewer_t* module, AVFrame* av_frame);

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */

ERROR_T MODULE_Display_Init(void)
 {
     ERROR_T ret = ERROR_OK;

    printf("MODULE INIT => Display handler.\n");

    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        printf("SDL Video already initialized\n");
        return ERROR_OK;
    }

    /* SDL INIT */
    ret = SDL_Init(SDL_INIT_VIDEO);
    if(ret != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Not Initialized params.");
    }

    ret = pthread_mutex_init(&mutex_sdl, NULL);
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize mutex.");
    }


    /* GET DISPLAY INFO */
    SDL_GetDesktopDisplayMode(0, &moduleImageViewer.dm);

    printf("Image Viwer => Module => Display Resolution [ %d x %d ] %dHz %d\n", moduleImageViewer.dm.w, moduleImageViewer.dm.h, \
                                                                                 moduleImageViewer.dm.refresh_rate, \
                                                                                 moduleImageViewer.dm.format);

    moduleImageViewer.window = SDL_CreateWindow(
             "Brik_EV",
             SDL_WINDOWPOS_UNDEFINED,




             SDL_WINDOWPOS_UNDEFINED,
             moduleImageViewer.dm.w,
             moduleImageViewer.dm.h,
             0
         );

     if (!moduleImageViewer.window) {
         ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "SDL: could not create window - exiting.");
     }

     moduleImageViewer.renderer = SDL_CreateRenderer(moduleImageViewer.window, -1, 0);
     if (!moduleImageViewer.renderer) {
         ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "SDL: could not create renderer - exiting.");
     }
     /* IMAGE INIT */
     clean_window(&moduleImageViewer);

     moduleImageViewer.texture = SDL_CreateTexture(
         moduleImageViewer.renderer,
         SDL_PIXELFORMAT_YV12,
         SDL_TEXTUREACCESS_STREAMING,
         moduleImageViewer.dm.w,
         moduleImageViewer.dm.h
     );

     if (!moduleImageViewer.texture) {
         ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "SDL: could not create texture - exiting.");
     }

     /* Clean Before Window Screen */
     ret = clean_window(&moduleImageViewer);


     sFPS_Init();

     return ret;
 }

 ERROR_T MODULE_Display_Clean(void)
{
    return clean_window(&moduleImageViewer);
}

 ERROR_T MODULE_Display_Update(AVFrame* av_frame)
{
    return update_frame(&moduleImageViewer, av_frame);
}


 ERROR_T MODULE_Display_Destroy(void)
{
    ERROR_T ret = ERROR_OK;

    SDL_DestroyTexture(moduleImageViewer.texture);
    SDL_DestroyRenderer(moduleImageViewer.renderer);
    SDL_DestroyWindow(moduleImageViewer.window);
    SDL_Quit();

    return ret;
}

float MODULE_Display_FPS(void)
{
    return framespersecond;
}

ERROR_T MODULE_Display_Init_Overlay(int width, int height, uint32_t format, uint32_t dstformat)
{
    ERROR_T ret = ERROR_OK;

    pthread_mutex_lock(&mutex_sdl);

    /* NOT USE IN SDL2 */

    pthread_mutex_unlock(&mutex_sdl);

    return ret;
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Screen
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */

static ERROR_T update_frame(moduleImageViewer_t* module, AVFrame* av_frame)
{
    ERROR_T ret = ERROR_OK;

    int image_w = 0, image_h = 0;

    /* GET PRINT TARGET INFO */
    ret = update_texture(module, av_frame);
    if (ret == 0)
    {
        SDL_QueryTexture(module->texture, NULL, NULL, &image_w, &image_h);
    }

    module->dest_rect.x = 0;
    module->dest_rect.y = 0;
    module->dest_rect.w = module->dm.w;
    module->dest_rect.h = module->dm.h;

    /* Drawing Image */
    //error += SDL_RenderClear(moduleImageViewer->renderer);
    ret = SDL_RenderCopy(module->renderer, module->texture, NULL, &module->dest_rect);
    if(!ret)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Get error while copying texture to renderer.");
    }

    SDL_RenderPresent(module->renderer);

    return ret;
}

static ERROR_T clean_window(moduleImageViewer_t* module)
{
    ERROR_T ret = ERROR_OK;

    pthread_mutex_lock(&mutex_sdl);

    /* Clear */
    ret += SDL_RenderClear(module->renderer);
    ret += SDL_SetRenderDrawColor(module->renderer, 35, 39, 42, 255);
    if(ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Get error from renderer while clearing.");
    }

    SDL_RenderPresent(module->renderer);

    pthread_mutex_unlock(&mutex_sdl);

    return ret;
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  IMAGE
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */

static ERROR_T update_texture(moduleImageViewer_t* module, AVFrame* av_frame)
{
    ERROR_T ret = ERROR_OK;

    if(module->renderer == NULL)
    {
        printf("No renderer found while loading texture\n");
    }

    /* Update Texture*/
    if(module->texture == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED, "Unable to find the texture to update.");
    }

#if true
    SDL_UpdateYUVTexture(module->texture,
                         NULL,
                         av_frame->data[0],
                         av_frame->linesize[0],
                         av_frame->data[1],
                         av_frame->linesize[1],
                         av_frame->data[2],
                         av_frame->linesize[2]);
#endif

    ret = SDL_UpdateTexture(module->texture, NULL, av_frame->extended_data, av_frame->width);

    sFPS_Update();

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
