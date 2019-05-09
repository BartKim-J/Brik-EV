/**
 * @file connect_mgmt.h
 * @author Lex
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __CONNECT_MGMT_H_
#define __CONNECT_MGMT_H_

#define MAX_CONNECTION  32

extern int cm_get_connection_count();
extern int cm_add_new_connection(int32_t connection, int32_t connection_type, int32_t session_id);
extern int cm_close_current_connection(int32_t connection);
extern int cm_close_all_connections(void);

#endif
