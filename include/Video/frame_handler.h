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

typedef enum fh_msg_type {
    FH_MSG_TYPE_VIDEO_CODEC       = 1,
    FH_MSG_TYPE_VIDEO_UPDATE      = 2,
    FH_MSG_TYPE_VIDEO_STOP        = 3,
    FH_MSG_TYPE_VIDEO_NONE,
} FH_MSG_T;

typedef struct frame_data {
    bool isOccupied;

    AVFrame *frame;
    AVFrame *hw_frame;

    AVFrame *target_frame; // Just for pointing frame.

    void* packet;
    void* payload;

} frame_data_t;

typedef struct frame_data_msg {
    void* frameData;
} frame_data_msg_t;

/*************************************************************
 * @name Video Handler Module
 *
 *////@{
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
extern ERROR_T MODULE_FrameHandler_Init(void);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_Destroy(void);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_FrameHandler_Cleanup(void);

/** @brief
 *
 *  @return frame_data_t pointer
 *
 *  @note
 */
extern frame_data_t* Module_FrameHandler_BufferAlloc(AVPacketPacket* packet, void* payload);

/** @brief
 *
 *  @return frame_data_t pointer
 *
 *  @note
 */
extern ERROR_T Module_FrameHandler_BufferFree(frame_data_t* frameData);

/** @brief
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
#endif /* VIDEO_HANDLER */
/**@}*/
