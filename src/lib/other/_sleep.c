/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 给用户的系统调用：休眠自己，可以设定什么时候唤醒。
 */

#include <lib.h>
#define sleep   _sleep
#include <unistd.h>

PUBLIC int sleep(time_t mills){
    Message out;

    out.m2_l1 = mills;
    return _syscall(FLY, SLEEP, &out);
}


