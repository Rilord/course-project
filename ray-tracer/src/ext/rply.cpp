
// ext/rply.cpp*

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>

#include "ext/rply.h"


#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef __int8 t_ply_int8;
typedef __int16 t_ply_int16;
typedef __int32 t_ply_int32;
typedef unsigned __int8 t_ply_uint8;
typedef unsigned __int16 t_ply_uint16;
typedef unsigned __int32 t_ply_uint32;
#define PLY_INT8_MAX (127)
#define PLY_INT8_MIN (-PLY_INT8_MAX - 1)
#define PLY_INT16_MAX (32767)
#define PLY_INT16_MIN (-PLY_INT16_MAX - 1)
#define PLY_INT32_MAX (2147483647)
#define PLY_INT32_MIN (-PLY_INT32_MAX - 1)
#define PLY_UINT8_MAX (255)
#define PLY_UINT16_MAX (65535)
#define PLY_UINT32_MAX (4294967295)
#else
#define __STDC_LIMIT_MACROS
#include <stdint.h>
typedef int8_t t_ply_int8;
typedef int16_t t_ply_int16;
typedef int32_t t_ply_int32;
typedef uint8_t t_ply_uint8;
typedef uint16_t t_ply_uint16;
typedef uint32_t t_ply_uint32;
#define PLY_INT8_MIN INT8_MIN
#define PLY_INT8_MAX INT8_MAX
#define PLY_INT16_MIN INT16_MIN
#define PLY_INT16_MAX INT16_MAX
#define PLY_INT32_MIN INT32_MIN
#define PLY_INT32_MAX INT32_MAX
#define PLY_UINT8_MAX UINT8_MAX
#define PLY_UINT16_MAX UINT16_MAX
#define PLY_UINT32_MAX UINT32_MAX
#endif


#define WORDSIZE 256
#define LINESIZE 1024
#define BUFFERSIZE (8 * 1024)

typedef enum e_ply_io_mode_ { PLY_READ, PLY_WRITE } e_ply_io_mode;

static const char *const ply_storage_mode_list[] = {
    "binary_big_endian", "binary_little_endian", "ascii",
    NULL};

static const char *const ply_type_list[] = {
    "int8",    "uint8",  "int16", "uint16", "int32",  "uint32", "float32",
    "float64", "char",   "uchar", "short",  "ushort", "int",    "uint",
    "float",   "double", "list",  NULL};


typedef struct t_ply_argument_ {
    p_ply_element element;
    long instance_index;
    p_ply_property property;
    long length, value_index;
    double value;
    void *pdata;
    long idata;
} t_ply_argument;


typedef struct t_ply_property_ {
    char name[WORDSIZE];
    e_ply_type type, value_type, length_type;
    p_ply_read_cb read_cb;
    void *pdata;
    long idata;
} t_ply_property;


typedef struct t_ply_element_ {
    char name[WORDSIZE];
    long ninstances;
    p_ply_property property;
    long nproperties;
} t_ply_element;


typedef int (*p_ply_ihandler)(p_ply ply, double *value);
typedef int (*p_ply_ichunk)(p_ply ply, void *anydata, size_t size);
typedef struct t_ply_idriver_ {
    p_ply_ihandler ihandler[16];
    p_ply_ichunk ichunk;
    const char *name;
} t_ply_idriver;
typedef t_ply_idriver *p_ply_idriver;

typedef int (*p_ply_ohandler)(p_ply ply, double value);
typedef int (*p_ply_ochunk)(p_ply ply, void *anydata, size_t size);
typedef struct t_ply_odriver_ {
    p_ply_ohandler ohandler[16];
    p_ply_ochunk ochunk;
    const char *name;
} t_ply_odriver;
typedef t_ply_odriver *p_ply_odriver;


typedef struct t_ply_ {
    e_ply_io_mode io_mode;
    e_ply_storage_mode storage_mode;
    p_ply_element element;
    long nelements;
    char *comment;
    long ncomments;
    char *obj_info;
    long nobj_infos;
    FILE *fp;
    int rn;
    char buffer[BUFFERSIZE];
    size_t buffer_first, buffer_token, buffer_last;
    p_ply_idriver idriver;
    p_ply_odriver odriver;
    t_ply_argument argument;
    long welement, wproperty;
    long winstance_index, wvalue_index, wlength;
    p_ply_error_cb error_cb;
    void *pdata;
    long idata;
} t_ply;


namespace {
extern t_ply_idriver ply_idriver_ascii;
extern t_ply_idriver ply_idriver_binary;
extern t_ply_idriver ply_idriver_binary_reverse;
extern t_ply_odriver ply_odriver_ascii;
extern t_ply_odriver ply_odriver_binary;
extern t_ply_odriver ply_odriver_binary_reverse;
};

static int ply_read_word(p_ply ply);
static int ply_check_word(p_ply ply);
static void ply_finish_word(p_ply ply, size_t size);
static int ply_read_line(p_ply ply);
static int ply_check_line(p_ply ply);
static int ply_read_chunk(p_ply ply, void *anybuffer, size_t size);
static int ply_read_chunk_reverse(p_ply ply, void *anybuffer, size_t size);
static int ply_write_chunk(p_ply ply, void *anybuffer, size_t size);
static int ply_write_chunk_reverse(p_ply ply, void *anybuffer, size_t size);
static void ply_reverse(void *anydata, size_t size);


static int ply_find_string(const char *item, const char *const list[]);
static p_ply_element ply_find_element(p_ply ply, const char *name);
static p_ply_property ply_find_property(p_ply_element element,
                                        const char *name);


static int ply_read_header_magic(p_ply ply);
static int ply_read_header_format(p_ply ply);
static int ply_read_header_comment(p_ply ply);
static int ply_read_header_obj_info(p_ply ply);
static int ply_read_header_property(p_ply ply);
static int ply_read_header_element(p_ply ply);


static void ply_error_cb(p_ply ply, const char *message);
static void ply_ferror(p_ply ply, const char *fmt, ...);


