/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内存管理器内的函数原型
 */

/* 结构体声明 */
struct mm_process_s;

/*================================================================================================*/
/* main.c */
/*================================================================================================*/
_PROTOTYPE( void set_reply, (int proc_nr, int rs) );
_PROTOTYPE( void mm_print_info, (char *info) );

/*================================================================================================*/
/* alloc.c */
/*================================================================================================*/
_PROTOTYPE( void mem_init, (phys_clicks total, phys_clicks free) );
_PROTOTYPE( phys_clicks alloc_mem, (phys_clicks clicks) );
_PROTOTYPE( void free_mem, (phys_clicks base, phys_clicks clicks) );

/*================================================================================================*/
/* forkexit.c */
/*================================================================================================*/
_PROTOTYPE( int do_fork, (void) );
_PROTOTYPE( int do_mm_exit, (void) );
_PROTOTYPE( void mm_exit, (struct mm_process_s *rmp, int exit_status) );
_PROTOTYPE( int do_wait, (void) );

/*================================================================================================*/
/* exec.c */
/*================================================================================================*/
_PROTOTYPE( int do_exec, (void) );

/*================================================================================================*/
/* misc.c */
/*================================================================================================*/
_PROTOTYPE( int do_block, (void) );

/*================================================================================================*/
/* utils.c */
/*================================================================================================*/
//_PROTOTYPE( int allowed, (char *name_buf, struct stat *s_buf, int mask)	);
_PROTOTYPE( int no_sys, (void)						);
_PROTOTYPE( void mm_panic, (const char *msg, int errno)				);
//_PROTOTYPE( void tell_fs, (int what, int p1, int p2, int p3)		);
