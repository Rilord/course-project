; inffasx64.asm is a hand tuned assembler version of inffast.c - fast decoding
; version for AMD64 on Windows using Microsoft C compiler
;
; inffasx64.asm is automatically convert from AMD64 portion of inffas86.c
; inffasx64.asm is called by inffas8664.c, which contain more info.


; to compile this file, I use option
;   ml64.exe /Flinffasx64 /c /Zi inffasx64.asm
;   with Microsoft Macro Assembler (x64) for AMD64
;

; This file compile with Microsoft Macro Assembler (x64) for AMD64
;
;   ml64.exe is given with Visual Studio 2005/2008/2010 and Windows WDK
;
;   (you can get Windows WDK with ml64 for AMD64 from
;      http://www.microsoft.com/whdc/Devtools/wdk/default.mspx for low price)
;


.code
inffas8664fnc PROC

; see http://weblogs.asp.net/oldnewthing/archive/2004/01/14/58579.aspx and
; http://msdn.microsoft.com/library/en-us/kmarch/hh/kmarch/64bitAMD_8e951dd2-ee77-4728-8702-55ce4b5dd24a.xml.asp
;
; All registers must be preserved across the call, except for
;   rax, rcx, rdx, r8, r-9, r10, and r11, which are scratch.


	mov [rsp-8],rsi
	mov [rsp-16],rdi
	mov [rsp-24],r12
	mov [rsp-32],r13
	mov [rsp-40],r14
	mov [rsp-48],r15
	mov [rsp-56],rbx

	mov rax,rcx

	mov	[rax+8], rbp       ;
	mov	[rax], rsp

	mov	rsp, rax          ;

	mov	rsi, [rsp+16]      ;
	mov	rdi, [rsp+32]      ;
	mov	r9, [rsp+24]       ;
	mov	r10, [rsp+48]      ;
	mov	rbp, [rsp+64]      ;
	mov	r11, [rsp+72]      ;
	mov	rdx, [rsp+80]      ;
	mov	ebx, [rsp+88]      ;
	mov	r12d, [rsp+100]    ;
	mov	r13d, [rsp+104]    ;
                                          ;
                                          ;


	cld
	cmp	r10, rdi
	je	L_one_time           ;
	cmp	r9, rsi

    jne L_do_loop


L_one_time:
	mov	r8, r12           ;
	cmp	bl, 32
	ja	L_get_length_code_one_time

	lodsd                         ;
	mov	cl, bl            ;
	add	bl, 32             ;
	shl	rax, cl
	or	rdx, rax          ;
	jmp	L_get_length_code_one_time

ALIGN 4
L_while_test:
	cmp	r10, rdi
	jbe	L_break_loop
	cmp	r9, rsi
	jbe	L_break_loop

L_do_loop:
	mov	r8, r12           ;
	cmp	bl, 32
	ja	L_get_length_code    ;

	lodsd                         ;
	mov	cl, bl            ;
	add	bl, 32             ;
	shl	rax, cl
	or	rdx, rax          ;

L_get_length_code:
	and	r8, rdx            ;
	mov	eax, [rbp+r8*4]  ;

	mov	cl, ah            ;
	sub	bl, ah            ;
	shr	rdx, cl           ;

	test	al, al
	jnz	L_test_for_length_base ;

	mov	r8, r12            ;
	shr	eax, 16            ;
	stosb

L_get_length_code_one_time:
	and	r8, rdx            ;
	mov	eax, [rbp+r8*4] ;

L_dolen:
	mov	cl, ah            ;
	sub	bl, ah            ;
	shr	rdx, cl           ;

	test	al, al
	jnz	L_test_for_length_base ;

	shr	eax, 16            ;
	stosb
	jmp	L_while_test

ALIGN 4
L_test_for_length_base:
	mov	r14d, eax         ;
	shr	r14d, 16           ;
	mov	cl, al

	test	al, 16
	jz	L_test_for_second_level_length ;
	and	cl, 15             ;
	jz	L_decode_distance    ;

L_add_bits_to_len:
	sub	bl, cl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx          ;
	shr	rdx, cl
	add	r14d, eax         ;

L_decode_distance:
	mov	r8, r13           ;
	cmp	bl, 32
	ja	L_get_distance_code  ;

	lodsd                         ;
	mov	cl, bl            ;
	add	bl, 32             ;
	shl	rax, cl
	or	rdx, rax          ;

L_get_distance_code:
	and	r8, rdx           ;
	mov	eax, [r11+r8*4] ;

L_dodist:
	mov	r15d, eax         ;
	shr	r15d, 16           ;
	mov	cl, ah
	sub	bl, ah            ;
	shr	rdx, cl           ;
	mov	cl, al            ;

	test	al, 16             ;
	jz	L_test_for_second_level_dist
	and	cl, 15             ;
	jz	L_check_dist_one

L_add_bits_to_dist:
	sub	bl, cl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax                 ;
	and	eax, edx          ;
	shr	rdx, cl
	add	r15d, eax         ;

L_check_window:
	mov	r8, rsi           ;
	mov	rax, rdi
	sub	rax, [rsp+40]      ;

	cmp	eax, r15d
	jb	L_clip_window        ;

	mov	ecx, r14d         ;
	mov	rsi, rdi
	sub	rsi, r15          ;

	sar	ecx, 1
	jnc	L_copy_two           ;

	rep     movsw
	mov	al, [rsi]
	mov	[rdi], al
	inc	rdi

	mov	rsi, r8           ;
	jmp	L_while_test

L_copy_two:
	rep     movsw
	mov	rsi, r8           ;
	jmp	L_while_test

ALIGN 4
L_check_dist_one:
	cmp	r15d, 1            ;
	jne	L_check_window
	cmp	[rsp+40], rdi      ;
	je	L_check_window

	mov	ecx, r14d         ;
	mov	al, [rdi-1]
	mov	ah, al

	sar	ecx, 1
	jnc	L_set_two
	mov	[rdi], al
	inc	rdi

L_set_two:
	rep     stosw
	jmp	L_while_test

ALIGN 4
L_test_for_second_level_length:
	test	al, 64
	jnz	L_test_for_end_of_block ;

	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx         ;
	add	eax, r14d        ;
	mov	eax, [rbp+rax*4] ;
	jmp	L_dolen

ALIGN 4
L_test_for_second_level_dist:
	test	al, 64
	jnz	L_invalid_distance_code ;

	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx         ;
	add	eax, r15d        ;
	mov	eax, [r11+rax*4] ;
	jmp	L_dodist

ALIGN 4
L_clip_window:
	mov	ecx, eax         ;
	mov	eax, [rsp+92]     ;
	neg	ecx                ;

	cmp	eax, r15d
	jb	L_invalid_distance_too_far ;

	add	ecx, r15d         ;
	cmp	dword ptr [rsp+96], 0
	jne	L_wrap_around_window ;

	mov	rsi, [rsp+56]     ;
	sub	eax, ecx         ;
	add	rsi, rax         ;

	mov	eax, r14d        ;
	cmp	r14d, ecx
	jbe	L_do_copy           ;

	sub	eax, ecx         ;
	rep     movsb
	mov	rsi, rdi
	sub	rsi, r15         ;
	jmp	L_do_copy

ALIGN 4
L_wrap_around_window:
	mov	eax, [rsp+96]     ;
	cmp	ecx, eax
	jbe	L_contiguous_in_window ;

	mov	esi, [rsp+92]     ;
	add	rsi, [rsp+56]     ;
	add	rsi, rax         ;
	sub	rsi, rcx         ;
	sub	ecx, eax         ;

	mov	eax, r14d        ;
	cmp	eax, ecx
	jbe	L_do_copy           ;

	sub	eax, ecx         ;
	rep     movsb
	mov	rsi, [rsp+56]     ;
	mov	ecx, [rsp+96]     ;
	cmp	eax, ecx
	jbe	L_do_copy           ;

	sub	eax, ecx         ;
	rep     movsb
	mov	rsi, rdi
	sub	rsi, r15         ;
	jmp	L_do_copy

ALIGN 4
L_contiguous_in_window:
	mov	rsi, [rsp+56]     ;
	add	rsi, rax
	sub	rsi, rcx         ;

	mov	eax, r14d        ;
	cmp	eax, ecx
	jbe	L_do_copy           ;

	sub	eax, ecx         ;
	rep     movsb
	mov	rsi, rdi
	sub	rsi, r15         ;
	jmp	L_do_copy           ;

ALIGN 4
L_do_copy:
	mov	ecx, eax         ;
	rep     movsb

	mov	rsi, r8          ;
	jmp	L_while_test

L_test_for_end_of_block:
	test	al, 32
	jz	L_invalid_literal_length_code
	mov	dword ptr [rsp+116], 1
	jmp	L_break_loop_with_status

L_invalid_literal_length_code:
	mov	dword ptr [rsp+116], 2
	jmp	L_break_loop_with_status

L_invalid_distance_code:
	mov	dword ptr [rsp+116], 3
	jmp	L_break_loop_with_status

L_invalid_distance_too_far:
	mov	dword ptr [rsp+116], 4
	jmp	L_break_loop_with_status

L_break_loop:
	mov	dword ptr [rsp+116], 0

L_break_loop_with_status:
;
	mov	[rsp+16], rsi     ;
	mov	[rsp+32], rdi     ;
	mov	[rsp+88], ebx     ;
	mov	[rsp+80], rdx     ;

	mov	rax, [rsp]       ;
	mov	rbp, [rsp+8]
	mov	rsp, rax



	mov rsi,[rsp-8]
	mov rdi,[rsp-16]
	mov r12,[rsp-24]
	mov r13,[rsp-32]
	mov r14,[rsp-40]
	mov r15,[rsp-48]
	mov rbx,[rsp-56]

    ret 0
;          :
;          : "m" (ar)
;          : "memory", "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi",
;            "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
;    );

inffas8664fnc 	ENDP
;_TEXT	ENDS
END
