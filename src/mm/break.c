/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含处理BREAK系统调用的实现。
 *
 * 正如我们在内存管理器的代码里所看到的，Flyanx现在所使用的内存模型是及其简单的：每个进程
 * 在创建的时候都会获得一块内存用于存放自己的所有数据（T、D&S），它永远不会在内存中被移动、
 * 永远不会被交换出内存、永远不会增长和缩小。FORK创建的子进程使用的内存块数量和父进程一样。
 * 如果子进程在以后要执行EXEC调用执行一个文件，则应该从文件EXEC的ELF可执行标头中获取新的
 * 内存大小。
 *
 * 文文件的入口点是：
 *  - do_break：     执行BREAK/SBREAK系统调用以增加或缩小数据段（同时也是文本段和堆栈段）
 */

#include "mm.h"
#include <signal.h>
#include "mmproc.h"
#include "param.h"


/*===========================================================================*
 *				do_break  				     *
 *			执行BREAK/SBREAK系统调用
 *===========================================================================*/
PUBLIC int do_break(){
    /*
     *
     */
}



