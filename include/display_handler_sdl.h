/**
 * @file display_handler_sdl.h
 * @author Bato
 * @date 7 Mar 2019
 * @brief
 *
 */
#ifndef __DISPLAY_HANDLER_SDL_H_
#define __DISPLAY_HANDLER_SDL_H_

/*************************************************************
 * @struct frameBuffer_Image
 * @brief
 * @{
 */

#ifdef SDL2
 typedef struct image_viewer
 {
     SDL_Window     *window;
     SDL_Renderer *renderer;
     SDL_Texture   *texture;
     SDL_DisplayMode dm;

     SDL_Rect     dest_rect;

     uint32_t    image_w;
     uint32_t    image_h;
 } moduleImageViewer_t;
#else
typedef struct image_viewer
{
    SDL_Surface     *screen;
    SDL_Overlay     *overlay;
    struct SwsContext *sws_ctx;
    SDL_Rect        rect_overlay_dst;

    int    screen_w;
    int    screen_h;
    uint32_t    image_w;
    uint32_t    image_h;
} moduleImageViewer_t;
#endif

 /*************************************************************@}*/


#endif
