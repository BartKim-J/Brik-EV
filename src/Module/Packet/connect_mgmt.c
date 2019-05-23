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
    int is_occupied;
    int32_t connection;
    int32_t connection_type;
    int32_t session_id;
} connection_status_t;

#define CONN_OCCUPIED      1
#define CONN_NOT_COOUPIED  0

#define CONN_PARAM_UNSET   (-1)

/* ******* GLOBAL VARIABLE ******* */

static int connection_count = 0;
static connection_status_t conn_status[MAX_CONNECTION];

/* ******* STATIC FUNCTIONS ******* */
int MODULE_ConnectManager_GetCount()
{
    return connection_count;
}

ERROR_T MODULE_ConnectManager_Open(int connection, int32_t connection_type, int32_t session_id)
{
    int i = 0;

    for(i = 0; i < MAX_CONNECTION; i++)
    {
        if (conn_status[i].is_occupied == CONN_NOT_COOUPIED)
        {
            conn_status[i].is_occupied     = CONN_OCCUPIED;
            conn_status[i].connection      = connection;
            conn_status[i].connection_type = connection_type;
            conn_status[i].session_id      = session_id;

            connection_count += 1;

            return ERROR_OK;
        }
    }
    return ERROR_NOT_OK;
}

ERROR_T MODULE_ConnectManager_Close(int32_t connection)
{
    int i = 0;

    for(i = 0; i< MAX_CONNECTION; i++)
    {
        if (conn_status[i].is_occupied == CONN_OCCUPIED && conn_status[i].connection == connection)
        {
            close(conn_status[i].connection);
            conn_status[i].is_occupied     = CONN_NOT_COOUPIED;
            conn_status[i].connection      = CONN_PARAM_UNSET;
            conn_status[i].connection_type = CONN_PARAM_UNSET;
            conn_status[i].session_id      = CONN_PARAM_UNSET;

            connection_count -= 1;

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
        if (conn_status[i].is_occupied == CONN_OCCUPIED)
        {
            close(conn_status[i].connection);
            conn_status[i].is_occupied     = CONN_NOT_COOUPIED;
            conn_status[i].connection      = CONN_PARAM_UNSET;
            conn_status[i].connection_type = CONN_PARAM_UNSET;
            conn_status[i].session_id      = CONN_PARAM_UNSET;

            connection_count -= 1;
        }
    }

    return ERROR_OK;
}
