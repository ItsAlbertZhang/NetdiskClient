#ifndef __MYLIBRARY_H__
#define __MYLIBRARY_H__

// file.c

// 检查文件是否存在, 存在返回 1, 不存在返回 0. dir 可以为 NULL.
int file_exist(const char *dir, const char *filename);

// 从文件中读取字符串并写入 str, 长度至多为 maxlen, 返回值为成功读取长度. dir 可以为 NULL.
int read_string_from_file(char *str, int maxlen, const char *dir, const char *filename);

// 将字符串写入文件, 长度为 len, 返回值为实际写入长度. dir 可以为 NULL.
int write_file_from_string(const char *str, int len, const char *dir, const char *filename);

// rsa.c

#define PRIKEY 0
#define PUBKEY 1

int rsa_encrypt(const unsigned char *plaintext, unsigned char *ciphertext, RSA *rsa, int rsa_type);
int rsa_decrypt(unsigned char *plaintext, const unsigned char *ciphertext, RSA *rsa, int rsa_type);

// log.c

extern char logbuf[4096];

// 记录日志
int logging(int type, const char *str);

// random.c

// 生成随机字符串
int random_gen_str(char *str, int len, int seed);

#endif /* __MYLIBRARY_H__ */