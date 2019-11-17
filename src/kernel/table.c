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
 * 的所有数据声明分配存储空间。除了global.h 和 process.h中的结构以外,tty.h中定义的由终端任务使用的几个全局变量
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

/* 下面从系统任务号从小到大给出每个系统任务的启动例程。此处的名称循序
 * 与<flyanx/common.h>中分配给任务的数值一致。
 */
/* 一个128字的小栈 */
#define SMALL_STACK (128 * sizeof(char*))

/* 终端任务栈大小  -17 */
#define TTY_TASK_STASK      (3 * SMALL_STACK)
#define SYNC_ALARM_STACK    SMALL_STACK

/* 闲置任务     -6 */
#if (CHIP == INTEL)
#define	IDLE_TASK_STACK	((3+3+4) * sizeof(char*))  /* 3 个中断, 3 缓存 , 4 个字节 */
#else
#define IDLE_TASK_STACK	    SMALL_STACK
#endif

/* 时钟任务栈大小 */
#define CLOCK_TASK_STACK SMALL_STACK

/* 所有系统任务的栈空间大小 */
#define TOT_TASK_STACK_SPACE    (IDLE_TASK_STACK + CLOCK_TASK_STACK)

#define	HARDWARE_STACK	    0		/* 虚拟任务，使用内核堆栈 */

/* 为系统任务表的所有表象分配空间 */
PUBLIC TaskTab tasktab[] = {
        /* 终端任务，必须存在 @TODO 未完成 */

        /* 闲置任务 */
        { idle_task, IDLE_TASK_STACK, "IDLE_TASK" },

        /* 时钟任务 */
        { clock_task, CLOCK_TASK_STACK, "CLOCK_TASK" },
        /* 硬件任务，没有任何数据和正文，占个位置 - 用作判断中断 */
        { 0, HARDWARE_STACK, "HARDWARE" },

//        /* 内存管理器 */
//        { 0,			0,		"MM"		},
//        /* 文件系统 */
//        { 0,			0,		"FS"		},
//        /* 飞彦扩展管理器 */
//        { 0,			0,		"FLY"		},
//        /* 起源进程 */
        { 0,			0,		"ORIGIN"		},
};

/* 所有系统任务堆栈的堆栈空间。 （声明为（char *）使其对齐。） */
PUBLIC char *task_stack[TOT_TASK_STACK_SPACE / sizeof(char *)];

/*===========================================================================*
 *                                   map_drivers                                    *
 *                                   映射驱动程序              *
 *===========================================================================*/
PUBLIC void map_drivers(){
    /* 将驱动程序映射到控制器，并将任务表更新为该选择子。@TODO */


}

/* 虽然已经尽量将所有用户可设置的配置消息单独放在include/flyanx/config.h中,但是在将tasktab的大小与
 * NR_TASKS相匹配时仍可能会导致错误。在table.c的结尾处使用一个小技巧对这个错误进行检测。方法是在这里声明
 * 一个dummy_tasktab,声明的方式是假如发生了前述的错误,则dummy_tasktab的大小是非法的,从而导致编译错误。
 * 由于哑数组声明为extern,此处并不为它分配空间(其他地方也不为其分配空间)。因为在代码中任何地方都不会
 * 引用到它,所以编译器不会受任何影响。
 *
 * 简单解释：减去的是MM、FS、FLY和ORIGIN，这些都不属于系统任务
 */
//#define NKT (sizeof(tasktab) / sizeof(struct tasktab_s) - (ORIGIN_PROC_NR + 1))
#define NKT ( sizeof(tasktab) / sizeof(struct tasktab_s) - (1) )

extern int dummy_tasktab_check[NR_TASKS == NKT ? 1 : -1];


