#include "png.h"

/*
    Old versions of libpng do not contain some functions that pdfTeX uses, so
    we must use correct library.
    Fri Sep  4 23:40:11 CEST 1998 (pj)

    If your compiler can not eat this, you should use ANSI-C compliant one.
    Please report it to pdfTeX@tug.org too.
    Thu Sep 10 22:18:29 CEST 1998 (pj)
*/

#if PNG_LIBPNG_VER < 10001
#error "Your system libpng is too old for pdfTeX, please use pdfTeX's one."
#endif /* PNG_LIBPNG_VER < 10001 */

#include "jpg.h"
#ifdef HAVE_TIFF
#include "tiffx.h"
#endif

#define PDF_INFO(N)      (image_tab[N].image_struct.pdf)
#define PNG_PTR(N)       (image_tab[N].image_struct.png.png_ptr)
#define PNG_INFO(N)      (image_tab[N].image_struct.png.info_ptr)
#define JPG_INFO(N)      (image_tab[N].image_struct.jpg)
#ifdef HAVE_TIFF
#define TIFF_INFO(N)      (image_tab[N].image_struct.tiff)
#endif
#define IMG_TYPE(N)      (image_tab[N].image_type)
#define IMG_NAME(N)      (image_tab[N].image_name)
#define IMG_REFCOUNT(N)  (image_tab[N].ref_count)

#define IMAGE_TYPE_PDF  0
#define IMAGE_TYPE_PNG  1
#define IMAGE_TYPE_JPG  2
#ifdef HAVE_TIFF
#define IMAGE_TYPE_TIFF 3
#endif

typedef struct {
    png_structp png_ptr;
    png_infop info_ptr;
} png_image_struct;

typedef struct {
    int width;
    int height;
    int orig_x;
    int orig_y;
    void *doc;
    void *xref;
} pdf_image_struct;

typedef struct {
    short ref_count;          
    short image_type;
    char *image_name;
    union {
        pdf_image_struct *pdf;
        png_image_struct png;
        JPG_IMAGE_INFO *jpg;
#ifdef HAVE_TIFF
        TIFF_IMAGE_INFO *tiff;
#endif
    } image_struct;
} image_entry;

extern image_entry *image_ptr, *image_tab;
extern integer image_max;

extern integer read_pdf_info(char*);
extern void write_epdf(integer, integer);
extern void epdf_delete();
extern void epdf_free();
extern integer read_png_info(integer);
extern void write_png(integer, integer);
extern integer read_jpg_info(integer);
extern void write_jpg(integer, integer);
#ifdef HAVE_TIFF
extern integer read_tiff_info(integer);
extern void write_tiff(integer, integer);
#endif
