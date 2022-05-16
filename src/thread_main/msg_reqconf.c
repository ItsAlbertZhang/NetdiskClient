#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int msg_reqconf(struct program_stat_t *program_stat) {
    int ret = 0;

    char sendbuf_msgtype = MT_REQCONF;
    int sendbuf_msglen = 0;
    char *sendbuf_msg = NULL;
    if (program_stat->serverpub_rsa) {
        char serverpub_str[4096] = {0};
        ret = rsa_rsa2str(serverpub_str, program_stat->serverpub_rsa, PUBKEY);
        RET_CHECK_BLACKLIST(-1, ret, "rsa_rsa2str");
        sendbuf_msg = MD5(serverpub_str, strlen(serverpub_str), NULL);
        sendbuf_msglen = strlen(sendbuf_msg);
    }

    ret = send(program_stat->connect_fd, &sendbuf_msgtype, sizeof(sendbuf_msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send");
    ret = send(program_stat->connect_fd, &sendbuf_msglen, sizeof(sendbuf_msglen), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send");
    if (sendbuf_msglen) {
        ret = send(program_stat->connect_fd, sendbuf_msg, sendbuf_msglen, 0);
        RET_CHECK_BLACKLIST(-1, ret, "send");
    }

    char recvbuf_msgtype = 0;
    int recvbuf_confirmlen = 0;
    char recvbuf_confirm[1024] = {0};
    int recvbuf_rsastrlen = 0;
    char recvbuf_rsastr[4096] = {0};

    ret = recv_n(program_stat->connect_fd, &recvbuf_msgtype, sizeof(recvbuf_msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv_n");
    ret = recv_n(program_stat->connect_fd, &recvbuf_confirmlen, sizeof(recvbuf_confirmlen), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv_n");
    ret = recv_n(program_stat->connect_fd, recvbuf_confirm, recvbuf_confirmlen, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv_n");
    ret = recv_n(program_stat->connect_fd, &recvbuf_rsastrlen, sizeof(recvbuf_rsastrlen), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv_n");
    ret = recv_n(program_stat->connect_fd, recvbuf_rsastr, recvbuf_rsastrlen, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv_n");

    if (recvbuf_rsastrlen) {
        logging(LOG_WARN, "本地公钥不存在或与服务端不匹配, 已重新从服务端下载公钥, 是否要使用并将其保存至本地?");
        char savepubrsa_input;
        int savepubrsa = -1;
        while (-1 == savepubrsa) {
            if (savepubrsa_input == '\n') {
                printf("你输入的数据不合法! 请输入\"y\"或\"n\":");
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
        RET_CHECK_BLACKLIST(-1, savepubrsa, "savepubrsa");
        if (savepubrsa) {
            ret = rsa_str2rsa(recvbuf_rsastr, &program_stat->serverpub_rsa, PUBKEY);
            RET_CHECK_BLACKLIST(-1, ret, "rsa_str2rsa");
            write_file_from_string(recvbuf_rsastr, recvbuf_rsastrlen, program_stat->config_dir, "serverpub.pem");
            RET_CHECK_BLACKLIST(-1, ret, "write_file_from_string");
            logging(LOG_INFO, "成功保存服务器公钥至本地.");
        } else {
            logging(LOG_FATAL, "已拒绝来自服务器的公钥, 程序已关闭.");
            exit(0);
        }
    }

    return 0;
}