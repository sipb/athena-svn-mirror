#include "libpdftex.h"

#define ENC_BUF_SIZE    1024

static FILE *enc_file;

#define ENC_OPEN()      texpsheaderbopenin(enc_file)
#define ENC_GETCHAR()   xgetc(enc_file)
#define ENC_CLOSE()     xfclose(enc_file, filename)
#define ENC_EOF()       feof(enc_file)

enc_entry *enc_ptr, *enc_tab = 0;
int enc_max;
char enc_line[ENC_BUF_SIZE];

integer add_enc(char *s)
{
    enc_entry *e;
    if (enc_tab != 0) {
        for (e = enc_tab; e < enc_ptr; e++)
            if (strcmp(s, e->name) == 0)
                return e - enc_tab;
    }
    ENTRY_ROOM(enc, 256);
    enc_ptr->name = xstrdup(s);
    enc_ptr->obj_num = 0;
    return enc_ptr++ - enc_tab;
}

void enc_getline()
{
    char *p;
    int c;
restart:
    if (ENC_EOF())
        pdftex_fail("unexpected end of file");
    p = enc_line;
    do {
        c = ENC_GETCHAR();
        APPEND_CHAR_TO_BUF(c, p, enc_line, ENC_BUF_SIZE);
    } while (c != 10);
    APPEND_EOL(p, enc_line, ENC_BUF_SIZE);
    if (p - enc_line <= 2 || *enc_line == '%')
        goto restart;
}

void write_enc(enc_entry *e)
{
    char buf[ENC_BUF_SIZE], *q, *r;
    int  i, names_count;
    boolean is_notdef;
    filename = e->name;
    for (i = 0; i < MAX_CHAR_NUM; e->glyph_names[i++] = notdef);
    packfilename(maketexstring(filename), getnullstr(), getnullstr());
    if (!ENC_OPEN()) {
        pdftex_warn("cannot open encoding file for reading");
        return;
    }
    tex_printf("<%s", e->name);
    is_notdef = false;
    enc_getline();
    if (*enc_line != '/' || (r = strchr(enc_line, '[')) == NULL)
        pdftex_fail("invalid encoding vector: name or `[' missing: `%s'", enc_line);
    pdfnewdict(0, 0);
    e->obj_num = objptr;
    names_count = 0;
    pdf_printf("/Type /Encoding\n/Differences [");
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
            if (!strcmp(buf, notdef))  {
                if (!is_notdef) {
                    pdf_printf(" %i/%s", names_count, buf);
                    is_notdef = true;
                }
            }
            else {
                if (is_notdef || (names_count == 0)) {
                    pdf_printf(" %i", names_count);
                    is_notdef = false;
                }
                pdf_printf("/%s", buf);
                e->glyph_names[names_count] = xstrdup(buf);
            }
            if (names_count++ >= MAX_CHAR_NUM)
                pdftex_fail("encoding vector contains more than %i names",
                     (int)MAX_CHAR_NUM);
        }
        if (*r != 10 && *r != '%') {
            if (strncmp(r, "] def", strlen("] def")) == 0) 
                goto done;
            else
                pdftex_fail("invalid encoding vector: a name or `] def' expected: `%s'", enc_line);
        }
        enc_getline();
        r = enc_line;
    }
done:
    pdf_printf("]\n>> endobj\n");
    ENC_CLOSE();
    tex_printf(">");
}

integer enc_objnum(integer i)
{
    if (enc_tab[i].obj_num == 0)
        write_enc(enc_tab + i);
    return enc_tab[i].obj_num;
}

void enc_free()
{
    enc_entry *e;
    int k;
    for (e = enc_tab; e < enc_ptr; e++) {
        XFREE(e->name);
        if (e->obj_num != 0) {
            for (k = 0; k < MAX_CHAR_NUM; k++)
                if (e->glyph_names[k] != notdef)
                    XFREE(e->glyph_names[k]);
        }
    }
    XFREE(enc_tab);
}
