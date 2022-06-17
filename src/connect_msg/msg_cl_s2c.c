#include "connect_msg.h"
#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "thread_child.h"

int msg_cl_s2c(char *cmd) {
    int ret = 0;

    // 读取命令, 准备拉起子线程
    struct queue_elem_t elem;
    bzero(&elem, sizeof(elem));
    sscanf(cmd, "%*s%s", elem.dir);

    printf("%s\n", elem.dir);

    return 0;
}