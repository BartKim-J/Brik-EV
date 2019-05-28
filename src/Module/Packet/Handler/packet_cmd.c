/**
 * @file video_handler.c
 * @author Ben
 * @date 5 Tue 2019
 * @brief
 *
 * @bug sw decoder not working maybe sdl option.
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
/* ******* INCLUDE ******* */
#include "brik_api.h"
#include "packet_cmd.h"


/* ******* STATIC DEFINE ******* */

/* ******* GLOBAL VARIABLE ******* */

/* ******* STATIC FUNCTIONS ******* */
/* *** Packet Commands *** */
static void packet_cmd_connect(int connection_client, void* packet);
static void packet_cmd_disconnect(int connection_client, void* packet);
static void packet_cmd_pause(int connection_client, void* packet);
static void packet_cmd_resume(int connection_client, void* packet);
static void packet_cmd_sleep(int connection_client, void* packet);
static void packet_cmd_wakeup(int connection_client, void* packet);
static void packet_cmd_dispmode(int connection_client, void* packet);
static void packet_cmd_bcenable(int connection_client, void* packet);
static void packet_cmd_timesync(int connection_client, void* packet);
static void packet_cmd_avreset(int connection_client, void* packet);
static void packet_cmd_machinename(int connection_client, void* packet, void* payload);
static void packet_cmd_checkconn(int connection_client, void* packet);
static void packet_cmd_winstate(int connection_client, void* packet);
static void packet_cmd_win_notify(int connection_client, void* packet);

ERROR_T LOCAL_PacketHandler_command(int connection_client, void* packet, void* payload)
{
    ERROR_T ret = ERROR_OK;
    CommandPacket* packet_cmd = (CommandPacket*) packet;
    int32_t     packet_type   = PACKET_CMD_UNKOWN;

    packet_type = packet_cmd->hdr.type;
    switch (packet_type)
    {
        case PACKET_CMD_CONNECT:
            packet_cmd_connect(connection_client, packet);
            break;
        case PACKET_CMD_DISCONNECT:
            packet_cmd_disconnect(connection_client, packet);
            break;
        case PACKET_CMD_PAUSE:
            packet_cmd_pause(connection_client, packet);
            break;
        case PACKET_CMD_RESUME:
            packet_cmd_resume(connection_client, packet);
            break;
        case PACKET_CMD_SLEEP:
            packet_cmd_sleep(connection_client, packet);
            break;
        case PACKET_CMD_WAKEUP:
            packet_cmd_wakeup(connection_client, packet);
            break;
        case PACKET_CMD_DISPMODE:
            packet_cmd_dispmode(connection_client, packet);
            break;
        case PACKET_CMD_BCENABLE:
            packet_cmd_bcenable(connection_client, packet);
            break;
        case PACKET_CMD_TIMESYNC:
            packet_cmd_timesync(connection_client, packet);
            break;
        case PACKET_CMD_AVRESET:
            packet_cmd_avreset(connection_client, packet);
            break;
        case PACKET_CMD_MACHINENAME:
            packet_cmd_machinename(connection_client, packet, payload);
            break;
        case PACKET_CMD_CHECKCONN:
            packet_cmd_checkconn(connection_client, packet);
            break;
        case PACKET_CMD_WINSTATE:
            packet_cmd_winstate(connection_client, packet);
            break;
        case PACKET_CMD_WIN_NOTIFY:
            packet_cmd_win_notify(connection_client, packet);
            break;
        case PACKET_CMD_UNKOWN:
        default:
            printf("Invalid command packet type 0x%x\n", packet_type);
            break;
    }

    return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void packet_cmd_connect(int connection_client, void* packet)
{
    int32_t connection_type;        //param[0]
    int32_t session_id;             //param[1]
    CommandPacket* received_packet;
    CommandPacket response_packet;
    video_data_msg_t * message;
    int msg_ret = 0;
    int32_t result = 0;
    int send_len = 0;

    printf("PACKET_CMD_CONNECT received\n");

    memset(&response_packet, 0, sizeof(response_packet));

    received_packet = (CommandPacket*)packet;

    connection_type = received_packet->param[0];
    session_id      = received_packet->param[1];

    response_packet.hdr.type = PACKET_CMD_CONNECT_RESP;
    response_packet.hdr.payloadSize = 0;
    response_packet.param[0] = connection_type;
    response_packet.param[1] = session_id;

    printf("connection_client 0x%08x, connection_type: 0x%08x, session_id: 0x%08x current Connected Cnt : %d\n", \
            connection_client, connection_type, session_id, MODULE_ConnectManager_GetCount());

    if(MODULE_ConnectManager_GetCount() < MAX_CONNECTION)
    {        // identify connection type
        switch(connection_type)
        {
            case CONNECTION_TYPE_CONTROL:
                printf("Control Connection request received from connection %d\n", connection_client);
                result = MODULE_ConnectManager_Open(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish control connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_VIDEO:
                printf("Video Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = MODULE_ConnectManager_Open(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish video connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_AUDIO:
                printf("Audio Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = MODULE_ConnectManager_Open(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish audio connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_BACKCHANNEL:
                printf("Audio Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = MODULE_ConnectManager_Open(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish backchannel connection(maximum exceed)\n");
                }
                result = ERROR_OK;
                break;
            default:
                printf("Invalid connection type 0x%8x\n", connection_type);
                result = ERROR_NOT_OK;
                break;
        }
    }
    else
    {
        printf("Maximum connection exceed!\n");
        result = ERROR_NOT_OK;
    }

    if (result == ERROR_OK)
    {
        // TODO: check if dead connection
        // dead connection: same connection type is remained on different connection
    }

    //generate response
    response_packet.param[2] = result;
    send_len = send(connection_client, (void*)&response_packet, sizeof(response_packet), 0);

    if (send_len < ERROR_OK)
    {
        printf("Socket %d had been disconnected from the remote host\n", connection_client);
        MODULE_ConnectManager_Close(connection_client);
    }
    else
    {
        printf("%d bytes packet PACKET_CMD_CONNECT_RESP sent to connection %d\n", send_len, connection_client);
    }

    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Failed to allocate video handler message: VH_MSG_TYPE_VIDEO_STOP.");
    }

    printf("sending video stop message = %p, extradata = %p\n", message->packet, message->payload);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_CONNECT);
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video stop message to video handler: %d\n", msg_ret);
    }
}

