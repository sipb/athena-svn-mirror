#include "libpdftex.h"

typedef signed char     TTF_CHAR;
typedef unsigned char   TTF_BYTE;
typedef signed short    TTF_SHORT;
typedef unsigned short  TTF_USHORT;
typedef signed long     TTF_LONG;
typedef unsigned long   TTF_ULONG;
typedef unsigned long   TTF_FIXED;
typedef unsigned short  TTF_FUNIT;
typedef signed short    TTF_FWORD;
typedef unsigned short  TTF_UFWORD;
typedef unsigned short  TTF_F2DOT14;

#define TTF_CHAR_SIZE    1
#define TTF_BYTE_SIZE    1
#define TTF_SHORT_SIZE   2
#define TTF_USHORT_SIZE  2
#define TTF_LONG_SIZE    4
#define TTF_ULONG_SIZE   4
#define TTF_FIXED_SIZE   4
#define TTF_FWORD_SIZE   2
#define TTF_UFWORD_SIZE  2
#define TTF_F2DOT14_SIZE 2

#define ARG_1_AND_2_ARE_WORDS       (1<<0)
#define ARGS_ARE_XY_VALUES          (1<<1)
#define ROUND_XY_TO_GRID            (1<<2)
#define WE_HAVE_A_SCALE             (1<<3)
#define RESERVED                    (1<<4)
#define MORE_COMPONENTS             (1<<5)
#define WE_HAVE_AN_X_AND_Y_SCALE    (1<<6)
#define WE_HAVE_A_TWO_BY_TWO        (1<<7)
#define WE_HAVE_INSTRUCTIONS        (1<<8)
#define USE_MY_METRICS              (1<<9)

#define GET_NUM(t)      ((t)getnum(t##_SIZE))
#define SKIP(n)         getnum(n)

#define GET_CHAR()      GET_NUM(TTF_CHAR)
#define GET_BYTE()      GET_NUM(TTF_BYTE)
#define GET_SHORT()     GET_NUM(TTF_SHORT)
#define GET_USHORT()    GET_NUM(TTF_USHORT)
#define GET_LONG()      GET_NUM(TTF_LONG)
#define GET_ULONG()     GET_NUM(TTF_ULONG)
#define GET_FIXED()     GET_NUM(TTF_FIXED)
#define GET_FUNIT()     GET_NUM(TTF_FUNIT)
#define GET_FWORD()     GET_NUM(TTF_FWORD)
#define GET_UFWORD()    GET_NUM(TTF_UFWORD)
#define GET_F2DOT14()   GET_NUM(TTF_F2DOT14)

#define PUT_NUM(t,n)    ((t)putnum(t##_SIZE, n))

#define PUT_CHAR(n)     PUT_NUM(TTF_CHAR, n)
#define PUT_BYTE(n)     PUT_NUM(TTF_BYTE, n)
#define PUT_SHORT(n)    PUT_NUM(TTF_SHORT, n)
#define PUT_USHORT(n)   PUT_NUM(TTF_USHORT, n)
#define PUT_LONG(n)     PUT_NUM(TTF_LONG, n)
#define PUT_ULONG(n)    PUT_NUM(TTF_ULONG, n)
#define PUT_FIXED(n)    PUT_NUM(TTF_FIXED, n)
#define PUT_FUNIT(n)    PUT_NUM(TTF_FUNIT, n)
#define PUT_FWORD(n)    PUT_NUM(TTF_FWORD, n)
#define PUT_UFWORD(n)   PUT_NUM(TTF_UFWORD, n)
#define PUT_F2DOT14(n)  PUT_NUM(TTF_F2DOT14, n)

#define COPY_BYTE()     PUT_BYTE(GET_BYTE())
#define COPY_CHAR()     PUT_CHAR(GET_CHAR())
#define COPY_USHORT()   PUT_USHORT(GET_USHORT())
#define COPY_SHORT()    PUT_SHORT(GET_SHORT())
#define COPY_ULONG()    PUT_ULONG(GET_ULONG())
#define COPY_LONG()     PUT_LONG(GET_LONG())
#define COPY_FIXED()    PUT_FIXED(GET_FIXED())
#define COPY_FUNIT()    PUT_FUNIT(GET_FUNIT())
#define COPY_FWORD()    PUT_FWORD(GET_FWORD())
#define COPY_UFWORD()   PUT_UFWORD(GET_UFWORD())
#define COPY_F2DOT14()  PUT_F2DOT14(GET_F2DOT14())

#define NMACGLYPHS      258
#define TABDIR_OFF      12

typedef struct {
    char tag[4];
    TTF_ULONG checksum;
    TTF_ULONG offset;
    TTF_ULONG length;
} tabdir_entry;

typedef struct {
    TTF_LONG offset;
    TTF_LONG newoffset;
    TTF_UFWORD advWidth;
    TTF_FWORD lsb;
    char *name;             /* name of glyph */
    TTF_SHORT newindex;      /* new index of glyph in output file */
    TTF_USHORT name_index;   /* index of name as read from font file */
} glyph_entry;

