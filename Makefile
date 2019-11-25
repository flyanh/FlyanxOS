#
# Makefile
# Created by flyan on 2019/11/9.
# QQ: 1341662010
# QQ-Group:909830414
# gitee: https://gitee.com/flyanh/
#

# ===============================================
# 所有的变量

# 头文件目录
u = /usr
i = include
h = $i/flyanx
s = $i/sys
b = $i/ibm
l = $i/lib

# c文科目录
t = target
tb = $t/boot
tk = $t/kernel
tl = $t/lib

# FlyanxOS　的内存装载点
# 这个值必须存在且相等在文件"load.inc"中的 'KernelEntryPointPhyAddr'！
ENTRYPOINT 		= 0x30400

# 内核文件的挂载点偏移地址
# 它必须和 ENTRYPOINT 相同
ENRTYOFFSET 	= 0x400

# 所需要的软盘镜像，可以自指定已存在的软盘镜像，如果不存在，将创建新的
OSImage			= Flyanx.img
# 镜像挂载点，自指定，必须存在于自己的计算机上，如果没有请自行创建一下
FloppyMountPoint= /media/floppyDisk

# 所使用的编译程序，参数
ASM 			= nasm
DASM 			= ndisasm
CC 				= gcc
LD				= ld
ASMFlagsOfBoot	= -I src/boot/include/
ASMFlagsOfKernel= -f elf -I src/kernel/
CFlags			= -I$i -c -fno-builtin
LDFlags			= -s -Ttext $(ENTRYPOINT)
DASMFlags		= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# 依赖关系
a = src/kernel/kernel.h $h/config.h $h/const.h $h/type.h $h/syslib.h \
    $s/types.h $i/string.h $i/limits.h $i/errno.h \
    src/kernel/const.h src/kernel/type.h src/kernel/prototype.h src/kernel/global.h

lib = $i/lib.h $h/common.h $h/syslib.h


# ===============================================
# 目标程序以及编译中间文件

FlyanxBoot		= $(tb)/boot.bin $(tb)/loader.bin
FlyanxKernel	= $(tk)/kernel.bin
FlyanxKernelHead = $(tk)/kernel.o

KernelObjs      = $(tk)/start.o $(tk)/protect.o $(tk)/kernel_386_lib.o \
                  $(tk)/table.o $(tk)/main.o $(tk)/process.o \
                  $(tk)/message.o $(tk)/exception.o $(tk)/system.o \
                  $(tk)/clock.o $(tk)/tty.o $(tk)/keyboard.o \
                  $(tk)/console.o $(tk)/i8259.o  $(tk)/dmp.o \
                  $(tk)/misc.o

LibObjs         = $(tl)/i386/message.o \
                  $(tl)/syslib/string.o $(tl)/syslib/kernel_debug.o $(tl)/syslib/kprintf.o
Objs			= $(FlyanxKernelHead) $(KernelObjs) $(LibObjs)

DASMOutPut		= kernel.bin.asm

# ===============================================
# 默认选项，编译全部
all: everything
	@echo "已经编译并生成 Flyanx OS 内核！！！"
# 只编译链接内核
kernel: $(FlyanxKernel)
# 所有的伪命令，将不会被识别为文件
.PHONY : all everything image debug run realclean clean

# ===============================================
# 所有的伪命令

everything: $(FlyanxBoot) $(FlyanxKernel)

only_kernel: $(FlyanxKernel)

# 将生成 boot sector　和　kernel　二进制文件写入系统软盘镜像中
image: $(OSImage) $(FlyanxBoot)
	dd if=$(tb)/boot.bin of=$(OSImage) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(OSImage) $(FloppyMountPoint)
	sudo cp -fv $(tb)/loader.bin $(FloppyMountPoint)
	sudo cp -fv $(FlyanxKernel) $(FloppyMountPoint)
	sudo umount $(FloppyMountPoint)

#　使用 bochs 来进行 debug
debug: $(OSImage)
	bochs -q

# 使用 qemu 来运行查看我们的系统镜像,如果需要更真实的模拟,请使用虚拟机,例如vbox,vpc或微软的vm
run: $(OSImage)
	qemu-system-i386 -drive file=$(OSImage),if=floppy

# 完全清理，包括生成的boot和内核文件（二进制文件）
realclean:
	-rm -f $(FlyanxBoot) $(FlyanxKernel) $(Objs)

# 清理所有中间编译文件
clean:
	-rm -f $(Objs)

# ===============================================
# 所有的文件生成规则

# 镜像
$(OSImage):
	dd if=/dev/zero of=$(OSImage) bs=512 count=2880
	mkfs.fat $(OSImage)

# BootLoader
$(tb)/boot.bin: src/boot/include/load.inc
$(tb)/boot.bin: src/boot/include/fat12hdr.inc
$(tb)/boot.bin : src/boot/boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

$(tb)/loader.bin : src/boot/loader.asm src/boot/include/load.inc src/boot/include/fat12hdr.inc src/boot/include/pm.inc
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# 内核
$(FlyanxKernel): $(Objs)
	$(LD) $(LDFlags) -o $(FlyanxKernel) $(Objs)

