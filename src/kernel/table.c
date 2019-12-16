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
#include <termios.h>
#include <flyanx/common.h>
#include "process.h"
#include "tty.h"
#include <ibm/int86.h>

/* 下面从系统任务号从小到大给出每个系统任务的启动例程。此处的名称循序
 * 与<flyanx/common.h>中分配给任务的数值一致。
 */
/* 进程堆栈相关 */
/* 一个128字节的小栈 */
#define SMALL_STACK (128 * sizeof(char*))
/* 这是一个普通进程堆栈大小，16KB */
#define NORMAL_STACK (0x4000 * sizeof(char*))

/* 终端任务栈大小  -17 */
#define TTY_TASK_STASK      (3 * SMALL_STACK)
#define SYNC_ALARM_STACK    SMALL_STACK

/* 闲置任务     -6 */
#if (CHIP == INTEL)
#define	IDLE_TASK_STACK	((3+3+4) * sizeof(char*))  /* 3 个中断, 3 缓存 , 4 个字节 */
#else
#define IDLE_TASK_STACK	    SMALL_STACK
#endif

/* 控制器任务堆栈大小 */
#define CONTROLLER_STACK    (2 * SMALL_STACK)
/* 时钟任务栈大小 */
#define CLOCK_TASK_STACK    SMALL_STACK
/* 系统任务，内核堆栈 */
#define SYS_TASK_STACK      SMALL_STACK
/* 虚拟硬件任务，使用内核堆栈，但它没有空间 */
#define	HARDWARE_STACK	    0
/* 内存管理器服务，使用内核堆栈 */
#define MM_STACK            (5 * SMALL_STACK)
/* 文件系统服务，使用内核堆栈 */
#define FS_STACK            (5 * SMALL_STACK)
/* 飞彦扩展器服务，使用内核堆栈 */
#define FLY_STACK           (5 * SMALL_STACK)
/* 起源进程，使用内核堆栈 */
#define ORIGIN_STACK        NORMAL_STACK

/* 所有系统任务的栈空间大小 */
#define TOT_TASK_STACK_SPACE    (TTY_TASK_STASK + CLOCK_TASK_STACK + (NR_CONTROLLERS * CONTROLLER_STACK) + \
                                 IDLE_TASK_STACK + SYS_TASK_STACK + MM_STACK + FS_STACK + FLY_STACK + ORIGIN_STACK \
                                 )

/* 为系统任务表的所有表象分配空间 */
PUBLIC TaskTab tasktab[] = {
        /* 终端任务，必须存在 */
        { &tty_task, TTY_TASK_STASK, "TTY_TASK"  },

        /* 时钟任务，它很重要，需要第二个运行 */
        { &clock_task, CLOCK_TASK_STACK, "CLOCK_TASK" },

        /* 控制器任务，例如：硬盘任务 */
#if NR_CONTROLLERS >= 3
        { nop_task,		CONTROLLER_STACK,	"(C2)"		},
#endif
#if NR_CONTROLLERS >= 2
        { nop_task,		CONTROLLER_STACK,	"(C1)"		},
#endif
#if NR_CONTROLLERS >= 1
        { &nop_task,		CONTROLLER_STACK,	"(C0)"		},
#endif

        /* 闲置任务 */
        { &idle_task, IDLE_TASK_STACK, "IDLE_TASK" },

        /* 系统任务 */
        { &system_task, SYS_TASK_STACK, "SYS_TASK" },

        /* 硬件任务，没有任何数据和正文，占个位置 - 用作判断硬件中断 */
        { 0, HARDWARE_STACK, "HARDWARE" },

        /* 内存管理器 */
        { &mm_main, MM_STACK, "MM"		},
        /* 文件系统 */
        { &fs_main, FS_STACK, "FS"		},
        /* 飞彦扩展管理器 */
        { &fly_main, FLY_STACK, "FLY"		},
        /* 起源进程 */
        { &origin_main, ORIGIN_STACK, "ORIGIN"		}
};

/* 驱动任务定义 */
typedef struct driver_tab {
    char name[8];
    task_t *driver;
} DriverTab;

/* 从驱动程序名称到驱动程序功能的映射，例如 “bios”-> bios_wini */
PRIVATE DriverTab driver_tab[] = {
#if ENABLE_AT_WINI
        /* 默认硬盘驱动 */
        {"AT_HD0", &at_winchester_task },
#endif
};
#define FIRST_DRIVER_TASK   NR_CONTROLLERS + 1   /* 第一个驱动任务所处任务表中的位置， */

/*===========================================================================*
 *                                   map_drivers                                    *
 *                                   映射驱动程序              *
 *===========================================================================*/
PUBLIC void map_drivers(void){
    /* 将驱动程序映射到控制器，并将任务表更新为该选择。
     * 这段程序不难理解，只要你多加琢磨。
     */

    DriverTab *driver;
    char *driver_name;
    TaskTab *task;
    int t = FIRST_DRIVER_TASK, c = 0;

    task = &tasktab[t];
    driver = &driver_tab[c];
    for(; c < NR_CONTROLLERS; c++, t--){
        driver_name = driver->name;
        task->initial_pc = driver->driver;
        strcpy(task->name, driver_name);
        task--;
        driver++;
    }
}

/* 所有系统任务堆栈的堆栈空间。 （声明为（char *）使其对齐。） */
PUBLIC char *task_stack[TOT_TASK_STACK_SPACE / sizeof(char *)];

/* 虽然已经尽量将所有用户可设置的配置消息单独放在include/flyanx/config.h中,但是在将tasktab的大小与
 * NR_TASKS相匹配时仍可能会导致错误。在table.c的结尾处使用一个小技巧对这个错误进行检测。方法是在这里声明
 * 一个dummy_tasktab,声明的方式是假如发生了前述的错误,则dummy_tasktab的大小是非法的,从而导致编译错误。
 * 由于哑数组声明为extern,此处并不为它分配空间(其他地方也不为其分配空间)。因为在代码中任何地方都不会
 * 引用到它,所以编译器不会受任何影响。
 *
 * 简单解释：减去的是MM、FS、FLY和ORIGIN，这些都不属于系统任务
 */
#define NKT (sizeof(tasktab) / sizeof(struct tasktab_s) - (ORIGIN_PROC_NR + 1))
//#define NKT ( sizeof(tasktab) / sizeof(struct tasktab_s) - 1 )

extern int dummy_tasktab_check[NR_TASKS == NKT ? 1 : -1];

/* 下面给系统服务分配高速缓冲区 */
/* 6MB~7MB: 文件系统使用 */
PUBLIC u8_t *fs_buffer  = (u8_t*)0x600000;
PUBLIC const int FS_BUFFER_SIZE = 0x100000;

/* 7MB~8MB: 内存管理器使用 */
PUBLIC u8_t *mm_buffer  = (u8_t*)0x700000;
PUBLIC const int MM_BUFFER_SIZE = 0x100000;

/* 8MB~9MB: 飞彦拓展管理器使用 */
PUBLIC u8_t *fly_buffer  = (u8_t*)0x800000;
PUBLIC const int FLY_BUFFER_SIZE = 0x100000;

/* 9MB~11MB: 日志使用（DEBUG） */
PUBLIC char *log_buffer  = (char *)0x900000;
PUBLIC const int LOG_BUFFER_SIZE = 0x100000;
PUBLIC char *log_disk_buffer  = (char *)0xA00000;
PUBLIC const int LOG_DISK_BUFFER_SIZE = 0x100000;



