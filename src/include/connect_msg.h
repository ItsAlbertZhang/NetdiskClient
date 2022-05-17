#ifndef __CONNECT_MSG_H__
#define __CONNECT_MSG_H__

#include "head.h"
#include "main.h"

// 消息类型标志
enum msg_type {
    MT_NULL,    // 编号 0 为空请求
    MT_REQCONF, // 下发验证请求
    MT_REGIST,  // 注册请求
    MT_LOGIN,   // 登录请求
    MT_RECONN,  // 重连请求
    MT_COMM_S,  // 短命令请求
    MT_COMM_L,  // 长命令请求
};

// 循环接收, 避免因网络数据分包导致的错误
size_t recv_n(int connect_fd, void *buf, size_t len, int flags);

// 判断命令类型
int connect_msg_cmdtype(char *cmd);

// 下发验证请求
int msg_reqconf(struct program_stat_t *program_stat);

// 注册请求
int msg_regist(struct program_stat_t *program_stat);

#endif /* __CONNECT_MSG_H__ */