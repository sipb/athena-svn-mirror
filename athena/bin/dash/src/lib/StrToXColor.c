/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXColor.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXColor.c,v 1.1 1993-07-02 00:00:58 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include <ctype.h>

#define MONO 0
#define GRAY 1
#define COLOR 2


/*
 * string to Pixel conversion
 */
int StrToXPixel(display, string, pixel)
     Display *display;
     char *string;
     unsigned long *pixel;
{
  char errtext[100];
  char *error = "couldn't %s color: %s";
  XColor x_color;
  char *copy, *address2, *address3, *color;
  char sep = ',';
  char *ptr;
  static int init=0;
  static int dpy_type;

  copy = XjNewString(string);

  /*
   * figure out the display type -- MONO, GRAY or COLOR
   */
  if (!init)
    {
      init = 1;
      if (DisplayPlanes(display, DefaultScreen(display)) > 1)
	{
	  Visual *visual;
	  visual =
	    DefaultVisualOfScreen(DefaultScreenOfDisplay(display));
	  switch(visual->class)
	    {
	    case StaticGray:
	    case GrayScale:
	      dpy_type = GRAY;
	      break;
	    case StaticColor:
	    case PseudoColor:
	    case TrueColor:
	    case DirectColor:
	      dpy_type = COLOR;
	      break;
	    default:
	      dpy_type = MONO;
	      break;
	    }
	}
    }

  /*
   * parse the resource -- if there are 2 or 3 "words", then
   * assign them to the other addresses.
   */
  if ((ptr = address2 = index(copy, sep))  !=  NULL)
    {
      ptr--;			/* strip trailing spaces off word */
      while (isspace(*ptr))	/*   before comma */
	ptr--;
      *++ptr = '\0';		/* terminate string */

      address2++;		/* strip leading spaces off word */
      while (isspace(*address2)) /*   after comma */
	address2++;

      if ((ptr = address3 = index(address2, sep))  !=  NULL)
	{
	  ptr--;		/* strip trailing spaces off word */
	  while (isspace(*ptr))	/*   before comma */
	    ptr--;
	  *++ptr = '\0';	/* terminate string */
		  
	  address3++;		/* strip leading spaces off word */
	  while (isspace(*address3)) /*   after comma */
	    address3++;

	  ptr = address3 + strlen(address3) - 1; /* strip trailing */
	  while (isspace(*ptr))	/*   spaces off last word... */
	    ptr--;
	  *++ptr = '\0';	/* terminate string */
	}
      else
	{			/* if there are only 2 words, then */
	  address3 = address2;	/* set the "color address" to the */
	  address2 = copy;	/* 2nd one, and the "grayscale */
	}			/* address" to the 1st one... */
    }
  else				/* if there is only 1 word, then */
    address3 = address2 = copy;	/* set all addresses to it... */

  /*
   * figure out which address to use based on the display type.
   */
  switch(dpy_type)
    {
    case COLOR:
      color = address3;
      break;
    case GRAY:
      color = address2;
      break;
    default:
      color = copy;
      break;
    }

  /*
   * now figure out what color it is and fill it in.
   */
  if (strcasecmp(XjNone, color) == 0)
    {
      *pixel = -1;
      XjFree(copy);
      return 0;
    }

  if (strcmp(XjDefaultForeground, color) == 0)
    {
      *pixel = BlackPixel(display, DefaultScreen(display));
      XjFree(copy);
      return 0;
    }

  if (strcmp(XjDefaultBackground, color) == 0)
    {
      *pixel = WhitePixel(display, DefaultScreen(display));
      XjFree(copy);
      return 0;
    }

  if (!XParseColor(display,
		   DefaultColormap(display, DefaultScreen(display)),
		   color,
		   &x_color))
    {
      sprintf(errtext, error, "parse", color);
      XjWarning(errtext);
      *pixel = -1;
      XjFree(copy);
      return -1;
    }


  if (!XAllocColor(display,
		   DefaultColormap(display, DefaultScreen(display)),
		   &x_color))
    {
      sprintf(errtext, error, "allocate", color);
      XjWarning(errtext);
      *pixel = -1;
      XjFree(copy);
      return -1;
    }

  *pixel = x_color.pixel;
  XjFree(copy);
  return 0;
}



/*
 * string to Color conversion
 */
int StrToXColor(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  unsigned long pixel;

  if (StrToXPixel(display, address, &pixel))
    {
      /* we give black for foreground or white for background, in */
      /* case we can't figure something better out */

      if (resource->resource_class == XjCForeground)
	*((int *)((char *)where + resource->resource_offset)) =
	  BlackPixel(display, DefaultScreen(display));
      else
	*((int *)((char *)where + resource->resource_offset)) =
	  WhitePixel(display, DefaultScreen(display));
    }
  else
    *((int *)((char *)where + resource->resource_offset)) = pixel;

  return 0;
}