static void ply_init(p_ply ply);
static void ply_element_init(p_ply_element element);
static void ply_property_init(p_ply_property property);
static p_ply ply_alloc(void);
static p_ply_element ply_grow_element(p_ply ply);
static p_ply_property ply_grow_property(p_ply ply, p_ply_element element);
static void *ply_grow_array(p_ply ply, void **pointer, long *nmemb, long size);


static e_ply_storage_mode ply_arch_endian(void);
static int ply_type_check(void);


static int ply_read_element(p_ply ply, p_ply_element element,
                            p_ply_argument argument);
static int ply_read_property(p_ply ply, p_ply_element element,
                             p_ply_property property, p_ply_argument argument);
static int ply_read_list_property(p_ply ply, p_ply_element element,
                                  p_ply_property property,
                                  p_ply_argument argument);
static int ply_read_scalar_property(p_ply ply, p_ply_element element,
                                    p_ply_property property,
                                    p_ply_argument argument);



#define BWORD(p) (p->buffer + p->buffer_token)
#define BLINE(p) (p->buffer + p->buffer_token)


#define BFIRST(p) (p->buffer + p->buffer_first)


#define BSIZE(p) (p->buffer_last - p->buffer_first)


#define BSKIP(p, s) (p->buffer_first += s)


static int BREFILL(p_ply ply) {

    size_t size = BSIZE(ply);
    memmove(ply->buffer, BFIRST(ply), size);
    ply->buffer_last = size;
    ply->buffer_first = ply->buffer_token = 0;

    size = fread(ply->buffer + size, 1, BUFFERSIZE - size - 1, ply->fp);

    ply->buffer[BUFFERSIZE - 1] = '\0';

    if (size <= 0) return 0;

    ply->buffer_last += size;
    return 1;
}



static int ply_read_header_magic(p_ply ply) {
    char *magic = ply->buffer;
    if (!BREFILL(ply)) {
        ply->error_cb(ply, "Unable to read magic number from file");
        return 0;
    }

    if (magic[0] != 'p' || magic[1] != 'l' || magic[2] != 'y' ||
        !isspace(magic[3])) {
        ply->error_cb(ply, "Wrong magic number. Expected 'ply'");
        return 0;
    }

    ply->rn = magic[3] == '\r' && magic[4] == '\n';
    BSKIP(ply, 3);
    return 1;
}



p_ply ply_open(const char *name, p_ply_error_cb error_cb, long idata,
               void *pdata) {
    FILE *fp = NULL;
    p_ply ply = ply_alloc();
    if (error_cb == NULL) error_cb = ply_error_cb;
    if (!ply) {
        error_cb(NULL, "Out of memory");
        return NULL;
    }
    ply->idata = idata;
    ply->pdata = pdata;
    ply->io_mode = PLY_READ;
    ply->error_cb = error_cb;
    if (!ply_type_check()) {
        error_cb(ply, "Incompatible type system");
        free(ply);
        return NULL;
    }
    assert(name);
    fp = fopen(name, "rb");
    if (!fp) {
        error_cb(ply, "Unable to open file");
        free(ply);
        return NULL;
    }
    ply->fp = fp;
    return ply;
}

int ply_read_header(p_ply ply) {
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    if (!ply_read_header_magic(ply)) return 0;
    if (!ply_read_word(ply)) return 0;

    if (!ply_read_header_format(ply)) {
        ply_ferror(ply, "Invalid file format");
        return 0;
    }

    while (strcmp(BWORD(ply), "end_header")) {
        if (!ply_read_header_comment(ply) && !ply_read_header_element(ply) &&
            !ply_read_header_obj_info(ply)) {
            ply_ferror(ply, "Unexpected token '%s'", BWORD(ply));
            return 0;
        }
    }

    if (ply->rn) {
        if (BSIZE(ply) < 1 && !BREFILL(ply)) {
            ply_ferror(ply, "Unexpected end of file");
            return 0;
        }
        BSKIP(ply, 1);
    }
    return 1;
}

long ply_set_read_cb(p_ply ply, const char *element_name,
                     const char *property_name, p_ply_read_cb read_cb,
                     void *pdata, long idata) {
    p_ply_element element = NULL;
    p_ply_property property = NULL;
    assert(ply && element_name && property_name);
    element = ply_find_element(ply, element_name);
    if (!element) return 0;
    property = ply_find_property(element, property_name);
    if (!property) return 0;
    property->read_cb = read_cb;
    property->pdata = pdata;
    property->idata = idata;
    return (int)element->ninstances;
}

int ply_read(p_ply ply) {
    long i;
    p_ply_argument argument;
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    argument = &ply->argument;

    for (i = 0; i < ply->nelements; i++) {
        p_ply_element element = &ply->element[i];
        argument->element = element;
        if (!ply_read_element(ply, element, argument)) return 0;
    }
    return 1;
}


p_ply ply_create(const char *name, e_ply_storage_mode storage_mode,
                 p_ply_error_cb error_cb, long idata, void *pdata) {
    FILE *fp = NULL;
    p_ply ply = ply_alloc();
    if (error_cb == NULL) error_cb = ply_error_cb;
    if (!ply) {
        error_cb(NULL, "Out of memory");
        return NULL;
    }
    if (!ply_type_check()) {
        error_cb(ply, "Incompatible type system");
        free(ply);
        return NULL;
    }
    assert(name && storage_mode <= PLY_DEFAULT);
    fp = fopen(name, "wb");
    if (!fp) {
        error_cb(ply, "Unable to create file");
        free(ply);
        return NULL;
    }
    ply->idata = idata;
    ply->pdata = pdata;
    ply->io_mode = PLY_WRITE;
    if (storage_mode == PLY_DEFAULT) storage_mode = ply_arch_endian();
    if (storage_mode == PLY_ASCII)
        ply->odriver = &ply_odriver_ascii;
    else if (storage_mode == ply_arch_endian())
        ply->odriver = &ply_odriver_binary;
    else
        ply->odriver = &ply_odriver_binary_reverse;
    ply->storage_mode = storage_mode;
    ply->fp = fp;
    ply->error_cb = error_cb;
    return ply;
}

