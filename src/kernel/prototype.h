/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核所需的函数原型
 * 所有那些必须在其定义所在文件外被感知的函数的原型都放在proto.h中。
 *
 * 它使用了_PROTOTYPE技术，这样，Flyanx核心便既可以使用传统的C编译器(由Kernighan和Richie定义)，
 * 例如Minix初始提供的编译器；又可以使用一个现代的ANSI标准C编译器。
 *
 * 这其中的许多函数原型是与系统相关的,包括中断和异常处理例程以及用汇编语言写的一些函数。
 */

#ifndef FLYANX_PROTOTYPE_H
#define FLYANX_PROTOTYPE_H

/* 结构体声明 */
struct process_s;
struct message_s;
struct tty_s;
struct console_s;

/*================================================================================================*/
/* kernel.asm */
/*================================================================================================*/
_PROTOTYPE( void restart, (void) );
_PROTOTYPE( void idle_task, (void) );

/*================================================================================================*/
/* main.c */
/*================================================================================================*/
_PROTOTYPE(int main, (void)						);
_PROTOTYPE( void panic, (const char *msg, int errno)				);
_PROTOTYPE(void raw_clear_screen, (void) );
_PROTOTYPE( void idle_test_task, (void) );
_PROTOTYPE( void ok_print, (char* msg, char* ok) );

/*================================================================================================*/
/* protect.c */
/*================================================================================================*/
_PROTOTYPE( void protect_init, (void) );
_PROTOTYPE( phys_bytes seg2phys, (U16_t seg) );
_PROTOTYPE( void phys2seg, (u16_t *seg, vir_bytes *off, phys_bytes phys) );

/*================================================================================================*/
/* message.c */
/*================================================================================================*/
_PROTOTYPE( int flyanx_send, (struct process_s *caller_ptr, int dest, struct message_s *message_ptr) );
_PROTOTYPE( int flyanx_receive, (struct process_s *caller_ptr, int src, struct message_s *message_ptr) );
_PROTOTYPE( int sys_call, (int function, int src_dest, struct message_s *message_ptr) );


/*================================================================================================*/
/* clock.c */
/*================================================================================================*/
_PROTOTYPE( void clock_task, (void)					);
_PROTOTYPE( void milli_delay, (time_t millisec) );

/*================================================================================================*/
/* tty.c */
/*================================================================================================*/
_PROTOTYPE( void tty_task, (void) );
_PROTOTYPE( void tty_wakeup, (clock_t now) );
_PROTOTYPE( int input_handler, (struct tty_s *tty, char *buffer, int count) );

/*================================================================================================*/

/*================================================================================================*/
/* process.c */
/*================================================================================================*/
_PROTOTYPE( void interrupt, (int task) );
_PROTOTYPE( int lock_flyan_send, (struct process_s *call_ptr, int dest, struct message_s *message_ptr) );
_PROTOTYPE( void lock_hunter, (void) );
_PROTOTYPE( void lock_ready, (struct process_s *proc) );
_PROTOTYPE( void lock_unready, (struct process_s *proc) );
_PROTOTYPE( void lock_schedule, (void) );
_PROTOTYPE( void unhold, (void) );

/*================================================================================================*/
/* exception.c */
/*================================================================================================*/
_PROTOTYPE( void exception_handler, (int vec_nr, int errno) );

/*================================================================================================*/
/* i8259.c */
/*================================================================================================*/
_PROTOTYPE( void interrupt_init, (int mine)						);
_PROTOTYPE( void put_irq_handler, (int irq, irq_handler_t handler) );
_PROTOTYPE( int spurious_irq, (int ) );

/*================================================================================================*/
/* keyboard.c */
/*================================================================================================*/
_PROTOTYPE( void keyboard_init, (struct tty_s *tty) );
_PROTOTYPE( int keyboard_loadmap, (phys_bytes user_phys) );
_PROTOTYPE( void wreboot, (int how) );

/*================================================================================================*/
/* kernel_386_lib.asm  */
/*================================================================================================*/
_PROTOTYPE( void copy_msg, (int src,phys_clicks src_clicks,vir_bytes src_offset,
        phys_clicks dst_clicks, vir_bytes dst_offset) );
