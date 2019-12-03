#
# Makefile
# Created by flyan on 2019/11/9.
# QQ: 1341662010
# QQ-Group:909830414
# gitee: https://gitee.com/flyanh/
#

# ===============================================
# FlyanxOS　的内存装载点
# 这个值必须存在且相等在文件"load.inc"中的 'KernelEntryPointPhyAddr'！
ENTRYPOINT 		= 0x1000
# 内核文件的挂载点偏移地址
# 它必须和 ENTRYPOINT 相同
ENRTYOFFSET 	= 0
# ===============================================
# 所有的变量

# 头文件目录
u = /usr
i = include
h = $i/flyanx
s = $i/sys
b = $i/ibm
l = $i/lib

# c文件所在目录
sk = src/kernel
sog = src/origin
smm = src/mm
sfs = src/fs
sfly = src/fly

# 编辑链接中间目录
t = target
tb = $t/boot
tk = $t/kernel
tl = $t/lib
tog = $t/origin
tmm = $t/mm
tfs = $t/fs
tfly = $t/fly

# 所需要的软盘镜像，可以自指定已存在的软盘镜像，系统内核将被写入这里面
FD			= Flyanx.img
# 硬盘镜像
HD          = 100M_HD.img

# 镜像挂载点，自指定，必须存在于自己的计算机上，如果没有请自行创建一下
FloppyMountPoint= /media/floppyDisk

# 所使用的编译程序，参数
ASM 			= nasm
DASM 			= ndisasm
CC 				= gcc
LD				= ld
ASMFlagsOfBoot	= -I src/boot/include/
ASMFlagsOfKernel= -f elf -I $(sk)/
CFlags			= -I$i -c -fno-builtin
LDFlags			= -s -Ttext $(ENTRYPOINT)
DASMFlags		= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# 依赖关系
ka = $(sk)/kernel.h $h/config.h $h/const.h $h/type.h $h/syslib.h \
     $s/types.h $i/string.h $i/limits.h $i/errno.h $(sk)/const.h \
     $(sk)/type.h $(sk)/prototype.h $(sk)/global.h

mma = $(smm)/mm.h $h/config.h $s/types.h $h/const.h $h/type.h \
      $i/fcntl.h $i/unistd.h $i/limits.h $i/errno.h $h/syslib.h \
      $(smm)/const.h $(smm)/type.h $(smm)/prototype.h $(smm)/global.h

fsa = $(sfs)/fs.h $h/config.h $s/types.h $h/const.h $h/type.h \
      $i/limits.h $i/errno.h $h/syslib.h \
      $(sfs)/const.h $(sfs)/type.h $(sfs)/prototype.h $(sfs)/global.h

flya = $(sfly)/fly.h $h/config.h $s/types.h $h/const.h $h/type.h \
       $i/limits.h $i/errno.h $h/syslib.h \
       $(sfly)/const.h $(sfly)/type.h $(sfly)/prototype.h $(sfly)/global.h

lib = $i/lib.h $h/common.h $h/syslib.h


# ===============================================
# 目标程序以及编译中间文件

FlyanxBoot		= $(tb)/boot.bin $(tb)/loader.bin
FlyanxKernel	= $(tk)/kernel.bin
FlyanxKernelHead = $(tk)/kernel.o

# 内核本体，微内核只实现基本功能
KernelObjs      = $(tk)/start.o $(tk)/protect.o $(tk)/kernel_386_lib.o \
                  $(tk)/table.o $(tk)/main.o $(tk)/process.o \
                  $(tk)/message.o $(tk)/exception.o $(tk)/system.o \
                  $(tk)/clock.o $(tk)/tty.o $(tk)/keyboard.o \
                  $(tk)/console.o $(tk)/i8259.o  $(tk)/dmp.o \
                  $(tk)/misc.o $(tk)/driver.o $(tk)/at_wind.o

# 运行在系统上的服务进程和起源进程，现在有：MM、FS、FLY、ORIGIN
ProcObjs        = $(tog)/origin.o \
                  $(tmm)/main.o \
                  $(tmm)/table.o $(tmm)/utils.o $(tmm)/alloc.o $(tmm)/forkexit.o \
                  $(tmm)/misc.o \
                  $(tmm)/exec.o \
                  $(tfs)/main.o \
                  $(tfly)/main.o

