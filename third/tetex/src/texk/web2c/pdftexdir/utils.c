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

$Id: utils.c,v 1.1.1.2 2003-02-25 22:16:39 amb Exp $
*/

#include "ptexlib.h"
#include "zlib.h"
#include <kpathsea/c-vararg.h>
#include <kpathsea/c-proto.h>
#include "pdftexextra.h" /* define BANNER */

char *cur_file_name = 0;
strnumber last_tex_string;
static char print_buf[PRINTF_BUF_SIZE];
static char *jobname_cstr = 0;
char *job_id_string = 0;
long int last_tab_index; /* for use with entry_room */

typedef char    ff_buf_entry;
ff_buf_entry    *ff_buf_ptr, *ff_buf_tab = 0;
integer         ff_buf_max;

typedef char    fnstr_entry;
fnstr_entry     *fnstr_ptr, *fnstr_tab;
integer         fnstr_max;

integer ff_offset(void)
{
    return ff_buf_ptr - ff_buf_tab;
}

void ff_seek(integer offset)
{
     ff_buf_ptr = ff_buf_tab + offset;
}

void ff_putchar(eightbits b)
{
    entry_room(ff_buf, 1, FF_BUF_SIZE);
    *ff_buf_ptr++ = b;
}

void ff_flush(void)
{
    ff_buf_entry *p;
    integer n;
    for (p = ff_buf_tab; p < ff_buf_ptr;) {
        n = pdfbufsize - pdfptr;
        if (ff_buf_ptr - p < n)
            n = ff_buf_ptr - p;
        memcpy(pdfbuf + pdfptr, p, (unsigned)n);
        pdfptr += n;
        if (pdfptr == pdfbufsize)
            pdfflush();
        p += n;
    }
    ff_buf_ptr = ff_buf_tab;
}

static void fnstr_append(char *s)
{
    int l = strlen(s) + 1;
    entry_room(fnstr, l, l);
    strcat(fnstr_ptr, s);
    fnstr_ptr = strend(fnstr_ptr);
}

void make_subset_tag(fm_entry *fm_cur, integer fn_offset)
{
    char tag[7];
    unsigned long crc;
    eightbits *fontname_ptr = ff_buf_tab + fn_offset;
    int i, l = strlen(job_id_string);
    fnstr_tab = 0;
    entry_room(fnstr, 1, l + 1);
    strcpy(fnstr_tab, job_id_string);
    fnstr_ptr = strend(fnstr_tab);
    if (fm_cur->tfm_name != 0) {
        fnstr_append(" TFM name: ");
        fnstr_append(fm_cur->tfm_name);
    }
    fnstr_append(" PS name: ");
    if (font_keys[FONTNAME_CODE].valid)
        fnstr_append(font_keys[FONTNAME_CODE].value.string);
    else if (fm_cur->ps_name != 0)
        fnstr_append(fm_cur->ps_name);
    fnstr_append(" Encoding: ");
    if (fm_cur->encoding >= 0 && enc_tab[fm_cur->encoding].name != 0)
        fnstr_append(enc_tab[fm_cur->encoding].name);
    else
        fnstr_append("built-in");
    fnstr_append(" CharSet: ");
    for (i = 0; i <= MAX_CHAR_CODE; i++)
        if (pdfcharmarked(tex_font, i) && t1_glyph_names[i] != notdef) {
            fnstr_append(" /");
            fnstr_append(t1_glyph_names[i]);
        }
    if (fm_cur->charset != 0) {
        fnstr_append(" Extra CharSet: ");
        fnstr_append(fm_cur->charset);
    }
    crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, fnstr_tab, strlen(fnstr_tab));
    xfree(fnstr_tab);
    /* we need to fit a 32-bit number into a string of 6 uppercase chars long;
     * there are 26 uppercase chars ==> each char represents a number in range
     * 0..25. The maximal number that can be represented by the tag is
     * 26^6 - 1, which is a number between 2^28 and 2^29. Thus the bits 29..31
     * of the CRC must be dropped out.
     */
    for (i = 0; i < 6; i++) {
        fontname_ptr[i] = tag[i] = 'A' + crc % 26;
        crc /= 26;
    }
    tag[6] = 0;
    fm_cur->subset_tag = xstrdup(tag);
}

void pdf_puts(char *s)
{
    pdfroom(strlen(s) + 1);
    while (*s)
        pdfbuf[pdfptr++] = *s++;
}

void pdf_printf(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(print_buf, fmt, args);
    pdf_puts(print_buf);                                    
    va_end(args);
}

strnumber maketexstring(const char *s)
{
    int l;
    if (s == 0 || *s == 0)
        return getnullstr();
    l = strlen(s);
    check_buf(poolptr + l, poolsize);
    while (l-- > 0)
        strpool[poolptr++] = *s++;
    last_tex_string = makestring();
    return last_tex_string;
}

void tex_printf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(print_buf, fmt, args);
    print(maketexstring(print_buf));
    flushstr(last_tex_string);
    xfflush(stdout);
    va_end(args);
}

/* Helper for pdftex_fail. */
static void safe_print(char *str)
{
    char *c;
    for (c = str; *c; ++c)
        print(*c);
}

/* pdftex_fail may be called when a buffer overflow has happened/is
   happening, therefore may not call mktexstring.  However, with the
   current implementation it appears that error messages are misleading,
   possibly because pool overflows are detected too late. */
