

#ifndef NO_UNDERLINE
#	define	match_init	_match_init
#	define	longest_match	_longest_match
#endif

#define	scanend		ebx
#define	scanendw	bx
#define	chainlenwmask	edx
#define	curmatch	rsi
#define	curmatchd	esi
#define	windowbestlen	r8
#define	scanalign	r9
#define	scanalignd	r9d
#define	window		r10
#define	bestlen		r11
#define	bestlend	r11d
#define	scanstart	r12d
#define	scanstartw	r12w
#define scan		r13
#define nicematch	r14d
#define	limit		r15
#define	limitd		r15d
#define prev		rcx


#define	MAX_MATCH	(258)
#define	MIN_MATCH	(3)
#define	MIN_LOOKAHEAD	(MAX_MATCH + MIN_MATCH + 1)
#define	MAX_MATCH_8	((MAX_MATCH + 7) & ~7)


#define	LocalVarsSize	(112)
#define _chainlenwmask	( 8-LocalVarsSize)(%rsp)
#define _windowbestlen	(16-LocalVarsSize)(%rsp)
#define save_r14        (24-LocalVarsSize)(%rsp)
#define save_rsi        (32-LocalVarsSize)(%rsp)
#define save_rbx        (40-LocalVarsSize)(%rsp)
#define save_r12        (56-LocalVarsSize)(%rsp)
#define save_r13        (64-LocalVarsSize)(%rsp)
#define save_r15        (80-LocalVarsSize)(%rsp)


.globl	match_init, longest_match









#ifndef CURRENT_LINX_XCODE_MAC_X64_STRUCTURE
#define dsWSize		( 68)(%rdi)
#define dsWMask		( 76)(%rdi)
#define dsWindow	( 80)(%rdi)
#define dsPrev		( 96)(%rdi)
#define dsMatchLen	(144)(%rdi)
#define dsPrevMatch	(148)(%rdi)
#define dsStrStart	(156)(%rdi)
#define dsMatchStart	(160)(%rdi)
#define dsLookahead	(164)(%rdi)
#define dsPrevLen	(168)(%rdi)
#define dsMaxChainLen	(172)(%rdi)
#define dsGoodMatch	(188)(%rdi)
#define dsNiceMatch	(192)(%rdi)

#else 

#ifndef STRUCT_OFFSET
#	define STRUCT_OFFSET	(0)
#endif


#define dsWSize		( 56 + STRUCT_OFFSET)(%rdi)
#define dsWMask		( 64 + STRUCT_OFFSET)(%rdi)
#define dsWindow	( 72 + STRUCT_OFFSET)(%rdi)
#define dsPrev		( 88 + STRUCT_OFFSET)(%rdi)
#define dsMatchLen	(136 + STRUCT_OFFSET)(%rdi)
#define dsPrevMatch	(140 + STRUCT_OFFSET)(%rdi)
#define dsStrStart	(148 + STRUCT_OFFSET)(%rdi)
#define dsMatchStart	(152 + STRUCT_OFFSET)(%rdi)
#define dsLookahead	(156 + STRUCT_OFFSET)(%rdi)
#define dsPrevLen	(160 + STRUCT_OFFSET)(%rdi)
#define dsMaxChainLen	(164 + STRUCT_OFFSET)(%rdi)
#define dsGoodMatch	(180 + STRUCT_OFFSET)(%rdi)
#define dsNiceMatch	(184 + STRUCT_OFFSET)(%rdi)

#endif




.text



longest_match:

		mov     %rsi, save_rsi
		mov     %rbx, save_rbx
		mov	%r12, save_r12
		mov     %r13, save_r13
		mov     %r14, save_r14
		mov     %r15, save_r15







		movl	dsPrevLen, %eax
		movl	dsGoodMatch, %ebx
		cmpl	%ebx, %eax
		movl	dsWMask, %eax
		movl	dsMaxChainLen, %chainlenwmask
		jl	LastMatchGood
		shrl	$2, %chainlenwmask
LastMatchGood:






		decl	%chainlenwmask
		shll	$16, %chainlenwmask
		orl	%eax, %chainlenwmask



		movl	dsNiceMatch, %eax
		movl	dsLookahead, %ebx
		cmpl	%eax, %ebx
		jl	LookaheadLess
		movl	%eax, %ebx
LookaheadLess:	movl	%ebx, %nicematch



		mov	dsWindow, %window
		movl	dsStrStart, %limitd
		lea	(%limit, %window), %scan




		mov	%scan, %scanalign
		negl	%scanalignd
		andl	$3, %scanalignd




		movl	dsWSize, %eax
		subl	$MIN_LOOKAHEAD, %eax
		xorl	%ecx, %ecx
		subl	%eax, %limitd
		cmovng	%ecx, %limitd



		movl	dsPrevLen, %bestlend



		lea	(%window, %bestlen), %windowbestlen
		mov	%windowbestlen, _windowbestlen





		movzwl	(%scan), %scanstart
		movzwl	-1(%scan, %bestlen), %scanend
		mov	dsPrev, %prev



		movl	%chainlenwmask, _chainlenwmask
		jmp	LoopEntry