_PROTOTYPE( void phys_copy, (phys_bytes source, phys_bytes dest, phys_bytes count) );
_PROTOTYPE( void out_byte, (port_t port, U8_t value) );
_PROTOTYPE( U8_t in_byte, (port_t port) );
_PROTOTYPE( void out_word, (port_t port, U16_t value) );
_PROTOTYPE(U16_t in_word, (port_t port) );
_PROTOTYPE( void disable_irq, (u32_t intRequest) );
_PROTOTYPE( void enable_irq, (u32_t intRequest) );
_PROTOTYPE( void interrupt_lock, (void) );
_PROTOTYPE( void interrupt_unlock, (void) );
_PROTOTYPE( void level0, (void (*func)(void)) );

/*================================================================================================*/
/* system.c  */
/*================================================================================================*/


/*================================================================================================*/
/* table.c */
/*================================================================================================*/
_PROTOTYPE( void map_drivers, (void)					);

/*================================================================================================*/
/* console.c */
/*================================================================================================*/
_PROTOTYPE( void putk, (int ch) );
_PROTOTYPE( void toggle_scroll, (void) );
_PROTOTYPE( void screen_init, (struct tty_s *tty) );
_PROTOTYPE( void switch_console, (u16_t console_line) );
_PROTOTYPE( void clear_srceen, (int line) );

/*================================================================================================*/
/* kernel_debug.c  */
/*================================================================================================*/
_PROTOTYPE( void delay_by_loop, (int ltime) );
_PROTOTYPE( void simple_brk_point, (int code) );

/*================================================================================================*/
/* dmp.c */
/*================================================================================================*/
_PROTOTYPE( void proc_dmp, (void) );
_PROTOTYPE( void map_dmp, (void) );

/*================================================================================================*/
/* misc.c */
/*================================================================================================*/
_PROTOTYPE( void memory_init, (void) );

/*================================================================================================*/
/*  硬件中断处理程序。 */
/*================================================================================================*/
_PROTOTYPE( void	hwint00, (void) );
_PROTOTYPE( void	hwint01, (void) );
_PROTOTYPE( void	hwint02, (void) );
_PROTOTYPE( void	hwint03, (void) );
_PROTOTYPE( void	hwint04, (void) );
_PROTOTYPE( void	hwint05, (void) );
_PROTOTYPE( void	hwint06, (void) );
_PROTOTYPE( void	hwint07, (void) );
_PROTOTYPE( void	hwint08, (void) );
_PROTOTYPE( void	hwint09, (void) );
_PROTOTYPE( void	hwint10, (void) );
_PROTOTYPE( void	hwint11, (void) );
_PROTOTYPE( void	hwint12, (void) );
_PROTOTYPE( void	hwint13, (void) );
_PROTOTYPE( void	hwint14, (void) );
_PROTOTYPE( void	hwint15, (void) );

/*================================================================================================*/
/* 软件中断处理程序，按数字顺序。 */
/*================================================================================================*/
_PROTOTYPE( void level0_call, (void) );         /* 中断向量：37 */
_PROTOTYPE( void flyanx_386_syscall, (void) );  /* 中断向量：121 */

/*================================================================================================*/
/* 异常中断处理函数 */
/*================================================================================================*/
_PROTOTYPE( void	divide_error, (void) );
_PROTOTYPE( void	single_step_exception, (void) );
_PROTOTYPE( void	nmi, (void) );
_PROTOTYPE( void	breakpoint_exception, (void) );
_PROTOTYPE( void	overflow, (void) );
_PROTOTYPE( void	bounds_check, (void) );
_PROTOTYPE( void	inval_opcode, (void) );
_PROTOTYPE( void	copr_not_available, (void) );
_PROTOTYPE( void	double_fault, (void) );
_PROTOTYPE( void	copr_seg_overrun, (void) );
_PROTOTYPE( void	inval_tss, (void) );
_PROTOTYPE( void	segment_not_present, (void) );
_PROTOTYPE( void	stack_exception, (void) );
_PROTOTYPE( void	general_protection, (void) );
_PROTOTYPE( void	page_fault, (void) );
_PROTOTYPE( void	copr_error, (void) );

#endif //FLYANX_PROTOTYPE_H
