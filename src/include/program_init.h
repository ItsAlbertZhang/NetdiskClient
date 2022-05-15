#ifndef __PROGRAM_INIT_H__
#define __PROGRAM_INIT_H__

#define MAX_CONFIG_ROWS 16
#define MAX_CONFIG_LENGTH 256

#include "head.h"
#include "main.h"

// 读取配置文件前, 获取配置文件目录
int getconfig_init(char *dir, int dirlen);
// 读取配置文件
int getconfig(const char *config_dir, const char *filename, char config[][MAX_CONFIG_LENGTH]);

// 初始化 tcp
int tcp_connect(const char *config_dir, char config[][MAX_CONFIG_LENGTH]);

#endif /* __PROGRAM_INIT_H__ */