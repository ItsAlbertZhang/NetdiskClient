#ifndef __MYLIBRARY_H__
#define __MYLIBRARY_H__

// file.c

// 检查文件是否存在, 存在返回 1, 不存在返回 0. dir 可以为 NULL.
int file_exist(const char *dir, const char *filename);

// 从文件中读取字符串并写入 str, 长度至多为 maxlen, 返回值为成功读取长度. dir 可以为 NULL.
int read_string_from_file(char *str, int maxlen, const char *dir, const char *filename);

// 将字符串写入文件, 长度为 len, 返回值为实际写入长度. dir 可以为 NULL.
int write_file_from_string(const char *str, int len, const char *dir, const char *filename);

// getconfig.c

#define MAX_CONFIG_ROWS 16
#define MAX_CONFIG_LENGTH 256

// 获取配置文件
int getconfig(const char *config_dir, const char *filename, char config[][MAX_CONFIG_LENGTH]);

// queue.c

// queue.c

struct queue_t {
    void *elem_array; // 队列数组, 数据类型为 int
    size_t elemsize;  // 元素大小
    int len;          // 队列长度
    int front;        // front 为队头元素下标
    int rear;         // rear 为队尾元素下标 + 1
    char tag;         // tag 为 1 代表上一次为入队操作, 为 0 代表为出队操作.
};

// 初始化队列
int queue_init(struct queue_t **pQ, size_t elemsize, int len);

// 销毁队列
int queue_destroy(struct queue_t **pQ);

// 入队. 队满返回 -1, 否则返回 0.
int queue_in(struct queue_t *Q, const void *elem);

// 出队. 队空返回 -1, 否则返回 0.
int queue_out(struct queue_t *Q, void *elem);

// rsa.c

#define PRIKEY 0
#define PUBKEY 1

int rsa_encrypt(const unsigned char *plaintext, unsigned char *ciphertext, RSA *rsa, int rsa_type);
int rsa_decrypt(unsigned char *plaintext, const unsigned char *ciphertext, RSA *rsa, int rsa_type);
int rsa_rsa2str(char *str, RSA *rsa, int rsa_type);
int rsa_str2rsa(const char *str, RSA **rsa, int rsa_type);

// log.c

extern char logbuf[4096];

// 记录日志
int logging(int type, const char *str);

// epoll.c

// 添加 epoll 监听
int epoll_add(int fd);

// 移除 epoll 监听
int epoll_del(int fd);

// random.c

// 生成随机字符串
int random_gen_str(char *str, int len, int seed);

#endif /* __MYLIBRARY_H__ */