typedef struct {
    char *name;             /* name of glyph */
    short newindex;         /* new index of glyph in output file */
} ttfenc_entry;

typedef struct {
    TTF_USHORT platform_id;
    TTF_USHORT encoding_id;
    TTF_USHORT language_id;
    TTF_USHORT name_id;
    TTF_USHORT length;
    TTF_USHORT offset;
} name_record;

typedef struct {
    TTF_USHORT platform_id;
    TTF_USHORT encoding_id;
    TTF_ULONG  offset;
    TTF_USHORT format;
} cmap_entry;

static TTF_USHORT ntabs;
static TTF_ULONG checksum;
static TTF_USHORT upem;
static TTF_FIXED post_format;
static TTF_SHORT loca_format;
static TTF_ULONG ttf_offset;
static TTF_ULONG last_glyf_offset;
static TTF_USHORT glyphs_count;
static TTF_USHORT new_glyphs_count;
static TTF_USHORT nhmtxs;
static TTF_USHORT new_ntabs;

/* 
 * reindexing glyphs is a bit unclear: `glyph_tab' contains glyphs in
 * original order in font file, `ttfenc_tab' is the new encoding vector and
 * `glyph_index' is the new order of glyphs. So n-th glyph in new order is
 * located at `glyph_tab[glyph_index[n]]'. The field `newindex' of entries in
 * both `glyph_tab' and `ttfenc_tab' contains the index of the corresponding
 * glyph...
 *
 */

static glyph_entry *glyph_tab;
static ttfenc_entry *ttfenc_tab;
static short *glyph_index;
static int ncmaptabs;
static cmap_entry *cmap_tab;
static name_record *name_tab;
static int name_record_num;
static char *name_storage;
static int name_storage_size;
static tabdir_entry *dir_tab;
static char *glyph_name_buf;
static int chkcount;
static FILE *ttf_file;
static char id_str[] = "index";

integer ttf_length;

#define INFILE ttf_file
#define OUTFILE pdffile

#define TTF_OPEN()      truetypebopenin(ttf_file)
#define TTF_CLOSE()     xfclose(ttf_file, filename)
#define TTF_GETCHAR()   xgetc(ttf_file)
#define TTF_EOF()       feof(ttf_file)

#include "macnames.c"

static char *newtabnames[] = {
    "OS/2",
    "PCLT",
    "cmap",
    "cvt ",
    "fpgm",
    "glyf",
    "head",
    "hhea",
    "hmtx",
    "loca",
    "maxp",
    "name",
    "post",
    "prep"
};

#define DEFAULT_NTABS       14

static unsigned char addchksm(unsigned char b)
{
    checksum += (b << (8*(4 - ++chkcount)));
    if (chkcount == 4)
        chkcount = 0;
    return b;
}

static long putnum(int s, long n)
{
    long i = n;
    char buf[TTF_LONG_SIZE + 1], *p = buf;
    while (s-- > 0) {
        *p++ = i & 0xFF;
        i >>= 8;
    }
    p--;
    while (p >= buf)
        xputc(addchksm(*p--), OUTFILE);
    return n;
}

static long getnum(int s)
{
    long i = 0;
    int c;
    while (s > 0) {
        if ((c = TTF_GETCHAR()) < 0)
            pdftex_fail("unexpected EOF");
        i = (i << 8) + c;
        s--;
    }
    return i;
}

static TTF_ULONG getchksm()
{
    if (chkcount != 0)
        putnum(4 - chkcount, 0);
    return checksum;
}

static long funit(long n)
{
    if (n < 0)
        return -((-n/upem)*1000 + ((-n%upem)*1000)/upem);
    else
        return (n/upem)*1000 + ((n%upem)*1000)/upem;
}

static void ncopy(int n)
{
    while (n-- > 0)
        COPY_BYTE();
}

static tabdir_entry *name_lookup(char *s, boolean required)
{
    tabdir_entry *tab;
    for (tab = dir_tab; tab - dir_tab < ntabs; tab++)
        if (strncmp(tab->tag, s, 4) == 0)
            break;
    if (tab - dir_tab == ntabs) {
        if (required)
            pdftex_fail("can't find table `%s'", s);
        else
            tab = 0;
    }
    return tab;
}

static tabdir_entry *seek_tab(char *name, TTF_LONG offset)
{
    tabdir_entry *tab = name_lookup(name, true);
    sprintf(print_buf, "%s while reading table `%s'", filename, name);
    xfseek(INFILE, tab->offset + offset, SEEK_SET, print_buf);
    return tab;
}

static void seek_off(char *name, TTF_LONG offset)
{
    sprintf(print_buf, "%s while reading table %s", filename, name);
    xfseek(INFILE, offset, SEEK_SET, print_buf);
}

static void copy_encoding()
{
    int i;
    char **n = cur_glyph_names = enc_tab[fm_cur->encoding].glyph_names;
    ttfenc_entry *e = ttfenc_tab = XTALLOC(MAX_CHAR_NUM, ttfenc_entry);
    for (i = 0; i < MAX_CHAR_NUM; i++) {
        if (pdfischarused(tex_font, i))
            e->name = *n;
        else
            e->name = notdef;
        n++;
        e++;
    }
}