int ply_add_element(p_ply ply, const char *name, long ninstances) {
    p_ply_element element = NULL;
    assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
    assert(name && strlen(name) < WORDSIZE && ninstances >= 0);
    if (strlen(name) >= WORDSIZE || ninstances < 0) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    element = ply_grow_element(ply);
    if (!element) return 0;
    strcpy(element->name, name);
    element->ninstances = ninstances;
    return 1;
}

int ply_add_scalar_property(p_ply ply, const char *name, e_ply_type type) {
    p_ply_element element = NULL;
    p_ply_property property = NULL;
    assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
    assert(name && strlen(name) < WORDSIZE);
    assert(type < PLY_LIST);
    if (strlen(name) >= WORDSIZE || type >= PLY_LIST) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    element = &ply->element[ply->nelements - 1];
    property = ply_grow_property(ply, element);
    if (!property) return 0;
    strcpy(property->name, name);
    property->type = type;
    return 1;
}

int ply_add_list_property(p_ply ply, const char *name, e_ply_type length_type,
                          e_ply_type value_type) {
    p_ply_element element = NULL;
    p_ply_property property = NULL;
    assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
    assert(name && strlen(name) < WORDSIZE);
    if (strlen(name) >= WORDSIZE) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    assert(length_type < PLY_LIST);
    assert(value_type < PLY_LIST);
    if (length_type >= PLY_LIST || value_type >= PLY_LIST) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    element = &ply->element[ply->nelements - 1];
    property = ply_grow_property(ply, element);
    if (!property) return 0;
    strcpy(property->name, name);
    property->type = PLY_LIST;
    property->length_type = length_type;
    property->value_type = value_type;
    return 1;
}

int ply_add_property(p_ply ply, const char *name, e_ply_type type,
                     e_ply_type length_type, e_ply_type value_type) {
    if (type == PLY_LIST)
        return ply_add_list_property(ply, name, length_type, value_type);
    else
        return ply_add_scalar_property(ply, name, type);
}

int ply_add_comment(p_ply ply, const char *comment) {
    char *new_comment = NULL;
    assert(ply && comment && strlen(comment) < LINESIZE);
    if (!comment || strlen(comment) >= LINESIZE) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    new_comment = (char *)ply_grow_array(ply, (void **)&ply->comment,
                                         &ply->ncomments, LINESIZE);
    if (!new_comment) return 0;
    strcpy(new_comment, comment);
    return 1;
}

int ply_add_obj_info(p_ply ply, const char *obj_info) {
    char *new_obj_info = NULL;
    assert(ply && obj_info && strlen(obj_info) < LINESIZE);
    if (!obj_info || strlen(obj_info) >= LINESIZE) {
        ply_ferror(ply, "Invalid arguments");
        return 0;
    }
    new_obj_info = (char *)ply_grow_array(ply, (void **)&ply->obj_info,
                                          &ply->nobj_infos, LINESIZE);
    if (!new_obj_info) return 0;
    strcpy(new_obj_info, obj_info);
    return 1;
}

int ply_write_header(p_ply ply) {
    long i, j;
    assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
    assert(ply->element || ply->nelements == 0);
    assert(!ply->element || ply->nelements > 0);
    if (fprintf(ply->fp, "ply\nformat %s 1.0\n",
                ply_storage_mode_list[ply->storage_mode]) <= 0)
        goto error;
    for (i = 0; i < ply->ncomments; i++)
        if (fprintf(ply->fp, "comment %s\n", ply->comment + LINESIZE * i) <= 0)
            goto error;
    for (i = 0; i < ply->nobj_infos; i++)
        if (fprintf(ply->fp, "obj_info %s\n", ply->obj_info + LINESIZE * i) <=
            0)
            goto error;
    for (i = 0; i < ply->nelements; i++) {
        p_ply_element element = &ply->element[i];
        assert(element->property || element->nproperties == 0);
        assert(!element->property || element->nproperties > 0);
        if (fprintf(ply->fp, "element %s %ld\n", element->name,
                    element->ninstances) <= 0)
            goto error;
        for (j = 0; j < element->nproperties; j++) {
            p_ply_property property = &element->property[j];
            if (property->type == PLY_LIST) {
                if (fprintf(ply->fp, "property list %s %s %s\n",
                            ply_type_list[property->length_type],
                            ply_type_list[property->value_type],
                            property->name) <= 0)
                    goto error;
            } else {
                if (fprintf(ply->fp, "property %s %s\n",
                            ply_type_list[property->type], property->name) <= 0)
                    goto error;
            }
        }
    }
    return fprintf(ply->fp, "end_header\n") > 0;
error:
    ply_ferror(ply, "Error writing to file");
    return 0;
}

int ply_write(p_ply ply, double value) {
    p_ply_element element = NULL;
    p_ply_property property = NULL;
    int type = -1;
    int breakafter = 0;
    int spaceafter = 1;
    if (ply->welement > ply->nelements) return 0;
    element = &ply->element[ply->welement];
    if (ply->wproperty > element->nproperties) return 0;
    property = &element->property[ply->wproperty];
    if (property->type == PLY_LIST) {
        if (ply->wvalue_index == 0) {
            type = property->length_type;
            ply->wlength = (long)value;
        } else
            type = property->value_type;
    } else {
        type = property->type;
        ply->wlength = 0;
    }
    if (!ply->odriver->ohandler[type](ply, value)) {
        ply_ferror(ply, "Failed writing %s of %s %d (%s: %s)", property->name,
                   element->name, ply->winstance_index, ply->odriver->name,
                   ply_type_list[type]);
        return 0;
    }
    ply->wvalue_index++;
    if (ply->wvalue_index > ply->wlength) {
        ply->wvalue_index = 0;
        ply->wproperty++;
    }
    if (ply->wproperty >= element->nproperties) {
        ply->wproperty = 0;
        ply->winstance_index++;
        breakafter = 1;
        spaceafter = 0;
    }
    if (ply->winstance_index >= element->ninstances) {
        ply->winstance_index = 0;
        do {
            ply->welement++;
            element = &ply->element[ply->welement];
        } while (ply->welement < ply->nelements && !element->ninstances);
    }
    if (ply->storage_mode == PLY_ASCII) {
        return (!spaceafter || putc(' ', ply->fp) > 0) &&
               (!breakafter || putc('\n', ply->fp) > 0);
    } else {
        return 1;
    }
}

