




typedef enum {
        TYPE,
        STORED,
        TABLE,
            LEN,
    DONE,
    BAD
} inflate_mode;




struct inflate_state {

    unsigned char FAR *window;

    unsigned ncode;
    unsigned nlen;
    unsigned ndist;
    unsigned have;
    code FAR *next;
    unsigned short lens[320];
    unsigned short work[288];
    code codes[ENOUGH];
};