#define TTF_APPEND_BYTE(B)\
do {\
    if (name_tab[i].platform_id == 3)\
        *q++ = 0;\
    *q++ = B;\
} while (0)

static void check_name()
{
    int i, l;
    static char *name_prefix = "Embedded";
    char  buf[1024], *p, *r, *s, *t;
    int new_name_storage_size = name_storage_size +
        name_record_num*7*2*(strlen(fm_cur->prefix) + 1);
    char *new_name_storage = XTALLOC(new_name_storage_size, char),
        *q = new_name_storage;
    for (i = 0; i < name_record_num; i++) {
        if (name_tab[i].platform_id == 1 && 
/*              name_tab[i].encoding_id == 0 && */
            name_tab[i].name_id == 6) {
            r = name_storage + name_tab[i].offset;
            l = name_tab[i].length;
            strcpy(buf, name_prefix);
            p = strend(buf);
            *p++ = '-';
            for (; l > 0; l--)
                *p++ = *r++;
            *p = 0;
            font_keys[FONTNAME_CODE].value.s = xstrdup(buf);
            font_keys[FONTNAME_CODE].valid = true;
        }
        r = name_storage + name_tab[i].offset;
        l = name_tab[i].length;
        name_tab[i].offset = q - new_name_storage;
        t = q;
        switch (name_tab[i].name_id) {
        case 1: case 3: case 4: case 6:
            s = name_prefix;
            for (; *s != 0; s++)
                TTF_APPEND_BYTE(*s);
        }
        switch (name_tab[i].name_id) {
        case 1: case 4:
            TTF_APPEND_BYTE(' ');
            break;
        case 3:
            TTF_APPEND_BYTE(':');
            TTF_APPEND_BYTE(' ');
            break;
        case 6:
            TTF_APPEND_BYTE('-');
            break;
        }
        for (; l > 0; l--)
            *q++ = *r++;
        name_tab[i].length = q - t;
    }
    XFREE(name_storage);
    name_storage = new_name_storage;
    name_storage_size = q - new_name_storage;
}

static void read_name()
{
    int i;
    tabdir_entry *tab = seek_tab("name", TTF_USHORT_SIZE);
    char *p;
    name_record_num = GET_USHORT();
    name_tab = XTALLOC(name_record_num, name_record);
    name_storage_size = tab->length - 
         (3*TTF_USHORT_SIZE + name_record_num*6*TTF_USHORT_SIZE);
    name_storage = XTALLOC(name_storage_size, char);
    SKIP(TTF_USHORT_SIZE);
    for (i = 0; i < name_record_num; i++) {
        name_tab[i].platform_id = GET_USHORT();
        name_tab[i].encoding_id = GET_USHORT();
        name_tab[i].language_id = GET_USHORT();
        name_tab[i].name_id = GET_USHORT();
        name_tab[i].length = GET_USHORT();
        name_tab[i].offset = GET_USHORT();
    }
    for (p = name_storage; p - name_storage < name_storage_size; p++)
         *p = GET_CHAR();
    check_name();
}

static void read_mapx()
{
    int i;
    glyph_entry *glyph;
    seek_tab("maxp", TTF_FIXED_SIZE);
    glyph_tab = XTALLOC(1 + (glyphs_count = GET_USHORT()), glyph_entry);
    for (glyph = glyph_tab; glyph - glyph_tab < glyphs_count; glyph++) {
        glyph->newindex = -1;
        glyph->newoffset = 0;
        glyph->name_index = 0;
        glyph->name = notdef;
    }
    glyph_index = XTALLOC(glyphs_count, short);
    glyph_index[0] = 0; /* index of ".notdef" glyph */
    glyph_index[1] = 1; /* index of ".null" glyph */
    for (i = 0; i < MAX_KEY_CODE; i++)
        if (i != FONTNAME_CODE)
            font_keys[i].valid = true;
}

static void read_head()
{
    seek_tab("head", 2*TTF_FIXED_SIZE + 2*TTF_ULONG_SIZE + TTF_USHORT_SIZE);
    upem = GET_USHORT();
    SKIP(16);
    font_keys[FONTBBOX1_CODE].value.i = funit(GET_FWORD());
    font_keys[FONTBBOX2_CODE].value.i = funit(GET_FWORD());
    font_keys[FONTBBOX3_CODE].value.i = funit(GET_FWORD());
    font_keys[FONTBBOX4_CODE].value.i = funit(GET_FWORD());
    SKIP(2*TTF_USHORT_SIZE + TTF_SHORT_SIZE);
    loca_format = GET_SHORT();
}

static void read_hhea()
{
    seek_tab("hhea", TTF_FIXED_SIZE);
    font_keys[ASCENT_CODE].value.i = funit(GET_FWORD());
    font_keys[DESCENT_CODE].value.i = funit(GET_FWORD());
    SKIP(TTF_FWORD_SIZE + TTF_UFWORD_SIZE + 3*TTF_FWORD_SIZE + 8*TTF_SHORT_SIZE);
    nhmtxs = GET_USHORT();
}

