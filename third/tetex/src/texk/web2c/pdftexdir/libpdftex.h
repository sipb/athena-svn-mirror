#include "webmacros.h"

#ifdef WIN32
#define M_PI       3.1415926535897932385E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#define M_PI_2     1.5707963267948966192E0  /*Hex  2^ 0 * 1.921FB54442D18 */
#define M_PI_4     7.8539816339744830962E-1 /*Hex  2^-1 * 1.921FB54442D18 */
#endif

#define MAX_CHAR_NUM    256

#define CHECK_BUF(size, buf_size)                         \
    if ((size) >= buf_size - 2)                           \
        pdftex_fail("buffer overflow [%li bytes]", (long int)(buf_size))

#define APPEND_CHAR_TO_BUF(c, p, buf, buf_size) do {       \
    if (c == 9)                                            \
        c = 32;                                            \
    if (c == 13 || c == EOF)                               \
        c = 10;                                            \
    if (c != ' ' || (p > buf && p[-1] != 32)) {            \
        CHECK_BUF(p - buf, buf_size);                      \
        *p++ = c;                                          \
    }                                                      \
} while (0)

#define APPEND_EOL(p, buf, buf_size) do {                  \
    if (p - buf > 1 && p[-1] != 10) {                      \
        CHECK_BUF(p - buf, buf_size);                      \
        *p++ = 10;                                         \
    }                                                      \
    if (p - buf > 2 && p[-2] == 32) {                      \
        p[-2] = 10;                                        \
        p--;                                               \
    }                                                      \
    *p = 0;                                                \
} while (0)

#define READ_FIELD(r, q, buf) do {                         \
    for (q = buf; *r != 32 && *r != 10; *q++ = *r++);      \
    *q = 0;                                                \
    if (*r == 32)                                          \
        r++;                                               \
} while (0)

#define ENTRY_ROOM(T, S) do {                              \
    if (T##_tab == 0) {                                    \
        T##_max = (S);                                     \
        T##_tab = XTALLOC(T##_max, T##_entry);             \
        T##_ptr = T##_tab;                                 \
    }                                                      \
    else if (T##_ptr - T##_tab == T##_max) {               \
        T##_tab = XRETALLOC(T##_tab, T##_max + (S), T##_entry); \
        T##_ptr = T##_tab + T##_max;                       \
        T##_max += (S);                                    \
    }                                                      \
} while (0)

#define XFREE(P)            do { if (P != 0) free(P); } while (0)

#define strend(S)           strchr(S, 0)

#define ASCENT_CODE         0
#define CAPHEIGHT_CODE      1
#define DESCENT_CODE        2
#define FONTNAME_CODE       3
#define ITALIC_ANGLE_CODE   4
#define STEMV_CODE          5
#define XHEIGHT_CODE        6
#define FONTBBOX1_CODE      7
#define FONTBBOX2_CODE      8
#define FONTBBOX3_CODE      9
#define FONTBBOX4_CODE      10
#define MAX_KEY_CODE        (FONTBBOX1_CODE + 1)
#define FONT_KEYS_NUM       (FONTBBOX4_CODE + 1)

#define F_INCLUDED  0x01
#define F_SUBSETTED 0x02
#define F_TRUETYPE  0x04
#define F_BASEFONT  0x08
#define F_NOPARSING 0x10
#define F_PGCFONT   0x20

#define is_included()   (fm_cur->font_type & F_INCLUDED)
#define is_subsetted()  (fm_cur->font_type & F_SUBSETTED)
#define is_truetype()   (fm_cur->font_type & F_TRUETYPE)
#define is_basefont()   (fm_cur->font_type & F_BASEFONT)
#define is_noparsing()  (fm_cur->font_type & F_NOPARSING)
#define is_pcgfont()    (fm_cur->font_type & F_PGCFONT)
#define is_reencoded()  (fm_cur->encoding >= 0)
#define is_slanted()    (fm_cur->slant != 0)
#define is_extended()   (fm_cur->extend > 0)

typedef struct {
    char *pdfname;
    char *t1name;
    union {
      integer i;
      char *s;
    } value;
    boolean valid;
} key_entry;

typedef struct {
    integer obj_num;
    char *name;
    char *glyph_names[MAX_CHAR_NUM];
} enc_entry;

typedef struct {
    char *tex_name;           /* TFM file name */
    char *base_name;          /* PostScript name */
    integer flags;            /* font flags */
    char *ff_name;            /* font file name */
    char *mm_name;            /* mm-instance name */
    char *prefix;             /* prefix for subsetted font */
    int encoding;             /* index to table of encoding vectors */
    int font_type;            /* font type */
    integer slant;
    integer extend;
} fm_entry;

extern char *builtin_glyph_names[];
extern enc_entry *enc_ptr, *enc_tab;
extern char *filename;
extern fm_entry *fm_cur, *fm_ptr, *fm_tab;
extern char **cur_glyph_names;
extern char *mapfiles;
extern boolean font_file_not_found;
extern key_entry font_keys[];
extern char notdef[];
extern char print_buf[];
extern integer t1_length1, t1_length2, t1_length3;
extern internalfontnumber tex_font;
extern integer ttf_length;

extern void tex_printf(char *, ...);
extern void pdf_printf(char *, ...);
extern void pdftex_fail(char *, ...);
extern void pdftex_warn(char *, ...);
extern int xfflush(FILE *);
extern size_t xfwrite(void *, size_t, size_t, FILE *);
extern int xgetc(FILE *);
extern int xputc(int, FILE *);
extern char *makecstring(integer);
extern boolean str_eq_cstr(strnumber, char *);
extern strnumber maketexstring(char *);
extern void pdfout(integer);
extern integer pdfoffset();
extern void flush_print_buf();
extern integer myatodim(char **);
extern integer myatol(char **);
extern void readconfig();
extern integer fmlookup();
extern fm_entry *fm_ext_entry(internalfontnumber);
extern void fm_free();
extern integer add_enc(char *);
extern integer enc_objnum(integer);
extern void enc_free();
extern void writettf();
extern void writet1();
extern void writet3();
extern void img_free();
extern void vf_free();
extern void libpdffinish();
extern void getbbox(internalfontnumber);
extern void writeEPDF(char *, ...);
extern void epdf_free();
