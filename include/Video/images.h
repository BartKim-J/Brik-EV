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

typedef enum image_resource {
    INTRO_IMAGE,
    IMAGE_COUNT
} imageResoure_t;

extern ERROR_T MODULE_Image_UpdateImage(imageResoure_t imgIndex);
extern ERROR_T MODULE_Image_LoadImages(void);
extern ERROR_T MODULE_Image_CleanImages(void);
