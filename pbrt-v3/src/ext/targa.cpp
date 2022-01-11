
// ext/targa.cpp*


// static const char rcsid[] =
//    "$Id: targa.c,v 1.8 2004/10/09 09:30:26 emikulic Exp $";

#define TGA_KEEP_MACROS
#include "ext/targa.h"
#include <stdlib.h>
#include <string.h>

#define SANE_DEPTH(x) ((x) == 8 || (x) == 16 || (x) == 24 || (x) == 32)
#define UNMAP_DEPTH(x) ((x) == 16 || (x) == 24 || (x) == 32)

static const char tga_id[] =
    "\0\0\0\0"
    "\0\0\0\0"
    "TRUEVISION-XFILE.";

static const size_t tga_id_length = 26;


static tga_result tga_read_rle(tga_image *dest, FILE *fp);
static tga_result tga_write_row_RLE(FILE *fp, const tga_image *src,
                                    const uint8_t *row);
typedef enum { RAW, RLE } packet_type;
static packet_type rle_packet_type(const uint8_t *row, const uint16_t pos,
                                   const uint16_t width, const uint16_t bpp);
static uint8_t rle_packet_len(const uint8_t *row, const uint16_t pos,
                              const uint16_t width, const uint16_t bpp,
                              const packet_type type);

uint8_t tga_get_attribute_bits(const tga_image *tga) {
    return tga->image_descriptor & TGA_ATTRIB_BITS;
}

int tga_is_right_to_left(const tga_image *tga) {
    return (tga->image_descriptor & TGA_R_TO_L_BIT) != 0;
}

int tga_is_top_to_bottom(const tga_image *tga) {
    return (tga->image_descriptor & TGA_T_TO_B_BIT) != 0;
}

int tga_is_colormapped(const tga_image *tga) {
    return (tga->image_type == TGA_IMAGE_TYPE_COLORMAP ||
            tga->image_type == TGA_IMAGE_TYPE_COLORMAP_RLE);
}

int tga_is_rle(const tga_image *tga) {
    return (tga->image_type == TGA_IMAGE_TYPE_COLORMAP_RLE ||
            tga->image_type == TGA_IMAGE_TYPE_BGR_RLE ||
            tga->image_type == TGA_IMAGE_TYPE_MONO_RLE);
}

int tga_is_mono(const tga_image *tga) {
    return (tga->image_type == TGA_IMAGE_TYPE_MONO ||
            tga->image_type == TGA_IMAGE_TYPE_MONO_RLE);
}


const char *tga_error(const tga_result errcode) {
    switch (errcode) {
    case TGA_NOERR:
        return "no error";
    case TGAERR_FOPEN:
        return "error opening file";
    case TGAERR_EOF:
        return "premature end of file";
    case TGAERR_WRITE:
        return "error writing to file";
    case TGAERR_CMAP_TYPE:
        return "invalid color map type";
    case TGAERR_IMG_TYPE:
        return "invalid image type";
    case TGAERR_NO_IMG:
        return "no image data included";
    case TGAERR_CMAP_MISSING:
        return "color-mapped image without color map";
    case TGAERR_CMAP_PRESENT:
        return "non-color-mapped image with extraneous color map";
    case TGAERR_CMAP_LENGTH:
        return "color map has zero length";
    case TGAERR_CMAP_DEPTH:
        return "invalid color map depth";
    case TGAERR_ZERO_SIZE:
        return "the image dimensions are zero";
    case TGAERR_PIXEL_DEPTH:
        return "invalid pixel depth";
    case TGAERR_NO_MEM:
        return "out of memory";
    case TGAERR_NOT_CMAP:
        return "image is not color mapped";
    case TGAERR_RLE:
        return "RLE data is corrupt";
    case TGAERR_INDEX_RANGE:
        return "color map index out of range";
    case TGAERR_MONO:
        return "image is mono";
    default:
        return "unknown error code";
    }
}


tga_result tga_read(tga_image *dest, const char *filename) {
    tga_result result;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) return TGAERR_FOPEN;
    result = tga_read_from_FILE(dest, fp);
    fclose(fp);
    return result;
}


