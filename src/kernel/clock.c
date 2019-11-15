/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含时钟相关，最主要的是时钟系统任务（时钟驱动程序）
 * 时钟任务接收留个参数的消息类型：
 *  1.HARD_INT
 *  2.GET_UPTIME（获取时钟的运行时间（滴答））
 *  3.GET_TIME（获得真实时间（秒））
 *  4.SET_TIME(设置以秒计时的新时间)
 *  5.SET_ALARM(设置一个闹钟)
 *  6.SET_SYNC_ALARM（设置一个同步闹钟)
 *
 * 其中，HARD_INT是当发生时钟中断并有工作可做的时候发往驱动程序的消息，例如当必须发出
 * 一个报警或一个进程已经运行了很长时间的情况。
 *  GET_UPTIME用来获取从启动开始后的时钟滴答数时间，GET_TIME则返回从1970年1月1日上午
 * 12:00开始以秒的当前时间。SET_TIME设置实际时间，它可以被超级用户激活。
 *  SET_ALARM允许一个进程设置一个定时器，该定时器经过一个指定滴答计数的时间间隔后，将
 * 引起某事件的发生。当用户进程执行ALARM调用时，它向存储器发一条消息，管理器再把消息发
 * 往时钟驱动程序，当报警发生时。时钟驱动程序向存储管理器发回一条消息，然后由它向相关进
 * 程发回一个信号。当然了，需要启动监视定时器的任务也使用SET_ALARM，当定时器到达时，就
 * 简单地调用提交的过程。但驱动程序并不知道调用的过程做什么。
 * SET_SYNC_ALARM和SET_ALARM相似，但是用来设置同步闹钟(Synchronous Alarm)。同步闹钟
 * 发送一条消息到进程而不是产生一个信号或调用一个过程。同步报警任务负责向各个需要消息的进
 * 程分派消息。
 */

#include "kernel.h"
#include <signal.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"

/* 常量定义 */
#define ONE_TICK_MILLISECOND    10          /* 一个滴答有多少毫秒，这个初始化时就决定了 */
#define SCHEDULE_MILLISECOND    100         /* 用户进程调度的频率（毫秒） */
#define SCHEDULE_TICKS          (SCHEDULE_MILLISECOND / ONE_TICK_MILLISECOND)  /* 用户进程调度的频率（滴答） */
#define ONE_SEC_TICKS           (1000 / ONE_TICK_MILLISECOND);    /* 一秒有多少个滴答 */

/* 时钟, 8253 / 8254 PIT (可编程间隔定时器)参数 */
#define TIMER0			    0x40 	    /* I/O port for timer channel 0 */
#define TIMER_MODE		    0x43	    /* I/O port for timee mode control */
#define RATE_GENERATOR	    0x34	    /* 00-11-010-0
								         * Counter0 - LSB the MSB - rate generator - binary
								         */
#define TIMER_FREQ		    1193182L    /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  (TIMER_FREQ / HZ)  /* initial value for counter*/
#define CLOCK_ACK_BIT	    0x80	        /* PS/2 clock interrupt acknowledge bit */

/* 时钟任务变量 */
PRIVATE clock_t realticks;      /* 系统开机后运行的时间(滴答数)，也是开机后时钟运行的时间 */
PRIVATE time_t realtime;       /* 系统开机后运行的时间(s)，也是开机后时钟运行的时间 */
PRIVATE time_t boot_t;          /* 系统启动时间(s) */
PRIVATE clock_t next_alarm;     /* 下一个信号或闹钟发生的时刻 */
PRIVATE Message msg;            /* 发送和接收的消息缓冲区 */
PRIVATE int watchdog_proc;      /* 看门狗：存放闹钟需要调用的函数数组 */

/* 由中断处理程序更改的变量 */
PRIVATE clock_t  pending_ticks; /* 中断挂起的时间 */
PRIVATE int schedule_ticks = SCHEDULE_TICKS;     /* 用户进程调度时间，当为0时候，进行程序调度 */
PRIVATE Process *prev_proc;     /* 最后使用时钟任务的用户进程 */

/* 本地函数原型 */
FORWARD _PROTOTYPE( void init_clock, (void) );

/*===========================================================================*
 *				clock_task				     *
 *				时钟任务/时钟驱动程序
 *===========================================================================*/
PUBLIC void clock_task(){
    /* 时钟任务的主程序是一个无限循环，重复获取消息，再根据消息类型进行处理，然后发送一个
     * 应答(CLOCK_INT除外)。每种类型的消息由不同的过程处理。
     *
     * 在这顺便提一句，系统的运行时间只有当接收到外界的消息后，才会进行更新。这个机制可以
     * 使计算机功耗更低，但是在任意瞬间，realtime可能是不准确的，但这种机制能保证当有需要
     * 时，时间永远是准确的。如果你的手表当你看它的时候是准时的，而当你不看它的时候，它却
     * 不准时，这有关系吗？
     */
    printf("Clock task start runing...\n");

    int mess_type;      /* 消息类型，通过其判断用户需要什么服务 */

    /* 初始化时钟任务 */
    init_clock();
    int i = 0;
    /* 时钟任务主循环，一直得到工作，处理工作，回复处理结果 */
    while (TRUE){
        /* 从外界得到一条消息 */

        /* 已经得到用户发来的消息请求，现在开始校准时间，记得先锁住中断 */
        interrupt_lock();
        realticks += pending_ticks;  /* 加上从上次计时后过去的滴答数到realtime上 */
        realtime = realticks / ONE_SEC_TICKS;   /* 计算按秒数计算的真实时间 */
        pending_ticks ^= pending_ticks; /* 好了，上次计时后过去的滴答数可以清零了 */
        interrupt_unlock();

    }

}

