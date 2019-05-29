/**
 * @file packet_handler.c
 * @author Lex
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
/* ******* INCLUDE ******* */
#include "brik_api.h"
#include "packet_handler.h"

#include "packet_cmd.h"
#include "packet_video.h"
#include "packet_audio.h"

/* ******* STATIC DEFINE ******* */
typedef struct packetHandlerManager {
        bool    isOccupied;

        pthread_t p_thread;
        int  connection_fd;
} packet_handler_manager_t;

/* ******* GLOBAL VARIABLE ******* */
static packet_handler_manager_t handlerManger[MAX_CONNECTION];

/* ******* STATIC FUNCTIONS ******* */
/* *** THREAD *** */
static void  thread_PacketHandler_Cleanup(void *arg);
static void* thread_PacketHandler(void* arg);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_PacketHandler_Init(int socket_fd)
{
    ERROR_T ret = ERROR_NOT_OK;
    int   index = 0;

    for(index = 0; index < MAX_CONNECTION; index++)
    {
        //if(!handlerManger[index].isOccupied)
        {
            //handlerManger[index].isOccupied      = true;
            handlerManger[index].connection_fd   = socket_fd;
            ret = pthread_create(&handlerManger[index].p_thread, NULL, thread_PacketHandler, (void*)(&handlerManger[index].connection_fd));
            if(ret != ERROR_OK)
            {
                ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to initialize packet handler thread.");
            }

            return ERROR_OK;
        }
    }

    return ERROR_NOT_OK;
}

ERROR_T MODULE_PacketHandler_Destroy(void)
{
    ERROR_T ret = ERROR_OK;

    MODULE_ConnectManager_CloseAll();

    return ret;
}

/* * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * *
 *
 *  Thread
 *
 * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
static void thread_PacketHandler_Cleanup(void *arg)
{
    ERROR_SystemLog("Brik Start Packet Handler Thread Clenaup. \n\n");

    char* received_payload = arg;

    if(received_payload != NULL)
    {
        free(received_payload);
        received_payload = NULL;
    }
}

static void* thread_PacketHandler(void *arg)
{
    int  connection_client    = -1;
    int  received_payload_len = 0;

    char buf_recv[PACKET_SIZE];
    int  readlen = 0;

    int32_t packet_type       = 0;
    int32_t payload_length    = 0;
    Packet* received_packet   = NULL;
    char*   received_payload  = NULL;

    //get connection as thread argument
    connection_client = *((int *)arg);

    // Thread Cleanup
    pthread_cleanup_push(thread_PacketHandler_Cleanup, received_payload);

    while(true)
    {
        // get 128 bytes of data from socket
        readlen = recv(connection_client, buf_recv, PACKET_SIZE, 0);

        if (readlen <= 0)
        {
            printf("socket might had been disconnected from the remote host.\n");
            MODULE_ConnectManager_Close(connection_client);

            return NULL;
        }

        if(readlen < PACKET_SIZE)
        {
            printf("packet size %d from connection %d too small\n", readlen, connection_client);
            continue;
        }

        received_packet = (Packet*)buf_recv;

        packet_type = received_packet->hdr.type;
        payload_length = received_packet->hdr.payloadSize;

        if(((packet_type & PACKET_TYPE_CMD_MASK)   != PACKET_TYPE_CMD_VALUE) &&
           ((packet_type & PACKET_TYPE_VIDEO_MASK) != PACKET_TYPE_VIDEO_VALUE) &&
           ((packet_type & PACKET_TYPE_AUDIO_MASK) != PACKET_TYPE_AUDIO_VALUE) &&
           ((packet_type & PACKET_TYPE_BACKCHANNEL_MASK) != PACKET_TYPE_BACKCHANNEL_VALUE))
        {
            printf("Invalid packet type 0x%08x had been received\n", packet_type);
            // in some cases
            continue;
        }

        if(payload_length > 0)
        {
            //collect payload
            received_payload = (char*)malloc(payload_length);
            if (received_payload == NULL)
            {
                ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate payload buffer.");
            }

            received_payload_len = 0;

            while (received_payload_len < payload_length)
            {
                readlen = recv(connection_client, &received_payload[received_payload_len],
                    MIN(SIZE_MAX_PAYLOAD_CHUNK, (payload_length - received_payload_len)), 0);

                if (readlen <= 0)
                {
                    printf("Socket reception Error!!!\n");
                    printf("socket might had been disconnected from the remote host.");
                    MODULE_ConnectManager_Close(connection_client);
                    return NULL;
                }

                received_payload_len += readlen;
            }
        }

        if((packet_type & PACKET_TYPE_CMD_MASK) == PACKET_TYPE_CMD_VALUE)
        {
            LOCAL_PacketHandler_command(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_VIDEO_MASK) == PACKET_TYPE_VIDEO_VALUE)
        {
            LOCAL_PacketHandler_Video(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_AUDIO_MASK) == PACKET_TYPE_AUDIO_VALUE)
        {
            LOCAL_PacketHandler_Audio(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_BACKCHANNEL_MASK) == PACKET_TYPE_BACKCHANNEL_VALUE)
        {
            LOCAL_PacketHandler_Backchannel(connection_client, received_packet, received_payload);
        }
        else
        {
            printf("Invalid_packet_type, 0x%08x had been received\n", packet_type);
        }

        if(payload_length > 0)
        {
            free(received_payload);
            received_payload = NULL;
        }
    }

    MODULE_ConnectManager_Close(connection_client);
    pthread_cleanup_pop(0);

    return ERROR_OK;
}