tga_result tga_read_from_FILE(tga_image *dest, FILE *fp) {
#define BARF(errcode)           \
    {                           \
        tga_free_buffers(dest); \
        return errcode;         \
    }

#define READ(destptr, size) \
    if (fread(destptr, size, 1, fp) != 1) BARF(TGAERR_EOF)

#define READ16(dest)                                         \
    {                                                        \
        if (fread(&(dest), 2, 1, fp) != 1) BARF(TGAERR_EOF); \
        dest = letoh16(dest);                                \
    }

    dest->image_id = NULL;
    dest->color_map_data = NULL;
    dest->image_data = NULL;

    READ(&dest->image_id_length, 1);
    READ(&dest->color_map_type, 1);
    if (dest->color_map_type != TGA_COLOR_MAP_ABSENT &&
        dest->color_map_type != TGA_COLOR_MAP_PRESENT)
        BARF(TGAERR_CMAP_TYPE);

    READ(&dest->image_type, 1);
    if (dest->image_type == TGA_IMAGE_TYPE_NONE) BARF(TGAERR_NO_IMG);

    if (dest->image_type != TGA_IMAGE_TYPE_COLORMAP &&
        dest->image_type != TGA_IMAGE_TYPE_BGR &&
        dest->image_type != TGA_IMAGE_TYPE_MONO &&
        dest->image_type != TGA_IMAGE_TYPE_COLORMAP_RLE &&
        dest->image_type != TGA_IMAGE_TYPE_BGR_RLE &&
        dest->image_type != TGA_IMAGE_TYPE_MONO_RLE)
        BARF(TGAERR_IMG_TYPE);

    if (tga_is_colormapped(dest) &&
        dest->color_map_type == TGA_COLOR_MAP_ABSENT)
        BARF(TGAERR_CMAP_MISSING);

    if (!tga_is_colormapped(dest) &&
        dest->color_map_type == TGA_COLOR_MAP_PRESENT)
        BARF(TGAERR_CMAP_PRESENT);

    READ16(dest->color_map_origin);
    READ16(dest->color_map_length);
    READ(&dest->color_map_depth, 1);
    if (dest->color_map_type == TGA_COLOR_MAP_PRESENT) {
        if (dest->color_map_length == 0) BARF(TGAERR_CMAP_LENGTH);

        if (!UNMAP_DEPTH(dest->color_map_depth)) BARF(TGAERR_CMAP_DEPTH);
    }

    READ16(dest->origin_x);
    READ16(dest->origin_y);
    READ16(dest->width);
    READ16(dest->height);

    if (dest->width == 0 || dest->height == 0) BARF(TGAERR_ZERO_SIZE);

    READ(&dest->pixel_depth, 1);
    if (!SANE_DEPTH(dest->pixel_depth) ||
        (dest->pixel_depth != 8 && tga_is_colormapped(dest)))
        BARF(TGAERR_PIXEL_DEPTH);

    READ(&dest->image_descriptor, 1);

    if (dest->image_id_length > 0) {
        dest->image_id = (uint8_t *)malloc(dest->image_id_length);
        if (dest->image_id == NULL) BARF(TGAERR_NO_MEM);
        READ(dest->image_id, dest->image_id_length);
    }

    if (dest->color_map_type == TGA_COLOR_MAP_PRESENT) {
        dest->color_map_data = (uint8_t *)malloc(
            (dest->color_map_origin + dest->color_map_length) *
            dest->color_map_depth / 8);
        if (dest->color_map_data == NULL) BARF(TGAERR_NO_MEM);
        READ(dest->color_map_data +
                 (dest->color_map_origin * dest->color_map_depth / 8),
             dest->color_map_length * dest->color_map_depth / 8);
    }

    dest->image_data =
        (uint8_t *)malloc(dest->width * dest->height * dest->pixel_depth / 8);
    if (dest->image_data == NULL) BARF(TGAERR_NO_MEM);

    if (tga_is_rle(dest)) {

        tga_result result = tga_read_rle(dest, fp);
        if (result != TGA_NOERR) BARF(result);
    } else {

        READ(dest->image_data,
             dest->width * dest->height * dest->pixel_depth / 8);
    }

    return TGA_NOERR;
#undef BARF
#undef READ
#undef READ16
}


