/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统必须对空闲内存保持跟踪，同时进行分配，alloc.c完成这些功能。
 * flyanx0.1暂时只实现最普通的分配，不支持更复杂的调度，所以这个
 * 文件它非常的简洁。
 *
 * 该文件的入口点是：
 *  - alloc_mem     请求一块给定大小的内存。
 *  - free_mem      归还不再需要的内存
 */
#include "mm.h"
#include <flyanx/common.h>
#include <flyanx/callnr.h>
#include <signal.h>     /* 导入它只是因为mmproc.h需要 */
#include "mmproc.h"

/*===========================================================================*
 *				alloc_mem				     *
 *			为一个进程分配内存
 *===========================================================================*/
PUBLIC phys_clicks alloc_mem(
        int pid,            /* 哪个进程？ */
        phys_clicks clicks  /* 请求的内存块数量 */
){
    /* 这个函数及其简单，就只是为一个进程分配一块独有的内存块 */

    /* 如果是给起源进程分配，那没必要了，因为内核已经分配好了 */
//    if(pid == ORIGIN_PID) return NO_MEM;
//    if()
}


