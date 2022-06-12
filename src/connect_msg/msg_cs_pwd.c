#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_cs_pwd_sendbuf_t {
    char msgtype; // 消息类型
};

struct msg_cs_pwd_recvbuf_t {
    char msgtype;   // 消息类型
    int pwd_len;    // 下一字段的长度
    char pwd[1024]; // 当前工作目录
};

static int msg_cs_pwd_send(int connect_fd, const struct msg_cs_pwd_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cs_pwd_recv(int connect_fd, struct msg_cs_pwd_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cs_pwd_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->pwd_len, sizeof(recvbuf->pwd_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->pwd, recvbuf->pwd_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_cs_pwd(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_pwd_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CS_PWD;

    // 发送消息
    ret = msg_cs_pwd_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_pwd_send");

    return 0;
}

int msgrecv_cs_pwd(void) {
    int ret = 0;

    // 准备资源
    struct msg_cs_pwd_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息
    ret = msg_cs_pwd_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cs_pwd_recv");

    // 输出
    logging(LOG_OUTPUT, recvbuf.pwd);
    // printf("%s\n", recvbuf.pwd);

    return 0;
}