int ply_close(p_ply ply) {
    long i;
    assert(ply && ply->fp);
    assert(ply->element || ply->nelements == 0);
    assert(!ply->element || ply->nelements > 0);

    if (ply->io_mode == PLY_WRITE &&
        fwrite(ply->buffer, 1, ply->buffer_last, ply->fp) < ply->buffer_last) {
        ply_ferror(ply, "Error closing up");
        return 0;
    }
    fclose(ply->fp);

    if (ply->element) {
        for (i = 0; i < ply->nelements; i++) {
            p_ply_element element = &ply->element[i];
            if (element->property) free(element->property);
        }
        free(ply->element);
    }
    if (ply->obj_info) free(ply->obj_info);
    if (ply->comment) free(ply->comment);
    free(ply);
    return 1;
}


p_ply_element ply_get_next_element(p_ply ply, p_ply_element last) {
    assert(ply);
    if (!last) return ply->element;
    last++;
    if (last < ply->element + ply->nelements)
        return last;
    else
        return NULL;
}

int ply_get_element_info(p_ply_element element, const char **name,
                         long *ninstances) {
    assert(element);
    if (name) *name = element->name;
    if (ninstances) *ninstances = (long)element->ninstances;
    return 1;
}

p_ply_property ply_get_next_property(p_ply_element element,
                                     p_ply_property last) {
    assert(element);
    if (!last) return element->property;
    last++;
    if (last < element->property + element->nproperties)
        return last;
    else
        return NULL;
}

int ply_get_property_info(p_ply_property property, const char **name,
                          e_ply_type *type, e_ply_type *length_type,
                          e_ply_type *value_type) {
    assert(property);
    if (name) *name = property->name;
    if (type) *type = property->type;
    if (length_type) *length_type = property->length_type;
    if (value_type) *value_type = property->value_type;
    return 1;
}

const char *ply_get_next_comment(p_ply ply, const char *last) {
    assert(ply);
    if (!last) return ply->comment;
    last += LINESIZE;
    if (last < ply->comment + LINESIZE * ply->ncomments)
        return last;
    else
        return NULL;
}

const char *ply_get_next_obj_info(p_ply ply, const char *last) {
    assert(ply);
    if (!last) return ply->obj_info;
    last += LINESIZE;
    if (last < ply->obj_info + LINESIZE * ply->nobj_infos)
        return last;
    else
        return NULL;
}


int ply_get_argument_element(p_ply_argument argument, p_ply_element *element,
                             long *instance_index) {
    assert(argument);
    if (!argument) return 0;
    if (element) *element = argument->element;
    if (instance_index) *instance_index = argument->instance_index;
    return 1;
}

int ply_get_argument_property(p_ply_argument argument, p_ply_property *property,
                              long *length, long *value_index) {
    assert(argument);
    if (!argument) return 0;
    if (property) *property = argument->property;
    if (length) *length = argument->length;
    if (value_index) *value_index = argument->value_index;
    return 1;
}

int ply_get_argument_user_data(p_ply_argument argument, void **pdata,
                               long *idata) {
    assert(argument);
    if (!argument) return 0;
    if (pdata) *pdata = argument->pdata;
    if (idata) *idata = argument->idata;
    return 1;
}

double ply_get_argument_value(p_ply_argument argument) {
    assert(argument);
    if (!argument) return 0.0;
    return argument->value;
}

int ply_get_ply_user_data(p_ply ply, void **pdata, long *idata) {
    assert(ply);
    if (!ply) return 0;
    if (pdata) *pdata = ply->pdata;
    if (idata) *idata = ply->idata;
    return 1;
}


static int ply_read_list_property(p_ply ply, p_ply_element element,
                                  p_ply_property property,
                                  p_ply_argument argument) {
    int l;
    p_ply_read_cb read_cb = property->read_cb;
    p_ply_ihandler *driver = ply->idriver->ihandler;

    p_ply_ihandler handler = driver[property->length_type];
    double length;
    if (!handler(ply, &length)) {
        ply_ferror(ply, "Error reading '%s' of '%s' number %d", property->name,
                   element->name, argument->instance_index);
        return 0;
    }

    argument->length = (long)length;
    argument->value_index = -1;
    argument->value = length;
    if (read_cb && !read_cb(argument)) {
        ply_ferror(ply, "Aborted by user");
        return 0;
    }

    handler = driver[property->value_type];

    for (l = 0; l < (long)length; l++) {

        argument->value_index = l;
        if (!handler(ply, &argument->value)) {
            ply_ferror(ply,
                       "Error reading value number %d of '%s' of "
                       "'%s' number %d",
                       l + 1, property->name, element->name,
                       argument->instance_index);
            return 0;
        }

        if (read_cb && !read_cb(argument)) {
            ply_ferror(ply, "Aborted by user");
            return 0;
        }
    }
    return 1;
}

static int ply_read_scalar_property(p_ply ply, p_ply_element element,
                                    p_ply_property property,
                                    p_ply_argument argument) {
    p_ply_read_cb read_cb = property->read_cb;
    p_ply_ihandler *driver = ply->idriver->ihandler;
    p_ply_ihandler handler = driver[property->type];
    argument->length = 1;
    argument->value_index = 0;
    if (!handler(ply, &argument->value)) {
        ply_ferror(ply, "Error reading '%s' of '%s' number %d", property->name,
                   element->name, argument->instance_index);
        return 0;
    }
    if (read_cb && !read_cb(argument)) {
        ply_ferror(ply, "Aborted by user");
        return 0;
    }
    return 1;
}

