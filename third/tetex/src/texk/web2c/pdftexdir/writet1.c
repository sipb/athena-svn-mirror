#include "libpdftex.h"

integer t1_length1, t1_length2, t1_length3;

static char *standard_glyph_names[MAX_CHAR_NUM] = {
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, "space", "exclam", "quotedbl",
"numbersign", "dollar", "percent", "ampersand", "quoteright", "parenleft",
"parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
"zero", "one", "two", "three", "four", "five", "six", "seven", "eight",
"nine", "colon", "semicolon", "less", "equal", "greater", "question", "at",
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
"backslash", "bracketright", "asciicircum", "underscore", "quoteleft", "a",
"b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
"q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar",
"braceright", "asciitilde", notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, "exclamdown", "cent", "sterling", "fraction", "yen", "florin",
"section", "currency", "quotesingle", "quotedblleft", "guillemotleft",
"guilsinglleft", "guilsinglright", "fi", "fl", notdef, "endash", "dagger",
"daggerdbl", "periodcentered", notdef, "paragraph", "bullet",
"quotesinglbase", "quotedblbase", "quotedblright", "guillemotright",
"ellipsis", "perthousand", notdef, "questiondown", notdef, "grave", "acute",
"circumflex", "tilde", "macron", "breve", "dotaccent", "dieresis", notdef,
"ring", "cedilla", notdef, "hungarumlaut", "ogonek", "caron", "emdash",
notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef, notdef,
notdef, notdef, notdef, notdef, notdef, notdef, notdef, "AE", notdef,
"ordfeminine", notdef, notdef, notdef, notdef, "Lslash", "Oslash", "OE",
"ordmasculine", notdef, notdef, notdef, notdef, notdef, "ae", notdef, notdef,
notdef, "dotlessi", notdef, notdef, "lslash", "oslash", "oe", "germandbls",
notdef, notdef, notdef, notdef
};

#define T1_BUF_SIZE   4096

#define ENC_BUITIN    0
#define ENC_STANDARD  1

#define T1_TYPE_1     1
#define T1_TYPE_2     2
#define T1_TYPE_3     3

typedef unsigned char byte;

typedef struct {
    char *name;
    byte *data;
    unsigned short len; /* length of the whole string */
    unsigned short cslen; /* length of the encoded part of the string */
    boolean used;
} cs_entry;

static cs_entry *cs_tab, *cs_ptr;
static unsigned short t1_dr, t1_er;
static unsigned short t1_c1 = 52845, t1_c2 = 22719;
static unsigned short t1_cslen, t1_lenIV;
static char t1_line[T1_BUF_SIZE], *t1_line_ptr, *cs_start;
static boolean t1_in_eexec, t1_ispfa, t1_charstring;
static long t1_block_length;
static FILE *t1_file;

#define T1_OPEN()       typeonebopenin(t1_file)
#define T1_CLOSE()      xfclose(t1_file, filename)
#define T1_GETCHAR()    xgetc(t1_file)
#define T1_EOF()        feof(t1_file)
#define T1_PREFIX(s)    (!strncmp(t1_line, s, strlen(s)))
#define T1_PUTCHAR(c)   pdfout(c)
#define T1_CHARSTRINGS() strstr(t1_line, "/CharStrings")

#define T1_CHECK_EOF()                                     \
    if (T1_EOF())                                          \
        pdftex_fail("unexpected end of file");

#define T1_PRINTF(S) do {                                  \
    sprintf(t1_line, S);                                   \
    t1_line_ptr = strend(t1_line);                      \
    t1_putline();                                          \
} while (0)

static void t1_check_pfa()
{
    int c = T1_GETCHAR();
    if (c != 128)
        t1_ispfa = true;
    else 
        t1_ispfa = false;
    ungetc(c, t1_file);
}

static byte t1_getbyte()
{
    int c = T1_GETCHAR();
    if (t1_ispfa)
        return c;
    if (t1_block_length == 0) {
        if (c != 128)
            pdftex_fail("invalid marker");
        c = T1_GETCHAR();
        if (c == 3)
            pdftex_fail("unexpected end of file");
        t1_block_length = T1_GETCHAR() & 0xff;
        t1_block_length |= (T1_GETCHAR() & 0xff) << 8;
        t1_block_length |= (T1_GETCHAR() & 0xff) << 16;
        t1_block_length |= (T1_GETCHAR() & 0xff) << 24;
        c = T1_GETCHAR();
        T1_CHECK_EOF();
    }
    t1_block_length--;
    return c;
}