LibObjs         = $(tl)/i386/message.o \
                  $(tl)/ansi/string.o $(tl)/syslib/kernel_debug.o $(tl)/syslib/kprintf.o \
                  $(tl)/putk.o $(tl)/ansi/stringc.o

Objs			= $(FlyanxKernelHead) $(KernelObjs) $(LibObjs) $(ProcObjs)

DASMOutPut		= kernel.bin.asm

# ===============================================
# 默认选项，提示如何编译
nop:
	@echo "哎呀，你为什么不试试'make image'或者'make all'呢？ ^ - ^"
# 编译全部
all: everything
	@echo "已经编译并生成 Flyanx OS 内核！！！"
# 只编译链接内核（不包括boot）
kernel: $(FlyanxKernel)
# 所有的伪命令，将不会被识别为文件
.PHONY : all everything image debug run realclean clean buildimg

# ===============================================
# 所有的伪命令

everything: $(FlyanxBoot) $(FlyanxKernel)

only_kernel: $(FlyanxKernel)

# 将生成 boot sector　和　kernel　二进制文件写入系统软盘镜像中
image: $(FD) $(FlyanxBoot)
	dd if=$(tb)/boot.bin of=$(FD) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(FD) $(FloppyMountPoint)
	sudo cp -fv $(tb)/loader.bin $(FloppyMountPoint)
	sudo cp -fv $(FlyanxKernel) $(FloppyMountPoint)
	sudo umount $(FloppyMountPoint)

#　使用 bochs 来进行 debug
debug: $(FD)
	bochs -q

# flyanx现在使用软盘+硬盘启动
run: $(FD)
	@echo "请使用Vbox虚拟机启动Flyanx，先挂载挂生成的Flyanx.img镜像到软盘上，然后下载已经格式化的硬盘从：htttp://。"
	@echo "然后挂载这个硬盘，启动它！OK :)"
# 	qemu-system-i386 -drive file=$(FD),if=floppy


# 完全清理，包括生成的boot和内核文件（二进制文件）
realclean:
	-rm -f $(FlyanxBoot) $(FlyanxKernel) $(Objs)

# 清理所有中间编译文件
clean:
	-rm -f $(Objs)

# 制作系统镜像
buildimg:
	dd if=$(tb)/boot.bin of=$(FD) bs=512 count=1 conv=notrunc
	dd if=$(tb)/hd_boot.bin of=$(HD) bs=1 count=446 conv=notrunc
	dd if=boot/hd_boot.bin of=$(HD) seek=510 skip=510 bs=1 count=2 conv=notrunc
	sudo mount -o loop $(FD) $(FloppyMountPoint)
	sudo cp -fv $(tb)/loader.bin $(FloppyMountPoint)
	sudo cp -fv $(FlyanxKernel) $(FloppyMountPoint)
	sudo umount $(FloppyMountPoint)

# ===============================================
# 所有的文件生成规则

# 镜像
$(FD):
	dd if=/dev/zero of=$(FD) bs=512 count=2880
	mkfs.fat $(FD)

# 内核加载器
$(tb)/boot.bin: src/boot/include/load.inc
$(tb)/boot.bin: src/boot/include/fat12hdr.inc
$(tb)/boot.bin : src/boot/boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

$(tb)/loader.bin : src/boot/loader.asm src/boot/include/load.inc src/boot/include/fat12hdr.inc src/boot/include/pm.inc
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# 内核
$(FlyanxKernel): $(Objs)
	$(LD) $(LDFlags) -o $(FlyanxKernel) $(Objs)

