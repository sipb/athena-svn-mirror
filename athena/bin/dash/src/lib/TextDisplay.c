/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.c,v $
 * $Author: ghudson $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.c,v 1.4 1996-09-19 22:23:31 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Jets.h"
#include "TextDisplay.h"
#include "xselect.h"
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#define START True
#define END False

#define offset(field) XjOffset(TextDisplayJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNdisplayWidth, XjCDisplayWidth, XjRInt, sizeof(int),
      offset(textDisplay.displayWidth), XjRString, "80" },
  { XjNdisplayHeight, XjCDisplayHeight, XjRInt, sizeof(int),
      offset(textDisplay.displayHeight), XjRString, "5" },
  { XjNtext, XjCText, XjRString, sizeof(char *),
      offset(textDisplay.text), XjRString,""},
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(textDisplay.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(textDisplay.background), XjRString, XjDefaultBackground },
  { XjNhighlightForeground, XjCForeground, XjRString, sizeof(int),
      offset(textDisplay.hl_fg_name), XjRString, "" },
  { XjNhighlightBackground, XjCBackground, XjRString, sizeof(int),
      offset(textDisplay.hl_bg_name), XjRString, "" },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(textDisplay.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(textDisplay.font), XjRString, XjDefaultFont },
  { XjNresizeProc, XjCResizeProc, XjRCallback, sizeof(XjCallback *),
      offset(textDisplay.resizeProc), XjRString, NULL },
  { XjNscrollProc, XjCScrollProc, XjRCallback, sizeof(XjCallback *),
      offset(textDisplay.scrollProc), XjRString, NULL },
  { XjNinternalBorder, XjCBorderWidth, XjRInt, sizeof(int),
      offset(textDisplay.internalBorder), XjRString, "2" },
  { XjNmultiClickTime, XjCMultiClickTime, XjRInt, sizeof(int),
      offset(textDisplay.multiClickTime), XjRString, "250" },
  { XjNcharClass, XjCCharClass, XjRString, sizeof(char *),
      offset(textDisplay.charClass), XjRString, (caddr_t) NULL},
  { XjNscrollDelay, XjCScrollDelay, XjRInt, sizeof(int),
      offset(textDisplay.scrollDelay1), XjRString, "100" },
  { XjNscrollDelay2, XjCScrollDelay, XjRInt, sizeof(int),
      offset(textDisplay.scrollDelay2), XjRString, "50" },
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize(), appendLines(), drawChars();
static Boolean event_handler();

