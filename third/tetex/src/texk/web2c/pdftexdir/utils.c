#include "libpdftex.h"
#include <kpathsea/c-vararg.h>
#include <kpathsea/c-proto.h>

char *filename = 0;
char print_buf[1024];


integer pdfoffset()
{
    return pdfgone + pdfptr;
}

void pdfout(integer c)
{
    pdfbuf[pdfptr++] = c;
    if (pdfptr == pdfbufsize)
        pdfflush();
}

void pdf_printf(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(print_buf, fmt, args);
    flush_print_buf();                                    
    va_end(args);
}

void tex_printf(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(print_buf, fmt, args);
    print(maketexstring(print_buf));
    flushstring();
    xfflush(stdout);
    va_end(args);
}

void pdftex_fail(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    println();
    tex_printf("Error: %s", program_invocation_name);
    if (filename)
        tex_printf(" (file %s)", filename);
    tex_printf(": ");
    vsprintf(print_buf, fmt, args);
    print(maketexstring(print_buf));
    flushstring();
    va_end(args);
    println();
    exit(-1);
}

void pdftex_warn(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    println();
    tex_printf("Warning: %s", program_invocation_name);
    if (filename)
        tex_printf(" (file %s)", filename);
    tex_printf(": ");
    vsprintf(print_buf, fmt, args);
    print(maketexstring(print_buf));
    flushstring();
    va_end(args);
    println();
}

void flush_print_buf()
{
    char *p = print_buf;
    if (sizeof(pdfbuf) - pdfptr <= 1024)
        pdfflush();
    while (*p)
        pdfbuf[pdfptr++] = *p++;
}

strnumber maketexstring(char *s)
{
    int l;
    if (s == 0 || !*s)
        return getnullstr();
    l = strlen(s);
    CHECK_BUF(poolptr + l, poolsize);
    while (l-- > 0)
        strpool[poolptr++] = *s++;
    return makestring();
}

char *makecstring(integer s)
{
    static char cstrbuf[1024];
    char *p = cstrbuf;
    int i, l = strstart[s + 1] - strstart[s];
    CHECK_BUF(l, 1024);
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

void libpdffinish()
{
    fm_free();
    enc_free();
    img_free();
    vf_free();
    epdf_free();
}
