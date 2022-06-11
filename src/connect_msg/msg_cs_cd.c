#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_cd_sendbuf_t {
    char msgtype;   // 消息类型
    int dir_len;    // 下一字段的长度
    char dir[1024]; // 当前目录下的文件
};

struct msg_cs_cd_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

static int msg_cs_cd_send(int connect_fd, const struct msg_cs_cd_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->dir_len, sizeof(sendbuf->dir_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->dir, sendbuf->dir_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_cd_recv(int connect_fd, struct msg_cs_cd_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_cd_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_cd(char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_cs_cd_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_CD;

    sscanf(cmd, "%*s%s", sendbuf.dir);
    sendbuf.dir_len = strlen(sendbuf.dir);

    // 发送消息
    ret = msg_cs_cd_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_cd_send");

    return 0;
}

int msgrecv_cs_cd(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_cd_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_cd_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_cd_recv");

    if (DISAPPROVE == recvbuf.approve) {
        logging(LOG_ERROR, "目录有误, 已进入无误部分的最后一级目录.");
    }

    return ret;
}