static int hexval(int c)
{
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= '0' && c <= '9')
        return c - '0';
    else
        return 0;
}

static byte edecrypt(byte cipher)
{
    byte plain;
    if (t1_ispfa) {
        while (cipher == 10 || cipher == 13)
            cipher = t1_getbyte();
        cipher = (hexval(cipher) << 4) + hexval(t1_getbyte());
    }
    plain = (cipher^(t1_dr >> 8));
    t1_dr = (cipher + t1_dr)*t1_c1 + t1_c2;
    return plain;
}

static byte cdecrypt(byte cipher, unsigned short *cr)
{
    byte plain = (cipher^(*cr >> 8));
    *cr = (cipher + *cr)*t1_c1 + t1_c2;
    return plain;
}

static byte eencrypt(byte plain)
{
    byte cipher = (plain^(t1_er >> 8));
    t1_er = (cipher + t1_er)*t1_c1 + t1_c2;
    return cipher;
}

static char *eol(char *s)
{
    char *p = strend(s);
    if (p - s > 1 && p[-1] != 10) {
        *p++ = 10;
        *p = 0;
    }
    return p;
}

static boolean t1_suffix(char *s) 
{
    char *s1 = t1_line_ptr - 1, 
         *s2 = strend(s) - 1;
    if (*s1 == 10)
        s1--;
    while (s1 >= t1_line && s2 >= s) {
        if (*s1-- != *s2--)
            return false;
    }
    return s1 >= t1_line - 1;
}

static void t1_getline() 
{
    int c, l;
    char *p;
restart:
    t1_line_ptr = t1_line;
    t1_cslen = 0;
    c = t1_getbyte();
    while (!T1_EOF()) {
        if (t1_in_eexec) 
            c = edecrypt(c);
        APPEND_CHAR_TO_BUF(c, t1_line_ptr, t1_line, T1_BUF_SIZE);
        if (c == 10)
            break;
        if (t1_charstring && !t1_cslen && 
            (t1_line_ptr - t1_line > 4) && 
            (t1_suffix(" RD ") || t1_suffix(" -| ")))
        {
            p = t1_line_ptr - 5;
            while (*p !=  ' ')
                p--;
            t1_cslen = l = atoi(p + 1);
            cs_start = t1_line_ptr;
            CHECK_BUF(t1_line_ptr - t1_line + l, T1_BUF_SIZE);
            while (l-- > 0) {
                *t1_line_ptr++ = edecrypt(t1_getbyte());
                T1_CHECK_EOF();
            }
        }
        c = t1_getbyte();
    }
    APPEND_EOL(t1_line_ptr, t1_line, T1_BUF_SIZE);
    if (t1_line_ptr - t1_line <= 1)
        goto restart;
}

void t1_putline() 
{
    char *p = t1_line;
    if (t1_line_ptr - t1_line <= 1)
        return;
    if (t1_in_eexec) 
        while (p < t1_line_ptr)
            T1_PUTCHAR(eencrypt(*p++));
    else 
        while (p < t1_line_ptr)
            T1_PUTCHAR(*p++);
}

void t1_modify_fm()
{
 /*
  * font matrix is given as six numbers a0..a5, which stands for the matrix
  * 
  *           a0 a1 0
  *     M =   a2 a3 0
  *           a4 a5 1
  * 
  * ExtendFont is given as
  * 
  *           e 0 0
  *     E =   0 1 0
  *           0 0 1
  * 
  * SlantFont is given as
  * 
  *           1 0 0
  *     S =   s 1 0
  *           0 0 1
  * 
  * and the final transformation is
  * 
  *                    e*a0        e*a1       0
  *     F =  E.S.M  =  s*e*a0+a2   s*e*a1+a3  0
  *                    a4          a5         1
  */
    double e, s, a[6], b[6];
    int i;
    char buf[1024], *p, *q, *r;
    if ((p = strchr(t1_line, '[')) == 0)
        if ((p = strchr(t1_line, '{')) == 0)
            pdftex_fail("an array expected: `%s'", t1_line);
    strncpy(buf, t1_line, p - t1_line + 1);
    for (i = 0, q = p + 1; i < 6; i++) {
        a[i] = strtod(q, &r);
        q = r + 1;
    }
    if (fm_cur->extend != 0)
        e = fm_cur->extend*1E-3;
    else
        e = 1;
    s = fm_cur->slant*1E-3;
    b[0] = e*a[0];
    b[1] = e*a[1];
    b[2] = s*e*a[0] + a[2];
    b[3] = s*e*a[1] + a[3];
    b[4] = a[4];
    b[5] = a[5];
    q = buf + (p - t1_line + 1);
    for (i = 0; i < 6; i++) {
        sprintf(q, "%G ", b[i]);
        q = strend(q);
    }
    strcpy(q, r);
    strcpy(t1_line, buf);
    t1_line_ptr = eol(t1_line);
}

