#include "program_init.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int program_init(struct program_stat_t *program_stat) {
    int ret = 0;

    // 获取配置文件目录
    char config_dir[1024];
    ret = getconfig_init(config_dir, sizeof(config_dir));
    logging(LOG_INFO, "成功获取配置文件目录.");

    char config[MAX_CONFIG_ROWS][MAX_CONFIG_LENGTH];

    // 初始化 tcp
    program_stat->connect_fd = tcp_connect(config_dir, config);
    RET_CHECK_BLACKLIST(-1, program_stat->connect_fd, "tcp_connect");
    logging(LOG_INFO, "成功初始化 tcp.");

    return 0;
}