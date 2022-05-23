#include "head.h"
#include "mylibrary.h"

int getconfig(const char *config_dir, const char *filename, char config[][MAX_CONFIG_LENGTH]) {
    int ret = 0;
    char fullname[1024] = {0};
    sprintf(fullname, "%s%s", config_dir, filename); // 由 config 目录绝对路径与配置文件名拼接得到配置文件绝对路径

    FILE *fp = fopen(fullname, "rb"); // 打开配置文件
    RET_CHECK_BLACKLIST(NULL, fp, "fopen");
    for (ret = 0; ret < MAX_CONFIG_ROWS; ret++) {
        // 按行读取配置文件
        if (NULL == fgets(config[ret], MAX_CONFIG_LENGTH, fp)) {
            break; // 读到文件尾部, 结束
        } else {
            config[ret][strlen(config[ret]) - 1] = 0; // 替换行尾的 \n 为 \0
        }
    }
    fclose(fp); //关闭配置文件
    return ret;
}