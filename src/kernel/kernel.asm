;================================================================================================
; 文件：kernel.asm
; 作用：Flyanx系统内核文件
; 作者： Flyan
; QQ: 1341662010
; QQ-Group:909830414
; gitee: https://gitee.com/flyanh/
;================================================================================================
; 导入和导出

; 导入头文件
%include "kernelConst.inc"

; 导入函数
extern  cstart				; 改变gdt_ptr，让它指向新的GDT
extern  main	            ; 内核主函数
extern  disp_str			; 显示一个字符串
extern	spurious_irq	    ; 默认中断请求处理程序
extern	exception_handler	; 异常处理程序

extern	int_request_table	; 中断处理程序表
extern	syscall_table	    ; 系统调用表

; 导入全局变量
extern  display_position	; 显示位置（不同于光标哦）
extern  gdt_ptr;			; GDT指针
extern  idt_ptr;			; IDT指针
extern	curr_proc		    ; 当前系统运行的进程
extern	tss					; 任务状态段
extern	kernel_reenter		; 中断重入标志

;================================================================================================
; 32 位数据段
bits 32
[SECTION .data]
	nop

;================================================================================================
; 32 位栈段
[SECTION .bss]
StackSpace      resb    2 * 1024
StackTop:       ; 栈顶

;================================================================================================
; 32 位代码段
[SECTION .text]     ; 程序代码在此

; 导出函数和符号
global  _start      ; 导出_start程序开始处，这样链接器才能识别

global  restart				; 函数:进程重新启动
global 	flyanxSysCall		; 系统调用

; 以下是异常中断处理程序
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error

; 以下是中断处理程序，一共16个可用(主和从)
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15

_start:

    ; 此时内存看上去是这样的（更详细的内存情况在 LOADER.ASM 中有说明）：
	;              ┃                              ┃
	;              ┃                 ...         ┃
	;              ┣━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■Page  Tables■■■■■■	　┃
	;              ┃■■■■■(大小由LOADER决定)■■■■	　　┃ PageTblBase
	;    00101000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■Page Directory Table■■■■┃ PageDirBase = 1M
	;    00100000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□ Hardware  Reserved □□□□┃ B8000h ← gs
	;       9FC00h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■LOADER.BIN■■■■■■┃ somewhere in LOADER ← esp
	;       90000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■KERNEL.BIN■■■■■■┃
	;       80000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■KERNEL■■■■■■■┃ 30400h ← KERNEL 入口 (KernelEntryPointPhyAddr)
	;       30000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┋                 ...          ┋
	;              ┋                              ┋
	;           0h ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
	;
	;
	; GDT 以及相应的描述符是这样的：
	;
	;		              Descriptors               Selectors
	;              ┏━━━━━━━━━━━━━━━━━━┓
	;              ┃         Dummy Descriptor     ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_C   (0～4G)┃   8h = cs
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_RW  (0～4G)┃  10h = ds, es, fs, ss
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_VIDEO           ┃  1Bh = gs
	;              ┗━━━━━━━━━━━━━━━━━━┛
	;
	; 注意! 在使用 C 代码的时候一定要保证 ds, es, ss 这几个段寄存器的值是一样的
	; 因为编译器有可能编译出使用它们的代码, 而编译器默认它们是一样的. 比如串拷贝操作会用到 ds 和 es.
	;
	;

    ; 把　esp 从　LOADER 挪到 KERNEL　处
	mov esp, StackTop       ; 堆栈在 bss 段中
    
    mov dword [display_position], (160 * 13 + 2 * 0)  ; 显示位置从 第 13 行 第 0 列 开始
    
    sgdt    [gdt_ptr]    ; cstart() 中将会用到 gdt_ptr
    call    cstart      ; 在此函数中改变了gdt_ptr，让它指向新的GDT
    lgdt    [gdt_ptr]    ; 使用新的GDT

    lidt    [idt_ptr]	; 加载idtr

    jmp SELECTOR_KERNEL_CS:csinit


csinit:         ; “这个跳转指令强制使用刚刚初始化的结构”——<<OS:D&I 2nd>> P90.
    xor eax, eax
    mov ax, SELECTOR_TSS
    ltr ax

	jmp 0x40:0		; 手动触发一个General Protection异常,调试异常机制
	;ud2				; 手动触发一个Undefined Opcode异常,调试异常机制

	; 跳入c语言编写的内核主函数，以后我们的工作将主要在c语言下开发
	; 这里,我们又迎来一个质的飞跃,汇编虽然好,只是不够骚
    jmp main

