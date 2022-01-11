

#include "zutil.h"
#include "inftree9.h"

#define MAXBITS 15

const char inflate9_copyright[] =
   " inflate9 1.2.8 Copyright 1995-2013 Mark Adler ";



int inflate_table9(type, lens, codes, table, bits, work)
codetype type;
unsigned short FAR *lens;
unsigned codes;
code FAR * FAR *table;
unsigned FAR *bits;
unsigned short FAR *work;
{
    unsigned len;
    unsigned sym;
    unsigned min, max;
    unsigned root;
    unsigned curr;
    unsigned drop;
    int left;
    unsigned used;
    unsigned huff;
    unsigned incr;
    unsigned fill;
    unsigned low;
    unsigned mask;
    code this;
    code FAR *next;
    const unsigned short FAR *base;
    const unsigned short FAR *extra;
    int end;
    unsigned short count[MAXBITS+1];
    unsigned short offs[MAXBITS+1];
    static const unsigned short lbase[31] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17,
        19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115,
        131, 163, 195, 227, 3, 0, 0};
    static const unsigned short lext[31] = {
        128, 128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129,
        130, 130, 130, 130, 131, 131, 131, 131, 132, 132, 132, 132,
        133, 133, 133, 133, 144, 72, 78};
    static const unsigned short dbase[32] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49,
        65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
        4097, 6145, 8193, 12289, 16385, 24577, 32769, 49153};
    static const unsigned short dext[32] = {
        128, 128, 128, 128, 129, 129, 130, 130, 131, 131, 132, 132,
        133, 133, 134, 134, 135, 135, 136, 136, 137, 137, 138, 138,
        139, 139, 140, 140, 141, 141, 142, 142};




    for (len = 0; len <= MAXBITS; len++)
        count[len] = 0;
    for (sym = 0; sym < codes; sym++)
        count[lens[sym]]++;


    root = *bits;
    for (max = MAXBITS; max >= 1; max--)
        if (count[max] != 0) break;
    if (root > max) root = max;
    if (max == 0) return -1;
    for (min = 1; min <= MAXBITS; min++)
        if (count[min] != 0) break;
    if (root < min) root = min;


    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= count[len];
        if (left < 0) return -1;
    }
    if (left > 0 && (type == CODES || max != 1))
        return -1;


    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + count[len];


    for (sym = 0; sym < codes; sym++)
        if (lens[sym] != 0) work[offs[lens[sym]]++] = (unsigned short)sym;




    switch (type) {
    case CODES:
        base = extra = work;
        end = 19;
        break;
    case LENS:
        base = lbase;
        base -= 257;
        extra = lext;
        extra -= 257;
        end = 256;
        break;
    default:
        base = dbase;
        extra = dext;
        end = -1;
    }


    huff = 0;
    sym = 0;
    len = min;
    next = *table;
    curr = root;
    drop = 0;
    low = (unsigned)(-1);
    used = 1U << root;
    mask = used - 1;


    if ((type == LENS && used >= ENOUGH_LENS) ||
        (type == DISTS && used >= ENOUGH_DISTS))
        return 1;


    for (;;) {

        this.bits = (unsigned char)(len - drop);
        if ((int)(work[sym]) < end) {
            this.op = (unsigned char)0;
            this.val = work[sym];
        }
        else if ((int)(work[sym]) > end) {
            this.op = (unsigned char)(extra[work[sym]]);
            this.val = base[work[sym]];
        }
        else {
            this.op = (unsigned char)(32 + 64);
            this.val = 0;
        }


        incr = 1U << (len - drop);
        fill = 1U << curr;
        do {
            fill -= incr;
            next[(huff >> drop) + fill] = this;
        } while (fill != 0);


        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;


        sym++;
        if (--(count[len]) == 0) {
            if (len == max) break;
            len = lens[work[sym]];
        }


        if (len > root && (huff & mask) != low) {

            if (drop == 0)
                drop = root;


            next += 1U << curr;


            curr = len - drop;
            left = (int)(1 << curr);
            while (curr + drop < max) {
                left -= count[curr + drop];
                if (left <= 0) break;
                curr++;
                left <<= 1;
            }


            used += 1U << curr;
            if ((type == LENS && used >= ENOUGH_LENS) ||
                (type == DISTS && used >= ENOUGH_DISTS))
                return 1;


            low = huff & mask;
            (*table)[low].op = (unsigned char)curr;
            (*table)[low].bits = (unsigned char)root;
            (*table)[low].val = (unsigned short)(next - *table);
        }
    }


    this.op = (unsigned char)64;
    this.bits = (unsigned char)(len - drop);
    this.val = (unsigned short)0;
    while (huff != 0) {

        if (drop != 0 && (huff & mask) != low) {
            drop = 0;
            len = root;
            next = *table;
            curr = root;
            this.bits = (unsigned char)len;
        }


        next[huff >> drop] = this;


        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;
    }


    *table += used;
    *bits = root;
    return 0;
}
