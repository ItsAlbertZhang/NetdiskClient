#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_login_sendbuf_t {
    char msgtype;          // 消息类型
    int username_len;      // 下一字段的长度
    char username[30];     // 用户名
    int pwd_ciprsa_len;    // 下一字段的长度
    char pwd_ciprsa[1024]; // 密码密文
};

struct msg_login_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0
#define USERNAME_NOT_EXIST -1
#define PWD_ERROR -2

static int msg_login_send(int connect_fd, const struct msg_login_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->username_len, sizeof(sendbuf->username_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->username, sendbuf->username_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->pwd_ciprsa_len, sizeof(sendbuf->pwd_ciprsa_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->pwd_ciprsa, sendbuf->pwd_ciprsa_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_login_recv(int connect_fd, struct msg_login_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_login_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msgsend_login(const char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_login_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_LOGIN;

    char pwd_plaintext[64] = {0};

    if (sscanf(cmd, "%*s%s%s", sendbuf.username, pwd_plaintext) <= 0 || strlen(pwd_plaintext) < 3 || strlen(pwd_plaintext) > 30) {
        logging(LOG_ERROR, "格式错误! 请输入 \"login 用户名 密码\" 以登录.");
        return -1;
    }

    sendbuf.username_len = strlen(sendbuf.username);

    // 将密码与 token 第二部分异或
    for (int i = 0; i < strlen(pwd_plaintext); i++) {
        pwd_plaintext[i] = pwd_plaintext[i] ^ program_stat->token2nd[i];
    }
    // 对异或后的密码进行 rsa 加密
    sendbuf.pwd_ciprsa_len = rsa_encrypt(pwd_plaintext, sendbuf.pwd_ciprsa, program_stat->serverpub_rsa, PUBKEY);
    RET_CHECK_BLACKLIST(-1, sendbuf.pwd_ciprsa_len, "rsa_encrypt");
    // 销毁密码明文, 确保安全
    bzero(pwd_plaintext, sizeof(pwd_plaintext));
    // 向服务端发送登录信息
    ret = msg_login_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_login_send");

    return 0;
}

int msgrecv_login(void) {
    int ret = 0;

    // 准备资源
    struct msg_login_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 获取服务端的回复信息.
    ret = msg_login_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_login_recv");

    if (APPROVE == recvbuf.approve) {
        logging(LOG_INFO, "登录成功.");
        ret = APPROVE;
    } else if (USERNAME_NOT_EXIST == recvbuf.approve) {
        logging(LOG_WARN, "用户名不存在.");
        ret = USERNAME_NOT_EXIST;
    } else if (PWD_ERROR == recvbuf.approve) {
        logging(LOG_WARN, "密码错误.");
        ret = PWD_ERROR;
    }

    return ret;
}