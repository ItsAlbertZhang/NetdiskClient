#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_main.h"

int connect_msg_handle(struct program_stat_t *program_stat) {
    int ret = 0;
    char cmd[1024] = {0};
    read(STDIN_FILENO, cmd, sizeof(cmd));
    int cmdtype = connect_msg_cmdtype(cmd);
    switch (cmdtype) {
    case MT_NULL:
        logging(LOG_WARN, "无效的命令.");
        break;
    case MT_REQCONF:
        logging(LOG_INFO, "执行下发验证请求.");
        break;
    case MT_LOGIN:
        logging(LOG_INFO, "执行登录请求.");
        ret = msg_login(program_stat, cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_login 执行出错.");
        }
        break;
    case MT_REGIST:
        logging(LOG_INFO, "执行注册请求.");
        ret = msg_regist(program_stat, cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_regist 执行出错.");
        }
        break;
    case MT_RECONN:
        logging(LOG_INFO, "执行连接请求.");
        break;
    case MT_COMM_S:
        logging(LOG_INFO, "执行短命令请求.");
        break;
    case MT_COMM_L:
        logging(LOG_INFO, "执行长命令请求.");
        break;
    default:
        break;
    }

    return 0;
}

// 循环接收, 避免因网络数据分包导致的错误
size_t recv_n(int connect_fd, void *buf, size_t len, int flags) {
    int ret = 0;
    char *p = (char *)buf;
    size_t recved_len = 0;
    while (recved_len < len) {
        ret = recv(connect_fd, p + recved_len, len - recved_len, flags);
        RET_CHECK_BLACKLIST(-1, ret, "recv");
        if (0 == ret) {
            return 0; // 当对端断开时, 返回 0.
        }
        recved_len += ret;
    }
    return recved_len; // 正常情况下, 返回接收到的字节数.
}

// 判断命令类型
int connect_msg_cmdtype(char *cmd) {
    int ret = 0;

    if (!strncmp(cmd, "regist", strlen("regist"))) {
        return MT_REGIST;
    }
    if (!strncmp(cmd, "login", strlen("login"))) {
        return MT_LOGIN;
    }

    return 0;
}