static tga_result tga_read_rle(tga_image *dest, FILE *fp) {
#define RLE_BIT BIT(7)
#define READ(dest, size) \
    if (fread(dest, size, 1, fp) != 1) return TGAERR_EOF

    uint8_t *pos;
    uint32_t p_loaded = 0, p_expected = dest->width * dest->height;
    uint8_t bpp = dest->pixel_depth / 8;

    pos = dest->image_data;

    while ((p_loaded < p_expected) && !feof(fp)) {
        uint8_t b;
        READ(&b, 1);
        if (b & RLE_BIT) {

            uint8_t count, tmp[4], i;

            count = (b & ~RLE_BIT) + 1;
            READ(tmp, bpp);

            for (i = 0; i < count; i++) {
                p_loaded++;
                if (p_loaded > p_expected) return TGAERR_RLE;
                memcpy(pos, tmp, bpp);
                pos += bpp;
            }
        } else
        {
            uint8_t count;

            count = (b & ~RLE_BIT) + 1;
            if (p_loaded + count > p_expected) return TGAERR_RLE;

            p_loaded += count;
            READ(pos, bpp * count);
            pos += count * bpp;
        }
    }
    return TGA_NOERR;
#undef RLE_BIT
#undef READ
}


tga_result tga_write(const char *filename, const tga_image *src) {
    tga_result result;
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) return TGAERR_FOPEN;
    result = tga_write_to_FILE(fp, src);
    fclose(fp);
    return result;
}


#define PIXEL(ofs) (row + (ofs)*bpp)
static tga_result tga_write_row_RLE(FILE *fp, const tga_image *src,
                                    const uint8_t *row) {
#define WRITE(src, size) \
    if (fwrite(src, size, 1, fp) != 1) return TGAERR_WRITE

    uint16_t pos = 0;
    uint16_t bpp = src->pixel_depth / 8;

    while (pos < src->width) {
        packet_type type = rle_packet_type(row, pos, src->width, bpp);
        uint8_t len = rle_packet_len(row, pos, src->width, bpp, type);
        uint8_t packet_header;

        packet_header = len - 1;
        if (type == RLE) packet_header |= BIT(7);

        WRITE(&packet_header, 1);
        if (type == RLE) {
            WRITE(PIXEL(pos), bpp);
        } else
        {
            WRITE(PIXEL(pos), bpp * len);
        }

        pos += len;
    }

    return TGA_NOERR;
#undef WRITE
}


#define SAME(ofs1, ofs2) (memcmp(PIXEL(ofs1), PIXEL(ofs2), bpp) == 0)

static packet_type rle_packet_type(const uint8_t *row, const uint16_t pos,
                                   const uint16_t width, const uint16_t bpp) {
    if (pos == width - 1) return RAW;
    if (SAME(pos, pos + 1))
    {
        if (bpp > 1) return RLE;


        if ((pos < width - 2) && SAME(pos + 1, pos + 2)) return RLE;
    }
    return RAW;
}


static uint8_t rle_packet_len(const uint8_t *row, const uint16_t pos,
                              const uint16_t width, const uint16_t bpp,
                              const packet_type type) {
    uint8_t len = 2;

    if (pos == width - 1) return 1;
    if (pos == width - 2) return 2;

    if (type == RLE) {
        while (pos + len < width) {
            if (SAME(pos, pos + len))
                len++;
            else
                return len;

            if (len == 128) return 128;
        }
    } else
    {
        while (pos + len < width) {
            if (rle_packet_type(row, pos + len, width, bpp) == RAW)
                len++;
            else
                return len;
            if (len == 128) return 128;
        }
    }
    return len;
}

#undef SAME
#undef PIXEL


