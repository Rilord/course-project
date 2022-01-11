

#include "zutil.h"
#include "infback9.h"
#include "inftree9.h"
#include "inflate9.h"

#define WSIZE 65536UL


int ZEXPORT inflateBack9Init_(strm, window, version, stream_size)
z_stream FAR *strm;
unsigned char FAR *window;
const char *version;
int stream_size;
{
    struct inflate_state FAR *state;

    if (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (int)(sizeof(z_stream)))
        return Z_VERSION_ERROR;
    if (strm == Z_NULL || window == Z_NULL)
        return Z_STREAM_ERROR;
    strm->msg = Z_NULL;
    if (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    if (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (struct inflate_state FAR *)ZALLOC(strm, 1,
                                               sizeof(struct inflate_state));
    if (state == Z_NULL) return Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    return Z_OK;
}


#ifdef MAKEFIXED
#include <stdio.h>

void makefixed9(void)
{
    unsigned sym, bits, low, size;
    code *next, *lenfix, *distfix;
    struct inflate_state state;
    code fixed[544];


    sym = 0;
    while (sym < 144) state.lens[sym++] = 8;
    while (sym < 256) state.lens[sym++] = 9;
    while (sym < 280) state.lens[sym++] = 7;
    while (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);


    sym = 0;
    while (sym < 32) state.lens[sym++] = 5;
    distfix = next;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(next), &(bits), state.work);


    puts("    ");
    puts("");
    puts("    ");
    puts("");
    size = 1U << 9;
    printf("    static const code lenfix[%u] = {", size);
    low = 0;
    for (;;) {
        if ((low % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[low].op, lenfix[low].bits,
               lenfix[low].val);
        if (++low == size) break;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    low = 0;
    for (;;) {
        if ((low % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[low].op, distfix[low].bits,
               distfix[low].val);
        if (++low == size) break;
        putchar(',');
    }
    puts("\n    };");
}
#endif




#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } while (0)


#define PULL() \
    do { \
        if (have == 0) { \
            have = in(in_desc, &next); \
            if (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } while (0)


#define PULLBYTE() \
    do { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } while (0)


#define NEEDBITS(n) \
    do { \
        while (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } while (0)


#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))


#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)


#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } while (0)


#define ROOM() \
    do { \
        if (left == 0) { \
            put = window; \
            left = WSIZE; \
            wrap = 1; \
            if (out(out_desc, put, (unsigned)left)) { \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } while (0)


int ZEXPORT inflateBack9(strm, in, in_desc, out, out_desc)
z_stream FAR *strm;
in_func in;
void FAR *in_desc;
out_func out;
void FAR *out_desc;
{
    struct inflate_state FAR *state;
    z_const unsigned char FAR *next;
    unsigned char FAR *put;
    unsigned have;
    unsigned long left;
    inflate_mode mode;
    int lastblock;
    int wrap;
    unsigned char FAR *window;
    unsigned long hold;
    unsigned bits;
    unsigned extra;
    unsigned long length;
    unsigned long offset;
    unsigned long copy;
    unsigned char FAR *from;
    code const FAR *lencode;
    code const FAR *distcode;
    unsigned lenbits;
    unsigned distbits;
    code here;
    code last;
    unsigned len;
    int ret;
    static const unsigned short order[19] =
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#include "inffix9.h"


    if (strm == Z_NULL || strm->state == Z_NULL)
        return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;


    strm->msg = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    left = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;


    for (;;)
        switch (mode) {
        case TYPE:

            if (lastblock) {
                BYTEBITS();
                mode = DONE;
                break;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            case 0:
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                break;
            case 1:
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;
                break;
            case 2:
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                break;
            case 3:
                strm->msg = (char *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            break;

        case STORED:

            BYTEBITS();
            NEEDBITS(32);
            if ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (char *)"invalid stored block lengths";
                mode = BAD;
                break;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();


            while (length != 0) {
                copy = length;
                PULL();
                ROOM();
                if (copy > have) copy = have;
                if (copy > left) copy = left;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            break;

        case TABLE:

            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            if (state->nlen > 286) {
                strm->msg = (char *)"too many length symbols";
                mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));


            state->have = 0;
            while (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            while (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code const FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid code lengths set";
                mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));


            state->have = 0;
            while (state->have < state->nlen + state->ndist) {
                for (;;) {
                    here = lencode[BITS(lenbits)];
                    if ((unsigned)(here.bits) <= bits) break;
                    PULLBYTE();
                }
                if (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                else {
                    if (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        if (state->have == 0) {
                            strm->msg = (char *)"invalid bit length repeat";
                            mode = BAD;
                            break;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    else if (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    else {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    if (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (char *)"invalid bit length repeat";
                        mode = BAD;
                        break;
                    }
                    while (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }


            if (mode == BAD) break;


            if (state->lens[256] == 0) {
                strm->msg = (char *)"invalid code -- missing end-of-block";
                mode = BAD;
                break;
            }


            state->next = state->codes;
            lencode = (code const FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid literal/lengths set";
                mode = BAD;
                break;
            }
            distcode = (code const FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            if (ret) {
                strm->msg = (char *)"invalid distances set";
                mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        case LEN:

            for (;;) {
                here = lencode[BITS(lenbits)];
                if ((unsigned)(here.bits) <= bits) break;
                PULLBYTE();
            }
            if (here.op && (here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + here.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;


            if (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned char)(length);
                left--;
                mode = LEN;
                break;
            }


            if (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                break;
            }


            if (here.op & 64) {
                strm->msg = (char *)"invalid literal/length code";
                mode = BAD;
                break;
            }


            extra = (unsigned)(here.op) & 31;
            if (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));


            for (;;) {
                here = distcode[BITS(distbits)];
                if ((unsigned)(here.bits) <= bits) break;
                PULLBYTE();
            }
            if ((here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + here.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            if (here.op & 64) {
                strm->msg = (char *)"invalid distance code";
                mode = BAD;
                break;
            }
            offset = (unsigned)here.val;


            extra = (unsigned)(here.op) & 15;
            if (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            if (offset > WSIZE - (wrap ? 0: left)) {
                strm->msg = (char *)"invalid distance too far back";
                mode = BAD;
                break;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));


            do {
                ROOM();
                copy = WSIZE - offset;
                if (copy < left) {
                    from = put + copy;
                    copy = left - copy;
                }
                else {
                    from = put - offset;
                    copy = left;
                }
                if (copy > length) copy = length;
                length -= copy;
                left -= copy;
                do {
                    *put++ = *from++;
                } while (--copy);
            } while (length != 0);
            break;

        case DONE:

            ret = Z_STREAM_END;
            if (left < WSIZE) {
                if (out(out_desc, window, (unsigned)(WSIZE - left)))
                    ret = Z_BUF_ERROR;
            }
            goto inf_leave;

        case BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;

        default:
            ret = Z_STREAM_ERROR;
            goto inf_leave;
        }


  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    return ret;
}

int ZEXPORT inflateBack9End(strm)
z_stream FAR *strm;
{
    if (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        return Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    return Z_OK;
}
