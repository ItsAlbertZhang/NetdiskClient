#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_regist_sendbuf_t {
    char msgtype;          // 消息类型
    int username_len;      // 下一字段的长度
    char username[30];     // 用户名
    int pwd_ciprsa_len;    // 下一字段的长度
    char pwd_ciprsa[1024]; // 密码密文
};

struct msg_regist_recvbuf_t {
    char msgtype; // 消息类型
    char approve; // 批准标志
};

#define APPROVE 1
#define DISAPPROVE 0

static int msg_regist_send(int connect_fd, const struct msg_regist_sendbuf_t *sendbuf) {
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

static int msg_regist_recv(int connect_fd, struct msg_regist_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_regist_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->msgtype, sizeof(recvbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->approve, sizeof(recvbuf->approve), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msg_regist(const char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_regist_sendbuf_t sendbuf;
    struct msg_regist_recvbuf_t recvbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    bzero(&recvbuf, sizeof(recvbuf));
    sendbuf.msgtype = MT_REGIST;

    int pwd_check = DISAPPROVE;
    char pwd_plaintext[64] = {0};

    ret = sscanf(cmd, "%*s%s%s", sendbuf.username, pwd_plaintext);
    if (ret > 0) {
        sendbuf.username_len = strlen(sendbuf.username);
        if (strlen(pwd_plaintext) >= 3 || strlen(pwd_plaintext) <= 30) {
            pwd_check = APPROVE;
        }
    } else {
        // 核验用户名. 期间可能有与服务端的多次通信.
        recvbuf.approve = DISAPPROVE;
        while (DISAPPROVE == recvbuf.approve) {
            // 获取用户希望注册的用户名
            printf("[REGIST] 请输入想要注册的用户名:");
            fflush(stdout);
            scanf("%s", sendbuf.username);
            sendbuf.username_len = strlen(sendbuf.username);
            // 向服务端发送核验用户名的消息. 此时密码为空 (之前已经对 sendbuf 结构体进行过 bzero).
            ret = msg_regist_send(program_stat->connect_fd, &sendbuf);
            RET_CHECK_BLACKLIST(-1, ret, "msg_regist_send");
            // 获取服务端的回复信息
            ret = msg_regist_recv(program_stat->connect_fd, &recvbuf);
            RET_CHECK_BLACKLIST(-1, ret, "msg_regist_recv");
            if (DISAPPROVE == recvbuf.approve) { // 说明服务端拒绝了该用户名
                printf("[REGIST] 无法使用该用户名. 请重试.\n");
            }
        }
    }

    // 获取密码并进行正式注册
    char *pwd_plaintext_p = NULL;
    while (DISAPPROVE == pwd_check) {
        // 第一次从用户输入中获取密码
        pwd_plaintext_p = getpass("[REGIST] 请输入密码(无回显), 可输入 exit 退出:");
        // 如果用户希望中断注册 (在 getpass 函数中无法通过 SIGINT 信号退出), 则跳出循环. 注意到此时会直接跳转到函数尾注册失败的情形.
        if (0 == strcmp(pwd_plaintext_p, "exit")) {
            break;
        }
        // 如果第一次输入的密码不符合规范, 则重新进行循环.
        if (strlen(pwd_plaintext_p) < 3 || strlen(pwd_plaintext_p) > 30) {
            printf("[ERROR]  密码长度过短或过长, 请确保密码为 3 ~ 30 个字符.\n");
            continue;
        }
        // 将第一次输入的密码归档保存
        strcpy(pwd_plaintext, pwd_plaintext_p);
        // 第二次从用户输入中获取密码
        pwd_plaintext_p = getpass("[REGIST] 请再次输入密码(无回显):");
        // 如果两次输入的密码不一致, 则重新进行循环.
        if (0 != strcmp(pwd_plaintext_p, pwd_plaintext)) {
            printf("[ERROR]  两次输入的密码不一致. 请重新输入.\n");
            continue;
        }
        // 如果一切步骤正常进行, 说明密码本地核验通过
        pwd_check = APPROVE;
    }
    // 在密码本地核验通过的情况下, 向服务端正式提交注册申请
    if (APPROVE == pwd_check) {
        // 对密码进行 token2nd 异或
        for (int i = 0; i < strlen(pwd_plaintext); i++) {
            pwd_plaintext[i] = pwd_plaintext[i] ^ program_stat->token2nd[i];
        }
        // 对异或后的密码进行 rsa 加密
        sendbuf.pwd_ciprsa_len = rsa_encrypt(pwd_plaintext, sendbuf.pwd_ciprsa, program_stat->serverpub_rsa, PUBKEY);
        RET_CHECK_BLACKLIST(-1, sendbuf.pwd_ciprsa_len, "rsa_encrypt");
        // 销毁密码明文, 确保安全
        bzero(pwd_plaintext, sizeof(pwd_plaintext));
        pwd_plaintext_p = NULL;
        // 向服务端发送正式的注册信息. 这条信息同时包括用户名和密码.
        ret = msg_regist_send(program_stat->connect_fd, &sendbuf);
        RET_CHECK_BLACKLIST(-1, ret, "msg_regist_send");
        // 获取服务端的回复信息. 注意, 服务端在客户端的一个连接进行注册的整个过程中并非是阻塞等待的.
        // 因此在极端情况下, 服务端仍旧可能不批准注册请求.
        // 如: 在核验用户名通过后, 有其他连接抢先一步注册了该用户名.
        ret = msg_regist_recv(program_stat->connect_fd, &recvbuf);
        RET_CHECK_BLACKLIST(-1, ret, "msg_regist_recv");
    }

    if (APPROVE == recvbuf.approve) {
        logging(LOG_INFO, "注册成功.");
        ret = 0;
    } else {
        logging(LOG_WARN, "注册失败.");
        ret = -1;
    }

    return ret;
}