/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含时钟相关，最主要的是时钟系统任务（同时也是时钟驱动程序）
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
PRIVATE clock_t ticks;          /* 时钟运行的时间(滴答数)，也是开机后时钟运行的时间 */
PRIVATE time_t realtime;        /* 时钟运行的时间(s)，也是开机后时钟运行的时间 */
PRIVATE time_t boot_time;       /* 系统启动时间(s) */
PRIVATE clock_t next_alarm;     /* 下一个信号或闹钟发生的时刻，称之为任务闹钟 */
PRIVATE Message msg;            /* 发送和接收的消息缓冲区 */
PRIVATE int watchdog_proc;      /* 存放触发了喂狗动作的进程 */
/* 看门狗，存放闹钟需要调用的函数数组 */
PRIVATE WatchDog watch_dog[NR_TASKS + NR_PROCS];

/* 由中断处理程序更改的变量 */
PRIVATE clock_t  pending_ticks; /* 中断挂起的时间 */
PRIVATE int schedule_ticks = SCHEDULE_TICKS;     /* 用户进程调度时间，当为0时候，进行程序调度 */
PRIVATE Process *prev_proc;     /* 最后使用时钟任务的用户进程 */

/* 本地函数原型 */
FORWARD _PROTOTYPE( void init_clock, (void) );
FORWARD _PROTOTYPE( void do_set_sync_alarm, (Message *m_ptr) );
FORWARD _PROTOTYPE( void do_set_alarm, (Message *m_ptr) );
FORWARD _PROTOTYPE( void do_set_time, (Message *m_ptr) );
FORWARD _PROTOTYPE( void do_get_time, (void) );
FORWARD _PROTOTYPE( void do_get_uptime, (void) );
FORWARD _PROTOTYPE( void do_clock_int, (void) );
FORWARD _PROTOTYPE( void common_set_alarm, (int proc_nr, clock_t delta_ticks, WatchDog func) );

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
    int mess_type;      /* 消息类型，通过其判断用户需要什么服务 */

    /* 初始化时钟 */
    init_clock();

    /* 时钟任务主循环，一直得到工作，处理工作，回复处理结果 */
    while (TRUE){

        /* 从外界得到一条消息 */
        receive(ANY, &msg);

        /* 提取消息类型 */
        mess_type = msg.type;

        /* 已经得到用户发来的消息请求，现在开始校准时间，记得先锁住中断 */
        interrupt_lock();
        ticks += pending_ticks;             /* 加上从上次计时后过去的滴答数到时钟真实时间ticks上 */
        realtime = ticks / HZ;              /* 计算按秒数计算的真实时钟时间 */
        pending_ticks = 0;                  /* 好了，上次计时后过去的滴答数可以清零了 */
        interrupt_unlock();

        /* 根据消息类型提供不同的功能服务 */
        switch (mess_type){
            case HARD_INT:      do_clock_int();             break;      /* 时钟产生的硬件中断，强制，不能忽略  */
            case GET_UPTIME:    do_get_uptime();	        break;      /* 获取从启动开始后的时钟滴答数时间 */
            case GET_TIME:      do_get_time();              break;      /* 获取系统时间（时间戳） */
            case SET_TIME:	    do_set_time(&msg);	        break;      /* 设置系统时间 */
            case SET_ALARM:	    do_set_alarm(&msg);	        break;      /* 设置定时器 */
            case SET_SYNC_ALARM:do_set_sync_alarm(&msg);    break;      /* 设置同步闹钟 */
            default: panic("Clock task got bad message", msg.type);  /* 当然了，获取到不识别的操作就宕机 */
        }

        /* 完成工作，现在给出回复，硬件中断除外 */
        msg.type = OK;
        if(mess_type != HARD_INT){
            send(msg.source, &msg);
        }
    }

}

/*===========================================================================*
 *				do_clock_int				     *
 *				  处理时钟中断
 *===========================================================================*/
