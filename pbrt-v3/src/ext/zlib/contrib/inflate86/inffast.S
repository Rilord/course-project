



#if ! defined( GAS_COFF ) && ! defined( GAS_ELF )

#if defined( WIN32 ) || defined( __CYGWIN__ )
#define GAS_COFF
#else
#define GAS_ELF
#endif

#endif


#if defined( GAS_COFF )


#define inflate_fast _inflate_fast
#define inflate_fast_use_mmx _inflate_fast_use_mmx

#endif


.file "inffast.S"

.globl inflate_fast

.text
.align 4,0
.L_invalid_literal_length_code_msg:
.string "invalid literal/length code"

.align 4,0
.L_invalid_distance_code_msg:
.string "invalid distance code"

.align 4,0
.L_invalid_distance_too_far_msg:
.string "invalid distance too far back"

#if ! defined( NO_MMX )
.align 4,0
.L_mask:
.long 0
.long 1
.long 3
.long 7
.long 15
.long 31
.long 63
.long 127
.long 255
.long 511
.long 1023
.long 2047
.long 4095
.long 8191
.long 16383
.long 32767
.long 65535
.long 131071
.long 262143
.long 524287
.long 1048575
.long 2097151
.long 4194303
.long 8388607
.long 16777215
.long 33554431
.long 67108863
.long 134217727
.long 268435455
.long 536870911
.long 1073741823
.long 2147483647
.long 4294967295
#endif

.text


#define next_in_strm   0
#define avail_in_strm  4
#define next_out_strm  12
#define avail_out_strm 16
#define msg_strm       24
#define state_strm     28


#define mode_state     0
#define wsize_state    32
#define write_state    40
#define window_state   44
#define hold_state     48
#define bits_state     52
#define lencode_state  68
#define distcode_state 72
#define lenbits_state  76
#define distbits_state 80


#define local_var_size 64
#define strm_sp        88
#define start_sp       92


#define out            60
#define window         56
#define wsize          52
#define write          48
#define in             44
#define beg            40
#define buf            28
#define len            24
#define last           20
#define end            16
#define dcode          12
#define lcode           8
#define dmask           4
#define lmask           0


#define INFLATE_MODE_TYPE 11
#define INFLATE_MODE_BAD  26


#if ! defined( USE_MMX ) && ! defined( NO_MMX )

#define RUN_TIME_MMX

#define CHECK_MMX    1
#define DO_USE_MMX   2
#define DONT_USE_MMX 3

.globl inflate_fast_use_mmx

.data

.align 4,0
inflate_fast_use_mmx:
.long CHECK_MMX

#if defined( GAS_ELF )

.type   inflate_fast_use_mmx,@object
.size   inflate_fast_use_mmx,4
#endif

#endif

#if defined( GAS_COFF )

.def inflate_fast; .scl 2; .type 32; .endef
#endif

.text

.align 32,0x90
inflate_fast:
        pushl   %edi
        pushl   %esi
        pushl   %ebp
        pushl   %ebx
        pushf
        subl    $local_var_size, %esp
        cld

#define strm_r  %esi
#define state_r %edi

        movl    strm_sp(%esp), strm_r
        movl    state_strm(strm_r), state_r


        movl    avail_in_strm(strm_r), %edx
        movl    next_in_strm(strm_r), %eax

        addl    %eax, %edx
        subl    $11, %edx

        movl    %eax, in(%esp)
        movl    %edx, last(%esp)

        movl    start_sp(%esp), %ebp
        movl    avail_out_strm(strm_r), %ecx
        movl    next_out_strm(strm_r), %ebx

        subl    %ecx, %ebp
        negl    %ebp
        addl    %ebx, %ebp

        subl    $257, %ecx
        addl    %ebx, %ecx

        movl    %ebx, out(%esp)
        movl    %ebp, beg(%esp)
        movl    %ecx, end(%esp)



        movl    lencode_state(state_r), %eax
        movl    distcode_state(state_r), %ecx

        movl    %eax, lcode(%esp)
        movl    %ecx, dcode(%esp)

        movl    $1, %eax
        movl    lenbits_state(state_r), %ecx
        shll    %cl, %eax
        decl    %eax
        movl    %eax, lmask(%esp)

        movl    $1, %eax
        movl    distbits_state(state_r), %ecx
        shll    %cl, %eax
        decl    %eax
        movl    %eax, dmask(%esp)

        movl    wsize_state(state_r), %eax
        movl    write_state(state_r), %ecx
        movl    window_state(state_r), %edx

        movl    %eax, wsize(%esp)
        movl    %ecx, write(%esp)
        movl    %edx, window(%esp)

        movl    hold_state(state_r), %ebp
        movl    bits_state(state_r), %ebx

