;================================================================================================
; 文件：kernel_386_lib.asm
; 作用：Flyanx系统内核内核库文件
; 作者： Flyan
; 联系QQ： 1341662010
;================================================================================================
; 导入和导出

; 导入头文件
%include "asmconst.inc"

; 导入变量
extern  display_position    ; 显示位置（不同于光标哦）
extern  level0_func         ; 导入提权函数
extern  video_size          ; 视频存储器大小
extern  video_mask          ; 视频存储器的大小索引
extern  blank_color         ; 空白显示颜色码

; 导入c语言函数
extern  simple_brk_point           ; 简单断点

;================================================================================================
; 库函数开始
[SECTION .lib]

; 导出库函数
global  phys_copy
global  disp_str
global  disp_color_str
global  out_byte
global  in_byte
global  out_word
global  in_word
global  enable_irq
global  disable_irq
global  interrupt_lock
global  interrupt_unlock
global	port_read
global	port_write
global  level0
global  reset

;*===========================================================================*
;*				phys_copy				     *
;*===========================================================================*
; PUBLIC void phys_copy(phys_bytes source, phys_bytes destination,
;			phys_bytes bytecount);
;* 将物理内存中任意处的一个数据块拷贝到任意的另外一处 *
;* 参数中的两个地址都是绝对地址，也就是地址0确实表示整个地址空间的第一个字节， *
;* 并且三个参数均为无符号长整数 *
PC_ARGS     equ     16    ; 用于到达复制的参数堆栈的栈顶
align 16
phys_copy:
    push esi
    push edi
    push es

    ; 获得所有参数
    mov esi, [esp + PC_ARGS]            ; source
    mov edi, [esp + PC_ARGS + 4]        ; destination
    mov ecx, [esp + PC_ARGS + 4 + 4]    ; bytecoun
    ; 注：因为得到的就是物理地址，所以esi和edi无需再转换，直接就表示一个真实的位置。
phys_copy_start:
    cmp ecx, 0              ; 判断bytecount
    jz phys_copy_end        ; if( bytecount == 0 ); jmp phys_copy_end

    mov al, [esi]
    inc esi

    mov byte [edi], al
    inc edi

    dec ecx                 ; bytecount--
    jmp phys_copy_start
phys_copy_end:
    pop es
    pop edi
    pop esi
    ret
;================================================================================================
;                  void disp_str(char * info);	显示一个黑底白字的字符串
align 16
disp_str:
    push ebp
    mov ebp, esp

    mov esi, [ebp + 8]          ; 得到字符串信息
    mov edi, [display_position]  ; 得到显示位置
    mov ah, 0Fh                 ; 设置显示属性：黑底白字
.1:
    lodsb
    test al, al
    jz  .2
    cmp al, 0Ah     ; 回车？
    jnz .3
    push eax
    mov eax, edi
    mov bl, 160
    div bl
    and eax, 0FFh
    inc eax
    mov bl, 160
    mul bl
    mov edi, eax
    pop eax
    jmp .1
.3:
    mov [gs:edi], ax    ; 将显示文字放到显存中
    add edi, 2
    jmp .1
.2:
    mov [display_position], edi  ; 更新显示位置

    pop ebp
    ret

;================================================================================================
;                  void disp_color_str(char * string, int color);	显示有颜色属性的字符串
align 16
disp_color_str:
    push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [display_position]
	mov	ah, [ebp + 12]	; color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[display_position], edi

	pop	ebp
	ret

;================================================================================================
;                  void out_byte(port_t port, U8_t value) ;	向端口输出数据
align 16
out_byte:
    mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	nop	; 一点延迟
	nop
	ret

;================================================================================================
;                  PUBLIC U8_t in_byte(port_t port);	从端口拿取数据
align 16
in_byte:
    mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx
	nop	; 一点延迟
	nop
	ret