$(tk)/kernel.o: src/kernel/kernel.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tk)/start.o: $a
$(tk)/start.o: src/kernel/protect.h
$(tk)/start.o: src/kernel/start.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/i8259.o: $a
$(tk)/i8259.o: src/kernel/i8259.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/main.o: $a
$(tk)/main.o: $i/unistd.h
$(tk)/main.o: $i/signal.h
$(tk)/main.o: $i/a.out.h
$(tk)/main.o: $h/callnr.h
$(tk)/main.o: $h/common.h
$(tk)/main.o: src/kernel/process.h
$(tk)/main.o: src/kernel/main.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/protect.o: $a
$(tk)/protect.o: src/kernel/process.h
$(tk)/protect.o: src/kernel/protect.h
$(tk)/protect.o: src/kernel/protect.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/kernel_386_lib.o: src/kernel/kernel_386_lib.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/syslib/string.o: src/lib/syslib/string.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/syslib/kernel_debug.o: $(lib)
$(tl)/syslib/kernel_debug.o: src/lib/syslib/kernel_debug.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/exception.o: $a
$(tk)/exception.o: $i/signal.h
$(tk)/exception.o: src/kernel/process.h
$(tk)/exception.o: src/kernel/exception.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/system.o: $a
$(tk)/system.o:	$i/stdlib.h
$(tk)/system.o:	$i/signal.h
$(tk)/system.o:	$i/unistd.h
# $(tk)/system.o:	$s/sigcontext.h
# $(tk)/system.o:	$s/ptrace.h
# $(tk)/system.o:	$s/svrctl.h
$(tk)/system.o:	$h/callnr.h
$(tk)/system.o:	$h/common.h
$(tk)/system.o:	src/kernel/process.h
$(tk)/system.o:	src/kernel/protect.h
# $(tk)/system.o:	src/kernel/assert.h
$(tk)/system.o: src/kernel/system.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/table.o:	$a
$(tk)/table.o:	$i/stdlib.h
# $(tk)/table.o:	$i/termios.h
$(tk)/table.o:	$h/common.h
$(tk)/table.o:	src/kernel/process.h
# $(tk)/table.o:	src/kernel/tty.h
$(tk)/table.o:	$b/int86.h
$(tk)/table.o: src/kernel/table.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/clock.o: $a
$(tk)/clock.o: $i/signal.h
$(tk)/clock.o: $h/callnr.h
$(tk)/clock.o: $h/common.h
$(tk)/clock.o: src/kernel/clock.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/process.o: $a
$(tk)/process.o: $h/callnr.h
$(tk)/process.o: $h/common.h
$(tk)/process.o: src/kernel/process.h
$(tk)/process.o: src/kernel/process.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/process.o: $a
$(tk)/process.o: $h/callnr.h
$(tk)/process.o: $h/common.h
$(tk)/process.o: src/kernel/process.h
$(tk)/message.o: src/kernel/message.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/console.o: $a
$(tk)/console.o: $i/termios.h
$(tk)/console.o: $h/callnr.h
$(tk)/console.o: $h/common.h
$(tk)/console.o: src/kernel/protect.h
$(tk)/console.o: src/kernel/tty.h
$(tk)/console.o: src/kernel/process.h
$(tk)/console.o: src/kernel/console.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/keyboard.o: $a
$(tk)/keyboard.o: $i/termios.h
$(tk)/keyboard.o: $i/signal.h
$(tk)/keyboard.o: $i/unistd.h
$(tk)/keyboard.o: $h/callnr.h
$(tk)/keyboard.o: $h/common.h
$(tk)/keyboard.o: $h/keymap.h
$(tk)/keyboard.o: src/kernel/tty.h
$(tk)/keyboard.o: src/kernel/keymaps/us-std.src
$(tk)/keyboard.o: src/kernel/keyboard.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/tty.o: $a
$(tk)/tty.o: $i/termios.h
$(tk)/tty.o: $s/ioctl.h
$(tk)/tty.o: $i/signal.h
$(tk)/tty.o: $h/callnr.h
$(tk)/tty.o: $h/common.h
$(tk)/tty.o: $h/keymap.h
$(tk)/tty.o: src/kernel/tty.h
$(tk)/tty.o: src/kernel/process.h
$(tk)/tty.o: src/kernel/tty.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/dmp.o: $a
$(tk)/dmp.o: $h/common.h
$(tk)/dmp.o: src/kernel/process.h
$(tk)/dmp.o: src/kernel/dmp.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/misc.o: $a
$(tk)/misc.o: src/kernel/assert.h
$(tk)/misc.o: $i/stdlib.h
$(tk)/misc.o: $h/common.h
$(tk)/misc.o: src/kernel/misc.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/i386/message.o: src/lib/i386/message.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/syslib/kprintf.o: $i/stdarg.h
$(tl)/syslib/kprintf.o: $i/stddef.h
$(tl)/syslib/kprintf.o: $i/limits.h
$(tl)/syslib/kprintf.o: src/lib/syslib/kprintf.c
	$(CC) $(CFlags) -o $@ $<

# ===============================================

# vim:ft=make
#
