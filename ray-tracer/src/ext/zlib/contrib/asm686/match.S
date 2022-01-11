

#ifndef NO_UNDERLINE
#define	match_init	_match_init
#define	longest_match	_longest_match
#endif

#define	MAX_MATCH	(258)
#define	MIN_MATCH	(3)
#define	MIN_LOOKAHEAD	(MAX_MATCH + MIN_MATCH + 1)
#define	MAX_MATCH_8	((MAX_MATCH + 7) & ~7)



#define	chainlenwmask		0

#define	window			4
#define	windowbestlen		8
#define	scanstart		16
#define	scanend			12
#define	scanalign		20
#define	nicematch		24
#define	bestlen			28
#define	scan			32

#define	LocalVarsSize		(36)





#define	deflatestate		56
#define	curmatch		60



#define zlib1222add		(8)

#define	dsWSize			(36+zlib1222add)
#define	dsWMask			(44+zlib1222add)
#define	dsWindow		(48+zlib1222add)
#define	dsPrev			(56+zlib1222add)
#define	dsMatchLen		(88+zlib1222add)
#define	dsPrevMatch		(92+zlib1222add)
#define	dsStrStart		(100+zlib1222add)
#define	dsMatchStart		(104+zlib1222add)
#define	dsLookahead		(108+zlib1222add)
#define	dsPrevLen		(112+zlib1222add)
#define	dsMaxChainLen		(116+zlib1222add)
#define	dsGoodMatch		(132+zlib1222add)
#define	dsNiceMatch		(136+zlib1222add)


.file "match.S"

.globl	match_init, longest_match

.text


.cfi_sections	.debug_frame

longest_match:

.cfi_startproc



		pushl	%ebp
		.cfi_def_cfa_offset 8
		.cfi_offset ebp, -8
		pushl	%edi
		.cfi_def_cfa_offset 12
		pushl	%esi
		.cfi_def_cfa_offset 16
		pushl	%ebx
		.cfi_def_cfa_offset 20
		subl	$LocalVarsSize, %esp
		.cfi_def_cfa_offset LocalVarsSize+20






		movl	deflatestate(%esp), %edx
		movl	curmatch(%esp), %ecx






 
		movl	dsPrevLen(%edx), %eax
		movl	dsGoodMatch(%edx), %ebx
		cmpl	%ebx, %eax
		movl	dsWMask(%edx), %eax
		movl	dsMaxChainLen(%edx), %ebx
		jl	LastMatchGood
		shrl	$2, %ebx
LastMatchGood:






		decl	%ebx
		shll	$16, %ebx
		orl	%eax, %ebx
		movl	%ebx, chainlenwmask(%esp)



		movl	dsNiceMatch(%edx), %eax
		movl	dsLookahead(%edx), %ebx
		cmpl	%eax, %ebx
		jl	LookaheadLess
		movl	%eax, %ebx
LookaheadLess:	movl	%ebx, nicematch(%esp)



		movl	dsWindow(%edx), %esi
		movl	%esi, window(%esp)
		movl	dsStrStart(%edx), %ebp
		lea	(%esi,%ebp), %edi
		movl	%edi, scan(%esp)




		movl	%edi, %eax
		negl	%eax
		andl	$3, %eax
		movl	%eax, scanalign(%esp)




		movl	dsWSize(%edx), %eax
		subl	$MIN_LOOKAHEAD, %eax
		subl	%eax, %ebp
		jg	LimitPositive
		xorl	%ebp, %ebp
LimitPositive:



		movl	dsPrevLen(%edx), %eax
		movl	%eax, bestlen(%esp)



		addl	%eax, %esi
		movl	%esi, windowbestlen(%esp)





		movzwl	(%edi), %ebx
		movl	%ebx, scanstart(%esp)
		movzwl	-1(%edi,%eax), %ebx
		movl	%ebx, scanend(%esp)
		movl	dsPrev(%edx), %edi



		movl	chainlenwmask(%esp), %edx
		jmp	LoopEntry

.balign 16


LookupLoop:
		andl	%edx, %ecx
		movzwl	(%edi,%ecx,2), %ecx
		cmpl	%ebp, %ecx
		jbe	LeaveNow
		subl	$0x00010000, %edx
		js	LeaveNow
LoopEntry:	movzwl	-1(%esi,%ecx), %eax
		cmpl	%ebx, %eax
		jnz	LookupLoop
		movl	window(%esp), %eax
		movzwl	(%eax,%ecx), %eax
		cmpl	scanstart(%esp), %eax
		jnz	LookupLoop



		movl	%edx, chainlenwmask(%esp)






		movl	window(%esp), %esi
		movl	scan(%esp), %edi
		addl	%ecx, %esi
		movl	scanalign(%esp), %eax
		movl	$(-MAX_MATCH_8), %edx
		lea	MAX_MATCH_8(%edi,%eax), %edi
		lea	MAX_MATCH_8(%esi,%eax), %esi


LoopCmps:
		movl	(%esi,%edx), %eax
		xorl	(%edi,%edx), %eax
		jnz	LeaveLoopCmps
		movl	4(%esi,%edx), %eax
		xorl	4(%edi,%edx), %eax
		jnz	LeaveLoopCmps4
		addl	$8, %edx
		jnz	LoopCmps
		jmp	LenMaximum
LeaveLoopCmps4:	addl	$4, %edx
LeaveLoopCmps:	testl	$0x0000FFFF, %eax
		jnz	LenLower
		addl	$2, %edx
		shrl	$16, %eax
LenLower:	subb	$1, %al
		adcl	$0, %edx




		lea	(%edi,%edx), %eax
		movl	scan(%esp), %edi
		subl	%edi, %eax
		cmpl	$MAX_MATCH, %eax
		jge	LenMaximum




		movl	deflatestate(%esp), %edx
		movl	bestlen(%esp), %ebx
		cmpl	%ebx, %eax
		jg	LongerMatch
		movl	windowbestlen(%esp), %esi
		movl	dsPrev(%edx), %edi
		movl	scanend(%esp), %ebx
		movl	chainlenwmask(%esp), %edx
		jmp	LookupLoop






LongerMatch:	movl	nicematch(%esp), %ebx
		movl	%eax, bestlen(%esp)
		movl	%ecx, dsMatchStart(%edx)
		cmpl	%ebx, %eax
		jge	LeaveNow
		movl	window(%esp), %esi
		addl	%eax, %esi
		movl	%esi, windowbestlen(%esp)
		movzwl	-1(%edi,%eax), %ebx
		movl	dsPrev(%edx), %edi
		movl	%ebx, scanend(%esp)
		movl	chainlenwmask(%esp), %edx
		jmp	LookupLoop



LenMaximum:	movl	deflatestate(%esp), %edx
		movl	$MAX_MATCH, bestlen(%esp)
		movl	%ecx, dsMatchStart(%edx)




LeaveNow:
		movl	deflatestate(%esp), %edx
		movl	bestlen(%esp), %ebx
		movl	dsLookahead(%edx), %eax
		cmpl	%eax, %ebx
		jg	LookaheadRet
		movl	%ebx, %eax
LookaheadRet:



		addl	$LocalVarsSize, %esp
		.cfi_def_cfa_offset 20
		popl	%ebx
		.cfi_def_cfa_offset 16
		popl	%esi
		.cfi_def_cfa_offset 12
		popl	%edi
		.cfi_def_cfa_offset 8
		popl	%ebp
		.cfi_def_cfa_offset 4
.cfi_endproc
match_init:	ret