/*==========================================================================*
 *				milli_delay				    *
 *				毫秒级别延迟
 *==========================================================================*/
PUBLIC void milli_delay(millisec)
time_t millisec;
{
    /**
     * 毫秒级的延迟函数
     *
     * 这个函数是为需要极短延迟的任务提供的。它用C语言编写，没有引入任何硬件相关性，
     * 但是使用了一种人们只有在低级汇编语言中找到的技术。它把计数器初始化为零，然后
     * 对其快速轮询直到到达指定的值。
     * 这种忙等待技术一般应该避免，但是，实现的必要性要求不能遵循一般的规则。
     *
     * 注意：毫秒级，也只能达到10毫秒内的延迟
     * 因为10ms是因为我们设置系统发生时钟中断的时间间隔，我们没法做到更精确于此值的
     * 延迟，但是对于所有需要精确的延迟技术的函数，这已经足够了。
     * 还有一点，本函数不能被时钟任务自己调用，因为时钟任务负责更新时间信息，如果这
     * 样做了，将会导致的灾难即是，本函数一直循环，但是它堵塞了时钟任务，导致时间无
     * 法更新，那么就会一直堵塞下去！暂时还未找到更好的实现方法，可能将在下一个版本
     * 解决。
     */

    // 首先，我们得到当前的时钟滴答数
    unsigned long ticks = realticks;
    unsigned long enter_millis = 0;       /* 进入延迟函数的总时间 */
    while( enter_millis < millisec ){     /* 只要还没达到延迟时间的要求，就继续循环 */
        enter_millis = (realticks - ticks) * 10;
    }
}

/*===========================================================================*
 *				clock_handler				     *
 *				时钟中断处理
 *===========================================================================*/
PRIVATE int clock_handler(irq)
int irq;
{
    /**
     * 时钟中断处理程序
     *
     * 中断发生时，并不立即更新realtime。中断例程只维护变量pending_ticks计数器，并且完成
     * 象把当前的滴答数计入到进程中并减少当前的时间片等简单工作，仅当必须完成更复杂的任务时，
     * 才向时钟任务发消息。这是和Flyanx任务通过消息通信思想的一种妥协，也是对消耗CPU时间的
     * 一种让步。在一个慢速的机器上，人们发现，和在每个时钟中断向中断任务发消息的工作方式相
     * 比，按这种方式工作可以提高15%的速度。
     */
     register Process *proc;
     register u16_t ticks;
     clock_t now;
     if(ps_mca){
         /* 确认 PS/2 时钟中断 */
         out_byte(PORT_B, in_byte(PORT_B) | CLOCK_ACK_BIT);
     }

     /* 更新用户和系统会计时间
      *
      * 首先为当前进程的用户时间充电（增加用户使用CPU的时间）。
      * 如果当前进程的用户时间是不可计费的，则说明系统任务正在运行使用，我们
      * 则对其调用的子进程的系统时间进行充电。
      * 因此，不可计费任务的用户时间就是可计费用户的系统时间。
      */
     if(kernel_reenter != 0) {
         // 如果发生了中断冲入，说明当前进程是硬件中断触发的核心代码（中断处理程序）
         proc = proc_addr(HARDWARE);
     } else{
         // 没发生中断冲入，当前进程就是curr_proc指向的进程
         proc = curr_proc;
     }
     ticks = lost_ticks + 1;    /* 时钟滴答数，等于时钟任务外的时钟滴答数 + 本次的滴答数(1) */
     lost_ticks = 0;            /* 时钟任务外的时钟滴答数归零 */
     proc->user_time += ticks;  /* 充电 */
     /* 如果不可计费，给bill_proc指向的进程的系统时间充电 */
     if(proc != bill_proc && proc != proc_addr(IDLE_TASK)) bill_proc->sys_time += ticks;
     pending_ticks += ticks;    /* 对中断挂起滴答时间充电 */
     now = realticks + pending_ticks;    /* 当前实际时间 = 开机运行时间 + 中断挂起滴答时间 */

     /* 闹钟时间到了？ 或者 到了应该进行时钟任务调度的时候
      * 产生一个时钟中断，激活时钟任务
      * @TODO 闹钟机制还未实现
      */
//     if(next_alarm <= now ||
//             (schedule_ticks == 1 && bill_proc == prev_proc && ready_head[USER_QUEUE] != NIL_PROC) ){
//         interrupt(CLOCK_TASK);
//         return 1;  /* 使其再能发生时钟中断 */
//     }

     /* 调度时间到了？ */
     if(--schedule_ticks == 0){
         /* 如果消费进程等于最后使用时钟任务的进程，重新调度 */
         if(bill_proc == prev_proc) lock_schedule();
         schedule_ticks = SCHEDULE_TICKS;   /* 调度时间计数重置 */
         prev_proc = bill_proc;             /* 设置最后使用时钟任务的用户进程为消费进程 */
     }

    return 1;       /* 返回1，使其再能发生时钟中断 */
}

#if (CHIP == INTEL)

/*===========================================================================*
 *				init_clock				     *
 *				时钟初始化
 *===========================================================================*/
PRIVATE void init_clock()
{
    /* 设置定时器芯片的模式和时间延迟以产生每秒100次的时钟滴答中断
     * 即10ms产生一次中断
     */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8_t)(TIMER_COUNT / HZ));
    out_byte(TIMER0, (u8_t )(TIMER_COUNT >> 8));
    /* 设定时钟中断处理例程，并开启时钟中断 */
    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);
}

#endif  /* CHIP == INTEL */



