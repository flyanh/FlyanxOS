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

/*================================================================================================*/
/* main.c */
/*================================================================================================*/
_PROTOTYPE( int main, (void)						);
_PROTOTYPE( void panic, (const char *msg, int errno)				);
_PROTOTYPE( void clear_screen, (void) );

/*================================================================================================*/
/* protect.c */
/*================================================================================================*/
_PROTOTYPE( void protect_init, (void) );
_PROTOTYPE( vir_bytes seg2phys, (u8_t seg) );

/*================================================================================================*/
/* exception.c */
/*================================================================================================*/
_PROTOTYPE( void exception_handler, (unsigned vec_nr, int errno, int eip, int cs, int eflags) );

/*================================================================================================*/
/* i8259.c */
/*================================================================================================*/
_PROTOTYPE( void interrupt_init, (int mine)						);
_PROTOTYPE( void put_irq_handler, (int irq, irq_handler_t handler) );
_PROTOTYPE( int spurious_irq, (int ) );

/*================================================================================================*/
/* kernel_lib.asm  */
/*================================================================================================*/
_PROTOTYPE( void out_byte, (u16_t port, u8_t value) );
_PROTOTYPE( u8_t in_byte, (u16_t port) );
_PROTOTYPE( void disable_irq, (int intRequest) );
_PROTOTYPE( void enable_irq, (int intRequest) );
_PROTOTYPE( void interrupt_lock, (void) );
_PROTOTYPE( void interrupt_unlock, (void) );

/*================================================================================================*/
/* system.c  */
/*================================================================================================*/
_PROTOTYPE( void bad_syscall, (void) );

/*================================================================================================*/
/* kernel_debug.c  */
/*================================================================================================*/
_PROTOTYPE( void clear_screen, (void) );
_PROTOTYPE( char* itoa, (char *str, int num,u8_t radix) );
_PROTOTYPE( void disp_inum, (int num, u8_t radix) );
_PROTOTYPE(void delay_by_loop, (int looptime) );

#endif //FLYANX_PROTOTYPE_H
