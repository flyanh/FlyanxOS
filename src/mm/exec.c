/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件处理EXEC系统调用，它执行的工作如下：
 *  - 检查权限-文件是否可执行？
 *  - 读取文件头得到各段长度和总长度
 *  - 从调用者处取得参数和环境
 *  - 分配新内存和释放旧内存
 *  - 把堆栈复制到新的内存映像中
 *  - 把数据（可能还有正文）段复制到新的内存映像中
 *  - 检查处理setuid、setgid位
 *  - 设置进程表项
 *  - 告诉内核进程现在是可运行的了
 *  - 将偏移量保存到初始argc（用于ps）
 */

#include "mm.h"
#include <sys/stat.h>
#include <flyanx/callnr.h>
#include <elf.h>
#include <signal.h>
#include <string.h>
#include "mmproc.h"
#include "param.h"

/*===========================================================================*
 *				do_exec					     *
 *===========================================================================*/
PUBLIC int do_exec(void){

}

