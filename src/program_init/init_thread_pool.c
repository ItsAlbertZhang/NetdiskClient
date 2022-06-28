#include "head.h"
#include "main.h"
#include "mylibrary.h"
#include "program_init.h"
#include "thread_child.h"

int init_pthread_pool(void) {
    int ret = 0;
    char config[MAX_CONFIG_ROWS][MAX_CONFIG_LENGTH];

    // 获取线程池配置
    ret = getconfig(program_stat->config_dir, "pthread.config", config);
    RET_CHECK_BLACKLIST(-1, ret, "getconfig");

    // 初始化线程状态
    program_stat->thread_stat.pth_num = atoi(config[0]);
    program_stat->thread_stat.pthid = (pthread_t *)malloc(sizeof(pthread_t) * program_stat->thread_stat.pth_num); // 初始化线程 id 数组
    // 初始化线程资源
    queue_init(&program_stat->thread_stat.thread_resource.task_queue, sizeof(struct thread_task_queue_elem_t), atoi(config[1])); // 初始化任务队列
    pipe(program_stat->thread_stat.thread_resource.pipe_fd);                                                                     // 初始化管道
    pthread_mutex_init(&program_stat->thread_stat.thread_resource.mutex, NULL);                                                  // 初始化线程锁
    pthread_cond_init(&program_stat->thread_stat.thread_resource.cond, NULL);                                                    // 初始化条件变量
    // 初始化文件池所在目录
    strncpy(program_stat->thread_stat.thread_resource.filepool_dir, program_stat->config_dir, strlen(program_stat->config_dir) - strlen("config/"));
    strcat(program_stat->thread_stat.thread_resource.filepool_dir, "file/");
    // 初始化子线程独占资源
    queue_init(&program_stat->thread_stat.thread_resource.exclusive_resources_queue, sizeof(struct thread_exclusive_resources_queue_elem_t), atoi(config[0]));
    struct thread_exclusive_resources_queue_elem_t elem;
    for (int i = 0; i < atoi(config[0]); i++) {
        bzero(&elem.progress_bar, sizeof(struct progress_t));
        ret = pipe(elem.pipefd);
        RET_CHECK_BLACKLIST(-1, ret, "pipe");
        queue_in(program_stat->thread_stat.thread_resource.exclusive_resources_queue, &elem);
    }
    // 初始化进度条
    queue_init(&program_stat->thread_stat.thread_resource.progress_queue, sizeof(struct progress_t *), atoi(config[0]));

    // 拉起子线程
    for (int i = 0; i < program_stat->thread_stat.pth_num; i++) {
        ret = pthread_create(&program_stat->thread_stat.pthid[i], NULL, thread_child_handle, (void *)&program_stat->thread_stat.thread_resource);
        THREAD_RET_CHECK(ret, "pthread_create");
    }

    return 0;
}