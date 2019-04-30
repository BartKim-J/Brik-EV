/**
 * @file display_handler.c
 * @author Ben
 * @date 4 Tue 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifdef DISPLAY_TYPE_FB
#include "brik_api.h"
#include "display_handler.h"


#ifdef linux
/* OS */
#include <linux/fb.h>

/* SYSTEM */
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/mman.h>
#include <sys/ioctl.h>

/* FILE IO */
#include <fcntl.h>
#else

#endif

/* ******* STATIC DEFINE ******* */
#define WINDOW_NAME_MAX_LENGTH 255
#define WINDOW_NAME "/dev/fb0"


/* ******* GLOBAL VARIABLE ******* */
static FB_Window_t gWindow; // NEED SEQUNCE

/* ******* STATIC FUNCTIONS ******* */
/* *** FRAME BUFFER *** */
static int sLoadWindow(const char* devicename, FB_Window_t* window);
//static int sUpdateWindow(const FB_Window_t window, const AVFrame image);
static int sCleanWindow(const FB_Window_t window);
static int sDistroyWindow(FB_Window_t* window);

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
int dh_display_init(void)
{
    int ret = 0;

    /* FOR @DEBUG NEED FIX IT */
    sDistroyWindow(&gWindow);

    /* Check and Load Window */
    ret = sLoadWindow(WINDOW_NAME, &gWindow);

    /* Clean Before Window Screen */
    ret = sCleanWindow(gWindow);

    return ret;
}

void dh_display_clean(void)
{
    sCleanWindow(gWindow);
}

int dh_display_distory(void)
{
    int ret = 0;

    return ret;
}

int dh_display_decoded_frame(AVFrame* frame)
{
    return 0;
}


/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  WINDOW( static )
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */

static int sLoadWindow(const char* devicename, FB_Window_t* window)
{
    int ret = 0;

    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if(devicename == NULL)
    {
        return -1;
    }

    if(window == NULL)
    {
        return -1;
    }

    fbfd = open(devicename, O_RDWR);
    if (fbfd == -1)
    {
        return -1;
    }

    window->name = (char*)malloc(strlen(devicename));
    if(window->name == NULL)
    {
        return -1;
    }
    strcpy(window->name, devicename);

    /* GET FIX SCREEN INFO */
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        return -1;
    }

    /* GET VARIABLE SCREEN INFO */
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        return -1;
    }

    window->h   = vinfo.yres;
    window->w   = vinfo.xres;
    window->bpp = vinfo.bits_per_pixel;

    window->screenSize = (window->w * window->h) * (window->bpp / 8);

    printf("Image Viwer => Module => Window Resolution [ %d x %d ], %dbpp\n", window->w, window->h, window->bpp);

    close(fbfd);

    return ret;
}

/*
static int sUpdateWindow(const FB_Window_t window, const AVFrame layer)
{
    int ret = 0;

    int fbfd = 0;
    int bp = 0;

    char* fbp = 0;

    unsigned short* fbbuff = NULL;
    unsigned char*  fbptr  = NULL;
    unsigned char*  imptr  = NULL;

    if(window.name == NULL)
    {
        SN_SYS_ERROR_StatusCheck(SN_STATUS_NOT_INITIALIZED, "window not initialzed.");
    }

    fbfd = open(window.name, O_RDWR);
    if (fbfd == -1)
    {
        SN_SYS_ERROR_StatusCheck(SN_STATUS_NOT_INITIALIZED, "cannot open framebuffer device.");
    }

    fbbuff = convertRGB2FB(fbfd, image.rgb, image.w * image.h, window.bpp, &bp);

    fbp = (char *)mmap(0, window.screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1)
    {
        SN_SYS_ERROR_StatusCheck(SN_STATUS_NOT_OK, "failed to map framebuffer device to memory.");
    }

    fbptr = (unsigned char *)fbp;
    imptr = (unsigned char *)fbbuff;

    for (int i = 0; i < image.h; i++, fbptr += image.w * bp, imptr += image.w * bp)
    {
        memcpy(fbptr, imptr, image.w * bp);
    }

    free(fbbuff);
    fbbuff = NULL;

    munmap(fbp, window.screenSize);

    close(fbfd);

    return ret;
}
*/

static int sCleanWindow(const FB_Window_t window)
{
    int ret = 0;

    int fbfd = 0;
    int i = 0;
    int j = 0;
    long int location = 0;
    char *fbp = 0;

    if(window.name == NULL)
    {
        return -1;
    }

    fbfd = open(window.name, O_RDWR);
    if (fbfd == -1)
    {
        return -1;
    }

    /* Map the device to memory */
    fbp = (char *)mmap(0, window.screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1)
    {
        return -1;
    }

    /* Figure out where in memory to put the pixel */
    for (i = 0; i < window.h; i++)
    {

        for (j = 0; j < window.w; j++)
        {
            location = (j * (window.bpp / 8)) +
                       (i * (window.w * (window.bpp / 8)));

            if(window.bpp == 32)
            {
                *(fbp + location)     = 0;        // black
                *(fbp + location + 1) = 0;
                *(fbp + location + 2) = 0;
                *(fbp + location + 3) = 255;
                //location += 4;
            }
            else //assume 16bpp
            {
                int b = 0;    //black
                int g = 0;
                int r = 0;
                unsigned short int t = r<<11 | g << 5 | b;
                *((unsigned short int*)(fbp + location)) = t;
            }
            //else 8bpp
        }
    }

    munmap(fbp, window.screenSize);

    close(fbfd);

    return ret;
}

static int sDistroyWindow(FB_Window_t* window)
{
    int ret = 0;

    if(window == NULL)
    {
        return -1;
    }

    window->name       = NULL;

    window->screenSize = 0;
    window->h          = 0;
    window->w          = 0;
    window->bpp        = 0;

    return ret;
}




#endif
