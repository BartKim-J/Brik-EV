/**
 * @file brik_api.h
 * @author bato
 * @date 23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */

#ifndef __SOCKET_LISTENER_H_
#define __SOCKET_LISTENER_H_

/*************************************************************
 * @name Socket Listner Module API.
 * @note use 1 thread.
 *
 *////@{

/** @brief Socket Linster Module Init.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_SocketListener_Init(void);

/** @brief Socket Linster Destroy Init.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_SocketListener_Destroy(void);

/*************************************************************@}*/

#endif