static int ply_read_property(p_ply ply, p_ply_element element,
                             p_ply_property property, p_ply_argument argument) {
    if (property->type == PLY_LIST)
        return ply_read_list_property(ply, element, property, argument);
    else
        return ply_read_scalar_property(ply, element, property, argument);
}

static int ply_read_element(p_ply ply, p_ply_element element,
                            p_ply_argument argument) {
    long j, k;

    for (j = 0; j < element->ninstances; j++) {
        argument->instance_index = j;

        for (k = 0; k < element->nproperties; k++) {
            p_ply_property property = &element->property[k];
            argument->property = property;
            argument->pdata = property->pdata;
            argument->idata = property->idata;
            if (!ply_read_property(ply, element, property, argument)) return 0;
        }
    }
    return 1;
}

static int ply_find_string(const char *item, const char *const list[]) {
    int i;
    assert(item && list);
    for (i = 0; list[i]; i++)
        if (!strcmp(list[i], item)) return i;
    return -1;
}

static p_ply_element ply_find_element(p_ply ply, const char *name) {
    p_ply_element element;
    int i, nelements;
    assert(ply && name);
    element = ply->element;
    nelements = ply->nelements;
    assert(element || nelements == 0);
    assert(!element || nelements > 0);
    for (i = 0; i < nelements; i++)
        if (!strcmp(element[i].name, name)) return &element[i];
    return NULL;
}

static p_ply_property ply_find_property(p_ply_element element,
                                        const char *name) {
    p_ply_property property;
    int i, nproperties;
    assert(element && name);
    property = element->property;
    nproperties = element->nproperties;
    assert(property || nproperties == 0);
    assert(!property || nproperties > 0);
    for (i = 0; i < nproperties; i++)
        if (!strcmp(property[i].name, name)) return &property[i];
    return NULL;
}

static int ply_check_word(p_ply ply) {
    size_t size = strlen(BWORD(ply));
    if (size >= WORDSIZE) {
        ply_ferror(ply, "Word too long");
        return 0;
    } else if (size == 0) {
        ply_ferror(ply, "Unexpected end of file");
        return 0;
    }
    return 1;
}

static int ply_read_word(p_ply ply) {
    size_t t = 0;
    assert(ply && ply->fp && ply->io_mode == PLY_READ);

    while (1) {
        t = strspn(BFIRST(ply), " \n\r\t");

        if (t >= BSIZE(ply)) {
            if (!BREFILL(ply)) {
                ply_ferror(ply, "Unexpected end of file");
                return 0;
            }
        } else
            break;
    }
    BSKIP(ply, t);

    t = strcspn(BFIRST(ply), " \n\r\t");

    if (t < BSIZE(ply)) {
        ply_finish_word(ply, t);
        return ply_check_word(ply);
    }

    if (!BREFILL(ply)) {

        ply_finish_word(ply, t);
        return ply_check_word(ply);


    }

    t += strcspn(BFIRST(ply) + t, " \n\r\t");

    if (t >= BSIZE(ply)) {
        ply_ferror(ply, "Token too large");
        return 0;
    }

    ply_finish_word(ply, t);
    return ply_check_word(ply);
}

static void ply_finish_word(p_ply ply, size_t size) {
    ply->buffer_token = ply->buffer_first;
    BSKIP(ply, size);
    *BFIRST(ply) = '\0';
    BSKIP(ply, 1);
}

static int ply_check_line(p_ply ply) {
    if (strlen(BLINE(ply)) >= LINESIZE) {
        ply_ferror(ply, "Line too long");
        return 0;
    }
    return 1;
}

static int ply_read_line(p_ply ply) {
    const char *end = NULL;
    assert(ply && ply->fp && ply->io_mode == PLY_READ);

    end = strchr(BFIRST(ply), '\n');

    if (end) {
        ply->buffer_token = ply->buffer_first;
        BSKIP(ply, end - BFIRST(ply));
        *BFIRST(ply) = '\0';
        BSKIP(ply, 1);
        return ply_check_line(ply);
    } else {
        end = ply->buffer + BSIZE(ply);

        if (!BREFILL(ply)) {
            ply_ferror(ply, "Unexpected end of file");
            return 0;
        }
    }

    end = strchr(end, '\n');

    if (!end) {
        ply_ferror(ply, "Token too large");
        return 0;
    }

    ply->buffer_token = ply->buffer_first;
    BSKIP(ply, end - BFIRST(ply));
    *BFIRST(ply) = '\0';
    BSKIP(ply, 1);
    return ply_check_line(ply);
}

static int ply_read_chunk(p_ply ply, void *anybuffer, size_t size) {
    char *buffer = (char *)anybuffer;
    size_t i = 0;
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    assert(ply->buffer_first <= ply->buffer_last);
    while (i < size) {
        if (ply->buffer_first < ply->buffer_last) {
            buffer[i] = ply->buffer[ply->buffer_first];
            ply->buffer_first++;
            i++;
        } else {
            ply->buffer_first = 0;
            ply->buffer_last = fread(ply->buffer, 1, BUFFERSIZE, ply->fp);
            if (ply->buffer_last <= 0) return 0;
        }
    }
    return 1;
}

static int ply_write_chunk(p_ply ply, void *anybuffer, size_t size) {
    char *buffer = (char *)anybuffer;
    size_t i = 0;
    assert(ply && ply->fp && ply->io_mode == PLY_WRITE);
    assert(ply->buffer_last <= BUFFERSIZE);
    while (i < size) {
        if (ply->buffer_last < BUFFERSIZE) {
            ply->buffer[ply->buffer_last] = buffer[i];
            ply->buffer_last++;
            i++;
        } else {
            ply->buffer_last = 0;
            if (fwrite(ply->buffer, 1, BUFFERSIZE, ply->fp) < BUFFERSIZE)
                return 0;
        }
    }
    return 1;
}

