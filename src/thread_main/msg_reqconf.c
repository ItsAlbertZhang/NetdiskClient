#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

struct msg_reqconf_sendbuf_t {
    char msgtype;             // 消息类型
    int clientpubrsa_len;     // 下一字段的长度
    char clientpubrsa[1024];  // 客户端公钥字符串
    int serverpub_md5_len;    // 下一字段的长度
    char serverpub_md5[1024]; // 客户端本地存储的服务端公钥字符串的 MD5 校验码
};

struct msg_reqconf_recvbuf_t {
    char msgtype;                // 消息类型
    int confirm_len;             // 下一字段的长度
    char confirm[64];            // 会话确认码
    int token_len;               // 下一字段的长度
    char token_ciphertext[1024]; // token 密文, 用于客户端建立新连接时的短验证
    int serverpub_str_len;       // 下一字段的长度
    char serverpub_str[1024];    // 服务端公钥. 当无需传输时, 此字段与上一字段置空.
};

static int msg_reqconf_send(int connect_fd, const struct msg_reqconf_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->clientpubrsa_len, sizeof(sendbuf->clientpubrsa_len) + sendbuf->clientpubrsa_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->serverpub_md5_len, sizeof(sendbuf->serverpub_md5_len) + sendbuf->serverpub_md5_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_reqconf_recv(int connect_fd, struct msg_reqconf_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_reqconf_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->msgtype, sizeof(recvbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->confirm_len, sizeof(recvbuf->confirm_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->confirm, recvbuf->confirm_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->token_len, sizeof(recvbuf->token_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->token_ciphertext, recvbuf->token_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->serverpub_str_len, sizeof(recvbuf->serverpub_str_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, recvbuf->serverpub_str, recvbuf->serverpub_str_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msg_reqconf(void) {
    int ret = 0;

    // 准备资源
    struct msg_reqconf_sendbuf_t sendbuf;
    struct msg_reqconf_recvbuf_t recvbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    bzero(&recvbuf, sizeof(recvbuf));
    sendbuf.msgtype = MT_REQCONF;

    // 将客户端公钥填入 sendbuf 结构体
    ret = rsa_rsa2str(sendbuf.clientpubrsa, program_stat->public_rsa, PUBKEY);
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
    ret = msg_reqconf_send(program_stat->connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_reqconf_send");

    // 接收消息
    ret = msg_reqconf_recv(program_stat->connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_reqconf_recv");

    // 对接收到的确认码进行处理 (confirm 成员)
    strcpy(program_stat->confirm, recvbuf.confirm);
    sprintf(logbuf, "接收到来自服务器的确认码: %s", program_stat->confirm);
    logging(LOG_DEBUG, logbuf); // 将确认码作为 DEBUG 信息打印

    // 对接收到的 token 进行处理
    ret = rsa_decrypt(program_stat->token, recvbuf.token_ciphertext, program_stat->private_rsa, PRIKEY);
    RET_CHECK_BLACKLIST(-1, ret, "rsa_decrypt");
    sprintf(logbuf, "接收到来自服务器的token: %s", program_stat->token);
    logging(LOG_DEBUG, logbuf); // 将 token 作为 DEBUG 信息打印

    // 对可能接收到的服务端公钥进行处理 (serverpub_str 成员)
    if (recvbuf.serverpub_str_len) { // serverpub_str_len 不为 0, 本地公钥不存在或与服务端不匹配, 服务端向本地发送了一个新的密钥.
        logging(LOG_WARN, "[REQCONF] 本地公钥不存在或与服务端不匹配, 已重新从服务端下载公钥, 是否要使用并将其保存至本地?");
        printf("[REQCONF] 请输入(y/n):");
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
            ret = rsa_str2rsa(recvbuf.serverpub_str, &program_stat->serverpub_rsa, PUBKEY);
            RET_CHECK_BLACKLIST(-1, ret, "rsa_str2rsa");
            write_file_from_string(recvbuf.serverpub_str, recvbuf.serverpub_str_len, program_stat->config_dir, "serverpub.pem");
            RET_CHECK_BLACKLIST(-1, ret, "write_file_from_string");
            logging(LOG_INFO, "成功保存服务端公钥至本地.");
        } else { // 用户拒绝来自服务端的公钥
            logging(LOG_FATAL, "已拒绝来自服务端的公钥, 程序已关闭.");
            exit(0);
        }
    }

    return 0;
}