/**
 * @file decoder.h
 * @author Ben
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __DECODER_H_
#define __DECODER_H_

/*************************************************************
 * @name Decoder Module
 *
 *////@{
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
extern ERROR_T MODULE_Decoder_Init(CodecDataPacket* packet, void* extradata, int extra_len);

/** @brief
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Uninit(void);

/** @brief
 *
 *  @param
 *  @param
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Write(AVPacketPacket* packet, void* payload);

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
extern ERROR_T MODULE_Decoder_Receive(frame_data_t* frameData);

/*************************************************************@}*/

#endif
