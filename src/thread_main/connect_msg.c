#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_main.h"

int connect_msg_handle(void) {
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
        ret = msg_login(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_login 执行出错.");
        }
        break;
    case MT_REGIST:
        logging(LOG_INFO, "执行注册请求.");
        ret = msg_regist(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_regist 执行出错.");
        }
        break;
    case MT_DUPCONN:
        logging(LOG_INFO, "执行拷贝连接请求.");
        ret = msg_dupconn(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_dupconn 执行出错.");
        }
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

// 循环发送, 并在发送失败时自动执行重连
size_t send_n(int connect_fd, const void *buf, size_t len, int flags) {
    int ret = 0;
    char *p = (char *)buf;
    size_t recved_len = 0;
    // 传入参数不合法时的处理
    if (-1 == connect_fd) {
        // 在某些情况下, 即使连接已经恢复, 传入的 connect_fd 也有可能为 -1.
        // 比如某个函数连续调用了两次 send_n, 而其 connect_fd 又是从上一级函数获取的.
        // 则第二次调用 send_n 时, 连接已经恢复, 但传入的 connect_fd 依旧为 -1.
        // 只有当连接真的没有恢复时, 才执行 dupconn.
        if (-1 == program_stat->connect_fd) {
            int dupconn_ret = msg_dupconn(NULL);
            if (-1 == dupconn_ret) {
                logging(LOG_ERROR, "msg_dupconn 执行出错.");
            }
        }
        // 传入的 connect_fd 为 -1, 说明调用 send_n 的函数欲发送数据的对端一定是主线程连接(而非子线程连接).
        // 因此, 可以自行从 program_stat 中拿取 connect_fd.
        connect_fd = program_stat->connect_fd;
    }

    while (recved_len < len) {
        ret = send(connect_fd, p + recved_len, len - recved_len, flags);
        // 进行发送时发现连接断开
        if (ret < 1) {
            int dupconn_ret = msg_dupconn(NULL);
            if (-1 == dupconn_ret) {
                logging(LOG_ERROR, "msg_dupconn 执行出错.");
            }
        }
        recved_len += ret;
    }
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
    if (!strncmp(cmd, "dup", strlen("dup"))) {
        return MT_DUPCONN;
    }

    return 0;
}