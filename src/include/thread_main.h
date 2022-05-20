#ifndef __THREAD_MAIN_H__
#define __THREAD_MAIN_H__

#include "head.h"
#include "main.h"

extern int epfd;

// 消息处理函数
int connect_msg_handle(void);

// 初始化连接
int connect_init(const char *config_dir);

#endif /* __THREAD_MAIN_H__ */