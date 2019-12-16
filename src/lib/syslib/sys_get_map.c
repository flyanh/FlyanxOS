/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/13.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 服务器想要知道某个进程的内存映像
 */

#include "syslib.h"

PUBLIC int sys_get_map(int proc_nr, MemoryMap *mptr){
    Message out;

    out.m1_i1 = proc_nr;
    out.m1_p1 = (char*)mptr;
    return task_call(SYS_TASK, SYS_GET_MAP, &out);
}