static void read_pclt()
{
    if (name_lookup("PCLT", false) != 0) {
        seek_tab("PCLT", TTF_FIXED_SIZE + TTF_ULONG_SIZE + TTF_USHORT_SIZE);
        font_keys[XHEIGHT_CODE].value.i = funit(GET_USHORT());
        SKIP(2*TTF_USHORT_SIZE);
        font_keys[CAPHEIGHT_CODE].value.i = funit(GET_USHORT());
    }
    else {
        font_keys[XHEIGHT_CODE].valid = false;
        font_keys[CAPHEIGHT_CODE].valid = false;
    }
}

static void read_hmtx()
{
    glyph_entry *glyph;
    TTF_UFWORD last_advWidth;
    seek_tab("hmtx", 0);
    for (glyph = glyph_tab; glyph - glyph_tab < nhmtxs; glyph++) {
        glyph->advWidth = GET_UFWORD();
        glyph->lsb = GET_UFWORD();
    }
    if (nhmtxs < glyphs_count) {
        last_advWidth = glyph[-1].advWidth;
        for (;glyph - glyph_tab <  glyphs_count; glyph++) {
            glyph->advWidth = last_advWidth;
            glyph->lsb = GET_UFWORD();
        }
    }
}

static void read_post()
{
    int k, nnames;
    long length;
    char *p;
    glyph_entry *glyph;
    tabdir_entry *tab = seek_tab("post", 0);
    post_format = GET_FIXED();
    font_keys[ITALIC_ANGLE_CODE].value.i = GET_FIXED() >> 16;
    SKIP(2*TTF_FWORD_SIZE + 5*TTF_ULONG_SIZE);
    switch (post_format) {
    case 0x10000:
        for (glyph = glyph_tab; glyph - glyph_tab < NMACGLYPHS; glyph++) {
            glyph->name = mac_glyph_names[glyph - glyph_tab];
            glyph->name_index = glyph - glyph_tab;
        }
        break;
    case 0x20000:
        nnames = GET_USHORT(); /* some fonts have this value different from nglyphs */
        for (glyph = glyph_tab; glyph - glyph_tab < nnames; glyph++)
            glyph->name_index = GET_USHORT();
        length = tab->length - (xftell(INFILE, filename) - tab->offset);
        glyph_name_buf = XTALLOC(length, char);
        for (p = glyph_name_buf; p - glyph_name_buf < length;) {
            for (k = GET_BYTE(); k > 0; k--)
                *p++ = GET_CHAR();
            *p++ = 0;
        }
        for (glyph = glyph_tab; glyph - glyph_tab < nnames; glyph++) {
            if (glyph->name_index < NMACGLYPHS)
                glyph->name = mac_glyph_names[glyph->name_index];
            else {
                p = glyph_name_buf;
                k = glyph->name_index - NMACGLYPHS;
                for (; k > 0; k--)
                    p = strend(p) + 1;
                glyph->name = p;
            }
        }
        break;
    default:
        pdftex_fail("unsupported format (%8X) of `post' table"
, (unsigned int) post_format);
    }
}

static void read_loca()
{
    glyph_entry *glyph;
    seek_tab("loca", 0);
    if (loca_format != 0)
        for (glyph = glyph_tab; glyph - glyph_tab < glyphs_count + 1; glyph++)
            glyph->offset = GET_ULONG();
    else
        for (glyph = glyph_tab; glyph - glyph_tab < glyphs_count + 1; glyph++)
            glyph->offset = GET_USHORT() << 1;
}

static void read_font()
{
    int i;
    long version;
    tabdir_entry *tab;
    if ((version= GET_FIXED()) != 0x00010000)
        pdftex_fail("unsupport version %8lx; can handle only version 1.0", version);
    dir_tab = XTALLOC(ntabs = GET_USHORT(), tabdir_entry);
    SKIP(3*TTF_USHORT_SIZE);
    for (tab = dir_tab; tab - dir_tab < ntabs; tab++) {
        for (i = 0; i < 4; i++)
            tab->tag[i] = GET_CHAR();
        tab->checksum = GET_ULONG();
        tab->offset = GET_ULONG();
        tab->length = GET_ULONG();
    }
    if (name_lookup("PCLT", false) == 0)
        new_ntabs--;
    if (name_lookup("fpgm", false) == 0)
        new_ntabs--;
    if (name_lookup("cvt ", false) == 0)
        new_ntabs--;
    if (name_lookup("prep", false) == 0)
        new_ntabs--;
    read_mapx();
    read_head();
    read_hhea();
    read_pclt();
    read_hmtx();
    read_post();
    read_loca();
    read_name();
}

#define RESET_CHKSM() do {                                 \
    checksum = 0;                                          \
    chkcount = 0;                                          \
    tab->offset = xftell(OUTFILE, filename) - ttf_offset;  \
} while (0)

#define SET_CHKSM() do {                                   \
    tab->length = xftell(OUTFILE, filename) - tab->offset - ttf_offset; \
    tab->checksum = getchksm();                            \
} while (0)

