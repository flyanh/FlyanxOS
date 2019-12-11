/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：创建一个新文件。
 */

#include <lib.h>
#define creat   _creat
#include <fcntl.h>

PUBLIC int _creat(_CONST char *name, Mode_t mode){
    Message out;

    out.m3_i2 = mode;
    /* 如果可能的话，将用户的路径字符串加载到消息中 */
    load_name(name, &out);
    return _syscall(FS, CREAT, &out);
}
