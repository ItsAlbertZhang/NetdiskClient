#ifndef __MAIN_H__
#define __MAIN_H__

#include "head.h"
#include "mylibrary.h"

struct progress_t {
    size_t filesize;
    size_t completedsize;
    size_t lastsize;
    time_t starttime;
    char filename[64];
};

// 子线程互斥资源
struct thread_exclusive_resources_queue_elem_t {
    struct progress_t progress_bar; // 用于进度条
    int pipefd[2];                  // 用于 splice
};

// 子线程共用资源
struct thread_resource_t {
    struct queue_t *task_queue;                // 任务队列, 元素 struct thread_task_queue_elem_t
    struct queue_t *exclusive_resources_queue; // 每个子线程的独占资源队列, 元素 struct thread_exclusive_resources_queue_elem_t
    struct queue_t *progress_queue;            // 进度条队列, 元素 struct progress_t * (之所以采用指针, 是为了方便在主线程访问)
    int pipe_fd[2];                            // 管道 (子线程通过管道向主线程发送消息)
    pthread_mutex_t mutex;                     // 线程锁, 加解锁后方可对队列进行操作
    pthread_cond_t cond;                       // 子线程等待所在的条件变量
    char filepool_dir[1024];                   // 文件池所在目录
};

// 线程池状态
struct thread_stat_t {
    pthread_t *pthid;
    int pth_num;
    struct thread_resource_t thread_resource;
};

// 进程状态
struct program_stat_t {
    char config_dir[1024];
    RSA *serverpub_rsa;
    struct thread_stat_t thread_stat;
    int connect_fd;
    char token[64];
    int pretoken;
};

#define QUEUE_FLAG_S2C 0
#define QUEUE_FLAG_C2S 1

// 队列结构体
struct thread_task_queue_elem_t {
    char flag;         // 工作模式 (s2c or c2s)
    int connect_fd;    // 连接对端的 socket 文件描述符
    size_t filesize;   // 文件大小
    char filename[64]; // 文件名
};

extern struct program_stat_t *program_stat;

enum log_type {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
    LOG_INPUT,
    LOG_OUTPUT,
};

int program_init(void);

int thread_main_handle(void);

#endif /* __MAIN_H__ */