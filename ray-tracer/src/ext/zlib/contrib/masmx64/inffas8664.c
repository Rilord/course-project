

#include <stdio.h>
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"







    typedef struct inffast_ar {


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
 size_t hold;
 unsigned bits;
 unsigned wsize;
 unsigned write;
 unsigned lmask;
 unsigned dmask;
 unsigned len;
 unsigned dist;
 unsigned status;
    } type_ar;
#ifdef ASMINF

void inflate_fast(strm, start)
z_streamp strm;
unsigned start;
{
    struct inflate_state FAR *state;
    type_ar ar;
    void inffas8664fnc(struct inffast_ar * par);



#if (defined( __GNUC__ ) && defined( __amd64__ ) && ! defined( __i386 )) || (defined(_MSC_VER) && defined(_M_AMD64))
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




    while (((size_t)(void *)ar.in & (sizeof(ar.hold) / 2 - 1)) != 0) {
        ar.hold += (unsigned long)*ar.in++ << ar.bits;
        ar.bits += 8;
    }

    inffas8664fnc(&ar);

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
    state->hold = (unsigned long)ar.hold;
    state->bits = ar.bits;
    return;
}

#endif
