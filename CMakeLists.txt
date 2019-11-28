#设置CMake的最小版本需求
cmake_minimum_required(VERSION 3.9)
#设置工程名称
project(Flyanx C)
#设置C语言所使用的标准
set(CMAKE_C_STANDARD 90)
#导入头文件目录，特别是针对于操作系统工程
include_directories(HEAD "./include")

#添加可执行文件，有main函数的文件，这里设置为内核的主函数
add_executable(Flyanx ./src/kernel/main.c src/kernel/kernel.h include/flyanx/config.h include/sys/types.h include/flyanx/syslib.h include/limits.h include/string.h include/unistd.h include/flyanx/callnr.h include/flyanx/common.h src/kernel/process.h src/lib/syslib/kprintf.c src/kernel/const.h src/kernel/type.h src/kernel/prototype.h src/kernel/global.h src/kernel/start.c src/kernel/protect.h src/kernel/protect.c src/kernel/i8259.c src/kernel/exception.c include/stdlib.h src/kernel/table.c src/kernel/system.c src/lib/syslib/syslib.h include/lib.h src/lib/syslib/kernel_debug.c src/origin/origin.c include/stdarg.h include/stddef.h src/lib/syslib/putk.c src/kernel/process.c src/kernel/message.c src/kernel/clock.c src/kernel/message.h src/kernel/keyboard.c include/flyanx/keymap.h src/kernel/tty.h include/termios.h src/kernel/tty.c src/kernel/misc.c src/kernel/dmp.c include/stdio.h src/kernel/test.c src/kernel/console.c)