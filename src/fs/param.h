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
#define addr            fs_inbox.m2_l1
#define buffer          fs_inbox.m1_p1

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
