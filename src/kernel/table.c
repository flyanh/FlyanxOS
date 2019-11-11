/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件其编译生成的目标文件将包含所有的核心数据结构,我们已经看到许多这类数据结构定义在glo.h 和proc.h中。
 *
 * 文件开始就定义了宏_TABLE,正好位于 #include语句之前。该定义将导致EXTERN被定义为空串,于是为EXTERN之后
 * 的所有数据声明分配存储空间。除了glo.h 和 proc.h中的结构以外,tty.h中定义的由终端任务使用的几个全局变量
 * 也都在这里分配存储空间。
 *
 * 这个方法非常巧妙，使用这个方法，一次头文件extern声明全局变量，在这个文件中导入，又将真正的变量声明出来并
 * 分配空间。
 */

#define _TABLE

#include "kernel.h"
#include <stdlib.h>
//#include <termios.h>
#include <flyanx/common.h>
#include "process.h"
//#include "tty.h"
#include <ibm/int86.h>

/* 系统调用表 */
EXTERN flyanx_syscall_t syscall_table[NR_SYS_CALL] = {
        bad_syscall, bad_syscall
};

