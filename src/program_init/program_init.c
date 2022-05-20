#include "program_init.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int program_init() {
    int ret = 0;

    // 获取配置文件目录
    ret = init_getconfig(program_stat->config_dir, sizeof(program_stat->config_dir));
    logging(LOG_INFO, "成功获取配置文件目录.");

    char config[MAX_CONFIG_ROWS][MAX_CONFIG_LENGTH];

    // 初始化并获取 rsa 密钥
    ret = init_rsa_keys(&program_stat->private_rsa, &program_stat->public_rsa, &program_stat->serverpub_rsa, program_stat->config_dir);
    RET_CHECK_BLACKLIST(-1, ret, "init_rsa_keys");
    logging(LOG_INFO, "成功获取 rsa 密钥.");

    return 0;
}