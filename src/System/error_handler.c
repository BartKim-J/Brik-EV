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
#define LINUX_REBOOT_CMD_POWER_OFF 0x4321fedc

/* ******* GLOBAL VARIABLE ******* */

/* ******* STATIC FUNCTIONS ******* */
static ERROR_T sInitModules(void);
static ERROR_T sDestoryModuels(void);

inline void sDebugging(void);
static void sReboot(void);
static void sExit(void);

static bool    isRebooting = false;

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

        if(!isRebooting)
        {
            isRebooting = true;

            sReboot();

            isRebooting = false;
        }
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

    MODULE_Image_UpdateImage(ERROR_IMAGE);

    ERROR_SystemLog("Brik Rebooting... \n\n");

    sleep(5);

    ret = sDestoryModuels();
    if(ret == ERROR_OK)
    {
        ret = sInitModules();
    }
    else
    {
        ERROR_SystemLog("Brik Destroy Failed!! \n\n");
    }


    if(ret != ERROR_OK)
    {
        ERROR_SystemLog("Brik Restart Failed!! \n\n");
        ERROR_SystemLog("System Reboot!! \n\n");

        sync();
        reboot(LINUX_REBOOT_CMD_POWER_OFF);
        exit(ERROR_NOT_OK);
    }

    ERROR_SystemLog("Brik Restart. \n\n");
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

    ERROR_SystemLog("Brik Restart Moduels... \n\n");

    ERROR_SystemLog("Brik Restart Display Moduels... \n\n");
    ret = MODULE_Display_Init();
    if (ret != ERROR_OK)
    {
        return ret;
    }

    ERROR_SystemLog("Brik Restart Video Handler Moduels... \n\n");
    ret = MODULE_VideoHandler_Init();
    if (ret != ERROR_OK)
    {
        return ret;
    }

    ERROR_SystemLog("Brik Restart Packet Handler Moduels... \n\n");
    /*
        PACKET HANDLER s
     */

    return ret;
}

static ERROR_T sDestoryModuels(void)
{
    ERROR_T ret = ERROR_OK;

    ERROR_SystemLog("Brik Destroy Moduels... \n\n");

    ERROR_SystemLog("Brik Destroy Display Moduels... \n\n");

    ret = MODULE_Display_Destroy();
    if (ret != ERROR_OK)
    {
        return ret;
    }


    ERROR_SystemLog("Brik Destroy Packet Handler Moduels... \n\n");
    ret = MODULE_PacketHandler_Destroy();
    if (ret != ERROR_OK)
    {
        return ret;
    }

    ERROR_SystemLog("Brik Destroy Video Handler Moduels... \n\n");
    ret = MODULE_VideoHandler_Destroy();
    if (ret != ERROR_OK)
    {
        return ret;
    }

    ERROR_SystemLog("Brik All Moduels Destroyed... \n\n");

    return ret;
}
