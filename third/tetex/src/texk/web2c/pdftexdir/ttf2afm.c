#include <kpathsea/c-auto.h>
#include <kpathsea/c-fopen.h>
#include <kpathsea/c-limits.h>
#include <kpathsea/c-memstr.h>
#include <kpathsea/c-proto.h>
#include <kpathsea/c-std.h>
#include <kpathsea/c-unistd.h>
#include <kpathsea/c-vararg.h>
#include <kpathsea/getopt.h>
#include <time.h>

typedef unsigned char   TTF_BYTE;
typedef signed char     TTF_CHAR;
typedef unsigned short  TTF_USHORT;
typedef signed short    TTF_SHORT;
typedef unsigned long   TTF_ULONG;
typedef signed long     TTF_LONG;
typedef unsigned long   TTF_FIXED;
typedef unsigned short  TTF_FUNIT;
typedef signed short    TTF_FWORD;
typedef unsigned short  TTF_UFWORD;
typedef unsigned short  TTF_F2DOT14;

typedef struct {
    char tag[4];
    TTF_ULONG checksum;
    TTF_ULONG offset;
    TTF_ULONG length;
} dirtab_entry;

typedef struct {
    TTF_ULONG wx;
    char *name;
    TTF_USHORT index;
    TTF_LONG bbox[4];
    TTF_LONG offset;
    char found;
} mtx_entry;

typedef struct _kern_entry {
    TTF_FWORD value;
    TTF_USHORT adjacent;
    struct _kern_entry *next;
} kern_entry;

typedef struct {
    TTF_USHORT platform_id;
    TTF_USHORT encoding_id;
    TTF_ULONG  offset;
} cmap_entry;

typedef struct {
    TTF_USHORT endCode;
    TTF_USHORT startCode;
    TTF_USHORT idDelta;
    TTF_USHORT idRangeOffset;
} seg_entry;


#define TTF_BYTE_SIZE       1
#define TTF_CHAR_SIZE       1
#define TTF_USHORT_SIZE     2
#define TTF_SHORT_SIZE      2
#define TTF_ULONG_SIZE      4
#define TTF_LONG_SIZE       4
#define TTF_FIXED_SIZE      4
#define TTF_FWORD_SIZE      2
#define TTF_UFWORD_SIZE     2
#define TTF_F2DOT14_SIZE    2

#define GET_TYPE(t)         ((t)getnum(t##_SIZE))

#define GET_BYTE()      GET_TYPE(TTF_BYTE)
#define GET_CHAR()      GET_TYPE(TTF_CHAR)
#define GET_USHORT()    GET_TYPE(TTF_USHORT)
#define GET_SHORT()     GET_TYPE(TTF_SHORT)
#define GET_ULONG()     GET_TYPE(TTF_ULONG)
#define GET_LONG()      GET_TYPE(TTF_LONG)
#define GET_FIXED()     GET_TYPE(TTF_FIXED)
#define GET_FUNIT()     GET_TYPE(TTF_FUNIT)
#define GET_FWORD()     GET_TYPE(TTF_FWORD)
#define GET_UFWORD()    GET_TYPE(TTF_UFWORD)
#define GET_F2DOT14()   GET_TYPE(TTF_F2DOT14)

#define NTABS           24

#define CHECK_BUF(size, buf_size)                          \
    if ((size) >= buf_size - 2)                            \
        fail("buffer overflow [%i bytes]", buf_size)

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

#define XTALLOC(n, t) ((t *) xmalloc ((n) * sizeof (t)))
#define XFREE(P)            do { if (P != 0) free(P); } while (0)

#define strend(S)           strchr(S, 0)

#define NMACGLYPHS      258
#define MAX_CHAR_NUM    256
#define ENC_BUF_SIZE    1024

#define ENC_GETCHAR()   xgetc(encfile)
#define ENC_EOF()       feof(encfile)

#define SKIP(n)         getnum(n)
#define PRINT_STR(S)    if (S != 0) fprintf(outfile, #S " %s\n", S)
#define PRINT_DIMEN(N)  if (N != 0) fprintf(outfile, #N " %i\n", (int)GET_TTF_FUNIT(N))

