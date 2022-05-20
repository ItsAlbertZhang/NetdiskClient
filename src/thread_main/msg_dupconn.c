#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_main.h"

struct msg_dupconn_sendbuf_t {
    char msgtype;                // 消息类型
    int token_len;               // 下一字段的长度
    char token_ciphertext[1024]; // 密码密文
};

struct msg_dupconn_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

static int msg_dupconn_send(int connect_fd, const struct msg_dupconn_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->token_len, sizeof(sendbuf->token_len) + sendbuf->token_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_dupconn_recv(int connect_fd, struct msg_dupconn_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_dupconn_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->msgtype, sizeof(recvbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msg_dupconn(const char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_dupconn_sendbuf_t sendbuf;
    struct msg_dupconn_recvbuf_t recvbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    bzero(&recvbuf, sizeof(recvbuf));
    sendbuf.msgtype = MT_DUPCONN;

    // 建立新连接
    int connect_fd = connect_init(program_stat->config_dir);
    RET_CHECK_BLACKLIST(-1, connect_fd, "connect_init");
    // 如果是重连请求, 则将其添加至 epoll 监听
    if (-1 == program_stat->connect_fd) {
        program_stat->connect_fd = connect_fd;
        ret = epoll_add(connect_fd);
        RET_CHECK_BLACKLIST(-1, ret, "epoll_add");
    }

    // 加密
    sendbuf.token_len = rsa_encrypt(program_stat->token, sendbuf.token_ciphertext, program_stat->serverpub_rsa, PUBKEY);

    // 向服务端发送信息.
    ret = msg_dupconn_send(connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_dupconn_send");
    // 获取服务端的回复信息.
    ret = msg_dupconn_recv(connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_dupconn_send");

    if (APPROVE == recvbuf.approve) {
        logging(LOG_INFO, "拷贝连接成功.");
        ret = 0;
    } else {
        logging(LOG_WARN, "拷贝连接失败.");
        return -1;
    }

    // 返回新连接文件描述符
    return connect_fd;
}