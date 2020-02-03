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
 * 出后，它将会称为一个僵尸进程。
 */
#define _POSIX_SOURCE   /* 起源进程虽然特殊（和内核链接在一起），但是它本质上还是用户进程，所以需要POSIX标准的一些支持 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>

int untar(const char *filename, const char *parent_dir);
char *login(int tty_index);
void fly_shell(const char * tty_name, int tty_index);
static void shabby_shell(const char * tty_name);

/*===========================================================================*
 *                            origin_main                                        *
 *                            起源程序主函数             *
 *===========================================================================*/
void origin_main(void){

    /* 打开标准输入/输出流（文件描述符） */
    int stdin_fd = open("/dev_tty0", O_RDWR);
    int stdout_fd = open("/dev_tty0", O_RDWR);

    printf("{ORIGIN}-> Do something for init...\n");

    /* 提取"cmd.tar"归档文件到根目录 */
    untar("/cmd.tar", "/");

    /* 所有的控制台文件（除了0号，0号作为系统启动日志输出的地方） */
    char *tty_list[] = {"/dev_tty1", "/dev_tty2"};

    /* 创建新线程为每个控制台运行Shell */
    int pid, ppid, i;
    for(i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++){
        if((pid = fork()) != 0){    /* 父进程应该执行的代码 */
            printf("{ORIGIN}-> [parent is running, child pid: %d]\n", pid);
        } else {                    /* 子进程应该执行的代码 */
            pid = getpid();
            ppid = getppid();
            printf("{ORIGIN}-> [fork child is running, pid: %d, parent pid: %d]\n", pid, ppid);
            /* 在运行shell之前，先将先前打开的终端文件关闭 */
            close(stdout_fd);
            close(stdin_fd);

            /* 为该控制台终端打开fly_shell */
            fly_shell(tty_list[i], i + 1);
        }
    }

    int status;
    while ((pid = wait(&status)) != -1){  /* 源进程等待所有shell退出 */
        printf("child (%d) exited with status: %d.\n", pid, status);
    }
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
    int fd_stdin  = open(tty_name, O_RDWR);     /* 打开标准输入输出流 */
    int fd_stdout = open(tty_name, O_RDWR);

    char rdbuf[128];

    while (1) {
        printf("root@chenu # ");
        int r = read(0, rdbuf, 128);
        rdbuf[r - 1] = 0;       /* 长度-1：回车键会产生一个换行符，我们不需要这个换行符！ */
        int argc = 0;
        char * argv[_POSIX_ARG_MAX];
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

        /* 内键命令：exit，退出当前shell */
        if(strcmp(rdbuf, "exit") == 0){
            printf("bye~\n");
            close(fd_stdout);
            close(fd_stdin);
            exit(0);
        }

        int fd = open(argv[0], O_RDWR);
        if (fd == -1) {
            if (rdbuf[0]) {
                printf("{%s}\n", rdbuf);
            }
        }
        else {
            close(fd);
            int pid = fork();
            if (pid != 0) { /* parent */
                int s;
                wait(&s);
            }
            else {	/* child */
                execv(argv[0], argv);
            }
        }
    }
}


