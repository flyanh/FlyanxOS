/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/5.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统服务遇到一些特殊情况，需要停止内核的工作前，调用本例程可以使
 * 终端属性变为：蓝底白字（高亮）。这样错误信息将更加显眼，适合用户
 * 和开发人员进行诊断。
 */

#include "syslib.h"

PUBLIC void sys_blues(void){
    Message out;
    (void)task_call(SYS_TASK, SYS_BLUES, &out);
}

