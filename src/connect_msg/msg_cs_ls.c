#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_ls_sendbuf_t {
    char msgtype; // 消息类型
};

struct msg_cs_ls_recvbuf_t {
    char msgtype;   // 消息类型
    int res_len;    // 下一字段的长度
    char res[4096]; // 当前目录下的文件
};

static int msg_cs_ls_send(int connect_fd, const struct msg_cs_ls_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_ls_recv(int connect_fd, struct msg_cs_ls_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_ls_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->res_len, sizeof(recvbuf->res_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->res, recvbuf->res_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_ls(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_ls_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_LS;

    // 发送消息
    ret = msg_cs_ls_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_ls_send");

    return 0;
}

int msgrecv_cs_ls(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_ls_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_ls_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_ls_recv");

    // 输出
    logging(LOG_OUTPUT, recvbuf.res);
    // printf("%s", recvbuf.res);

    return 0;
}