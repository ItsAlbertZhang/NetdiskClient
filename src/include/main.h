#ifndef __MAIN_H__
#define __MAIN_H__

#include "head.h"

// 进程状态
struct program_stat_t {
    char config_dir[1024];
    RSA *serverpub_rsa;
    int connect_fd;
    char token2nd[64];
    int token1st;
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