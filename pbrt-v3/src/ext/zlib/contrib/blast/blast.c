



#include <setjmp.h>
#include "blast.h"

#define local static
#define MAXBITS 13
#define MAXWIN 4096


struct state {

    blast_in infun;
    void *inhow;
    unsigned char *in;
    unsigned left;
    int bitbuf;
    int bitcnt;


    jmp_buf env;


    blast_out outfun;
    void *outhow;
    unsigned next;
    int first;
    unsigned char out[MAXWIN];
};


local int bits(struct state *s, int need)
{
    int val;


    val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->left == 0) {
            s->left = s->infun(s->inhow, &(s->in));
            if (s->left == 0) longjmp(s->env, 1);
        }
        val |= (int)(*(s->in)++) << s->bitcnt;
        s->left--;
        s->bitcnt += 8;
    }


    s->bitbuf = val >> need;
    s->bitcnt -= need;


    return val & ((1 << need) - 1);
}


struct huffman {
    short *count;
    short *symbol;
};


local int decode(struct state *s, struct huffman *h)
{
    int len;
    int code;
    int first;
    int count;
    int index;
    int bitbuf;
    int left;
    short *next;

    bitbuf = s->bitbuf;
    left = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->count + 1;
    while (1) {
        while (left--) {
            code |= (bitbuf & 1) ^ 1;
            bitbuf >>= 1;
            count = *next++;
            if (code < first + count) {
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                return h->symbol[index + (code - first)];
            }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
            len++;
        }
        left = (MAXBITS+1) - len;
        if (left == 0) break;
        if (s->left == 0) {
            s->left = s->infun(s->inhow, &(s->in));
            if (s->left == 0) longjmp(s->env, 1);
        }
        bitbuf = *(s->in)++;
        s->left--;
        if (left > 8) left = 8;
    }
    return -9;
}


local int construct(struct huffman *h, const unsigned char *rep, int n)
{
    int symbol;
    int len;
    int left;
    short offs[MAXBITS+1];
    short length[256];


    symbol = 0;
    do {
        len = *rep++;
        left = (len >> 4) + 1;
        len &= 15;
        do {
            length[symbol++] = len;
        } while (--left);
    } while (--n);
    n = symbol;


    for (len = 0; len <= MAXBITS; len++)
        h->count[len] = 0;
    for (symbol = 0; symbol < n; symbol++)
        (h->count[length[symbol]])++;
    if (h->count[0] == n)
        return 0;


    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= h->count[len];
        if (left < 0) return left;
    }


    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];


    for (symbol = 0; symbol < n; symbol++)
        if (length[symbol] != 0)
            h->symbol[offs[length[symbol]]++] = symbol;


    return left;
}


local int decomp(struct state *s)
{
    int lit;
    int dict;
    int symbol;
    int len;
    unsigned dist;
    int copy;
    unsigned char *from, *to;
    static int virgin = 1;
    static short litcnt[MAXBITS+1], litsym[256];
    static short lencnt[MAXBITS+1], lensym[16];
    static short distcnt[MAXBITS+1], distsym[64];
    static struct huffman litcode = {litcnt, litsym};
    static struct huffman lencode = {lencnt, lensym};
    static struct huffman distcode = {distcnt, distsym};

    static const unsigned char litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};

    static const unsigned char lenlen[] = {2, 35, 36, 53, 38, 23};

    static const unsigned char distlen[] = {2, 20, 53, 230, 247, 151, 248};
    static const short base[16] = {
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    static const char extra[16] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};


    if (virgin) {
        construct(&litcode, litlen, sizeof(litlen));
        construct(&lencode, lenlen, sizeof(lenlen));
        construct(&distcode, distlen, sizeof(distlen));
        virgin = 0;
    }


    lit = bits(s, 8);
    if (lit > 1) return -1;
    dict = bits(s, 8);
    if (dict < 4 || dict > 6) return -2;


    do {
        if (bits(s, 1)) {

            symbol = decode(s, &lencode);
            len = base[symbol] + bits(s, extra[symbol]);
            if (len == 519) break;


            symbol = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << symbol;
            dist += bits(s, symbol);
            dist++;
            if (s->first && dist > s->next)
                return -3;


            do {
                to = s->out + s->next;
                from = to - dist;
                copy = MAXWIN;
                if (s->next < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->next;
                if (copy > len) copy = len;
                len -= copy;
                s->next += copy;
                do {
                    *to++ = *from++;
                } while (--copy);
                if (s->next == MAXWIN) {
                    if (s->outfun(s->outhow, s->out, s->next)) return 1;
                    s->next = 0;
                    s->first = 0;
                }
            } while (len != 0);
        }
        else {

            symbol = lit ? decode(s, &litcode) : bits(s, 8);
            s->out[s->next++] = symbol;
            if (s->next == MAXWIN) {
                if (s->outfun(s->outhow, s->out, s->next)) return 1;
                s->next = 0;
                s->first = 0;
            }
        }
    } while (1);
    return 0;
}


int blast(blast_in infun, void *inhow, blast_out outfun, void *outhow)
{
    struct state s;
    int err;


    s.infun = infun;
    s.inhow = inhow;
    s.left = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;


    s.outfun = outfun;
    s.outhow = outhow;
    s.next = 0;
    s.first = 1;


    if (setjmp(s.env) != 0)
        err = 2;
    else
        err = decomp(&s);


    if (err != 1 && s.next && s.outfun(s.outhow, s.out, s.next) && err == 0)
        err = 1;
    return err;
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned char **buf)
{
    static unsigned char hold[CHUNK];

    *buf = hold;
    return fread(hold, 1, CHUNK, (FILE *)how);
}

local int outf(void *how, unsigned char *buf, unsigned len)
{
    return fwrite(buf, 1, len, (FILE *)how) != len;
}


int main(void)
{
    int ret, n;


    ret = blast(inf, stdin, outf, stdout);
    if (ret != 0) fprintf(stderr, "blast error: %d\n", ret);


    n = 0;
    while (getchar() != EOF) n++;
    if (n) fprintf(stderr, "blast warning: %d unused bytes of input\n", n);


    return ret;
}
#endif
