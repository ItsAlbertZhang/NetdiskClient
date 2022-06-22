#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_child.h"

struct msg_cl_s2c_sendbuf_t {
    char msgtype;   // 消息类型
    int dir_len;    // 下一字段的长度
    char dir[1024]; // 原文件或目录
};

struct msg_cl_s2c_recvbuf_t {
    char msgtype;
    size_t filesize;
    int filename_len;
    char filename[64];
};

static int msg_cl_s2c_send(int connect_fd, const struct msg_cl_s2c_sendbuf_t *sendbuf) {
    int ret = 0;

    ret = send_n(connect_fd, &sendbuf->msgtype, sizeof(sendbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, &sendbuf->dir_len, sizeof(sendbuf->dir_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");
    ret = send_n(connect_fd, sendbuf->dir, sendbuf->dir_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "send_n");

    return 0;
}

static int msg_cl_s2c_recv(int connect_fd, struct msg_cl_s2c_recvbuf_t *recvbuf) {
    int ret = 0;

    bzero(recvbuf, sizeof(struct msg_cl_s2c_recvbuf_t));
    ret = recv_n(connect_fd, &recvbuf->msgtype, sizeof(recvbuf->msgtype), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->filesize, sizeof(recvbuf->filesize), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    ret = recv_n(connect_fd, &recvbuf->filename_len, sizeof(recvbuf->filename_len), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    printf("msgtype = %d, filesize = %ld, filenamelen = %d\n", recvbuf->msgtype, recvbuf->filesize, recvbuf->filename_len);
    ret = recv_n(connect_fd, recvbuf->filename, recvbuf->filename_len, 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");

    return 0;
}

int msg_cl_s2c(char *cmd) {
    int ret = 0;

    // 准备资源
    struct msg_cl_s2c_sendbuf_t sendbuf;
    struct msg_cl_s2c_recvbuf_t recvbuf;
    bzero(&sendbuf, sizeof(sendbuf));
    bzero(&recvbuf, sizeof(recvbuf));
    sendbuf.msgtype = MT_CL_S2C;

    sscanf(cmd, "%*s%s", sendbuf.dir);
    sendbuf.dir_len = strlen(sendbuf.dir);

    // 建立新连接
    int cl_connect_fd = 0;
    ret = msgsend_dupconn(&cl_connect_fd);
    // 读掉服务端回复的消息.
    char msgtype_temp = 0;
    ret = recv_n(cl_connect_fd, &msgtype_temp, sizeof(msgtype_temp), 0);
    RET_CHECK_BLACKLIST(-1, ret, "recv");
    if (MT_DUPCONN == msgtype_temp) {
        msgrecv_dupconn(cl_connect_fd);
    }

    // 发送消息
    ret = msg_cl_s2c_send(cl_connect_fd, &sendbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cl_s2c_send");

    // 获取服务端的回复信息
    ret = msg_cl_s2c_recv(cl_connect_fd, &recvbuf);
    RET_CHECK_BLACKLIST(-1, ret, "msg_cl_s2c_recv");

    if (0 == recvbuf.filesize) {
        logging(LOG_ERROR, "下载未成功执行, 请检查输入.");
    } else {
        // 唤起子线程执行任务
        struct thread_task_queue_elem_t elem;
        bzero(&elem, sizeof(elem));
        elem.flag = QUEUE_FLAG_S2C;
        elem.connect_fd = cl_connect_fd;
        elem.filesize = recvbuf.filesize;
        strcpy(elem.filename, recvbuf.filename);

        // 入队. (队列为线程资源队列, 用于存放待子线程处理的请求.)
        ret = queue_in(program_stat->thread_stat.thread_resource.queue, &elem);

        // 向子线程发送委派请求
        pthread_cond_signal(&program_stat->thread_stat.thread_resource.cond);
        logging(LOG_DEBUG, "已向子线程发出委派请求.");
    }

    return 0;
}