#undef strm_r
#undef state_r

#define in_r       %esi
#define from_r     %esi
#define out_r      %edi

        movl    in(%esp), in_r
        movl    last(%esp), %ecx
        cmpl    in_r, %ecx
        ja      .L_align_long

        addl    $11, %ecx
        subl    in_r, %ecx
        movl    $12, %eax
        subl    %ecx, %eax
        leal    buf(%esp), %edi
        rep     movsb
        movl    %eax, %ecx
        xorl    %eax, %eax
        rep     stosb
        leal    buf(%esp), in_r
        movl    in_r, last(%esp)
        jmp     .L_is_aligned


.L_align_long:
        testl   $3, in_r
        jz      .L_is_aligned
        xorl    %eax, %eax
        movb    (in_r), %al
        incl    in_r
        movl    %ebx, %ecx
        addl    $8, %ebx
        shll    %cl, %eax
        orl     %eax, %ebp
        jmp     .L_align_long

.L_is_aligned:
        movl    out(%esp), out_r

#if defined( NO_MMX )
        jmp     .L_do_loop
#endif

#if defined( USE_MMX )
        jmp     .L_init_mmx
#endif



#if defined( RUN_TIME_MMX )
.L_check_mmx:
        cmpl    $DO_USE_MMX, inflate_fast_use_mmx
        je      .L_init_mmx
        ja      .L_do_loop

        pushl   %eax
        pushl   %ebx
        pushl   %ecx
        pushl   %edx
        pushf
        movl    (%esp), %eax
        xorl    $0x200000, (%esp)
        popf
        pushf
        popl    %edx
        xorl    %eax, %edx
        jz      .L_dont_use_mmx
        xorl    %eax, %eax
        cpuid
        cmpl    $0x756e6547, %ebx
        jne     .L_dont_use_mmx
        cmpl    $0x6c65746e, %ecx
        jne     .L_dont_use_mmx
        cmpl    $0x49656e69, %edx
        jne     .L_dont_use_mmx
        movl    $1, %eax
        cpuid
        shrl    $8, %eax
        andl    $15, %eax
        cmpl    $6, %eax
        jne     .L_dont_use_mmx
        testl   $0x800000, %edx
        jnz     .L_use_mmx
        jmp     .L_dont_use_mmx
.L_use_mmx:
        movl    $DO_USE_MMX, inflate_fast_use_mmx
        jmp     .L_check_mmx_pop
.L_dont_use_mmx:
        movl    $DONT_USE_MMX, inflate_fast_use_mmx
.L_check_mmx_pop:
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        jmp     .L_check_mmx
#endif




#if defined ( NO_MMX ) || defined( RUN_TIME_MMX )

#define hold_r     %ebp
#define bits_r     %bl
#define bitslong_r %ebx

.align 32,0x90
.L_while_test:

        cmpl    out_r, end(%esp)
        jbe     .L_break_loop

        cmpl    in_r, last(%esp)
        jbe     .L_break_loop

.L_do_loop:

        cmpb    $15, bits_r
        ja      .L_get_length_code

        xorl    %eax, %eax
        lodsw
        movb    bits_r, %cl
        addb    $16, bits_r
        shll    %cl, %eax
        orl     %eax, hold_r

.L_get_length_code:
        movl    lmask(%esp), %edx
        movl    lcode(%esp), %ecx
        andl    hold_r, %edx
        movl    (%ecx,%edx,4), %eax

.L_dolen:

        movb    %ah, %cl
        subb    %ah, bits_r
        shrl    %cl, hold_r


        testb   %al, %al
        jnz     .L_test_for_length_base

        shrl    $16, %eax
        stosb
        jmp     .L_while_test

.L_test_for_length_base:

#define len_r %edx
        movl    %eax, len_r
        shrl    $16, len_r
        movb    %al, %cl

        testb   $16, %al
        jz      .L_test_for_second_level_length
        andb    $15, %cl
        jz      .L_save_len
        cmpb    %cl, bits_r
        jae     .L_add_bits_to_len

        movb    %cl, %ch
        xorl    %eax, %eax
        lodsw
        movb    bits_r, %cl
        addb    $16, bits_r
        shll    %cl, %eax
        orl     %eax, hold_r
        movb    %ch, %cl

