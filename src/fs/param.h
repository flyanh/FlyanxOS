/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/5.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 * 
 * 本文件为包含参数的消息域定义了简短有意义的名称
 */

#ifndef _FS_PARAM_H
#define _FS_PARAM_H

/* 以下名称是输入消息中变量的同义词。 */
#define addr                fs_inbox.m2_l1
#define in_buffer           fs_inbox.m1_p1
#define in_fd               fs_inbox.m1_i1
#define in_child	        fs_inbox.m1_i1
#define in_fd2              fs_inbox.m1_i2
#define in_ls_fd            fs_inbox.m2_i1
#define fmode	            fs_inbox.m3_i2
#define f_cmode1            fs_inbox.m1_i3
#define f_cpathname         fs_inbox.m1_p1
#define pathname	        fs_inbox.m3_p1
#define pathname1	        fs_inbox.m1_p1
#define pathname2	        fs_inbox.m1_p2
#define	pathname_length     fs_inbox.m3_i1
#define pathname1_length    fs_inbox.m1_i1
#define pathname2_length    fs_inbox.m1_i2
#define in_parent	        fs_inbox.m1_i2
#define pathname_in_msg     fs_inbox.m3_ca1     /* 如果路径名较短，将被存放在消息中 */
#define in_pid	            fs_inbox.m1_i3
#define in_bytes            fs_inbox.m1_i2
#define in_offset           fs_inbox.m2_l1
#define in_whence           fs_inbox.m2_i2
#define in_request          fs_inbox.m1_i2


/* 以下名称是输出消息中变量的同义词。 */
#define reply_type      fs_outbox.type
#define reply_l1        fs_outbox.m2_l1
#define reply_i1        fs_outbox.m1_i1
#define reply_i2        fs_outbox.m1_i2
#define reply_t1        fs_outbox.m4_l1
#define reply_t2        fs_outbox.m4_l2
#define reply_t3        fs_outbox.m4_l3
#define reply_t4        fs_outbox.m4_l4
#define reply_t5        fs_outbox.m4_l5

#endif //_FS_PARAM_H