void t1_modify_italic()
{
    double a;
    char buf[1024], *p, *r;
    if (fm_cur->slant == 0)
        return;
    p = strchr(t1_line, ' ');
    strncpy(buf, t1_line, p - t1_line + 1);
    a = strtod(p, &r);
    a = a - atan(fm_cur->slant*1E-3)*(180/M_PI);
    sprintf(buf + (p - t1_line + 1), "%.2g", a);
    strcpy(strend(buf), r);
    strcpy(t1_line, buf);
    t1_line_ptr = eol(t1_line);
    font_keys[ITALIC_ANGLE_CODE].value.i = (a > 0) ? a + 0.5 : a - 0.5;
    font_keys[ITALIC_ANGLE_CODE].valid = true;
}

void t1_scan_param() 
{
    int i, k;
    char *p, buf[1024], *q, *r;
    key_entry *key;
    if (*t1_line != '/')
        return;
    if (T1_PREFIX("/lenIV")) {
        t1_lenIV = atoi(strchr(t1_line, ' ') + 1);
        return;
    }
    if (is_extended() || is_slanted()) {
        if (strncmp(t1_line + 1, "UniqueID", strlen("UniqueID")) == 0) {
            t1_line_ptr = t1_line;
            return;
        }
        if (strncmp(t1_line + 1, "FontMatrix", strlen("FontMatrix")) == 0) {
            t1_modify_fm();
            return;
        }
        if (strncmp(t1_line + 1, "ItalicAngle", strlen("ItalicAngle")) == 0) {
            t1_modify_italic();
            return;
        }
    }
    for (key = font_keys; key - font_keys  < MAX_KEY_CODE; key++)
        if (!strncmp(t1_line + 1, key->t1name, strlen(key->t1name)))
          break;
    if (key - font_keys == MAX_KEY_CODE)
        return;
    key->valid = true;
    p = t1_line + strlen(key->t1name) + 1;
    if (*p == ' ')
        p++;
    if ((k = key - font_keys) == FONTNAME_CODE) {
        if (*p != '/')
            pdftex_fail("a name expected: `%s'", t1_line);
        r = ++p; /* skip the slash */
        for (q = buf; *p != ' ' && *p != 10; *q++ = *p++);
        *q = 0;
        if (is_extended()) {
            sprintf(q, "-Extend_%i", (int)fm_cur->extend);
        }
        if (is_slanted()) {
            sprintf(q, "-Slant_%i", (int)fm_cur->slant);
        }
        key->value.s = xstrdup(buf);
        if (is_included() && is_subsetted()) {
            strcpy(buf, p);
            sprintf(r, "%s+%s%s", fm_cur->prefix, key->value.s, buf);
            t1_line_ptr = eol(r);
        }
        return;
    }
    if ((k == STEMV_CODE ||  k == FONTBBOX1_CODE) &&
        (*p == '[' || *p == '{'))
        p++;
    if (k == FONTBBOX1_CODE) {
        for (i = 0; i < 4; i++) {
            key[i].value.i = atoi(p);
            if (*p == '-')
                p++;
            while (isdigit(*p++));
        }
        return;
    }
    key->value.i = atoi(p);
}

