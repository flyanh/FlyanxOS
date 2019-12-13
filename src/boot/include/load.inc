; Copyright (C) 2007 Free Software Foundation, Inc.
; See the copyright notice in the file /usr/LICENSE.
; Created by flyan on 2019/11/9.
; QQ: 1341662010
; QQ-Group:909830414
; gitee: https://gitee.com/flyanh/
;
; 包含了启动内核的一些信息
; 软盘启动/硬盘启动都需要这个文件

;; 内核挂载点
; 注意：1、必须与 MAKEFILE 中参数 -Ttext 的值相等!!
;      2、这是个物理地址而非仅仅是个偏移，如果 -Ttext 的值为 0x328000，则它的值也应该是 0x328000。
KERNEL_ENTRY_POINT_PHY_ADDR     equ 0x1000

; 页目录开始地址:		1M
PAGE_DIR_BASE		equ	0x100000
; 页表开始地址:		1M + 4K
PAGE_TABLE_BASE		equ	0x101000

; LOADER.BIN 被加载的位置　--- 段地址
LOADER_SEG        equ     0x9000

; LOADER.BIN 被加载的位置　--- 偏移地址  
OffsetOfLoader      equ     0x100
LOADER_OFFSET      equ     0x100
; LOADER.BIN 被加载到的位置　--- 物理地址　(= LOADER_SEG * 10h)
LOADER_PHY_ADDR     equ LOADER_SEG * 10h

; KERNEL.BIN 被加载到的位置 ----  段地址
KERNEL_SEG	        equ	    0x7000
; KERNEL.BIN 被加载到的位置 ---- 偏移地址
KERNEL_OFFSET	equ	    0
KERNEL_PHY_ADDR	equ	KERNEL_SEG * 10h

;; 为kernel.bin装载保留的内存大小(字节)，内核文件大小不应该超过这个大小！
KERNEL_HAVE_SPACE   equ LOADER_PHY_ADDR - KERNEL_PHY_ADDR

;; 超级块将被存储在的位置：[SUPER_BLOCK_SEG:0x900]
SUPER_BLOCK_SEG     equ 0x70

;; 引导（启动）参数
; 注意：
;   以下的宏应该和C代码里的同步，它们在include/flyanx/config.h下
BOOT_PARAM_ADDR     equ 0x700
BOOT_PARAM_MAGIC    equ 0x328

;; 根设备的基地址所处扇区号
; ===== 加载时我们不计算根设备的基本扇区号
; ===== 但将其定义为宏有两个原因：
; ===== 1. 它对于任何特定系统都是一个常数
; ===== 2. 它简化了引导扇区和加载程序
ROOT_BASE		equ	0xF000

;; flyanx文件系统的一些参数，对应于src/fs下很多头文件的参数
SUPER_MAGIC             equ 0x3EA            ; 超级块的魔数
; 超级块数据结构的属性参数所在内存中的相对偏移位置，将被写到硬盘中用于识别flyanx文件系统
SB_MAGIC                equ 4 * 0       ; 魔数，用于识别超级块
SB_NR_INODES            equ 4 * 1       ; 多少个索引节点
SB_NR_SECTS             equ 4 * 2       ; 多少个扇区
SB_NR_IMAP_SECTS        equ 4 * 3       ; 多少个<索引节点位图>扇区
SB_NR_SMAP_SECTS        equ 4 * 4       ; 多少个<扇区位图>扇区
SB_FIRST_SECT           equ 4 * 5       ; 第一个数据扇区的编号
SB_NR_INODE_SECT        equ 4 * 6       ; 多少个<索引节点>扇区
SB_ROOT_INODE           equ 4 * 7       ; 根目录的索引节点
SB_INODE_SIZE           equ 4 * 8       ; 索引节点的长度
SB_INODE_SIZE_OFF       equ 4 * 9       ; 结构'Inode::size'的偏移量
SB_INODE_START_OFF      equ 4 * 10      ; 结构'Inode::start_sect'的偏移量
SB_DIR_ENT_SIZE         equ 4 * 11      ; 结构'DirectoryEntry'的大小
SB_DIR_ENT_INODE_OFF    equ 4 * 12      ; 结构'DirectoryEntry::inode_nr'的偏移
SB_DIR_ENT_NAME_OFF     equ 4 * 13      ; 结构'DirectoryEntry::name'的偏移








