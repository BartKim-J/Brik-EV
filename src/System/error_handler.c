/**
 * @file error_handler.c
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

/* ******* STATIC DEFINE ******* */

/* ******* GLOBAL VARIABLE ******* */

/* ******* STATIC FUNCTIONS ******* */
static void sDebugging(void);
static void sReboot(void);
static void sExit(void);

void ERROR_SystemLog(const char* message)
{
    printf("%s\n", message); fflush(stdout);
}


inline void ERROR_StatusCheck_Inline(BRIK_STATUS errorStatus, const char* errorMessage, const char* _file, const char* _func, const int _line)
{
    if(errorStatus != BRIK_STATUS_OK)
    {
        printf("\n\n\n* SN ERROR * \nFILE : %s\nFUNC : %s\nLINE : %d\nMSG  : %s\nCODE : %d\n\n\n",_file, _func, _line, errorMessage, errorStatus); fflush(stdout);

        sExit();
    }
}

static void sDebugging(void)
{
    while(true);
}

static void sReboot(void)
{
    /*
    SN_SYS_ERROR_SystemLog("System Reboot. \n\n");

    sync();
    reboot(RB_AUTOBOOT);
    */
    sExit();
}

static void sExit(void)
{
    exit(-1);
}
