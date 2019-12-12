/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这个系统调用跟message.c里面的不一样，这个系统调用是给
 * 用户开放的，而message.c里的是给所有人开放，本文件中的
 * 对其进行的封装和限制，使用户只能使用send_receive()，
 * 且参数也限制了药包过户只能使用开放的系统调用号。
 */

#include <lib.h>

PUBLIC int _syscall(
        int who,        /* 想发给谁？ */
        int syscallnr,  /* 想请求什么调用？ */
        register Message *msg_ptr   /* 发送的消息 */
){
    int status;

    msg_ptr->type = syscallnr;
    status = send_receive(who, msg_ptr);
    if(status != 0){
        /* 失败了？ */
        /* 发送的消息type存放这个错误状态 */
        msg_ptr->type = status;
    }
    if(msg_ptr->type < 0){
        /* 如果错误状态 小于零，那么我们将其置为正数放到errno中。 */
        errno = -msg_ptr->type;
        return -1;
    }
    /* OK（0）或者大于0？直接返回即可 */
    return msg_ptr->type;
}