#define GET_TTF_FUNIT(n) \
    (n < 0 ? -((-n/upem)*1000 + ((-n%upem)*1000)/upem) :\
    ((n/upem)*1000 + ((n%upem)*1000)/upem))

char *FontName = 0;
char *FullName = 0;
char *Notice = 0;
TTF_LONG ItalicAngle = 0;
TTF_LONG IsFixedPitch = 0;
TTF_LONG FontBBox1 = 0;
TTF_LONG FontBBox2 = 0;
TTF_LONG FontBBox3 = 0;
TTF_LONG FontBBox4 = 0;
TTF_LONG UnderlinePosition = 0;
TTF_LONG UnderlineThickness = 0;
TTF_LONG CapHeight = 0;
TTF_LONG XHeight = 0;
TTF_LONG Ascender = 0;
TTF_LONG Descender = 0;

char *filename = 0;
char *b_name = 0;
FILE *fontfile, *encfile, *outfile = 0;
char enc_line[ENC_BUF_SIZE];
int print_all = 0;
int print_index = 0;
int print_cmap = 0;

TTF_USHORT upem;
TTF_USHORT ntabs;
int nhmtx;
int post_format;
int loca_format;
int nglyphs;
int nkernpairs = 0;
int names_count = 0;
char *ps_glyphs_buf = 0;
dirtab_entry *dir_tab;
mtx_entry *mtx_tab;
kern_entry *kern_tab;
char *enc_names[MAX_CHAR_NUM];

#include "macnames.c"

void fail(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\nError: ttf2afm");
    if (filename)
        fprintf(stderr, "(file %s)", filename);
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(-1);
}

void warn(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\nWarning: ttf2afm");
    if (filename)
        fprintf(stderr, "(file %s)", filename);
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}


void *xmalloc(unsigned long size)
{
    void *p = (void *)malloc(size);
    if (p == 0)
        fail("malloc() failed (%lu bytes)", (unsigned long)size);
    return p;
}

FILE *xfopen(char *name, char *mode)
{
    FILE *f = fopen(name, mode);
    if (f == 0)
        fail("fopen() failed");
    return f;
}

void xfclose(FILE *f)
{
  if (fclose (f) != 0)
        fail("fclose() failed");
}

long xftell(FILE *f)
{
    long offset = ftell(f);
    if (offset < 0)
        fail("ftell() failed");
    return offset;
}

int xgetc(FILE *stream)
{
    int c = getc(stream);
    if (c < 0 && c != EOF)
        fail("getc() failed");
    return c;
}

char *xstrdup(char *s)
{
    char *p = XTALLOC(strlen(s) + 1, char);
    return strcpy(p, s);
}

long getnum(int s)
{
    long i = 0;
    int c;
    while (s > 0) {
        if ((c = xgetc(fontfile)) < 0)
            fail("unexpected EOF");
        i = (i << 8) + c;
        s--;
    }
    return i;
}

dirtab_entry *name_lookup(char *s)
{
    dirtab_entry *p;
    for (p = dir_tab; p - dir_tab < ntabs; p++)
        if (strncmp(p->tag, s, 4) == 0)
            break;
    if (p - dir_tab == ntabs)
        p = 0;
    return p;
}

void seek_tab(char *name, TTF_LONG offset)
{
    dirtab_entry *p = name_lookup(name);
    if (p == 0)
        fail("can't find table `%s'", name);
    if (fseek(fontfile, p->offset + offset, SEEK_SET) < 0)
        fail("fseek() failed while reading `%s' table", name);
}

void seek_off(char *name, TTF_LONG offset)
{
    if (fseek(fontfile, offset, SEEK_SET) < 0)
        fail("fseek() failed while reading `%s' table", name);
}

void store_kern_value(TTF_USHORT i, TTF_USHORT j, TTF_FWORD v)
{
    kern_entry *pk;
    for (pk = kern_tab + i; pk->next != 0; pk = pk->next);
    pk->next = XTALLOC(1, kern_entry);
    pk = pk->next;
    pk->next = 0;
    pk->adjacent = j;
    pk->value = v;
}

TTF_FWORD get_kern_value(TTF_USHORT i, TTF_USHORT j)
{
    kern_entry *pk;
    for (pk = kern_tab + i; pk->next != 0; pk = pk->next)
        if (pk->adjacent == j)
            return pk->value;
    return 0;
}