void t1_builtin_enc()
{
    int i, end_encoding = 0, counter = 0;
    char buf[1024], line[1024], *r, *q;
       /*
        * At this moment "/Encoding" is the prefix of t1_line
        * 
        * We have two possible forms of Encoding vector. The first case is
        * 
        *     /Encoding [/a /b /c...] readonly def
        * 
        * and the second case can look like
        * 
        *     /Encoding 256 array 0 1 255 {1 index exch /.notdef put} for
        *     dup 0 /x put
        *     dup 1 /y put
        *     ...
        *     readonly def
        */
    if (T1_PREFIX("/Encoding [") || T1_PREFIX("/Encoding[")) { /* the first case */
        r = strchr(t1_line, '[') + 1;
        strncpy(line, t1_line, r - t1_line);
        line[r - t1_line + 1] = 0;
        if (*r == 32)
            r++;
        for(;;) {
            while (*r == '/') {
                for (q = buf, r++; *r != 32 && *r != 10 && *r != ']' && *r != '/'; *q++ = *r++);
                *q = 0;
                if (*r == 32)
                    r++;
                if (strcmp(buf, notdef) != 0 && pdfischarused(tex_font, counter))
                    builtin_glyph_names[counter] = xstrdup(buf);
                sprintf(strend(line), "/%s", buf);
                counter++;
            }
            if (*r != 10 && *r != '%') {
                if (strncmp(r, "] def", strlen("] def")) == 0 ||
                    strncmp(r, "] readonly def", strlen("] readonly def")) == 0) {
                    end_encoding = 1;
                    strcat(line, r);
                }
                else
                    pdftex_fail("a name or `] def' or `] readonly def' expected: `%s'", t1_line);
            }
            strcpy(t1_line, line);
            t1_line_ptr = eol(t1_line);
            t1_putline();
            if (end_encoding)
                break;
            T1_CHECK_EOF();
            t1_getline();
            r = t1_line;
            *line = 0;
        }
    }
    else { /* the second case */
        t1_putline();
        do {
            T1_CHECK_EOF();
            t1_getline();
            if (sscanf(t1_line, "dup %u%256s put", &i, buf) == 2 && *buf == '/') {
                if (pdfischarused(tex_font, i))
                    builtin_glyph_names[i] = xstrdup(buf + 1); /* skip the slash */
                else
                    continue;
            }
            t1_putline();
        } while (!t1_suffix("def"));
    }
}

void cs_store()
{
    char *p, *q, buf[T1_BUF_SIZE];
    for (p = t1_line, q = buf; *p != ' '; *q++ = *p++);
    *q = 0;
    cs_ptr->name = xstrdup(buf + 1); /* don't store the slash */
    cs_ptr->used = false;
    memcpy(buf, cs_start - 4, t1_cslen + 4); /* copy " RD " + cs data to buf */
    for (p = cs_start + t1_cslen, q = buf + t1_cslen + 4; *p != 10; *q++ = *p++);
    /* copy the end of cs data to buf */
    *q++ = 10; 
    cs_ptr->len = q - buf;
    cs_ptr->cslen = t1_cslen;
    cs_ptr->data = XTALLOC(cs_ptr->len, byte);
    memcpy(cs_ptr->data, buf, cs_ptr->len);
    cs_ptr++;
}

#define cs_getchar() cdecrypt(*data++, &cr)

static void cs_mark(char *name)
{
    byte *data;
    int i, b;
    long val1=0, val2=0, cs_len;
    unsigned short cr;
    cs_entry *cs;
    for (cs = cs_tab; cs < cs_ptr; cs++) {
        if (!strcmp(cs->name, name)) {
            if (cs->used)
                return;
            else {
                cs->used = true;
                cr = 4330; 
                cs_len = cs->cslen;
                data = cs->data + 4;
                for (i = 0; i < t1_lenIV; i++, cs_len--)
                    cs_getchar();
                while (cs_len > 0) {
                    --cs_len;
                    b = cs_getchar();
                    if (b >= 32) {
                        val2 = val1;
                        if (b <= 246)
                            val1 = b - 139;
                        else if (b <= 250) {
                            --cs_len;
                            val1 = ((b - 247) << 8) + 108 + cs_getchar();
                        } 
                        else if (b <= 254) {
                            --cs_len;
                            val1 = -((b - 251) << 8) - 108 - cs_getchar();
                        } 
                        else if (b == 255) {
                            cs_len -= 4;
                            val1 =  (cs_getchar() & 0xff) << 24;
                            val1 |= (cs_getchar() & 0xff) << 16;
                            val1 |= (cs_getchar() & 0xff) <<  8;
                            val1 |= (cs_getchar() & 0xff) <<  0;
                            if (sizeof(int) > 4 && val1 & (1U << 31))
                                for (i = 4; i < sizeof(int); i++)
                                    val1 |= 0xff << (i * 8);
                        }
                    }
                    else if (b == 12) {
                        cs_len--;
                        if (cs_getchar() == 6) {
                            cs_mark(standard_glyph_names[val1]);
                            cs_mark(standard_glyph_names[val2]);
                        }
                    }
                }
                return;
            }
        }
    }
    pdftex_warn("glyph `%s' undefined", name);
}

