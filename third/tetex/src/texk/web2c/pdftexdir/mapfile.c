/*
Copyright (c) 1996-2002 Han The Thanh, <thanh@pdftex.org>

This file is part of pdfTeX.

pdfTeX is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

pdfTeX is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with pdfTeX; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

$Id: mapfile.c,v 1.1.1.2 2003-02-25 22:09:33 amb Exp $
*/

#include "ptexlib.h"
#include <kpathsea/c-auto.h>
#include <kpathsea/c-memstr.h>

#define FM_BUF_SIZE     1024

static FILE *fm_file;

#define fm_open()       \
    open_input (&fm_file, kpse_tex_ps_header_format, FOPEN_RBIN_MODE)
#define fm_close()      xfclose(fm_file, cur_file_name)
#define fm_getchar()    xgetc(fm_file)
#define fm_eof()        feof(fm_file)

fm_entry *fm_cur, *fm_ptr, *fm_tab = 0;
static int fm_max;
char *mapfiles = 0;
static char nontfm[] = "<nontfm>";

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

#define read_field(r, q, buf) do {                         \
    for (q = buf; *r != ' ' && *r != 10; *q++ = *r++);     \
    *q = 0;                                                \
    skip(r, ' ');                                          \
} while (0)

#define set_field(F) do {                                  \
    if (q > buf)                                           \
        fm_ptr->F = xstrdup(buf);                          \
    if (*r == 10)                                          \
        goto done;                                         \
} while (0)

static void fm_new_entry(void)
{
    entry_room(fm, 1, 25);
    fm_ptr->tfm_name        = 0;
    fm_ptr->ps_name         = 0;
    fm_ptr->flags           = 0;
    fm_ptr->ff_name         = 0;
    fm_ptr->subset_tag      = 0;
    fm_ptr->type            = 0;
    fm_ptr->slant           = 0;
    fm_ptr->extend          = 0;
    fm_ptr->expansion       = 0;
    fm_ptr->ff_objnum       = 0;
    fm_ptr->fn_objnum       = 0;
    fm_ptr->fd_objnum       = 0;
    fm_ptr->charset         = 0;
    fm_ptr->encoding        = -1;
    fm_ptr->found           = false;
    fm_ptr->all_glyphs      = false;
    fm_ptr->tfm_num         = getnullfont();
}

