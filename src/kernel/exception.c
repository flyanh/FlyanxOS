/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件只包含异常处理程序
 * 异常处理程序例程被kernel.asm中的异常处理代码的汇编语言部分调用;
 */

#include "kernel.h"
#include <signal.h>
#include "process.h"

/* 异常处理结构
 */
typedef struct exception_s{
   char *msg;           /* 异常信息 */
   int sig_num;         /* 异常处理信号（发送给用户进程） */
} Exception;

PRIVATE Exception ex_data[] = {
        "#DE Divide Error", SIGKILL,
        "#DB RESERVED", SIGKILL,
        "—  NMI Interrupt", SIGKILL,
        "#BP Breakpoint", SIGKILL,
        "#OF Overflow", SIGKILL,
        "#BR BOUND Range Exceeded", SIGKILL,
        "#UD Invalid Opcode (Undefined Opcode)", SIGKILL,
        "#NM Device Not Available (No Math Coprocessor)", SIGKILL,
        "#DF Double Fault", SIGKILL,
        "    Coprocessor Segment Overrun (reserved)", SIGKILL,
        "#TS Invalid TSS", SIGKILL,
        "#NP Segment Not Present", SIGKILL,
        "#SS Stack-Segment Fault", SIGKILL,
        "#GP General Protection", SIGKILL,
        "#PF Page Fault", SIGKILL,
        "—  (Intel reserved. Do not use.)", SIGKILL,
        "#MF x87 FPU Floating-Point Error (Math Fault)", SIGKILL,
        "#AC Alignment Check", SIGKILL,
        "#MC Machine Check", SIGKILL,
        "#XF SIMD Floating-Point Exception", SIGKILL,
};

/*==========================================================================*
 *				exception_handler				    *
 *				异常处理
 *
 * unsigned vec_nr;         异常向量
 * int errno;              异常代码
 * int eip;               发生异常后的eip
 * int cs;                cs，同上
 * int eflags;             eflags，同上
 *==========================================================================*/
PUBLIC void exception_handler(int vec_nr, int err_no)
{
    /* 处理所有的异常
    *
    * 异常导致的结果各不相同，有的被忽略、有的导致系统崩溃、有的导致向进程发消息。
    * 在处理时，由用户进程引起的异常被转换成信号。用户自己编写的程序是可能有错的，但由操
    * 作系统本身引起的异常则表明发生了严重错误，并产生了不可恢复的故障,这时可以panic宕机。
    */

    /* ex_data数组存放当发生严重错误时应打印的错误信息和对每种异常应向用户进程发送的信号。 @TODO 发送的信号未完成 */
    register Exception *ep;
    Process *saved_proc;

    // 保存当前运行的进程，因为它可能会被调试语句更改。
    saved_proc = curr_proc;

    ep = &ex_data[vec_nr];

    /* 一些机器上的不可屏蔽中断会产生2号伪中断异常 */
    if(vec_nr == 2){
        printf("got spurious NMI!\n");
        return;
    }

    /* 如果没发生中断重入，且当前运行的进程是用户进程，我们发一个信号给它@TODO */
//    if(kernel_reenter == 0 && is_user_proc(saved_proc)){
////        interrupt_unlock(); /* 这个调用是受保护的就像sys_call() */
//        return;
//    }

    /* 如果上面两个条件都没有满足，说明这是内核代码的的异常，它不应该发生的...
     * 但没办法，我们打印这些异常信息，最后使用panic结束内核的运行。
     */
    blue_screen();
    if(ep->msg == NIL_PTR){
        printf("\nException %d no exist...\n", vec_nr);
    } else {
        printf("\n%s\n", ep->msg);
    }
    printf("Process number %d, pc = 0x%04x:0x%08x\n",
           saved_proc->nr,
           (unsigned) saved_proc->regs.cs,
           (unsigned) saved_proc->regs.pc);
    if(err_no != 0xFFFFFFFF){
        printf("ERROR CODE: %d\n", err_no);
    }
    wreboot(2);
//    panic("Exception in system code...", NO_NUM);
}

