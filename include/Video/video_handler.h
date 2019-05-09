/**
 * @file video_handler.h
 * @author Ben
 * @date
 * @brief
 *
 * @bug sw decoder not working maybe sdl option.
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 *
 * @defgroup VIDEO_HANDLER Vider Handler
 * @ingroup MODULE
 * @brief Video Handler Module Functions.
 * @{
 */
#ifndef __VIDEO_HANDLER_H_
#define __VIDEO_HANDLER_H_

typedef enum vh_msg_type {
    VH_MSG_TYPE_VIDEO_CODEC      = 1,
    VH_MSG_TYPE_VIDEO_DATA       = 2,
    VH_MSG_TYPE_VIDEO_CONNECT    = 3,
    VH_MSG_TYPE_VIDEO_DISCONNECT = 4,
    VH_MSG_TYPE_VIDEO_NONE,
} VH_MSG_T;

typedef struct video_data_msg {
    void* packet;
    void* payload;
} video_data_msg_t;

/*************************************************************
 * @name Video Handler Module
 *
 *////@{
/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_VideoHandler_Init(void);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_VideoHandler_Destroy(void);

/** @brief
 *
 *  @param msg
 *  @param message_type
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_VideoHandler_SendMessage(void* msg, VH_MSG_T message_type);

/*************************************************************@}*/
#endif /* VIDEO_HANDLER */
/**@}*/
