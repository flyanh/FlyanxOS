#
# Makefile
# Created by flyan on 2019/11/9.
# QQ: 1341662010
# QQ-Group:909830414
# gitee: https://gitee.com/flyanh/
#
# 这个Makefile负责编译所有实用命令程序，它们将会被移动到硬盘上，
# flyanx可以运行它们。
#

# flyanx的用户程序编译之后的挂载点：0x1000
ENTRYPOINT 		= 0x1000
# ===============================================
# 所有的变量

# 头文件目录
u = /usr
i = ../../include
h = $i/flyanx
s = $i/sys
b = $i/ibm
l = $i/lib

# 内核的编译链接中间目录
t = ../../target
tb = $t/boot
tk = $t/kernel
tl = $t/lib

# 所有实用程序目录
alldir:         = simple

# 硬盘镜像
HD              = ../../500MHD.img

# 所使用的编译程序，参数
ASM 			= nasm
DASM 			= objdump
CC 				= gcc
LD				= ld
ASMFLAGS	    = -f elf
CFLAGS			= -I$i -O2 -c -fno-builtin -fno-stack-protector -D_FLYANX -D_POSIX_SOURCE -Wall
LDFLAGS			= -Ttext $(ENTRYPOINT)
DASMFLAGS		= -D

# flyanx标准库
LIB             = $(tl)/flyanxlib.a

# 依赖关系

# ===============================================
# 生成的所有命令行程序
SIMPLE          = pwd


COMMANDS        = $(SIMPLE)
# ===============================================
# 默认选项，提示如何编译
nop:
	@echo "请先make all，然后make install..."
# 编译全部
all: clean everything
	@echo "已经编译所有的命令行实用程序..."

# 声明所有的伪命令，将不会被识别为文件
.PHONY : everything final clean realclean disasm all install

# ===============================================
# 所有的伪命令
everything: $(COMMANDS)

install: all
	cp $(tb)/hd_loader.bin ./ -v
	mv hd_loader.bin hdldr.bin
	cp $(tk)/kernel.bin ./ -v
	tar vcf install.tar kernel.bin $(COMMANDS) hdldr.bin
	dd if=install.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' $h/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l install.tar | awk -F " " '{print $$5}'` conv=notrunc

final: all clean

clean:
	-rm -rf simple/*.o

realclean: clean
	-rm -rf $(COMMANDS)

# ===============================================
# 所有的文件生成规则
# ============ flyanx程序启动器 ============
start.o: flyanx_prog_start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

# ============ 简单的命令行程序 ============
simple/pwd.o: $i/stdio.h
simple/pwd.o: simple/pwd.c
	$(CC) $(CFLAGS) -o $@ $<

pwd: simple/pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?









# ===============================================