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

#if true // Video Only
#define MAX_CONNECTION  2 //CONTROL + VIDEO ( 2 CONNECTION = 1 SET )
#else    // Video and Audio
#define MAX_CONNECTION  4 //CONTROL + VIDEO + AUDIO + BACKCHANNEL( 4 CONNECTION = 1 SET )
#endif

/*************************************************************
 * @name Video Handler Module
 *
 *////@{

/** @brief It will be return connected client count.
 *
 *  @return count of connections
 *
 *  @note
 */
extern int MODULE_ConnectManager_GetCount(void);

/** @brief open connection by paramter
 *
 *  @param connection_client
 *  @param connection_type
 *  @param session_id
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_Open(int32_t connection_client, int32_t connection_type, int32_t session_id);

/** @brief close connection by connection_client
 *
 *  @param connection_client
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_Close(int32_t connection_client);

/** @brief Close all connected client.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_CloseAll(void);

/*************************************************************@}*/

#endif
