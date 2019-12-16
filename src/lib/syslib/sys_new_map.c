/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/14.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 一个新进程已经创建，需要报告它的内存映像给内核。
 */

#include "syslib.h"

PUBLIC int sys_new_map(int proc_nr, MemoryMap *mptr){
    Message out;

    out.m1_i1 = proc_nr;
    out.m1_p1 = (char*)mptr;
    return task_call(SYS_TASK, SYS_NEW_MAP, &out);
}