static int ply_write_chunk_reverse(p_ply ply, void *anybuffer, size_t size) {
    int ret = 0;
    ply_reverse(anybuffer, size);
    ret = ply_write_chunk(ply, anybuffer, size);
    ply_reverse(anybuffer, size);
    return ret;
}

static int ply_read_chunk_reverse(p_ply ply, void *anybuffer, size_t size) {
    if (!ply_read_chunk(ply, anybuffer, size)) return 0;
    ply_reverse(anybuffer, size);
    return 1;
}

static void ply_reverse(void *anydata, size_t size) {
    char *data = (char *)anydata;
    char temp;
    size_t i;
    for (i = 0; i < size / 2; i++) {
        temp = data[i];
        data[i] = data[size - i - 1];
        data[size - i - 1] = temp;
    }
}

static void ply_init(p_ply ply) {
    ply->element = NULL;
    ply->nelements = 0;
    ply->comment = NULL;
    ply->ncomments = 0;
    ply->obj_info = NULL;
    ply->nobj_infos = 0;
    ply->idriver = NULL;
    ply->odriver = NULL;
    ply->buffer[0] = '\0';
    ply->buffer_first = ply->buffer_last = ply->buffer_token = 0;
    ply->welement = 0;
    ply->wproperty = 0;
    ply->winstance_index = 0;
    ply->wlength = 0;
    ply->wvalue_index = 0;
}

static void ply_element_init(p_ply_element element) {
    element->name[0] = '\0';
    element->ninstances = 0;
    element->property = NULL;
    element->nproperties = 0;
}

static void ply_property_init(p_ply_property property) {
    property->name[0] = '\0';
    property->type = (e_ply_type)-1;
    property->length_type = (e_ply_type)-1;
    property->value_type = (e_ply_type)-1;
    property->read_cb = (p_ply_read_cb)NULL;
    property->pdata = NULL;
    property->idata = 0;
}

static p_ply ply_alloc(void) {
    p_ply ply = (p_ply)calloc(1, sizeof(t_ply));
    if (!ply) return NULL;
    ply_init(ply);
    return ply;
}

static void *ply_grow_array(p_ply ply, void **pointer, long *nmemb, long size) {
    void *temp = *pointer;
    long count = *nmemb + 1;
    if (!temp)
        temp = malloc(count * size);
    else
        temp = realloc(temp, count * size);
    if (!temp) {
        ply_ferror(ply, "Out of memory");
        return NULL;
    }
    *pointer = temp;
    *nmemb = count;
    return (char *)temp + (count - 1) * size;
}

static p_ply_element ply_grow_element(p_ply ply) {
    p_ply_element element = NULL;
    assert(ply);
    assert(ply->element || ply->nelements == 0);
    assert(!ply->element || ply->nelements > 0);
    element = (p_ply_element)ply_grow_array(
        ply, (void **)&ply->element, &ply->nelements, sizeof(t_ply_element));
    if (!element) return NULL;
    ply_element_init(element);
    return element;
}

static p_ply_property ply_grow_property(p_ply ply, p_ply_element element) {
    p_ply_property property = NULL;
    assert(ply);
    assert(element);
    assert(element->property || element->nproperties == 0);
    assert(!element->property || element->nproperties > 0);
    property = (p_ply_property)ply_grow_array(ply, (void **)&element->property,
                                              &element->nproperties,
                                              sizeof(t_ply_property));
    if (!property) return NULL;
    ply_property_init(property);
    return property;
}

static int ply_read_header_format(p_ply ply) {
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    if (strcmp(BWORD(ply), "format")) return 0;
    if (!ply_read_word(ply)) return 0;
    ply->storage_mode =
        (e_ply_storage_mode)ply_find_string(BWORD(ply), ply_storage_mode_list);
    if (ply->storage_mode == (e_ply_storage_mode)(-1)) return 0;
    if (ply->storage_mode == PLY_ASCII)
        ply->idriver = &ply_idriver_ascii;
    else if (ply->storage_mode == ply_arch_endian())
        ply->idriver = &ply_idriver_binary;
    else
        ply->idriver = &ply_idriver_binary_reverse;
    if (!ply_read_word(ply)) return 0;
    if (strcmp(BWORD(ply), "1.0")) return 0;
    if (!ply_read_word(ply)) return 0;
    return 1;
}

static int ply_read_header_comment(p_ply ply) {
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    if (strcmp(BWORD(ply), "comment")) return 0;
    if (!ply_read_line(ply)) return 0;
    if (!ply_add_comment(ply, BLINE(ply))) return 0;
    if (!ply_read_word(ply)) return 0;
    return 1;
}

static int ply_read_header_obj_info(p_ply ply) {
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    if (strcmp(BWORD(ply), "obj_info")) return 0;
    if (!ply_read_line(ply)) return 0;
    if (!ply_add_obj_info(ply, BLINE(ply))) return 0;
    if (!ply_read_word(ply)) return 0;
    return 1;
}

static int ply_read_header_property(p_ply ply) {
    p_ply_element element = NULL;
    p_ply_property property = NULL;

    if (strcmp(BWORD(ply), "property")) return 0;
    element = &ply->element[ply->nelements - 1];
    property = ply_grow_property(ply, element);
    if (!property) return 0;

    if (!ply_read_word(ply)) return 0;
    property->type = (e_ply_type)ply_find_string(BWORD(ply), ply_type_list);
    if (property->type == (e_ply_type)(-1)) return 0;
    if (property->type == PLY_LIST) {

        if (!ply_read_word(ply)) return 0;
        property->length_type =
            (e_ply_type)ply_find_string(BWORD(ply), ply_type_list);
        if (property->length_type == (e_ply_type)(-1)) return 0;
        if (!ply_read_word(ply)) return 0;
        property->value_type =
            (e_ply_type)ply_find_string(BWORD(ply), ply_type_list);
        if (property->value_type == (e_ply_type)(-1)) return 0;
    }

    if (!ply_read_word(ply)) return 0;
    strcpy(property->name, BWORD(ply));
    if (!ply_read_word(ply)) return 0;
    return 1;
}

