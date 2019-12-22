/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：执行一个文件，给出命令行参数指针数组。
 */

#include <lib.h>
#define execve  _execve
#define execv   _execv
#include <unistd.h>
#include <string.h>

PUBLIC int execv(
        const char *path,       /* 执行文件的全路径 */
        char * argv[]           /* 执行时的命令行参数指针数组，必须以0结尾 */
){
    Message out;
    char **vp = argv;
    char exec_stack[ARG_MAX];
    int stack_len = 0;

    /* 创建一个堆栈内存映像，该映像仅需要内核稍作修改即可用于
     * 准备执行的进程。
     */

    /* 首先我们跳过参数数组中的文件名 */
    while (*vp){
        stack_len += sizeof(char*);
        vp++;
    }
    /* 检查堆栈是否溢出，存不下了？ */
    if((stack_len + 2 * sizeof(char*)) >= ARG_MAX){
        errno = E2BIG;
        return -1;
    }

    *((int*)(&exec_stack[stack_len])) = 0;
    stack_len += sizeof(char*);


    /* 将参数字符串数组拷贝到堆栈中 */
    char **q = (char**)exec_stack;
    for(vp = argv; *vp != 0; vp++){
        *q++ = &exec_stack[stack_len];
        if((stack_len + strlen(*vp) + 1) >= ARG_MAX){
            errno = E2BIG;
            return -1;
        }

        strcpy(&exec_stack[stack_len], *vp); /* 拷贝参数字符串 */
        stack_len += strlen(*vp);
        exec_stack[stack_len] = 0;
        stack_len++;
    }

    /* 好了，可以最终执行系统调用了 */
    out.m1_p1 = (char*) path;
    out.m1_i1 = strlen(path) + 1;
    out.m1_p2 = exec_stack;
    out.m1_i2 = stack_len;
    (void)_syscall(MM, EXEC, &out);   /* 执行新程序，如果成功下面将不会再执行的到 */

    return -1;
}


