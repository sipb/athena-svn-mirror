extern "C" {

#include <kpathsea/c-auto.h>

/* the following code is extremly ugly but needed for including web2c/config.h */

#include <kpathsea/c-proto.h>     /* define P?H macros */

typedef const char *const_string; /* including kpathsea/types.h */
                                  /* doesn't work on some systems */

#define KPATHSEA_CONFIG_H         /* avoid including other kpathsea header files */
                                  /* from web2c/config.h */

#ifdef CONFIG_H                   /* CONFIG_H has been defined by some xpdf */
#undef CONFIG_H                   /* header file */
#endif

#include <web2c/config.h>         /* define type integer */

extern int epdf_width;
extern int epdf_height;
extern int epdf_orig_x;
extern int epdf_orig_y;
extern void *epdf_doc;
extern void *epdf_xref;
extern integer pdfimageb;
extern integer pdfimagec;
extern integer pdfimagei;
extern integer pdftext;
extern integer pdfincludeformresources;

extern integer read_pdf_info(char*);
extern void write_epdf(integer, integer);
extern void epdf_delete();
extern void epdf_free();
extern void epdf_check_mem();
extern void writeEPDF(char *fmt,...);
extern void pdfout(int);
extern integer zpdfcreateobj(integer, integer);
extern integer zpdfnewobj(integer, integer);
extern integer zpdfnewdict(integer, integer);
extern integer zpdfbeginobj(integer);
extern integer zpdfbegindict(integer);
extern void pdfprintimageattr();
extern void add_extra_xobjects();
extern void add_other_resources();
extern void add_extra_fonts();
extern void add_resources_name(char *);
extern void pdfbeginstream();
extern void pdfendstream();
extern integer objptr;
extern integer pdfstreamlength;
}
