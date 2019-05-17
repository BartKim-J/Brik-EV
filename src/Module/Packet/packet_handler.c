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

/* ******* STATIC DEFINE ******* */
typedef struct connection_info {


} connection_info_t;

/* ******* GLOBAL VARIABLE ******* */

static pthread_t p_threads[MAX_CONNECTION];
static int connection_fd[MAX_CONNECTION];
static bool connectionFlag = false;

/* ******* STATIC FUNCTIONS ******* */
/* *** Handler  *** */
static void* ph_thread_packet_handler(void* arg);
static void handle_cmd_packet(int connection_client, void* packet, void* payload);
static void handle_video_packet(int connection_client, void* packet, void* payload);
static void handle_audio_packet(int connection_client, void* packet, void* payload);
static void handle_backchannel_packet(int connection_client, void* packet, void* payload);

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

/* *** Packet Payload *** */
static void packet_video_codec(int connection_client, void* packet, void* payload);
static void packet_video_data(int connection_client, void* packet, void* payload);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ERROR_T MODULE_PacketHandler_Init(int index, int socket_fd)
{
    ERROR_T ret = ERROR_NOT_OK;

    connection_fd[index] = socket_fd;
    ret = pthread_create(&p_threads[index], NULL, ph_thread_packet_handler, (void*)&connection_fd[index]);

    return ret;
}

ERROR_T MODULE_PacketHandler_Destroy(void)
{
    ERROR_T ret = ERROR_OK;
    int i = 0;

    for(i = 0; i < MAX_CONNECTION; i++)
    {

    }

    return ret;
}

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

    printf("connection_client 0x%08x, connection_type: 0x%08x, session_id: 0x%08x\n", connection_client, connection_type, session_id);

    if((cm_get_connection_count() < MAX_CONNECTION) && (connectionFlag == false))
    {        // identify connection type
        switch(connection_type)
        {
            case CONNECTION_TYPE_CONTROL:
                printf("Control Connection request received from connection %d\n", connection_client);
                result = cm_add_new_connection(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish control connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_VIDEO:

                connectionFlag = true; // DEBUG

                printf("Video Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = cm_add_new_connection(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish video connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_AUDIO:
                printf("Audio Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = cm_add_new_connection(connection_client, connection_type, session_id);
                if (result != ERROR_OK)
                {
                    printf("Failed to establish audio connection(maximum exceed)\n");
                }
                break;

            case CONNECTION_TYPE_BACKCHANNEL:
                printf("Audio Connection request received from connection %d\n", connection_client);
                //TODO: initialize video data queue
                result = cm_add_new_connection(connection_client, connection_type, session_id);
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
        cm_close_current_connection(connection_client);
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

    usleep(500000);
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
        cm_close_current_connection(connection_client);
    }

    // TODO: check dead connection

    // Workaround: close all existing connection
    // Check which one would be cm_close_current_connection or cm_close_all_connection
#if 0
    cm_close_current_connection(connection_client);
#else
    cm_close_all_connections();
#endif
    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_UNKNOWN_MESSAGE ,"Failed to allocate video handler message: VH_MSG_TYPE_VIDEO_DISCONNECT.");
    }

    connectionFlag = false; // DEBUG

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
        cm_close_current_connection(connection_client);
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
        cm_close_current_connection(connection_client);
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

static void handle_cmd_packet(int connection_client, void* packet, void* payload)
{
    CommandPacket* packet_cmd = (CommandPacket*) packet;
    int32_t packet_type;

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
        default:
            printf("Invalid command packet type 0x%x\n", packet_type);
            break;
    }

}

static void packet_video_codec(int connection_client, void* packet, void* payload)
{
    CodecDataPacket* received_codec_packet;
    void* extradata;
    void* codec_packet;

/*
    int32_t video_codec_type;
    int32_t video_codec_width;
    int32_t video_codec_height;
    int32_t video_codec_framerate;
*/

    video_data_msg_t* message;
    int msg_ret = 0;

    received_codec_packet = (CodecDataPacket*)packet;

    //extradata = payload;
    codec_packet = malloc(sizeof(CodecDataPacket));
    if (codec_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate codec packet.");
    }
    memcpy(codec_packet, received_codec_packet, sizeof(CodecDataPacket));

    extradata = malloc(received_codec_packet->hdr.payloadSize);
    if (extradata == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate extradata to pass.");
    }
    memcpy(extradata, payload, received_codec_packet->hdr.payloadSize);

/*
    video_codec_type = received_codec_packet->video.codecType;   // Only support VIDEO_CODEC_H264 for now
    video_codec_width = received_codec_packet->video.width;
    video_codec_height = received_codec_packet->video.height;
    video_codec_framerate = received_codec_packet->video.framerate;  // Not used for now
*/

    // Need to parse SPS from extradata to get video resolution?
    // check if real resolution is okay.

    // let the video handler thread know the new video setting.


    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video handler message.");
    }

    message->packet = codec_packet;
    message->payload = extradata;

    printf("sending video codec codec_packet = %p, extradata = %p\n", codec_packet, extradata);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_CODEC);
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video_codec message to video handler: %d\n", msg_ret);
    }
}

