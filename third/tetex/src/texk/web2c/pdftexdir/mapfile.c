#include "libpdftex.h"

#define FM_BUF_SIZE     1024

static FILE *fm_file;

#define FM_OPEN()       texpsheaderbopenin(fm_file)
#define FM_CLOSE()      xfclose(fm_file, filename)
#define FM_GETCHAR()    xgetc(fm_file)
#define FM_EOF()        feof(fm_file)

fm_entry *fm_cur, *fm_ptr, *fm_tab = 0;
static int fm_max;
internalfontnumber tex_font;
boolean font_file_not_found;
char *builtin_glyph_names[MAX_CHAR_NUM];
char **cur_glyph_names;

static char *basefont_names[14] = {
    "Courier",
    "Courier-Bold",
    "Courier-Oblique",
    "Courier-BoldOblique",
    "Helvetica",
    "Helvetica-Bold",
    "Helvetica-Oblique",
    "Helvetica-BoldOblique",
    "Symbol",
    "Times-Roman",
    "Times-Bold",
    "Times-Italic",
    "Times-BoldItalic",
    "ZapfDingbats"
};

#define SET_FIELD(F) do {                                  \
    if (q > buf)                                           \
        fm_ptr->F = xstrdup(buf);                          \
    if (*r == 10)                                          \
        goto done;                                         \
} while (0)

void fm_read_info()
{
    double d;
    int i, a, b, c, tex_font_num;
    char fm_line[FM_BUF_SIZE], buf[FM_BUF_SIZE];
    fm_entry *e;
    char *p, *q, *r, *s, *n = mapfiles;
    for (;;) {
        if (fm_file == 0) {
            if (*n == 0) {
                XFREE(mapfiles);
                filename = 0;
                return;
            }
            s = strchr(n, '\n');
            *s = 0;
            filename = n;
            n = s + 1;
            packfilename(maketexstring(filename), getnullstr(), getnullstr());
            if (!FM_OPEN()) {
                pdftex_warn("cannot open font map file");
                continue;
            }
            tex_printf("[%s", (nameoffile+1));
        }
        if (FM_EOF()) {
            FM_CLOSE();
            tex_printf("]");
            fm_file = 0;
            continue;
        }
        ENTRY_ROOM(fm, 256);
        fm_ptr->tex_name = 0;
        fm_ptr->base_name = 0;
        fm_ptr->flags = 0;
        fm_ptr->ff_name = 0;
        fm_ptr->mm_name = 0;
        fm_ptr->prefix = 0;
        fm_ptr->encoding = -1;
        fm_ptr->font_type = 0;
        fm_ptr->slant = 0;
        fm_ptr->extend = 0;
        p = fm_line;
        do {
            c = FM_GETCHAR();
            APPEND_CHAR_TO_BUF(c, p, fm_line, FM_BUF_SIZE);
        } while (c != 10);
        APPEND_EOL(p, fm_line, FM_BUF_SIZE);
        c = *fm_line;
        if (p - fm_line == 1 || c == '*' || c == '#' || c == ';' || c == '%')
            continue;
        r = fm_line;
        READ_FIELD(r, q, buf);
        for (e = fm_tab; e < fm_ptr; e++)
            if (!strcmp(e->tex_name, buf)) {
                pdftex_warn("entry for font `%s' already exists", buf);
                goto bad_line;
            }
        SET_FIELD(tex_name);
        p = r;
        READ_FIELD(r, q, buf);
        if (*buf != '<' && *buf != '"')
            SET_FIELD(base_name);
        else
            r = p; /* unget the field */
        if (isdigit(*r)) { /* font flags given */
            fm_ptr->flags = atoi(r);
            while (isdigit(*r))
                r++;
        }
        else
            fm_ptr->flags = 4;
reswitch:
        if (*r == ' ')
            r++;
        a = b = 0;
        if (*r == '!')
            a = *r++;
        else {
            if (*r == '<')
                a = *r++;
            if (*r == '<' || *r == '[')
                b = *r++;
        }
        switch (*r) {
        case 10:
            goto done;
        case '"':
            r++;
parse_next:
            if (*r == ' ')
                r++;
            d = strtod(r, &s);
            if (s > r) {
                if (*s == ' ')
                    s++;
                if (strncmp(s, "SlantFont", strlen("SlantFont")) == 0) {
                    fm_ptr->slant = (integer)((d+0.0005)*1000);
                    r = s + strlen("SlantFont");
                }
                else if (strncmp(s, "ExtendFont", strlen("ExtendFont")) == 0) {
                    fm_ptr->extend = (integer)((d+0.0005)*1000);
                    r = s + strlen("ExtendFont");
                }
                else {
                    pdftex_warn("unknown name `%s'", s);
                    for (r = s; *r != ' ' && *r != '"'; r++);
                }
            }
            else
                for (; *r != ' ' && *r != '"' && *r != 10; r++);
            if (*r == '"') {
                r++;
                goto reswitch;
            }
            else if (*r == ' ')
                goto parse_next;
            else
                goto warn_bad_line;
        default:
            READ_FIELD(r, q, buf);
            if ((a == '<' && b == '[') ||
                (a != '!' && b == 0 && (strlen(buf) > 4) &&
                 !strcasecmp(strend(buf) - 4, ".enc"))) {
                fm_ptr->encoding = add_enc(buf);
                goto reswitch;
            }
            if (a == '<') {
                fm_ptr->font_type |= F_INCLUDED;
                if (b == 0)
                    fm_ptr->font_type |= F_SUBSETTED;
            }
            else if (a == '!')
                fm_ptr->font_type |= F_NOPARSING;
            SET_FIELD(ff_name);
            goto reswitch;
        }
done:
        if (fm_ptr->base_name != 0) {
            for (i = 0; i < 14; i++)
                if (!strcmp(basefont_names[i], fm_ptr->base_name))
                    break;
            if (i < 14)
                fm_ptr->font_type |= F_BASEFONT;
            else if (fm_ptr->ff_name == 0)
                goto warn_bad_line;
        }
        if (fm_ptr->ff_name != 0 &&
            !strcasecmp(strend(fm_ptr->ff_name) - 4, ".ttf"))
            fm_ptr->font_type |= F_TRUETYPE;
        fm_ptr++;
        continue;
warn_bad_line:
        pdftex_warn("invalid line in map file: `%s'", fm_line);
bad_line:
        XFREE(fm_ptr->tex_name);
        XFREE(fm_ptr->base_name);
        XFREE(fm_ptr->ff_name);
    }
}

