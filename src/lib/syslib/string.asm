;================================================================================================
; 文件：string.asm
; 作用：Flyanx系统内核串库文件
; 作者： Flyan
; 联系QQ： 1341662010
;================================================================================================
; 库函数开始
[SECTION .lib]

; 导出库函数
global	memcpy
global	memset
global  strcpy
global	strlen
;================================================================================================
; void* memcpy(void* es:p_dst, void* ds:p_src, int size);
memcpy:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp + 8]  ; edi -> 目的地地址
    mov esi, [ebp + 12]	; esi -> 源地址（要复制的数据首地址）
	mov ecx, [ebp + 16]	; ecx <- 计数器（复制的数据大小）
.1:
	cmp ecx, 0			; 判断计数器
	jz	.2				; if(计算器 == 0); jmp .2

	mov al, [ds:esi]		; @
	inc esi					; #
							; #-> 逐字节移动并复制
	mov byte [es:edi], al	; #
	inc edi					; @

	dec ecx				; 计数器--
	jmp	.1				; 循环
.2:
	mov eax, [ebp + 8]	; 设置返回值

	pop ecx
	pop edi
	pop esi
	mov esp, ebp
	pop ebp

	ret
;================================================================================================
; void memset(void* p_dst, char ch, int size);
memset:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp + 8]  ; edi -> 目的地地址
    mov edx, [ebp + 12]	; esi -> 要初始化的字符
	mov ecx, [ebp + 16]	; ecx <- 计数器（复制的数据大小）
.1:
    cmp ecx, 0  ; 判断计数器
    jz  .2      ; 计数器为零跳出

    mov byte [edi], dl
    inc edi

    dec ecx     ; 计数器==
    jmp .1      ; 循环
.2:
    pop ecx
	pop edi
	pop esi
	mov esp, ebp
	pop ebp

	ret
;================================================================================================
; char* strcpy(char* p_dst, char* p_src);
strcpy:
    push ebp
	mov ebp, esp

    mov edi, [ebp + 8]			; edi -> 目的地地址
	mov esi, [ebp + 12]			; esi -> 源地址（要复制的数据首地址）
.1:
	mov al, [esi]				; @
	inc esi						; #
								; #-> 逐字节移动
	mov byte [edi], al			; #
	inc edi						; @

	cmp al, 0					; 是否遇到 '\0'
	jnz .1						; 没遇到就进行循环，遇到说明移动完成

	mov eax, [ebp + 8]			; 返回值
	
	pop ebp
	ret
;================================================================================================
; unsigned int strlen(char* p_str);
strlen:
    push    ebp
    mov     ebp, esp

    mov     eax, 0                  ; 字符串长度开始是 0
    mov     esi, [ebp + 8]          ; esi 指向首地址
.1:
    cmp     byte [esi], 0           ; 看 esi 指向的字符是否是 '\0'
    jz      .2                      ; 如果是 '\0'，程序结束
    inc     esi                     ; 如果不是 '\0'，esi 指向下一个字符
    inc     eax                     ;         并且，eax 自加一
    jmp     .1                      ; 如此循环
.2:
    pop     ebp
    ret                             ; 函数结束，返回
;================================================================================================
