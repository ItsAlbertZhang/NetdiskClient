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
    MT_DUPCONN,  // 拷贝连接请求
    MT_CS_PWD,   // 短命令(command short): pwd
    MT_CS_LS,    // 短命令(command short): ls
    MT_CS_CD,    // 短命令(command short): cd
    MT_CS_RM,    // 短命令(command short): rm
    MT_CS_MV,    // 短命令(command short): mv
    MT_CS_CP,    // 短命令(command short): cp
    MT_CS_MKDIR, // 短命令(command short): mkdir
    MT_CS_RMDIR, // 短命令(command short): rmdir
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

// pwd 命令请求
int msgsend_cs_pwd(void);
int msgrecv_cs_pwd(void);

// ls 命令请求
int msgsend_cs_ls(void);
int msgrecv_cs_ls(void);

// cd 命令请求
int msgsend_cs_cd(char *cmd);
int msgrecv_cs_cd(void);

// rm 命令请求
int msgsend_cs_rm(char *cmd);
int msgrecv_cs_rm(void);

// mv 命令请求
int msgsend_cs_mv(char *cmd);
int msgrecv_cs_mv(void);

// cp 命令请求
int msgsend_cs_cp(char *cmd);
int msgrecv_cs_cp(void);

// mkdir 命令请求
int msgsend_cs_mkdir(char *cmd);
int msgrecv_cs_mkdir(void);

// rmdir 命令请求
int msgsend_cs_rmdir(char *cmd);
int msgrecv_cs_rmdir(void);

#endif /* __CONNECT_MSG_H__ */