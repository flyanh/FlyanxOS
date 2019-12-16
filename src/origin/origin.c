/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 起源程序，这是操作系统的第一个用户程序。
 * 这个程序成为操作系统的一个运行进程后，操作系统以后将会使用它来创建新的进程
 * 分支，即进行fork操作。所以这个进程在只需要做一些简单的工作就可以退出了，退
 * 出后，它将会称为一个僵尸进程，但是不会被操作系统清理，它就默默的留在那里等
 * 待操作系统使用它创建新的进程，它是一个模板。
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static void shabby_shell(const char * tty_name);

/*===========================================================================*
 *                            origin_main                                        *
 *                            起源程序主函数             *
 *===========================================================================*/
void origin_main(void){
#define FOREVER -1

    /* 打开标准输入/输出流（文件描述符） */
    open("/dev_tty0", O_RDWR);
    open("/dev_tty0", O_RDWR);

    printf("{ORIGIN}-> Do something for init...\n");

    /* 所有的控制台文件 */
//    char *tty_list[2] = {"/dev_tty1", "/dev_tty2"};

    int pid;
    if((pid = fork()) != 0){   /* 父进程应该执行的代码 */
        printf("{ORIGIN}-> [parent is running, child pid:%d]\n", pid);
    } else {        /* 子进程应该执行的代码 */
        printf("{ORIGIN}-> [fork child is running]\n");
    }

    /* 创建新线程运行Shell */
//    int i;
//    for(i = 0; i < 2; i++){
//        int pid = fork();
//        if(pid > 0){   /* 父进程应该执行的代码 */
//            printf("{ORIGIN}-> [parent is running, child pid:%d]\n", pid);
//        } else {        /* 子进程应该执行的代码 */
//            printf("fuck!\n");
////            printf("{ORIGIN}-> [child is running, pid:%d]\n", 6);
////            close(stdin_fd);
////            close(stdout_fd);
////
////            shabby_shell(tty_list[i]);
//        }
//    }

    /* 现在还没啥事做，先永久堵塞自己 */
    sleep(FOREVER);
}

/*===========================================================================*
 *                            shabby_shell                                        *
 *                            TINIX的简单Shell             *
 *===========================================================================*/
static void shabby_shell(const char * tty_name)
{
    /* 这是一个极其简单的Shell，它来自于TINIX，只能根据输入打开一个文件，
     * 但是它用来测试我们的FORK等功能已经完全够用了，后面将会被抛弃。
     */
    int stdin_fd = open(tty_name, O_RDWR);
    int stdout_fd = open(tty_name, O_RDWR);

    char rdbuf[128];

    while (1) {
        printf("$ ");
        int r = read(0, rdbuf, 70);
        rdbuf[r] = 0;

        int argc = 0;
        char * argv[0x400];
        char * p = rdbuf;
        char * s;
        int word = 0;
        char ch;
        do {
            ch = *p;
            if (*p != ' ' && *p != 0 && !word) {
                s = p;
                word = 1;
            }
            if ((*p == ' ' || *p == 0) && word) {
                word = 0;
                argv[argc++] = s;
                *p = 0;
            }
            p++;
        } while(ch);
        argv[argc] = 0;

        int fd = open(argv[0], O_RDWR);
        if (fd == -1) {
            if (rdbuf[0]) {
                printf("{");
                printf(rdbuf, r);
                printf("}\n");
            }
        }
        else {
            close(fd);
            printf("that good, you exec from file.\n");
//            int pid = fork();
//            if (pid != 0) { /* parent */
//                int s;
//                wait(&s);
//            }
//            else {	/* child */
//                execv(argv[0], argv);
//            }
        }
    }
    close(1);
    close(0);
}


