#include "libpdftex.h"
#include <kpathsea/c-vararg.h>
#include <kpathsea/c-proto.h>

typedef char *res_name_entry;

extern void epdf_check_mem();

char *extra_fonts = 0;
char *extra_xobjects = 0;
char *other_resources = 0;
res_name_entry *res_name_ptr, *res_name_tab = 0;
int res_name_max;

void writeEPDF(char *fmt,...) {
    va_list args;

    va_start(args, fmt);
    vsprintf(print_buf, fmt, args);
    flush_print_buf();                                    
    va_end(args);
}

void add_resources_name(char *s)
{
    res_name_entry *e;
    if (res_name_tab != 0) {
        for (e = res_name_tab; e < res_name_ptr; e++)
            if (strcmp(*e, s) == 0)
                pdftex_warn("duplicate of resource name `%s'", s);
    }
    ENTRY_ROOM(res_name, 256);
    *res_name_ptr++ = xstrdup(s);
}

void appendresourcesname(integer prefix, integer i)
{
    static char buf[1024];
    sprintf(buf, "%s%i",  makecstring(prefix), (int)i);
    add_resources_name(buf);
}

void deleteresourcesnames()
{
    res_name_entry *p;
    if (res_name_tab == 0)
        return;
    for (p = res_name_tab; p < res_name_ptr; p++)
        XFREE(*p);
    XFREE(res_name_tab);
    res_name_tab = 0;
}

void add_extra_fonts()
{
    int l = strlen(print_buf) + 1;
    if (extra_fonts == 0) {
        extra_fonts = XTALLOC(l, char);
        *extra_fonts = 0;
    }
    else
        extra_fonts = 
            XRETALLOC(extra_fonts, strlen(extra_fonts) + l, char);
    strcat(extra_fonts, print_buf);
}

void printextrafonts()
{
    pdf_printf(extra_fonts);
    XFREE(extra_fonts);
    extra_fonts = 0;
}

void add_extra_xobjects()
{
    int l = strlen(print_buf) + 1;
    if (extra_xobjects == 0) {
        extra_xobjects = XTALLOC(l, char);
        *extra_xobjects = 0;
    }
    else
        extra_xobjects = 
            XRETALLOC(extra_xobjects, strlen(extra_xobjects) + l, char);
    strcat(extra_xobjects, print_buf);
}

void printextraxobjects()
{
    if (extra_xobjects != 0) {
        pdf_printf(extra_xobjects);
        XFREE(extra_xobjects);
    }
    extra_xobjects = 0;
}

void add_other_resources()
{
    int l = strlen(print_buf) + 1;
    if (other_resources == 0) {
        other_resources = XTALLOC(l, char);
        *other_resources = 0;
    }
    else
        other_resources = 
            XRETALLOC(other_resources, strlen(other_resources) + l, char);
    strcat(other_resources, print_buf);
}

void printotherresources()
{
    if (other_resources != 0) {
        pdf_printf(other_resources);
        XFREE(other_resources);
    }
    other_resources = 0;
}

void epdf_free()
{
    epdf_check_mem();
    XFREE(extra_fonts);
    XFREE(extra_xobjects);
    XFREE(other_resources);
    deleteresourcesnames();
}
