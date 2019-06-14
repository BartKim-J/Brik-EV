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

/** @brief Decoder init
 *
 *  @param packet
 *  @param extradata
 *  @param extra_len
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Init(CodecDataPacket* packet, void* extradata, int extra_len);

/** @brief Decoder uninit
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Uninit(void);

/** @brief decoding packet data
 *
 *  @param packet
 *  @param payload
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Write(AVPacketPacket* packet, void* payload);

/** @brief get decoded frame data
 *
 *  @param frameData
 *
 *  @return ERROR_T
 *
 *  @note
 */
extern ERROR_T MODULE_Decoder_Receive(frame_data_t* frameData);

/*************************************************************@}*/

#endif
