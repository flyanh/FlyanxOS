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
HD          = 500M_HD.img

# 镜像挂载点，自指定，必须存在于自己的计算机上，如果没有请自行创建一下
FloppyMountPoint= /media/floppyDisk

# 所使用的编译程序，参数
ASM 			= nasm
DASM 			= ndisasm
CC 				= gcc
LD				= ld
ASMFlagsOfBoot	= -I src/boot/include/
ASMFlagsOfKernel= -f elf -I $(sk)/
ASMFlagsOfSysCall = -f elf
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

FlyanxBoot		= $(tb)/boot.bin $(tb)/loader.bin $(tb)/hd_boot.bin $(tb)/hd_loader.bin
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
                  $(tfs)/main.o $(tfs)/table.o $(tfs)/device.o $(tfs)/utils.o \
                  $(tfs)/super.o $(tfs)/inode.o $(tfs)/open.o $(tfs)/file.o \
                  $(tfs)/path.o $(tfs)/read_write.o $(tfs)/link.o $(tfs)/statdir.o \
                  $(tfs)/pipe.o $(tfs)/misc.o \
                  $(tfly)/main.o $(tfly)/table.o $(tfly)/utils.o $(tfly)/misc.o

LibObjs         = $(tl)/i386/message.o \
                  $(tl)/ansi/string.o $(tl)/ansi/memcmp.o $(tl)/syslib/kernel_debug.o $(tl)/syslib/kprintf.o \
                  $(tl)/syslib/putk.o $(tl)/ansi/stringc.o $(tl)/syslib/task_call.o \
                  $(tl)/syslib/sys_sudden.o $(tl)/syslib/sys_blues.o $(tl)/syslib/sys_copy.o \
                  $(tl)/other/loadname.o $(tl)/other/syscall.o $(tl)/other/errno.o $(tl)/other/_sleep.o \
                  $(tl)/stdio/vsprintf.o $(tl)/stdio/printf.o $(tl)/stdio/sprintf.o \
                  $(tl)/posix/_open.o $(tl)/posix/_creat.o $(tl)/posix/_close.o $(tl)/posix/_mkdir.o \
                  $(tl)/posix/_read.o $(tl)/posix/_write.o $(tl)/posix/_link.o $(tl)/posix/_unlink.o \
                  $(tl)/posix/_lseek.o $(tl)/posix/_stat.o $(tl)/posix/_fstat.o \
                  $(tl)/syscall/open.o $(tl)/syscall/creat.o $(tl)/syscall/close.o \
                  $(tl)/syscall/mkdir.o $(tl)/syscall/read.o $(tl)/syscall/write.o \
                  $(tl)/syscall/link.o $(tl)/syscall/unlink.o $(tl)/syscall/lseek.o \
                  $(tl)/syscall/stat.o $(tl)/syscall/fstat.o $(tl)/syscall/sleep.o

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

image: buildimg

#　使用 bochs 来进行 debug
debug: $(FD)
	bochs -q

# flyanx现在使用软盘+硬盘启动
run: $(FD) harddisk.tar.gz
	@echo "请使用VBox虚拟机先挂载挂生成的Flyanx.img镜像到软盘上，然后解压harddisk.tar.gz下的\
	硬盘镜像，然后挂载500M_HD.vdi到VBox的硬盘上，Flyanx就可以启动了 :)"


# 完全清理，包括生成的boot和内核文件（二进制文件）
realclean:
	-rm -f $(FlyanxBoot) $(FlyanxKernel) $(Objs)

# 清理所有中间编译文件
clean:
	-rm -f $(Objs)