.L_add_bits_to_len:
        movl    $1, %eax
        shll    %cl, %eax
        decl    %eax
        subb    %cl, bits_r
        andl    hold_r, %eax
        shrl    %cl, hold_r
        addl    %eax, len_r

.L_save_len:
        movl    len_r, len(%esp)
#undef  len_r

.L_decode_distance:


        cmpb    $15, bits_r
        ja      .L_get_distance_code

        xorl    %eax, %eax
        lodsw
        movb    bits_r, %cl
        addb    $16, bits_r
        shll    %cl, %eax
        orl     %eax, hold_r

.L_get_distance_code:
        movl    dmask(%esp), %edx
        movl    dcode(%esp), %ecx
        andl    hold_r, %edx
        movl    (%ecx,%edx,4), %eax

#define dist_r %edx
.L_dodist:
        movl    %eax, dist_r
        shrl    $16, dist_r
        movb    %ah, %cl
        subb    %ah, bits_r
        shrl    %cl, hold_r


        movb    %al, %cl

        testb   $16, %al
        jz      .L_test_for_second_level_dist
        andb    $15, %cl
        jz      .L_check_dist_one
        cmpb    %cl, bits_r
        jae     .L_add_bits_to_dist

        movb    %cl, %ch
        xorl    %eax, %eax
        lodsw
        movb    bits_r, %cl
        addb    $16, bits_r
        shll    %cl, %eax
        orl     %eax, hold_r
        movb    %ch, %cl

.L_add_bits_to_dist:
        movl    $1, %eax
        shll    %cl, %eax
        decl    %eax
        subb    %cl, bits_r
        andl    hold_r, %eax
        shrl    %cl, hold_r
        addl    %eax, dist_r
        jmp     .L_check_window

.L_check_window:


        movl    in_r, in(%esp)
        movl    out_r, %eax
        subl    beg(%esp), %eax

        cmpl    dist_r, %eax
        jb      .L_clip_window

        movl    len(%esp), %ecx
        movl    out_r, from_r
        subl    dist_r, from_r

        subl    $3, %ecx
        movb    (from_r), %al
        movb    %al, (out_r)
        movb    1(from_r), %al
        movb    2(from_r), %dl
        addl    $3, from_r
        movb    %al, 1(out_r)
        movb    %dl, 2(out_r)
        addl    $3, out_r
        rep     movsb

        movl    in(%esp), in_r
        jmp     .L_while_test

.align 16,0x90
.L_check_dist_one:
        cmpl    $1, dist_r
        jne     .L_check_window
        cmpl    out_r, beg(%esp)
        je      .L_check_window

        decl    out_r
        movl    len(%esp), %ecx
        movb    (out_r), %al
        subl    $3, %ecx

        movb    %al, 1(out_r)
        movb    %al, 2(out_r)
        movb    %al, 3(out_r)
        addl    $4, out_r
        rep     stosb

        jmp     .L_while_test

.align 16,0x90
.L_test_for_second_level_length:

        testb   $64, %al
        jnz     .L_test_for_end_of_block

        movl    $1, %eax
        shll    %cl, %eax
        decl    %eax
        andl    hold_r, %eax
        addl    %edx, %eax
        movl    lcode(%esp), %edx
        movl    (%edx,%eax,4), %eax
        jmp     .L_dolen

.align 16,0x90
.L_test_for_second_level_dist:

        testb   $64, %al
        jnz     .L_invalid_distance_code

        movl    $1, %eax
        shll    %cl, %eax
        decl    %eax
        andl    hold_r, %eax
        addl    %edx, %eax
        movl    dcode(%esp), %edx
        movl    (%edx,%eax,4), %eax
        jmp     .L_dodist

.align 16,0x90
.L_clip_window:

#define nbytes_r %ecx
        movl    %eax, nbytes_r
        movl    wsize(%esp), %eax
        negl    nbytes_r
        movl    window(%esp), from_r

        cmpl    dist_r, %eax
        jb      .L_invalid_distance_too_far

        addl    dist_r, nbytes_r
        cmpl    $0, write(%esp)
        jne     .L_wrap_around_window

        subl    nbytes_r, %eax
        addl    %eax, from_r


