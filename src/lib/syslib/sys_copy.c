/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 * 
 * 系统复制
 */

#include "syslib.h"

PUBLIC int sys_copy(
        int src_proc,			    /* 数据源的进程 */
        int src_seg,			    /* 数据源在进程中所处的段: TEXT, DATA 或 STACK */
        phys_bytes src_vir,		    /* 数据源虚拟地址 */
        int dest_proc,			    /* 目标的进程 */
        int dest_seg,			    /* 目标在进程中所处的段: TEXT, DATA 或 STACK*/
        phys_bytes dest_vir,		/* 目标的虚拟地址 */
        phys_bytes bytes		    /* 复制多少字节？ */
){
    /* 传输数据块。
     * 这个例程调用了内核的phys_copy，但是它是提供给服务器使用的，所以可能会有
    * 用户进程的参与，所以需要更多的信息（可能）用来转化为物理地址。
    */

    Message cp_msg;

    if(bytes == 0) return OK;
    cp_msg.SRC_SPACE = src_seg;
    cp_msg.SRC_PROC_NR = src_proc;
    cp_msg.SRC_BUFFER = (long)src_vir;

    cp_msg.DEST_SPACE = dest_seg;
    cp_msg.DEST_PROC_NR = dest_proc;
    cp_msg.DEST_BUFFER = (long)dest_vir;

    cp_msg.COPY_BYTES = (long) bytes;
    return task_call(SYS_TASK, SYS_COPY, &cp_msg);
}