static void packet_cmd_disconnect(int connection_client, void* packet)
{
    CommandPacket response_packet;
    int send_len = 0;
    video_data_msg_t * message;
    int msg_ret = 0;

    printf("PACKET_CMD_DISCONNECT received via socket %d\n", connection_client);

    // send response packet first
    memset(&response_packet, 0, sizeof(response_packet));
    response_packet.hdr.type = PACKET_CMD_DISCONNECT_RESP;
    response_packet.hdr.payloadSize = 0;

    send_len = send(connection_client, (void*)&response_packet, sizeof(response_packet), 0);

    if (send_len <= ERROR_OK)
    {
        printf("Connection %d had been disconnected from the remote host\n", connection_client);
        MODULE_ConnectManager_Close(connection_client);
    }

    // TODO: check dead connection

    // Workaround: close all existing connection
    // Check which one would be MODULE_ConnectManager_Close or MODULE_ConnectManager_CloseAll
#if false
    MODULE_ConnectManager_Close(connection_client);
#else
    MODULE_ConnectManager_CloseAll();
#endif

    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Failed to allocate video handler message: VH_MSG_TYPE_VIDEO_DISCONNECT.");
    }

    printf("sending video stop message = %p, extradata = %p\n", message->packet, message->payload);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_DISCONNECT );
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video stop message to video handler: %d\n", msg_ret);
    }
}

static void packet_cmd_pause(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_resume(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_sleep(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_wakeup(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_dispmode(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_bcenable(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_timesync(int connection_client, void* packet)
{
    CommandPacket* received_packet;
    CommandPacket response_packet;
    int send_len = 0;

    int64_t sender_time;
    int64_t receiver_time;

    //printf("PACKET_CMD_TIMESYNC received via conection %d\n", connection_client);

    received_packet = (CommandPacket*)packet;

    memset(&response_packet, 0, sizeof(response_packet));
    response_packet.hdr.type = PACKET_CMD_TIMESYNC_RESP;


    sender_time = received_packet->l_param[0];

    // TODO: get real time value from the system
    receiver_time = sender_time;
    received_packet->l_param[0] = sender_time;
    received_packet->l_param[1] = receiver_time;

    send_len = send(connection_client, (void*)&response_packet, sizeof(response_packet), 0);

    if (send_len < 0)
    {
        printf("Connection %d had been disconnected from the remote host\n", connection_client);
        MODULE_ConnectManager_Close(connection_client);
    }
}

static void packet_cmd_avreset(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_machinename(int connection_client, void* packet, void* payload)
{
    CommandPacket* received_packet;
    CommandPacket response_packet;
    int send_len = 0;
    char name_buf[128] = {0, };

    printf("PACKET_CMD_MACHINENAME received via conection %d\n", connection_client);

    received_packet = (CommandPacket*)packet;

    memset(&response_packet, 0, sizeof(response_packet));
    response_packet.hdr.type = PACKET_CMD_MACHINENAME_RESP;

    response_packet.hdr.payloadSize = 0;

    snprintf(name_buf, MIN(received_packet->hdr.payloadSize, 127), "%s", (char*)payload);

    printf("Machine Name: %s\n", name_buf);

    //set client machine name to display on the screen

    send_len = send(connection_client, (void*)&response_packet, sizeof(response_packet), 0);

    if (send_len < 0)
    {
        printf("Connection %d had been disconnected from the remote host\n", connection_client);
        MODULE_ConnectManager_Close(connection_client);
    }
}

static void packet_cmd_checkconn(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_winstate(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

static void packet_cmd_win_notify(int connection_client, void* packet)
{
    // Do nothing for now
    return;
}

