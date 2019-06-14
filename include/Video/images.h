/**
 * @file images.h
 * @author bato
 * @date 23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __IMAGES_H_
#define __IMAGES_H_

/*************************************************************
 * @name Images Struture
 *
 *////@{
typedef enum image_resource {
    INTRO_IMAGE,
    ERROR_IMAGE,
    IMAGE_COUNT
} imageResoure_t;

/*************************************************************@}*/

/*************************************************************
 * @name Images Module
 *
 *////@{

/** @brief Display to screen from image buffers
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Image_UpdateImage(imageResoure_t imgIndex);

/** @brief load to image buffer from all image files.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Image_LoadImages(void);

/** @brief cleanup all image buffers
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Image_CleanImages(void);

/*************************************************************@}*/

#endif
