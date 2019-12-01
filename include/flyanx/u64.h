/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此文件提供操纵64位磁盘地址的功能。
 */

#ifndef _FLYANX_U64_H
#define _FLYANX_U64_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

u64_t add64(u64_t i, u64_t j);
u64_t add64u(u64_t i, unsigned j);
u64_t add64ul(u64_t i, unsigned long j);
u64_t sub64(u64_t i, u64_t j);
u64_t sub64u(u64_t i, unsigned j);
u64_t sub64ul(u64_t i, unsigned long j);
unsigned diff64(u64_t i, u64_t j);
u64_t cvu64(unsigned i);
u64_t cvul64(unsigned long i);
unsigned cv64u(u64_t i);
unsigned long cv64ul(u64_t i);
unsigned long div64u(u64_t i, unsigned j);
unsigned rem64u(u64_t i, unsigned j);
u64_t mul64u(unsigned long i, unsigned j);
int cmp64(u64_t i, u64_t j);
int cmp64u(u64_t i, unsigned j);
int cmp64ul(u64_t i, unsigned long j);
unsigned long ex64lo(u64_t i);
unsigned long ex64hi(u64_t i);
u64_t make64(unsigned long lo, unsigned long hi);

#endif //_FLYANX_U64_H
