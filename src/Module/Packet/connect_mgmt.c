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


typedef struct connection_status{
    int is_occupied;
    int32_t connection;
    int32_t connection_type;
    int32_t session_id;
}connection_status_t;

static int connection_count = 0;
static connection_status_t conn_status[MAX_CONNECTION];

int cm_get_connection_count()
{
    return connection_count;
}

int cm_add_new_connection(int connection, int32_t connection_type, int32_t session_id)
{
    int i = 0;

    for(i = 0; i < MAX_CONNECTION; i++)
    {
        if (conn_status[i].is_occupied == 0)
        {
            conn_status[i].is_occupied = 1;
            conn_status[i].connection = connection;
            conn_status[i].connection_type = connection_type;
            conn_status[i].session_id = session_id;
            connection_count += 1;
            return ERROR_OK;
        }
    }
    return ERROR_NOT_OK;
}

int cm_close_current_connection(int32_t connection)
{
    int i = 0;
    for(i = 0; i< MAX_CONNECTION; i++)
    {
        if (conn_status[i].is_occupied == 1 && conn_status[i].connection == connection)
        {
            close(conn_status[i].connection);
            conn_status[i].is_occupied = 0;
            conn_status[i].connection = -1;
            conn_status[i].connection_type = -1;
            conn_status[i].session_id = -1;
            connection_count -= 1;
            return ERROR_OK;
        }
    }
    return ERROR_NOT_OK;
}

int cm_close_all_connections(void)
{
    int i = 0;
    for(i = 0; i< MAX_CONNECTION; i++)
    {
        if (conn_status[i].is_occupied == 1)
        {
            close(conn_status[i].connection);
            conn_status[i].is_occupied = 0;
            conn_status[i].connection = -1;
            conn_status[i].connection_type = -1;
            conn_status[i].session_id = -1;
            connection_count -= 1;
        }
    }
    return ERROR_OK;
}
