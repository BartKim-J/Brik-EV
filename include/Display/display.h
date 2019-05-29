/**
 * @file display.h
 * @author Ben
 * @date 4 Tue 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __DISPLAY_HANDLER_H_
#define __DISPLAY_HANDLER_H_


#ifdef DISPLAY_TYPE_FB
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
#endif

/*************************************************************
 * @name Display Module
 *
 *////@{
/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T  MODULE_Display_Init(void);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T  MODULE_Display_Init_Overlay(int width, int height, uint32_t src_format, uint32_t dst_format);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T  MODULE_Display_Clean(void);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T  MODULE_Display_Update(AVFrame* frame);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T  MODULE_Display_Destroy(void);

/*************************************************************@}*/

/*************************************************************
 * @name FPS
 *
 *////@{
#ifdef DISPLAY_TYPE_SDL
/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern float    MODULE_Display_FPS(void);

/*************************************************************@}*/
#endif

#endif
