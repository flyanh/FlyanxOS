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

;================================================================================================
; 库函数开始
[SECTION .lib]

; 导出库函数,有关函数的作用,可以查阅prototype.h文件;详细信息请往下查阅其函数代码
global  copy_msg
global  disp_str
global  disp_color_str
global  out_byte
global  in_byte
global  enable_irq
global  disable_irq
global  interrupt_lock
global  interrupt_unlock
global  level0

;*===========================================================================*
;*				copy_msg					     *
;*              复制消息
;*===========================================================================*
;* 尽管用phys_copy就可以完成消息的拷贝（在下面），然而这是一个更快的专门过程
;* copy_msg被用作消息拷贝的目的
;* 函数原型：copy_msg (int src,phys_clicks src_clicks,vir_bytes src_offset,
;*                 		phys_clicks dst_clicks, vir_bytes dst_offset );
;* 各参数意义：
;* source: 发送者的进程号，它将被拷贝到接收者缓冲区的m_source域
;* src_clicks: 源数据的段基地址
;* dst_clicks: 目的地的段基址
;* src_offset: 源数据的偏移
;* dst_offset: 目的地的偏移
;* 使用块和偏移指定源和目标的方式比phys_copy所用的32位物理地址更加的高效。

; CM_ARGS db =
copy_msg:


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
;                  PUBLIC unsigned in_byte(port_t port);	从端口拿取数据
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
;                  void level0(void (*func)(void))	解锁中断
; 将任务提权到最高特权级 - 0级
; 它主要用于这样的操作，如复位CPU、或访问PC的ROM BIOS例程。
level0:
    mov eax, [esp + 4]
    mov [level0_func], eax
    int LEVEL0_VECTOR
    ret

;================================================================================================

