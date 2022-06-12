#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_cp_sendbuf_t {
    char msgtype;        // 消息类型
    int cpsource_len;    // 下一字段的长度
    char cpsource[1024]; // 原文件或目录
    int cpdir_len;       // 下一字段的长度
    char cpdir[1024];    // 目标路径
    int rename_len;      // 下一字段的长度
    char rename[64];     // 重命名
};

struct msg_cs_cp_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

static int msg_cs_cp_send(int connect_fd, const struct msg_cs_cp_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->cpsource_len, sizeof(sendbuf->cpsource_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->cpsource, sendbuf->cpsource_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->cpdir_len, sizeof(sendbuf->cpdir_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->cpdir, sendbuf->cpdir_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->rename_len, sizeof(sendbuf->rename_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->rename, sendbuf->rename_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_cp_recv(int connect_fd, struct msg_cs_cp_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_cp_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_cp(char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_cs_cp_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_CP;

    sscanf(cmd, "%*s%s%s", sendbuf.cpsource, sendbuf.cpdir);
    sendbuf.cpsource_len = strlen(sendbuf.cpsource);
    sendbuf.cpdir_len = strlen(sendbuf.cpdir);
    // 从路径中分离路径与重命名
    char *plast = sendbuf.cpdir + sendbuf.cpdir_len - 1;
    while ('/' != *plast && plast >= sendbuf.cpdir) {
        plast--;
    }
    plast++;
    if (*plast) {
        strcpy(sendbuf.rename, plast);
    }
    sendbuf.rename_len = strlen(sendbuf.rename);
    *plast = 0;

    // 发送消息
    ret = msg_cs_cp_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_cp_send");

    return 0;
}

int msgrecv_cs_cp(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_cp_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_cp_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_cp_recv");

    if (DISAPPROVE == recvbuf.approve) {
        logging(LOG_ERROR, "出现错误, 命令未执行.");
    }

    return ret;
}