void free_tabs()
{
    int i;
    kern_entry *p, *q, *r;
    XFREE(ps_glyphs_buf);
    XFREE(dir_tab);
    XFREE(mtx_tab);
    for (i = 0; i < MAX_CHAR_NUM; i++)
        if (enc_names[i] != notdef)
            free(enc_names[i]);
    for (p = kern_tab; p - kern_tab < nglyphs; p++)
        if (p->next != 0) {
            for (q = p->next; q != 0; q = r) {
                r = q->next;
                XFREE(q);
            }
        }
    XFREE(kern_tab);
}

void enc_getline()
{
    char *p;
    int c;
restart:
    if (ENC_EOF())
        fail("unexpected end of file");
    p = enc_line;
    do {
        c = ENC_GETCHAR();
        APPEND_CHAR_TO_BUF(c, p, enc_line, ENC_BUF_SIZE);
    } while (c != 10);
    APPEND_EOL(p, enc_line, ENC_BUF_SIZE);
    if (p - enc_line <= 2 || *enc_line == '%')
        goto restart;
}

void read_encoding(char *encname)
{
    char buf[ENC_BUF_SIZE], *q, *r;
    int i;
    filename = encname;
    if ((encfile = xfopen(encname, FOPEN_RBIN_MODE)) == 0)
        fail("can't open encoding file for reading");
    enc_getline();
    if (*enc_line != '/' || (r = strchr(enc_line, '[')) == NULL)
        fail("invalid encoding vector: name or `[' missing:\n%s", enc_line);
    for (i = 0; i < MAX_CHAR_NUM; i++)
        enc_names[i] = notdef;
    if (r[1] == 32)
        r += 2;
    else
        r++;
    for (;;) {
        while (*r == '/') {
            for (q = buf, r++; *r != 32 && *r != 10 && *r != ']' && *r != '/'; *q++ = *r++);
            *q = 0;
            if (*r == 32)
                r++;
            if (strcmp(buf, notdef) != 0)
                enc_names[names_count] = xstrdup(buf);
            if (names_count++ >= MAX_CHAR_NUM)
                fail("encoding vector contains more than %i names",
                     (int)MAX_CHAR_NUM);
        }
        if (*r != 10 && *r != '%')
            if (strncmp(r, "] def", strlen("] def")) == 0)
                goto done;
            else
                fail("invalid encoding vector: a name or `] def' expected:\n%s", enc_line);
        enc_getline();
        r = enc_line;
    }
done:
    xfclose(encfile);
}

