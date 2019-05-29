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

/* ******* STATIC DEFINE ******* */
#define LISTEN_PORT         9500
#define MSG_QUEUE             30

/* ******* GLOBAL VARIABLE ******* */
static pthread_t       thread_sl;

static int sock_tcp;
static struct sockaddr_in  sock_tcp_addr;
static struct sockaddr_in  client_addr;

/* ******* STATIC FUNCTIONS ******* */
/* *** THREAD *** */
static void  thread_SocketListner_Cleanup(void *arg);
static void* thread_SocketListner(void *arg);

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
    sock_tcp_addr.sin_port        = htons(LISTEN_PORT);
    sock_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_tcp, (struct sockaddr*)&sock_tcp_addr, sizeof(sock_tcp_addr)) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to bind socket listener.");
    }

    if(listen(sock_tcp, MSG_QUEUE) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Listen Socket Failed.");
    }

    ret = pthread_create(&thread_sl, NULL, thread_SocketListner, NULL);
    if(ret != ERROR_OK)
    {
      ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize video thread.");
    }

    return ret;
}

ERROR_T MODULE_SocketListener_Destroy(void)
{
    ERROR_T ret = ERROR_OK;
    void*   tret = NULL;

    ret = pthread_cancel(thread_sl);
    if (ret != ERROR_OK)
    {
        ERROR_SystemLog("Brik Failed to try cancle thread. \n\n");
    }

    ret = pthread_join(thread_sl, &tret);
    if (ret != ERROR_NOT_OK)
    {
        if(tret != PTHREAD_CANCELED)
        {
            ERROR_SystemLog("Brik Failed Frame Handler Thread Clenaup. \n\n");
        }
    }


    if(close(sock_tcp) == ERROR_NOT_OK)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Close Socket Failed.");
    }

    return ret;
}

static void thread_SocketListner_Cleanup(void *arg)
{
    ERROR_SystemLog("Brik Start Socket Listenr Thread Clenaup. \n\n");
}

static void* thread_SocketListner(void *arg)
{
    ERROR_T ret = ERROR_OK;

    int          connection_client = 0;
    unsigned int client_addr_size  = 0;

    // Thread Cleanup
    pthread_cleanup_push(thread_SocketListner_Cleanup, NULL);

    while(true)
    {
        client_addr_size = sizeof(client_addr);
        connection_client = accept(sock_tcp, (struct sockaddr *)&client_addr, &client_addr_size);

        if(connection_client == ERROR_NOT_OK)
        {
            close(connection_client);
            ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Connection Failed.");
        }
        else
        {
            printf("new connection event ( %d )\n", connection_client);
            ret = MODULE_PacketHandler_Init(connection_client);

            if(ret == ERROR_NOT_OK)
            {
                printf("Socket Already Connected.\n");
            }
            else
            {
                printf("New Socket Connected.\n");
            }
        }
    }

    pthread_cleanup_pop(0);

    return ERROR_OK;
}
