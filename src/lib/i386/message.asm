;================================================================================================
; 文件：message.asm
; 作用：i386体系的消息通信向外暴露的的系统调用
; 作者： Flyan
; 联系QQ： 1341662010
;================================================================================================
; 库函数开始
[SECTION .lib]

; 导出库函数
global  send
global  receive
global  send_receive


; 常量定义，请查看include/flyanx/common.h文件下的定义或其他头文件
SEND        equ     1
RECEIVE     equ     2
SEND_REC    equ     3
SYS_VEC     equ     47         ; 系统调用向量

SRC_DEST    equ     4
MESSAGE     equ     8
;*========================================================================*
;                           send                            *
;                       执行系统调用SEND
;*========================================================================*
; 本例程执行系统调用函数，function为SEND，系统调用原型:
;   int sys_call(function, src_dest, message_ptr)
; 本例程只是对function = SEND的封装。
send:
    mov ecx, SEND               ; ecx = 调用操作是发送消息，function = SEND
    jmp com
;*========================================================================*
;                           receive                               *
;                         执行系统调用RECIIVE
;*========================================================================*
; 本例程执行系统调用函数，function为RECIIVE
receive:
    mov ecx, RECEIVE                ; ecx = 调用操作是发送消息，function = RECEIVE
    jmp com                      ; 公共的处理

;*========================================================================*
;                           send_receive                            *
;                           执行系统调用SEND_REC
;*========================================================================*
; 本例程执行系统调用函数，function为RECIIVE
send_receive:
    mov ecx, SEND_REC               ; ecx = 调用操作是发送消息，function = RECEIVE
    jmp com                      ; 公共的处理
; 公共处理
com:
    mov eax, [esp + SRC_DEST]          ; eax = 目标地址
    mov ebx, [esp + MESSAGE]          ; ebx = 消息缓冲首址
    int SYS_VEC                 ; 执行系统调用
    ret
