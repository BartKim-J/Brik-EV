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
#ifndef __FRAME_HANDLER_H_
#define __FRAME_HANDLER_H_

/*************************************************************
 * @name Frame Handler Strucuture
 *
 *////@{

typedef enum fh_msg_type {
    FH_MSG_TYPE_VIDEO_START       = 1,
    FH_MSG_TYPE_VIDEO_STOP        = 2,
    FH_MSG_TYPE_VIDEO_UPDATE      = 3,
    FH_MSG_TYPE_VIDEO_NONE,
} FH_MSG_T;

typedef struct frame_data {
    bool isOccupied;

    AVFrame *frame;        /* software frame */
    AVFrame *hw_frame;     /* hardware frame. ONLY USED WITH Hardware decoder */

    AVFrame *target_frame; /* decoded frame */

    void* packet;          /*  */
    void* payload;         /*  */

} frame_data_t;

typedef struct frame_data_msg {
    void* frameData;       /* frameData pointer */
} frame_data_msg_t;

/*************************************************************@}*/

/*************************************************************
 * @name Frame Handler Module
 *
 *////@{
/** @brief frame handler init.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_Init(void);

/** @brief frame handler destroy.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_Destroy(void);

/** @brief frame handler buffer cleanup.
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_Cleanup(void);

/** @brief frame handler buffer push
 *
 *  @return frame_data_t pointer
 *
 *  @note
 */
extern frame_data_t* Module_FrameHandler_BufferAlloc(AVPacketPacket* packet, void* payload);

/** @brief frame handler buffer pop
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T Module_FrameHandler_BufferFree(frame_data_t* frameData);

/** @brief
 *
 *  @return frame_data_t pointer
 *
 *  @warnning change process it take so many time.
 */
extern void MODULE_FrameHandler_MutexLock(void);

/** @brief
 *
 *  @return void
 *
 *  @note
 *  @warnning change process it take so many time.
 */
extern void MODULE_FrameHandler_MutexUnlock(void);

/** @brief get frame buffer delayed.
 *
 *  @return  FPD(frame buffer per delayed)
 *
 *  @note
 */
extern long MODULE_FrameHandler_FPD(void);

/** @brief send message to frame buffer handler.
 *
 *  @param msg
 *  @param message_type
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_SendMessage(void* msg, FH_MSG_T message_type);

/*************************************************************@}*/
#endif /* FRAME_HANDLER */
/**@}*/
