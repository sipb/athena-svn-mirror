/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToPixmap.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToPixmap.c,v 1.1 1993-07-02 00:03:19 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/lib/X11/bitmaps"
#endif
static char *bitmapdir = BITMAPDIR;

/*
 * String to Pixmap conversion
 */
int StrToPixmap(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  XjPixmap *newPixmap = NULL;
  XjPixmap p;
  char errtext[100];
  char *error = "error reading bitmap %s%s";
  char *no_mem = ", no memory";
  char *ptr = (char *) address;
  int status = 1;

  if (ptr == NULL)
    {
      *((XjPixmap **)((char *)where + resource->resource_offset)) = NULL;
      return 0;
    }

  switch(XReadBitmapFile(display, window,
			 ptr, &p.width, &p.height,
			 &p.pixmap, &p.hot_x, &p.hot_y))
    {
    case BitmapOpenFailed:
    case BitmapFileInvalid:
      if (ptr[0] != '/')
	{
	  char *new_ptr;
	  int ret;

	  new_ptr = (char *) XjMalloc(strlen(ptr)
				      + strlen(bitmapdir) + 2);
	  sprintf(new_ptr, "%s/%s", bitmapdir, ptr);
	  ret = StrToPixmap(display, window, where, resource, type, new_ptr);
	  XjFree(new_ptr);
	  if (!ret)
	    return 0;
	}
      sprintf(errtext, error, ptr, "");
      XjWarning(errtext);
      break;

    case BitmapNoMemory:
      sprintf(errtext, error, ptr, no_mem);
      XjWarning(errtext);
      break;

    case BitmapSuccess:
      newPixmap = (XjPixmap *)XjMalloc((unsigned) sizeof(XjPixmap));
      bcopy((char *)&p, (char *)newPixmap, sizeof(XjPixmap));
      status = 0;
      break;
    }

  *((XjPixmap **)((char *)where + resource->resource_offset)) = newPixmap;

  return status;
}