TextDisplayClassRec textDisplayClassRec = {
  {
    /* class name */		"TextDisplay",
    /* jet size   */		sizeof(TextDisplayRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		initialize,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		expose,
    /* querySize */     	querySize,
    /* move */			move,
    /* resize */        	resize,
    /* destroy */       	destroy,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass textDisplayJetClass = (JetClass)&textDisplayClassRec;

     /* The "charClass" table below, and the functions */
     /* "SetCharacterClassRange" and "set_character_class" following it */
     /* are taken from the xterm sources, and as such are subject to the */
     /* copyright included here...:    */

/*
 * Copyright 1988 Massachusetts Institute of Technology
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
** double click table for cut and paste in 8 bits
**
** This table is divided in four parts :
**
**	- control characters	[0,0x1f] U [0x80,0x9f]
**	- separators		[0x20,0x3f] U [0xa0,0xb9]
**	- binding characters	[0x40,0x7f] U [0xc0,0xff]
**  	- execeptions
*/
static int charClass[256] = {
/* NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL */
    32,   1,   1,   1,   1,   1,   1,   1,
/*  BS   HT   NL   VT   NP   CR   SO   SI */
     1,  32,   1,   1,   1,   1,   1,   1,
/* DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB */
     1,   1,   1,   1,   1,   1,   1,   1,
/* CAN   EM  SUB  ESC   FS   GS   RS   US */
     1,   1,   1,   1,   1,   1,   1,   1,
/*  SP    !    "    #    $    %    &    ' */
    32,  33,  34,  35,  36,  37,  38,  39,
/*   (    )    *    +    ,    -    .    / */
    40,  41,  42,  43,  44,  45,  46,  47,
/*   0    1    2    3    4    5    6    7 */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   8    9    :    ;    <    =    >    ? */
    48,  48,  58,  59,  60,  61,  62,  63,
/*   @    A    B    C    D    E    F    G */
    64,  48,  48,  48,  48,  48,  48,  48,
/*   H    I    J    K    L    M    N    O */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   P    Q    R    S    T    U    V    W */ 
    48,  48,  48,  48,  48,  48,  48,  48,
/*   X    Y    Z    [    \    ]    ^    _ */
    48,  48,  48,  91,  92,  93,  94,  48,
/*   `    a    b    c    d    e    f    g */
    96,  48,  48,  48,  48,  48,  48,  48,
/*   h    i    j    k    l    m    n    o */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   p    q    r    s    t    u    v    w */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   x    y    z    {    |    }    ~  DEL */
    48,  48,  48, 123, 124, 125, 126,   1,
/* x80  x81  x82  x83  IND  NEL  SSA  ESA */
     1,   1,   1,   1,   1,   1,   1,   1,
/* HTS  HTJ  VTS  PLD  PLU   RI  SS2  SS3 */
     1,   1,   1,   1,   1,   1,   1,   1,
/* DCS  PU1  PU2  STS  CCH   MW  SPA  EPA */
     1,   1,   1,   1,   1,   1,   1,   1,
/* x98  x99  x9A  CSI   ST  OSC   PM  APC */
     1,   1,   1,   1,   1,   1,   1,   1,
/*   -    i   c/    L   ox   Y-    |   So */
   160, 161, 162, 163, 164, 165, 166, 167,
/*  ..   c0   ip   <<    _        R0    - */
   168, 169, 170, 171, 172, 173, 174, 175,
/*   o   +-    2    3    '    u   q|    . */
   176, 177, 178, 179, 180, 181, 182, 183,
/*   ,    1    2   >>  1/4  1/2  3/4    ? */
   184, 185, 186, 187, 188, 189, 190, 191,
/*  A`   A'   A^   A~   A:   Ao   AE   C, */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  E`   E'   E^   E:   I`   I'   I^   I: */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  D-   N~   O`   O'   O^   O~   O:    X */ 
    48,  48,  48,  48,  48,  48,  48, 216,
/*  O/   U`   U'   U^   U:   Y'    P    B */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  a`   a'   a^   a~   a:   ao   ae   c, */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  e`   e'   e^   e:    i`  i'   i^   i: */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   d   n~   o`   o'   o^   o~   o:   -: */
    48,  48,  48,  48,  48,  48,  48,  248,
/*  o/   u`   u'   u^   u:   y'    P   y: */
    48,  48,  48,  48,  48,  48,  48,  48};

int SetCharacterClassRange (low, high, value)
    register int low, high;		/* in range of [0..255] */
    register int value;			/* arbitrary */
{

    if (low < 0 || high > 255 || high < low) return (-1);

    for (; low <= high; low++) charClass[low] = value;

    return (0);
}


/*
 * set_character_class - takes a string of the form
 * 
 *                 low[-high]:val[,low[-high]:val[...]]
 * 
 * and sets the indicated ranges to the indicated values.
 */

int set_character_class (s)
    register char *s;
{
    register int i;			/* iterator, index into s */
    int len;				/* length of s */
    int acc;				/* accumulator */
    int low, high;			/* bounds of range [0..127] */
    int base;				/* 8, 10, 16 (octal, decimal, hex) */
    int numbers;			/* count of numbers per range */
    int digits;				/* count of digits in a number */
    static char *errfmt = "%s in range string \"%s\" (position %d)\n";
    char errtext[100];

    if (!s || !s[0]) return -1;

    base = 10;				/* in case we ever add octal, hex */
    low = high = -1;			/* out of range */

    for (i = 0, len = strlen (s), acc = 0, numbers = digits = 0;
	 i < len; i++) {
	char c = s[i];

	if (isspace(c)) {
	    continue;
	} else if (isdigit(c)) {
	    acc = acc * base + (c - '0');
	    digits++;
	    continue;
	} else if (c == '-') {
	    low = acc;
	    acc = 0;
	    if (digits == 0) {
		sprintf (errtext, errfmt, "missing number", s, i);
		XjWarning(errtext);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    continue;
	} else if (c == ':') {
	    if (numbers == 0)
	      low = acc;
	    else if (numbers == 1)
	      high = acc;
	    else {
		sprintf (errtext, errfmt, "too many numbers", s, i);
		XjWarning(errtext);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    acc = 0;
	    continue;
	} else if (c == ',') {
	    /*
	     * now, process it
	     */

	    if (high < 0) {
		high = low;
		numbers++;
	    }
	    if (numbers != 2) {
		sprintf (errtext, errfmt, "bad value number", s, i);
		XjWarning(errtext);
	    } else if (SetCharacterClassRange (low, high, acc) != 0) {
		sprintf (errtext, errfmt, "bad range", s, i);
		XjWarning(errtext);
	    }

	    low = high = -1;
	    acc = 0;
	    digits = 0;
	    numbers = 0;
	    continue;
	} else {
	    sprintf (errtext, errfmt, "bad character", s, i);
	    XjWarning(errtext);
	    return (-1);
	}				/* end if else if ... else */

    }

    if (low < 0 && high < 0) return (0);

    /*
     * now, process it
     */

    if (high < 0) high = low;
    if (numbers < 1 || numbers > 2) {
        sprintf (errtext, errfmt, "bad value number", s, i);
	XjWarning(errtext);
    } else if (SetCharacterClassRange (low, high, acc) != 0) {
        sprintf (errtext, errfmt, "bad range", s, i);
	XjWarning(errtext);
    }

    return (0);
}
     /* The "charClass" table above, and the functions */
     /* "SetCharacterClassRange" and "set_character_class" following it */
     /* are taken from the xterm sources, and as such are subject to the */
     /* copyright included above. */




static void initialize(me)
     TextDisplayJet me;
{
  me->textDisplay.realized = 0;

  me->textDisplay.charWidth = me->textDisplay.font->max_bounds.width;
  me->textDisplay.charHeight = (me->textDisplay.font->ascent
				+ me->textDisplay.font->descent);

  me->textDisplay.lineStartsSize = 1000;
  me->textDisplay.lineStarts = (char **)XjMalloc(1000 * sizeof(char *));
  me->textDisplay.startSelect = 0;
  me->textDisplay.endSelect = 0;
  me->textDisplay.realStart = 0;
  me->textDisplay.realEnd = 0;
  me->textDisplay.selection = NULL;
  me->textDisplay.clickTimes = 0;
  me->textDisplay.buttonDown = False;

  /*
   * Initialize the first lines for safety...  trust me...
   * (it's in case there's no text in the jet and someone tries to do a
   *  multi-click to select some text...  it also keeps "appendLines"
   *  from breaking...)
   */
  me->textDisplay.lineStarts[0] = "";

  set_character_class(me->textDisplay.charClass);
  xselInitAtoms(me->core.display);
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     TextDisplayJet me;
{
  unsigned long valuemask, valuemask2;
  unsigned long pixel;
  XGCValues values;

  if (me->textDisplay.reverseVideo)
    {
      int swap = me->textDisplay.foreground;
      me->textDisplay.foreground = me->textDisplay.background;
      me->textDisplay.background = swap;
    }

  if (strcmp(me->textDisplay.hl_fg_name, "")
      &&  !StrToXPixel(XjDisplay(me), me->textDisplay.hl_fg_name, &pixel))
    me->textDisplay.hl_foreground = pixel;
  else
    me->textDisplay.hl_foreground = me->textDisplay.background;

  if (strcmp(me->textDisplay.hl_bg_name, "")
      &&  !StrToXPixel(XjDisplay(me), me->textDisplay.hl_bg_name, &pixel))
    me->textDisplay.hl_background = pixel;
  else
    me->textDisplay.hl_background = me->textDisplay.foreground;

  values.function = GXcopy;
  values.font = me->textDisplay.font->fid;
  values.foreground = me->textDisplay.foreground;
  values.background = me->textDisplay.background;
  valuemask = GCForeground | GCBackground | GCFont | GCFunction;
  me->textDisplay.gc = XjCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

  values.foreground = values.background;
  valuemask2 = GCForeground | GCFunction;
  me->textDisplay.gc_clear = XjCreateGC(me->core.display,
					me->core.window,
					valuemask2,
					&values);


  values.foreground = ((me->textDisplay.foreground
			== me->textDisplay.hl_foreground)
		       ? me->textDisplay.background
		       : me->textDisplay.hl_foreground);
  values.background = ((me->textDisplay.background
			== me->textDisplay.hl_background)
		       ? me->textDisplay.foreground
		       : me->textDisplay.hl_background);
  valuemask = GCForeground | GCBackground | GCFont | GCFunction;
  me->textDisplay.selectgc = XjCreateGC(me->core.display,
					me->core.window,
					valuemask,
					&values);

  values.foreground = values.background;
  valuemask2 = GCForeground | GCFunction;
  me->textDisplay.gc_fill = XjCreateGC(me->core.display,
				       me->core.window,
				       valuemask2,
				       &values);

  me->textDisplay.visLines = (me->core.height -
			      2 * me->textDisplay.internalBorder) /
				me->textDisplay.charHeight;

  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ButtonPressMask | ButtonReleaseMask
		| ButtonMotionMask | KeyPressMask );

  me->textDisplay.realized = 1;
}

static void destroy(me)
     TextDisplayJet me;
{
  XjFreeGC(me->core.display, me->textDisplay.gc);
  XjFreeGC(me->core.display, me->textDisplay.selectgc);
  XjFreeGC(me->core.display, me->textDisplay.gc_fill);
  XjFreeGC(me->core.display, me->textDisplay.gc_clear);

  XjUnregisterWindow(me->core.window, (Jet) me);
}

static void querySize(me, size)
     TextDisplayJet me;
     XjSize *size;
{
  size->width = me->textDisplay.displayWidth * me->textDisplay.charWidth +
    2 * me->textDisplay.internalBorder;
  size->height = me->textDisplay.displayHeight * me->textDisplay.charHeight +
    2 * me->textDisplay.internalBorder;
}

static void move(me, x, y)
     TextDisplayJet me;
     int x, y;
{
  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     TextDisplayJet me;
     XjSize *size;
{
  me->core.width = size->width;
  me->core.height = size->height;
  me->textDisplay.visLines = (me->core.height -
			      2 * me->textDisplay.internalBorder) /
				me->textDisplay.charHeight;
  me->textDisplay.columns = (me->core.width -
			     2 * me->textDisplay.internalBorder) /
			       me->textDisplay.charWidth;
  if (me->textDisplay.realized)
    appendLines(me, me->textDisplay.text, 0);
}

static void drawLine(me, line, y)
     TextDisplayJet me;
     int line, y;
{
  int c = 0, length;
  char *ptr, *end, *last;

  ptr = me->textDisplay.lineStarts[line];
  last = me->textDisplay.lineStarts[line + 1];

  while (ptr != last)
    {
      end = ptr;

      while (end < last && *end != '\t')
	end++;

      length = end - ptr;

      if (length != 0)
	{
	  XDrawString(me->core.display, me->core.window,
		      me->textDisplay.gc,
		      me->core.x + me->textDisplay.internalBorder +
		      c * me->textDisplay.charWidth,
		      y + me->textDisplay.font->ascent,
		      ptr, length - (ptr[length - 1] == '\n'));
	  c += length;
	}

      if (end < last && *end == '\t') /* could be reduced, but... */
	{
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < last && *end == '\t')
	    {
	      end++;
	      length++;
	      c += 8;
	    }
	}
	    
      ptr += length;
    }
}

static void drawText(me, start, num, y)
     TextDisplayJet me;
     int start;
     int num, y;
{
  char *first, *last;

  first = me->textDisplay.lineStarts[start];
  last = me->textDisplay.lineStarts[MIN(start + num,
					   me->textDisplay.numLines)];

  
  if (me->textDisplay.startSelect == me->textDisplay.endSelect ||
      last <= me->textDisplay.startSelect ||
      first >= me->textDisplay.endSelect)
    {
      while (num != 0 && start < me->textDisplay.numLines)
	{
	  drawLine(me, start, y);
	  start++;
	  num--;
	  y += me->textDisplay.charHeight;
	}
/*
      drawChars(me, me->textDisplay.gc, first, last);
*/
      return;
    }

  /*
   * Since we're here, there must be some selected text drawn.
   */
  if (first <= me->textDisplay.startSelect) /* last > startSelect */
    {
      drawChars(me, me->textDisplay.gc,
		first, me->textDisplay.startSelect);
      first = me->textDisplay.startSelect;
    }

  drawChars(me, me->textDisplay.selectgc,
	    first, MIN(me->textDisplay.endSelect, last));

  if (last > me->textDisplay.endSelect)
    drawChars(me, me->textDisplay.gc,
	      me->textDisplay.endSelect, last);
}

static void drawSome(me, gc, y, startOfLine, start, last)
     TextDisplayJet me;
     GC gc;
     int y;
     char *startOfLine, *start, *last;
{
  int c = 0, length;
  char *ptr, *end;

/*  fprintf(stdout, "%d\n", gc == me->textDisplay.selectgc); */
  /*
   * At the end of this while, c is the column number to start
   * drawing at, and ptr is start.
   */
  ptr = startOfLine;
  while (ptr != start)
    {
      end = ptr;

      while (end < start && *end != '\t')
	end++;

      length = end - ptr;
      c += length;

      if (end < start && *end == '\t') /* could be reduced, but... */
	{
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < start && *end == '\t')
	    {
	      end++;
	      length++;
	      c += 8;
	    }
	}
	    
      ptr += length;
    }

  while (ptr != last)
    {
      end = ptr;

      while (end < last && *end != '\t')
	end++;

      length = end - ptr;

      if (length != 0)
	{
	  XDrawImageString(me->core.display, me->core.window,
			   gc,
			   me->core.x + me->textDisplay.internalBorder +
			   c * me->textDisplay.charWidth,
			   y + me->textDisplay.font->ascent,
			   ptr, length - (ptr[length - 1] == '\n'));
/*	  fprintf(stdout, "%.*s",  length - (ptr[length - 1] == '\n'),
		  ptr);
	  fflush(stdout); */
	  c += length;

	  /* this is gross... */
	  if (ptr[length-1] == '\n')
	    {
	      int tmp = (c-1) * me->textDisplay.charWidth +
		me->textDisplay.internalBorder;

	      XFillRectangle(me->core.display, me->core.window,
			     ((gc == me->textDisplay.selectgc)
			      ? me->textDisplay.gc_fill
			      : me->textDisplay.gc_clear),
			     me->core.x + tmp,
			     y,
			     MAX(0, (me->textDisplay.columns + 1 - c)
				 * me->textDisplay.charWidth),
			     (me->textDisplay.font->ascent +
			      me->textDisplay.font->descent));
	    }
	}

      if (end < last && *end == '\t') /* could be reduced, but... */
	{
	  XDrawImageString(me->core.display, me->core.window,
			   gc,
			   me->core.x + me->textDisplay.internalBorder +
			   c * me->textDisplay.charWidth,
			   y + me->textDisplay.font->ascent,
			   "        ", 8 - (c%8));
/*	  fprintf(stdout, "%.*s", 8-(c%8), "        ");
	  fflush(stdout); */
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < last && *end == '\t')
	    {
	      XDrawImageString(me->core.display, me->core.window,
			       gc,
			       me->core.x + me->textDisplay.internalBorder +
			       c * me->textDisplay.charWidth,
			       y + me->textDisplay.font->ascent,
			       "        ", 8);
	      end++;
	      length++;
	      c += 8;
/*	      fprintf(stdout, "        ");
	      fflush(stdout); */
	    }
	}
	    
      ptr += length;
    }
}

static void drawChars(me, gc, start, end)
     TextDisplayJet me;
     GC gc;
     char *start, *end;
{
  int y;
  int line;

  for (line = me->textDisplay.topLine;
       line < me->textDisplay.topLine + me->textDisplay.visLines &&
       line < me->textDisplay.numLines;
       line++)
    if (me->textDisplay.lineStarts[line + 1] > start)
      break;

  if (me->textDisplay.lineStarts[line] > start)
    start = me->textDisplay.lineStarts[line];

  if (start >= end)
    return;

  if (line >= me->textDisplay.topLine + me->textDisplay.visLines ||
      line >= me->textDisplay.numLines)
    return;

  y = me->core.y + me->textDisplay.internalBorder +
    (line - me->textDisplay.topLine) * me->textDisplay.charHeight;

  for (;line < me->textDisplay.topLine + me->textDisplay.visLines &&
       line < me->textDisplay.numLines; line++)
    {
      drawSome(me, gc, y, me->textDisplay.lineStarts[line],
	       start,
	       MIN(me->textDisplay.lineStarts[line + 1], end));
      y += me->textDisplay.charHeight;
      start = me->textDisplay.lineStarts[line + 1];
      if (start > end)
	break;
    }
}

static void drawCharacters(me, start, end)
     TextDisplayJet me;
     char *start, *end;
{
  if (start < me->textDisplay.startSelect)
    {
      if (end <= me->textDisplay.startSelect)
	{
	  drawChars(me, me->textDisplay.gc, start, end);
	  return;
	}

      drawChars(me, me->textDisplay.gc,
		start,
		me->textDisplay.startSelect);
      start = me->textDisplay.startSelect;
    }

  if (start < me->textDisplay.endSelect)
    {
      if (end <= me->textDisplay.endSelect)
	{
	  drawChars(me, me->textDisplay.selectgc, start, end);
	  return;
	}

      drawChars(me, me->textDisplay.selectgc,
		start,
		me->textDisplay.endSelect);
      start = me->textDisplay.endSelect;
    }

  drawChars(me, me->textDisplay.gc, start, end);
}

/*
 * This draws only the regions of text whose selected
 * state has changed.
 */
static void showSelect(me, selStart, selEnd)
     TextDisplayJet me;
     char *selStart, *selEnd;
{
  int i, j;
  char *k, *s[4];

  s[0] = selStart;
  s[1] = selEnd;
  s[2] = me->textDisplay.startSelect;
  s[3] = me->textDisplay.endSelect;

  /* sort the array into order. */
  for (i = 0; i < 3; i++)
    for (j = i + 1; j < 4; j++)
      if (s[i] > s[j])
	{
	  k = s[i];
	  s[i] = s[j];
	  s[j] = k;
	}

  if (s[0] != s[1])
    drawCharacters(me, s[0], s[1]);

  if (s[2] != s[3])
    drawCharacters(me, s[2], s[3]);
}


static int timerid = -1;

void auto_scroll(me, id)
     TextDisplayJet me;
     int id;			/* ARGSUSED */
{
  Window junkwin;
  int x, y, junk;
  unsigned int junkmask;
  XEvent event;

  timerid = -1;

  XQueryPointer(XjDisplay(me), XjWindow(me),
		&junkwin, &junkwin, &junk, &junk, &x, &y, &junkmask);

  event.type = MotionNotify;	/* Fake out the event_handler into */
  event.xbutton.x = x;		/* thinking that the mouse has moved. */
  event.xbutton.y = y;		/* It will then deal with selecting and */
  event_handler(me, &event);	/* scrolling more, as needed. */
}


char *where(me, x, y, line)
     TextDisplayJet me;
     int x, y;
     int *line;
{
  char *ptr, *end;
  int col, tmp, vert, c = 0;
  int clickTimes = me->textDisplay.clickTimes % 5;
#define LINE *line
  
  if (me->textDisplay.numLines == 0)
    {
      LINE = 0;
      return me->textDisplay.lineStarts[0];
    }

  vert = (y - me->core.y - me->textDisplay.internalBorder);
  LINE = ((y - me->core.y - me->textDisplay.internalBorder) /
	  me->textDisplay.charHeight) + me->textDisplay.topLine;
  col = ((x - me->core.x - me->textDisplay.internalBorder) /
	 me->textDisplay.charWidth);

  /*
   * Deal with autoscrolling...  first check if we want to scroll up...
   */
  if (vert < -20  &&  me->textDisplay.topLine > 0
      &&  timerid == -1
      &&  (me->textDisplay.scrollDelay1 >= 0 ||
	   me->textDisplay.scrollDelay2 >= 0))
    {
      SetLine(me, me->textDisplay.topLine - 1);
      XjCallCallbacks(me, me->textDisplay.scrollProc, NULL);

      timerid = XjAddWakeup(auto_scroll, me,
			    (vert < -40  &&
			     me->textDisplay.scrollDelay2 >= 0)
			    ? me->textDisplay.scrollDelay2
			    : me->textDisplay.scrollDelay1);
    }

  /* bounds checking */
  if (LINE < me->textDisplay.topLine)
    LINE = me->textDisplay.topLine;
#ifdef notdef
  if (LINE < 0)			/* is this really necessary??? */
    LINE = 0;
#endif
  if (vert < -40)
    return me->textDisplay.lineStarts[LINE];

  /*
   * ...now check if we're past the last line, and the last line is
   *  *not* at the bottom of the jet...  if this is the case, then we
   *  want to return the end of the text...
   */
  if (LINE > me->textDisplay.numLines
      &&  me->textDisplay.visLines > me->textDisplay.numLines)
    {
      LINE = me->textDisplay.numLines;
      return me->textDisplay.lineStarts[me->textDisplay.numLines];
    }

  /*
   * ...then check if we want to scroll down...
   */
  tmp = me->textDisplay.topLine + me->textDisplay.visLines;
  if (LINE >= tmp)
    LINE = tmp - 1;

  if (vert - me->core.height > 20  &&  tmp < me->textDisplay.numLines
      &&  timerid == -1
      &&  (me->textDisplay.scrollDelay1 >= 0 ||
	   me->textDisplay.scrollDelay2 >= 0))
    {
      SetLine(me, me->textDisplay.topLine + 1);	/* scroll down... */
      XjCallCallbacks(me, me->textDisplay.scrollProc, NULL);
      LINE += 1;

      timerid = XjAddWakeup(auto_scroll, me,
			    (vert - me->core.height > 40  &&
			     me->textDisplay.scrollDelay2 >= 0)
			    ? me->textDisplay.scrollDelay2
			    : me->textDisplay.scrollDelay1);
    }

  /* bounds checking */
#ifdef notdef
  if (LINE == me->textDisplay.numLines - 1)
    {
#endif
      if (vert - me->core.height > 40)
	return me->textDisplay.lineStarts[LINE + 1];
#ifdef notdef
    }
#endif
  if (LINE >= me->textDisplay.numLines)
    LINE = me->textDisplay.numLines - 1;


  ptr = me->textDisplay.lineStarts[LINE];
  end = me->textDisplay.lineStarts[LINE + 1];

  while (c < col && ptr < end)
    {
      while ((c < col) && *ptr != '\t' && ptr < end)
	{
	  ptr++;
	  c++;
	}

      if ((c < col) && ptr < end && *ptr == '\t')
	{
	  c = 8 * ((c / 8) + 1);
	  ptr++;

	  while ((c < col) && *ptr == '\t' && ptr < end)
	    {
	      c += 8;
	      ptr++;
	    }
	}
    }

  if (clickTimes == 1)
    return ptr;

#ifdef notdef
  if (me->textDisplay.columns <= col)	/* to deal with dragging off the */
    return ptr;				/* right edge */	
#endif
  if (ptr == end)
    ptr = end - 1;

  return ptr;

#undef LINE
}

char *find_boundary(me, ptr, line, find_start)
     TextDisplayJet me;
     char **ptr;
     int line;
     Boolean find_start;
{
  int clickTimes = me->textDisplay.clickTimes % 5;

  if (me->textDisplay.numLines == 0)
    return *ptr;

  if (clickTimes == 1)		/* single-clicks - select by char */
    return *ptr;

  if (clickTimes == 2)		/* double-clicks - select by "word" */
    {
      char *tmp;
      
      if (find_start)
	{
	  tmp = *ptr - 1;
	  while(tmp >= me->textDisplay.lineStarts[0] &&
		charClass[*tmp] == charClass[**ptr])
	    tmp--;
	  tmp++;
	}
      else
	{
	  tmp = *ptr;
	  while(tmp <= me->textDisplay.lineStarts[me->textDisplay.numLines] &&
		charClass[*tmp] == charClass[**ptr])
	    tmp++;
	}
      return tmp;
    }

  if (clickTimes == 3)		/* triple-clicks - select by "line" */
    return me->textDisplay.lineStarts[(find_start)
				      ? line
				      : MIN(me->textDisplay.numLines,
					    line + 1)];

  if (clickTimes == 4)		/* quad-clicks - select by "paragraph" */
    {
      char *tmp;
      int blankline, x;

      if (find_start)
	{
	  /*
	   *  scan backward to the beginning of the "paragraph"
	   */
	  for (x = line;
	       x >= 0;
	       x--)
	    {
	      blankline = True;
	      if (x < me->textDisplay.numLines)
		for (tmp = me->textDisplay.lineStarts[x];
		     blankline && tmp <= (me->textDisplay.lineStarts[x+1] - 1);
		     tmp++)
		  if (*tmp != ' '  && *tmp != '\t' &&
		      *tmp != '\n' && *tmp != '\0')
		    blankline = False;

	      if (blankline)
		return me->textDisplay.lineStarts[(x == line) ? x : x+1];
	    }
	  return me->textDisplay.lineStarts[0];
	}

      else
	{
	  /*
	   *  scan forward to the end of the "paragraph"
	   */
	  for (x = line;
	       x < me->textDisplay.numLines;
	       x++)
	    {
	      blankline = True;
	      for (tmp = me->textDisplay.lineStarts[x];
		   blankline && tmp <= (me->textDisplay.lineStarts[x+1] - 1);
		   tmp++)
		if (*tmp != ' '  && *tmp != '\t' &&
		    *tmp != '\n' && *tmp != '\0')
		  blankline = False;

	      if (blankline)
		return me->textDisplay.lineStarts[(x == line) ? x+1 : x];
	    }
	  return me->textDisplay.lineStarts[x];
	}
    }

				/* quint-clicks - select all text */
  return me->textDisplay.lineStarts[(find_start)
				    ? 0 : me->textDisplay.numLines];
}


static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  drawText((TextDisplayJet) me,
	   ((TextDisplayJet) me)->textDisplay.topLine,
	   ((TextDisplayJet) me)->textDisplay.visLines,
	   me->core.y + ((TextDisplayJet) me)->textDisplay.internalBorder);
}

static void appendLines(me, text, num)
     TextDisplayJet me;
     char *text;
     int num;
{
  int c, col;
  char *top = me->textDisplay.lineStarts[me->textDisplay.topLine];

  if (me->textDisplay.columns < 1 ||
      text == NULL)
    return;

  while (*text != '\0')
    {
      /* The +2 is a useful hack. */
      if (num + 2 > me->textDisplay.lineStartsSize)
	{
	  me->textDisplay.lineStarts =
	    (char **)XjRealloc(me->textDisplay.lineStarts,
			       (1000 + me->textDisplay.lineStartsSize)
			       * sizeof(char *));
	  me->textDisplay.lineStartsSize += 1000;
	}

      me->textDisplay.lineStarts[num] = text;
      if (text == top)
	me->textDisplay.topLine = num;

      col = 0;
      for (c = 0; col < me->textDisplay.columns; c++)
	switch(text[c])
	  {
	  case '\0':
	    c--;
	  case '\n':
	    col = me->textDisplay.columns;
	    break;
	  case '\t':
	    col = 8 * ((col / 8) + 1);
	    break;
	  default:
	    col++;
	    break;
	  }
      text += c;
      if (/* text > me->textDisplay.text && not necessary */
	  text[0] == '\n' && text[-1] != '\n') /* then wrapped */
	text++; /* stupid to <cr> here */

      num++;
    }

  me->textDisplay.lineStarts[num] = text;
  me->textDisplay.numLines = num;
  if (text == top)
    me->textDisplay.topLine = num;

  if (me->textDisplay.realized)
    XjCallCallbacks(me, me->textDisplay.resizeProc, NULL);
}

void showNewLines(me, start)
     TextDisplayJet me;
     int start;
{
  if (start >= me->textDisplay.topLine &&
      start <= (me->textDisplay.topLine + me->textDisplay.visLines - 1))
    drawText(me,
	     start,
	     me->textDisplay.topLine + me->textDisplay.visLines - start,
	     me->core.y + me->textDisplay.internalBorder +
	     (start - me->textDisplay.topLine) *
	     me->textDisplay.charHeight);
}

void AddText(me)
     TextDisplayJet me;
{
  int l;

  l = me->textDisplay.numLines;
  if (l > 0)  l--;
  appendLines(me, me->textDisplay.lineStarts[l], l);
  if (me->textDisplay.realized)
    showNewLines(me, l);
}

void MoveText(me, text, offset)
     TextDisplayJet me;
     char *text;
     int offset;
{
  me->textDisplay.startSelect = (char *)
    MAX((int) text, (int) me->textDisplay.startSelect - offset);
  me->textDisplay.endSelect = (char *)
    MAX((int) text, (int) me->textDisplay.endSelect - offset);
  me->textDisplay.realStart = (char *)
    MAX((int) text, (int) me->textDisplay.realStart - offset);
  me->textDisplay.realEnd = (char *)
    MAX((int) text, (int) me->textDisplay.realEnd - offset);

  me->textDisplay.text = text;
  me->textDisplay.topLine = 0;

  appendLines(me, text, 0);
  if (me->textDisplay.realized)
    XClearArea(me->core.display, me->core.window,
	       me->core.x, me->core.y,
	       me->core.width, me->core.height, True);
}

void SetText(me, text)
     TextDisplayJet me;
     char *text;
{
  me->textDisplay.startSelect = text;
  me->textDisplay.endSelect = text;
  me->textDisplay.realStart = text;
  me->textDisplay.realEnd = text;

  MoveText(me, text, 0);
}

int TopLine(me)
     TextDisplayJet me;
{
  return me->textDisplay.topLine;
}

int VisibleLines(me)
     TextDisplayJet me;
{
  return me->textDisplay.visLines;
}

int CountLines(me)
     TextDisplayJet me;
{
  return me->textDisplay.numLines;
}

#define abs(x) (((x) < 0) ? (-(x)) : (x))

void SetLine(me, value)
     TextDisplayJet me;
     int value;
{
  int diff, charHeight;

  if (value == me->textDisplay.topLine)
    return;

  charHeight = me->textDisplay.charHeight;

  diff = value - me->textDisplay.topLine;
  if (diff > 0 &&
      diff < me->textDisplay.visLines)
    { /* scroll */
      XCopyArea(me->core.display,
		me->core.window,
		me->core.window,
		me->textDisplay.gc,
		me->core.x,
		me->core.y + me->textDisplay.internalBorder +
		diff * charHeight,
		me->core.width,
		(me->textDisplay.visLines - diff) * charHeight,
		me->core.x,
		me->core.y + me->textDisplay.internalBorder);

      XClearArea(me->core.display,
		 me->core.window,
		 me->core.x,
		 me->core.y + me->textDisplay.internalBorder +
		 (me->textDisplay.visLines - diff) * charHeight,
		 me->core.width,
		 diff * charHeight,
		 False);

      me->textDisplay.topLine = value;

      drawText(me,
	       value + me->textDisplay.visLines - diff,
	       diff,
	       me->core.y + me->textDisplay.internalBorder +
	       (me->textDisplay.visLines - diff) * charHeight);
    }
  else
    if (diff < 0 &&
	(-diff) < me->textDisplay.visLines)
      {
	/* scroll */
	diff = -diff;
	
	XCopyArea(me->core.display,
		  me->core.window,
		  me->core.window,
		  me->textDisplay.gc,
		  me->core.x,
		  me->core.y + me->textDisplay.internalBorder,
		  me->core.width,
		  (me->textDisplay.visLines - diff) * charHeight,
		  me->core.x,
		  me->core.y + me->textDisplay.internalBorder +
		  diff * charHeight);

	XClearArea(me->core.display,
		   me->core.window,
		   me->core.x,
		   me->core.y + me->textDisplay.internalBorder,
		   me->core.width,
		   diff * charHeight,
		   False);
      
	me->textDisplay.topLine = value;

	drawText(me,
		 me->textDisplay.topLine,
		 diff,
		 me->core.y + me->textDisplay.internalBorder);
      }
  else
    { /* completely new screen... */
      XClearArea(me->core.display,
		 me->core.window,
		 me->core.x,
		 me->core.y,
		 me->core.width,
		 me->core.height,
		 False);

      me->textDisplay.topLine = value;

      drawText(me,
	       me->textDisplay.topLine,
	       me->textDisplay.visLines,
	       me->core.y + me->textDisplay.internalBorder);
    }
}

static Boolean event_handler(me, event)
     TextDisplayJet me;
     XEvent *event;
{
  char *w;
  int line;
  char *oldStart, *oldEnd;
  int length;

  oldStart = me->textDisplay.startSelect;
  oldEnd = me->textDisplay.endSelect;

  switch(event->type)
    {
    case KeyPress:
      switch (XLookupKeysym(&(event->xkey), 0))
	{
	case XK_Left:
	case XK_Up:
	  if (me->textDisplay.topLine > 0)
	    SetLine(me, me->textDisplay.topLine - 1);
	  break;

	case XK_Right:
	case XK_Down:
	  if (me->textDisplay.topLine + me->textDisplay.visLines
	      < me->textDisplay.numLines)
	    SetLine(me, me->textDisplay.topLine + 1);
	  break;

	case XK_R9:		/* "PgUp" on the Sun kbd. */
	case XK_Prior:
	  {
	    int tmp = me->textDisplay.topLine - (me->textDisplay.visLines - 1);
	    SetLine(me, MAX(0, tmp));
	  }
	  break;

	case XK_R15:		/* "PgDn" on the Sun kbd. */
	case XK_Next:
	  if (me->textDisplay.numLines > me->textDisplay.visLines)
	    {
	      int tmp = me->textDisplay.topLine + me->textDisplay.visLines - 1;
	      int tmp2 = me->textDisplay.numLines - me->textDisplay.visLines;
	      SetLine(me, MIN(tmp, tmp2));
	    }
	  break;

	case XK_R7:		/* "Home" on the Sun kbd. */
	case XK_Home:
	  SetLine(me, 0);
	  break;

	case XK_R13:		/* "End" on the Sun kbd. */
	case XK_End:
	  if (me->textDisplay.numLines > me->textDisplay.visLines)
	    SetLine(me, me->textDisplay.numLines - me->textDisplay.visLines);
	  break;

	default:
	  return False;
	}

      XjCallCallbacks(me, me->textDisplay.scrollProc, NULL);
      break;

    case ButtonRelease:
      if (me->textDisplay.buttonDown == False)
	break;
      me->textDisplay.buttonDown = False;

      if (event->xbutton.button == Button1)
	me->textDisplay.lastUpTime = event->xbutton.time;

      me->textDisplay.realStart = me->textDisplay.startSelect;
      me->textDisplay.realEnd = me->textDisplay.endSelect;

      if (me->textDisplay.startSelect == me->textDisplay.endSelect)
	break;

      if (xselGetOwnership(me->core.display, /* bug! */
			   me->core.window, 
			   event->xbutton.time))
	{
	  length = (int)(me->textDisplay.realEnd -
			 me->textDisplay.realStart);
	  if (me->textDisplay.selection != NULL)
	    XjFree(me->textDisplay.selection);
	  me->textDisplay.selection = (char *)XjMalloc(length + 1);
	  memcpy(me->textDisplay.selection,
		me->textDisplay.realStart, length);
	  me->textDisplay.selection[length] = '\0';
	}
      break;

    case ButtonPress:
      if (me->textDisplay.buttonDown == True)
	break;

      /* MUST compute clickTimes before calling "where" below, as the */
      /* return values from it depend on the value of clickTime... */

      if (event->xbutton.button == Button1)
	{
	  if (me->textDisplay.clickTimes != 0
	      && event->xbutton.time - me->textDisplay.lastUpTime
	      < me->textDisplay.multiClickTime)
	    me->textDisplay.clickTimes++;
	  else
	    me->textDisplay.clickTimes = 1;
	}

      /* deal with button3 being first one pushed... */
      if (event->xbutton.button == Button3
	  &&  me->textDisplay.clickTimes == 0)
	me->textDisplay.clickTimes = 1;

      me->textDisplay.buttonDown = True;
      me->textDisplay.whichButton = event->xbutton.button;
      w = where(me, event->xbutton.x, event->xbutton.y, &line);

      switch(me->textDisplay.whichButton)
	{
	case Button1:
	  me->textDisplay.startSelect = me->textDisplay.startPivot =
	    find_boundary(me, &w, line, START);
	  me->textDisplay.endSelect = me->textDisplay.endPivot =
	    find_boundary(me, &w, line, END);
	  break;

	case Button3:
	  if ((int)w <
	      ((int)me->textDisplay.realStart +
	       (int)me->textDisplay.realEnd)
	      / 2)
	    {
	      me->textDisplay.startEnd = 0;
	      me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	      me->textDisplay.endSelect = me->textDisplay.realEnd;
	    }
	  else
	    {
	      me->textDisplay.startEnd = 1;
	      me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	      me->textDisplay.startSelect = me->textDisplay.realStart;
	    }
	  break;
	}

      showSelect(me, oldStart, oldEnd);
      break;

    case MotionNotify:
      /* sigh... Xqvss can't be trusted, due to kernel bug */
      if (me->textDisplay.buttonDown == False)
	break;

      w = where(me, event->xbutton.x, event->xbutton.y, &line);

      switch(me->textDisplay.whichButton)
	{
	case Button1:
	  me->textDisplay.startSelect = me->textDisplay.startPivot;
	  me->textDisplay.endSelect = me->textDisplay.endPivot;

	  if (w >= me->textDisplay.endPivot)
	    me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	  else if (w < me->textDisplay.startPivot)
	    me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	  break;

	case Button3:
	  if (w > me->textDisplay.realEnd)
	    {
	      me->textDisplay.startEnd = 1;
	      me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	      me->textDisplay.startSelect = me->textDisplay.realStart;
	    }
	  else if (w < me->textDisplay.realStart)
	    {
	      me->textDisplay.startEnd = 0;
	      me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	      me->textDisplay.endSelect = me->textDisplay.realEnd;
	    }
	  else if (me->textDisplay.startEnd == 0)
	    me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	  else
	    me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	  break;
	}
      showSelect(me, oldStart, oldEnd);
      break;

    case SelectionRequest:
      xselProcessSelection(me->core.display, me->core.window, event,
			   me->textDisplay.selection);
      break;

    case SelectionClear:
      xselOwnershipLost(event->xselectionclear.time);
      me->textDisplay.endSelect = me->textDisplay.startSelect;
      showSelect(me, oldStart, oldEnd);
      if (me->textDisplay.selection != NULL)
	{
	  XjFree(me->textDisplay.selection);
	  me->textDisplay.selection = NULL;
	}
      break;

    default:
      return False;
    }

  return True;
}