void fm_read_info(void)
{
    float d;
    int i, a, b, c;
    char fm_line[FM_BUF_SIZE], buf[FM_BUF_SIZE];
    fm_entry *e;
    char *p, *q, *r, *s, *n = mapfiles;
    for (;;) {
        if (fm_file == 0) {
            if (*n == 0) {
                xfree(mapfiles);
                cur_file_name = 0;
                if (fm_tab == 0)
                     fm_new_entry();
                return;
            }
            s = strchr(n, '\n');
            *s = 0;
            set_cur_file_name(n);
            n = s + 1;
            if (!fm_open()) {
                pdftex_warn("cannot open font map file");
                continue;
            }
            cur_file_name = nameoffile + 1;
            tex_printf("{%s", cur_file_name);
        }
        if (fm_eof()) {
            fm_close();
            tex_printf("}");
            fm_file = 0;
            continue;
        }
        fm_new_entry();
        p = fm_line;
        do {
            c = fm_getchar();
            append_char_to_buf(c, p, fm_line, FM_BUF_SIZE);
        } while (c != 10);
        append_eol(p, fm_line, FM_BUF_SIZE);
        c = *fm_line;
        if (p - fm_line == 1 || c == '*' || c == '#' || c == ';' || c == '%')
            continue;
        r = fm_line;
        read_field(r, q, buf);
        if (strcmp(buf, nontfm) == 0)
            fm_ptr->tfm_name = nontfm;
        else {
            for (e = fm_tab; e < fm_ptr; e++)
                if (e->tfm_name != nontfm && strcmp(e->tfm_name, buf) == 0) {
                    pdftex_warn("entry for `%s' already exists, duplicates ignored", buf);
                    goto bad_line;
                }
            set_field(tfm_name);
        }
        p = r;
        read_field(r, q, buf);
        if (*buf != '<' && *buf != '"')
            set_field(ps_name);
        else
            r = p; /* unget the field */
        if (isdigit(*r)) { /* font flags given */
            fm_ptr->flags = atoi(r);
            while (isdigit(*r))
                r++;
        }
        else
            fm_ptr->flags = 4; /* treat as Symbol font */
reswitch:
        skip(r, ' ');
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
            skip(r, ' ');
            if (sscanf(r, "%f", &d) > 0) {
                for (s = r; *s != ' ' && *s != '"' && *s != 10; s++);
                skip(s, ' ');
                if (strncmp(s, "SlantFont", strlen("SlantFont")) == 0) {
                    fm_ptr->slant = (integer)((d+0.0005)*1000);
                    r = s + strlen("SlantFont");
                }
                else if (strncmp(s, "ExtendFont", strlen("ExtendFont")) == 0) {
                    fm_ptr->extend = (integer)((d+0.0005)*1000);
                    r = s + strlen("ExtendFont");
                }
                else {
                    pdftex_warn("invalid entry for `%s': unknown name `%s' ignored", fm_ptr->tfm_name, s);
                    for (r = s; *r != ' ' && *r != '"' && *r != 10; r++);
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
            else {
                pdftex_warn("invalid entry for `%s': unknown line format", fm_ptr->tfm_name);
                goto bad_line;
            }
        default:
            read_field(r, q, buf);
            if ((a == '<' && b == '[') ||
                (a != '!' && b == 0 && (strlen(buf) > 4) &&
                 !strcasecmp(strend(buf) - 4, ".enc"))) {
                fm_ptr->encoding = add_enc(buf);
                goto reswitch;
            }
            if (a == '<') {
                set_included(fm_ptr);
                if (b == 0)
                    set_subsetted(fm_ptr);
            }
            else if (a == '!')
                set_noparsing(fm_ptr);
            set_field(ff_name);
            goto reswitch;
        }
done:
        if (fm_ptr->ps_name != 0) {
            for (i = 0; i < 14; i++)
                if (!strcmp(basefont_names[i], fm_ptr->ps_name))
                    break;
            if (i < 14) {
                set_basefont(fm_ptr);
                unset_included(fm_ptr);
                unset_subsetted(fm_ptr);
                unset_truetype(fm_ptr);
                unset_fontfile(fm_ptr);
            }
            else if (fm_ptr->ff_name == 0) {
                pdftex_warn("invalid entry for `%s': font file missing", fm_ptr->tfm_name);
                goto bad_line;
            }
        }
        if (fm_fontfile(fm_ptr) != 0 && 
            strcasecmp(strend(fm_fontfile(fm_ptr)) - 4, ".ttf") == 0)
            set_truetype(fm_ptr);
        if ((fm_ptr->slant != 0 || fm_ptr->extend != 0) &&
            (!is_included(fm_ptr) || is_truetype(fm_ptr))) {
            pdftex_warn("invalid entry for `%s': SlantFont/ExtendFont can be used only with embedded T1 fonts", fm_ptr->tfm_name);
            goto bad_line;
        }
        if (is_truetype(fm_ptr) && (is_reencoded(fm_ptr)) &&
            !is_subsetted(fm_ptr)) {
            pdftex_warn("invalid entry for `%s': only subsetted TrueType font can be reencoded", fm_ptr->tfm_name);
            goto bad_line;
        }
        if (abs(fm_ptr->slant) >= 2000) {
            pdftex_warn("invalid entry for `%s': too big value of SlantFont (%.3g)",
                        fm_ptr->tfm_name, fm_ptr->slant/1000.0);
            goto bad_line;
        }
        if (abs(fm_ptr->extend) >= 2000) {
            pdftex_warn("invalid entry for `%s': too big value of ExtendFont (%.3g)",
                        fm_ptr->tfm_name, fm_ptr->extend/1000.0);
            goto bad_line;
        }
        fm_ptr++;
        continue;
bad_line:
        if (fm_ptr->tfm_name != nontfm)
            xfree(fm_ptr->tfm_name);
        xfree(fm_ptr->ps_name);
        xfree(fm_ptr->ff_name);
    }
}

char *mk_basename(char *exname)
{
    char buf[SMALL_BUF_SIZE], *p = exname, *q, *r;
    if ((r = strrchr(p, '.')) == 0)
        r = strend(p);
    for (q = r - 1; q > p && isdigit(*q); q--);
    if (q <= p || q == r - 1 || (*q != '+' && *q != '-'))
        pdftex_fail("invalid name of expanded font (%s)", p);
    strncpy(buf, p, (unsigned)(q - p));
    buf[q - p] = 0;
    strcat(buf, r);
    return xstrdup(buf);
}

char *mk_exname(char *basename, int e)
{
    char buf[SMALL_BUF_SIZE], *p = basename, *r;
    int i;
    if ((r = strrchr(p, '.')) == 0)
        r = strend(p);
    strncpy(buf, p, (unsigned)(r - p));
    sprintf(buf + (r - p), "%+i", e);
    strcat(buf, r);
    return xstrdup(buf);
}

internalfontnumber tfmoffm(integer i)
{
    return fm_tab[i].tfm_num;
}

void checkextfm(strnumber s, integer e)
{
    char *p;
    if (e == 0)
        return;
    p = mk_exname(makecstring(s), e);
    kpse_find_tfm(p); /* to create MM instance if needed */
    xfree(p);
}

static fm_entry *mk_ex_fm(internalfontnumber f, int fm_index, int ex)
{
    fm_entry *e;
    fm_new_entry();
    e = fm_tab + fm_index;
    fm_ptr->subset_tag = 0;
    fm_ptr->flags = e->flags;
    fm_ptr->encoding = e->encoding;
    fm_ptr->type = e->type;
    fm_ptr->slant = e->slant;
    fm_ptr->extend = e->extend;
    fm_ptr->tfm_name = xstrdup(makecstring(fontname[f]));
    fm_ptr->ff_name = xstrdup(e->ff_name);
    fm_ptr->ff_objnum = pdfnewobjnum();
    fm_ptr->expansion = ex;
    fm_ptr->tfm_num = f;
    return fm_ptr++;
}

int lookup_fontmap(char *bname)
{
    fm_entry *p;
    char *s = bname;
    int i;
    if (fm_tab == 0)
        fm_read_info();
    if (bname == 0)
        return -1;
    if (strlen(s) > 7) { /* check for subsetted name tag */
        for (i = 0; i < 6; i++, s++)
            if (!(*s >= 'A' && *s <= 'Z'))
                break;
        if (i == 6 && *s == '+')
            bname = s + 1;
    }
    for (p = fm_tab; p < fm_ptr; p++)
        if (p->ps_name != 0 && strcmp(p->ps_name, bname) == 0) {
            if (is_basefont(p) || is_noparsing(p) || !is_included(p))
                return -1;
            if (p->tfm_num == getnullfont()) {
                if (p->tfm_name == nontfm)
                    p->tfm_num = newnullfont();
                else
                    p->tfm_num = gettfmnum(maketexstring(p->tfm_name));
            }
            i = p->tfm_num;
            if (pdffontmap[i] == -1)
                pdffontmap[i] = p - fm_tab;
            if (p->ff_objnum == 0 && is_included(p))
                p->ff_objnum = pdfnewobjnum();
            if (!fontused[i])
                pdfinitfont(i);
            return p - fm_tab;
        }
    return -1;
}

static boolean isdigitstr(char *s)
{
    for (;*s != 0; s++)
        if (!isdigit(*s))
            return false;
    return true;
}

static void init_fm(fm_entry *fm, internalfontnumber f)
{
    if (fm->fd_objnum == 0)
        fm->fd_objnum = pdfnewobjnum();
    if (fm->ff_objnum == 0 && is_included(fm))
        fm->ff_objnum = pdfnewobjnum();
    if (fm->tfm_num == getnullfont())
        fm->tfm_num = f;
}

integer fmlookup(internalfontnumber f)
{
    char *tfm, *p;
    fm_entry *fm, *exfm;
    int e, l;
    if (fm_tab == 0)
        fm_read_info();
    tfm = makecstring(fontname[f]);
    /* loop up for tfm_name */
    for (fm = fm_tab; fm < fm_ptr; fm++)
        if (fm->tfm_name != nontfm && strcmp(tfm, fm->tfm_name) == 0) {
            init_fm(fm, f);
            return fm - fm_tab;
        }
    /* expanded fonts; loop up for base tfm_name */
    if (pdffontexpandratio[f] != 0) {
        tfm = mk_basename(tfm);
        if ((e = getexpandfactor(f)) != 0)
            goto ex_font;
        for (fm = fm_tab; fm < fm_ptr; fm++)
            if (fm->tfm_name != nontfm && strcmp(tfm, fm->tfm_name) == 0) {
                init_fm(fm, f);
                return fm - fm_tab;
            }
    }
    return -2;
ex_font:
    l = strlen(tfm);
    /* look up for expanded fonts in reversed direction, as they are
     * appended to the end of fm_tab */
    for (fm = fm_ptr - 1; fm >= fm_tab; fm--)
        if (fm->tfm_name != nontfm && strncmp(tfm, fm->tfm_name, l) == 0) {
            p = fm->tfm_name + l;
            if (fm->expansion == e && (*p == '+' || *p == '-') &&
                isdigitstr(p + 1))
                return fm - fm_tab;
            else if (fm->expansion == 0 && *p == 0) {
                exfm = mk_ex_fm(f, fm - fm_tab, e);
                init_fm(exfm, f);
                return exfm - fm_tab;
            }
        }
    return -2;
}

void fix_ffname(fm_entry *fm, char *name)
{
    xfree(fm->ff_name);
    fm->ff_name = xstrdup(name);
    fm->found   = true;     /* avoid next searching for the same file */
}

void fm_free(void)
{
    fm_entry *fm;
    for (fm = fm_tab; fm < fm_ptr; fm++) {
        if (fm->tfm_name != nontfm)
            xfree(fm->tfm_name);
        xfree(fm->ps_name);
        xfree(fm->ff_name);
        xfree(fm->subset_tag);
        xfree(fm->charset);
    }
    xfree(fm_tab);
}
