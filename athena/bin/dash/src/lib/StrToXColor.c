/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXColor.c,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToXColor.c,v 1.3 1994-05-08 23:29:57 cfields Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include <ctype.h>

#define MONO 0
#define GRAY 1
#define COLOR 2

#define MAXCOLORS 50
Colormap xjcolormap;
unsigned long writecolors[MAXCOLORS];

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
  int cnum, i;
  static int wcavail;

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
	  wcavail = 0;
	  switch(visual->class)
	    {
	    case StaticGray:
	    case GrayScale:
	      dpy_type = GRAY;
	      break;
	    case PseudoColor:
	      wcavail = 1;
	    case StaticColor:
	    case TrueColor:
	    case DirectColor:
	      dpy_type = COLOR;
	      break;
	    default:
	      dpy_type = MONO;
	      break;
	    }
	}

      for (i = 0; i < MAXCOLORS; i++)
	writecolors[i] = XjNoColor;
      xjcolormap = DefaultColormap(display,
				   DefaultScreen(display));
    }

  /*
   * parse the resource -- if there are 2 or 3 "words", then
   * assign them to the other addresses.
   */
  if ((ptr = address2 = (char *) index(copy, sep))  !=  NULL)
    {
      ptr--;			/* strip trailing spaces off word */
      while (isspace(*ptr))	/*   before comma */
	ptr--;
      *++ptr = '\0';		/* terminate string */

      address2++;		/* strip leading spaces off word */
      while (isspace(*address2)) /*   after comma */
	address2++;

      if ((ptr = address3 = (char *) index(address2, sep))  !=  NULL)
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

  /*
   * Support for writable color cells.
   */
  if (strncmp(XjColor, color, strlen(XjColor)) == 0)
    {
      cnum = atoi(&color[strlen(XjColor)]);
      if (cnum < 0 || cnum > MAXCOLORS || !wcavail)
	{
	  if (!wcavail)
	    XjWarning("display does not support dynamic colors");
	  else
	    {
	      sprintf(errtext,
		      "invalid color number: %d is not between 0 and %d",
		      cnum, MAXCOLORS);
	      XjWarning(errtext);
	    }
	  XjFree(copy);
	  *pixel = -1;
	  return -1;
	}

      if (writecolors[cnum] == XjNoColor)
	{
	  if (!XAllocColorCells(display, xjcolormap,
				False, NULL, 0,
				&writecolors[cnum], 1))
	    {
	      sprintf(errtext, "could not allocate %s", color);
	      XjWarning(errtext);
	      XjFree(copy);
	      *pixel = -1;
	      return -1;
	    }
	}

      *pixel = writecolors[cnum];
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
