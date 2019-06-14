/**
 * @file brik.c
 * @author Bato
 * @date  23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */

/* ******* INCLUDE ******* */
#include "brik_api.h"

/* ******* GLOBAL VARIABLE ******* */

/* ******* STATIC FUNCTIONS ******* */
/* *** Module *** */
static ERROR_T sModules_Init(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  main
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char* argv[])
{
    ERROR_T ret = ERROR_OK;

//    ERROR_SystemLog("\n\n Brik Start. \n");

    ret = sModules_Init();

    while(true)
    {
        /*/ Brik is Working /*/
    }

    return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Modules
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static ERROR_T sModules_Init(void)
{
    ERROR_T ret = ERROR_OK;

    ret = MODULE_Display_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize Display.");
    }

    ret = MODULE_VideoHandler_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize Video Handler.");
    }
    ret = MODULE_FrameHandler_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize Socket Listener.");
    }

    ret = MODULE_SocketListener_Init();
    if (ret != ERROR_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize Socket Listener.");
    }

#if false // @DEBUG
    sleep(5);

    ERROR_StatusCheck(BRIK_STATUS_NOT_OK ,"TEST REBOOT");
#endif

    return ret;
}


