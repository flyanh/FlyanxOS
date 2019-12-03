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
 * 待操作系统使用它fork新的程序。
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <flyanx/type.h>
#include <flyanx/common.h>
#include <flyanx/syslib.h>

/*===========================================================================*
 *                            origin_main                                        *
 *                            起源程序主函数             *
 *===========================================================================*/
void origin_main(void){
#define FOREVER -1
    Message msg;


    /* 现在还没啥事做，先永久堵塞自己 */
    msg.type = 5;
    msg.m6_l1 = FOREVER;
    while (1){
        send_receive(0, &msg);
    }
}

