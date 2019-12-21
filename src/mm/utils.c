/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含了内存管理器的管理例程。
 *
 * 该文件的入口点是：
 *  - allowed:	        查看是否允许访问
 *  - mm_no_sys:	    为无效的系统调用号调用此例程
 *  - mm_panic:	        MM发生致命错误，无法继续
 *  - mm_tell_fs:	    文件系统接口
 */

#include "mm.h"
#include <flyanx/callnr.h>
#include "mmproc.h"
#include "param.h"

/*===========================================================================*
 *				mm_no_sys					     *
 *			  无效调用处理
 *===========================================================================*/
PUBLIC int mm_no_sys()
{
    /* 请求了MM未实现的系统调用号。
     * 本过程应该永远不被调用，提供它只是为了处理用户用非法的或不是由内存管理器处理的
     * 系统调用号调用内存管理器的情况。
     */
    return EINVAL;
}

/*===========================================================================*
 *                         mm_panic                                   *
 *                       系统无法继续运行                               *
 *===========================================================================*/
PUBLIC void mm_panic(msg, err_no)
        const char *msg;
        int err_no;
{
    /* 只有在内存管理器检测到一个它无法恢复的严重错误时才会被调用。它向系统任务报告错误，系统任务
     * 紧急停止系统。它不该被轻易调用。例如当检测到内部不一致（例如，编程错误或定义的常数的非法值）时
     * ，会引起恐慌然后宕机。
     */

    /* OK，蓝屏吧（致敬XP）*/
    sys_blues();

    if(msg != NULL){
        printf("Memory manager panic: %s", msg);
        if(err_no != NO_NUM) printf(" %d", err_no);
        printf("\n");
    }
    sys_sudden(RBT_PANIC);  /* 死机 */
}

/*===========================================================================*
 *				mm_tell_fs					     *
 *			MM现在有些事要告诉FS
 *===========================================================================*/
PUBLIC void mm_tell_fs(int what, int p1, int p2, int p3){
    /**
     * 当内存管理器处理的事件需要通知文件系统时，它构造一条消息并发给文件系统。
     *
     * MM仅将此例程用于通知FS以下事件：
     *      tell_fs(EXEC, proc, 0, 0)
     *      tell_fs(EXIT, proc, 0, 0)
     *      tell_fs(FORK, child. parent, pid)
     */

    Message out;

    out.m1_i1 = p1;
    out.m1_i2 = p2;
    out.m1_i3 = p3;
    (void) task_call(FS_PROC_NR, what, &out);
}



