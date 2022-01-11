#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_EXT_RPLY_H
#define PBRT_EXT_RPLY_H

// ext/rply.h*


#define RPLY_VERSION "RPly 1.1.3"
#define RPLY_COPYRIGHT "Copyright (C) 2003-2013 Diego Nehab"
#define RPLY_AUTHORS "Diego Nehab"



typedef struct t_ply_ *p_ply;
typedef struct t_ply_element_ *p_ply_element;
typedef struct t_ply_property_ *p_ply_property;
typedef struct t_ply_argument_ *p_ply_argument;


typedef enum e_ply_storage_mode_ {
    PLY_BIG_ENDIAN,
    PLY_LITTLE_ENDIAN,
    PLY_ASCII,
    PLY_DEFAULT
} e_ply_storage_mode;


typedef enum e_ply_type {
    PLY_INT8,
    PLY_UINT8,
    PLY_INT16,
    PLY_UINT16,
    PLY_INT32,
    PLY_UIN32,
    PLY_FLOAT32,
    PLY_FLOAT64,
    PLY_CHAR,
    PLY_UCHAR,
    PLY_SHORT,
    PLY_USHORT,
    PLY_INT,
    PLY_UINT,
    PLY_FLOAT,
    PLY_DOUBLE,
    PLY_LIST
} e_ply_type;


typedef void (*p_ply_error_cb)(p_ply ply, const char *message);


int ply_get_ply_user_data(p_ply ply, void **pdata, long *idata);


p_ply ply_open(const char *name, p_ply_error_cb error_cb, long idata,
               void *pdata);


int ply_read_header(p_ply ply);


typedef int (*p_ply_read_cb)(p_ply_argument argument);


long ply_set_read_cb(p_ply ply, const char *element_name,
                     const char *property_name, p_ply_read_cb read_cb,
                     void *pdata, long idata);


int ply_get_argument_element(p_ply_argument argument, p_ply_element *element,
                             long *instance_index);


int ply_get_argument_property(p_ply_argument argument, p_ply_property *property,
                              long *length, long *value_index);


int ply_get_argument_user_data(p_ply_argument argument, void **pdata,
                               long *idata);


double ply_get_argument_value(p_ply_argument argument);


int ply_read(p_ply ply);


p_ply_element ply_get_next_element(p_ply ply, p_ply_element last);


const char *ply_get_next_comment(p_ply ply, const char *last);


const char *ply_get_next_obj_info(p_ply ply, const char *last);


int ply_get_element_info(p_ply_element element, const char **name,
                         long *ninstances);


p_ply_property ply_get_next_property(p_ply_element element,
                                     p_ply_property last);


int ply_get_property_info(p_ply_property property, const char **name,
                          e_ply_type *type, e_ply_type *length_type,
                          e_ply_type *value_type);


p_ply ply_create(const char *name, e_ply_storage_mode storage_mode,
                 p_ply_error_cb error_cb, long idata, void *pdata);


int ply_add_element(p_ply ply, const char *name, long ninstances);


int ply_add_property(p_ply ply, const char *name, e_ply_type type,
                     e_ply_type length_type, e_ply_type value_type);


int ply_add_list_property(p_ply ply, const char *name, e_ply_type length_type,
                          e_ply_type value_type);


int ply_add_scalar_property(p_ply ply, const char *name, e_ply_type type);


int ply_add_comment(p_ply ply, const char *comment);


int ply_add_obj_info(p_ply ply, const char *obj_info);


int ply_write_header(p_ply ply);


int ply_write(p_ply ply, double value);


int ply_close(p_ply ply);



#endif  // PBRT_EXT_RPLY_H
