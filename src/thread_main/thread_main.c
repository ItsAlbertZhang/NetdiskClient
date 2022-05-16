#include "thread_main.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"

int thread_main_handle(struct program_stat_t *program_stat) {
    int ret = 0;
    ret = connect_msg_handle(program_stat);
    RET_CHECK_BLACKLIST(-1, ret, "connect_msg_handle");
    return 0;
}