void read_font()
{
    long i, j, k, l, n, platform_id, encoding_id;
    TTF_FWORD kern_value;
    char buf[1024], *p;
    dirtab_entry *pd;
    kern_entry *pk;
    mtx_entry *pm;
    SKIP(TTF_FIXED_SIZE);
    ntabs = GET_USHORT();
    SKIP(3*TTF_USHORT_SIZE);
    dir_tab = XTALLOC(ntabs, dirtab_entry);
    for (pd = dir_tab; pd - dir_tab < ntabs; pd++) {
        pd->tag[0] = GET_CHAR();
        pd->tag[1] = GET_CHAR();
        pd->tag[2] = GET_CHAR();
        pd->tag[3] = GET_CHAR();
        SKIP(TTF_ULONG_SIZE);
        pd->offset = GET_ULONG();
        pd->length = GET_ULONG();
    }
    seek_tab("head", 2*TTF_FIXED_SIZE + 2*TTF_ULONG_SIZE + TTF_USHORT_SIZE);
    upem = GET_USHORT();
    SKIP(16);
    FontBBox1 = GET_FWORD();
    FontBBox2 = GET_FWORD();
    FontBBox3 = GET_FWORD();
    FontBBox4 = GET_FWORD();
    SKIP(TTF_USHORT_SIZE);
    SKIP(TTF_USHORT_SIZE + TTF_SHORT_SIZE);
    loca_format = GET_SHORT();
    seek_tab("maxp", TTF_FIXED_SIZE);
    nglyphs = GET_USHORT();
    mtx_tab = XTALLOC(nglyphs, mtx_entry);
    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
        pm->name = 0;
        pm->found = 0;
    }
    seek_tab("hhea", TTF_FIXED_SIZE);
    Ascender = GET_FWORD();
    Descender = GET_FWORD();
    SKIP(TTF_FWORD_SIZE + TTF_UFWORD_SIZE + 3*TTF_FWORD_SIZE + 8*TTF_SHORT_SIZE);
    nhmtx = GET_USHORT();
    seek_tab("hmtx", 0);
    for (pm = mtx_tab; pm - mtx_tab < nhmtx; pm++) {
        pm->wx = GET_UFWORD();
        SKIP(TTF_FWORD_SIZE);
    }
    i = pm[-1].wx;
    for (; pm - mtx_tab < nglyphs; pm++)
        pm->wx = i;
    seek_tab("post", 0);
    post_format = GET_FIXED();
    ItalicAngle = GET_FIXED();
    UnderlinePosition = GET_FWORD();
    UnderlineThickness = GET_FWORD();
    IsFixedPitch = GET_ULONG();
    SKIP(4*TTF_ULONG_SIZE);
    switch (post_format) {
    case 0x00010000:
        for (pm = mtx_tab; pm - mtx_tab < NMACGLYPHS; pm++)
            pm->name = mac_glyph_names[pm - mtx_tab];
        break;
    case 0x00020000:
        l = GET_USHORT(); /* some fonts have this value different from nglyphs */
        for (pm = mtx_tab; pm - mtx_tab < l; pm++)
            pm->index = GET_USHORT();
        if ((pd = name_lookup("post")) == 0)
            fail("can't find table `post'");
        n = pd->length - (xftell(fontfile) - pd->offset);
        ps_glyphs_buf = XTALLOC(n + 1, char);
        for (p = ps_glyphs_buf; p - ps_glyphs_buf < n;) {
            for (i = GET_BYTE(); i > 0; i--)
                *p++ = GET_CHAR();
            *p++ = 0;
        }
        for (pm = mtx_tab; pm - mtx_tab < l; pm++) {
            if (pm->index < NMACGLYPHS)
                pm->name = mac_glyph_names[pm->index];
            else {
                k = pm->index - NMACGLYPHS;
                for (p = ps_glyphs_buf; k > 0; k--)
                    p = (char *)strend(p) + 1;
                pm->name = p;
            }
        }
        break;
    default:
        fail("unsupported format (%8X) of `post' table", post_format);
    }
    seek_tab("loca", 0);
    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
        pm->offset = (loca_format == 1 ? GET_ULONG() : GET_USHORT() << 1);
    if ((pd = name_lookup("glyf")) == 0)
        fail("can't find table `glyf'");
    for (n = pd->offset, pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
        if (pm->offset != (pm + 1)->offset) {
            seek_off("glyf", n + pm->offset);
            SKIP(TTF_SHORT_SIZE);
            pm->bbox[0] = GET_FWORD();
            pm->bbox[1] = GET_FWORD();
            pm->bbox[2] = GET_FWORD();
            pm->bbox[3] = GET_FWORD();
        }
        else { /* get the BBox from .notdef */
            pm->bbox[0] = mtx_tab[0].bbox[0];
            pm->bbox[1] = mtx_tab[0].bbox[1];
            pm->bbox[2] = mtx_tab[0].bbox[2];
            pm->bbox[3] = mtx_tab[0].bbox[3];
        }
    seek_tab("name", TTF_USHORT_SIZE);
    i = ftell(fontfile);
    n = GET_USHORT();
    j = GET_USHORT() + i - TTF_USHORT_SIZE;
    i += 2*TTF_USHORT_SIZE;
    while (n-- > 0) {
        seek_off("name", i);
        platform_id = GET_USHORT();
        encoding_id = GET_USHORT();
        GET_USHORT(); /* skip language_id */
        k = GET_USHORT();
        l = GET_USHORT();
        if ((platform_id == 1 && encoding_id == 0) &&
            (k == 0 || k == 4 || k == 6)) {
            seek_off("name", j + GET_USHORT());
            for (p = buf; l-- > 0; p++)
                *p = GET_CHAR();
            *p++ = 0;
            p = xstrdup(buf);
            switch (k) {
            case 0:  Notice = p; break;
            case 4:  FullName = p; break;
            case 6:  FontName = p; break;
            }
            if (Notice != 0 && FullName != 0 && FontName != 0)
                break;
        }
        i += 6*TTF_USHORT_SIZE;
    }
    if ((pd = name_lookup("PCLT")) != 0) {
        seek_off("PCLT", pd->offset + TTF_FIXED_SIZE + TTF_ULONG_SIZE + TTF_USHORT_SIZE);
        XHeight = GET_USHORT();
        SKIP(2*TTF_USHORT_SIZE);
        CapHeight = GET_USHORT();
    }
    if ((pd = name_lookup("kern")) == 0)
        return;
    kern_tab = XTALLOC(nglyphs, kern_entry);
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++) {
        pk->next = 0;
        pk->value = 0;
    }
    seek_off("kern", pd->offset + TTF_USHORT_SIZE);
    for (n = GET_USHORT(); n > 0; n--) {
        SKIP(2*TTF_USHORT_SIZE);
        k = GET_USHORT();
        if (!(k & 1) || (k & 2) || (k & 4))
            return;
        if (k >> 8 != 0) {
            fprintf(stderr, "warning: only format 0 supported of `kern' \
            subtables, others are ignored\n");
            continue;
        }
        k = GET_USHORT();
        SKIP(3*TTF_USHORT_SIZE);
        while (k-- > 0) {
            i = GET_USHORT();
            j = GET_USHORT();
            kern_value = GET_FWORD();
            if (kern_value != 0) {
                store_kern_value(i, j, kern_value);
                nkernpairs++;
            }
        }
    }
}

