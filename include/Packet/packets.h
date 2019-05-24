/**
 * @file packets.h
 * @author Lex
 * @date
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __PACKETS_H_
#define __PACKETS_H_

// #CONSTANTS
#define PACKET_SIZE   128
#define SIZE_MAX_PAYLOAD_CHUNK  2048

// #packet type mask
#define PACKET_TYPE_CMD_MASK    0x0000FFFF
#define PACKET_TYPE_CMD_VALUE   0x0000C001
#define PACKET_TYPE_VIDEO_MASK    0x0003FFF0
#define PACKET_TYPE_VIDEO_VALUE   0x0001F000
#define PACKET_TYPE_AUDIO_MASK    0x0003FFF0
#define PACKET_TYPE_AUDIO_VALUE   0x0002F000
#define PACKET_TYPE_BACKCHANNEL_MASK    0x0000FFFF
#define PACKET_TYPE_BACKCHANNEL_VALUE   0x0000D001

// #Codec data type
#define PACKET_VIDEO_CODEC   0x0001F001
#define PACKET_AUDIO_CODEC   0x0002F001

// # Video Codecs
#define VIDEO_CODEC_H264   0
#define VIDEO_CODEC_VP9   1
#define VIDEO_CODEC_VP8   2
#define VIDEO_CODEC_HEVC   3

// Audio Codecs
#define AUDIO_CODEC_AAC   0
#define AUDIO_CODEC_VORBIS   1
#define AUDIO_CODEC_AAC_ELD   2

// Packet Type
#define PACKET_VIDEO_PACKET   0x0001F002
#define PACKET_AUDIO_PACKET   0x0002F002

// Cursor packet
#define PACKET_CURSOR   0x0001F003
#define PACKET_CURSOR_POS   0x0001F004

// Back channel packet_type
#define PACKET_BACKCHANNEL   0x0001D001

// Command packet type
#define PACKET_CMD_CONNECT            0x0001C001
#define PACKET_CMD_CONNECT_RESP       0xA001C001
#define PACKET_CMD_DISCONNECT         0x0002C001
#define PACKET_CMD_DISCONNECT_RESP	  0xA002C001
#define PACKET_CMD_PAUSE			  0x0003C001
#define PACKET_CMD_PAUSE_RESP		  0xA003C001
#define PACKET_CMD_RESUME			  0x0004C001
#define PACKET_CMD_RESUME_RESP		  0xA004C001
#define PACKET_CMD_SLEEP			  0x0005C001
#define PACKET_CMD_SLEEP_RESP		  0xA005C001
#define PACKET_CMD_WAKEUP			  0x0006C001
#define PACKET_CMD_WAKEUP_RESP		  0xA006C001
#define PACKET_CMD_DISPMODE			  0x0007C001
#define PACKET_CMD_DISPMODE_RESP	  0xA007C001
#define PACKET_CMD_BCENABLE			  0x0008C001
#define PACKET_CMD_BCENABLE_RESP	  0xA008C001
#define PACKET_CMD_TIMESYNC			  0x0009C001
#define PACKET_CMD_TIMESYNC_RESP	  0xA009C001
#define PACKET_CMD_AVRESET			  0x000AC001
#define PACKET_CMD_AVRESET_RESP		  0xA00AC001
#define PACKET_CMD_MACHINENAME		  0x000BC001
#define PACKET_CMD_MACHINENAME_RESP   0xA00BC001
#define PACKET_CMD_CHECKCONN		  0x000CC001
#define PACKET_CMD_CHECKCONN_RESP	  0xA00CC001
#define PACKET_CMD_WINSTATE			  0x000DC001
#define PACKET_CMD_WINSTATE_RESP	  0xA00DC001
#define PACKET_CMD_WIN_NOTIFY		  0x000EC001
#define PACKET_CMD_WIN_NOTIFY_RESP	  0xA00EC001

// Connection Type
#define CONNECTION_TYPE_CONTROL       0
#define CONNECTION_TYPE_VIDEO         1
#define CONNECTION_TYPE_AUDIO         2
#define CONNECTION_TYPE_BACKCHANNEL   3

// Window state
#define WINDOW_STATE_SINGLE   0
#define WINDOW_STATE_MULTI   1
#define WINDOW_STATE_HIDDEN   2

//structure for packet data

typedef struct {
	int32_t type;
	int32_t payloadSize;
	int64_t sendTime;
	int64_t recvTime;
} PacketHeader;

typedef struct {
	PacketHeader hdr;
	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader)];
} Packet;

typedef struct {
	PacketHeader hdr;
	struct _video {
		int32_t codecType;
		int32_t width;
		int32_t height;
		int32_t framerate;
		int32_t rotation;
		int32_t screenMagnification;
	} video;

	struct _audio {
		int32_t codecType;
		int32_t sampleRate;
		int32_t channels;
		int32_t frameSize;
	} audio;

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(struct _video) - sizeof(struct _audio)];
} CodecDataPacket;

typedef struct {
	PacketHeader hdr;
	struct _avpacket {
		int64_t timestamp;
		int32_t encodingDelay;
	} avpacket;

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(struct _avpacket)];
} AVPacketPacket;

typedef struct {
	PacketHeader hdr;
	struct _cursor {
		int64_t timestamp;

		int32_t visible;

		int32_t hotspotX;
		int32_t hotspotY;

		int32_t width;
		int32_t height;

		int32_t maskBpp;
		int32_t maskBitsSize;

		int32_t colorBpp;
		int32_t colorBitsSize;

		int32_t screenMagnification;
	} cursor;

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(struct _cursor)];
} CursorPacket;


typedef struct {
	PacketHeader hdr;
	struct _cursorPos {
		int32_t positionX;
		int32_t positionY;
	} cursorPos;

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(struct _cursorPos)];
} CursorPositionPacket;

typedef struct {
	PacketHeader hdr;
	struct _backchannel
	{
		int64_t timestamp;
	} backchannel;

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(struct _backchannel)];
} BackchannelPacket;


typedef struct {
	PacketHeader hdr;

	int32_t param[10];		// 4 * 10 = 40
	int64_t l_param[6];		// 8 *  6 = 48

	uint8_t padding[PACKET_SIZE - sizeof(PacketHeader) - sizeof(int32_t) * 10 - sizeof(int64_t) * 6];
} CommandPacket;

#endif
