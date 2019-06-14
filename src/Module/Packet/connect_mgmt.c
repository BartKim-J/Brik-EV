/**
 * @file connect_mgmt.c
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
#include "connect_mgmt.h"

/* ******* STATIC DEFINE ******* */
typedef struct connection_status {
    bool    isOccupied;
    int32_t connection_client;
    int32_t connection_type;
    int32_t session_id;
} connection_status_t;

#define CONN_PARAM_UNSET   (-1)

/* ******* GLOBAL VARIABLE ******* */
static int connection_count = 0;
static connection_status_t conn_status[MAX_CONNECTION];

/* ******* STATIC FUNCTIONS ******* */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Extern Functions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int MODULE_ConnectManager_GetCount(void)
{
    return connection_count;
}

ERROR_T MODULE_ConnectManager_Open(int connection_client, int32_t connection_type, int32_t session_id)
{
    int i = 0;

    for(i = 0; i < MAX_CONNECTION; i++)
    {
        if(!conn_status[i].isOccupied)
        {
            conn_status[i].isOccupied        = true;
            conn_status[i].connection_client = connection_client;
            conn_status[i].connection_type   = connection_type;
            conn_status[i].session_id        = session_id;

            connection_count++;

            return ERROR_OK;
        }
    }

    return ERROR_NOT_OK;
}

ERROR_T MODULE_ConnectManager_Close(int32_t connection_client)
{
    int i = 0;

    for(i = 0; i< MAX_CONNECTION; i++)
    {
        if((conn_status[i].isOccupied) && (conn_status[i].connection_client == connection_client))
        {
            close(conn_status[i].connection_client);
            conn_status[i].isOccupied        = false;
            conn_status[i].connection_client = CONN_PARAM_UNSET;
            conn_status[i].connection_type   = CONN_PARAM_UNSET;
            conn_status[i].session_id        = CONN_PARAM_UNSET;

            connection_count--;

            return ERROR_OK;
        }
    }
    return ERROR_NOT_OK;
}

ERROR_T MODULE_ConnectManager_CloseAll(void)
{
    int i = 0;

    for(i = 0; i< MAX_CONNECTION; i++)
    {
        if (conn_status[i].isOccupied)
        {
            close(conn_status[i].connection_client);
            conn_status[i].isOccupied        = false;
            conn_status[i].connection_client = CONN_PARAM_UNSET;
            conn_status[i].connection_type   = CONN_PARAM_UNSET;
            conn_status[i].session_id        = CONN_PARAM_UNSET;

            connection_count--;
        }
    }

    return ERROR_OK;
}
