/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这里采用符号方式定义了文件控制操作使用的许多参数。
 * 例如,它允许在 open 调用中使用宏O_RDONLY 来代替数值0作为参数。
 * 尽管该文件主要由文件系统引用,但它的定义在核心和内存管理器中也多次用到。
 */

#ifndef _FCNTL_H
#define _FCNTL_H


#ifndef _TYPES_H
#include <sys/types.h>
#endif




#endif //_FCNTL_H
