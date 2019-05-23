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

/*************************************************************
 * @name Video Handler Module
 *
 *////@{
/** @brief
 *
 *  @return count
 *
 *  @note
 */
extern int MODULE_ConnectManager_GetCount();

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_ConnectManager_Open(int32_t connection, int32_t connection_type, int32_t session_id);

/** @brief
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
extern ERROR_T MODULE_ConnectManager_CloseAll();

/*************************************************************@}*/

#endif
