/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fly.h"
#include <sys/signal.h>
#include <flyanx/callnr.h>

/*===========================================================================*
 *				fly_no_sys					     *
 *			  无效调用处理
 *===========================================================================*/
PUBLIC int fly_no_sys(void)
{
    /* 请求了FLY未实现的系统调用号。
     * 本过程应该永远不被调用，提供它只是为了处理用户用非法的或不是由内存管理器处理的
     * 系统调用号调用内存管理器的情况。
     */
    return EINVAL;
}

/*===========================================================================*
 *                         mm_panic                                   *
 *                       系统无法继续运行                               *
 *===========================================================================*/
PUBLIC void fly_panic(msg, err_no)
        const char *msg;
        int err_no;
{
    /* 只有在内存管理器检测到一个它无法恢复的严重错误时才会被调用。它向系统任务报告错误，系统任务
     * 紧急停止系统。它不该被轻易调用。例如当检测到内部不一致（例如，编程错误或定义的常数的非法值）时
     * ，会引起恐慌然后宕机。
     */

    if(msg != NULL){
        printf("Memory manager panic: %s", msg);
        if(err_no != NO_NUM) printf(" %d", err_no);
        printf("\n");
    }
}
