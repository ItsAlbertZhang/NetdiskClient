# NetdiskClient

Netdisk client project practice.

本项目使用 C 语言进行编写, 并使用 Makefile 进行编译与链接.

本项目为 NetdiskServer 项目的伴生成品, 且大量复用了其中的内容. 在了解本项目之前, 应当对 NetdiskServer 项目有足够的了解.

本项目的开发手册见 NetdiskServer 项目下的 Manual.md.

## 运行程序

### 第一步: 安装环境与依赖

本项目运行在 Linux 环境下, 测试环境为 Ubuntu 20.04.

依赖: `-lpthread` , `-lssl` , `-lcrypto`.

测试环境下为: 编译安装 openssl 1.1.1 软件包.

### 第二步: 编译

使用 `make` 命令以进行编译.

在编译时定义不同的参数, 编译结果会随之发生变化.

- `DEBUG` 参数: 定义该参数会使程序在终端输出 DEBUG 日志.

### 第三步: 配置 .config 文件

详见 config/README.md.