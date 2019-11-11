;================================================================================================
; 文件：kernel_lib.asm
; 作用：Flyanx系统内核内核库文件
; 作者： Flyan
; 联系QQ： 1341662010
;================================================================================================
; 导入和导出

; 导入头文件
%include "kernelConst.inc"
; 导入变量
extern display_position

;================================================================================================
; 库函数开始
[SECTION .lib]

; 导出库函数,有关函数的作用,可以查阅prototype.h文件;详细信息请往下查阅其函数代码
global  disp_str
global  disp_color_str
global  out_byte
global  in_byte
global  enable_irq
global  disable_irq
global  interrupt_lock
global  interrupt_unlock

;================================================================================================
;                  void disp_str(char * info);	显示一个黑底白字的字符串
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
;                  void out_byte(u16_t port, u8_t value) ;	向端口输出数据
out_byte:
    mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	nop	; 一点延迟
	nop
	ret

;================================================================================================
;                  f8 in_byte(u16_t port);	从端口拿取数据
in_byte:
    mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx
	nop	; 一点延迟
	nop
	ret

;================================================================================================
;                  void disable_irq(int intRequest);	关闭一个特定的中断
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
interrupt_lock:
    cli
	ret

;================================================================================================
;                  interrupt_unlock	解锁中断
interrupt_unlock:
    sti
    ret

;================================================================================================