#ifndef __MAIN_H__
#define __MAIN_H__

#include "head.h"

#define MAX_CONFIG_ROWS 16
#define MAX_CONFIG_LENGTH 256

// 进程状态
struct program_stat_t {
    char config_dir[1024];
    RSA *private_rsa;
    RSA *public_rsa;
    RSA *serverpub_rsa;
    int connect_fd;
    char confirm[64];
    char token[1024];
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