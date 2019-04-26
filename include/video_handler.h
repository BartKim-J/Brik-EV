#ifndef __VIDEO_HANDLER_H_
#define __VIDEO_HANDLER_H_

#define VH_MSG_TYPE_VIDEO_CODEC 1
#define VH_MSG_TYPE_VIDEO_DATA  2
#define VH_MSG_TYPE_VIDEO_STOP  3

typedef struct video_data_msg{
    void* packet;
    void* payload;
}video_data_msg_t;

extern int vh_init_handler_thread(void);
extern int vh_send_message(void* msg, int message_type);
#endif
