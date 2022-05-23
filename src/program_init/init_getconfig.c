#include "head.h"
#include "main.h"
#include "program_init.h"

int init_getconfig() {
    int ret = 0;
    // 如果使用 getcwd, 则获取到的为工作目录, 在不同目录下启动程序会导致工作目录不一致.
    ret = readlink("/proc/self/exe", program_stat->config_dir, sizeof(program_stat->config_dir));
    RET_CHECK_BLACKLIST(-1, ret, "readlink");
    // 此时 dir 为 "/home/yx/Netdisk/NetdiskServer/debug/bin/netdisk_server"
    // 对 dir 进行字符串处理, 将其修改为 config 目录
    int cnt = 3;
    while (ret > 0 && cnt > 0) {
        ret--;
        if (program_stat->config_dir[ret] == '/') {
            cnt--;
        }
    }
    ret++;
    program_stat->config_dir[ret] = 0;
    strcat(program_stat->config_dir, "config/");
    // 最终 config_dir 为 "/home/yx/Netdisk/NetdiskServer/config/"
    return ret + sizeof("config/") - 1;
}
