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

#define MAX_CONNECTION  8//CONTROL & VIDEO CHANNEL ( 2 CONNECTION = 1 SET )

extern int MODULE_ConnectManager_GetCount();
extern int MODULE_ConnectManager_Open(int32_t connection, int32_t connection_type, int32_t session_id);
extern int MODULE_ConnectManager_Close(int32_t connection);
extern int MODULE_ConnectManager_CloseAll();

#endif
