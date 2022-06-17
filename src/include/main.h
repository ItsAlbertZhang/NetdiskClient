#ifndef __MAIN_H__
#define __MAIN_H__

#include "head.h"

// 子线程共用资源
struct thread_resource_t {
    struct queue_t *queue;   // 任务队列
    pthread_mutex_t mutex;   // 线程锁, 加解锁后方可对队列进行操作
    pthread_cond_t cond;     // 子线程等待所在的条件变量
    char filepool_dir[1024]; // 文件池所在目录
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