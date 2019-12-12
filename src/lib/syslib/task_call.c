/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/5.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供一个接口给服务器调用，它与_sys_call()基本相同，唯一不同
 * 是taskcall()返回负错误的直接编码，不需要errno编码。
 */

#include <lib.h>
#include <flyanx/syslib.h>

PUBLIC int task_call(int who, int sys_callnr, Message *out){
    int status;

    out->type = sys_callnr;
    status = send_receive(who, out);
    if(status != 0) return status;
    else return out->type;
}
