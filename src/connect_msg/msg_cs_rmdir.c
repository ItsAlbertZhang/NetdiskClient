#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_rmdir_sendbuf_t {
    char msgtype;       // 消息类型
    int dirname_len;    // 下一字段的长度
    char dirname[64]; // 原文件或目录
};

struct msg_cs_rmdir_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

static int msg_cs_rmdir_send(int connect_fd, const struct msg_cs_rmdir_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->dirname_len, sizeof(sendbuf->dirname_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->dirname, sendbuf->dirname_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_rmdir_recv(int connect_fd, struct msg_cs_rmdir_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_rmdir_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_rmdir(char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_cs_rmdir_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_RMDIR;

    sscanf(cmd, "%*s%s", sendbuf.dirname);
    sendbuf.dirname_len = strlen(sendbuf.dirname);

    // 发送消息
    ret = msg_cs_rmdir_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_rmdir_send");

    return 0;
}

int msgrecv_cs_rmdir(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_rmdir_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_rmdir_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_rmdir_recv");

    if (DISAPPROVE == recvbuf.approve) {
        logging(LOG_ERROR, "目录不存在或目录非空, 命令未执行.");
    }

    return ret;
}