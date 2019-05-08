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
static ERROR_T sInitModules(void);
static ERROR_T sDestoryModuels(void);

inline void sDebugging(void);
static void sReboot(void);
static void sExit(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ERROR_SystemLog(const char* message)
{
    printf("%s\n", message); fflush(stdout);
}


inline void ERROR_StatusCheck_Inline(BRIK_STATUS errorStatus, const char* errorMessage, const char* _file, const char* _func, const int _line)
{
    if(errorStatus != BRIK_STATUS_OK)
    {
        printf("\n\n\n* BRIK ERROR * \nFILE : %s\nFUNC : %s\nLINE : %d\nMSG  : %s\nCODE : %d\n\n\n",_file, _func, _line, errorMessage, errorStatus); fflush(stdout);

        sReboot();
        //sExit();
        //sDebugging();
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Static Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void sDebugging(void)
{
    ERROR_SystemLog("Brik Breakpoint. \n\n");

    while(true);

    sExit();
    sReboot();
}

static void sReboot(void)
{
    ERROR_T ret = ERROR_OK;

    ret = sDestoryModuels();

    ret = sInitModules();

    ERROR_SystemLog("Brik Reboot. \n\n");

    if(ret != ERROR_OK)
    {
        ERROR_SystemLog("System Reboot. \n\n");
        sExit();
    }
}

static void sExit(void)
{
    ERROR_T ret = ERROR_OK;

    ret = sDestoryModuels();
    if(ret != ERROR_OK)
    {
        ERROR_SystemLog("System Reboot. \n\n");
        exit(ERROR_NOT_OK);
    }
    else
    {
        ERROR_SystemLog("Brik Shutdown. \n\n");
        exit(ERROR_NOT_OK);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Modules Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static ERROR_T sInitModules(void)
{
    ERROR_T ret = ERROR_OK;

    ret = MODULE_Display_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to Initialize Display.");
    }

    ret = MODULE_VideoHandler_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to Initialize Video Handler.");
    }

    return ret;
}

static ERROR_T sDestoryModuels(void)
{
    ERROR_T ret = ERROR_OK;

    ret = MODULE_Display_Destroy();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to Destroy Display.");
    }

    ret = MODULE_VideoHandler_Destroy();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to Destroy Video Handler.");
    }

    return ret;
}
