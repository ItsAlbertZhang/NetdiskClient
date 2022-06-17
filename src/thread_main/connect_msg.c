#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_main.h"

int connect_sendmsg_handle(void) {
    int ret = 0;
    char cmd[1024] = {0};
    read(STDIN_FILENO, cmd, sizeof(cmd));
    int cmdtype = connect_sendmsg_cmdtype(cmd);
    switch (cmdtype) {
    case MT_NULL:
        logging(LOG_WARN, "无效的命令.");
        break;
    case MT_CONNINIT:
        logging(LOG_INFO, "执行下发验证请求.");
        break;
    case MT_REGIST:
        logging(LOG_INFO, "执行注册请求.");
        ret = msgsend_regist(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_regist 执行出错.");
        }
        break;
    case MT_LOGIN:
        logging(LOG_INFO, "执行登录请求.");
        ret = msgsend_login(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_login 执行出错.");
        }
        break;
    case MT_DUPCONN:
        logging(LOG_INFO, "执行拷贝连接请求.");
        ret = msgsend_dupconn();
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_dupconn 执行出错.");
        }
        break;
    case MT_CS_PWD:
        logging(LOG_INFO, "执行 pwd 命令请求.");
        ret = msgsend_cs_pwd();
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_pwd 执行出错.");
        }
        break;
    case MT_CS_LS:
        logging(LOG_INFO, "执行 ls 命令请求.");
        ret = msgsend_cs_ls();
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_ls 执行出错.");
        }
        break;
    case MT_CS_CD:
        logging(LOG_INFO, "执行 cd 命令请求.");
        ret = msgsend_cs_cd(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_cd 执行出错.");
        }
        break;
    case MT_CS_RM:
        logging(LOG_INFO, "执行 rm 命令请求.");
        ret = msgsend_cs_rm(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_rm 执行出错.");
        }
        break;
    case MT_CS_MV:
        logging(LOG_INFO, "执行 mv 命令请求.");
        ret = msgsend_cs_mv(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_mv 执行出错.");
        }
        break;
    case MT_CS_CP:
        logging(LOG_INFO, "执行 cp 命令请求.");
        ret = msgsend_cs_cp(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_cp 执行出错.");
        }
        break;
    case MT_CS_MKDIR:
        logging(LOG_INFO, "执行 mkdir 命令请求.");
        ret = msgsend_cs_mkdir(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_mkdir 执行出错.");
        }
        break;
    case MT_CS_RMDIR:
        logging(LOG_INFO, "执行 rmdir 命令请求.");
        ret = msgsend_cs_rmdir(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msgsend_cs_rmdir 执行出错.");
        }
        break;
    case MT_CL_S2C:
        logging(LOG_INFO, "执行 download 命令请求.");
        ret = msg_cl_s2c(cmd);
        if (-1 == ret) {
            logging(LOG_ERROR, "msg_cl_s2c 执行出错.");
        }
        break;
    default:
        break;
    }

    return 0;
}

int connect_recvmsg_handle(void) {
    int ret = 0;
    char msgtype; // 消息类型
    // 接受来自服务端的消息类型标志
    ret = recv_n(program_stat->connect_fd, &msgtype, 1, 0);
    if (0 == ret) {
        // 对方已断开连接
        ret = epoll_del(program_stat->connect_fd); // 将 socket_fd 添加至 epoll 监听
        RET_CHECK_BLACKLIST(-1, ret, "epoll_del");
        close(program_stat->connect_fd);
        program_stat->connect_fd = -1;
        logging(LOG_DEBUG, "服务端连接已断开.");
    } else {
        switch (msgtype) {
        case MT_CONNINIT:
            // logging(LOG_INFO, "收到下发验证请求的回复.");
            msgrecv_conninit();
            break;
        case MT_REGIST:
            // logging(LOG_INFO, "收到注册请求的回复.");
            msgrecv_regist();
            break;
        case MT_LOGIN:
            // logging(LOG_INFO, "收到登录请求的回复.");
            msgrecv_login();
            break;
        case MT_DUPCONN:
            // logging(LOG_INFO, "收到拷贝连接请求的回复.");
            msgrecv_dupconn();
            break;
        case MT_CS_PWD:
            // logging(LOG_INFO, "收到 pwd 命令请求的回复.");
            msgrecv_cs_pwd();
            break;
        case MT_CS_LS:
            // logging(LOG_INFO, "收到 ls 命令请求的回复.");
            msgrecv_cs_ls();
            break;
        case MT_CS_CD:
            // logging(LOG_INFO, "收到 cd 命令请求的回复.");
            msgrecv_cs_cd();
            break;
        case MT_CS_RM:
            // logging(LOG_INFO, "收到 rm 命令请求的回复.");
            msgrecv_cs_rm();
            break;
        case MT_CS_MV:
            // logging(LOG_INFO, "收到 mv 命令请求的回复.");
            msgrecv_cs_mv();
            break;
        case MT_CS_CP:
            // logging(LOG_INFO, "收到 cp 命令请求的回复.");
            msgrecv_cs_cp();
            break;
        case MT_CS_MKDIR:
            // logging(LOG_INFO, "收到 mkdir 命令请求的回复.");
            msgrecv_cs_mkdir();
            break;
        case MT_CS_RMDIR:
            // logging(LOG_INFO, "收到 rmdir 命令请求的回复.");
            msgrecv_cs_rmdir();
            break;
        case MT_CL_S2C:
            // logging(LOG_INFO, "收到 download 命令请求的回复.");
            // msgrecv_cs_rmdir();
            break;
        default:
            break;
        }
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
            int dupconn_ret = msgsend_dupconn();
            if (-1 == dupconn_ret) {
                logging(LOG_ERROR, "msgsend_dupconn 执行出错.");
            }
        }
        // 传入的 connect_fd 为 -1, 说明调用 send_n 的函数欲发送数据的对端一定是主线程连接(而非子线程连接).
        // 因此, 可以自行从 program_stat 中拿取 connect_fd.
        connect_fd = program_stat->connect_fd;
    }

    while (recved_len < len) {
        ret = send(connect_fd, p + recved_len, len - recved_len, flags);
        RET_CHECK_BLACKLIST(-1, ret, "send");
        recved_len += ret;
    }
}

// 判断命令类型
int connect_sendmsg_cmdtype(char *cmd) {
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
    if (!strncmp(cmd, "pwd", strlen("pwd"))) {
        return MT_CS_PWD;
    }
    if (!strncmp(cmd, "ls", strlen("ls"))) {
        return MT_CS_LS;
    }
    if (!strncmp(cmd, "cd", strlen("cd"))) {
        return MT_CS_CD;
    }
    if (!strncmp(cmd, "rmdir", strlen("rmdir"))) {
        return MT_CS_RMDIR;
    }
    if (!strncmp(cmd, "rm", strlen("rm"))) {
        return MT_CS_RM;
    }
    if (!strncmp(cmd, "mv", strlen("mv"))) {
        return MT_CS_MV;
    }
    if (!strncmp(cmd, "cp", strlen("cp"))) {
        return MT_CS_CP;
    }
    if (!strncmp(cmd, "mkdir", strlen("mkdir"))) {
        return MT_CS_MKDIR;
    }
    if (!strncmp(cmd, "download", strlen("download"))) {
        return MT_CL_S2C;
    }

    return 0;
}