int null_glyph(char *s)
{
    if (s != 0 &&
        (strcmp(s, ".null") == 0 ||
         strcmp(s, ".notdef") == 0))
        return 1;
    return 0;
}

void print_afm(char *date, char *fontname)
{
    int i, ncharmetrics;
    mtx_entry *pm;
    int new_nkernpairs;
    short mtx_index[MAX_CHAR_NUM], *idx;
    char **pe;
    kern_entry *pk, *qk;
    static char id_str[] = "index";
    int l = strlen(id_str);
    char buf[20];
    fprintf(outfile, "Comment Converted at %s by ttf2afm from font file `%s'", date, fontname);
    fprintf(outfile, "\nStartFontMetrics 2.0\n");
    PRINT_STR(FontName);
    PRINT_STR(FullName);
    PRINT_STR(Notice);
    fprintf(outfile, "ItalicAngle %i", (int)(ItalicAngle/0x10000));
    if (ItalicAngle%0x10000 > 0)
        fprintf(outfile, ".%i", (int)((ItalicAngle%0x10000)*1000)/0x10000);
    fprintf(outfile, "\n");
    fprintf(outfile, "IsFixedPitch %s\n", IsFixedPitch ? "true" : "false");
    fprintf(outfile, "FontBBox %i %i %i %i\n", 
            (int)GET_TTF_FUNIT(FontBBox1),
            (int)GET_TTF_FUNIT(FontBBox2),
            (int)GET_TTF_FUNIT(FontBBox3),
            (int)GET_TTF_FUNIT(FontBBox4));
    PRINT_DIMEN(UnderlinePosition);
    PRINT_DIMEN(UnderlineThickness);
    PRINT_DIMEN(CapHeight);
    PRINT_DIMEN(XHeight);
    PRINT_DIMEN(Ascender);
    PRINT_DIMEN(Descender);
    ncharmetrics = nglyphs;
    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
        if (null_glyph(pm->name))
            ncharmetrics--;
    if (names_count == 0) { /* external encoding vector not given */
        fprintf(outfile, "\nStartCharMetrics %u\n", ncharmetrics);
        for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
            if (null_glyph(pm->name))
                continue;
            pm->found = 1;
            fprintf(outfile, "C -1 ; WX %i ; N ", (int)GET_TTF_FUNIT(pm->wx));
            if (pm->name == 0 || print_index)
                fprintf(outfile, "index%i", (int)(pm - mtx_tab));
            else
                fprintf(outfile, "%s", pm->name);
            fprintf(outfile, " ; B %i %i %i %i ;\n", 
                       (int)GET_TTF_FUNIT(pm->bbox[0]),
                       (int)GET_TTF_FUNIT(pm->bbox[1]),
                       (int)GET_TTF_FUNIT(pm->bbox[2]),
                       (int)GET_TTF_FUNIT(pm->bbox[3]));
        }
    }
    else { /* external encoding vector given */
        for (idx = mtx_index; idx - mtx_index < MAX_CHAR_NUM; *idx++ = 0);
        if (!print_all)
            ncharmetrics = 0;
        for (pe = enc_names; pe - enc_names < names_count; pe++) {
            if (*pe == notdef)
                continue;
            if (strncmp(*pe, id_str, l) == 0) {
                i = atoi(*pe + l);
                for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
                    if (i == pm - mtx_tab)
                        break;
            }
            else 
                for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
                    if (pm->name != 0)
                        if (strcmp(*pe, pm->name) == 0)
                            break;
            if (pm - mtx_tab < nglyphs) {
                mtx_index[pe - enc_names] = pm - mtx_tab;
                if (pm->name == 0) {
                    sprintf(buf, "index%i", i);
                    pm->name = xstrdup(buf);
                }
                pm->found = 1;
                if (!print_all)
                    ncharmetrics++;
            }
            else
                warn("glyph `%s' not found", *pe);
        }
        fprintf(outfile, "\nStartCharMetrics %u\n", ncharmetrics);
        for (idx = mtx_index; idx - mtx_index < MAX_CHAR_NUM; idx++) {
            if (null_glyph(mtx_tab[*idx].name))
                continue;
            if (*idx != 0)
                if (mtx_tab[*idx].found == 1) {
                    fprintf(outfile, "C %d ; WX %i ; N ",
                           idx - mtx_index, 
                           (int)GET_TTF_FUNIT(mtx_tab[*idx].wx));
                    if (print_index)
                        fprintf(outfile, "index%i", (int)*idx);
                    else
                        fprintf(outfile, "%s", mtx_tab[*idx].name);
                    fprintf(outfile, " ; B %i %i %i %i ;\n", 
                           (int)GET_TTF_FUNIT(mtx_tab[*idx].bbox[0]),
                           (int)GET_TTF_FUNIT(mtx_tab[*idx].bbox[1]),
                           (int)GET_TTF_FUNIT(mtx_tab[*idx].bbox[2]),
                           (int)GET_TTF_FUNIT(mtx_tab[*idx].bbox[3]));
                }
        }
        if (print_all == 0)
            goto end_metrics;
        for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
            if (null_glyph(pm->name))
                continue;
            if (pm->found == 0) {
                fprintf(outfile, "C -1 ; WX %i ; N ",
                       (int)GET_TTF_FUNIT(pm->wx));
                if (pm->name == 0 || print_index)
                    fprintf(outfile, "index%i", (int)(pm - mtx_tab));
                else
                    fprintf(outfile, "%s", pm->name);
                fprintf(outfile, " ; B %i %i %i %i ;\n", 
                       (int)GET_TTF_FUNIT(pm->bbox[0]),
                       (int)GET_TTF_FUNIT(pm->bbox[1]),
                       (int)GET_TTF_FUNIT(pm->bbox[2]),
                       (int)GET_TTF_FUNIT(pm->bbox[3]));
            }
        }
    }