static void copytab(char *name)
{
    long i;
    tabdir_entry *tab = seek_tab(name, 0);
    RESET_CHKSM();
    for (i = tab->length; i > 0; i--)
        COPY_CHAR();
    SET_CHKSM();
}

#define BYTE_ENCODING_LENGTH  (MAX_CHAR_NUM*TTF_BYTE_SIZE + 3*TTF_USHORT_SIZE)

static void byte_encoding()
{
    ttfenc_entry *e;
    PUT_USHORT(0);  /* format number (0: byte encoding table) */
    PUT_USHORT(BYTE_ENCODING_LENGTH); /* length of table */
    PUT_USHORT(0);  /* version number */
    for (e = ttfenc_tab; e - ttfenc_tab < MAX_CHAR_NUM; e++)
        if (e->newindex < 256) {
            PUT_BYTE(e->newindex);
        }
        else {
            if (e->name != notdef)
                pdftex_warn("glyph `%s' has been mapped to `%s' in `byte_encoding' cmap table",
                     e->name, notdef);
            PUT_BYTE(0); /* notdef */
        }
}

#define TRIMMED_TABLE_MAP_LENGTH (TTF_USHORT_SIZE*(5 + MAX_CHAR_NUM))

static void trimmed_table_map()
{
    ttfenc_entry *e;
    PUT_USHORT(6);  /* format number (6): trimmed table mapping */
    PUT_USHORT(TRIMMED_TABLE_MAP_LENGTH);
    PUT_USHORT(0);  /* version number (0) */
    PUT_USHORT(0);  /* first character code */
    PUT_USHORT(MAX_CHAR_NUM);  /* number of character code in table */
    for (e = ttfenc_tab; e - ttfenc_tab < MAX_CHAR_NUM; e++)
        PUT_USHORT(e->newindex);
}

#define SEG_MAP_DELTA_LENGTH ((16 + MAX_CHAR_NUM)*TTF_USHORT_SIZE)

static void seg_map_delta()
{
    ttfenc_entry *e;
    PUT_USHORT(4);  /* format number (4: segment mapping to delta values) */
    PUT_USHORT(SEG_MAP_DELTA_LENGTH);
    PUT_USHORT(0);  /* version number */
    PUT_USHORT(4);  /* 2*segCount */
    PUT_USHORT(4);  /* searchRange */
    PUT_USHORT(1);  /* entrySelector */
    PUT_USHORT(0);  /* rangeShift */
    PUT_USHORT(255); /* endCount[0] */
    PUT_USHORT(0xFFFF); /* endCount[1] */
    PUT_USHORT(0); /* reversedPad */
    PUT_USHORT(0); /* startCount[0] */
    PUT_USHORT(0xFFFF); /* startCount[1] */
    PUT_USHORT(0); /* idDelta[0] */
    PUT_USHORT(1); /* idDelta[1] */
    PUT_USHORT(2*TTF_USHORT_SIZE); /* idRangeOffset[0] */
    PUT_USHORT(0); /* idRangeOffset[1] */
    for (e = ttfenc_tab; e - ttfenc_tab < MAX_CHAR_NUM; e++)
        PUT_USHORT(e->newindex);
}

#define CMAP_ENTRY_LENGTH (2*TTF_USHORT_SIZE + TTF_ULONG_SIZE)

void static select_cmap()
{
    int i, n;
    TTF_USHORT pid, eid, format;
    TTF_ULONG offset, cur_offset, cmap_offset;
    seek_tab("cmap", TTF_USHORT_SIZE); /* skip the table vesrion number (0) */
    n = GET_USHORT();
    cmap_tab = XTALLOC(n, cmap_entry);
    cur_offset = xftell(INFILE, filename);
    cmap_offset = cur_offset - 2*TTF_USHORT_SIZE;
    ncmaptabs = 0;
    for (i = 0; i < n; i++) {
        xfseek(INFILE, cur_offset, SEEK_SET, filename);
        pid = GET_USHORT();
        eid = GET_USHORT();
        offset = GET_ULONG();
        cur_offset += CMAP_ENTRY_LENGTH;
        xfseek(INFILE, cmap_offset + offset, SEEK_SET, filename);
        format = GET_USHORT();
        if (format == 0 || format == 4 || format == 6) {
            cmap_tab[ncmaptabs].platform_id  = pid;
/*          cmap_tab[ncmaptabs].encoding_id  = eid; */
            cmap_tab[ncmaptabs].encoding_id  = 0; /* symbol-like encoding; don't use any code page */
            cmap_tab[ncmaptabs].offset       = offset;
            cmap_tab[ncmaptabs].format       = format;
            if (format == 0 && new_glyphs_count > 256) {
                pdftex_warn("`byte encoding' subtable of cmap has been ignored (cannot map more than 254 characters");
                continue;
            }
            ncmaptabs++;
        }
        else
            pdftex_warn("format %i of encoding subtable unsupported", (int)format);
    }
}

