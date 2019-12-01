; Copyright (C) 2007 Free Software Foundation, Inc.
; See the copyright notice in the file /usr/LICENSE.
; Created by flyan on 2019/11/9.
; QQ: 1341662010
; QQ-Group:909830414
; gitee: https://gitee.com/flyanh/
;
;

; LOADER.BIN 被加载的位置　--- 段地址
BaseOfLoader        equ     0x09000
; LOADER.BIN 被加载的位置　--- 偏移地址  
OffsetOfLoader      equ     0x0100
; LOADER.BIN 被加载到的位置　--- 物理地址　(= BaseOfLoader * 10h)
BaseOfLoaderPhyAddr equ BaseOfLoader * 10h

; KERNEL.BIN 被加载到的位置 ----  段地址
BaseOfKernelFile	equ	    0x7000
; KERNEL.BIN 被加载到的位置 ---- 偏移地址
OffsetOfKernelFile	equ	    0
BaseOfKernelFilePhyAddr	equ	BaseOfKernelFile * 10h
; 注意：1、必须与 MAKEFILE 中参数 -Ttext 的值相等!!
;      2、这是个地址而非仅仅是个偏移，如果 -Ttext 的值为 0x400400，则它的值也应该是 0x400400。
KernelEntryPointPhyAddr	equ	0x1000
					
; 页目录开始地址:		1M
PageDirBase		equ	0x100000
; 页表开始地址:		1M + 4K
PageTblBase		equ	0x101000

; 注意：
;   以下的宏应该和C代码里的同步，它们在include/flyanx/config.h下
BOOT_PARAM_ADDR     equ 0x700
BOOT_PARAM_MAGIC    equ 0x3EA




