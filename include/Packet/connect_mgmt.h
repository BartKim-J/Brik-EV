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

#if false // Video Only
#define MAX_CONNECTION  2 //CONTROL + VIDEO ( 2 CONNECTION = 1 SET )
#else    // Video and Audio
#define MAX_CONNECTION  4 //CONTROL + VIDEO + AUDIO + BACKCHANNEL( 4 CONNECTION = 1 SET )
#endif

/*************************************************************
 * @name Video Handler Module
 *
 *////@{
/** @brief
 *
 *  @return count of connections
 *
 *  @note
 */
extern int MODULE_ConnectManager_GetCount(void);

/** @brief
 *
 *  @param
 *  @param
 *  @param
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_Open(int32_t connection, int32_t connection_type, int32_t session_id);

/** @brief
 *
 *  @param
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_Close(int32_t connection);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_CloseAll(void);

/*************************************************************@}*/

#endif
