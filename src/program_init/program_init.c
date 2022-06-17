#include "program_init.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int program_init() {
    int ret = 0;

    // 获取配置文件目录
    ret = init_getconfig();
    logging(LOG_INFO, "成功获取配置文件目录.");

    char config[MAX_CONFIG_ROWS][MAX_CONFIG_LENGTH];

    // 初始化并获取 rsa 密钥
    ret = init_rsa_keys();
    RET_CHECK_BLACKLIST(-1, ret, "init_rsa_keys");
    logging(LOG_INFO, "成功获取 rsa 密钥.");

    // 初始化线程池
    ret = init_pthread_pool();
    RET_CHECK_BLACKLIST(-1, ret, "init_pthread_pool");
    logging(LOG_INFO, "成功初始化线程池.");

    return 0;
}