;*===========================================================================*
;*				out_word				     *
;*===========================================================================*
;* PUBLIC void out_word(Port_t port, U16_t value);
;* 写一个字到某个i/o端口上 *
align 16
out_word:
    mov edx, [esp + 4]      ; 得到端口
    mov eax, [esp + 4 + 4]  ; 得到值
    out dx, ax              ; 输出一个字
    nop	; 一点延迟
    nop
    ret

;*===========================================================================*
;*				in_word					     *
;*===========================================================================*
; PUBLIC U16_t in_word(port_t port);
;* 读一个字从某个i/o端口并返回它 *
align 16
in_word:
    mov edx, [esp + 4]      ; 端口
    xor eax, eax
    in ax, dx               ; 读一个字
    nop	; 一点延迟
    nop
    ret

;================================================================================================
;                  void disable_irq(int intRequest);	关闭一个特定的中断
align 16
disable_irq:
    mov ecx, [esp + 4]		; get param --> intRequest
	pushf
	cli
	mov ah, 1
	rol ah, cl				; ah = (1 << (intRequest % 8))
	cmp cl, 8
	jae disable_8			; disable intRequest >= 8 at the slave 8259
disable_0:
	in al, INT_M_CTLMASK
	test al, ah
	jnz dis_already			; already disabled?
	or al, ah
	out INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov eax, 1				; disabled by this function
	ret
disable_8:
	in al, INT_S_CTLMASK
	test al, ah
	jnz dis_already			; already disabled?
	or al, ah
	out INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov eax, 1				; disable by this function
	ret
dis_already:
	popf
	xor eax, eax			; already disabled
	ret

;================================================================================================
;                  void enable_irq(int intRequest);	启用一个特定的中断
align 16
enable_irq:
    mov ecx, [esp + 4]		; get intRequest
	pushf
	cli
	mov ah, ~1
	rol ah, cl				; ah = ~(1 << (intRequest % 8))
	cmp cl, 8
	jae enable_8			; enable intRequest >= 8 at the slave 8259
enable_0:
	in al, INT_M_CTLMASK
	and al, ah
	out INT_M_CTLMASK, al	; clear bit at master 8259
	popf
	ret
enable_8:
	in al, INT_S_CTLMASK
	and al, ah
	out INT_S_CTLMASK, al	; clear bit at slave 8259
	popf
	ret

;================================================================================================
;                  interrupt_lock	锁中断
align 16
interrupt_lock:
    cli
	ret

;================================================================================================
;                  interrupt_unlock	解锁中断
align 16
interrupt_unlock:
    sti
    ret

;================================================================================================
;            void port_read(u16_t port, phys_bytes destination, unsigned bytcount);
port_read:
    mov edx, [esp + 4]          ; port
    mov edi, [esp + 4 + 4]      ; destination
    mov ecx, [esp + 4 + 4 + 4]  ; bytcount
    shr ecx, 1
    cld
    rep insw
    ret
;================================================================================================
;             void port_write(unsigned port, phys_bytes source, unsigned bytcount);
port_write:
    mov edx, [esp + 4]          ; port
    mov esi, [esp + 4 + 4]      ; source
    mov ecx, [esp + 4 + 4 + 4]  ; bytcount
    shr ecx, 1
    cld
    rep outsw
    ret
;================================================================================================
;                  void level0(void (*func)(void))	解锁中断
; 将任务提权到最高特权级 - 0级
; 它主要用于这样的操作，如复位CPU、或访问PC的ROM BIOS例程。
align 16
level0:
    mov eax, [esp + 4]
    mov [level0_func], eax
    int LEVEL0_VECTOR
    ret
;================================================================================================
;                   PUBLIC void reset(void);
;*                       使处理器复位             *
;* 这通过将一个空指针装入处理器的中断描述符表寄存器，然后执行一个软件中断实现， *
;* 它与硬件复位效果是一样的。这个例程一般在重启的时候使用。 *
reset:
    lidt    [idt_zero]
    int     3           ; 386保护模式不会想看到这个中断的，所以目的达到了
[SECTION .data]
idt_zero:   dd  0, 0
[SECTION .text]


;================================================================================================