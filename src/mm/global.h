/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#ifndef _MM_GLOBAL_H
#define _MM_GLOBAL_H

/* 当该文件被包含在定义了宏_TABLE的 table.c中时，宏EXTERN的定义被取消。 */
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

/* 全局变量 */
EXTERN int procs_in_use;                /* 有多少进程被标记为正在使用 */
EXTERN struct mm_process_s *curr_mp;    /* 指向一个当前正在处理的进程 */
extern u8_t *mm_buffer;                 /* 内存管理器使用的高速缓冲区首地址 */
extern const int MM_BUFFER_SIZE;        /* 高速缓冲区长度 */

/* 外界的调用参数在这 */
EXTERN Message mmsg_in;     /* 传入的消息保存在这 */
EXTERN pid_t mm_who;         /* 调用进程的进程号 */
EXTERN int mm_call;

extern _PROTOTYPE( int (*mm_call_handlers[]), (void) );    /* 系统调用处理函数在这里 */
extern char *core_name;     /* 生成核心映像的文件名
                             * 在进程非正常结束时，我们把进程的映像写到一个内核文件中，core_name
                             * 定义了这个文件的文件名。
                             */

/* 导入内核变量，这些变量已经由内核初始化并设置，现在只需要感知一下就可以了 */
extern phys_clicks kernel_base; /* 内核所在基地址 */
extern phys_clicks kernel_limit;/* 内核界限 */
extern BootParams boot_params;   /* 系统启动参数 */


#endif //_MM_GLOBAL_H
