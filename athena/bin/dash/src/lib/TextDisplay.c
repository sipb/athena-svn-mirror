/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.c,v 1.1 1991-09-03 11:09:51 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include "Jets.h"
#include "TextDisplay.h"

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
  { XjNtext, XjCText, XjRString, sizeof(char *),
     offset(textDisplay.text), XjRString,"this is\nsome sample\ntext\n\nhi!" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(textDisplay.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(textDisplay.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(textDisplay.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
     offset(textDisplay.font), XjRString, XjDefaultFont },
  { XjNresizeProc, XjCResizeProc, XjRCallback, sizeof(XjCallback *),
     offset(textDisplay.resizeProc), XjRString, NULL },
  { XjNinternalBorder, XjCBorderWidth, XjRInt, sizeof(int),
     offset(textDisplay.internalBorder), XjRString, "2" },
  { XjNmultiClickTime, XjCMultiClickTime, XjRInt, sizeof(int),
     offset(textDisplay.multiClickTime), XjRString, "250" },
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize(), appendLines(), drawChars();
static Boolean event_handler();

TextDisplayClassRec textDisplayClassRec = {
  {
    /* class name */	"TextDisplay",
    /* jet size   */	sizeof(TextDisplayRec),
    /* initialize */	initialize,
    /* prerealize */    NULL,
    /* realize */	realize,
    /* event */		event_handler,
    /* expose */	expose,
    /* querySize */     querySize,
    /* move */		move,
    /* resize */        resize,
    /* destroy */       destroy,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass textDisplayJetClass = (JetClass)&textDisplayClassRec;

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


static void initialize(me)
     TextDisplayJet me;
{
  me->textDisplay.realized = 0;

  me->textDisplay.charWidth = me->textDisplay.font->max_bounds.width;

  me->textDisplay.lineStartsSize = 1000;
  me->textDisplay.lineStarts = (char **)XjMalloc(1000 * sizeof(char *));
  me->textDisplay.startSelect = 0;
  me->textDisplay.endSelect = 0;
  me->textDisplay.realStart = 0;
  me->textDisplay.realEnd = 0;
  me->textDisplay.selection = NULL;
  me->textDisplay.clickTimes = 0;
  me->textDisplay.buttonDown = False;

  xselInitAtoms(me->core.display);
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     TextDisplayJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->textDisplay.reverseVideo)
    {
      int swap = me->textDisplay.foreground;
      me->textDisplay.foreground = me->textDisplay.background;
      me->textDisplay.background = swap;
    }

  values.function = GXcopy;
  values.font = me->textDisplay.font->fid;
  values.foreground = me->textDisplay.foreground;
  values.background = me->textDisplay.background;
  valuemask = GCForeground | GCBackground | GCFont | GCFunction;

  me->textDisplay.gc = XCreateGC(me->core.display,
			   me->core.window,
			   valuemask,
			   &values);

  values.foreground = me->textDisplay.background;
  values.background = me->textDisplay.foreground;
  me->textDisplay.selectgc = XCreateGC(me->core.display,
				       me->core.window,
				       valuemask,
				       &values);
  me->textDisplay.visLines = (me->core.height -
			      2 * me->textDisplay.internalBorder) /
		(me->textDisplay.font->ascent + me->textDisplay.font->descent);

  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, me);
  XjSelectInput(me->core.display, me->core.window,
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);

  me->textDisplay.realized = 1;
}

static void destroy(me)
     TextDisplayJet me;
{
  XFreeGC(me->core.display, me->textDisplay.gc);
}

static void querySize(me, size)
     TextDisplayJet me;
     XjSize *size;
{
  size->width = 80 * me->textDisplay.charWidth;
  size->height = 5 *
    (me->textDisplay.font->ascent + me->textDisplay.font->descent);
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
		(me->textDisplay.font->ascent + me->textDisplay.font->descent);
  me->textDisplay.columns = (me->core.width -
			     2 * me->textDisplay.internalBorder) /
			       me->textDisplay.charWidth;
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
	  y += me->textDisplay.font->ascent + me->textDisplay.font->descent;
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

/*  fprintf(stdout, "\n%d ", gc == me->textDisplay.selectgc); */
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
    (line - me->textDisplay.topLine) * (me->textDisplay.font->ascent +
					me->textDisplay.font->descent);

  for (;line < me->textDisplay.topLine + me->textDisplay.visLines &&
       line < me->textDisplay.numLines; line++)
    {
      drawSome(me, gc, y, me->textDisplay.lineStarts[line],
	       start,
	       MIN(me->textDisplay.lineStarts[line + 1], end));
      y += me->textDisplay.font->ascent + me->textDisplay.font->descent;
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

  /* six! */
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


char *where(me, x, y, line)
     TextDisplayJet me;
     int x, y;
     int *line;
{
  char *ptr, *end;
  int col, tmp, c = 0;

  if (me->textDisplay.numLines == 0)
    {
      ptr = me->textDisplay.lineStarts[0];
      return ptr;
    }

  *line = ((y - me->core.y - me->textDisplay.internalBorder) /
	   (me->textDisplay.font->ascent + me->textDisplay.font->descent)) +
	     me->textDisplay.topLine;
  col = ((x - me->core.x - me->textDisplay.internalBorder) /
	 me->textDisplay.charWidth);

  if (*line < me->textDisplay.topLine)
    *line = me->textDisplay.topLine;

  tmp = MIN(me->textDisplay.topLine + me->textDisplay.visLines,
	    me->textDisplay.numLines);
  if (*line >= tmp)
    *line = tmp - 1;

  ptr = me->textDisplay.lineStarts[*line];
  end = me->textDisplay.lineStarts[*line + 1];

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

  if (ptr == end)
    ptr = end - 1;
  return ptr;
}


char *find_start(me, ptr, line)
     TextDisplayJet me;
     char **ptr;
     int line;
{
  int clickTimes = me->textDisplay.clickTimes % 5;

  if (clickTimes == 1)		/* single-clicks - select by char */
    return *ptr;

  if (clickTimes == 2)		/* double-clicks - select by "word" */
    {
      char *tmp;

      for (tmp = *ptr - 1;
	   tmp >= me->textDisplay.lineStarts[0] &&
	   charClass[*tmp] == charClass[**ptr];
	   tmp--);
      tmp++;
      return tmp;
    }

  if (clickTimes == 3)		/* triple-clicks - select by "line" */
    return me->textDisplay.lineStarts[line];

  if (clickTimes == 4)		/* quad-clicks - select by "paragraph" */
    {
      char *tmp;
      int blankline, x;

      /*
       *  scan backward to the beginning of the "paragraph"
       */
      for (x = line;
	   x > 1;
	   x--)
	{
	  blankline = True;
	  for (tmp = me->textDisplay.lineStarts[x];
	       blankline && tmp < (me->textDisplay.lineStarts[x+1] - 1);
	       tmp++)
	    if (*tmp != ' ' && *tmp != '\t')
	      blankline = False;

	  if (blankline)
	    return me->textDisplay.lineStarts[(x == line) ? x : x+1];
	}
      return me->textDisplay.lineStarts[0];
    }

				/* quint-clicks - select all text */
  return me->textDisplay.lineStarts[0];
}


char *find_end(me, ptr, line)
     TextDisplayJet me;
     char **ptr;
     int line;
{
  int clickTimes = me->textDisplay.clickTimes % 5;

  if (clickTimes == 1)		/* single-clicks - select by char */
    return *ptr;

  if (clickTimes == 2)		/* double-clicks - select by "word" */
    {
      char *tmp;

      for (tmp = *ptr;
	   tmp <= me->textDisplay.lineStarts[me->textDisplay.numLines] &&
	   charClass[*tmp] == charClass[**ptr];
	   tmp++);
      return tmp;
    }

  if (clickTimes == 3)		/* triple-clicks - select by "line" */
    return me->textDisplay.lineStarts[line + 1];

  if (clickTimes == 4)		/* quad-clicks - select by "paragraph" */
    {
      char *tmp;
      int blankline, x;

      /*
       *  scan forward to the end of the "paragraph"
       */
      for (x = line;
	   x < me->textDisplay.numLines;
	   x++)
	{
	  blankline = True;
	  for (tmp = me->textDisplay.lineStarts[x];
	       blankline && tmp < (me->textDisplay.lineStarts[x+1] - 1);
	       tmp++)
	    if (*tmp != ' ' && *tmp != '\t')
	      blankline = False;

	  if (blankline)
	    return me->textDisplay.lineStarts[(x == line) ? x+1 : x];
	}
      return me->textDisplay.lineStarts[x];
    }

				/* quint-clicks - select all text */
  return me->textDisplay.lineStarts[me->textDisplay.numLines];
}


static void expose(me, event)
     TextDisplayJet me;
     XEvent *event;
{
  drawText(me,
	   me->textDisplay.topLine,
	   me->textDisplay.visLines,
	   me->core.y + me->textDisplay.internalBorder);
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
  if (me->textDisplay.realized)
    if (start >= me->textDisplay.topLine &&
	start <= (me->textDisplay.topLine + me->textDisplay.visLines - 1))
      drawText(me,
	       start,
	       me->textDisplay.topLine + me->textDisplay.visLines - start,
	       me->core.y + me->textDisplay.internalBorder +
	       (start - me->textDisplay.topLine) *
	       (me->textDisplay.font->ascent + me->textDisplay.font->descent));
}

void AddText(me)
     TextDisplayJet me;
{
  int l;

  l = me->textDisplay.numLines;
  if (l > 0)  l--;
  appendLines(me, me->textDisplay.lineStarts[l], l);
  /* (l == 0) ? 0 : l - 1],
     (l == 0) ? 0 : l - 1); */

  showNewLines(me, l /* (l == 0) ? 0 : l - 1 */ );
}

void MoveText(me, text, offset)
     TextDisplayJet me;
     char *text;
     int offset;
{
  me->textDisplay.startSelect =
    MIN(text, me->textDisplay.startSelect - offset);
  me->textDisplay.endSelect =
    MIN(text, me->textDisplay.endSelect - offset);
  me->textDisplay.realStart =
    MIN(text, me->textDisplay.realStart - offset);
  me->textDisplay.realEnd =
    MIN(text, me->textDisplay.realEnd - offset);

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

  charHeight = me->textDisplay.font->ascent + me->textDisplay.font->descent;

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
	  bcopy(me->textDisplay.realStart,
		me->textDisplay.selection, length);
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

      me->textDisplay.buttonDown = True;
      me->textDisplay.whichButton = event->xbutton.button;
      w = where(me, event->xbutton.x, event->xbutton.y, &line);

      switch(me->textDisplay.whichButton)
	{
	case Button1:
	  me->textDisplay.startSelect = me->textDisplay.startPivot =
	    find_start(me, &w, line);
	  me->textDisplay.endSelect = me->textDisplay.endPivot =
	    find_end(me, &w, line);
	  break;

	case Button3:
	  if ((int)w <
	      ((int)me->textDisplay.realStart +
	       (int)me->textDisplay.realEnd)
	      / 2)
	    {
	      me->textDisplay.startEnd = 0;
	      me->textDisplay.startSelect = find_start(me, &w, line);
	      me->textDisplay.endSelect = me->textDisplay.realEnd;
	    }
	  else
	    {
	      me->textDisplay.startEnd = 1;
	      me->textDisplay.endSelect = find_end(me, &w, line);
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
	    me->textDisplay.endSelect = find_end(me, &w, line);
	  else if (w < me->textDisplay.startPivot)
	    me->textDisplay.startSelect = find_start(me, &w, line);
	  break;

	case Button3:
	  if (w > me->textDisplay.realEnd)
	    {
	      me->textDisplay.startEnd = 1;
	      me->textDisplay.endSelect = find_end(me, &w, line);
	      me->textDisplay.startSelect = me->textDisplay.realStart;
	    }
	  else if (w < me->textDisplay.realStart)
	    {
	      me->textDisplay.startEnd = 0;
	      me->textDisplay.startSelect = find_start(me, &w, line);
	      me->textDisplay.endSelect = me->textDisplay.realEnd;
	    }
	  else if (me->textDisplay.startEnd == 0)
	    me->textDisplay.startSelect = find_start(me, &w, line);
	  else
	    me->textDisplay.endSelect = find_end(me, &w, line);
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
