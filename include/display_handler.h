/**
 * @file display_handler.h
 * @author Bato
 * @date 7 Mar 2019
 * @brief
 *
 */
#ifndef __DISPLAY_HANDLER_H_
#define __DISPLAY_HANDLER_H_

/*************************************************************
 * @struct frameBuffer_Image
 * @brief
 * @{
 */

typedef struct frameBuffer_Image
{
    unsigned char *rgb;
    unsigned char *alpha;

    int             w;
    int             h;

    int     colorType;
    int           bpp;
} FB_Image_t;

typedef struct frameBuffer_Window
{
    char* name;

    int            w;
    int            h;
    int          bpp;

    long  screenSize;
} FB_Window_t;

extern int      dh_display_init(void);
extern void     dh_display_init_video_overlay(int width, int height, uint32_t src_format, uint32_t dst_format);
extern void     dh_display_clean(void);
extern int      dh_display_decoded_frame(AVFrame* frame);
extern int      dh_display_destroy(void);
extern float    dh_display_get_FPS(void);

#endif
