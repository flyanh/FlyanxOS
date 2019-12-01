/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含flyanx的主程序。
 *
 * 例程main()初始化系统，并通过设置进程表，中断向量和安排要运行的每个任务来初始化自身
 * 来开始运行flyanx内核。
 *
 * 该文件的入口点是：
 *  - main:         flyanx的主程序
 *  - panic:        由于致命错误而中止flyanx
 */

#include "kernel.h"
#include <signal.h>
#include <unistd.h>
#include <a.out.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "protect.h"
#include "process.h"

/*===========================================================================*
 *                                   main                                    *
 *     Flyanx 内核的主体，从这里开始，我们的内核的90%可以由c语言开发了              *
 *===========================================================================*/
PUBLIC int main(){

    /* 主要做一些初始化工作，最重要的莫过于建立进程表 */
    /* 调用interrupt_init来初始化中断控制硬件
     * 该操作之所以放在这里是因为此前必须知道机器类型,因为完全依赖于硬件,所以该过程放在一个独立文件中。
     * 参数(1)，代表为Flyanx内核执行初始化，若是参数(0)则再次初始化硬件使其回到原始状态。
     *
     * 中断在这里初始化，那么则说明只有main函数真正执行起来，中断机制才能成功构建，如果在main函数之前
     * 产生了一个中断，那么将会没有效果。
     */
    interrupt_init(1);

    /* 进程表的所有表项都被标志为空闲;
     * 用于加快进程表访问的pproc_addr数组被循环的进行初始化。
     */
    register Process *proc;
    register int t;
    for(proc = BEG_PROC_ADDR, t = -NR_TASKS; proc < END_PROC_ADDR; ++proc, ++t){
        proc->nr = t;   /* 进程索引号 */
        /* 这一句等同于 -> p_process_addr[NR_TASK + t] = rp */
        (p_process_addr + NR_TASKS)[t] = proc;
    }

    /* 解析任务表中的驱动程序选择子映射 */
    map_drivers();

    /* 初始化多进程支持
     * 为系统任务和服务设置进程表，内核任务的堆栈被初始化为数据空间中的数组。
	 */
    TaskTab* p_task = tasktab;      /* 系统任务表的头指针 */
    reg_t k_task_stack_base = (reg_t) task_stack; /* 任务总栈栈顶，即基地址 */
    u16_t ldt_selector = SELECTOR_LDT_FIRST;    /* LDT选择子 */
    u8_t		privilege;		/* CPU权限 */
    u8_t		rpl;			/* 段访问权限 */
    int rs = get_kernel_map(&kernel_base, &kernel_limit);   /* 取内核映像 */
    if(rs != OK) {
        /* 如果获取内核映像错误，那么用户进程就不能启动起来，打印错误并死机（死循环，
         * 这里键盘驱动还没有启动，不能使用painc错误宕机）
         */
        disp_str("get kernel map failed, please trun off pc.\n");
        for(;;);
    }
    // 初始化进程表
    for(t = -NR_TASKS; t <= LOW_USER; ++t){
        proc = proc_addr(t);                /* t是进程插槽号 */
        p_task = &tasktab[t + NR_TASKS];    /* 得到任务 */
        strcpy(proc->name, p_task->name);	/* 进程名称 <-- 任务名称 */
        if(t < 0){  /* 任务 */
            if(p_task->stack_size > 0){
                /* 如果任务堆栈空间大于0，设置进程的堆栈保护字 */
                proc->stack_guard_word = (reg_t *) k_task_stack_base;
                *proc->stack_guard_word = SYS_TASK_STACK_GUARD;
            }
            /* 设置任务堆栈 */
            k_task_stack_base += p_task->stack_size;
            proc->regs.sp = k_task_stack_base;
            /* 设置任务权限 */
            privilege = rpl = proc->priority = PROC_PRI_TASK;
            /* 设置系统进程的pid，其是一个对flyan有意义的魔数，
             * 我们的启动参数魔数也是它。
             */
            proc->pid = SYSTEM_PID;

            /* 设置任务的LDT */
            proc->ldt_selector = ldt_selector;
            proc->ldt[TEXT] = gdt[SELECTOR_KERNEL_CS >> 3];
            proc->ldt[DATA] = gdt[SELECTOR_KERNEL_DS >> 3];
            /* 改变DPL */
            proc->ldt[TEXT].access = DA_C     | privilege << 5;
            proc->ldt[DATA].access = DA_DRW   | privilege << 5;
        } else {    /* 服务或用户进程 */
            /* 设置服务和用户进程堆栈 */
            k_task_stack_base += p_task->stack_size;
            proc->regs.sp = k_task_stack_base;
            /* 设置权限 */
            privilege = rpl = proc->priority = t < LOW_USER ? PROC_PRI_SERVER : PROC_PRI_USER;
            /* 设置系统服务器的pid。 */
            proc->pid = SERVER_PID;
            /* 设置服务和用户进程的LDT */
            proc->ldt_selector = ldt_selector;
            init_seg_desc(&proc->ldt[TEXT],
                    0,  /* 入口点之前的字节对于服务或用户进程没有用（浪费），所以没关系 */
                    (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT,
                    DA_32 | DA_LIMIT_4K | DA_C | privilege << 5
            );
            init_seg_desc(&proc->ldt[DATA],
                          0,  /* 入口点之前的字节对于服务或用户进程没有用（浪费），所以没关系 */
                          (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT,
                          DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5
            );
        }

        /* 初始化寄存器值，上下文环境 */
        proc->regs.cs   = (TEXT * DESCRIPTOR_SIZE) | SA_TIL | rpl;
        proc->regs.ds	=       /* 这里注意：ds es fs ss要相等，因为 */
            proc->regs.es	=   /* 在C语言编译器看它们是等同的。 */
                proc->regs.fs	=
                    proc->regs.ss	= (DATA * DESCRIPTOR_SIZE) | SA_TIL | rpl;
        proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
        proc->regs.pc   = (reg_t) p_task->initial_pc;
        proc->regs.psw = is_task_proc(proc) ? INIT_TASK_PSW : INIT_PSW;

        /* 如果进程不是IDLE或HARDWARE，就调用lock_ready()
         *
         * IDLE和HARDWARE这两项进程是不按通常方式调度的进程，
         * IDLE是一个空循环,在系统中无其他进程就绪时就运行它。
         * HARDWARE进程用于计费-它记录中断服务所用的时间。
         */
        if (!is_idle_hardware(t)) lock_ready(proc);	    /* 闲置任务, 硬件任务从不就绪，除非没有任何进程可以运行才会就绪闲置任务 */
        proc->flags = 0;            /* 进程刚初始化，处于不堵塞状态，所以标志值是全0 */

        ldt_selector += DESCRIPTOR_SIZE;
    }

    process[NR_TASKS + ORIGIN_PROC_NR].pid = 0;     /* 源进程id为0 */

    /* 设置消费进程，它需要一个初值。因为系统闲置刚刚启动，所以此时闲置进程是一个最合适的选择。
     * 随后在调用下一个函数lock_hunter进行第一次进程狩猎时可能会选择其他进程。
     */
    bill_proc = proc_addr(IDLE_TASK);
    proc_addr(IDLE_TASK)->priority = PROC_PRI_IDLE;
    lock_hunter();  /* 让我们看看，有什么进程那么幸运的被抓出来执行 */


    /* 最后,main的工作至此结束。在许多C程序中main是一个循环,但在Flyanx核心中,
     * 它的工作到初始化结束为止。restart的调用将启动第一个任务,控制权从此不再返回到main。
     *
     * restart作用是引发一个上下文切换,这样curr_proc所指向的进程将运行。
     * 当restart执行了第一次时,我们可以说Flyanx正在运行-它在执行一个进程。
     * restart被反复地执行,每当系统任务、服务器进程或用户进程放弃运行机会挂
     * 起时都要执行restart,无论挂起原因是等待输入还是在轮到其他进程运行时将控制器转交给它们。
     */
    restart();
}

/*===========================================================================*
 *                                   panic                                   *
 *                              系统无法继续运行                               *
 *===========================================================================*/
PUBLIC void panic(msg, errno)
_CONST char *msg;
int errno;
{
    /* 当系统发现无法继续运行下去的故障时将调用它。典型的如无法读取一个很关键的数据块、
     * 检测到内部状态不一致、或系统的一部分使用非法参数调用系统的另一部分等。
     * 这里对printf的调用实际上是调用printk,这样当正常的进程间通信无法使用时核心仍能够
     * 在控制台上输出信息。
     */

    if(msg != NULL){
        printf("\nFlyanx Kernel panic: %s", msg);
        if(errno != NO_NUM) printf(" %d", errno);
        printf("\n");
    }
    wreboot(RBT_REBOOT);
}

/*===========================================================================*
 *                                   raw_clear_screen                          *
 *                                     清屏                               *
 *===========================================================================*/
PUBLIC void raw_clear_screen(){
    /* 现在屏幕实在是有点脏了，所以我们可以清理一下屏幕
     * 该例程仅限内核调试使用，用户自然有用户该使用的例程。
     */
    display_position = 0;
    int i;
    for(i = 0;i < (160 * 25 + 2 * 80); i++){
        disp_str(" ");
    }
    display_position = 160;
}

/*===========================================================================*
 *                                   ok_print                          *
 *                                  打印成功信息                               *
 *===========================================================================*/
PUBLIC void ok_print(char* msg, char* ok){
    /* 当内核完成一项重大的工作时，需要显示一个成功信息，本函数完成这个功能
     * 可以打印类似于"message          [ OK ]"的信息，且可以占满整行
     */

    printf("%s", msg);
    int msg_len = strlen(msg);
    int ok_len = strlen(ok);
    for(; msg_len < (80 - 4 - ok_len); msg_len++){
        printf(" ");
    }
    printf("[ %s ]", ok);
}