tga_result tga_write_to_FILE(FILE *fp, const tga_image *src) {
#define WRITE(srcptr, size) \
    if (fwrite(srcptr, size, 1, fp) != 1) return TGAERR_WRITE

#define WRITE16(src)                                            \
    {                                                           \
        uint16_t _temp = htole16(src);                          \
        if (fwrite(&_temp, 2, 1, fp) != 1) return TGAERR_WRITE; \
    }

    WRITE(&src->image_id_length, 1);

    if (src->color_map_type != TGA_COLOR_MAP_ABSENT &&
        src->color_map_type != TGA_COLOR_MAP_PRESENT)
        return TGAERR_CMAP_TYPE;
    WRITE(&src->color_map_type, 1);

    if (src->image_type == TGA_IMAGE_TYPE_NONE) return TGAERR_NO_IMG;
    if (src->image_type != TGA_IMAGE_TYPE_COLORMAP &&
        src->image_type != TGA_IMAGE_TYPE_BGR &&
        src->image_type != TGA_IMAGE_TYPE_MONO &&
        src->image_type != TGA_IMAGE_TYPE_COLORMAP_RLE &&
        src->image_type != TGA_IMAGE_TYPE_BGR_RLE &&
        src->image_type != TGA_IMAGE_TYPE_MONO_RLE)
        return TGAERR_IMG_TYPE;
    WRITE(&src->image_type, 1);

    if (tga_is_colormapped(src) && src->color_map_type == TGA_COLOR_MAP_ABSENT)
        return TGAERR_CMAP_MISSING;
    if (!tga_is_colormapped(src) &&
        src->color_map_type == TGA_COLOR_MAP_PRESENT)
        return TGAERR_CMAP_PRESENT;
    if (src->color_map_type == TGA_COLOR_MAP_PRESENT) {
        if (src->color_map_length == 0) return TGAERR_CMAP_LENGTH;

        if (!UNMAP_DEPTH(src->color_map_depth)) return TGAERR_CMAP_DEPTH;
    }
    WRITE16(src->color_map_origin);
    WRITE16(src->color_map_length);
    WRITE(&src->color_map_depth, 1);

    WRITE16(src->origin_x);
    WRITE16(src->origin_y);

    if (src->width == 0 || src->height == 0) return TGAERR_ZERO_SIZE;
    WRITE16(src->width);
    WRITE16(src->height);

    if (!SANE_DEPTH(src->pixel_depth) ||
        (src->pixel_depth != 8 && tga_is_colormapped(src)))
        return TGAERR_PIXEL_DEPTH;
    WRITE(&src->pixel_depth, 1);

    WRITE(&src->image_descriptor, 1);

    if (src->image_id_length > 0) WRITE(&src->image_id, src->image_id_length);

    if (src->color_map_type == TGA_COLOR_MAP_PRESENT)
        WRITE(src->color_map_data +
                  (src->color_map_origin * src->color_map_depth / 8),
              src->color_map_length * src->color_map_depth / 8);

    if (tga_is_rle(src)) {
        uint16_t row;
        for (row = 0; row < src->height; row++) {
            tga_result result = tga_write_row_RLE(
                fp, src,
                src->image_data + row * src->width * src->pixel_depth / 8);
            if (result != TGA_NOERR) return result;
        }
    } else {

        WRITE(src->image_data, src->width * src->height * src->pixel_depth / 8);
    }

    WRITE(tga_id, tga_id_length);

    return TGA_NOERR;
#undef WRITE
#undef WRITE16
}




static void init_tga_image(tga_image *img, uint8_t *image, const uint16_t width,
                           const uint16_t height, const uint8_t depth) {
    img->image_id_length = 0;
    img->color_map_type = TGA_COLOR_MAP_ABSENT;
    img->image_type = TGA_IMAGE_TYPE_NONE;
    img->color_map_origin = 0;
    img->color_map_length = 0;
    img->color_map_depth = 0;
    img->origin_x = 0;
    img->origin_y = 0;
    img->width = width;
    img->height = height;
    img->pixel_depth = depth;
    img->image_descriptor = TGA_T_TO_B_BIT;
    img->image_id = NULL;
    img->color_map_data = NULL;
    img->image_data = image;
}

tga_result tga_write_mono(const char *filename, uint8_t *image,
                          const uint16_t width, const uint16_t height) {
    tga_image img;
    init_tga_image(&img, image, width, height, 8);
    img.image_type = TGA_IMAGE_TYPE_MONO;
    return tga_write(filename, &img);
}

tga_result tga_write_mono_rle(const char *filename, uint8_t *image,
                              const uint16_t width, const uint16_t height) {
    tga_image img;
    init_tga_image(&img, image, width, height, 8);
    img.image_type = TGA_IMAGE_TYPE_MONO_RLE;
    return tga_write(filename, &img);
}

