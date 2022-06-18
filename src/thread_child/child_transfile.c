#include "head.h"
#include "thread_child.h"

static size_t recv_splice_n(int connect_fd, int filefd, size_t filesize, int *pipefd);

int child_s2c(int connect_fd, int filefd, size_t filesize) {
    int ret = 0;
    // 初始化一条管道, 用于 splice 文件传输
    int pipefd[2] = {0};
    ret = pipe(pipefd);
    RET_CHECK_BLACKLIST(-1, ret, "pipe");
    ret = recv_splice_n(connect_fd, filefd, filesize, pipefd);
    RET_CHECK_BLACKLIST(-1, ret, "recv_splice_n");
    return 0;
}

static size_t recv_splice_n(int connect_fd, int filefd, size_t filesize, int *pipefd) {
    size_t ret = 0;
    size_t recvsize = 0;
    // struct progress_bar_t progress_bar;
    // progress_bar_init(&progress_bar, filesize, 1);
    while (recvsize < filesize) {
        ret = splice(connect_fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        ret = splice(pipefd[0], NULL, filefd, NULL, ret, SPLICE_F_MORE | SPLICE_F_MOVE);
        recvsize += ret;
        // printf("recvsize = %ld\n", recvsize);
        // progress_bar_handle(&progress_bar, recvsize);
    }
    // progress_bar_destroy(&progress_bar);

    return recvsize;
}