void pdftex_fail(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    println();
    safe_print("Error: ");
    safe_print(program_invocation_name);
    if (cur_file_name) {
        safe_print(" (file ");
        safe_print(cur_file_name);
        safe_print(")");
    }
    safe_print(": ");
    vsprintf(print_buf, fmt, args);
    safe_print(print_buf);
    va_end(args);
    println();
    safe_print(" ==> Fatal error occurred, the output PDF file is not finished!");
    println();
    exit(-1);
}

void pdftex_warn(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    println();
    tex_printf("Warning: %s", program_invocation_name);
    if (cur_file_name)
        tex_printf(" (file %s)", cur_file_name);
    tex_printf(": ");
    vsprintf(print_buf, fmt, args);
    print(maketexstring(print_buf));
    flushstr(last_tex_string);
    va_end(args);
    println();
}

char *makecstring(integer s)
{
    static char cstrbuf[MAX_CSTRING_LEN];
    char *p = cstrbuf;
    int i, l = strstart[s + 1] - strstart[s];
    check_buf(l, MAX_CSTRING_LEN);
    for (i = 0; i < l; i++)
        *p++ = strpool[i + strstart[s]];
    *p = 0;
    return cstrbuf;
}

boolean str_eq_cstr(strnumber n, char *s)
{
    int l;
    if (s == 0 || n == 0)
        return false;
    l = strstart[n];
    while (*s && l < strstart[n + 1] && *s == strpool[l])
        l++, s++;
    return !*s && l == strstart[n + 1];
}

void setjobid(int year, int month, int day, int time, int pdftexversion, int pdftexrevision)
{
    extern string versionstring; /* from web2c/lib/version.c */         
    extern KPSEDLL string kpathsea_version_string; /* from kpathsea/version.c */
    char *name_string = xstrdup(makecstring(jobname)),
         *format_string = xstrdup(makecstring(formatident)),
         *s = xtalloc(SMALL_BUF_SIZE + 
                      strlen(name_string) + 
                      strlen(format_string) + 
                      strlen(BANNER) + 
                      strlen(versionstring) + 
                      strlen(kpathsea_version_string), char);
    sprintf(s, "%.4d/%.2d/%.2d %.2d:%.2d %s %s %s %s %s",
            year, month, day, time/60, time%60, 
            name_string, format_string, BANNER, 
            versionstring, kpathsea_version_string);
    job_id_string = xstrdup(s);

    pdftexbanner = maketexstring(BANNER);
    
    xfree(name_string);
    xfree(format_string);
    xfree(s);
}

strnumber getresnameprefix(void)
{
    static char name_str[] =
"!\"$&'*+,-.0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\\\
^_`abcdefghijklmnopqrstuvwxyz|~";
    char prefix[6]; /* make a tag of 6 chars long */
    char *p = prefix; 
    unsigned long crc;
    int i, k, base = strlen(name_str);
    crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, job_id_string, strlen(job_id_string));
    for (i = 0; i < 6; i++) {
        prefix[i] = name_str[crc % base];
        crc /= base;
    }
    prefix[5] = 0;
    return maketexstring(prefix);
}

size_t xfwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (fwrite(ptr, size, nmemb, stream) != nmemb)
        pdftex_fail("fwrite() failed");
    return nmemb;
}

int xfflush(FILE *stream)
{
    if (fflush(stream) != 0)
        pdftex_fail("fflush() failed");
    return 0;
}

int xgetc(FILE *stream)
{
    int c = getc(stream);
    if (c < 0 && c != EOF)
        pdftex_fail("getc() failed");
    return c;
}

int xputc(int c, FILE *stream)
{
    int i = putc(c, stream);
    if (i < 0)
        pdftex_fail("putc() failed");
    return i;
}

void writestreamlength(integer length, integer offset)
{
    integer save_offset;
    if (jobname_cstr == 0)
        jobname_cstr = xstrdup(makecstring(jobname));
    save_offset = xftell(pdffile, jobname_cstr);
    xfseek(pdffile, offset, SEEK_SET, jobname_cstr);
    fprintf(pdffile, "%li", (long int)length);
    xfseek(pdffile, pdfoffset(), SEEK_SET, jobname_cstr);
}

scaled extxnoverd(scaled x, scaled n, scaled d)
{
    double r = (((double)x)*((double)n))/((double)d);
    if (r > 0)
        r += 0.5;
    else
        r -= 0.5;
    if (r >= (double)maxinteger || r <= -(double)maxinteger)
        pdftex_warn("arithmetic: number too big");
    return r;
}

void libpdffinish()
{
    xfree(ff_buf_tab);
    xfree(job_id_string);
    fm_free();
    enc_free();
    img_free();
    vf_free();
    epdf_free();
}

/* Converts any string given in in in an allowed PDF string which can be
 * handled by printf et.al.: \ is escaped to \\, paranthesis are escaped and
 * control characters are hexadecimal encoded.
 */
void convertStringToPDFString (char *in, char *out)
{
    int lin = strlen (in);
    int i, j;
    char buf[4];
    j = 0;
    for (i = 0; i < lin; i++) {
        if ((unsigned char)in[i] < ' ') {
            /* convert control characters into hex */
            sprintf (buf, "#%02x", (unsigned int)in[i]);
            out[j++] = buf[0];
            out[j++] = buf[1];
            out[j++] = buf[2];
            }
        else if ((in[i] == '(') || (in[i] == ')')) {
            /* escape paranthesis */
            out[j++] = '\\';
            out[j++] = in[i];
            }
        else if (in[i] == '\\') {
            /* escape backslash */
            out[j++] = '\\';
            out[j++] = '\\';
            }
        else {
            /* copy char :-) */
            out[j++] = in[i];
            }
        }
    out[j] = '\0';
}
