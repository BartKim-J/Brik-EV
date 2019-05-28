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
/* *** LOCAL *** */
#include "brik_api.h"

/* ******* GLOBAL VARIABLE ******* */
static int sock_tcp;
static struct sockaddr_in  sock_tcp_addr;
static struct sockaddr_in  client_addr;

/* ******* STATIC FUNCTIONS ******* */
/* *** Module *** */
static ERROR_T sModules_Init(void);

/* *** TCP/IP *** */
ERROR_T MODULE_SocketListener_Init(void);
ERROR_T MODULE_SocketListener_Destroy(void);

/* *** Handler *** */
static ERROR_T sClientHandler(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  main
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char* argv[])
{
    ERROR_T ret = ERROR_OK;

    ret = sModules_Init();

    ERROR_SystemLog("\n\n Brik Start. \n");

    while(true)
    {
        ret = sClientHandler();
        if(ret != ERROR_OK)
        {
            ERROR_StatusCheck(BRIK_STATUS_NOT_OK ,"Client Connetion Error.");
        }
    }

    return ERROR_OK;
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Socket & TCP
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_SocketListener_Init(void)
{
    ERROR_T ret = ERROR_OK;

    // Init socket and server thread
    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_tcp == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to open receiver socket.");
    }

    memset(&sock_tcp_addr, 0, sizeof(sock_tcp_addr));
    sock_tcp_addr.sin_family      = AF_INET;
    sock_tcp_addr.sin_port        = htons(9500);
    sock_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_tcp, (struct sockaddr*)&sock_tcp_addr, sizeof(sock_tcp_addr)) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to bind socket listener.");
    }

    if(listen(sock_tcp, 30) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Listen Socket Failed.");
    }

    return ret;
}

ERROR_T MODULE_SocketListener_Destroy(void)
{
    ERROR_T ret = ERROR_OK;

    if(close(sock_tcp) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Close Socket Failed.");
    }

    return ret;
}

static ERROR_T sClientHandler(void)
{
    ERROR_T ret = ERROR_OK;

    int connection_client = 0;

    unsigned int client_addr_size = 0;

    client_addr_size = sizeof(client_addr);
    connection_client = accept(sock_tcp, (struct sockaddr *)&client_addr, &client_addr_size);

    if(connection_client == ERROR_NOT_OK)
    {
        close(connection_client);
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Connection Failed.");
    }
    else
    {
        printf("new connection %d\n", connection_client);

        if(MODULE_PacketHandler_Init(connection_client) == ERROR_NOT_OK)
        {
            //return ret;
            printf("Over Connection.");
        }
    }

    return ret;
}
