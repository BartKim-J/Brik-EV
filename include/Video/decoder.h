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

extern ERROR_T MODULE_Decoder_Init(CodecDataPacket* packet, void* extradata, int extra_len);

extern ERROR_T MODULE_Decoder_Write(AVPacketPacket* packet, void* payload);
extern ERROR_T MODULE_Decoder_Receive(AVFrame *frame, AVFrame *hw_frame, int frameQueue);

#endif
