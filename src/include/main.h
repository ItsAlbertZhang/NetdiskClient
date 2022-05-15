#ifndef __MAIN_H__
#define __MAIN_H__

#include "head.h"

// 进程状态
struct program_stat_t {
    int connect_fd;
};

enum log_type {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

int program_init(struct program_stat_t *program_stat);

int thread_main_handle(struct program_stat_t *program_stat);

#endif /* __MAIN_H__ */