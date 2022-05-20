#include "main.h"
#include "head.h"
#include "mylibrary.h"

struct program_stat_t *program_stat = NULL;

int main(int argc, const char *argv[]) {
    int ret = 0;

    struct program_stat_t program_stat_s;
    bzero(&program_stat_s, sizeof(program_stat_s));
    program_stat = &program_stat_s;
    ret = program_init(); // 程序初始化函数
    RET_CHECK_BLACKLIST(-1, ret, "program_init");
    logging(LOG_INFO, "程序成功启动并初始化完毕.");

    ret = thread_main_handle(); // 主线程功能函数
    RET_CHECK_BLACKLIST(-1, ret, "main_thread_handle");

    return 0;
}