end_metrics:
    fprintf(outfile, "EndCharMetrics\n");
    if (nkernpairs == 0)
        goto end_kerns;
    new_nkernpairs = 0;
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++)
        if (!null_glyph(mtx_tab[pk-kern_tab].name) &&
            (print_all || mtx_tab[pk-kern_tab].found))
            for (qk = pk; qk != 0; qk = qk->next)
                if (qk->value != 0 &&
                    !null_glyph(mtx_tab[qk->adjacent].name) &&
                    (print_all || mtx_tab[qk->adjacent].found))
                    new_nkernpairs++;
    if (new_nkernpairs == 0)
        goto end_kerns;
    fprintf(outfile, "\nStartKernData\nStartKernPairs %i\n", new_nkernpairs);
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++)
        if (!null_glyph(mtx_tab[pk-kern_tab].name) &&
            (print_all || mtx_tab[pk-kern_tab].found))
            for (qk = pk; qk != 0; qk = qk->next)
                if (qk->value != 0 && 
                    !null_glyph(mtx_tab[qk->adjacent].name) &&
                    (print_all || mtx_tab[qk->adjacent].found)) {
                    fprintf(outfile, "KPX ");
                    if (mtx_tab[pk-kern_tab].name == 0 || print_index)
                        fprintf(outfile, "index%i ", (int)(pk - kern_tab));
                    else
                        fprintf(outfile, "%s ", mtx_tab[pk-kern_tab].name);
                    if (mtx_tab[qk->adjacent].name == 0 || print_index)
                        fprintf(outfile, "index%i ", (int)qk->adjacent);
                    else
                        fprintf(outfile, "%s ", mtx_tab[qk->adjacent].name);
                    fprintf(outfile, "%i\n", GET_TTF_FUNIT(qk->value));
                }
    fprintf(outfile, "EndKernPairs\nEndKernData\n");
