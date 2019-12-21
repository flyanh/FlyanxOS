/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含了用于请求信息中的系统调用参数的宏和四个用于应答消息中域的宏。
 */

#define m_exec_name     mmsg_in.m1_p1           /* 要执行的文件名 */
#define m_exec_nlen     mmsg_in.m1_i1           /* 要执行的文件名长度 */
#define m_pid		    mmsg_in.m1_i1           /* 进程号 */
#define m_status		mmsg_in.m1_i1           /* 状态码 */
#define m_sig_nr		mmsg_in.m1_i2           /* 信号 */
#define m_stack_ptr     mmsg_in.m1_p2           /* 栈指针 */
#define m_stack_bytes   mmsg_in.m1_i2           /* 栈大小 */


/* 以下名称是应答消息中变量的同义词。 */
#define reply_rs1       reply.type      /* 回复结果1 */
#define reply_rs2	    reply.m2_i1     /* 回复结果2 */
#define reply_ptr	    reply.m2_p1     /* 回复指针 */
#define reply_mask	    reply.m2_l1     /* 回复掩码 */