$(tk)/kernel.o: $(sk)/kernel.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tk)/start.o: $(ka)
$(tk)/start.o: $(sk)/protect.h
$(tk)/start.o: $(sk)/start.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/i8259.o: $(ka)
$(tk)/i8259.o: $(sk)/i8259.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/main.o: $(ka)
$(tk)/main.o: $i/unistd.h
$(tk)/main.o: $i/signal.h
$(tk)/main.o: $h/callnr.h
$(tk)/main.o: $h/common.h
$(tk)/main.o: $(sk)/process.h
$(tk)/main.o: $(sk)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/protect.o: $(ka)
$(tk)/protect.o: $(sk)/process.h
$(tk)/protect.o: $(sk)/protect.h
$(tk)/protect.o: $(sk)/protect.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/kernel_386_lib.o: $(sk)/kernel_386_lib.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/ansi/string.o: src/lib/ansi/string.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/ansi/stringc.o: $i/string.h
$(tl)/ansi/stringc.o: src/lib/ansi/stringc.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/kernel_debug.o: $(lib)
$(tl)/syslib/kernel_debug.o: src/lib/syslib/kernel_debug.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/exception.o: $(ka)
$(tk)/exception.o: $i/signal.h
$(tk)/exception.o: $(sk)/process.h
$(tk)/exception.o: $(sk)/exception.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/system.o: $(ka)
$(tk)/system.o:	$i/stdlib.h
$(tk)/system.o:	$i/signal.h
$(tk)/system.o:	$i/unistd.h
# $(tk)/system.o:	$s/sigcontext.h
# $(tk)/system.o:	$s/ptrace.h
# $(tk)/system.o:	$s/svrctl.h
$(tk)/system.o:	$h/callnr.h
$(tk)/system.o:	$h/common.h
$(tk)/system.o:	$(sk)/process.h
$(tk)/system.o:	$(sk)/protect.h
# $(tk)/system.o:	$(sk)/assert.h
$(tk)/system.o: $(sk)/system.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/table.o:	$(ka)
$(tk)/table.o:	$i/stdlib.h
# $(tk)/table.o:	$i/termios.h
$(tk)/table.o:	$h/common.h
$(tk)/table.o:	$(sk)/process.h
# $(tk)/table.o:	$(sk)/tty.h
$(tk)/table.o:	$b/int86.h
$(tk)/table.o: $(sk)/table.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/clock.o: $(ka)
$(tk)/clock.o: $i/signal.h
$(tk)/clock.o: $h/callnr.h
$(tk)/clock.o: $h/common.h
$(tk)/clock.o: $(sk)/process.h
$(tk)/clock.o: $(sk)/clock.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/process.o: $(ka)
$(tk)/process.o: $h/callnr.h
$(tk)/process.o: $h/common.h
$(tk)/process.o: $(sk)/process.h
$(tk)/process.o: $(sk)/process.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/process.o: $(ka)
$(tk)/process.o: $h/callnr.h
$(tk)/process.o: $h/common.h
$(tk)/process.o: $(sk)/process.h
$(tk)/message.o: $(sk)/message.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/console.o: $(ka)
$(tk)/console.o: $i/termios.h
$(tk)/console.o: $h/callnr.h
$(tk)/console.o: $h/common.h
$(tk)/console.o: $(sk)/protect.h
$(tk)/console.o: $(sk)/tty.h
$(tk)/console.o: $(sk)/process.h
$(tk)/console.o: $i/stdarg.h
$(tk)/console.o: $(sk)/console.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/keyboard.o: $(ka)
$(tk)/keyboard.o: $i/termios.h
$(tk)/keyboard.o: $i/signal.h
$(tk)/keyboard.o: $i/unistd.h
$(tk)/keyboard.o: $h/callnr.h
$(tk)/keyboard.o: $h/common.h
$(tk)/keyboard.o: $h/keymap.h
$(tk)/keyboard.o: $(sk)/tty.h
$(tk)/keyboard.o: $(sk)/keymaps/us-std.src
$(tk)/keyboard.o: $(sk)/keyboard.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/tty.o: $(ka)
$(tk)/tty.o: $i/termios.h
$(tk)/tty.o: $s/ioctl.h
$(tk)/tty.o: $i/signal.h
$(tk)/tty.o: $h/callnr.h
$(tk)/tty.o: $h/common.h
$(tk)/tty.o: $h/keymap.h
$(tk)/tty.o: $(sk)/tty.h
$(tk)/tty.o: $(sk)/process.h
$(tk)/tty.o: $(sk)/tty.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/dmp.o: $(ka)
$(tk)/dmp.o: $h/common.h
$(tk)/dmp.o: $(sk)/process.h
$(tk)/dmp.o: $(sk)/dmp.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/misc.o: $(ka)
$(tk)/misc.o: $(sk)/assert.h
$(tk)/misc.o: $i/stdlib.h
$(tk)/misc.o: $h/common.h
$(tk)/misc.o: $i/elf.h
$(tk)/misc.o: $(sk)/misc.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/driver.o: $(a)
$(tk)/driver.o: $s/ioctl.h
$(tk)/driver.o: $h/callnr.h
$(tk)/driver.o: $h/common.h
$(tk)/driver.o: $(sk)/process.h
$(tk)/driver.o: $(sk)/driver.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/at_wind.o: $(a)
$(tk)/at_wind.o: $(sk)/drvlib.h
$(tk)/at_wind.o: $h/callnr.h
$(tk)/at_wind.o: $h/common.h
$(tk)/at_wind.o: $s/ioctl.h
$(tk)/at_wind.o: $(sk)/process.h
$(tk)/at_wind.o: $(sk)/hd.h
$(tk)/at_wind.o: $(sk)/assert.h
$(tk)/at_wind.o: $(sk)/at_wind.c
	$(CC) $(CFlags) -o $@ $<

