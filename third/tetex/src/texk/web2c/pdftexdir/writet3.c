#include "libpdftex.h"
#include "pkin.h"
#include <kpathsea/tex-glyph.h>
#include <kpathsea/magstep.h>

#define T3_BUF_SIZE   1024

#define T3_TYPE_PK    0
#define T3_TYPE_PGC   1

static char t3_line[T3_BUF_SIZE], *t3_line_ptr;
FILE *t3_file;
static boolean t3_image_used;

static unsigned short t3_char_procs[MAX_CHAR_NUM];
static unsigned short t3_char_widths[MAX_CHAR_NUM];
static unsigned short t3_glyph_num;
static float t3_font_scale;
static long int t3_font_type;
static long int t3_b0, t3_b1, t3_b2, t3_b3;

#define T3_OPEN()       typeonebopenin(t3_file)
#define T3_CLOSE()      xfclose(t3_file, filename)
#define T3_GETCHAR()    xgetc(t3_file)
#define T3_EOF()        feof(t3_file)
#define T3_PREFIX(s)    (!strncmp(t3_line, s, strlen(s)))
#define T3_PUTCHAR(c)   pdfout(c)

#define T3_CHECK_EOF()                                     \
    if (T3_EOF())                                          \
        pdftex_fail("unexpected end of file");

static void t3_getline() 
{
    int c;
restart:
    t3_line_ptr = t3_line;
    c = T3_GETCHAR();
    while (!T3_EOF()) {
        APPEND_CHAR_TO_BUF(c, t3_line_ptr, t3_line, T3_BUF_SIZE);
        if (c == 10)
            break;
        c = T3_GETCHAR();
    }
    APPEND_EOL(t3_line_ptr, t3_line, T3_BUF_SIZE);
    if (t3_line_ptr - t3_line <= 1 || *t3_line == '%') {
        if (!T3_EOF())
            goto restart;
    }
}

static void t3_putline()
{
    char *p = t3_line;
    while (p < t3_line_ptr)
        T3_PUTCHAR(*p++);
}

/*
static void t3_print_char_width(internalfontnumber f, integer c)
{
    pdfprintint(dividescaled(getcharwidth(f, c),
                             xnoverd(pdffontsize[f], 1000, t3_font_scale), 3));
}
*/

static void update_bbox(integer llx, integer lly, integer urx, integer ury)
{
    if (t3_glyph_num == 0) {
        t3_b0 = llx;
        t3_b1 = lly;
        t3_b2 = urx;
        t3_b3 = ury;
    }
    else {
        if (llx < t3_b0)
            t3_b0 = llx;
        if (lly < t3_b1)
            t3_b1 = lly;
        if (urx > t3_b2)
            t3_b2 = urx;
        if (ury > t3_b3)
            t3_b3 = ury;
    }
}

static void t3_write_glyph(internalfontnumber f)
{
    static char t3_begin_glyph_str[] = "\\pdfglyph";
    static char t3_end_glyph_str[] = "\\endglyph";
    int glyph_index;
    int width, height, depth, llx, lly, urx, ury;
    t3_getline();
    if (T3_PREFIX(t3_begin_glyph_str)) {
        if (sscanf(t3_line + strlen(t3_begin_glyph_str) + 1,
                   "%i %i %i %i %i %i %i %i =", &glyph_index,
                   &width, &height, &depth, &llx, &lly, &urx, &ury) != 8) {
            pdftex_warn("invalid glyph preamble: `%s'", t3_line);
            return;
        }
        if (glyph_index < fontbc[f] || glyph_index > fontec[f])
            return;
    }
    else
        return;
    if (!pdfischarused(f, glyph_index)) {
        while (!T3_PREFIX(t3_end_glyph_str)) {
            T3_CHECK_EOF();
            t3_getline();
        }
        return;
    }
    update_bbox(llx, lly, urx, ury);
    t3_glyph_num++;
    pdfnewdict(0, 0);
    t3_char_procs[glyph_index] = objptr;
    if (width == 0) 
        t3_char_widths[glyph_index] = t3_font_scale*
            ((float)getcharwidth(f, glyph_index)/pdffontsize[f]);
    else
        t3_char_widths[glyph_index] = width;
    pdfbeginstream();
    t3_getline();
    pdf_printf("%li 0 %li %li %li %li d1\nq\n", 
               (long int) t3_char_widths[glyph_index], (long int)llx,
               (long int)lly, (long int)urx, (long int)ury);
    while (!T3_PREFIX(t3_end_glyph_str)) {
        T3_CHECK_EOF();
        if (T3_PREFIX("BI"))
            t3_image_used = true;
        t3_putline();
        t3_getline();
    }
    pdf_printf("Q\n");
    pdfendstream();
}

