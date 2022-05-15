#include "thread_main.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int thread_main_handle(struct program_stat_t *program_stat) {
    int ret = 0;
    //客户端发送数据
    char sendbuf[1024] = {0};
    // printf("请输入要发送的文本:\n");
    // fflush(stdin);
    // read(STDIN_FILENO, sendbuf, sizeof(sendbuf));
    sleep(5);
    sendbuf[0] = 1;
    ret = send(program_stat->connect_fd, sendbuf, strlen(sendbuf), 0);

    //客户端接收数据
    char recvbuf[1024] = {0};
    ret = recv(program_stat->connect_fd, recvbuf, sizeof(recvbuf), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    printf("接收到来自服务器的信息:\n%s\n", recvbuf);

    return ret;
}