#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_rm_sendbuf_t {
    char msgtype;   // 消息类型
    char rmtype;    // 删除类型
    int dir_len;    // 下一字段的长度
    char dir[1024]; // 当前目录下的文件
};

struct msg_cs_rm_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

#define RM_NULL 0
#define RM_R 1

static int msg_cs_rm_send(int connect_fd, const struct msg_cs_rm_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->rmtype, sizeof(sendbuf->rmtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->dir_len, sizeof(sendbuf->dir_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->dir, sendbuf->dir_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_rm_recv(int connect_fd, struct msg_cs_rm_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_rm_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_rm(char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_cs_rm_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_RM;

    char arg1[1024] = {0};
    char arg2[1024] = {0};
    ret = sscanf(cmd, "%*s%s%s", arg1, arg2);
    if(1 == ret) {
        strcpy(sendbuf.dir, arg1);
        sendbuf.rmtype = RM_NULL;
    }
    if(2 == ret && 0 == strcmp(arg1, "-r")) {
        strcpy(sendbuf.dir, arg2);
        sendbuf.rmtype = RM_R;
    }
    if(2 == ret && 0 == strcmp(arg2, "-r")) {
        strcpy(sendbuf.dir, arg1);
        sendbuf.rmtype = RM_R;
    }
    sendbuf.dir_len = strlen(sendbuf.dir);

    // 发送消息
    ret = msg_cs_rm_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_rm_send");

    return 0;
}

int msgrecv_cs_rm(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_rm_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_rm_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_rm_recv");

    if (DISAPPROVE == recvbuf.approve) {
        logging(LOG_ERROR, "文件路径有误, 或试图使用非递归方式删除一个非空文件夹.");
    }

    return ret;
}