boolean writepk(internalfontnumber f)
{
    kpse_glyph_file_type font_ret;
    integer res = cfgpkresolution();
    integer llx, lly, urx, ury;
    integer cw, rw, i, j;
    halfword *row;
    float width_scale;
    char *name;
    chardesc cd;
    integer dpi;
    if (res == 0)
        res = 600;
    dpi = 
        kpse_magstep_fix((unsigned)(res*((float)fontsize[f]/fontdsize[f])+0.5),
                         res, NULL);
    filename =  makecstring(fontname[f]);
    name = kpse_find_pk(filename, dpi, &font_ret);
    if (name == 0 ||
        !FILESTRCASEEQ(filename, font_ret.name) ||
        !kpse_bitmap_tolerance((double)font_ret.dpi, (double) dpi)) {
        pdftex_warn("Font %s at %li not found", filename, (long int)dpi);
        return false;
    }
    t3_font_type = T3_TYPE_PK;
    t3_image_used = true;
    t3_font_scale = (res/72.0)*((float)pdffontsize[f]/pdf1bp);
    width_scale = res/(72.0*pdf1bp);
    t3_file = xfopen(name, FOPEN_RBIN_MODE);
    tex_printf(" <%s", (char *)name);
    while (readchar((t3_glyph_num == 0), &cd) != 0) {
        t3_glyph_num++;
        if (!pdfischarused(f, cd.charcode) ||
            cd.cwidth < 1 || cd.cheight < 1) {
            XFREE(cd.raster);
            continue;
        }
        llx = -cd.xoff;
        lly = cd.yoff - cd.cheight + 1;
        urx = cd.cwidth + llx + 1;
        ury = cd.cheight + lly;
        update_bbox(llx, lly, urx, ury);
        pdfnewdict(0, 0);
        t3_char_procs[cd.charcode] = objptr;
        t3_char_widths[cd.charcode] = getcharwidth(f, cd.charcode)*width_scale;
        pdfbeginstream();
        pdf_printf("%li 0 %li %li %li %li d1\nq\n", (long int)cd.xescape, 
                   (long int)llx, (long int)lly, 
                   (long int)urx, (long int)ury);
        pdf_printf("%li 0 0 %li %li %li cm\nBI\n", (long int)cd.cwidth,
                   (long int)cd.cheight, (long int)llx, (long int)lly);
        pdf_printf("/W %li\n/H %li\n", 
                   (long int)cd.cwidth, (long int)cd.cheight);
        pdf_printf("/IM true\n/BPC 1\n/D [1 0]\nID ");
        cw = (cd.cwidth + 7)/8;
        rw = (cd.cwidth + 15)/16;
        row = cd.raster;
        for (i = 0; i < cd.cheight; i++) {
            for (j = 0; j < rw - 1; j++) {
                pdfout(*row/256);
                pdfout(*row%256);
                row++;
            }
            pdfout(*row/256);
            if (2*rw == cw)
                pdfout(*row%256);
            row++;
        }
        XFREE(cd.raster);
        pdf_printf("\nEI\nQ\n");
        pdfendstream();
    }
    return true;
}

void writet3(int objnum, internalfontnumber f)
{
    static char t3_font_scale_str[] = "\\pdffontscale";
    int i;
    t3_glyph_num = 0;
    t3_font_type = T3_TYPE_PGC;
    t3_image_used = false;
    for (i = 0; i < MAX_CHAR_NUM; i++) {
        t3_char_procs[i] = 0;
        t3_char_widths[i] = 0;
    }
    packfilename(fontname[f], getnullstr(), maketexstring(".pgc"));
    filename = makecstring(makenamestring());
    if (!T3_OPEN()) {
        if (writepk(f))
            goto write_font_dict;
        else
            return;
    }
    tex_printf("<%s", (nameoffile+1));
    t3_getline();
    if (!T3_PREFIX(t3_font_scale_str) ||
        sscanf(t3_line + strlen(t3_font_scale_str) + 1, "%f", &t3_font_scale) != 1 ||
        t3_font_scale <= 0 || t3_font_scale > 1000 ) {
        pdftex_warn("missing or invalid font scale");
        T3_CLOSE();
        return;
    }
    while (!T3_EOF())
        t3_write_glyph(f);
write_font_dict:
    pdfbegindict(objnum); /* Type 3 font dictionary */
    pdf_printf("/Type /Font\n/Subtype /Type3\n");
    pdf_printf("/Name /F%li\n", (long int)f);
    pdf_printf("/FontMatrix [%.5g 0 0 %.5g 0 0]\n", 
               1/t3_font_scale, 1/t3_font_scale);
    pdf_printf("/%s [ %li %li %li %li ]\n", 
               font_keys[FONTBBOX1_CODE].pdfname, 
               (long int)t3_b0, (long int)t3_b1, 
               (long int)t3_b2, (long int)t3_b3);
    pdf_printf("/Resources << /ProcSet [ /PDF %s] >>\n", 
               t3_image_used ? "/ImageB " : "");
    pdf_printf("/FirstChar %li\n/LastChar %li\n", 
               (long int)fontbc[f], (long int)fontec[f]);
    pdf_printf("/Widths %li 0 R\n/Encoding %li 0 R\n/CharProcs %li 0 R\n", 
               (long int)(objptr + 1), (long int)(objptr + 2),
               (long int)(objptr + 3));
    pdf_printf(">> endobj\n");
    pdfnewobj(0, 0); /* chars width array */
    pdf_printf("[ ");
    for (i = fontbc[f]; i <= fontec[f]; i++)
        pdf_printf("%li ", (long int)t3_char_widths[i]);
    pdf_printf("]\nendobj\n");
    pdfnewdict(0, 0); /* encoding dictionary */
    pdf_printf("/Type /Encoding\n/Differences [ %li ", (long int)fontbc[f]);
    for (i = fontbc[f]; i <= fontec[f]; i++)
        pdf_printf("/a%li", (long int)i);
    pdf_printf(" ]\n>> endobj\n");
    pdfnewdict(0, 0); /* CharProcs dictionary */
    for (i = fontbc[f]; i <= fontec[f]; i++)
        if (t3_char_procs[i] != 0)
            pdf_printf("/a%li %li 0 R\n", 
                       (long int)i, (long int)t3_char_procs[i]);
    pdf_printf(">> endobj\n");
    T3_CLOSE();
    tex_printf(">");
}
