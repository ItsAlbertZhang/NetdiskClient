#include "head.h"
#include "program_init.h"
#include "mylibrary.h"

int tcp_connect(const char *config_dir, char config[][MAX_CONFIG_LENGTH]) {
    int ret = 0;

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
    server_addr.sin_port = htons(atoi(config[1]));                                   //设置服务器端口
    server_addr.sin_addr.s_addr = inet_addr(config[0]);                              //设置服务器 IP
    ret = connect(connect_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //连接服务器
    if (-1 == ret) {
        logging(LOG_ERROR, "无法连接至服务器.");
        exit(0);
    }
    
    return connect_fd;
}