/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此函数用于将字符串加载到M3类型的消息中。如果字符串适合消息，
 * 则将其复制到该消息中。 如果不是，则传递指向它的指针。
 */

#include <lib.h>
#include <string.h>

PUBLIC void load_name(_CONST char *name, Message *msg_ptr){
    register size_t len;

    len = strlen(name) + 1;
    msg_ptr->m3_i1 = len;
    msg_ptr->m3_p1 = (char *)name;
    /* 可以放到M3消息类型的字符串中吗？ */
    if(len <= sizeof(msg_ptr->m3_ca1)){
        strcpy(msg_ptr->m3_ca1, name);
    }
}

