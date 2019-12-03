/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含了内存内存管理器的管理例程。
 *
 * 该文件的入口点是：
 *  - allowed:	    查看是否允许访问
 *  - no_sys:	    为无效的系统调用号调用此例程
 *  - panic:	    MM发生致命错误，无法继续
 *  - tell_fs:	    文件系统接口
 */

#include "mm.h"
#include <sys/stat.h>
#include <flyanx/callnr.h>
#include <fcntl.h>
//#include "mproc.h"

/*===========================================================================*
 *				no_sys					     *
 *			  无效调用处理
 *===========================================================================*/
PUBLIC int no_sys()
{
    /* 请求了MM未实现的系统调用号。
     * 本过程应该永远不被调用，提供它只是为了处理用户用非法的或不是由内存管理器处理的
     * 系统调用号调用内存管理器的情况。
     */
    return(EINVAL);
}

/*===========================================================================*
 *                         mm_panic                                   *
 *                       系统无法继续运行                               *
 *===========================================================================*/
PUBLIC void mm_panic(msg, errno)
        const char *msg;
        int errno;
{
    /* 只有在内存管理器检测到一个它无法恢复的严重错误时才会被调用。它向系统任务报告错误，系统任务
     * 紧急停止系统。它不该被轻易调用。例如当检测到内部不一致（例如，编程错误或定义的常数的非法值）时
     * ，会引起恐慌然后宕机。
     */

    if(msg != NULL){
        printf("Memory manager panic: %s", msg);
        if(errno != NO_NUM) printf(" %d", errno);
        printf("\n");
    }
}



