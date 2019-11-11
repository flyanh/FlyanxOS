#
# Makefile
# Created by flyan on 2019/11/9.
# QQ: 1341662010
# QQ-Group:909830414
# gitee: https://gitee.com/flyanh/
#

# ===============================================
# 所有的变量

# 目录
u = /usr
i = include
h = $i/flyanx
s = $i/sys
b = $i/ibm
l = $i/lib

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
ASMFlagsOfKernel= -f elf -I include/
CFlags			= -o2 -I include/ -c -fno-builtin
LDFlags			= -s -Ttext $(ENTRYPOINT)
DASMFlags		= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# 依赖关系
a = src/kernel/kernel.h $h/config.h $h/const.h $h/type.h $h/syslib.h \
    $s/types.h $i/string.h $i/limits.h $i/errno.h \
    src/kernel/const.h src/kernel/type.h src/kernel/prototype.h src/kernel/gloal.h

lib = $i/lib.h $h/common.h $h/syslib.h


# ===============================================
# 目标程序以及编译中间文件

FlyanxBoot		= target/boot/boot.bin target/boot/loader.bin
FlyanxKernel	= target/kernel/kernel.bin
FlyanxKernelHead = target/kernel/kernel.o
KernelObjs      = target/kernel/start.o target/kernel/i8259.o \
                  target/kernel/main.o target/kernel/protect.o target/kernel/exception.o \
                  target/kernel/system.o target/kernel/table.o
LibObjs         = target/lib/syslib/kernel_lib.o target/lib/syslib/string.o \
                  target/lib/syslib/kernel_debug.o
Objs			= $(FlyanxKernelHead) $(LibObjs) $(KernelObjs)

DASMOutPut		= kernel.bin.asm

# ===============================================
# 默认选项，编译全部
all: realclean everything clean
	@echo "正在编译生成 Flyanx OS 内核..."
# 所有的伪命令，将不会被识别为文件
.PHONY : all everything image debug run realclean clean

# ===============================================
# 所有的伪命令

everything: $(FlyanxBoot) $(FlyanxKernel)

# 将生成 boot sector　和　kernel　二进制文件写入系统软盘镜像中
image: $(OSImage) $(FlyanxBoot)
	dd if=target/boot/boot.bin of=$(OSImage) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(OSImage) $(FloppyMountPoint)
	sudo cp -fv target/boot/loader.bin $(FloppyMountPoint)
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
target/boot/boot.bin: src/boot/include/load.inc
target/boot/boot.bin: src/boot/include/fat12hdr.inc
target/boot/boot.bin : src/boot/boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

target/boot/loader.bin : src/boot/loader.asm src/boot/include/load.inc src/boot/include/fat12hdr.inc src/boot/include/pm.inc
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# 内核
$(FlyanxKernel): $(Objs)
	$(LD) $(LDFlags) -o $(FlyanxKernel) $(Objs)

target/kernel/kernel.o: include/kernelConst.inc
target/kernel/kernel.o: src/kernel/kernel.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

target/kernel/start.o: $a
target/kernel/start.o: src/kernel/protect.h
target/kernel/start.o: src/kernel/start.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/i8259.o: $a
target/kernel/i8259.o: src/kernel/i8259.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/main.o: $a
target/kernel/main.o: $i/unistd.h
target/kernel/main.o: $i/signal.h
target/kernel/main.o: $i/a.out.h
target/kernel/main.o: $h/callnr.h
target/kernel/main.o: $h/common.h
target/kernel/main.o: src/kernel/main.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/protect.o: $a
target/kernel/protect.o: src/kernel/process.h
target/kernel/protect.o: src/kernel/protect.h
target/kernel/protect.o: src/kernel/protect.c
	$(CC) $(CFlags) -o $@ $<

target/lib/syslib/kernel_lib.o: include/kernelConst.inc
target/lib/syslib/kernel_lib.o: src/lib/syslib/kernel_lib.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

target/lib/syslib/string.o: src/lib/syslib/string.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

target/lib/syslib/kernel_debug.o: $(lib)
target/lib/syslib/kernel_debug.o: src/lib/syslib/kernel_debug.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/exception.o: $a
target/kernel/exception.o: $i/signal.h
target/kernel/exception.o: src/kernel/process.h
target/kernel/exception.o: src/kernel/exception.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/system.o: $a
target/kernel/system.o:	$i/stdlib.h
target/kernel/system.o:	$i/signal.h
target/kernel/system.o:	$i/unistd.h
# target/kernel/system.o:	$s/sigcontext.h
# target/kernel/system.o:	$s/ptrace.h
# target/kernel/system.o:	$s/svrctl.h
target/kernel/system.o:	$h/callnr.h
target/kernel/system.o:	$h/common.h
target/kernel/system.o:	src/kernel/process.h
target/kernel/system.o:	src/kernel/protect.h
# target/kernel/system.o:	src/kernel/assert.h
target/kernel/system.o: src/kernel/system.c
	$(CC) $(CFlags) -o $@ $<

target/kernel/table.o:	$a
target/kernel/table.o:	$i/stdlib.h
# target/kernel/table.o:	$i/termios.h
target/kernel/table.o:	$h/common.h
target/kernel/table.o:	src/kernel/process.h
# target/kernel/table.o:	src/kernel/tty.h
target/kernel/table.o:	$b/int86.h
target/kernel/table.o: src/kernel/table.c
	$(CC) $(CFlags) -o $@ $<


# ===============================================

# vim:ft=make
#
