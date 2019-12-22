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
#include <fcntl.h>
#include <flyanx/common.h>
#include "protect.h"
#include "process.h"

_PROTOTYPE( int sprintf, (char *_buf, const char *_fmt, ...) );

/*===========================================================================*
 *                                   main                                    *
 *     Flyanx 内核的主体，从这里开始，我们的内核的90%可以由c语言开发了              *
 *===========================================================================*/
PUBLIC void flyanx_main(void){

    /* 主要做一些初始化工作，最重要的莫过于建立进程表 */

    /* 调用interrupt_init来初始化中断控制硬件
     * 该操作之所以放在这里是因为此前必须知道机器类型,因为完全依赖于硬件,所以该过程放在一个独立文件中。
     * 参数(1)，代表为Flyanx内核执行初始化，若是参数(0)则再次初始化硬件使其回到原始状态。但0还没有实
     * 现，它被放在了未来的计划中。
     * 注意：中断在这里初始化，那么说明只有main函数真正执行起来，中断机制才能成功构建，如果在内核主函数
     * 之前产生了一个中断，那么将会没有任何效果。
     */
    interrupt_init(1);

    /* 进程表的所有表项都被标志为空闲;
     * 用于加快进程表访问的pproc_addr数组被循环的进行初始化。
     */
    register Process *proc;
    register int t;
    for(proc = BEG_PROC_ADDR, t = -NR_TASKS; proc < END_PROC_ADDR; ++proc, ++t){
        proc->nr = t;   /* 进程索引号 */
        /* 这一句等同于 -> p_process_addr[NR_TASK + t] = proc */
        (p_process_addr + NR_TASKS)[t] = proc;
    }

    /* 映射任务表中的驱动程序选择。 */
    map_drivers();

    /* 初始化进程表之前首先需要获取内核的映像信息 */
    if(get_kernel_map(&kernel_base, &kernel_limit) != OK) {
        /* 如果获取内核映像错误，那么用户进程就不能启动起来，打印错误并死机（死循环，
         * 这里键盘驱动还没有启动，不能使用painc进行错误宕机）
         */
        disp_str("get kernel map failed, stop flyanx.\n");
        for(;;);
    }

    /* 初始化多进程支持
     * 为系统任务和服务设置进程表，内核任务的堆栈被初始化为数据空间中的数组。
	 */

    TaskTab*    p_task;         /* 系统任务表的头指针 */
    reg_t       k_task_stack_base = (reg_t)task_stack;  /* 任务总栈 */
    u8_t		privilege;		/* CPU权限 */
    u8_t		rpl;			/* 段访问权限 */
    for(t = -NR_TASKS; t <= LOW_USER; ++t){
        proc = proc_addr(t);                        /* t是进程插槽号 */
        p_task = &tasktab[t + NR_TASKS];            /* 得到任务 */
        strcpy(proc->name, p_task->name);	        /* 进程名称 <-- 任务名称 */
        k_task_stack_base += p_task->stack_size;    /* 任务堆栈指针 */
        if(t < 0){  /* 任务 */
            if(p_task->stack_size > 0){
                /* 如果任务堆栈空间大于0，设置进程的堆栈保护字 */
                proc->stack_guard_word = (reg_t *) k_task_stack_base;
                *proc->stack_guard_word = SYS_TASK_STACK_GUARD;
            }
            /* 设置任务权限 */
            proc->priority = PROC_PRI_TASK;
            privilege = rpl = TASK_PRIVILEGE;
            /* 设置系统进程的pid，其是一个对flyan有意义的魔数，
             * 我们的启动参数魔数也是它。
             */
            proc->pid = SYSTEM_PID;
        } else {    /* 服务或用户进程 */
            /* 设置权限，只是为了不混乱，但其实这两句很啰嗦，可以合并。 */
            proc->priority = t < LOW_USER ? PROC_PRI_SERVER : PROC_PRI_USER;
            privilege = rpl = t < LOW_USER ? SERVER_PRIVILEGE : USER_PRIVILEGE;
            /* 设置系统服务器的pid。 */
            proc->pid = SERVER_PID;
        }

        /* ================= 初始化进程的LDT信息 ================= */
        if(proc->priority != PROC_PRI_USER){
            /* ================= 设置任务和服务的LDT ================= */
            proc->ldt[CS_LDT_INDEX] = gdt[SELECTOR_KERNEL_CS / DESCRIPTOR_SIZE];
            proc->ldt[DS_LDT_INDEX] = gdt[SELECTOR_KERNEL_DS / DESCRIPTOR_SIZE];
            /* ================= 改变DPL描述符特权级 ================= */
            proc->ldt[CS_LDT_INDEX].access = DA_C     | privilege << 5;
            proc->ldt[DS_LDT_INDEX].access = DA_DRW   | privilege << 5;
            /* 设置任务和服务的内存映射，它们是都是0 */
            proc->map.base = proc->map.size = 0;
        } else {
            /* ================= 设置起源进程的LDT ================= */
            init_seg_desc(&proc->ldt[CS_LDT_INDEX],
                          0,  /* 入口点之前的字节虽然对于起源进程没有用（浪费掉了），但是没关系，这样足够简单 */
                          (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT,
                          DA_32 | DA_LIMIT_4K | DA_C | privilege << 5
            );
            init_seg_desc(&proc->ldt[DS_LDT_INDEX],
                          0,  /* 入口点之前的字节虽然对于起源进程没有用（浪费掉了），但是没关系，这样足够简单 */
                          (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT,
                          DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5
            );
            /* ================= 计算起源进程的内存映像信息 ================= */
            /* 首先是正文段的 */
            SegDescriptor *sdp = &proc->ldt[CS_LDT_INDEX];
            int ldt_text_limit, ldt_data_limit, ldt_data_base, ldt_data_size;
            /* 代码段基址 */
            proc->map.base = reassembly(sdp->base_high, 24,
                                        sdp->base_middle, 16,
                                        sdp->base_low);
            /* 代码段界限，单位以段粒度计算，要么字节要么4KB */
            ldt_text_limit = reassembly(0, 0,
                                        (sdp->granularity & 0xF), 16,
                                        sdp->limit_low);
            /* 代码段大小 */
            proc->map.size = ((ldt_text_limit + 1) *
                              ((sdp->granularity & (DA_LIMIT_4K >> 8)) ? 4096 : 1));
            /* 然后是数据段，堆栈段共用这一块区域。 */
            sdp = &proc->ldt[DS_LDT_INDEX];
            /* 数据段&堆栈段基址 */
            ldt_data_base = reassembly(sdp->base_high, 24,
                                       sdp->base_middle, 16,
                                       sdp->base_low);
            /* 数据段&堆栈段界限 */
            ldt_data_limit = reassembly((sdp->granularity & 0xF), 16,
                                        0, 0,
                                        sdp->limit_low);
            /* 数据段&堆栈段大小 */
            ldt_data_size = ((ldt_data_limit + 1) *
                             ((sdp->granularity & (DA_LIMIT_4K >> 8)) ? 4096 : 1));
            /* 我们并不加以细分正文、数据以及堆栈段，所以TEXT和DATA段的内存映像应该是相等的，
             * 如果不一致，那么系统就不必要继续向下启动了，它有可能为以后带来隐患。
             * 当然，我希望这是暂时的，因为这是我的毕业设计，这个系统应该功能齐全但是足够的
             * 简单，不至于耗费我太多的时间。
             */
            if((proc->map.base != ldt_data_base ) ||
               (ldt_text_limit != ldt_data_limit) ||
               (proc->map.size != ldt_data_size )){
                disp_str("shit! TEXT segment not equals DATA & STACK segment, stop flyanx.\n");
                for(;;);
            }
        }
        /* 初始化寄存器值，上下文环境 */
        proc->regs.cs   = (CS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl;
        proc->regs.ds	=       /* 这里注意：ds es fs ss要相等，因为 */
            proc->regs.es	=   /* 在C语言编译器看来它们是等同的。 */
            proc->regs.fs	=
            proc->regs.ss	= ((DS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl);
        proc->regs.gs	= ((SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl); /* gs被我们用来放置显存段 */
        proc->regs.pc   = (reg_t) p_task->initial_pc;
        proc->regs.esp  = (reg_t) k_task_stack_base;      /* 设置堆栈指针 */
        proc->regs.psw = is_task_proc(proc) ? INIT_TASK_PSW : INIT_PSW;
        /* 如果进程不是IDLE或HARDWARE，就调用lock_ready()
         *
         * IDLE和HARDWARE这两项进程是不按通常方式调度的进程，
         * IDLE是一个空循环,在系统中无其他进程就绪时就运行它。
         * HARDWARE进程用于计费-它记录中断服务所用的时间。
         */
        if (!is_idle_hardware(t)) lock_ready(proc);	    /* 闲置任务, 硬件任务从不就绪，除非没有任何进程可以运行才会就绪闲置任务 */
        proc->flags = 0;            /* 进程刚初始化，处于可运行状态，所以标志值的每位都是0 */
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
extern bool assert_panic;
PUBLIC void panic(msg, err_no)
_CONST char *msg;
int err_no;
{
    /* 当系统发现无法继续运行下去的故障时将调用它。典型的如无法读取一个很关键的数据块、
     * 检测到内部状态不一致、或系统的一部分使用非法参数调用系统的另一部分等。
     * 这里对printf的调用实际上是调用printk,这样当正常的进程间通信无法使用时核心仍能够
     * 在控制台上输出信息。
     */

    /* OK，蓝屏吧（致敬XP）
     * 但如果是从断言进入的本例程，就不做蓝屏操作了，因为断言例程已经做过了。
     * */
    if(assert_panic != TRUE){
        blue_screen();
    }

    if(msg != NULL){
        printf("\nFlyanx Kernel panic: %s", msg);
        if(err_no != NO_NUM) printf(" %d", err_no);
        printf("\n");
    }
    wreboot(RBT_PANIC);
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



