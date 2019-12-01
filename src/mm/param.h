/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含了用于请求信息中的系统调用参数的宏和四个用于应答消息中域的宏。
 */

#define m_addr		mmsg_in.m1_p1             /* D空间的新虚拟地址 */
#define m_exec_name	mmsg_in.m1_p1             /* 要执行的文件名 */
#define m_exec_len	mmsg_in.m1_i1             /* 执行文件的长度 */
#define m_func		mmsg_in.m6_f1             /* 例程指针 */
#define m_grpid		(gid_t) mmsg_in.m1_i1     /* 组号 */
#define m_namelen		mmsg_in.m1_i1             /* 名称长度 */
#define m_pid		    mmsg_in.m1_i1             /* 进程号 */
#define m_seconds		mmsg_in.m1_i1
#define m_sig		    mmsg_in.m6_i1             /* 信号 */
#define m_stack_bytes	mmsg_in.m1_i2             /* 栈大小 */
#define m_stack_ptr	mmsg_in.m1_p2             /* 栈指针 */
#define m_status		mmsg_in.m1_i1             /* 状态码 */
#define m_usr_id		(uid_t) mmsg_in.m1_i1     /* 用户号 */
#define m_request		mmsg_in.m2_i2
#define m_taddr		mmsg_in.m2_l1
#define m_data		mmsg_in.m2_l2             /* 数据 */
#define m_sig_nr		mmsg_in.m1_i2             /* 信号 */
#define m_sig_nsa		mmsg_in.m1_p1             /* 新的信号处理动作(sigaction结构) */
#define m_sig_osa		mmsg_in.m1_p2             /* 旧的有效的信号处理动作(sigaction结构) */
#define m_sig_ret		mmsg_in.m1_p3
#define m_sig_set		mmsg_in.m2_l1             /*  */
#define m_sig_how		mmsg_in.m2_i1
#define m_sig_flags	mmsg_in.m2_i2
#define m_sig_context	mmsg_in.m2_p1
#ifdef _SIGMESSAGE
#define m_sig_msg		mmsg_in.m1_i1
#endif
#define m_reboot_flag	mmsg_in.m1_i1             /* 重启标志 */
#define m_reboot_code	mmsg_in.m1_p1             /* 重启代码 */
#define m_reboot_size	mmsg_in.m1_i2
#define m_svrctl_req	mmsg_in.m2_i1
#define m_svrctl_argp	mmsg_in.m2_p1


/* 以下名称是应答消息中变量的同义词。 */
#define reply_type      reply.type     /* 回复类型 */
#define reply_type2	    reply.m2_i1      /* 回复类型2 */
#define reply_ptr	    reply.m2_p1      /* 回复指针 */
#define reply_mask	    reply.m2_l1 	    /* 回复掩码 */

