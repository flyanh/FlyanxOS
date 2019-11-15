/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含flyanx的消息通信机制
 * 这是实现微内核的核心，应当仔细研究。同时，微内核一直以来被诟病的性能问题。
 * 大部分性能消耗都来自于消息通信，因为发送消息需要在内存中复制消息，而这项
 * 工作需要消耗很多的系统资源，所以如果想优化flyanx内核，第一步就是解决消息
 * 的资源消耗问题。
 *
 * 该文件的入口点：
 *  - flyanx_send              向某个进程发送一条消息
 *  - flyanx_receive           接收来自某个进程的消息
 *  - flyanx_send_receive      向某个进程发送一条消息并等待该进程的回应
 *
 * 注意：这两个调用都会导致进程进入堵塞状态。
 */

#include "kernel.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"

/*===========================================================================*
 *				flyanx_send	    		     *
 *				向一个进程发送一条消息             *
 *===========================================================================*/
PUBLIC int flyanx_send(caller_ptr, dest, message_ptr)
register struct process_s *caller_ptr;	/* 调用进程，即谁想发消息？ */
int dest;			                /* 目标进程号，即谁将接收这条消息？或说这条消息将发给谁？ */
struct message_s *message_ptr;			    /* 消息 */
{
    /*
     *
     */

    return 0;
}

/*===========================================================================*
 *				flyanx_receive	    		     *
 *				接收来自某个进程的消息             *
 *===========================================================================*/
PUBLIC int flyanx_receive(caller_ptr, src, message_ptr)
register Process *caller_ptr;	/* 准备获取(接收)消息的进程 */
int src;			                /* 准备从哪个源进程接收消息（可以是任何），也就是发送消息的进程 */
Message *message_ptr;			    /* 消息 */
{
    /*
     *
     */

    return 0;
}

/*===========================================================================*
 *				flyanx_send_receive	    		     *
 *				向某个进程发送一条消息并等待该进程的回应             *
 *===========================================================================*/
PUBLIC int flyanx_send_receive(caller_ptr, sdest, message_ptr)
register Process *caller_ptr;	/* 调用进程，即谁想发消息？ */
int sdest;			                /* 既是目标进程，也是源进程 */
Message *message_ptr;			    /* 消息地址 */
{
    /*
     *
     */

    return 0;
}



