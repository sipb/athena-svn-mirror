/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXFont.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXFont.c,v 1.1 1993-07-01 23:57:28 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include "hash.h"

struct hash *fonts = NULL;

static XFontStruct *XjLoadQueryFontCache(display, name)
     Display *display;
     char *name;
{
  XrmQuark qname;
  XFontStruct *fs;

  if (fonts == NULL)
    fonts = create_hash(13);

  qname = XrmStringToQuark(name);
  fs = (XFontStruct *)hash_lookup(fonts, qname);

  if (fs == NULL)
    fs = XLoadQueryFont(display, name);

  if (fs != NULL)
    (void)hash_store(fonts, qname, (caddr_t) fs);

  return fs;
}

/*
 * string to XFontStruct conversion
 */
int StrToXFontStruct(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  char errtext[100];
  XFontStruct *fontstr;

  char *sub1 = "-*-*-medium-r-*-*-*-120-*-*-*-*-iso8859-1";
  char *trying = "trying to substitute `%s'.";

  if (strcmp(XjDefaultFont, address) != 0)
    {
      fontstr = XjLoadQueryFontCache(display, address);
      if (fontstr != NULL)
	{
	  *(XFontStruct **)
	    ((char *)where + resource->resource_offset) =
	      fontstr;
	  return 0;
	}
      else
	{
	  sprintf(errtext, "Unknown font: \"%s\"", address);
	  XjWarning(errtext);
	}
    }

  sprintf(errtext, trying, sub1);
  XjWarning(errtext);
  fontstr = XjLoadQueryFontCache(display, sub1);
	      
  if (fontstr == NULL)
    {
      char *sub2 = "fixed";

      sprintf(errtext, trying, sub2);
      XjWarning(errtext);
      fontstr = XjLoadQueryFontCache(display, sub2);
    }

  if (fontstr == NULL)
    {
      XrmQuark key;
      fontstr = (XFontStruct *) hash_give_any_value(fonts, &key);
      if (fontstr != NULL)
	{
	  sprintf(errtext, "substituting font: `%s'",
		  XrmQuarkToString(key));
	  XjWarning(errtext);
	}
    }

  if (fontstr == NULL)
    XjFatalError("couldn't get a font");
	      
  *(XFontStruct **)((char *)where + resource->resource_offset) =
    fontstr;
  return 0;
}