#define len_r %eax
        movl    len(%esp), len_r
        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1

        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1

.L_wrap_around_window:

#define write_r %eax
        movl    write(%esp), write_r
        cmpl    write_r, nbytes_r
        jbe     .L_contiguous_in_window

        addl    wsize(%esp), from_r
        addl    write_r, from_r
        subl    nbytes_r, from_r
        subl    write_r, nbytes_r
#undef write_r

        movl    len(%esp), len_r
        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1

        subl    nbytes_r, len_r
        rep     movsb
        movl    window(%esp), from_r
        movl    write(%esp), nbytes_r
        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1

.L_contiguous_in_window:

#define write_r %eax
        addl    write_r, from_r
        subl    nbytes_r, from_r
#undef write_r

        movl    len(%esp), len_r
        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r

.L_do_copy1:

#undef nbytes_r
#define in_r %esi
        movl    len_r, %ecx
        rep     movsb

        movl    in(%esp), in_r
        jmp     .L_while_test

#undef len_r
#undef dist_r

#endif




#if defined( USE_MMX ) || defined( RUN_TIME_MMX )

.align 32,0x90
.L_init_mmx:
        emms

#undef  bits_r
#undef  bitslong_r
#define bitslong_r %ebp
#define hold_mm    %mm0
        movd    %ebp, hold_mm
        movl    %ebx, bitslong_r

#define used_mm   %mm1
#define dmask2_mm %mm2
#define lmask2_mm %mm3
#define lmask_mm  %mm4
#define dmask_mm  %mm5
#define tmp_mm    %mm6

        movd    lmask(%esp), lmask_mm
        movq    lmask_mm, lmask2_mm
        movd    dmask(%esp), dmask_mm
        movq    dmask_mm, dmask2_mm
        pxor    used_mm, used_mm
        movl    lcode(%esp), %ebx
        jmp     .L_do_loop_mmx

.align 32,0x90
.L_while_test_mmx:

        cmpl    out_r, end(%esp)
        jbe     .L_break_loop

        cmpl    in_r, last(%esp)
        jbe     .L_break_loop

.L_do_loop_mmx:
        psrlq   used_mm, hold_mm

        cmpl    $32, bitslong_r
        ja      .L_get_length_code_mmx

        movd    bitslong_r, tmp_mm
        movd    (in_r), %mm7
        addl    $4, in_r
        psllq   tmp_mm, %mm7
        addl    $32, bitslong_r
        por     %mm7, hold_mm

.L_get_length_code_mmx:
        pand    hold_mm, lmask_mm
        movd    lmask_mm, %eax
        movq    lmask2_mm, lmask_mm
        movl    (%ebx,%eax,4), %eax

.L_dolen_mmx:
        movzbl  %ah, %ecx
        movd    %ecx, used_mm
        subl    %ecx, bitslong_r

        testb   %al, %al
        jnz     .L_test_for_length_base_mmx

        shrl    $16, %eax
        stosb
        jmp     .L_while_test_mmx

.L_test_for_length_base_mmx:
#define len_r  %edx
        movl    %eax, len_r
        shrl    $16, len_r

        testb   $16, %al
        jz      .L_test_for_second_level_length_mmx
        andl    $15, %eax
        jz      .L_decode_distance_mmx

        psrlq   used_mm, hold_mm
        movd    %eax, used_mm
        movd    hold_mm, %ecx
        subl    %eax, bitslong_r
        andl    .L_mask(,%eax,4), %ecx
        addl    %ecx, len_r

.L_decode_distance_mmx:
        psrlq   used_mm, hold_mm

        cmpl    $32, bitslong_r
        ja      .L_get_dist_code_mmx

        movd    bitslong_r, tmp_mm
        movd    (in_r), %mm7
        addl    $4, in_r
        psllq   tmp_mm, %mm7
        addl    $32, bitslong_r
        por     %mm7, hold_mm

.L_get_dist_code_mmx:
        movl    dcode(%esp), %ebx
        pand    hold_mm, dmask_mm
        movd    dmask_mm, %eax
        movq    dmask2_mm, dmask_mm
        movl    (%ebx,%eax,4), %eax

.L_dodist_mmx:
#define dist_r %ebx
        movzbl  %ah, %ecx
        movl    %eax, dist_r
        shrl    $16, dist_r
        subl    %ecx, bitslong_r
        movd    %ecx, used_mm

        testb   $16, %al
        jz      .L_test_for_second_level_dist_mmx
        andl    $15, %eax
        jz      .L_check_dist_one_mmx