tga_result tga_write_bgr(const char *filename, uint8_t *image,
                         const uint16_t width, const uint16_t height,
                         const uint8_t depth) {
    tga_image img;
    init_tga_image(&img, image, width, height, depth);
    img.image_type = TGA_IMAGE_TYPE_BGR;
    return tga_write(filename, &img);
}

tga_result tga_write_bgr_rle(const char *filename, uint8_t *image,
                             const uint16_t width, const uint16_t height,
                             const uint8_t depth) {
    tga_image img;
    init_tga_image(&img, image, width, height, depth);
    img.image_type = TGA_IMAGE_TYPE_BGR_RLE;
    return tga_write(filename, &img);
}


tga_result tga_write_rgb(const char *filename, uint8_t *image,
                         const uint16_t width, const uint16_t height,
                         const uint8_t depth) {
    tga_image img;
    init_tga_image(&img, image, width, height, depth);
    img.image_type = TGA_IMAGE_TYPE_BGR;
    (void)tga_swap_red_blue(&img);
    return tga_write(filename, &img);
}


tga_result tga_write_rgb_rle(const char *filename, uint8_t *image,
                             const uint16_t width, const uint16_t height,
                             const uint8_t depth) {
    tga_image img;
    init_tga_image(&img, image, width, height, depth);
    img.image_type = TGA_IMAGE_TYPE_BGR_RLE;
    (void)tga_swap_red_blue(&img);
    return tga_write(filename, &img);
}




tga_result tga_flip_horiz(tga_image *img) {
    uint16_t row;
    size_t bpp;
    uint8_t *left, *right;
    int r_to_l;

    if (!SANE_DEPTH(img->pixel_depth)) return TGAERR_PIXEL_DEPTH;
    bpp = (size_t)(img->pixel_depth / 8);

    for (row = 0; row < img->height; row++) {
        left = img->image_data + row * img->width * bpp;
        right = left + (img->width - 1) * bpp;


        while (left < right) {
            uint8_t buffer[4];


            memcpy(buffer, left, bpp);
            memcpy(left, right, bpp);
            memcpy(right, buffer, bpp);

            left += bpp;
            right -= bpp;
        }
    }


    r_to_l = tga_is_right_to_left(img);
    img->image_descriptor &= ~TGA_R_TO_L_BIT;
    if (!r_to_l)
        img->image_descriptor |= TGA_R_TO_L_BIT;


    return TGA_NOERR;
}


tga_result tga_flip_vert(tga_image *img) {
    uint16_t col;
    size_t bpp, line;
    uint8_t *top, *bottom;
    int t_to_b;

    if (!SANE_DEPTH(img->pixel_depth)) return TGAERR_PIXEL_DEPTH;
    bpp = (size_t)(img->pixel_depth / 8);
    line = bpp * img->width;

    for (col = 0; col < img->width; col++) {
        top = img->image_data + col * bpp;
        bottom = top + (img->height - 1) * line;


        while (top < bottom) {
            uint8_t buffer[4];


            memcpy(buffer, top, bpp);
            memcpy(top, bottom, bpp);
            memcpy(bottom, buffer, bpp);

            top += line;
            bottom -= line;
        }
    }


    t_to_b = tga_is_top_to_bottom(img);
    img->image_descriptor &= ~TGA_T_TO_B_BIT;
    if (!t_to_b)
        img->image_descriptor |= TGA_T_TO_B_BIT;


    return TGA_NOERR;
}


tga_result tga_color_unmap(tga_image *img) {
    uint8_t bpp = img->color_map_depth / 8;
    int pos;
    void *tmp;

    if (!tga_is_colormapped(img)) return TGAERR_NOT_CMAP;
    if (img->pixel_depth != 8) return TGAERR_PIXEL_DEPTH;
    if (!SANE_DEPTH(img->color_map_depth)) return TGAERR_CMAP_DEPTH;

    tmp = realloc(img->image_data, img->width * img->height * bpp);
    if (tmp == NULL) return TGAERR_NO_MEM;
    img->image_data = (uint8_t *)tmp;

    for (pos = img->width * img->height - 1; pos >= 0; pos--) {
        uint8_t c_index = img->image_data[pos];
        uint8_t *c_bgr = img->color_map_data + (c_index * bpp);

        if (c_index >= img->color_map_origin + img->color_map_length)
            return TGAERR_INDEX_RANGE;

        memcpy(img->image_data + (pos * bpp), c_bgr, (size_t)bpp);
    }


    img->image_type = TGA_IMAGE_TYPE_BGR;
    img->pixel_depth = img->color_map_depth;

    free(img->color_map_data);
    img->color_map_data = NULL;
    img->color_map_type = TGA_COLOR_MAP_ABSENT;
    img->color_map_origin = 0;
    img->color_map_length = 0;
    img->color_map_depth = 0;

    return TGA_NOERR;
}


