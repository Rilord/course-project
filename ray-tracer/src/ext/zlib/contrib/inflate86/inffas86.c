

#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"




void inflate_fast(strm, start)
z_streamp strm;
unsigned start;
{
    struct inflate_state FAR *state;
    struct inffast_ar {


 void *esp;
 void *ebp;
 unsigned char FAR *in;
 unsigned char FAR *last;
 unsigned char FAR *out;
 unsigned char FAR *beg;
 unsigned char FAR *end;
 unsigned char FAR *window;
 code const FAR *lcode;
 code const FAR *dcode;
 unsigned long hold;
 unsigned bits;
 unsigned wsize;
 unsigned write;
 unsigned lmask;
 unsigned dmask;
 unsigned len;
 unsigned dist;
 unsigned status;
    } ar;

#if defined( __GNUC__ ) && defined( __amd64__ ) && ! defined( __i386 )
#define PAD_AVAIL_IN 6
#define PAD_AVAIL_OUT 258
#else
#define PAD_AVAIL_IN 5
#define PAD_AVAIL_OUT 257
#endif


    state = (struct inflate_state FAR *)strm->state;
    ar.in = strm->next_in;
    ar.last = ar.in + (strm->avail_in - PAD_AVAIL_IN);
    ar.out = strm->next_out;
    ar.beg = ar.out - (start - strm->avail_out);
    ar.end = ar.out + (strm->avail_out - PAD_AVAIL_OUT);
    ar.wsize = state->wsize;
    ar.write = state->wnext;
    ar.window = state->window;
    ar.hold = state->hold;
    ar.bits = state->bits;
    ar.lcode = state->lencode;
    ar.dcode = state->distcode;
    ar.lmask = (1U << state->lenbits) - 1;
    ar.dmask = (1U << state->distbits) - 1;




    while (((unsigned long)(void *)ar.in & (sizeof(ar.hold) / 2 - 1)) != 0) {
        ar.hold += (unsigned long)*ar.in++ << ar.bits;
        ar.bits += 8;
    }

#if defined( __GNUC__ ) && defined( __amd64__ ) && ! defined( __i386 )
    __asm__ __volatile__ (
"        leaq    %0, %%rax\n"
"        movq    %%rbp, 8(%%rax)\n"
"        movq    %%rsp, (%%rax)\n"
"        movq    %%rax, %%rsp\n"
"        movq    16(%%rsp), %%rsi\n"
"        movq    32(%%rsp), %%rdi\n"
"        movq    24(%%rsp), %%r9\n"
"        movq    48(%%rsp), %%r10\n"
"        movq    64(%%rsp), %%rbp\n"
"        movq    72(%%rsp), %%r11\n"
"        movq    80(%%rsp), %%rdx\n"
"        movl    88(%%rsp), %%ebx\n"
"        movl    100(%%rsp), %%r12d\n"
"        movl    104(%%rsp), %%r13d\n"


"        cld\n"
"        cmpq    %%rdi, %%r10\n"
"        je      .L_one_time\n"
"        cmpq    %%rsi, %%r9\n"
"        je      .L_one_time\n"
"        jmp     .L_do_loop\n"

".L_one_time:\n"
"        movq    %%r12, %%r8\n"
"        cmpb    $32, %%bl\n"
"        ja      .L_get_length_code_one_time\n"

"        lodsl\n"
"        movb    %%bl, %%cl\n"
"        addb    $32, %%bl\n"
"        shlq    %%cl, %%rax\n"
"        orq     %%rax, %%rdx\n"
"        jmp     .L_get_length_code_one_time\n"

".align 32,0x90\n"
".L_while_test:\n"
"        cmpq    %%rdi, %%r10\n"
"        jbe     .L_break_loop\n"
"        cmpq    %%rsi, %%r9\n"
"        jbe     .L_break_loop\n"

".L_do_loop:\n"
"        movq    %%r12, %%r8\n"
"        cmpb    $32, %%bl\n"
"        ja      .L_get_length_code\n"

"        lodsl\n"
"        movb    %%bl, %%cl\n"
"        addb    $32, %%bl\n"
"        shlq    %%cl, %%rax\n"
"        orq     %%rax, %%rdx\n"

".L_get_length_code:\n"
"        andq    %%rdx, %%r8\n"
"        movl    (%%rbp,%%r8,4), %%eax\n"

"        movb    %%ah, %%cl\n"
"        subb    %%ah, %%bl\n"
"        shrq    %%cl, %%rdx\n"

"        testb   %%al, %%al\n"
"        jnz     .L_test_for_length_base\n"

"        movq    %%r12, %%r8\n"
"        shrl    $16, %%eax\n"
"        stosb\n"

".L_get_length_code_one_time:\n"
"        andq    %%rdx, %%r8\n"
"        movl    (%%rbp,%%r8,4), %%eax\n"

".L_dolen:\n"
"        movb    %%ah, %%cl\n"
"        subb    %%ah, %%bl\n"
"        shrq    %%cl, %%rdx\n"

"        testb   %%al, %%al\n"
"        jnz     .L_test_for_length_base\n"

"        shrl    $16, %%eax\n"
"        stosb\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_test_for_length_base:\n"
"        movl    %%eax, %%r14d\n"
"        shrl    $16, %%r14d\n"
"        movb    %%al, %%cl\n"

"        testb   $16, %%al\n"
"        jz      .L_test_for_second_level_length\n"
"        andb    $15, %%cl\n"
"        jz      .L_decode_distance\n"

".L_add_bits_to_len:\n"
"        subb    %%cl, %%bl\n"
"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        shrq    %%cl, %%rdx\n"
"        addl    %%eax, %%r14d\n"

".L_decode_distance:\n"
"        movq    %%r13, %%r8\n"
"        cmpb    $32, %%bl\n"
"        ja      .L_get_distance_code\n"

"        lodsl\n"
"        movb    %%bl, %%cl\n"
"        addb    $32, %%bl\n"
"        shlq    %%cl, %%rax\n"
"        orq     %%rax, %%rdx\n"

".L_get_distance_code:\n"
"        andq    %%rdx, %%r8\n"
"        movl    (%%r11,%%r8,4), %%eax\n"

".L_dodist:\n"
"        movl    %%eax, %%r15d\n"
"        shrl    $16, %%r15d\n"
"        movb    %%ah, %%cl\n"
"        subb    %%ah, %%bl\n"
"        shrq    %%cl, %%rdx\n"
"        movb    %%al, %%cl\n"

"        testb   $16, %%al\n"
"        jz      .L_test_for_second_level_dist\n"
"        andb    $15, %%cl\n"
"        jz      .L_check_dist_one\n"

".L_add_bits_to_dist:\n"
"        subb    %%cl, %%bl\n"
"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        shrq    %%cl, %%rdx\n"
"        addl    %%eax, %%r15d\n"

".L_check_window:\n"
"        movq    %%rsi, %%r8\n"
"        movq    %%rdi, %%rax\n"
"        subq    40(%%rsp), %%rax\n"

"        cmpl    %%r15d, %%eax\n"
"        jb      .L_clip_window\n"

"        movl    %%r14d, %%ecx\n"
"        movq    %%rdi, %%rsi\n"
"        subq    %%r15, %%rsi\n"

"        sarl    %%ecx\n"
"        jnc     .L_copy_two\n"

"        rep     movsw\n"
"        movb    (%%rsi), %%al\n"
"        movb    %%al, (%%rdi)\n"
"        incq    %%rdi\n"

"        movq    %%r8, %%rsi\n"
"        jmp     .L_while_test\n"

".L_copy_two:\n"
"        rep     movsw\n"
"        movq    %%r8, %%rsi\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_check_dist_one:\n"
"        cmpl    $1, %%r15d\n"
"        jne     .L_check_window\n"
"        cmpq    %%rdi, 40(%%rsp)\n"
"        je      .L_check_window\n"

"        movl    %%r14d, %%ecx\n"
"        movb    -1(%%rdi), %%al\n"
"        movb    %%al, %%ah\n"

"        sarl    %%ecx\n"
"        jnc     .L_set_two\n"
"        movb    %%al, (%%rdi)\n"
"        incq    %%rdi\n"

".L_set_two:\n"
"        rep     stosw\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_test_for_second_level_length:\n"
"        testb   $64, %%al\n"
"        jnz     .L_test_for_end_of_block\n"

"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        addl    %%r14d, %%eax\n"
"        movl    (%%rbp,%%rax,4), %%eax\n"
"        jmp     .L_dolen\n"

".align 32,0x90\n"
".L_test_for_second_level_dist:\n"
"        testb   $64, %%al\n"
"        jnz     .L_invalid_distance_code\n"

"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        addl    %%r15d, %%eax\n"
"        movl    (%%r11,%%rax,4), %%eax\n"
"        jmp     .L_dodist\n"

".align 32,0x90\n"
".L_clip_window:\n"
"        movl    %%eax, %%ecx\n"
"        movl    92(%%rsp), %%eax\n"
"        negl    %%ecx\n"

"        cmpl    %%r15d, %%eax\n"
"        jb      .L_invalid_distance_too_far\n"

"        addl    %%r15d, %%ecx\n"
"        cmpl    $0, 96(%%rsp)\n"
"        jne     .L_wrap_around_window\n"

"        movq    56(%%rsp), %%rsi\n"
"        subl    %%ecx, %%eax\n"
"        addq    %%rax, %%rsi\n"

"        movl    %%r14d, %%eax\n"
"        cmpl    %%ecx, %%r14d\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movq    %%rdi, %%rsi\n"
"        subq    %%r15, %%rsi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_wrap_around_window:\n"
"        movl    96(%%rsp), %%eax\n"
"        cmpl    %%eax, %%ecx\n"
"        jbe     .L_contiguous_in_window\n"

"        movl    92(%%rsp), %%esi\n"
"        addq    56(%%rsp), %%rsi\n"
"        addq    %%rax, %%rsi\n"
"        subq    %%rcx, %%rsi\n"
"        subl    %%eax, %%ecx\n"

"        movl    %%r14d, %%eax\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movq    56(%%rsp), %%rsi\n"
"        movl    96(%%rsp), %%ecx\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movq    %%rdi, %%rsi\n"
"        subq    %%r15, %%rsi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_contiguous_in_window:\n"
"        movq    56(%%rsp), %%rsi\n"
"        addq    %%rax, %%rsi\n"
"        subq    %%rcx, %%rsi\n"

"        movl    %%r14d, %%eax\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movq    %%rdi, %%rsi\n"
"        subq    %%r15, %%rsi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_do_copy:\n"
"        movl    %%eax, %%ecx\n"
"        rep     movsb\n"

"        movq    %%r8, %%rsi\n"
"        jmp     .L_while_test\n"

".L_test_for_end_of_block:\n"
"        testb   $32, %%al\n"
"        jz      .L_invalid_literal_length_code\n"
"        movl    $1, 116(%%rsp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_literal_length_code:\n"
"        movl    $2, 116(%%rsp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_distance_code:\n"
"        movl    $3, 116(%%rsp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_distance_too_far:\n"
"        movl    $4, 116(%%rsp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_break_loop:\n"
"        movl    $0, 116(%%rsp)\n"

".L_break_loop_with_status:\n"

"        movq    %%rsi, 16(%%rsp)\n"
"        movq    %%rdi, 32(%%rsp)\n"
"        movl    %%ebx, 88(%%rsp)\n"
"        movq    %%rdx, 80(%%rsp)\n"
"        movq    (%%rsp), %%rax\n"
"        movq    8(%%rsp), %%rbp\n"
"        movq    %%rax, %%rsp\n"
          :
          : "m" (ar)
          : "memory", "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi",
            "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
    );
#elif ( defined( __GNUC__ ) || defined( __ICC ) ) && defined( __i386 )
    __asm__ __volatile__ (
"        leal    %0, %%eax\n"
"        movl    %%esp, (%%eax)\n"
"        movl    %%ebp, 4(%%eax)\n"
"        movl    %%eax, %%esp\n"
"        movl    8(%%esp), %%esi\n"
"        movl    16(%%esp), %%edi\n"
"        movl    40(%%esp), %%edx\n"
"        movl    44(%%esp), %%ebx\n"
"        movl    32(%%esp), %%ebp\n"

"        cld\n"
"        jmp     .L_do_loop\n"

".align 32,0x90\n"
".L_while_test:\n"
"        cmpl    %%edi, 24(%%esp)\n"
"        jbe     .L_break_loop\n"
"        cmpl    %%esi, 12(%%esp)\n"
"        jbe     .L_break_loop\n"

".L_do_loop:\n"
"        cmpb    $15, %%bl\n"
"        ja      .L_get_length_code\n"

"        xorl    %%eax, %%eax\n"
"        lodsw\n"
"        movb    %%bl, %%cl\n"
"        addb    $16, %%bl\n"
"        shll    %%cl, %%eax\n"
"        orl     %%eax, %%edx\n"

".L_get_length_code:\n"
"        movl    56(%%esp), %%eax\n"
"        andl    %%edx, %%eax\n"
"        movl    (%%ebp,%%eax,4), %%eax\n"

".L_dolen:\n"
"        movb    %%ah, %%cl\n"
"        subb    %%ah, %%bl\n"
"        shrl    %%cl, %%edx\n"

"        testb   %%al, %%al\n"
"        jnz     .L_test_for_length_base\n"

"        shrl    $16, %%eax\n"
"        stosb\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_test_for_length_base:\n"
"        movl    %%eax, %%ecx\n"
"        shrl    $16, %%ecx\n"
"        movl    %%ecx, 64(%%esp)\n"
"        movb    %%al, %%cl\n"

"        testb   $16, %%al\n"
"        jz      .L_test_for_second_level_length\n"
"        andb    $15, %%cl\n"
"        jz      .L_decode_distance\n"
"        cmpb    %%cl, %%bl\n"
"        jae     .L_add_bits_to_len\n"

"        movb    %%cl, %%ch\n"
"        xorl    %%eax, %%eax\n"
"        lodsw\n"
"        movb    %%bl, %%cl\n"
"        addb    $16, %%bl\n"
"        shll    %%cl, %%eax\n"
"        orl     %%eax, %%edx\n"
"        movb    %%ch, %%cl\n"

".L_add_bits_to_len:\n"
"        subb    %%cl, %%bl\n"
"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        shrl    %%cl, %%edx\n"
"        addl    %%eax, 64(%%esp)\n"

".L_decode_distance:\n"
"        cmpb    $15, %%bl\n"
"        ja      .L_get_distance_code\n"

"        xorl    %%eax, %%eax\n"
"        lodsw\n"
"        movb    %%bl, %%cl\n"
"        addb    $16, %%bl\n"
"        shll    %%cl, %%eax\n"
"        orl     %%eax, %%edx\n"

".L_get_distance_code:\n"
"        movl    60(%%esp), %%eax\n"
"        movl    36(%%esp), %%ecx\n"
"        andl    %%edx, %%eax\n"
"        movl    (%%ecx,%%eax,4), %%eax\n"

".L_dodist:\n"
"        movl    %%eax, %%ebp\n"
"        shrl    $16, %%ebp\n"
"        movb    %%ah, %%cl\n"
"        subb    %%ah, %%bl\n"
"        shrl    %%cl, %%edx\n"
"        movb    %%al, %%cl\n"

"        testb   $16, %%al\n"
"        jz      .L_test_for_second_level_dist\n"
"        andb    $15, %%cl\n"
"        jz      .L_check_dist_one\n"
"        cmpb    %%cl, %%bl\n"
"        jae     .L_add_bits_to_dist\n"

"        movb    %%cl, %%ch\n"
"        xorl    %%eax, %%eax\n"
"        lodsw\n"
"        movb    %%bl, %%cl\n"
"        addb    $16, %%bl\n"
"        shll    %%cl, %%eax\n"
"        orl     %%eax, %%edx\n"
"        movb    %%ch, %%cl\n"

".L_add_bits_to_dist:\n"
"        subb    %%cl, %%bl\n"
"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        shrl    %%cl, %%edx\n"
"        addl    %%eax, %%ebp\n"

".L_check_window:\n"
"        movl    %%esi, 8(%%esp)\n"
"        movl    %%edi, %%eax\n"
"        subl    20(%%esp), %%eax\n"

"        cmpl    %%ebp, %%eax\n"
"        jb      .L_clip_window\n"

"        movl    64(%%esp), %%ecx\n"
"        movl    %%edi, %%esi\n"
"        subl    %%ebp, %%esi\n"

"        sarl    %%ecx\n"
"        jnc     .L_copy_two\n"

"        rep     movsw\n"
"        movb    (%%esi), %%al\n"
"        movb    %%al, (%%edi)\n"
"        incl    %%edi\n"

"        movl    8(%%esp), %%esi\n"
"        movl    32(%%esp), %%ebp\n"
"        jmp     .L_while_test\n"

".L_copy_two:\n"
"        rep     movsw\n"
"        movl    8(%%esp), %%esi\n"
"        movl    32(%%esp), %%ebp\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_check_dist_one:\n"
"        cmpl    $1, %%ebp\n"
"        jne     .L_check_window\n"
"        cmpl    %%edi, 20(%%esp)\n"
"        je      .L_check_window\n"

"        movl    64(%%esp), %%ecx\n"
"        movb    -1(%%edi), %%al\n"
"        movb    %%al, %%ah\n"

"        sarl    %%ecx\n"
"        jnc     .L_set_two\n"
"        movb    %%al, (%%edi)\n"
"        incl    %%edi\n"

".L_set_two:\n"
"        rep     stosw\n"
"        movl    32(%%esp), %%ebp\n"
"        jmp     .L_while_test\n"

".align 32,0x90\n"
".L_test_for_second_level_length:\n"
"        testb   $64, %%al\n"
"        jnz     .L_test_for_end_of_block\n"

"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        addl    64(%%esp), %%eax\n"
"        movl    (%%ebp,%%eax,4), %%eax\n"
"        jmp     .L_dolen\n"

".align 32,0x90\n"
".L_test_for_second_level_dist:\n"
"        testb   $64, %%al\n"
"        jnz     .L_invalid_distance_code\n"

"        xorl    %%eax, %%eax\n"
"        incl    %%eax\n"
"        shll    %%cl, %%eax\n"
"        decl    %%eax\n"
"        andl    %%edx, %%eax\n"
"        addl    %%ebp, %%eax\n"
"        movl    36(%%esp), %%ecx\n"
"        movl    (%%ecx,%%eax,4), %%eax\n"
"        jmp     .L_dodist\n"

".align 32,0x90\n"
".L_clip_window:\n"
"        movl    %%eax, %%ecx\n"
"        movl    48(%%esp), %%eax\n"
"        negl    %%ecx\n"
"        movl    28(%%esp), %%esi\n"

"        cmpl    %%ebp, %%eax\n"
"        jb      .L_invalid_distance_too_far\n"

"        addl    %%ebp, %%ecx\n"
"        cmpl    $0, 52(%%esp)\n"
"        jne     .L_wrap_around_window\n"

"        subl    %%ecx, %%eax\n"
"        addl    %%eax, %%esi\n"

"        movl    64(%%esp), %%eax\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movl    %%edi, %%esi\n"
"        subl    %%ebp, %%esi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_wrap_around_window:\n"
"        movl    52(%%esp), %%eax\n"
"        cmpl    %%eax, %%ecx\n"
"        jbe     .L_contiguous_in_window\n"

"        addl    48(%%esp), %%esi\n"
"        addl    %%eax, %%esi\n"
"        subl    %%ecx, %%esi\n"
"        subl    %%eax, %%ecx\n"

"        movl    64(%%esp), %%eax\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movl    28(%%esp), %%esi\n"
"        movl    52(%%esp), %%ecx\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movl    %%edi, %%esi\n"
"        subl    %%ebp, %%esi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_contiguous_in_window:\n"
"        addl    %%eax, %%esi\n"
"        subl    %%ecx, %%esi\n"

"        movl    64(%%esp), %%eax\n"
"        cmpl    %%ecx, %%eax\n"
"        jbe     .L_do_copy\n"

"        subl    %%ecx, %%eax\n"
"        rep     movsb\n"
"        movl    %%edi, %%esi\n"
"        subl    %%ebp, %%esi\n"
"        jmp     .L_do_copy\n"

".align 32,0x90\n"
".L_do_copy:\n"
"        movl    %%eax, %%ecx\n"
"        rep     movsb\n"

"        movl    8(%%esp), %%esi\n"
"        movl    32(%%esp), %%ebp\n"
"        jmp     .L_while_test\n"

".L_test_for_end_of_block:\n"
"        testb   $32, %%al\n"
"        jz      .L_invalid_literal_length_code\n"
"        movl    $1, 72(%%esp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_literal_length_code:\n"
"        movl    $2, 72(%%esp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_distance_code:\n"
"        movl    $3, 72(%%esp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_invalid_distance_too_far:\n"
"        movl    8(%%esp), %%esi\n"
"        movl    $4, 72(%%esp)\n"
"        jmp     .L_break_loop_with_status\n"

".L_break_loop:\n"
"        movl    $0, 72(%%esp)\n"

".L_break_loop_with_status:\n"

"        movl    %%esi, 8(%%esp)\n"
"        movl    %%edi, 16(%%esp)\n"
"        movl    %%ebx, 44(%%esp)\n"
"        movl    %%edx, 40(%%esp)\n"
"        movl    4(%%esp), %%ebp\n"
"        movl    (%%esp), %%esp\n"
          :
          : "m" (ar)
          : "memory", "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"
    );
#elif defined( _MSC_VER ) && ! defined( _M_AMD64 )
    __asm {
	lea	eax, ar
	mov	[eax], esp
	mov	[eax+4], ebp
	mov	esp, eax
	mov	esi, [esp+8]
	mov	edi, [esp+16]
	mov	edx, [esp+40]
	mov	ebx, [esp+44]
	mov	ebp, [esp+32]

	cld
	jmp	L_do_loop

ALIGN 4
L_while_test:
	cmp	[esp+24], edi
	jbe	L_break_loop
	cmp	[esp+12], esi
	jbe	L_break_loop

L_do_loop:
	cmp	bl, 15
	ja	L_get_length_code

	xor	eax, eax
	lodsw
	mov	cl, bl
	add	bl, 16
	shl	eax, cl
	or	edx, eax

L_get_length_code:
	mov	eax, [esp+56]
	and	eax, edx
	mov	eax, [ebp+eax*4]

L_dolen:
	mov	cl, ah
	sub	bl, ah
	shr	edx, cl

	test	al, al
	jnz	L_test_for_length_base

	shr	eax, 16
	stosb
	jmp	L_while_test

ALIGN 4
L_test_for_length_base:
	mov	ecx, eax
	shr	ecx, 16
	mov	[esp+64], ecx
	mov	cl, al

	test	al, 16
	jz	L_test_for_second_level_length
	and	cl, 15
	jz	L_decode_distance
	cmp	bl, cl
	jae	L_add_bits_to_len

	mov	ch, cl
	xor	eax, eax
	lodsw
	mov	cl, bl
	add	bl, 16
	shl	eax, cl
	or	edx, eax
	mov	cl, ch

L_add_bits_to_len:
	sub	bl, cl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx
	shr	edx, cl
	add	[esp+64], eax

L_decode_distance:
	cmp	bl, 15
	ja	L_get_distance_code

	xor	eax, eax
	lodsw
	mov	cl, bl
	add	bl, 16
	shl	eax, cl
	or	edx, eax

L_get_distance_code:
	mov	eax, [esp+60]
	mov	ecx, [esp+36]
	and	eax, edx
	mov	eax, [ecx+eax*4]

L_dodist:
	mov	ebp, eax
	shr	ebp, 16
	mov	cl, ah
	sub	bl, ah
	shr	edx, cl
	mov	cl, al

	test	al, 16
	jz	L_test_for_second_level_dist
	and	cl, 15
	jz	L_check_dist_one
	cmp	bl, cl
	jae	L_add_bits_to_dist

	mov	ch, cl
	xor	eax, eax
	lodsw
	mov	cl, bl
	add	bl, 16
	shl	eax, cl
	or	edx, eax
	mov	cl, ch

L_add_bits_to_dist:
	sub	bl, cl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx
	shr	edx, cl
	add	ebp, eax

L_check_window:
	mov	[esp+8], esi
	mov	eax, edi
	sub	eax, [esp+20]

	cmp	eax, ebp
	jb	L_clip_window

	mov	ecx, [esp+64]
	mov	esi, edi
	sub	esi, ebp

	sar	ecx, 1
	jnc	L_copy_two

	rep     movsw
	mov	al, [esi]
	mov	[edi], al
	inc	edi

	mov	esi, [esp+8]
	mov	ebp, [esp+32]
	jmp	L_while_test

L_copy_two:
	rep     movsw
	mov	esi, [esp+8]
	mov	ebp, [esp+32]
	jmp	L_while_test

ALIGN 4
L_check_dist_one:
	cmp	ebp, 1
	jne	L_check_window
	cmp	[esp+20], edi
	je	L_check_window

	mov	ecx, [esp+64]
	mov	al, [edi-1]
	mov	ah, al

	sar	ecx, 1
	jnc	L_set_two
	mov	[edi], al
	inc	edi

L_set_two:
	rep     stosw
	mov	ebp, [esp+32]
	jmp	L_while_test

ALIGN 4
L_test_for_second_level_length:
	test	al, 64
	jnz	L_test_for_end_of_block

	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx
	add	eax, [esp+64]
	mov	eax, [ebp+eax*4]
	jmp	L_dolen

ALIGN 4
L_test_for_second_level_dist:
	test	al, 64
	jnz	L_invalid_distance_code

	xor	eax, eax
	inc	eax
	shl	eax, cl
	dec	eax
	and	eax, edx
	add	eax, ebp
	mov	ecx, [esp+36]
	mov	eax, [ecx+eax*4]
	jmp	L_dodist

ALIGN 4
L_clip_window:
	mov	ecx, eax
	mov	eax, [esp+48]
	neg	ecx
	mov	esi, [esp+28]

	cmp	eax, ebp
	jb	L_invalid_distance_too_far

	add	ecx, ebp
	cmp	dword ptr [esp+52], 0
	jne	L_wrap_around_window

	sub	eax, ecx
	add	esi, eax

	mov	eax, [esp+64]
	cmp	eax, ecx
	jbe	L_do_copy

	sub	eax, ecx
	rep     movsb
	mov	esi, edi
	sub	esi, ebp
	jmp	L_do_copy

ALIGN 4
L_wrap_around_window:
	mov	eax, [esp+52]
	cmp	ecx, eax
	jbe	L_contiguous_in_window

	add	esi, [esp+48]
	add	esi, eax
	sub	esi, ecx
	sub	ecx, eax

	mov	eax, [esp+64]
	cmp	eax, ecx
	jbe	L_do_copy

	sub	eax, ecx
	rep     movsb
	mov	esi, [esp+28]
	mov	ecx, [esp+52]
	cmp	eax, ecx
	jbe	L_do_copy

	sub	eax, ecx
	rep     movsb
	mov	esi, edi
	sub	esi, ebp
	jmp	L_do_copy

ALIGN 4
L_contiguous_in_window:
	add	esi, eax
	sub	esi, ecx

	mov	eax, [esp+64]
	cmp	eax, ecx
	jbe	L_do_copy

	sub	eax, ecx
	rep     movsb
	mov	esi, edi
	sub	esi, ebp
	jmp	L_do_copy

ALIGN 4
L_do_copy:
	mov	ecx, eax
	rep     movsb

	mov	esi, [esp+8]
	mov	ebp, [esp+32]
	jmp	L_while_test

L_test_for_end_of_block:
	test	al, 32
	jz	L_invalid_literal_length_code
	mov	dword ptr [esp+72], 1
	jmp	L_break_loop_with_status

L_invalid_literal_length_code:
	mov	dword ptr [esp+72], 2
	jmp	L_break_loop_with_status

L_invalid_distance_code:
	mov	dword ptr [esp+72], 3
	jmp	L_break_loop_with_status

L_invalid_distance_too_far:
	mov	esi, [esp+4]
	mov	dword ptr [esp+72], 4
	jmp	L_break_loop_with_status

L_break_loop:
	mov	dword ptr [esp+72], 0

L_break_loop_with_status:

	mov	[esp+8], esi
	mov	[esp+16], edi
	mov	[esp+44], ebx
	mov	[esp+40], edx
	mov	ebp, [esp+4]
	mov	esp, [esp]
    }
#else
#error "x86 architecture not defined"
#endif

    if (ar.status > 1) {
        if (ar.status == 2)
            strm->msg = "invalid literal/length code";
        else if (ar.status == 3)
            strm->msg = "invalid distance code";
        else
            strm->msg = "invalid distance too far back";
        state->mode = BAD;
    }
    else if ( ar.status == 1 ) {
        state->mode = TYPE;
    }


    ar.len = ar.bits >> 3;
    ar.in -= ar.len;
    ar.bits -= ar.len << 3;
    ar.hold &= (1U << ar.bits) - 1;


    strm->next_in = ar.in;
    strm->next_out = ar.out;
    strm->avail_in = (unsigned)(ar.in < ar.last ?
                                PAD_AVAIL_IN + (ar.last - ar.in) :
                                PAD_AVAIL_IN - (ar.in - ar.last));
    strm->avail_out = (unsigned)(ar.out < ar.end ?
                                 PAD_AVAIL_OUT + (ar.end - ar.out) :
                                 PAD_AVAIL_OUT - (ar.out - ar.end));
    state->hold = ar.hold;
    state->bits = ar.bits;
    return;
}