PRIVATE void do_clock_int() {
    /* 尽管这个例程叫做这个名字，但要注意的一点是，并不是在每次时钟中断都会调用
     * 这个例程的。当中断处理程序确定有一些重要的工作不得不做时才会通知时钟任务
     * 让时钟任务来调用本例程。
     */
    register Process *proc;
    register int proc_nr;

    if(next_alarm <= ticks){    /* 任务闹钟是否已经响起？ */
        next_alarm = LONG_MAX;  /* 开始计算下次的任务闹钟时间，这期间先设置一个很大的值 */
        /* 虽然闹钟已经响起，但是有的任务喂狗函数可能已经不存在，所以要检查一下 */
        for(proc = BEG_PROC_ADDR; proc < END_PROC_ADDR; proc++){
            if(proc->alarm != 0){
                /* 进程的闹钟已经响起
                 *
                 * 如果是用户进程，给它发个信号；如果是个系统任务，则调用看门狗数组
                 * 中保存的以前的由任务指定的函数。也就是喂骨头时间到了，该喂骨头了。
                 */
                if(proc->alarm <= ticks){
                    proc_nr = proc->nr;
                    if(watch_dog[proc_nr + NR_TASKS]){      /* 去看门狗数组中看看被闹钟吵醒的任务还存不存在看门狗函数 */
                        watchdog_proc = proc_nr;            /* 存放触发喂狗函数的进程索引号 */
                        (*watch_dog[proc_nr + NR_TASKS])(); /* 执行喂狗函数（任务指定的函数） */
                    } else {
                        /* 如果看门狗数组中不存在该进程的喂狗函数，说明该进程是个用户进程，发送信号通知它 */
//                        cause_signal(proc_nr, SIGALRM);
                    }
                    proc->alarm = 0;
                }

                /* 确定下一个即将发生的闹钟 */
                if(proc->alarm != 0 && proc->alarm < next_alarm){
                    next_alarm = proc->alarm;
                }
            }
        }
    }
}

/*===========================================================================*
 *				do_get_uptime				     *
 *			获取从启动开始后的时钟滴答数时间
 *===========================================================================*/
PRIVATE void do_get_uptime() {
    msg.CLOCK_TIME = ticks;       /* 将时间放到消息中 */
}

/*===========================================================================*
 *				do_get_time				     *
 *			获取并返回当前时钟时间(以秒为单位)。
 *===========================================================================*/
PRIVATE void do_get_time() {
    msg.CLOCK_TIME = boot_time + realtime;  /* 系统时间 + 时钟真实时间 */
}

/*===========================================================================*
 *				do_set_time				     *
 *		设置实时时钟。只有超级用户才能使用这个调用。
 *===========================================================================*/
PRIVATE void do_set_time(Message *m_ptr) {
    boot_time = m_ptr->CLOCK_TIME - realtime; /* 计算系统启动时间：用户设置的时间 - 当前时钟时间 */
}

/*===========================================================================*
 *				do_setalarm				     *
 *				设置一个闹钟
 *===========================================================================*/
PRIVATE void do_set_alarm(Message *m_ptr) {
    register Process *proc;
    int proc_nr;            /* 哪个进程想要闹钟 */
    long delta_ticks;       /* 他想要多少个时钟滴答声后响铃？ */
    WatchDog func;          /* 闹钟响起执行的动作（只有系统任务才能设置） */

    /* 从消息中提取参数 */
    proc_nr = m_ptr->CLOCK_PROC_NR;     /* 稍后这个进程将被闹钟中断 */
    delta_ticks = m_ptr->DELTA_TICKS;   /* 等多久呢？ */
    func = (WatchDog) m_ptr->FUNC_TO_CALL;  /* 要调用的函数(仅限于系统任务) ，
                                             * 如果是用户进程，调用函数可以取空指针，就算给了也会被无视 */
    proc = proc_addr(proc_nr);          /* 得到进程实例 */
    /* 计算闹钟时间，并将其放入到即将回复的消息中| */
    msg.SECONDS_LEFT = (proc->alarm == 0 ? 0 : (proc->alarm - ticks + (HZ - 1)) / HZ);
    /* 给系统任务的要调用的函数设置为0，代表以后用信号通知它 */
    if(!is_task_proc(proc)) func = 0;
    /* 公共的设置并启动闹钟例程：真正要去实现设置并启动闹钟的事务 */
    common_set_alarm(proc_nr, delta_ticks, func);
}

/*===========================================================================*
 *				do_set_sync_alarm				     *
 *				设置一个同步闹钟
 *===========================================================================*/
PRIVATE void do_set_sync_alarm(Message *m_ptr) {
    /* 暂时不打算实现同步闹钟，
     * 同步闹钟主要是解决到有了网络后各种应用间的同步问题，
     * 但是网络我们暂时也没有支持，所以现在这个功能暂时搁
     * 置。
     */
    panic("Clock task sync alarm not yet supported.", msg.type);  /* 当然了，获取到不识别的操作就宕机 */
}

/*===========================================================================*
 *				common_set_alarm				     *
 *				公共的设置闹钟例程
 *===========================================================================*/
