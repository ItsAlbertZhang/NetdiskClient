#ifndef __CONNECT_MSG_H__
#define __CONNECT_MSG_H__

#include "head.h"
#include "main.h"

// 消息类型标志
enum msg_type {
    MT_NULL,     // 编号 0 为空请求
    MT_CONNINIT, // 下发验证请求
    MT_REGIST,   // 注册请求
    MT_LOGIN,    // 登录请求
    MT_DUPCONN,  // 重连请求
    MT_COMM_S,   // 短命令请求
    MT_COMM_L,   // 长命令请求
};

// 消息处理函数
int connect_sendmsg_handle(void);
int connect_recvmsg_handle(void);

// 循环接收, 避免因网络数据分包导致的错误
size_t recv_n(int connect_fd, void *buf, size_t len, int flags);

// 循环发送, 并在发送失败时自动执行重连
size_t send_n(int connect_fd, const void *buf, size_t len, int flags);

// 判断命令类型
int connect_sendmsg_cmdtype(char *cmd);

// 下发验证请求
int msgsend_conninit(void);
int msgrecv_conninit(void);

// 注册请求
int msgsend_regist(const char *cmd);
int msgrecv_regist(void);

// 登录请求
int msgsend_login(const char *cmd);
int msgrecv_login(void);

// 拷贝连接请求
int msgsend_dupconn(void);
int msgrecv_dupconn(int connect_fd);

#endif /* __CONNECT_MSG_H__ */