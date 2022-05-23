#include "head.h"
#include "mylibrary.h"
#include "program_init.h"

// 从密钥文件获取 rsa 结构体
static int get_rsa_from_file(RSA **rsa, const char *rsa_key_filename, int rsa_type);

int init_rsa_keys(void) {
    int ret = 0;

    // 拼接出公私钥的完整路径
    char serverpub_key_filename[1024] = {0};
    strcpy(serverpub_key_filename, program_stat->config_dir);
    strcat(serverpub_key_filename, "serverpub.pem");

    ret = 0;
    if (file_exist(NULL, serverpub_key_filename)) {
        // 服务端公钥存在
        ret += get_rsa_from_file(&program_stat->serverpub_rsa, serverpub_key_filename, PUBKEY);
    } else {
        ret = -1;
    }
    if (0 == ret) {
        logging(LOG_INFO, "成功获取在本地存储的服务端公钥.");
    }
    if (-1 == ret) {
        unlink(serverpub_key_filename);
        logging(LOG_WARN, "未在本地成功获取服务端公钥.");
    }

    return 0;
}

static int generate_rsa_keys(const char *private_key_filename, const char *public_key_filename) {
    int ret = 1;
    RSA *r = NULL;
    BIGNUM *bne = NULL;
    BIO *bp_public = NULL, *bp_private = NULL;

    int bits = 2048;
    unsigned long e = RSA_F4;

    // 1. generate rsa key
    if (1 == ret) {
        bne = BN_new();
        ret = BN_set_word(bne, e);
    }
    if (1 == ret) {
        r = RSA_new();
        ret = RSA_generate_key_ex(r, bits, bne, NULL);
    }

    // 2. save public key
    if (1 == ret) {
        bp_public = BIO_new_file(public_key_filename, "w+");
        ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    }

    // 3. save private key
    if (1 == ret) {
        bp_private = BIO_new_file(private_key_filename, "w+");
        ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);
    }

    // 4. free
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(r);
    BN_free(bne);

    if (1 == ret) {
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}

static int get_rsa_from_file(RSA **rsa, const char *rsa_key_filename, int rsa_type) {
    int ret = 0;

    // 打开 RSA 密钥文件
    int fd = open(rsa_key_filename, O_RDWR);
    RET_CHECK_BLACKLIST(-1, fd, "open");

    // 从文件中获取 RSA 信息
    char buf[4096] = {0};
    ret = read(fd, buf, sizeof(buf));
    RET_CHECK_BLACKLIST(-1, ret, "read");
    ret = rsa_str2rsa(buf, rsa, rsa_type);
    RET_CHECK_BLACKLIST(-1, ret, "rsa_str2rsa");

    close(fd);

    return 0;
}