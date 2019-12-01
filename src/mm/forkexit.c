/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此文件包含处理FORK、EXIT、和WAIT系统调用的实现
 */

#include "mm.h"
#include <sys/wait.h>
#include <flyanx/callnr.h>
#include <signal.h>
#include "mmproc.h"
#include "param.h"

#define LAST_FEW        3   /* 为超级用户保留的最后几个进程插槽 */

PRIVATE pid_t next_pid = ORIGIN_PID + 1;    /* 下一个要分配的pid */

/*===========================================================================*
 *				do_fork					     *
 *===========================================================================*/
PUBLIC int do_fork(void){

}

/*===========================================================================*
 *				do_mm_exit				     *
 *			 处理系统调用EXIT
 *===========================================================================*/
PUBLIC int do_mm_exit(void){
    /* 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。这样划分是因为mm_exit()也
     * 被用来处理被信号终止运行的进程。两者工作相同，但参数不同，所以这样划分是很方便的。
     */

    mm_exit(curr_mp, m_status);
    return (ERROR_NO_MESSAGE);      /* 死人不能再讲话 */
}

/*===========================================================================*
 *				mm_exit					     *
 *===========================================================================*/
PUBLIC void mm_exit(MMProcess *rmp, int exit_status){

}

/*===========================================================================*
 *				do_wait				     *
 *===========================================================================*/
PUBLIC int do_wait(void){

}
