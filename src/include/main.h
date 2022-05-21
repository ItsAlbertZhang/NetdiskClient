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
    char token123[64];
    int pretoken;
};

extern struct program_stat_t *program_stat;

enum log_type {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

int program_init(void);

int thread_main_handle(void);

#endif /* __MAIN_H__ */