#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "program_init.h"
#include "thread_main.h"

int connect_init(const char *config_dir) {
    int ret = 0;
    char config[MAX_CONFIG_ROWS][MAX_CONFIG_LENGTH];

    // 获取 tcp 配置
    ret = getconfig(config_dir, "tcp.config", config);
    RET_CHECK_BLACKLIST(-1, ret, "getconfig");
    // 1.socket
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    RET_CHECK_BLACKLIST(-1, ret, "socket");
    // 2.connect
    struct sockaddr_in server_addr;                                                  //定义 sockaddr_in 结构体
    bzero(&server_addr, sizeof(server_addr));                                        //清空结构体初始化
    server_addr.sin_family = AF_INET;                                                //使用 IPv4 通信协议
    server_addr.sin_port = htons(atoi(config[1]));                                   //设置服务端端口
    server_addr.sin_addr.s_addr = inet_addr(config[0]);                              //设置服务端 IP
    ret = connect(connect_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //连接服务端
    if (-1 == ret) {
        logging(LOG_ERROR, "无法连接至服务端.");
        exit(0);
    }

    return connect_fd;
}