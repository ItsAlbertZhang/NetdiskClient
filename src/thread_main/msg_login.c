#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_login_sendbuf_t {
    char msgtype;              // 消息类型
    int username_len;          // 下一字段的长度
    char username[30];         // 用户名
    int pwd_len;               // 下一字段的长度
    char pwd_ciphertext[1024]; // 密码密文
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
    ret = send_n(connect_fd, &sendbuf->username_len, sizeof(sendbuf->username_len) + sendbuf->username_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->pwd_len, sizeof(sendbuf->pwd_len) + sendbuf->pwd_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_login_recv(int connect_fd, struct msg_login_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_login_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->msgtype, sizeof(recvbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msg_login(const char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_login_sendbuf_t sendbuf;
    struct msg_login_recvbuf_t recvbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    bzero(&recvbuf, sizeof(recvbuf));
    sendbuf.msgtype = MT_LOGIN;

    int pwd_check = DISAPPROVE;
    char pwd_plaintext[64] = {0};

    ret = sscanf(cmd, "%*s%s%s", sendbuf.username, pwd_plaintext);
    if (ret > 0) {
        sendbuf.username_len = strlen(sendbuf.username);
        if (strlen(pwd_plaintext) >= 3 || strlen(pwd_plaintext) <= 30) {
            pwd_check = APPROVE;
        }
    } else {
        // 获取用户名
        printf("[LOGIN] 请输入你的用户名: ");
        fflush(stdout);
        scanf("%s", sendbuf.username);
        sendbuf.username_len = strlen(sendbuf.username);
    }

    while (DISAPPROVE == pwd_check) {
        char *pwd_plaintext_p = getpass("[LOGIN] 请输入密码(无回显), 可输入 exit 退出:");
        // 如果用户希望中断登录 (在 getpass 函数中无法通过 SIGINT 信号退出), 则跳出循环. 注意到此时会直接跳转到函数尾注册失败的情形.
        if (0 == strcmp(pwd_plaintext_p, "exit")) {
            break;
        }
        // 如果输入的密码不符合规范, 则重新进行循环.
        if (strlen(pwd_plaintext_p) < 3 || strlen(pwd_plaintext_p) > 30) {
            printf("[ERROR] 密码长度过短或过长, 请确保密码为 3 ~ 30 个字符.\n");
            continue;
        }
        strcpy(pwd_plaintext, pwd_plaintext_p);
        pwd_check = APPROVE;
    }

    if (APPROVE == pwd_check) {
        // 对密码进行确认码异或
        for (int i = 0; i < strlen(pwd_plaintext); i++) {
            pwd_plaintext[i] = pwd_plaintext[i] ^ program_stat->confirm[i];
        }
        // 对异或后的密码进行 rsa 加密
        sendbuf.pwd_len = rsa_encrypt(pwd_plaintext, sendbuf.pwd_ciphertext, program_stat->serverpub_rsa, PUBKEY);
        RET_CHECK_BLACKLIST(-1, sendbuf.pwd_len, "rsa_encrypt");
        // 销毁密码明文, 确保安全
        bzero(pwd_plaintext, sizeof(pwd_plaintext));
        // 向服务端发送登录信息.
        ret = msg_login_send(program_stat->connect_fd, &sendbuf);
        RET_CHECK_BLACKLIST(-1, ret, "msg_login_send");
        // 获取服务端的回复信息.
        ret = msg_login_recv(program_stat->connect_fd, &recvbuf);
        RET_CHECK_BLACKLIST(-1, ret, "msg_login_send");
    }

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