static int ply_read_header_element(p_ply ply) {
    p_ply_element element = NULL;
    long dummy;
    assert(ply && ply->fp && ply->io_mode == PLY_READ);
    if (strcmp(BWORD(ply), "element")) return 0;

    element = ply_grow_element(ply);
    if (!element) return 0;

    if (!ply_read_word(ply)) return 0;
    strcpy(element->name, BWORD(ply));

    if (!ply_read_word(ply)) return 0;
    if (sscanf(BWORD(ply), "%ld", &dummy) != 1) {
        ply_ferror(ply, "Expected number got '%s'", BWORD(ply));
        return 0;
    }
    element->ninstances = dummy;

    if (!ply_read_word(ply)) return 0;
    while (ply_read_header_property(ply) || ply_read_header_comment(ply) ||
           ply_read_header_obj_info(ply))
        ;
    return 1;
}

static void ply_error_cb(p_ply ply, const char *message) {
    (void)ply;
    fprintf(stderr, "RPly: %s\n", message);
}

static void ply_ferror(p_ply ply, const char *fmt, ...) {
    char buffer[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);
    ply->error_cb(ply, buffer);
}

static e_ply_storage_mode ply_arch_endian(void) {
    unsigned long i = 1;
    unsigned char *s = (unsigned char *)&i;
    if (*s == 1)
        return PLY_LITTLE_ENDIAN;
    else
        return PLY_BIG_ENDIAN;
}

static int ply_type_check(void) {
    assert(sizeof(t_ply_int8) == 1);
    assert(sizeof(t_ply_uint8) == 1);
    assert(sizeof(t_ply_int16) == 2);
    assert(sizeof(t_ply_uint16) == 2);
    assert(sizeof(t_ply_int32) == 4);
    assert(sizeof(t_ply_uint32) == 4);
    assert(sizeof(float) == 4);
    assert(sizeof(double) == 8);
    if (sizeof(t_ply_int8) != 1) return 0;
    if (sizeof(t_ply_uint8) != 1) return 0;
    if (sizeof(t_ply_int16) != 2) return 0;
    if (sizeof(t_ply_uint16) != 2) return 0;
    if (sizeof(t_ply_int32) != 4) return 0;
    if (sizeof(t_ply_uint32) != 4) return 0;
    if (sizeof(float) != 4) return 0;
    if (sizeof(double) != 8) return 0;
    return 1;
}


static int oascii_int8(p_ply ply, double value) {
    if (value > PLY_INT8_MAX || value < PLY_INT8_MIN) return 0;
    return fprintf(ply->fp, "%d", (t_ply_int8)value) > 0;
}

static int oascii_uint8(p_ply ply, double value) {
    if (value > PLY_UINT8_MAX || value < 0) return 0;
    return fprintf(ply->fp, "%d", (t_ply_uint8)value) > 0;
}

static int oascii_int16(p_ply ply, double value) {
    if (value > PLY_INT16_MAX || value < PLY_INT16_MIN) return 0;
    return fprintf(ply->fp, "%d", (t_ply_int16)value) > 0;
}

static int oascii_uint16(p_ply ply, double value) {
    if (value > PLY_UINT16_MAX || value < 0) return 0;
    return fprintf(ply->fp, "%d", (t_ply_uint16)value) > 0;
}

static int oascii_int32(p_ply ply, double value) {
    if (value > PLY_INT32_MAX || value < PLY_INT32_MIN) return 0;
    return fprintf(ply->fp, "%d", (t_ply_int32)value) > 0;
}

static int oascii_uint32(p_ply ply, double value) {
    if (value > PLY_UINT32_MAX || value < 0) return 0;
    return fprintf(ply->fp, "%d", (t_ply_uint32)value) > 0;
}

static int oascii_float32(p_ply ply, double value) {
    if (value < -FLT_MAX || value > FLT_MAX) return 0;
    return fprintf(ply->fp, "%g", (float)value) > 0;
}

static int oascii_float64(p_ply ply, double value) {
    if (value < -DBL_MAX || value > DBL_MAX) return 0;
    return fprintf(ply->fp, "%g", value) > 0;
}

static int obinary_int8(p_ply ply, double value) {
    t_ply_int8 int8 = (t_ply_int8)value;
    if (value > PLY_INT8_MAX || value < PLY_INT8_MIN) return 0;
    return ply->odriver->ochunk(ply, &int8, sizeof(int8));
}

static int obinary_uint8(p_ply ply, double value) {
    t_ply_uint8 uint8 = (t_ply_uint8)value;
    if (value > PLY_UINT8_MAX || value < 0) return 0;
    return ply->odriver->ochunk(ply, &uint8, sizeof(uint8));
}

static int obinary_int16(p_ply ply, double value) {
    t_ply_int16 int16 = (t_ply_int16)value;
    if (value > PLY_INT16_MAX || value < PLY_INT16_MIN) return 0;
    return ply->odriver->ochunk(ply, &int16, sizeof(int16));
}

static int obinary_uint16(p_ply ply, double value) {
    t_ply_uint16 uint16 = (t_ply_uint16)value;
    if (value > PLY_UINT16_MAX || value < 0) return 0;
    return ply->odriver->ochunk(ply, &uint16, sizeof(uint16));
}

static int obinary_int32(p_ply ply, double value) {
    t_ply_int32 int32 = (t_ply_int32)value;
    if (value > PLY_INT32_MAX || value < PLY_INT32_MIN) return 0;
    return ply->odriver->ochunk(ply, &int32, sizeof(int32));
}

static int obinary_uint32(p_ply ply, double value) {
    t_ply_uint32 uint32 = (t_ply_uint32)value;
    if (value > PLY_UINT32_MAX || value < 0) return 0;
    return ply->odriver->ochunk(ply, &uint32, sizeof(uint32));
}

