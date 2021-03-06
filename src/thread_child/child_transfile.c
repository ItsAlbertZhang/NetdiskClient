#include "head.h"
#include "thread_child.h"

static size_t recv_splice_n(int connect_fd, int filefd, struct progress_t *progress_bar, int *pipefd);

int child_s2c(int connect_fd, int filefd, struct progress_t *progress_bar, int pipefd[2]) {
    int ret = 0;
    ret = recv_splice_n(connect_fd, filefd, progress_bar, pipefd);
    RET_CHECK_BLACKLIST(-1, ret, "recv_splice_n");
    return 0;
}

static size_t recv_splice_n(int connect_fd, int filefd, struct progress_t *progress_bar, int *pipefd) {
    size_t ret = 0;
    // struct progress_t progress_bar;
    // progress_bar_init(&progress_bar, filesize, 1);
    while (progress_bar->completedsize < progress_bar->filesize) {
        ret = splice(connect_fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        ret = splice(pipefd[0], NULL, filefd, NULL, ret, SPLICE_F_MORE | SPLICE_F_MOVE);
        progress_bar->completedsize += ret;
        // printf("recvsize = %ld\n", recvsize);
        // progress_bar_handle(&progress_bar, recvsize);
    }
    // progress_bar_destroy(&progress_bar);

    return progress_bar->completedsize;
}