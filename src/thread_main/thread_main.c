#include "thread_main.h"
#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int epfd = -1;

int thread_main_handle(void) {
    int ret = 0;

    // 初始化连接
    program_stat->connect_fd = connect_init(program_stat->config_dir);
    RET_CHECK_BLACKLIST(-1, program_stat->connect_fd, "connect_init");
    logging(LOG_INFO, "成功初始化连接.");

    // epoll 初始化
    epfd = epoll_create(1); // 创建 epoll 句柄
    // 将与子线程通信的管道读端添加至 epoll 监听
    ret = epoll_add(program_stat->thread_stat.thread_resource.pipe_fd[0]);
    RET_CHECK_BLACKLIST(-1, ret, "epoll_add");
    // 将 stdin 添加至 epoll 监听
    ret = epoll_add(STDIN_FILENO);
    RET_CHECK_BLACKLIST(-1, ret, "epoll_add");
    // 将 connect_fd 添加至 epoll 监听
    ret = epoll_add(program_stat->connect_fd);
    RET_CHECK_BLACKLIST(-1, ret, "epoll_add");
    // 返回的监听结果 struct epoll_event
    struct epoll_event events;
    bzero(&events, sizeof(events));
    int ep_ready = 0; // 有消息来流的监听个数

    ret = msgsend_conninit(); // 向服务端发送下发验证请求
    RET_CHECK_BLACKLIST(-1, ret, "msgsend_conninit");
    logging(LOG_INFO, "成功与服务端建立连接.");

    char program_running_flag = 1; // 程序继续运行标志
    while (program_running_flag) {
        ep_ready = epoll_wait(epfd, &events, 1, -1); // 进行 epoll 多路监听
        RET_CHECK_BLACKLIST(-1, ep_ready, "epoll_wait");
        for (int i = 0; i < ep_ready; i++) {
            // 有来自子线程的消息, 有任务完成
            if (events.data.fd == program_stat->thread_stat.thread_resource.pipe_fd[0]) {
                // 获取文件名
                char buf[64];
                ret = read(program_stat->thread_stat.thread_resource.pipe_fd[0], buf, sizeof(buf));
                RET_CHECK_BLACKLIST(-1, ret, "read");
                sprintf(logbuf, "%s 任务完成.", buf);
                logging(LOG_INFO, logbuf);
            }
            // 有来自服务端的消息, 说明服务端断开了连接
            if (events.data.fd == program_stat->connect_fd) {
                ret = connect_recvmsg_handle();
                RET_CHECK_BLACKLIST(-1, ret, "connect_recvmsg_handle");
            }
            if (events.data.fd == STDIN_FILENO) { // 消息来自 stdin
                ret = connect_sendmsg_handle();
                RET_CHECK_BLACKLIST(-1, ret, "connect_sendmsg_handle");
            }
        }
    }

    return 0;
}