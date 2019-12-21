;; flyanx_prog_start.asm
;;

extern main
extern _exit

bits 32
[SECTION .text]
global _start

_start:
    push    ebx     ; 将envp压栈
    push    eax     ; argv
    push    ecx     ; argc

    call    main    ; 好的，现在执行用户程序的主函数

    push    eax     ; eax存放main函数的返回值
    call    _exit   ; _exit(eax)

    hlt             ; 它不应该到这