uint8_t *tga_find_pixel(const tga_image *img, uint16_t x, uint16_t y) {
    if (x >= img->width || y >= img->height) return NULL;

    if (!tga_is_top_to_bottom(img)) y = img->height - 1 - y;
    if (tga_is_right_to_left(img)) x = img->width - 1 - x;
    return img->image_data + (x + y * img->width) * img->pixel_depth / 8;
}


tga_result tga_unpack_pixel(const uint8_t *src, const uint8_t bits, uint8_t *b,
                            uint8_t *g, uint8_t *r, uint8_t *a) {
    switch (bits) {
    case 32:
        if (b) *b = src[0];
        if (g) *g = src[1];
        if (r) *r = src[2];
        if (a) *a = src[3];
        break;

    case 24:
        if (b) *b = src[0];
        if (g) *g = src[1];
        if (r) *r = src[2];
        if (a) *a = 0;
        break;

    case 16: {
        uint16_t src16 = (uint16_t)(src[1] << 8) | (uint16_t)src[0];

#define FIVE_BITS (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4))
        if (b) *b = ((src16)&FIVE_BITS) << 3;
        if (g) *g = ((src16 >> 5) & FIVE_BITS) << 3;
        if (r) *r = ((src16 >> 10) & FIVE_BITS) << 3;
        if (a) *a = (uint8_t)((src16 & BIT(15)) ? 255 : 0);
#undef FIVE_BITS
        break;
    }

    case 8:
        if (b) *b = *src;
        if (g) *g = *src;
        if (r) *r = *src;
        if (a) *a = 0;
        break;

    default:
        return TGAERR_PIXEL_DEPTH;
    }
    return TGA_NOERR;
}


tga_result tga_pack_pixel(uint8_t *dest, const uint8_t bits, const uint8_t b,
                          const uint8_t g, const uint8_t r, const uint8_t a) {
    switch (bits) {
    case 32:
        dest[0] = b;
        dest[1] = g;
        dest[2] = r;
        dest[3] = a;
        break;

    case 24:
        dest[0] = b;
        dest[1] = g;
        dest[2] = r;
        break;

    case 16: {
        uint16_t tmp;

#define FIVE_BITS (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4))
        tmp = (b >> 3) & FIVE_BITS;
        tmp |= ((g >> 3) & FIVE_BITS) << 5;
        tmp |= ((r >> 3) & FIVE_BITS) << 10;
        if (a > 127) tmp |= BIT(15);
#undef FIVE_BITS

        dest[0] = (uint8_t)(tmp & 0x00FF);
        dest[1] = (uint8_t)((tmp & 0xFF00) >> 8);
        break;
    }

    default:
        return TGAERR_PIXEL_DEPTH;
    }
    return TGA_NOERR;
}


tga_result tga_desaturate(tga_image *img, const int cr, const int cg,
                          const int cb, const int dv) {
    uint8_t bpp = img->pixel_depth / 8;
    uint8_t *dest, *src, *tmp;

    if (tga_is_mono(img)) return TGAERR_MONO;
    if (tga_is_colormapped(img)) {
        tga_result result = tga_color_unmap(img);
        if (result != TGA_NOERR) return result;
    }
    if (!UNMAP_DEPTH(img->pixel_depth)) return TGAERR_PIXEL_DEPTH;

    dest = img->image_data;
    for (src = img->image_data;
         src < img->image_data + img->width * img->height * bpp; src += bpp) {
        uint8_t b, g, r;
        (void)tga_unpack_pixel(src, img->pixel_depth, &b, &g, &r, NULL);

        *dest = (uint8_t)(((int)b * cb + (int)g * cg + (int)r * cr) / dv);
        dest++;
    }


    tmp = (uint8_t *)realloc(img->image_data, img->width * img->height);
    if (tmp == NULL) return TGAERR_NO_MEM;
    img->image_data = tmp;

    img->pixel_depth = 8;
    img->image_type = TGA_IMAGE_TYPE_MONO;
    return TGA_NOERR;
}