;================================================================================================
; 中断和异常 之 硬件中断
%macro	hwint_master 1	; 主中断处理模板
	call save				; 调用保存函数,保存当前正在运行的进程的上下文
		
	in al, INT_M_CTLMASK	; @
	or al, (1 << %1)		; #-> 屏蔽当前中断
	out INT_M_CTLMASK, al	; @

	mov al, EOI				; @
	out INT_M_CTL, al		; #-> 置 EOI 位
	sti						; #	  CPU 在响应中断时会自动关闭中断，这句之后就允许响应别的中断
							; @	  但因为上边已经屏蔽了当前中断，所以当前中断再次发生将不会响应
	push %1
	call [int_request_table + 4 * %1]	; 执行中断处理程序
	pop ecx
	cli
	
	in al, INT_M_CTLMASK		; @
	and al, ~(1 << %1)			; #-> 再次允许发生当前中断，因为上边已经执行完中断处理程序了嘛～
	out INT_M_CTLMASK, al		; @
	ret
%endmacro

ALIGN	16
hwint00:		; Interrupt routine for irq 0 (the clock)，时钟中断
	hwint_master	0

ALIGN	16
hwint01:		; Interrupt routine for irq 1 (keyboard)，键盘中断
	hwint_master	1

ALIGN	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
	hwint_master	2

ALIGN	16
hwint03:		; Interrupt routine for irq 3 (second serial)
	hwint_master	3

ALIGN	16
hwint04:		; Interrupt routine for irq 4 (first serial)
	hwint_master	4

ALIGN	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwint_master	5

ALIGN	16
hwint06:		; Interrupt routine for irq 6 (floppy)，软盘中断
	hwint_master	6

ALIGN	16
hwint07:		; Interrupt routine for irq 7 (printer)，打印机中断
	hwint_master	7

; ---------------------------------
%macro	hwint_slave	1		; 从中断处理模板
	push	%1
	call	spurious_irq
	add	esp, 4
%endmacro
; ---------------------------------

ALIGN	16
hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwint_slave	8

ALIGN	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
	hwint_slave	9

ALIGN	16
hwint10:		; Interrupt routine for irq 10
	hwint_slave	10

ALIGN	16
hwint11:		; Interrupt routine for irq 11
	hwint_slave	11

ALIGN	16
hwint12:		; Interrupt routine for irq 12
	hwint_slave	12

ALIGN	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwint_slave	13

ALIGN	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave	14

ALIGN	16
hwint15:		; Interrupt routine for irq 15
	hwint_slave	15
;================================================================================================
; 中断和异常 之 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt
;================================================================================================
; save : 执行中断或切换程序时，执行 save 保存 CPU 当前状态，保证之前的程序上下文环境
save:
	pushad		; @
	push ds		; #
	push es		; #-> 保存原CPU寄存器状态
	push fs		; #
	push gs		; @
	mov dx, ss
	mov ds, dx
	mov es, dx

	mov esi, esp	; esi = 进程表起始地址

	inc dword [kernel_reenter]		; 中断重入标志++
	cmp dword [kernel_reenter], 0	; if(kernel_reenter == 0){ 
	jne	.1							;
	mov esp, StackTop				;	mov esp, StackTop // 切换到内核栈
	push restart					;	push restart
	jmp [esi + RETADR - P_STACKBASE];	return;
.1:									; } else {	// 已经在内核栈,则不需要切换
	push restart_reenter			;	push restart_reenter
	jmp [esi + RETADR - P_STACKBASE];	return;
									; }
;================================================================================================
; flyanxSysCall : 系统调用
flyanxSysCall:
	call save
	push dword [curr_proc]
	sti			; 关中断

	push 	ecx
	push 	ebx
	call	[syscall_table + eax * 4]
	add esp, 4 * 3

	mov		[esi + EAXREG - P_STACKBASE], eax
	cli			; 开中断
	ret
;================================================================================================
; restart : 进程重新启动
restart:
	mov esp, [curr_proc]	; 离开内核栈，到正在运行的进程堆栈中
	lldt [esp + P_LDT_SEL]	; 每个进程有自己的 LDT，所以每次进程的切换都需要加载新的ldtr

	lea eax, [esp + P_STACKTOP]	; 得到仅此的栈顶地址
	mov dword [tss + TSS3_S_SP0], eax	
restart_reenter:
	dec dword [kernel_reenter]	
	pop gs 
	pop fs
	pop es
	pop ds
	popad
	add esp, 4
	iretd

;================================================================================================