static void write_cmap()
{
    cmap_entry *ce;
    long offset;
    tabdir_entry *tab = name_lookup("cmap", true);
    select_cmap();
    RESET_CHKSM();
    PUT_USHORT(0);  /* table version number (0) */
    PUT_USHORT(ncmaptabs);  /* number of encoding tables */
    offset = 2*TTF_USHORT_SIZE + ncmaptabs*CMAP_ENTRY_LENGTH;
    for (ce = cmap_tab; ce - cmap_tab < ncmaptabs; ce++) {
        ce->offset = offset;
        switch (ce->format) {
        case 0: 
            offset +=  BYTE_ENCODING_LENGTH;
            break;
        case 4: 
            offset +=  SEG_MAP_DELTA_LENGTH;
            break;
        case 6: 
            offset +=  TRIMMED_TABLE_MAP_LENGTH;
            break;
        default:
            pdftex_fail("invalid format (it should not have happened)");
        }
        PUT_USHORT(ce->platform_id);
        PUT_USHORT(ce->encoding_id);
        PUT_ULONG(ce->offset);
    }
    for (ce = cmap_tab; ce - cmap_tab < ncmaptabs; ce++) {
        switch (ce->format) {
        case 0: 
            byte_encoding();
            break;
        case 4: 
            seg_map_delta();
            break;
        case 6: 
            trimmed_table_map();
            break;
        }
    }
    SET_CHKSM();
    XFREE(cmap_tab);
}

static void write_name()
{
    int i;
    char *p;
    tabdir_entry *tab = name_lookup("name", true);
    RESET_CHKSM();
    PUT_USHORT(0); /* Format selector */
    PUT_USHORT(name_record_num);
    PUT_USHORT(3*TTF_USHORT_SIZE + name_record_num*6*TTF_USHORT_SIZE);
    for (i = 0; i < name_record_num; i++) {
        PUT_USHORT(name_tab[i].platform_id);
        PUT_USHORT(name_tab[i].encoding_id);
        PUT_USHORT(name_tab[i].language_id);
        PUT_USHORT(name_tab[i].name_id);
        PUT_USHORT(name_tab[i].length);
        PUT_USHORT(name_tab[i].offset);
    }
    for (p = name_storage; p - name_storage < name_storage_size; p++)
         PUT_CHAR(*p);
    SET_CHKSM();
}

static void copy_dirtab()
{
    tabdir_entry *tab;
    int i;
    fflush(OUTFILE);
    pdfgone += xftell(OUTFILE, filename) - ttf_offset;
    xfseek(OUTFILE, ttf_offset + TABDIR_OFF, SEEK_SET, filename);
    if (is_subsetted()) 
        for (i = 0; i < DEFAULT_NTABS; i++) {
            tab = name_lookup(newtabnames[i], false);
            if (tab == 0)
                continue;
            for (k = 0; k < 4; k++)
               PUT_CHAR(tab->tag[k]);
            PUT_ULONG(tab->checksum);
            PUT_ULONG(tab->offset);
            PUT_ULONG(tab->length);
        }
    else
        for (tab = dir_tab; tab - dir_tab < ntabs; tab++) {
            for (k = 0; k < 4; k++)
               PUT_CHAR(tab->tag[k]);
            PUT_ULONG(tab->checksum);
            PUT_ULONG(tab->offset);
            PUT_ULONG(tab->length);
        }
}

static void write_glyf()
{
    short *id, k;
    TTF_USHORT idx;
    TTF_USHORT flags;
    tabdir_entry *tab = name_lookup("glyf", true);
    long glyf_offset = tab->offset;
    long new_glyf_offset = xftell(OUTFILE, filename);
    RESET_CHKSM();
    for (id = glyph_index; id - glyph_index < new_glyphs_count; id++) {
        glyph_tab[*id].newoffset = xftell(OUTFILE, filename) - new_glyf_offset;
        if (glyph_tab[*id].offset != glyph_tab[*id + 1].offset) {
            seek_off("glyph", glyf_offset + glyph_tab[*id].offset);
            k = COPY_SHORT();
            ncopy(4*TTF_FWORD_SIZE);
            if (k < 0) {
                do {
                    flags = COPY_USHORT();
                    idx = GET_USHORT();
                    if (glyph_tab[idx].newindex < 0) {
                        glyph_tab[idx].newindex = new_glyphs_count;
                        glyph_index[new_glyphs_count++] = idx;
                        /* 
                            NOTICE: Here we change `new_glyphs_count',
                            which appears in the condition of the `for' loop
                        */
                    }
                    PUT_USHORT(glyph_tab[idx].newindex);
                    if (flags & ARG_1_AND_2_ARE_WORDS)
                        ncopy(2*TTF_SHORT_SIZE);
                    else
                        ncopy(TTF_USHORT_SIZE);
                    if (flags & WE_HAVE_A_SCALE)
                        ncopy(TTF_F2DOT14_SIZE);
                    else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
                        ncopy(2*TTF_F2DOT14_SIZE);
                    else if (flags & WE_HAVE_A_TWO_BY_TWO)
                        ncopy(4*TTF_F2DOT14_SIZE);
                } while (flags & MORE_COMPONENTS);
                if (flags & WE_HAVE_INSTRUCTIONS)
                    ncopy(COPY_USHORT());
            }
            else
                ncopy(glyph_tab[*id + 1].offset - glyph_tab[*id].offset - 
                    TTF_USHORT_SIZE - 4*TTF_FWORD_SIZE);
        }
    }
    last_glyf_offset = xftell(OUTFILE, filename) - new_glyf_offset;
    SET_CHKSM();
}