tga_result tga_desaturate_rec_601_1(tga_image *img) {
    return tga_desaturate(img, 2989, 5866, 1145, 10000);
}

tga_result tga_desaturate_rec_709(tga_image *img) {
    return tga_desaturate(img, 2126, 7152, 722, 10000);
}

tga_result tga_desaturate_itu(tga_image *img) {
    return tga_desaturate(img, 2220, 7067, 713, 10000);
}

tga_result tga_desaturate_avg(tga_image *img) {
    return tga_desaturate(img, 1, 1, 1, 3);
}


tga_result tga_convert_depth(tga_image *img, const uint8_t bits) {
    size_t src_size, dest_size;
    uint8_t src_bpp, dest_bpp;
    uint8_t *src, *dest;

    if (!UNMAP_DEPTH(bits) || !SANE_DEPTH(img->pixel_depth))
        return TGAERR_PIXEL_DEPTH;

    if (tga_is_colormapped(img)) {
        tga_result result = tga_color_unmap(img);
        if (result != TGA_NOERR) return result;
    }

    if (img->pixel_depth == bits) return TGA_NOERR;

    src_bpp = img->pixel_depth / 8;
    dest_bpp = bits / 8;

    src_size = (size_t)(img->width * img->height * src_bpp);
    dest_size = (size_t)(img->width * img->height * dest_bpp);

    if (src_size > dest_size) {
        void *tmp;


        dest = img->image_data;
        for (src = img->image_data;
             src < img->image_data + img->width * img->height * src_bpp;
             src += src_bpp) {
            uint8_t r, g, b, a;
            (void)tga_unpack_pixel(src, img->pixel_depth, &r, &g, &b, &a);
            (void)tga_pack_pixel(dest, bits, r, g, b, a);
            dest += dest_bpp;
        }


        tmp = realloc(img->image_data, img->width * img->height * dest_bpp);
        if (tmp == NULL) return TGAERR_NO_MEM;
        img->image_data = (uint8_t *)tmp;
    } else {

        void *tmp =
            realloc(img->image_data, img->width * img->height * dest_bpp);
        if (tmp == NULL) return TGAERR_NO_MEM;
        img->image_data = (uint8_t *)tmp;


        dest = img->image_data + (img->width * img->height - 1) * dest_bpp;
        for (src = img->image_data + (img->width * img->height - 1) * src_bpp;
             src >= img->image_data; src -= src_bpp) {
            uint8_t r, g, b, a;
            (void)tga_unpack_pixel(src, img->pixel_depth, &r, &g, &b, &a);
            (void)tga_pack_pixel(dest, bits, r, g, b, a);
            dest -= dest_bpp;
        }
    }

    img->pixel_depth = bits;
    return TGA_NOERR;
}


tga_result tga_swap_red_blue(tga_image *img) {
    uint8_t *ptr;
    uint8_t bpp = img->pixel_depth / 8;

    if (!UNMAP_DEPTH(img->pixel_depth)) return TGAERR_PIXEL_DEPTH;

    for (ptr = img->image_data;
         ptr < img->image_data + (img->width * img->height - 1) * bpp;
         ptr += bpp) {
        uint8_t r, g, b, a;
        (void)tga_unpack_pixel(ptr, img->pixel_depth, &b, &g, &r, &a);
        (void)tga_pack_pixel(ptr, img->pixel_depth, r, g, b, a);
    }
    return TGA_NOERR;
}


void tga_free_buffers(tga_image *img) {
    if (img->image_id != NULL) {
        free(img->image_id);
        img->image_id = NULL;
    }
    if (img->color_map_data != NULL) {
        free(img->color_map_data);
        img->color_map_data = NULL;
    }
    if (img->image_data != NULL) {
        free(img->image_data);
        img->image_data = NULL;
    }
}