PRIVATE void common_set_alarm(
        int proc_nr,            /* 哪个进程想要闹钟 */
        clock_t delta_ticks,    /* 他想要多少个时钟滴答声后响铃？ */
        WatchDog func           /* 闹钟响起执行的动作（只有系统任务才能设置） */
){
    /* 完成do_set_alarm（普通闹钟）和do_set_sync_alarm（同步闹钟）的
     * 共同设置和启动工作
     */
    register Process *proc;

    /* 得到要被叫醒的进程实例 */
    proc = proc_addr(proc_nr);
    /* 计算闹钟时间并放入到该进程中 */
    proc->alarm = (delta_ticks == 0 ? 0 : ticks + delta_ticks);
    /* 将要执行的函数放入到看门狗数组中 */
    watch_dog[proc_nr + NR_TASKS] = func;

    /* 寻找下一个即将发送的闹钟 */
    next_alarm = LONG_MAX;
    for(proc = BEG_PROC_ADDR; proc < END_PROC_ADDR; proc++){
        if(proc->alarm != 0 && proc->alarm < next_alarm) {
            next_alarm = proc->alarm;
        }
    }
}

/*===========================================================================*
 *				get_uptime				     *
 *===========================================================================*/
PUBLIC clock_t get_uptime(void)
{
/* 获取并返回当前时钟正常运行时间(以滴答为单位)。
 *
 * 与do_getuptime的区别是这个函数可以直接返回时间通过函数调用，而无需
 * 通过代价很大的消息传递去获取时间，但这个函数只允许系统任务调用，用户
 * 进程还是只能通过发送消息然后调用do_get_uptime的方式来获取。
 */

    clock_t uptime;

    interrupt_lock();
    uptime = ticks + pending_ticks;
    interrupt_unlock();
    return uptime;
}

/*==========================================================================*
 *				milli_delay				    *
 *				毫秒级别延迟
 *==========================================================================*/
PRIVATE clock_t milli_delay_alarm = FALSE;   /* FALSE：毫秒级别延迟函数退出，否则是一个闹钟值 */
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
     * 本函数也能被时钟任务调用，因为一旦时钟中断打开，本函数就能正常运行了，所以必须
     * 要在中时钟中断初始化后才能被正常调用，这是解决上一次遗留下来的问题。
     */

    /* 得出退出循环的闹钟时间 */
    milli_delay_alarm = ticks + millisec / ONE_TICK_MILLISECOND;
    clock_t enter_millis = 0;           /* 进入延迟函数的总时间 */
    while (milli_delay_alarm != FALSE) {};  /* 只要检测到闹钟还未被关闭，说明时间没到，继续循环 */
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
     register u16_t one_ticks;
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
     } else {
         // 没发生中断冲入，当前进程就是curr_proc指向的进程
         proc = curr_proc;
     }
     one_ticks = lost_ticks + 1;    /* 时钟滴答数，等于时钟任务外的时钟滴答数 + 本次的滴答数(1) */
     lost_ticks = 0;            /* 时钟任务外的时钟滴答数归零 */
     proc->user_time += one_ticks;  /* 充电 */
     /* 如果不可计费，给bill_proc指向的进程的系统时间充电 */
     if(proc != bill_proc && proc != proc_addr(IDLE_TASK)) bill_proc->sys_time += ticks;
     pending_ticks += one_ticks;        /* 对中断挂起滴答时间充电 */
     now = ticks + pending_ticks;   /* 当前实际时间 = 开机运行时间 + 中断挂起滴答时间 */

     /* 好了，如果终端任务的触发时间到了，唤醒其 */
     if(tty_wake_time <= now){
         tty_wakeup(now);
     }

    /* 任务闹钟时间到了？产生一个时钟中断，唤醒时钟任务 */
     if(next_alarm != FALSE && next_alarm <= now){
         interrupt(CLOCK_TASK);
         return ENABLE;  /* 使其再能发生时钟中断 */
     }

     /* 毫秒级休眠函数退出闹钟响了？为FALSE表示毫秒级休眠函数处于退出状态 */
     if(milli_delay_alarm != FALSE && milli_delay_alarm <= now){
         /* ok，现在毫秒级休眠函数需要退出了 */
         milli_delay_alarm = FALSE;
     }

     /* 调度时间到了？ */
     if(--schedule_ticks == 0 && bill_proc == prev_proc && ready_head[USER_QUEUE] != NIL_PROC) {
         /* 如果消费进程等于最后使用时钟任务的进程，重新调度 */
         if(bill_proc == prev_proc) lock_schedule();
         schedule_ticks = SCHEDULE_TICKS;   /* 调度时间计数重置 */
         prev_proc = bill_proc;             /* 设置最后使用时钟任务的用户进程为消费进程 */
     }

    return ENABLE;       /* 返回ENABLE，使其再能发生时钟中断 */
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



