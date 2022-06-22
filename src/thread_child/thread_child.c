#include "thread_child.h"
#include "head.h"
#include "main.h"

void *thread_child_handle(void *args) {
    int ret = 0;
    struct thread_resource_t *thread_resource = (struct thread_resource_t *)args;

    // 初始化工作环境
    // 先上锁再进循环
    pthread_mutex_lock(&thread_resource->mutex);
    while (1) {
        // 尝试出队, 注意此时锁处于上锁状态
        struct thread_task_queue_elem_t thread_task_queue_elem;
        ret = queue_out(thread_resource->task_queue, &thread_task_queue_elem);
        if (-1 == ret) {
            // 出队失败, 此时队列为空, 则在条件变量上等待
            pthread_cond_wait(&thread_resource->cond, &thread_resource->mutex);
        } else {
            // 出队成功, 拿到传输的对端所连接的 socket 文件描述符
            // 在解锁之前, 操作本次任务所需的独占资源
            struct thread_exclusive_resources_queue_elem_t exclusive_resource_queue_elem;
            queue_out(thread_resource->exclusive_resources_queue, &exclusive_resource_queue_elem);
            queue_in(thread_resource->progress_bar_queue, &exclusive_resource_queue_elem.progress_bar);
            exclusive_resource_queue_elem.progress_bar.filesize = thread_task_queue_elem.filesize;
            exclusive_resource_queue_elem.progress_bar.completedsize = 0;
            // 执行解锁操作
            pthread_mutex_unlock(&thread_resource->mutex);
            sprintf(logbuf, "子线程 %ld 已唤醒, 开始执行任务.", pthread_self());
            logging(LOG_INFO, logbuf);

            // 情况 QUEUE_FLAG_S2C: 执行 s2c 任务
            if (QUEUE_FLAG_S2C == thread_task_queue_elem.flag) {
                // 创建文件
                char filedir[1024] = {0};
                strcat(filedir, thread_resource->filepool_dir);
                strcat(filedir, thread_task_queue_elem.filename);
                int filefd = open(filedir, O_CREAT | O_TRUNC | O_RDWR, 0666);
                ftruncate(filefd, thread_task_queue_elem.filesize);
                // 执行文件传输
                ret = child_s2c(thread_task_queue_elem.connect_fd, filefd, exclusive_resource_queue_elem.progress_bar, exclusive_resource_queue_elem.pipefd);
                if (-1 == ret) {
                    sprintf(logbuf, "下载任务失败.");
                } else {
                    sprintf(logbuf, "成功完成下载.");
                }
                logging(LOG_INFO, logbuf);
                // 关闭文件
                close(filefd);
            }

            // 情况 QUEUE_FLAG_C2S: 执行 c2s 任务
            if (QUEUE_FLAG_C2S == thread_task_queue_elem.flag) {
                // code here
            }

            // 上锁, 并继续循环尝试出队.
            pthread_mutex_lock(&thread_resource->mutex);
        }
        // 注意: 尝试出队失败则 wait, 尝试出队成功则执行任务. 只有在 wait 的上下半之间(阻塞过程中), 以及执行任务期间, 锁处于解锁状态, 其他时间锁一律处于上锁状态.
    }

    return NULL;
}