.L_add_bits_to_dist_mmx:
        psrlq   used_mm, hold_mm
        movd    %eax, used_mm
        movd    hold_mm, %ecx
        subl    %eax, bitslong_r
        andl    .L_mask(,%eax,4), %ecx
        addl    %ecx, dist_r

.L_check_window_mmx:
        movl    in_r, in(%esp)
        movl    out_r, %eax
        subl    beg(%esp), %eax

        cmpl    dist_r, %eax
        jb      .L_clip_window_mmx

        movl    len_r, %ecx
        movl    out_r, from_r
        subl    dist_r, from_r

        subl    $3, %ecx
        movb    (from_r), %al
        movb    %al, (out_r)
        movb    1(from_r), %al
        movb    2(from_r), %dl
        addl    $3, from_r
        movb    %al, 1(out_r)
        movb    %dl, 2(out_r)
        addl    $3, out_r
        rep     movsb

        movl    in(%esp), in_r
        movl    lcode(%esp), %ebx
        jmp     .L_while_test_mmx

.align 16,0x90
.L_check_dist_one_mmx:
        cmpl    $1, dist_r
        jne     .L_check_window_mmx
        cmpl    out_r, beg(%esp)
        je      .L_check_window_mmx

        decl    out_r
        movl    len_r, %ecx
        movb    (out_r), %al
        subl    $3, %ecx

        movb    %al, 1(out_r)
        movb    %al, 2(out_r)
        movb    %al, 3(out_r)
        addl    $4, out_r
        rep     stosb

        movl    lcode(%esp), %ebx
        jmp     .L_while_test_mmx

.align 16,0x90
.L_test_for_second_level_length_mmx:
        testb   $64, %al
        jnz     .L_test_for_end_of_block

        andl    $15, %eax
        psrlq   used_mm, hold_mm
        movd    hold_mm, %ecx
        andl    .L_mask(,%eax,4), %ecx
        addl    len_r, %ecx
        movl    (%ebx,%ecx,4), %eax
        jmp     .L_dolen_mmx

.align 16,0x90
.L_test_for_second_level_dist_mmx:
        testb   $64, %al
        jnz     .L_invalid_distance_code

        andl    $15, %eax
        psrlq   used_mm, hold_mm
        movd    hold_mm, %ecx
        andl    .L_mask(,%eax,4), %ecx
        movl    dcode(%esp), %eax
        addl    dist_r, %ecx
        movl    (%eax,%ecx,4), %eax
        jmp     .L_dodist_mmx

.align 16,0x90
.L_clip_window_mmx:
#define nbytes_r %ecx
        movl    %eax, nbytes_r
        movl    wsize(%esp), %eax
        negl    nbytes_r
        movl    window(%esp), from_r

        cmpl    dist_r, %eax
        jb      .L_invalid_distance_too_far

        addl    dist_r, nbytes_r
        cmpl    $0, write(%esp)
        jne     .L_wrap_around_window_mmx

        subl    nbytes_r, %eax
        addl    %eax, from_r

        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1_mmx

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1_mmx

        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1_mmx

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1_mmx

.L_wrap_around_window_mmx:
#define write_r %eax
        movl    write(%esp), write_r
        cmpl    write_r, nbytes_r
        jbe     .L_contiguous_in_window_mmx

        addl    wsize(%esp), from_r
        addl    write_r, from_r
        subl    nbytes_r, from_r
        subl    write_r, nbytes_r
#undef write_r

        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1_mmx

        subl    nbytes_r, len_r
        rep     movsb
        movl    window(%esp), from_r
        movl    write(%esp), nbytes_r
        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1_mmx

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r
        jmp     .L_do_copy1_mmx

.L_contiguous_in_window_mmx:
#define write_r %eax
        addl    write_r, from_r
        subl    nbytes_r, from_r
#undef write_r

        cmpl    nbytes_r, len_r
        jbe     .L_do_copy1_mmx

        subl    nbytes_r, len_r
        rep     movsb
        movl    out_r, from_r
        subl    dist_r, from_r

.L_do_copy1_mmx:
#undef nbytes_r
#define in_r %esi
        movl    len_r, %ecx
        rep     movsb

        movl    in(%esp), in_r
        movl    lcode(%esp), %ebx
        jmp     .L_while_test_mmx