static void packet_video_data(int connection_client, void* packet, void* payload)
{
    AVPacketPacket* received_video_data;
    video_data_msg_t* message;
    int msg_ret = 0;

    void* framedata;
    void* video_packet;

    received_video_data = (AVPacketPacket*)packet;

    // decode here and display from video thread? or just pass whole payload to video thread?
    if (received_video_data->hdr.payloadSize < 4)
    {
        printf("Invalid h.264 packet. Init seq is not included\n");
        return;
    }

    video_packet = malloc(sizeof(AVPacketPacket));
    if (video_packet == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate AVPacket video packet.");
    }
    memcpy(video_packet, received_video_data, sizeof(AVPacketPacket));

    framedata = malloc(received_video_data->hdr.payloadSize);
    if (framedata == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate frame data.");
    }
    memcpy(framedata, payload, received_video_data->hdr.payloadSize);

    message = malloc(sizeof(video_data_msg_t));
    if (message == NULL)
    {
        ERROR_StatusCheck(BRIK_STATUS_NOT_INITIALIZED ,"Failed to allocate video handler message.");
    }

    message->packet = video_packet;
    message->payload = framedata;

    //printf("sending video data message packet = %p, payload = %p\n", video_packet, framedata);
    msg_ret = MODULE_VideoHandler_SendMessage((void*)message, VH_MSG_TYPE_VIDEO_DATA);
    if(msg_ret != ERROR_OK)
    {
        printf("Error while sending video_codec message to video handler: %d\n", msg_ret);
    }
}


static void handle_video_packet(int connection_client, void* packet, void* payload)
{
    Packet* received_video_packet;
    int32_t packet_type;

    received_video_packet = (Packet*)packet;
    packet_type = received_video_packet->hdr.type;

    switch(packet_type)
    {
        case PACKET_VIDEO_CODEC:
            packet_video_codec(connection_client, packet, payload);
            break;
        case PACKET_VIDEO_PACKET:
            packet_video_data(connection_client, packet, payload);
            break;
        case PACKET_CURSOR:
            //TODO: Handle Cursor data, Not supported by Airplay receiver
            break;
        case PACKET_CURSOR_POS:
            //TODO: Move cursor sprite, Not supported by Airplay receiver
            break;
        default:
            printf("Invalid video packet type\n");
            break;
    }
}

static void handle_audio_packet(int connection_client, void* packet, void* payload)
{
    //TODO: handle audio packets
}

static void handle_backchannel_packet(int connection_client, void* packet, void* payload)
{
    //TODO: Handle backchannel packets Not for Airplay
}

static void* ph_thread_packet_handler(void *arg)
{
    int connection_client = -1;
    int received_payload_len = 0;
    //get connection as thread argument
    connection_client = *((int *)arg);
    char buf_recv[PACKET_SIZE];
    int readlen = 0;

    int32_t packet_type;
    int32_t payload_length;
    Packet* received_packet;
    char* received_payload = NULL;

    while(true)
    {
        // get 128 bytes of data from socket
        readlen = recv(connection_client, buf_recv, PACKET_SIZE, 0);

        if (readlen <= 0)
        {
            printf("socket might had been disconnected from the remote host.\n");
            cm_close_current_connection(connection_client);

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
                    cm_close_current_connection(connection_client);
                    return NULL;
                }

                received_payload_len += readlen;
            }
        }

        if((packet_type & PACKET_TYPE_CMD_MASK) == PACKET_TYPE_CMD_VALUE)
        {
            handle_cmd_packet(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_VIDEO_MASK) == PACKET_TYPE_VIDEO_VALUE)
        {
            handle_video_packet(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_AUDIO_MASK) == PACKET_TYPE_AUDIO_VALUE)
        {
            handle_audio_packet(connection_client, received_packet, received_payload);
        }
        else if((packet_type & PACKET_TYPE_BACKCHANNEL_MASK) == PACKET_TYPE_BACKCHANNEL_VALUE)
        {
            handle_backchannel_packet(connection_client, received_packet, received_payload);
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

    cm_close_current_connection(connection_client);
    return NULL;
}