# 系统库
$(tl)/i386/message.o: src/lib/i386/message.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/syslib/kprintf.o: $i/stdarg.h
$(tl)/syslib/kprintf.o: $i/stddef.h
$(tl)/syslib/kprintf.o: $i/limits.h
$(tl)/syslib/kprintf.o: src/lib/syslib/kprintf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/putk.o: $i/lib.h
$(tl)/putk.o: $h/common.h
$(tl)/putk.o: $h/syslib.h
$(tl)/putk.o: $h/callnr.h
$(tl)/putk.o: $h/flylib.h
$(tl)/putk.o: src/lib/syslib/putk.c
	$(CC) $(CFlags) -o $@ $<

# 服务器和起源进程
# MM内存管理器服务器
$(tmm)/main.o: $(mma)
$(tmm)/main.o: $h/callnr.h
$(tmm)/main.o: $h/common.h
$(tmm)/main.o: $i/signal.h
$(tmm)/main.o: $i/fcntl.h
$(tmm)/main.o: $s/ioctl.h
$(tmm)/main.o: $(smm)/mmproc.h
$(tmm)/main.o: $(smm)/param.h
$(tmm)/main.o: $(smm)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/table.o: $(mma)
$(tmm)/table.o: $h/callnr.h
$(tmm)/table.o: $i/signal.h
$(tmm)/table.o: $(smm)/mmproc.h
$(tmm)/table.o: $(smm)/param.h
$(tmm)/table.o: $(smm)/table.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/alloc.o: $(mma)
$(tmm)/alloc.o: $h/common.h
$(tmm)/alloc.o: $h/callnr.h
$(tmm)/alloc.o: $i/signal.h
$(tmm)/alloc.o: $(smm)/mmproc.h
$(tmm)/alloc.o: $(smm)/alloc.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/forkexit.o: $(mma)
$(tmm)/forkexit.o: $i/signal.h
$(tmm)/forkexit.o: $(smm)/mmproc.h
$(tmm)/forkexit.o: $(smm)/param.h
$(tmm)/forkexit.o: $(smm)/forkexit.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/misc.o: $(mma)
$(tmm)/misc.o: $s/wait.h
$(tmm)/misc.o: $h/callnr.h
$(tmm)/misc.o: $i/signal.h
$(tmm)/misc.o: $(smm)/mmproc.h
$(tmm)/misc.o: $(smm)/param.h
$(tmm)/misc.o: $(smm)/misc.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/exec.o: $(mma)
$(tmm)/exec.o: $s/stat.h
$(tmm)/exec.o: $h/callnr.h
$(tmm)/exec.o: $i/elf.h
$(tmm)/exec.o: $i/signal.h
$(tmm)/exec.o: $i/string.h
$(tmm)/exec.o: $(smm)/mmproc.h
$(tmm)/exec.o: $(smm)/param.h
$(tmm)/exec.o: $(smm)/exec.c
	$(CC) $(CFlags) -o $@ $<

$(tmm)/utils.o: $(mma)
$(tmm)/utils.o: $(smm)/utils.c
	$(CC) $(CFlags) -o $@ $<

# FS文件系统服务器
$(tfs)/main.o: $(fsa)
$(tfs)/main.o: $(sfs)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tfly)/main.o: $(flya)
$(tfly)/main.o: $(sfly)/main.c
	$(CC) $(CFlags) -o $@ $<

# ORIGIN起源进程
$(tog)/origin.o: $(sog)/origin.c
	$(CC) $(CFlags) -o $@ $<

# ===============================================

# vim:ft=make
#