.balign 16


LookupLoop:
		andl	%chainlenwmask, %curmatchd
		movzwl	(%prev, %curmatch, 2), %curmatchd
		cmpl	%limitd, %curmatchd
		jbe	LeaveNow
		subl	$0x00010000, %chainlenwmask
		js	LeaveNow
LoopEntry:	cmpw	-1(%windowbestlen, %curmatch), %scanendw
		jne	LookupLoop
		cmpw	%scanstartw, (%window, %curmatch)
		jne	LookupLoop


		movl	%chainlenwmask, _chainlenwmask






		mov	$(-MAX_MATCH_8), %rdx
		lea	(%curmatch, %window), %windowbestlen
		lea	MAX_MATCH_8(%windowbestlen, %scanalign), %windowbestlen
		lea	MAX_MATCH_8(%scan, %scanalign), %prev


		prefetcht1	(%windowbestlen, %rdx)
		prefetcht1	(%prev, %rdx)



#undef USE_SSE

LoopCmps:
#ifdef USE_SSE

		movdqu	  (%windowbestlen, %rdx), %xmm1
		movdqu	  (%prev, %rdx), %xmm2
		pcmpeqb	%xmm2, %xmm1
		movdqu	16(%windowbestlen, %rdx), %xmm3
		movdqu	16(%prev, %rdx), %xmm4
		pcmpeqb	%xmm4, %xmm3
		movdqu	32(%windowbestlen, %rdx), %xmm5
		movdqu	32(%prev, %rdx), %xmm6
		pcmpeqb	%xmm6, %xmm5
		movdqu	48(%windowbestlen, %rdx), %xmm7
		movdqu	48(%prev, %rdx), %xmm8
		pcmpeqb	%xmm8, %xmm7


		pmovmskb %xmm1, %rax
		notw	%ax
		bsfw	%ax, %ax
		jnz	LeaveLoopCmps
		

		add	$8, %rdx
		jz LenMaximum
		add	$8, %rdx

		
		pmovmskb %xmm3, %rax
		notw	%ax
		bsfw	%ax, %ax
		jnz	LeaveLoopCmps
		
		
		add	$16, %rdx


		pmovmskb %xmm5, %rax
		notw	%ax
		bsfw	%ax, %ax
		jnz	LeaveLoopCmps
		
		add	$16, %rdx


		pmovmskb %xmm7, %rax
		notw	%ax
		bsfw	%ax, %ax
		jnz	LeaveLoopCmps
		
		add	$16, %rdx
		
		jmp	LoopCmps
LeaveLoopCmps:	add	%rax, %rdx
#else
		mov	(%windowbestlen, %rdx), %rax
		xor	(%prev, %rdx), %rax
		jnz	LeaveLoopCmps
		
		mov	8(%windowbestlen, %rdx), %rax
		xor	8(%prev, %rdx), %rax
		jnz	LeaveLoopCmps8

		mov	16(%windowbestlen, %rdx), %rax
		xor	16(%prev, %rdx), %rax
		jnz	LeaveLoopCmps16
				
		add	$24, %rdx
		jnz	LoopCmps
		jmp	LenMaximum
#	if 0


LeaveLoopCmps:	bsf	%rax, %rax
		shrl	$3, %eax
		add	%rax, %rdx
#	else
LeaveLoopCmps16:
		add	$8, %rdx
LeaveLoopCmps8:
		add	$8, %rdx
LeaveLoopCmps:	testl   $0xFFFFFFFF, %eax
		jnz     Check16
		add     $4, %rdx
		shr     $32, %rax
Check16:        testw   $0xFFFF, %ax
		jnz     LenLower
		add	$2, %rdx
		shrl	$16, %eax
LenLower:	subb	$1, %al
		adc	$0, %rdx
#	endif
#endif




		lea	(%prev, %rdx), %rax
		sub	%scan, %rax
		cmpl	$MAX_MATCH, %eax
		jge	LenMaximum




		cmpl	%bestlend, %eax
		jg	LongerMatch
		mov	_windowbestlen, %windowbestlen
		mov	dsPrev, %prev
		movl	_chainlenwmask, %edx
		jmp	LookupLoop






LongerMatch:
		movl	%eax, %bestlend
		movl	%curmatchd, dsMatchStart
		cmpl	%nicematch, %eax
		jge	LeaveNow

		lea	(%window, %bestlen), %windowbestlen
		mov	%windowbestlen, _windowbestlen

		movzwl	-1(%scan, %rax), %scanend
		mov	dsPrev, %prev
		movl	_chainlenwmask, %chainlenwmask
		jmp	LookupLoop



LenMaximum:
		movl	$MAX_MATCH, %bestlend
		movl	%curmatchd, dsMatchStart




LeaveNow:
		movl	dsLookahead, %eax
		cmpl	%eax, %bestlend
		cmovngl	%bestlend, %eax
LookaheadRet:



	mov	save_rsi, %rsi
	mov	save_rbx, %rbx
	mov	save_r12, %r12
	mov	save_r13, %r13
	mov	save_r14, %r14
	mov	save_r15, %r15

	ret

match_init:	ret
