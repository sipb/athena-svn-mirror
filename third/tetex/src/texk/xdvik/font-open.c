/* 
   font_open.c: find font filenames.  This bears no relation (but the
   interface) to the original font_open.c.

Copyright (c) 1999  The texk project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL PAUL VOJTA OR ANY OTHER AUTHOR OF THIS SOFTWARE BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   

 */

#include "xdvi-config.h"

#include "kpathsea/c-fopen.h"
#include "kpathsea/tex-glyph.h"


/* We try for a VF first because that's what dvips does.  Also, it's
   easier to avoid running mktexpk if we have a VF this way.  */

FILE *
font_open(char *font, char **font_ret, double dpi, int *dpi_ret,

	  int dummy, char **filename_ret
#ifdef T1LIB
	  , int *t1id
#endif
	  )
{
    char *name;
    kpse_glyph_file_type file_ret;
    UNUSED(dummy);

    /* defaults in case of success; filename_ret will be
       non-NULL iff the fallback font is used.
    */
    *font_ret = NULL;
    /* filename_ret is NULL iff a T1 version of a font has been used */
    *filename_ret = NULL;
    *dpi_ret = dpi;
    
#ifdef Omega
    name = kpse_find_ovf(font);
    if (!name)
#else
    name = kpse_find_vf(font);
#endif

#ifdef T1LIB
    if (resource.t1lib) {
	*t1id = -1;
    }
#endif /* T1LIB */

    if (name) { /* found a vf font */
	/* pretend it has the expected dpi value, else caller will complain */
	*dpi_ret = dpi;
	*filename_ret = name;
	return xfopen_local(name, FOPEN_R_MODE);
    }

#ifdef T1LIB
    if (resource.t1lib) {
	/* First try: T1 font of correct size */
	*t1id = find_T1_font(font);
	if (*t1id >= 0) {
	    TRACE_T1((stderr, "found T1 font %s", font));
	    return NULL;
	}
	TRACE_T1((stderr,
		  "T1 version of font %s not found, trying pixel version next, then fallback",
		  font));
    }
#endif /* T1LIB */
    /* Second try: PK/GF/... font within allowable size range */
    name = kpse_find_glyph(font, (unsigned)(dpi + .5),
			   kpse_any_glyph_format, &file_ret);

    if (name) { /* success */
	*dpi_ret = file_ret.dpi;
	*filename_ret = name;
	return xfopen_local(name, FOPEN_R_MODE);
    }
    else if (alt_font != NULL) {
	/* The strange thing about kpse_find_glyph() is that it
	   won't create a PK version of alt_font if it doesn't
	   already exist. So we invoke it explicitly a second time
	   for that one.
	*/
#ifdef T1LIB
	if (resource.t1lib) {
	    /* Third try: T1 version of fallback font */
	    *t1id = find_T1_font(alt_font);
	    if (*t1id >= 0) {
		TRACE_T1((stderr, "found fallback font %s\n", font));
		*font_ret = xstrdup(alt_font);
		return NULL;
	    }
	    TRACE_T1((stderr,
		      "Type1 version of fallback font %s not found, trying pixel version",
		      alt_font));
	}
#endif /* T1LIB */
	/* Forth try: PK version of fallback font */
	name = kpse_find_glyph(alt_font, (unsigned)(dpi + .5),
			       kpse_any_glyph_format, &file_ret);
	if (name) { /* success */
	    *dpi_ret = file_ret.dpi;
	    *filename_ret = name;
	    *font_ret = xstrdup(alt_font);
	    return xfopen_local(name, FOPEN_R_MODE);
	}
    }
    /* all other cases are failure */
    return NULL;
}
