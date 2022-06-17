#ifndef __PROGRAM_INIT_H__
#define __PROGRAM_INIT_H__

#include "head.h"
#include "main.h"
#include "mylibrary.h"

// 读取配置文件前, 获取配置文件目录
int init_getconfig(void);
// 读取配置文件
int getconfig(const char *config_dir, const char *filename, char config[][MAX_CONFIG_LENGTH]);

// 初始化并获取 rsa 密钥
int init_rsa_keys(void);

// 初始化线程池
int init_pthread_pool(void);

#endif /* __PROGRAM_INIT_H__ */