# 制作系统镜像
buildimg: $(FD $(HD) $(FlyanxBoot)
	dd if=$(tb)/boot.bin of=$(FD) bs=512 count=1 conv=notrunc
# 	dd if=$(tb)/hd_boot.bin of=$(HD) bs=1 count=446 conv=notrunc
# 	dd if=$(tb)/hd_boot.bin of=$(HD) seek=510 skip=510 bs=1 count=2 conv=notrunc
	sudo mount -o loop $(FD) $(FloppyMountPoint)
	sudo cp -fv $(tb)/loader.bin $(FloppyMountPoint)
	sudo cp -fv $(FlyanxKernel) $(FloppyMountPoint)
	sudo umount $(FloppyMountPoint)

# ===============================================
# 所有的文件生成规则
# ============ 镜像 ============
$(FD):
	dd if=/dev/zero of=$(FD) bs=512 count=2880
	mkfs.fat $(FD)

# ============ 内核加载器 ============
$(tb)/boot.bin: src/boot/include/load.inc
$(tb)/boot.bin: src/boot/include/fat12hdr.inc
$(tb)/boot.bin : src/boot/boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

$(tb)/loader.bin : src/boot/loader.asm src/boot/include/load.inc src/boot/include/fat12hdr.inc src/boot/include/pm.inc
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

$(tb)/hd_boot.bin: src/boot/include/load.inc
$(tb)/hd_boot.bin: src/boot/hd_boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

$(tb)/hd_loader.bin: src/boot/include/load.inc
$(tb)/hd_loader.bin: src/boot/include/pm.inc
$(tb)/hd_loader.bin: src/boot/hd_loader.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# ============ 内核 ============
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
$(tk)/main.o: $h/common.h
$(tk)/main.o: $(sk)/protect.h
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

$(tl)/ansi/memcmp.o: $i/string.h
$(tl)/ansi/memcmp.o: src/lib/ansi/memcmp.c
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
$(tk)/system.o:	$(sk)/assert.h
$(tk)/system.o: $(sk)/system.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/table.o:	$(ka)
$(tk)/table.o:	$i/stdlib.h
$(tk)/table.o:	$i/termios.h
$(tk)/table.o:	$h/common.h
$(tk)/table.o:	$(sk)/process.h
$(tk)/table.o:	$(sk)/tty.h
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

$(tk)/message.o: $(ka)
$(tk)/message.o: $h/callnr.h
$(tk)/message.o: $h/common.h
$(tk)/message.o: $(sk)/process.h
$(tk)/message.o: $(sk)/assert.h
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
$(tk)/at_wind.o: $h/callnr.h
$(tk)/at_wind.o: $h/common.h
$(tk)/at_wind.o: $s/ioctl.h
$(tk)/at_wind.o: $(sk)/process.h
$(tk)/at_wind.o: $(b)/partition.h
$(tk)/at_wind.o: $(h)/partition.h
$(tk)/at_wind.o: $(s)/dev.h
$(tk)/at_wind.o: $(sk)/hd.h
$(tk)/at_wind.o: $(sk)/assert.h
$(tk)/at_wind.o: $(sk)/at_wind.c
	$(CC) $(CFlags) -o $@ $<

# ============ 系统库 ============
$(tl)/i386/message.o: src/lib/i386/message.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/syslib/kprintf.o: $i/stdarg.h
$(tl)/syslib/kprintf.o: $i/stddef.h
$(tl)/syslib/kprintf.o: $i/limits.h
$(tl)/syslib/kprintf.o: src/lib/syslib/kprintf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/putk.o: $i/lib.h
$(tl)/syslib/putk.o: $h/common.h
$(tl)/syslib/putk.o: $h/syslib.h
$(tl)/syslib/putk.o: $h/callnr.h
$(tl)/syslib/putk.o: $h/flylib.h
$(tl)/syslib/putk.o: src/lib/syslib/putk.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/task_call.o: $i/lib.h
$(tl)/syslib/task_call.o: $h/syslib.h
$(tl)/syslib/task_call.o: src/lib/syslib/task_call.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/sys_sudden.o: $h/syslib.h
$(tl)/syslib/sys_sudden.o: $i/stdarg.h
$(tl)/syslib/sys_sudden.o: $i/unistd.h
$(tl)/syslib/sys_sudden.o: src/lib/syslib/sys_sudden.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/sys_blues.o: $h/syslib.h
$(tl)/syslib/sys_blues.o: src/lib/syslib/sys_blues.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/syslib/sys_copy.o: $h/syslib.h
$(tl)/syslib/sys_copy.o: src/lib/syslib/sys_copy.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/other/loadname.o: $i/lib.h
$(tl)/other/loadname.o: $i/string.h
$(tl)/other/loadname.o: src/lib/other/loadname.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/other/syscall.o: $i/lib.h
$(tl)/other/syscall.o: src/lib/other/syscall.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/other/errno.o: src/lib/other/errno.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/other/_sleep.o: $h/syslib.h
$(tl)/other/_sleep.o: $i/unistd.h
$(tl)/other/_sleep.o: src/lib/other/_sleep.c
	$(CC) $(CFlags) -o $@ $<

# 标准输入输出
$(tl)/stdio/vsprintf.o: $i/stdio.h
$(tl)/stdio/vsprintf.o: $i/stdarg.h
$(tl)/stdio/vsprintf.o: $i/unistd.h
$(tl)/stdio/vsprintf.o: src/lib/stdio/vsprintf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/printf.o: $i/stdio.h
$(tl)/stdio/printf.o: $i/stdarg.h
$(tl)/stdio/printf.o: $i/string.h
$(tl)/stdio/printf.o: src/lib/stdio/printf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/sprintf.o: $i/stdio.h
$(tl)/stdio/sprintf.o: $i/stdarg.h
$(tl)/stdio/sprintf.o: src/lib/stdio/sprintf.c
	$(CC) $(CFlags) -o $@ $<

# posix系统调用
$(tl)/posix/_open.o: $i/lib.h
$(tl)/posix/_open.o: $i/fcntl.h
$(tl)/posix/_open.o: $i/stdarg.h
$(tl)/posix/_open.o: $i/string.h
$(tl)/posix/_open.o: src/lib/posix/_open.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_creat.o: $i/lib.h
$(tl)/posix/_creat.o: $i/fcntl.h
$(tl)/posix/_creat.o: src/lib/posix/_creat.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_close.o: $i/lib.h
$(tl)/posix/_close.o: $i/unistd.h
$(tl)/posix/_close.o: src/lib/posix/_close.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_mkdir.o: $i/lib.h
$(tl)/posix/_mkdir.o: $s/stat.h
$(tl)/posix/_mkdir.o: $i/string.h
$(tl)/posix/_mkdir.o: src/lib/posix/_mkdir.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_read.o: $i/lib.h
$(tl)/posix/_read.o: $i/unistd.h
$(tl)/posix/_read.o: src/lib/posix/_read.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_write.o: $i/lib.h
$(tl)/posix/_write.o: $i/unistd.h
$(tl)/posix/_write.o: src/lib/posix/_write.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_link.o: $i/lib.h
$(tl)/posix/_link.o: $i/unistd.h
$(tl)/posix/_link.o: src/lib/posix/_link.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_unlink.o: $i/lib.h
$(tl)/posix/_unlink.o: $i/unistd.h
$(tl)/posix/_unlink.o: src/lib/posix/_unlink.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_lseek.o: $i/lib.h
$(tl)/posix/_lseek.o: $i/unistd.h
$(tl)/posix/_lseek.o: src/lib/posix/_lseek.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_stat.o: $i/lib.h
$(tl)/posix/_stat.o: $s/stat.h
$(tl)/posix/_stat.o: $i/string.h
$(tl)/posix/_stat.o: src/lib/posix/_stat.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/posix/_fstat.o: $i/lib.h
$(tl)/posix/_fstat.o: $s/stat.h
$(tl)/posix/_fstat.o: src/lib/posix/_fstat.c
	$(CC) $(CFlags) -o $@ $<

# 用户系统调用
$(tl)/syscall/open.o: src/lib/syscall/open.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/creat.o: src/lib/syscall/creat.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/close.o: src/lib/syscall/close.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/mkdir.o: src/lib/syscall/mkdir.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/read.o: src/lib/syscall/read.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/write.o: src/lib/syscall/write.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/link.o: src/lib/syscall/link.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/unlink.o: src/lib/syscall/unlink.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/lseek.o: src/lib/syscall/lseek.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/stat.o: src/lib/syscall/stat.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/fstat.o: src/lib/syscall/fstat.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

$(tl)/syscall/sleep.o: src/lib/syscall/sleep.asm
	$(ASM) $(ASMFlagsOfSysCall) -o $@ $<

# ============ 服务器和起源进程 ============
# ============ MM内存管理器服务器 ============
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
$(tmm)/forkexit.o: $s/wait.h
$(tmm)/forkexit.o: $h/callnr.h
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
$(tmm)/utils.o: $i/signal.h
$(tmm)/utils.o: $(smm)/mmproc.h
$(tmm)/utils.o: $(smm)/param.h
$(tmm)/utils.o: $(smm)/utils.c
	$(CC) $(CFlags) -o $@ $<

# ============ FS文件系统服务器 ============
$(tfs)/main.o: $(fsa)
$(tfs)/main.o: $h/callnr.h
$(tfs)/main.o: $h/common.h
$(tfs)/main.o: $h/partition.h
$(tfs)/main.o: $s/dev.h
$(tfs)/main.o: $(sfs)/dev.h
$(tfs)/main.o: $(sfs)/file.h
$(tfs)/main.o: $(sfs)/fsproc.h
$(tfs)/main.o: $(sfs)/inode.h
$(tfs)/main.o: $i/dir.h
$(tfs)/main.o: $(sfs)/super.h
$(tfs)/main.o: $(sfs)/param.h
$(tfs)/main.o: $(sfs)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/table.o: $(fsa)
$(tfs)/table.o: $h/callnr.h
$(tfs)/table.o: $h/common.h
$(tfs)/table.o: $(sfs)/file.h
$(tfs)/table.o: $(sfs)/fsproc.h
$(tfs)/table.o: $(sfs)/inode.h
$(tfs)/table.o: $(sfs)/super.h
$(tfs)/table.o: $(sfs)/table.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/device.o: $(fsa)
$(tfs)/device.o: $h/callnr.h
$(tfs)/device.o: $h/common.h
$(tfs)/device.o: $i/fcntl.h
$(tfs)/device.o: $s/dev.h
$(tfs)/device.o: $(sfs)/dev.h
$(tfs)/device.o: $(sfs)/param.h
$(tfs)/device.o: $(sfs)/device.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/super.o: $(fsa)
$(tfs)/super.o: $h/common.h
$(tfs)/super.o: $(sfs)/super.h
$(tfs)/super.o: $s/dev.h
$(tfs)/super.o: $(sfs)/dev.h
$(tfs)/super.o: $(sfs)/super.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/inode.o: $(fsa)
$(tfs)/inode.o: $h/common.h
$(tfs)/inode.o: $(sfs)/inode.h
$(tfs)/inode.o: $s/dev.h
$(tfs)/inode.o: $s/dev.h
$(tfs)/inode.o: $(sfs)/super.h
$(tfs)/inode.o: $(sfs)/inode.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/open.o: $(fsa)
$(tfs)/open.o: $h/callnr.h
$(tfs)/open.o: $h/common.h
$(tfs)/open.o: $i/fcntl.h
$(tfs)/open.o: $s/dev.h
$(tfs)/open.o: $(sfs)/dev.h
$(tfs)/open.o: $i/dir.h
$(tfs)/open.o: $(sfs)/file.h
$(tfs)/open.o: $(sfs)/param.h
$(tfs)/open.o: $(sfs)/open.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/file.o: $(fsa)
$(tfs)/file.o: $(sfs)/file.h
$(tfs)/file.o: $(sfs)/fsproc.h
$(tfs)/file.o: $(sfs)/inode.h
$(tfs)/file.o: $(sfs)/file.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/path.o: $(fsa)
$(tfs)/path.o: $i/string.h
$(tfs)/path.o: $h/callnr.h
$(tfs)/path.o: $h/common.h
$(tfs)/path.o: $(sfs)/file.h
$(tfs)/path.o: $(sfs)/fsproc.h
$(tfs)/path.o: $(sfs)/inode.h
$(tfs)/path.o: $s/dev.h
$(tfs)/path.o: $(sfs)/dev.h
$(tfs)/path.o: $(sfs)/super.h
$(tfs)/path.o: $i/dir.h
$(tfs)/path.o: $(sfs)/path.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/read_write.o: $(fsa)
$(tfs)/read_write.o: $i/fcntl.h
$(tfs)/read_write.o: $h/common.h
$(tfs)/read_write.o: $s/dev.h
$(tfs)/read_write.o: $(sfs)/dev.h
$(tfs)/read_write.o: $(sfs)/file.h
$(tfs)/read_write.o: $(sfs)/fsproc.h
$(tfs)/read_write.o: $(sfs)/inode.h
$(tfs)/read_write.o: $(sfs)/param.h
$(tfs)/read_write.o: $(sfs)/read_write.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/link.o: $(fsa)
$(tfs)/link.o: $s/stat.h
$(tfs)/link.o: $i/string.h
$(tfs)/link.o: $h/callnr.h
$(tfs)/link.o: $(sfs)/file.h
$(tfs)/link.o: $(sfs)/fsproc.h
$(tfs)/link.o: $(sfs)/inode.h
$(tfs)/link.o: $(sfs)/param.h
$(tfs)/link.o: $(sfs)/link.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/link.o: $(fsa)
$(tfs)/link.o: $s/stat.h
$(tfs)/link.o: $(sfs)/file.h
$(tfs)/link.o: $(sfs)/inode.h
$(tfs)/link.o: $(sfs)/param.h
$(tfs)/statdir.o: $(sfs)/statdir.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/pipe.o: $(fsa)
$(tfs)/pipe.o: $(sfs)/file.h
$(tfs)/pipe.o: $(sfs)/fsproc.h
$(tfs)/pipe.o: $(sfs)/inode.h
$(tfs)/pipe.o: $(sfs)/param.h
$(tfs)/pipe.o: $(sfs)/pipe.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/misc.o: $(fsa)
$(tfs)/misc.o: $i/fcntl.h
$(tfs)/misc.o: $i/unistd.h
$(tfs)/misc.o: $h/callnr.h
$(tfs)/misc.o: $h/common.h
$(tfs)/misc.o: $(sfs)/file.h
$(tfs)/misc.o: $(sfs)/fsproc.h
$(tfs)/misc.o: $(sfs)/inode.h
$(tfs)/misc.o: $s/dev.h
$(tfs)/misc.o: $(sfs)/dev.h
$(tfs)/misc.o: $(sfs)/param.h
$(tfs)/misc.o: $(sfs)/misc.c
	$(CC) $(CFlags) -o $@ $<

$(tfs)/utils.o: $(fsa)
$(tfs)/utils.o: $i/unistd.h
$(tfs)/utils.o: $h/common.h
$(tfs)/utils.o: $(sfs)/param.h
$(tfs)/utils.o: $(sfs)/utils.c
	$(CC) $(CFlags) -o $@ $<

# ============ FLY拓展管理器 ============
$(tfly)/main.o: $(flya)
$(tfly)/main.o: $h/callnr.h
$(tfly)/main.o: $h/common.h
$(tfly)/main.o: $(sfly)/param.h
$(tfly)/main.o: $(sfly)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tfly)/table.o: $(flya)
$(tfly)/table.o: $h/callnr.h
$(tfly)/table.o: $(sfly)/table.c
	$(CC) $(CFlags) -o $@ $<

$(tfly)/utils.o: $(flya)
$(tfly)/utils.o: $(sfly)/utils.c
	$(CC) $(CFlags) -o $@ $<

$(tfly)/misc.o: $(flya)
$(tfly)/misc.o: $(sfly)/param.h
$(tfly)/misc.o: $(sfly)/misc.c
	$(CC) $(CFlags) -o $@ $<

# ============ ORIGIN起源进程 ============
$(tog)/origin.o: $i/lib.h
$(tog)/origin.o: $i/fcntl.h
$(tog)/origin.o: $i/unistd.h
$(tog)/origin.o: $i/string.h
$(tog)/origin.o: $(sog)/origin.c
	$(CC) $(CFlags) -o $@ $<

# ===============================================

# vim:ft=make
#