end_kerns:
    fprintf(outfile, "EndFontMetrics\n");
}

int print_sep(FILE *file)
{
    static int names_counter = 0;
    if (++names_counter == 1) {
        fprintf(file, "\n");
        names_counter = 0;
        return 1;
    }
    fprintf(file, " ");
    return 0;
}

void print_encoding(char *fontname)
{
    long int netabs, i, k, length, last_sep;
    cmap_entry *cmap_tab, *e;
    seg_entry *seg_tab, *s;
    long cmap_offset;
    FILE *file;
    TTF_USHORT *glyphId, format, segCount;
    char *enc_name, *end_enc_name, *n;
    seek_tab("cmap", TTF_USHORT_SIZE); /* skip the table vesrion number (0) */
    netabs = GET_USHORT();
    cmap_offset = xftell(fontfile) - 2*TTF_USHORT_SIZE;
    cmap_tab = XTALLOC(netabs, cmap_entry);
    for (e = cmap_tab; e - cmap_tab < netabs; e++) {
        e->platform_id = GET_USHORT();
        e->encoding_id = GET_USHORT();
        e->offset = GET_ULONG();
    }
    enc_name = XTALLOC(strlen(b_name) + 20, char);
    strcpy(enc_name, b_name);
    end_enc_name = strend(enc_name);
    for (e = cmap_tab; e - cmap_tab < netabs; e++) {
        seek_off("cmap", cmap_offset + e->offset);
        format = GET_USHORT();
        if (format != 0 && format != 4) {
            warn("format %i of encoding subtable unsupported", (int)format);
            continue;
        }
        sprintf(end_enc_name, ".e%i%i", 
                (int)e->platform_id, (int)e->encoding_id);
        if ((file = xfopen(enc_name, FOPEN_W_MODE)) == 0)
            fail("cannot open file for writting (%s)\n", enc_name);
        fprintf(file, "%% Encoding table from font file %s\n", fontname);
        fprintf(file, "%% Platform ID %i\n", (int)e->platform_id);
        fprintf(file, "%% Encoding ID %i\n", (int)e->encoding_id);
        fprintf(file, "%% Format %i\n", (int)(format));
        fprintf(file, "/Encoding%i [\n", (int)(e - cmap_tab + 1));
        switch (format) {
        case 0:
            GET_USHORT(); /* skip length */
            GET_USHORT(); /* skip version number */
            for (i = 0; i < 256; i++) {
                k = GET_BYTE();
                n = mtx_tab[k].name;
                if (n == 0 || (print_index && n != notdef))
                    fprintf(file, "/index%i", (int)k);
                else
                    fprintf(file, "/%s", n);
                last_sep = print_sep(file);
            }
            break;
        case 4:
            length = GET_USHORT(); /* length of subtable */
            GET_USHORT(); /* skip the version number */
            segCount = GET_USHORT();
            segCount /= 2;
            GET_USHORT(); /* skip searchRange */
            GET_USHORT(); /* skip entrySelector */
            GET_USHORT(); /* skip rangeShift */
            seg_tab = XTALLOC(segCount, seg_entry);
            for (s = seg_tab; s - seg_tab < segCount; s++)
                s->endCode = GET_USHORT();
            GET_USHORT(); /* skip reversedPad */
            for (s = seg_tab; s - seg_tab < segCount; s++)
                s->startCode = GET_USHORT();
            for (s = seg_tab; s - seg_tab < segCount; s++)
                s->idDelta = GET_USHORT();
            for (s = seg_tab; s - seg_tab < segCount; s++)
                s->idRangeOffset = GET_USHORT();
            length -= 8*TTF_USHORT_SIZE + 4*segCount*TTF_USHORT_SIZE;
            glyphId = XTALLOC(length, TTF_USHORT);
            for (i = 0; i < length; i++)
                glyphId[i] = GET_USHORT();
            for (i = 0; i < MAX_CHAR_NUM; i++) {
                for (s = seg_tab; s - seg_tab < segCount; s++)
                    if (s->endCode >= i)
                        break;
                if (s - seg_tab < segCount && s->startCode <= i) {
                    if (s->idRangeOffset != 0) {
                        k = glyphId[(i-s->startCode) + s->idRangeOffset/2 - (segCount-(s-seg_tab))];
                        if (k != 0)
                            k = (k + s->idDelta) % 0xFFFF;
                    }
                    else
                        k = (s->idDelta + i) % 0XFFFF;
                    n = mtx_tab[k].name;
                    if (n == 0 || (print_index && n != notdef))
                        fprintf(file, "/index%i", (int)k);
                    else
                        fprintf(file, "/%s", n);
                }
                else
                    fprintf(file, "/%s", notdef);
                last_sep = print_sep(file);
            }
            if (last_sep == 0)
                fprintf(file, "\n");
            fprintf(file, "%% Characters with code larger than %i have been ignored:\n", 
                           (int)(MAX_CHAR_NUM - 1));
            for (s = seg_tab; s - seg_tab < segCount; s++) {
                if (s->endCode < MAX_CHAR_NUM)
                    continue;
                else if (s->endCode == 0xFFFF)
                    break;
                for (i = s->startCode; i <= s->endCode; i++) {
                    if (s->idRangeOffset != 0) {
                        k = glyphId[(i-s->startCode) + s->idRangeOffset/2 - (segCount-(s-seg_tab))];
                        if (k != 0)
                            k = (k + s->idDelta) % 0xFFFF;
                    }
                    else
                        k = s->idDelta + i;
                    fprintf(file, "%% %#x -> %i\n", (int)i, (int)k);
                }
            }
        }
        fprintf(file, "] def\n");
    }
    XFREE(enc_name);
}

