#设置CMake的最小版本需求
cmake_minimum_required(VERSION 3.9)
#设置工程名称
project(Flyanx)
#设置C语言所使用的标准
set(CMAKE_C_STANDARD 90)
#导入头文件目录，特别是针对于操作系统工程
include_directories(HEAD "./include")

#添加可执行文件，有main函数的文件，这里设置为内核的主函数
add_executable(Flyanx ./src/kernel/main.c src/kernel/kernel.h include/flyanx/config.h include/sys/types.h include/flyanx/syslib.h
        include/limits.h include/string.h include/unistd.h include/flyanx/callnr.h include/flyanx/common.h src/kernel/process.h
        src/lib/syslib/kprintf.c src/kernel/const.h src/kernel/type.h src/kernel/prototype.h src/kernel/global.h src/kernel/start.c
        src/kernel/protect.h src/kernel/protect.c src/kernel/i8259.c src/kernel/exception.c include/stdlib.h src/kernel/table.c
        src/kernel/system.c src/lib/syslib/syslib.h include/lib.h src/lib/syslib/kernel_debug.c src/origin/origin.c include/stdarg.h
        include/stddef.h src/lib/syslib/putk.c src/kernel/process.c src/kernel/message.c src/kernel/clock.c src/kernel/message.h
        src/kernel/keyboard.c include/flyanx/keymap.h src/kernel/tty.h include/termios.h src/kernel/tty.c src/kernel/misc.c
        src/kernel/dmp.c include/stdio.h src/kernel/test.c src/kernel/console.c include/flyanx/flylib.h
        include/sys/sigcontext.h include/sys/ptrace.h include/sys/svrctl.h include/ctype.h src/mm/mm.h src/fs/fs.h
        src/fs/const.h src/fs/type.h src/mm/prototype.h src/mm/global.h src/mm/main.c src/fs/main.c src/fly/main.c
        include/fcntl.h src/mm/utils.c src/mm/table.c src/fly/table.c src/mm/mmproc.h src/mm/param.h src/mm/alloc.c
        src/mm/fork_exit.c include/sys/wait.h src/mm/exec.c include/sys/stat.h src/lib/ansi/stringc.c src/kernel/at_wind.c
        src/kernel/driver.c include/flyanx/partition.h include/flyanx/u64.h src/kernel/hd.h src/kernel/at_wind.c
        src/mm/misc.c src/fs/file.h src/fs/table.c src/fs/utils.c src/fs/fsproc.h src/fs/super.h src/fs/inode.h include/dir.h
        include/sys/dev.h src/fs/dev.h src/fs/device.c src/fs/param.h src/lib/syslib/sys_sudden.c src/lib/syslib/task_call.c
        src/lib/syslib/sys_blues.c src/fs/super.c src/fs/inode.c src/fs/open.c src/lib/syslib/sys_copy.c src/fs/file.c
        src/fs/path.c src/lib/posix/_open.c src/lib/other/loadname.c src/lib/other/syscall.c src/lib/other/errno.c
        src/lib/posix/_creat.c src/lib/posix/_close.c src/lib/posix/_mkdir.c src/fs/read_write.c src/lib/posix/_read.c
        src/lib/posix/_write.c src/fs/link.c src/lib/ansi/memcmp.c src/lib/posix/_unlink.c src/lib/posix/_link.c src/fs/misc.c
        src/lib/posix/_lseek.c src/fs/statdir.c src/lib/posix/_fstat.c src/lib/posix/_stat.c src/fs/pipe.c src/lib/stdio/printf.c
        src/lib/stdio/vsprintf.c src/lib/other/_sleep.c src/fly/misc.c src/fly/utils.c src/fly/param.h src/lib/stdio/sprintf.c
        src/lib/syslib/sys_get_map.c src/lib/syslib/sys_fork.c src/lib/syslib/sys_new_map.c src/lib/posix/_fork.c
        src/mm/get_set.c src/lib/posix/_getpid.c src/lib/posix/_getppid.c src/lib/syslib/sys_exit.c src/lib/posix/_exit.c
        src/lib/posix/_wait.c src/lib/posix/_waitpid.c src/lib/posix/_execve.c src/mm/break.c src/lib/other/itoa.c
        src/lib/posix/_execv.c src/lib/posix/_exec.c src/lib/posix/_execl.c src/lib/syslib/sys_exec.c src/lib/syslib/sys_get_sp.c
        src/lib/syslib/sys_set_prog_frame.c src/commands/simple/ls.c src/commands/simple/pwd.c src/origin/tar.c src/internet/inet.c
        src/internet/inet.h
        src/origin/fly_shell.c)