fm_entry *fm_ext_entry(internalfontnumber f)
{
    char *p, *q, *r, buf[1024];
    ENTRY_ROOM(fm, 256);
    fm_ptr->tex_name = 0;
    fm_ptr->base_name = 0;
    if (fm_cur->prefix != 0)
        fm_ptr->prefix = xstrdup(fm_cur->prefix);
    else
        fm_ptr->prefix = 0;
    fm_ptr->flags = fm_cur->flags;
    fm_ptr->encoding = fm_cur->encoding;
    fm_ptr->font_type = fm_cur->font_type;
    fm_ptr->slant = fm_cur->slant;
    fm_ptr->extend = fm_cur->extend;
    fm_ptr->ff_name = xstrdup(fm_cur->ff_name);
    p = fm_cur->ff_name;
    if ((r = strrchr(p, '.')) == 0)
        r = strend(p);
    strncpy(buf, p, r - p);
    sprintf(buf + (r - p), "%+i", (int)pdfexpandfont[f]);
    for (q = strend(buf); *r != 0; *q++ = *r++);
    *q = 0;
    fm_ptr->mm_name = xstrdup(buf);
    return fm_ptr++;
}

void fm_free()
{
    fm_entry *fm;
    for (fm = fm_tab; fm < fm_ptr; fm++) {
        XFREE(fm->tex_name);
        XFREE(fm->base_name);
        XFREE(fm->ff_name);
        XFREE(fm->mm_name);
        XFREE(fm->prefix);
    }
    XFREE(fm_tab);
}

integer fmlookup(integer s)
{
    fm_entry *p;
    if (fm_tab == 0)
        fm_read_info();
    for (p = fm_tab; p < fm_ptr; p++)
        if (str_eq_cstr(s, p->tex_name)) {
            return p - fm_tab;
        }
    return -2;
}

/*
boolean sharedsrc(int f, int k)
{
    fm_entry *f_ptr = fm_tab + pdffontmap[f],
        *k_ptr = fm_tab + pdffontmap[k];
    if ((f_ptr->ff_name == 0 && k_ptr->ff_name == 0 &&
         f_ptr->base_name == 0 && k_ptr->base_name == 0) ||
        (f_ptr->ff_name != 0 && k_ptr->ff_name != 0 &&
         strcmp(f_ptr->ff_name, k_ptr->ff_name) == 0 &&
         (f_ptr->extend == k_ptr->extend) &&
         (f_ptr->slant == k_ptr->slant) &&
         ((f_ptr->encoding == -1 && k_ptr->encoding == -1) ||
          (f_ptr->encoding >= 0 && k_ptr->encoding >= 0 &&
           strcmp(enc_tab[f_ptr->encoding].name, 
                  enc_tab[k_ptr->encoding].name) == 0))))
        return true;
    return false;
}
*/
