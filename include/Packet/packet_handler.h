/**
 * @file packet_handler.h
 * @author Ben
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __PACKET_HANDLER_H_
#define __PACKET_HANDLER_H_

/*************************************************************
 * @name Packet Handler Module
 *
 *////@{
/** @brief
 *
 *  @param socket index
 *  @param socket_fd
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_PacketHandler_Init(int socket_fd);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_PacketHandler_Destroy(void);

/*************************************************************@}*/
#endif