void writet1()
{
    int i, size_pos, cs_dict_size, cs_count, encoding;
    char buf[T1_BUF_SIZE], *save_line1, *save_line2, *p;
    cs_entry *cs;
    if (fm_cur->mm_name != 0) {
        filename = fm_cur->mm_name;
        packfilename(maketexstring(filename), getnullstr(), getnullstr());
        if (T1_OPEN()) /* found mm instance */
            goto open_ok;
    }
    filename = fm_cur->ff_name;
    packfilename(maketexstring(filename), getnullstr(), getnullstr());
    if (!T1_OPEN()) {
        pdftex_warn("cannot open Type 1 font file for reading");
        font_file_not_found = true;
        return;
    }
    if (pdfexpandfont[f] != 0) { /* cannot open mm instance, use ExtendFont */
        if (fm_cur->extend == 0)
            fm_cur->extend = 1000;
        fm_cur->extend = xnoverd(fm_cur->extend, 1000 + pdfexpandfont[f], 1000);
    }
open_ok:
    tex_printf("<%s", fm_cur->ff_name);
    t1_lenIV = 4;
    t1_dr = 55665;
    t1_er = 55665;
    t1_in_eexec = false;
    t1_charstring = false;
    t1_charstring = false;
    t1_block_length = 0;
    t1_check_pfa();
    if (!is_included()) { /* scan parameters from font file */
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_scan_param();
        } while (!T1_PREFIX("currentfile eexec"));
        t1_in_eexec = true;
        for (i = 0; i < 4; i++) {
            T1_CHECK_EOF();
            edecrypt(t1_getbyte());
        }
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_scan_param();
        } while (!(T1_CHARSTRINGS() || T1_PREFIX("/Subrs")));
        tex_printf(">");
        T1_CLOSE();
        return;
    }
    if (!is_subsetted()) { /* include entire font */
        pdfsaveoffset = pdfoffset();
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_scan_param();
            t1_putline();
        } while (!T1_PREFIX("currentfile eexec"));
        t1_length1 = pdfoffset() - pdfsaveoffset;
        pdfsaveoffset = pdfoffset();
        t1_in_eexec = true;
        for (t1_line_ptr = t1_line, i = 0; i < 4; i++) {
            edecrypt(t1_getbyte());
            T1_CHECK_EOF();
            *t1_line_ptr++ = 0;
        }
        t1_putline(); /* to put the first four bytes */
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_scan_param();
            t1_putline();
        } while (!(T1_CHARSTRINGS() || T1_PREFIX("/Subrs")));
        t1_charstring = true;
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_putline();
        } while (!t1_suffix("mark currentfile closefile"));
        t1_length2 = pdfoffset() - pdfsaveoffset;
        pdfsaveoffset = pdfoffset();
        t1_in_eexec = false;
        t1_charstring = false;
        do {
            T1_CHECK_EOF();
            t1_getline();
            t1_putline();
        } while (!T1_PREFIX("cleartomark"));
        t1_length3 = pdfoffset() - pdfsaveoffset;
        tex_printf(">");
        T1_CLOSE();
        return;
    } 
    /* the rest is for partial downloading */
    pdfsaveoffset = pdfoffset();
    T1_CHECK_EOF();
    t1_getline();
    while (!T1_PREFIX("/Encoding")) {
        t1_scan_param();
        t1_putline();
        T1_CHECK_EOF();
        t1_getline();
    }
    if (t1_suffix("def")) {
        sscanf(t1_line + strlen("/Encoding"), "%256s", buf);
        if (!strcmp(buf, "StandardEncoding"))
            encoding = ENC_STANDARD;
        else 
            pdftex_fail("cannot subset font (unknown predefined encoding `%s')"
, buf);
    }
    else {
        encoding = ENC_BUITIN; /* the font has its own built-in encoding */
    }
    if (is_reencoded()) { /* we needn't read the built-in encoding */
        while (!t1_suffix("def")) {
            t1_putline();
            T1_CHECK_EOF();
            t1_getline();
        }
        t1_putline();
    }
    else if (encoding == ENC_BUITIN) { /* we must read the built-in encoding */
        for (i = 0; i < MAX_CHAR_NUM; i++)
            builtin_glyph_names[i] = notdef;
        t1_builtin_enc();
    }
    else
        t1_putline(); /* write the predefined encoding */
    do {
        T1_CHECK_EOF();
        t1_getline();
        t1_scan_param();
        t1_putline();
    } while (!T1_PREFIX("currentfile eexec"));
    t1_length1 = pdfoffset() - pdfsaveoffset;
    pdfsaveoffset = pdfoffset();
    t1_in_eexec = true;
    for (t1_line_ptr = t1_line, i = 0; i < 4; i++) {
        edecrypt(t1_getbyte());
        T1_CHECK_EOF();
        *t1_line_ptr++ = 0;
    }
    t1_putline(); /* to put the first four bytes */
    T1_CHECK_EOF();
    t1_getline();
    while (!(T1_CHARSTRINGS() || T1_PREFIX("/Subrs"))) {
        t1_scan_param();
        t1_putline();
        T1_CHECK_EOF();
        t1_getline();
    }
    t1_charstring = true;
    if (T1_PREFIX("/Subrs")) do {
        t1_putline();
        T1_CHECK_EOF();
        t1_getline();
    } while (!T1_CHARSTRINGS());
    size_pos = strstr(t1_line, "/CharStrings") +
        strlen("/CharStrings") + 1 - t1_line;
    cs_dict_size = atoi(t1_line + size_pos);
    cs_ptr = cs_tab = XTALLOC(cs_dict_size, cs_entry);
    save_line1 = xstrdup(t1_line);
    T1_CHECK_EOF();
    t1_getline();
    while (t1_cslen) {
        cs_store();
        T1_CHECK_EOF();
        t1_getline();
    }
    save_line2 = xstrdup(t1_line);
    if (is_reencoded())
        cur_glyph_names = enc_tab[fm_cur->encoding].glyph_names;
    else if (encoding == ENC_BUITIN)
        cur_glyph_names = builtin_glyph_names;
    else if (encoding == ENC_STANDARD)
        cur_glyph_names = standard_glyph_names;
    for (i = 0; i < MAX_CHAR_NUM; i++)
        if (pdfischarused(tex_font, i)) {
            if (cur_glyph_names[i] == notdef)
                pdftex_warn("character %i is mapped to %s", i, notdef);
            else
                cs_mark(cur_glyph_names[i]);
        }
    cs_mark(notdef);
    for (cs_count = 0, cs = cs_tab; cs < cs_ptr; cs++)
        if (cs->used)
            cs_count++;
    t1_line_ptr = t1_line;
    for (p = save_line1; p - save_line1 < size_pos;)
        *t1_line_ptr++ = *p++;
    for (;isdigit(*p); p++);
    sprintf(t1_line_ptr, "%u", cs_count);
    strcat(t1_line_ptr, p);
    XFREE(save_line1);
    t1_line_ptr = eol(t1_line);
    t1_putline();
    for (cs = cs_tab; cs < cs_ptr; cs++) {
        if (cs->used) {
            sprintf(t1_line, "/%s %u", cs->name, cs->cslen);
            p = strend(t1_line);
            memcpy(p, cs->data, cs->len);
            t1_line_ptr = p + cs->len;
            t1_putline();
        }
        XFREE(cs->data);
    }
    sprintf(t1_line, "%s", save_line2);
    t1_line_ptr = eol(t1_line);
    t1_putline();
    XFREE(save_line2);
    while (!t1_suffix("mark currentfile closefile")) {
        T1_CHECK_EOF();
        t1_getline();
        t1_putline();
    }
    t1_length2 = pdfoffset() - pdfsaveoffset;
    pdfsaveoffset = pdfoffset();
    t1_in_eexec = false;
    t1_charstring = false;
    while (!T1_PREFIX("cleartomark")) {
        T1_CHECK_EOF();
        t1_getline();
        t1_putline();
    }
    t1_length3 = pdfoffset() - pdfsaveoffset;
    tex_printf(">");
    XFREE(cs_tab);
    T1_CLOSE();
}