void usage()
{
    filename = 0;
    fprintf(stderr,
        "Usage: ttf2afm [-a] [-c encname] [-i] [-e encoding]  [-o output] fontfile\n"
            "-a: print all glyphs\n"
            "-c encname: print encoding tables to `encname.eMN'\n"
            "-i: print glyph names in form `index123'\n"
            "-e encoding: use encoding from file `encoding'\n"
            "-o output: output to file `output' instead of stdout\n"
            "fontfile: the TrueType font\n");
    _exit(-1);
}

int main(int argc, char **argv)
{
    char date[128];
    time_t t = time(&t);
    int c;
    while ((c = getopt(argc, argv, "ac:e:io:")) != -1)
        switch(c) {
        case 'a':
            print_all = 1;
            break;
        case 'c':
            print_cmap = 1;
            b_name = xstrdup(optarg);
            break;
        case 'e':
            filename = optarg;
            read_encoding(filename);
            break;
        case 'i':
            print_index = 1;
            break;
        case 'o':
            if (outfile != 0)
                usage();
            filename = optarg;
            outfile = xfopen(filename, FOPEN_W_MODE);
            if (outfile == 0)
                fail("cannot open file for writting");
            break;
        default:
            usage();
        }
    if (argc - optind != 1)
        usage();
    sprintf(date, "%s\n", ctime(&t));
    *(char *)strchr(date, '\n') = 0;
    filename = argv[optind];
    if ((fontfile = fopen(filename, FOPEN_RBIN_MODE)) == 0)
        fail("can't open font file for reading");
    read_font();
    if (outfile == 0)
        outfile = stdout;
    print_afm(date, filename);
    if (print_cmap)
        print_encoding(filename);
    XFREE(Notice);
    XFREE(FullName);
    XFREE(FontName);
    XFREE(b_name);
    xfclose(fontfile);
    return 0;
}