#undef hold_r
#undef bitslong_r

#endif




.L_invalid_distance_code:

        movl    $.L_invalid_distance_code_msg, %ecx
        movl    $INFLATE_MODE_BAD, %edx
        jmp     .L_update_stream_state

.L_test_for_end_of_block:

        testb   $32, %al
        jz      .L_invalid_literal_length_code

        movl    $0, %ecx
        movl    $INFLATE_MODE_TYPE, %edx
        jmp     .L_update_stream_state

.L_invalid_literal_length_code:

        movl    $.L_invalid_literal_length_code_msg, %ecx
        movl    $INFLATE_MODE_BAD, %edx
        jmp     .L_update_stream_state

.L_invalid_distance_too_far:

        movl    in(%esp), in_r
        movl    $.L_invalid_distance_too_far_msg, %ecx
        movl    $INFLATE_MODE_BAD, %edx
        jmp     .L_update_stream_state

.L_update_stream_state:

        movl    strm_sp(%esp), %eax
        testl   %ecx, %ecx
        jz      .L_skip_msg
        movl    %ecx, msg_strm(%eax)
.L_skip_msg:
        movl    state_strm(%eax), %eax
        movl    %edx, mode_state(%eax)
        jmp     .L_break_loop

.align 32,0x90
.L_break_loop:



#if defined( USE_MMX ) || defined( RUN_TIME_MMX )

#if defined( RUN_TIME_MMX )

        cmpl    $DO_USE_MMX, inflate_fast_use_mmx
        jne     .L_update_next_in

#endif

        movl    %ebp, %ebx

.L_update_next_in:

#endif

#define strm_r  %eax
#define state_r %edx


        movl    strm_sp(%esp), strm_r
        movl    %ebx, %ecx
        movl    state_strm(strm_r), state_r
        shrl    $3, %ecx
        subl    %ecx, in_r
        shll    $3, %ecx
        subl    %ecx, %ebx
        movl    out_r, next_out_strm(strm_r)
        movl    %ebx, bits_state(state_r)
        movl    %ebx, %ecx

        leal    buf(%esp), %ebx
        cmpl    %ebx, last(%esp)
        jne     .L_buf_not_used

        subl    %ebx, in_r
        movl    next_in_strm(strm_r), %ebx
        movl    %ebx, last(%esp)
        addl    %ebx, in_r
        movl    avail_in_strm(strm_r), %ebx
        subl    $11, %ebx
        addl    %ebx, last(%esp)

.L_buf_not_used:
        movl    in_r, next_in_strm(strm_r)

        movl    $1, %ebx
        shll    %cl, %ebx
        decl    %ebx

#if defined( USE_MMX ) || defined( RUN_TIME_MMX )

#if defined( RUN_TIME_MMX )

        cmpl    $DO_USE_MMX, inflate_fast_use_mmx
        jne     .L_update_hold

#endif

        psrlq   used_mm, hold_mm
        movd    hold_mm, %ebp

        emms

.L_update_hold:

#endif

        andl    %ebx, %ebp
        movl    %ebp, hold_state(state_r)

#define last_r %ebx


        movl    last(%esp), last_r
        cmpl    in_r, last_r
        jbe     .L_last_is_smaller

        subl    in_r, last_r
        addl    $11, last_r
        movl    last_r, avail_in_strm(strm_r)
        jmp     .L_fixup_out
.L_last_is_smaller:
        subl    last_r, in_r
        negl    in_r
        addl    $11, in_r
        movl    in_r, avail_in_strm(strm_r)

#undef last_r
#define end_r %ebx

.L_fixup_out:

        movl    end(%esp), end_r
        cmpl    out_r, end_r
        jbe     .L_end_is_smaller

        subl    out_r, end_r
        addl    $257, end_r
        movl    end_r, avail_out_strm(strm_r)
        jmp     .L_done
.L_end_is_smaller:
        subl    end_r, out_r
        negl    out_r
        addl    $257, out_r
        movl    out_r, avail_out_strm(strm_r)

#undef end_r
#undef strm_r
#undef state_r

.L_done:
        addl    $local_var_size, %esp
        popf
        popl    %ebx
        popl    %ebp
        popl    %esi
        popl    %edi
        ret

#if defined( GAS_ELF )

.type inflate_fast,@function
.size inflate_fast,.-inflate_fast
#endif