static void reindex_glyphs()
{
    ttfenc_entry *e;
    glyph_entry *glyph;
    int i, l = strlen(id_str);
    /* 
     * reindexing glyphs: we append index of used glyphs to `glyph_index'
     * while going through `ttfenc_tab'. After appending a new entry to
     * `glyph_index' we set field `newindex' of corresponding entries in both
     * `glyph_tab' and `ttfenc_tab' to the newly created index
     * 
     */
    for (e = ttfenc_tab; e - ttfenc_tab < MAX_CHAR_NUM; e++) {
        e->newindex = 0; /* index of ".notdef" glyph */
        if (e->name != notdef) {
            if (strncmp(e->name, id_str, l) == 0) {
                i = atoi(e->name + l);
                if (i < glyphs_count)
                    glyph = glyph_tab + i;
                else
                    glyph = glyph_tab + glyphs_count;
            }
            else {
                for (glyph = glyph_tab; glyph - glyph_tab < glyphs_count; glyph++)
                    if (glyph->name != notdef && strcmp(glyph->name, e->name) == 0)
                            break;
            }
            if (glyph - glyph_tab < glyphs_count) {
                if (glyph->newindex < 0) {
                    glyph_index[new_glyphs_count] = glyph - glyph_tab;
                    glyph->newindex = new_glyphs_count;
                    new_glyphs_count++;
                }
                e->newindex = glyph->newindex;
            }
            else
                pdftex_warn("glyph `%s' not found", e->name);
        }
    }
}
 
static void write_head()
{
    tabdir_entry *tab;
    tab = seek_tab("head", 0);
    RESET_CHKSM();
    ncopy(2*TTF_FIXED_SIZE + 2*TTF_ULONG_SIZE + 2*TTF_USHORT_SIZE + 16 + 
        4*TTF_FWORD_SIZE + 2*TTF_USHORT_SIZE + TTF_SHORT_SIZE);
    PUT_SHORT(loca_format);
    PUT_SHORT(0);
    SET_CHKSM();
}
 
static void write_hhea()
{
    tabdir_entry *tab;
    tab = seek_tab("hhea", 0);
    RESET_CHKSM();
    ncopy(TTF_FIXED_SIZE + 3*TTF_FWORD_SIZE + TTF_UFWORD_SIZE + 3*TTF_FWORD_SIZE + 8*TTF_SHORT_SIZE);
    PUT_USHORT(new_glyphs_count);
    SET_CHKSM();
}

static void write_htmx()
{
    short *id;
    tabdir_entry *tab = seek_tab("hmtx", 0);
    RESET_CHKSM();
    for (id = glyph_index; id - glyph_index < new_glyphs_count; id++) {
        PUT_UFWORD(glyph_tab[*id].advWidth);
        PUT_UFWORD(glyph_tab[*id].lsb);
    }
    SET_CHKSM();
}

static void write_loca()
{
    short *id;
    tabdir_entry *tab = seek_tab("loca", 0);
    RESET_CHKSM();
    loca_format = 0;
    if (last_glyf_offset >= 0x00020000 || (last_glyf_offset & 1))
        loca_format = 1;
    else
        for (id = glyph_index; id - glyph_index < new_glyphs_count; id++)
            if (glyph_tab[*id].newoffset & 1) {
                loca_format = 1;
                break;
            }
    if (loca_format != 0) {
        for (id = glyph_index; id - glyph_index < new_glyphs_count; id++)
            PUT_ULONG(glyph_tab[*id].newoffset);
        PUT_ULONG(last_glyf_offset);
    }
    else {
        for (id = glyph_index; id - glyph_index < new_glyphs_count; id++)
            PUT_USHORT(glyph_tab[*id].newoffset/2);
        PUT_USHORT(last_glyf_offset/2);
    }
    SET_CHKSM();
}

static void write_mapx()
{
    tabdir_entry *tab = seek_tab("maxp", TTF_FIXED_SIZE + TTF_USHORT_SIZE);
    RESET_CHKSM();
    PUT_FIXED(0x00010000);
    PUT_USHORT(new_glyphs_count);
    ncopy(13*TTF_USHORT_SIZE);
    SET_CHKSM();
}