static int obinary_float32(p_ply ply, double value) {
    float float32 = (float)value;
    if (value > FLT_MAX || value < -FLT_MAX) return 0;
    return ply->odriver->ochunk(ply, &float32, sizeof(float32));
}

static int obinary_float64(p_ply ply, double value) {
    return ply->odriver->ochunk(ply, &value, sizeof(value));
}


static int iascii_int8(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_INT8_MAX || *value < PLY_INT8_MIN) return 0;
    return 1;
}

static int iascii_uint8(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_UINT8_MAX || *value < 0) return 0;
    return 1;
}

static int iascii_int16(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_INT16_MAX || *value < PLY_INT16_MIN) return 0;
    return 1;
}

static int iascii_uint16(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_UINT16_MAX || *value < 0) return 0;
    return 1;
}

static int iascii_int32(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_INT32_MAX || *value < PLY_INT32_MIN) return 0;
    return 1;
}

static int iascii_uint32(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtol(BWORD(ply), &end, 10);
    if (*end || *value > PLY_UINT32_MAX || *value < 0) return 0;
    return 1;
}

static int iascii_float32(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtod(BWORD(ply), &end);
    if (*end || *value < -FLT_MAX || *value > FLT_MAX) return 0;
    return 1;
}

static int iascii_float64(p_ply ply, double *value) {
    char *end;
    if (!ply_read_word(ply)) return 0;
    *value = strtod(BWORD(ply), &end);
    if (*end || *value < -DBL_MAX || *value > DBL_MAX) return 0;
    return 1;
}

static int ibinary_int8(p_ply ply, double *value) {
    t_ply_int8 int8;
    if (!ply->idriver->ichunk(ply, &int8, 1)) return 0;
    *value = int8;
    return 1;
}

static int ibinary_uint8(p_ply ply, double *value) {
    t_ply_uint8 uint8;
    if (!ply->idriver->ichunk(ply, &uint8, 1)) return 0;
    *value = uint8;
    return 1;
}

static int ibinary_int16(p_ply ply, double *value) {
    t_ply_int16 int16;
    if (!ply->idriver->ichunk(ply, &int16, sizeof(int16))) return 0;
    *value = int16;
    return 1;
}

static int ibinary_uint16(p_ply ply, double *value) {
    t_ply_uint16 uint16;
    if (!ply->idriver->ichunk(ply, &uint16, sizeof(uint16))) return 0;
    *value = uint16;
    return 1;
}

static int ibinary_int32(p_ply ply, double *value) {
    t_ply_int32 int32;
    if (!ply->idriver->ichunk(ply, &int32, sizeof(int32))) return 0;
    *value = int32;
    return 1;
}

static int ibinary_uint32(p_ply ply, double *value) {
    t_ply_uint32 uint32;
    if (!ply->idriver->ichunk(ply, &uint32, sizeof(uint32))) return 0;
    *value = uint32;
    return 1;
}

static int ibinary_float32(p_ply ply, double *value) {
    float float32;
    if (!ply->idriver->ichunk(ply, &float32, sizeof(float32))) return 0;
    *value = float32;
    return 1;
}

static int ibinary_float64(p_ply ply, double *value) {
    return ply->idriver->ichunk(ply, value, sizeof(double));
}



namespace {
t_ply_idriver ply_idriver_ascii = {
    {iascii_int8, iascii_uint8, iascii_int16, iascii_uint16, iascii_int32,
     iascii_uint32, iascii_float32, iascii_float64, iascii_int8, iascii_uint8,
     iascii_int16, iascii_uint16, iascii_int32, iascii_uint32, iascii_float32,
     iascii_float64},
    NULL,
    "ascii input"};

t_ply_idriver ply_idriver_binary = {
    {ibinary_int8, ibinary_uint8, ibinary_int16, ibinary_uint16, ibinary_int32,
     ibinary_uint32, ibinary_float32, ibinary_float64, ibinary_int8,
     ibinary_uint8, ibinary_int16, ibinary_uint16, ibinary_int32,
     ibinary_uint32, ibinary_float32,
     ibinary_float64},
    ply_read_chunk,
    "binary input"};

t_ply_idriver ply_idriver_binary_reverse = {
    {ibinary_int8, ibinary_uint8, ibinary_int16, ibinary_uint16, ibinary_int32,
     ibinary_uint32, ibinary_float32, ibinary_float64, ibinary_int8,
     ibinary_uint8, ibinary_int16, ibinary_uint16, ibinary_int32,
     ibinary_uint32, ibinary_float32,
     ibinary_float64},
    ply_read_chunk_reverse,
    "reverse binary input"};

t_ply_odriver ply_odriver_ascii = {
    {oascii_int8, oascii_uint8, oascii_int16, oascii_uint16, oascii_int32,
     oascii_uint32, oascii_float32, oascii_float64, oascii_int8, oascii_uint8,
     oascii_int16, oascii_uint16, oascii_int32, oascii_uint32, oascii_float32,
     oascii_float64},
    NULL,
    "ascii output"};

t_ply_odriver ply_odriver_binary = {
    {obinary_int8, obinary_uint8, obinary_int16, obinary_uint16, obinary_int32,
     obinary_uint32, obinary_float32, obinary_float64, obinary_int8,
     obinary_uint8, obinary_int16, obinary_uint16, obinary_int32,
     obinary_uint32, obinary_float32,
     obinary_float64},
    ply_write_chunk,
    "binary output"};

t_ply_odriver ply_odriver_binary_reverse = {
    {obinary_int8, obinary_uint8, obinary_int16, obinary_uint16, obinary_int32,
     obinary_uint32, obinary_float32, obinary_float64, obinary_int8,
     obinary_uint8, obinary_int16, obinary_uint16, obinary_int32,
     obinary_uint32, obinary_float32,
     obinary_float64},
    ply_write_chunk_reverse,
    "reverse binary output"};
};


