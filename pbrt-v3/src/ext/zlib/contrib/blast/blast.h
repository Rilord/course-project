





typedef unsigned (*blast_in)(void *how, unsigned char **buf);
typedef int (*blast_out)(void *how, unsigned char *buf, unsigned len);



int blast(blast_in infun, void *inhow, blast_out outfun, void *outhow);