static void write_OS2()
{
    tabdir_entry *tab = seek_tab("OS/2", 0);
    TTF_USHORT version;
    RESET_CHKSM();
    version = GET_USHORT();
    if (version != 0x0000 && version != 0x0001)
        pdftex_fail("unknown verssion of OS/2 table (%4X)", version);
    PUT_USHORT(0x0001); /* fix version to 1*/
    ncopy(2*TTF_USHORT_SIZE + 13*TTF_SHORT_SIZE + 10*TTF_BYTE_SIZE);
    SKIP(4*TTF_ULONG_SIZE); /* ulUnicodeRange 1--4 */
    PUT_ULONG(0x00000003); /* Basic Latin + Latin-1 Supplement (0x0000--0x00FF) */
    PUT_ULONG(0x10000000); /* Private Use (0xE000--0xF8FF) */
    PUT_ULONG(0x00000000);
    PUT_ULONG(0x00000000);
    ncopy(4*TTF_CHAR_SIZE + TTF_USHORT_SIZE); /* achVendID + fsSelection */
    SKIP(2*TTF_USHORT_SIZE);
    PUT_USHORT(0x0000); /* usFirstCharIndex */
    PUT_USHORT(0xF0FF); /* usLastCharIndex */
    ncopy(5*TTF_USHORT_SIZE);
    /* for version 0 the OS/2 table ends here, the rest is for version 1 */ 
    PUT_ULONG(0x80000000); /* Symbol Character Set---don't use any code page */
    PUT_ULONG(0x00000000);
    SET_CHKSM();
}

static void write_post()
{
    int i;
    char *p;
    short *id;
    tabdir_entry *tab = seek_tab("post", TTF_FIXED_SIZE);
    RESET_CHKSM();
    PUT_FIXED(0x00020000);
    ncopy(TTF_FIXED_SIZE + 2*TTF_FWORD_SIZE + 5*TTF_ULONG_SIZE);
    PUT_USHORT(new_glyphs_count);
    for (k = 0, id = glyph_index; id - glyph_index < new_glyphs_count; id++) {
        i = glyph_tab[*id].name_index;
        if (i >= NMACGLYPHS)
            i = NMACGLYPHS + k++;
        PUT_USHORT(i);
    }
    for (id = glyph_index; id - glyph_index < new_glyphs_count; id++)
        if (glyph_tab[*id].name_index >= NMACGLYPHS) {
            p = glyph_tab[*id].name;
            i = strlen(p);
            PUT_BYTE(i);
            while (i-- > 0)
                PUT_CHAR(*p++);
        }
    SET_CHKSM();
}

static void init_font(int n)
{
    int i, k;
    ttf_offset = xftell(OUTFILE, filename);
    for (i = 1, k = 0; i <= new_ntabs; i <<= 1, k++);
    PUT_FIXED(0x00010000); /* font version */
    PUT_USHORT(n); /* number of tables */
    PUT_USHORT(i << 3); /* search range */
    PUT_USHORT(k - 1); /* entry selector */
    PUT_USHORT((n<<4) - (i<<3)); /* range shift */
    xfseek(OUTFILE, ttf_offset + TABDIR_OFF + n*4*TTF_ULONG_SIZE,
        SEEK_SET, filename);
}

static void subset_font()
{
    init_font(new_ntabs);
    if (name_lookup("PCLT", false) != 0)
        copytab("PCLT");
    if (name_lookup("fpgm", false) != 0)
        copytab("fpgm");
    if (name_lookup("cvt ", false) != 0)
        copytab("cvt ");
    if (name_lookup("prep", false) != 0)
        copytab("prep");
    reindex_glyphs();
    write_glyf();
    write_loca();
    write_OS2();
    write_head();
    write_hhea();
    write_htmx();
    write_mapx();
    write_name();
    write_post();
    write_cmap();
    copy_dirtab();
}

static void copy_font()
{
    tabdir_entry *tab;
    init_font(ntabs);
    for (tab = dir_tab; tab - dir_tab < ntabs; tab++)
        copytab(tab->tag);
    copy_dirtab();
}

void writettf()
{
    filename = fm_cur->ff_name;
    packfilename(maketexstring(filename), getnullstr(), getnullstr());
    if (!TTF_OPEN()) {
        pdftex_warn("cannot open TrueType font file for reading");
        font_file_not_found = true;
        return;
    }
    if (is_subsetted() && !is_reencoded()) {
        pdftex_warn("encoding vector required for TrueType font subsetting");
        return;
    }
    tex_printf("<%s", fm_cur->ff_name);
    glyph_name_buf = 0;
    new_glyphs_count = 2;
    new_ntabs = DEFAULT_NTABS;
    dir_tab = 0;
    glyph_tab = 0;
    glyph_index = 0;
    glyph_name_buf = 0;
    name_tab = 0;
    name_storage = 0;
    ttfenc_tab = 0;
    read_font();
    if (is_included()) {
        pdfsaveoffset = pdfoffset();
        pdfflush();
        if (is_reencoded())
            copy_encoding();
        if (is_subsetted())
            subset_font();
        else
            copy_font();
        xfflush(OUTFILE);
        xfseek(OUTFILE, pdfgone, SEEK_SET, filename);
        ttf_length = pdfoffset() - pdfsaveoffset;
        if (is_reencoded())
            XFREE(ttfenc_tab);
    }
    XFREE(dir_tab);
    XFREE(glyph_tab);
    XFREE(glyph_index);
    XFREE(glyph_name_buf);
    XFREE(name_tab);
    XFREE(name_storage);
    TTF_CLOSE();
    tex_printf(">");
}
