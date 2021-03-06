#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_conninit_sendbuf_t {
    char msgtype;             // 消息类型
    int clientpubrsa_len;     // 下一字段的长度
    char clientpubrsa[1024];  // 客户端公钥字符串
    int serverpub_md5_len;    // 下一字段的长度
    char serverpub_md5[1024]; // 客户端本地存储的服务端公钥字符串的 MD5 校验码
};

struct msg_conninit_recvbuf_t {
    char msgtype;            // 消息类型
    int pretoken;            // token 前缀, 其实质为最近一次连接时服务端的文件描述符
    int token_ciprsa_len;    // 下一字段的长度
    char token_ciprsa[1024]; // token 密文
    int serverpubrsa_len;    // 下一字段的长度
    char serverpubrsa[1024]; // 服务端公钥. 当无需传输时, 此字段与上一字段置空.
};

static int msg_conninit_send(int connect_fd, const struct msg_conninit_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->clientpubrsa_len, sizeof(sendbuf->clientpubrsa_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->clientpubrsa, sendbuf->clientpubrsa_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->serverpub_md5_len, sizeof(sendbuf->serverpub_md5_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->serverpub_md5, sendbuf->serverpub_md5_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_conninit_recv(int connect_fd, struct msg_conninit_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_conninit_recvbuf_t));

    ret = recv_n(connect_fd, &recvbuf->pretoken, sizeof(recvbuf->pretoken), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    ret = recv_n(connect_fd, &recvbuf->token_ciprsa_len, sizeof(recvbuf->token_ciprsa_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->token_ciprsa, recvbuf->token_ciprsa_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    ret = recv_n(connect_fd, &recvbuf->serverpubrsa_len, sizeof(recvbuf->serverpubrsa_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->serverpubrsa, recvbuf->serverpubrsa_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

static RSA *client_rsa;

int msgsend_conninit(void) {
    int ret = 0;

    // 准备资源
    struct msg_conninit_sendbuf_t sendbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    sendbuf.msgtype = MT_CONNINIT;

    // 初始化客户端密钥
    BIGNUM *bne = NULL;
    unsigned long e = RSA_F4;
    bne = BN_new();
    BN_set_word(bne, e);
    client_rsa = RSA_new();
    RSA_generate_key_ex(client_rsa, 2048, bne, NULL);
    BN_free(bne);

    // 将客户端公钥填入 sendbuf 结构体
    ret = rsa_rsa2str(sendbuf.clientpubrsa, client_rsa, PUBKEY);
    sendbuf.clientpubrsa_len = strlen(sendbuf.clientpubrsa);

    // 若已在初始化阶段成功获取服务端公钥, 则将其转换为字符串并计算 MD5 校验码, 填入 sendbuf 结构体中的 serverpub_md5 成员中.
    if (program_stat->serverpub_rsa) {
        char local_serverpub_str[1024] = {0};
        ret = rsa_rsa2str(local_serverpub_str, program_stat->serverpub_rsa, PUBKEY); // 将服务端公钥 RSA 结构体转换为字符串
        RET_CHECK_BLACKLIST(-1, ret, "rsa_rsa2str");
        strcpy(sendbuf.serverpub_md5, MD5(local_serverpub_str, strlen(local_serverpub_str), NULL)); // 获取字符串的 MD5 校验码
        sendbuf.serverpub_md5_len = strlen(sendbuf.serverpub_md5);
    }

    // 发送消息
    ret = msg_conninit_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_conninit_send");

    return 0;
}

int msgrecv_conninit(void) {
    int ret = 0;

    // 准备资源
    struct msg_conninit_recvbuf_t recvbuf;
    bzero(&recvbuf, sizeof(recvbuf));

    // 接收消息
    ret = msg_conninit_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_conninit_recv");

    // 对接收到的 token 进行处理
    program_stat->pretoken = recvbuf.pretoken; // token 第 1 部分
    char token_plain[1024] = {0};
    ret = rsa_decrypt(token_plain, recvbuf.token_ciprsa, client_rsa, PRIKEY);
    RET_CHECK_BLACKLIST(-1, ret, "rsa_decrypt");
    strcpy(program_stat->token, token_plain);
    sprintf(logbuf, "接收到来自服务器的 token: %s", program_stat->token);
    logging(LOG_DEBUG, logbuf); // 将 token 第二部分 作为 DEBUG 信息打印

    // 对可能接收到的服务端公钥进行处理 (serverpub_str 成员)
    if (recvbuf.serverpubrsa_len) { // serverpubrsa_len 不为 0, 本地公钥不存在或与服务端不匹配, 服务端向本地发送了一个新的密钥.
        logging(LOG_WARN, "[CONNINIT] 本地公钥不存在或与服务端不匹配, 已重新从服务端下载公钥, 是否要使用并将其保存至本地?");
        printf("[CONNINIT] 请输入(y/n):");
        fflush(stdout);
        char savepubrsa_input;
        int savepubrsa = -1;
        while (-1 == savepubrsa) { // 获取用户输入
            if (savepubrsa_input == '\n') {
                printf("[ERROR]   你输入的数据不合法! 请输入\"y\"或\"n\":");
                fflush(stdout);
            }
            savepubrsa_input = getchar();
            if (savepubrsa_input == 'y') {
                savepubrsa = 1;
            }
            if (savepubrsa_input == 'n') {
                savepubrsa = 0;
            }
        }
        if (1 == savepubrsa) { // 用户同意将服务端公钥保存并覆盖至本地
            ret = rsa_str2rsa(recvbuf.serverpubrsa, &program_stat->serverpub_rsa, PUBKEY);
            RET_CHECK_BLACKLIST(-1, ret, "rsa_str2rsa");
            write_file_from_string(recvbuf.serverpubrsa, recvbuf.serverpubrsa_len, program_stat->config_dir, "serverpub.pem");
            RET_CHECK_BLACKLIST(-1, ret, "write_file_from_string");
            logging(LOG_INFO, "成功保存服务端公钥至本地.");
        } else { // 用户拒绝来自服务端的公钥
            logging(LOG_FATAL, "已拒绝来自服务端的公钥, 程序已关闭.");
            exit(0);
        }
    }

    RSA_free(client_rsa);

    return 0;
}