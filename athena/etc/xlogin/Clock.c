/* $XConsortium: Clock.c,v 1.66 91/10/16 21:30:24 eswu Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/XawInit.h>
#include "ClockP.h"
#include <X11/Xosdefs.h>

#ifdef X_NOT_STDC_ENV
extern struct tm *localtime();
#endif

/* Initialization of defaults */

#define offset(field) XtOffsetOf(ClockRec, clock.field)
#define goffset(field) XtOffsetOf(WidgetRec, core.field)

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRImmediate, (XtPointer) 0},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRImmediate, (XtPointer) 0},
    {XtNupdate, XtCInterval, XtRInt, sizeof(int), 
        offset(update), XtRImmediate, (XtPointer) 60 },
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(fgpixel), XtRString, XtDefaultForeground},
    {XtNpadding, XtCMargin, XtRInt, sizeof(int),
        offset(padding), XtRImmediate, (XtPointer) 8},
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
        offset(font), XtRString, XtDefaultFont},
    {XtNbackingStore, XtCBackingStore, XtRBackingStore, sizeof (int),
    	offset (backing_store), XtRString, "default"},
};

#undef offset
#undef goffset

static void ClassInitialize(void);
static void Initialize(Widget request, Widget new, ArgList args,
		       Cardinal *num_args);
static void Realize(Widget gw, XtValueMask *valueMask,
		    XSetWindowAttributes *attrs);
static void Destroy(Widget gw);
static void Redisplay(Widget gw, XEvent *event, Region region);
static void clock_tic(XtPointer client_data, XtIntervalId *id);
static Boolean SetValues(Widget gcurrent, Widget grequest, Widget gnew,
			 ArgList args, Cardinal *num_args);

ClockClassRec clockClassRec = {
    { /* core fields */
    /* superclass		*/	(WidgetClass) &simpleClassRec,
    /* class_name		*/	"Clock",
    /* widget_size		*/	sizeof(ClockRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* resource_count		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry           */	XtInheritQueryGeometry,
    /* display_accelerator      */	XtInheritDisplayAccelerator,
    /* extension                */	NULL
    },
    { /* simple fields */
    /* change_sensitive         */      XtInheritChangeSensitive
    },
    { /* clock fields */
    /* ignore                   */      0
    }
};

WidgetClass clockWidgetClass = (WidgetClass) &clockClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void ClassInitialize(void)
{
    XawInitializeWidgetSet();
    XtAddConverter( XtRString, XtRBackingStore, XmuCvtStringToBackingStore,
		    NULL, 0 );
}

/* ARGSUSED */
static void Initialize(Widget request, Widget new, ArgList args,
		       Cardinal *num_args)
{
    ClockWidget w = (ClockWidget)new;
    XtGCMask		valuemask;
    XGCValues	myXGCV;
    int min_height, min_width;
    char *str;
    struct tm tm;
    long time_value;

    valuemask = GCForeground | GCBackground | GCFont | GCLineWidth;
    if (w->clock.font != NULL)
        myXGCV.font = w->clock.font->fid;
    else
        valuemask &= ~GCFont;	/* use server default font */

    (void) time(&time_value);
    tm = *localtime(&time_value);
    str = asctime(&tm);
    if (w->clock.font == NULL)
        w->clock.font = XQueryFont( XtDisplay(w),
				    XGContextFromGC(
						    DefaultGCOfScreen(XtScreen(w))) );
    min_width = XTextWidth(w->clock.font, str, strlen(str)) +
      2 * w->clock.padding;
    min_height = w->clock.font->ascent +
      w->clock.font->descent + 2 * w->clock.padding;

    if (w->core.width == 0)
	w->core.width = min_width;
    if (w->core.height == 0)
	w->core.height = min_height;

    myXGCV.foreground = w->clock.fgpixel;
    myXGCV.background = w->core.background_pixel;
    if (w->clock.font != NULL)
        myXGCV.font = w->clock.font->fid;
    else
        valuemask &= ~GCFont;	/* use server default font */
    myXGCV.line_width = 0;
    w->clock.myGC = XtGetGC((Widget)w, valuemask, &myXGCV);

    valuemask = GCForeground | GCLineWidth ;
    myXGCV.foreground = w->core.background_pixel;
    w->clock.EraseGC = XtGetGC((Widget)w, valuemask, &myXGCV);

    if (w->clock.update <= 0)
	w->clock.update = 60;	/* make invalid update's use a default */
    w->clock.interval_id = 0;
}

static void Realize(Widget gw, XtValueMask *valueMask,
		    XSetWindowAttributes *attrs)
{
     ClockWidget	w = (ClockWidget) gw;

     switch (w->clock.backing_store) {
     case Always:
     case NotUseful:
     case WhenMapped:
     	*valueMask |=CWBackingStore;
	attrs->backing_store = w->clock.backing_store;
	break;
     }
     (*clockWidgetClass->core_class.superclass->core_class.realize)
	 (gw, valueMask, attrs);
}

static void Destroy(Widget gw)
{
     ClockWidget w = (ClockWidget) gw;
     if (w->clock.interval_id) XtRemoveTimeOut (w->clock.interval_id);
     XtReleaseGC (gw, w->clock.myGC);
     XtReleaseGC (gw, w->clock.EraseGC);
}

/* ARGSUSED */
static void Redisplay(Widget gw, XEvent *event, Region region)
{
    ClockWidget w = (ClockWidget) gw;

    w->clock.prev_time_string[0] = '\0';
    clock_tic((XtPointer)w, (XtIntervalId)0);
}

/* ARGSUSED */
static void clock_tic(XtPointer client_data, XtIntervalId *id)
{
        ClockWidget w = (ClockWidget)client_data;    
	struct tm tm; 
	long	time_value;
	char	*time_ptr;
        register Display *dpy = XtDisplay(w);
        register Window win = XtWindow(w);
	int clear_from;
	int i, len, prev_len;

	if (id || !w->clock.interval_id)
	    w->clock.interval_id =
		XtAppAddTimeOut( XtWidgetToApplicationContext( (Widget) w),
				w->clock.update*1000, clock_tic, (XtPointer)w );
	(void) time(&time_value);
	tm = *localtime(&time_value);

	time_ptr = asctime(&tm);
	len = strlen (time_ptr);
	if (time_ptr[len - 1] == '\n') time_ptr[--len] = '\0';
	prev_len = strlen (w->clock.prev_time_string);
	for (i = 0; ((i < len) && (i < prev_len) && 
		     (w->clock.prev_time_string[i] == time_ptr[i])); i++);
	strcpy (w->clock.prev_time_string+i, time_ptr+i);

	XDrawImageString (dpy, win, w->clock.myGC,
			  (2+w->clock.padding +
			   XTextWidth (w->clock.font, time_ptr, i)),
			  2+w->clock.font->ascent+w->clock.padding,
			  time_ptr + i, len - i);
	/*
	 * Clear any left over bits
	 */
	clear_from = (XTextWidth (w->clock.font, time_ptr, len)
		      + 2 + w->clock.padding);
	if (clear_from < (int)w->core.width)
		XFillRectangle (dpy, win, w->clock.EraseGC,
				clear_from, 0, w->core.width - clear_from,
				w->core.height);
}
	
/* ARGSUSED */
static Boolean SetValues(Widget gcurrent, Widget grequest, Widget gnew,
			 ArgList args, Cardinal *num_args)
{
      ClockWidget current = (ClockWidget) gcurrent;
      ClockWidget new = (ClockWidget) gnew;
      Boolean redisplay = FALSE;
      XtGCMask valuemask;
      XGCValues	myXGCV;

      /* first check for changes to clock-specific resources.  We'll accept all
         the changes, but may need to do some computations first. */

      if (new->clock.update != current->clock.update) {
	  if (current->clock.interval_id)
	      XtRemoveTimeOut (current->clock.interval_id);
	  if (XtIsRealized( (Widget) new))
	      new->clock.interval_id = XtAppAddTimeOut( 
                                         XtWidgetToApplicationContext(gnew),
					 new->clock.update*1000,
				         clock_tic, (XtPointer)gnew);
      }

      if (new->clock.padding != current->clock.padding)
	   redisplay = TRUE;

      if (new->clock.font != current->clock.font)
	   redisplay = TRUE;

      if ((new->clock.fgpixel != current->clock.fgpixel)
          || (new->core.background_pixel != current->core.background_pixel)) {
          valuemask = GCForeground | GCBackground | GCFont | GCLineWidth;
	  myXGCV.foreground = new->clock.fgpixel;
	  myXGCV.background = new->core.background_pixel;
          myXGCV.font = new->clock.font->fid;
	  myXGCV.line_width = 0;
	  XtReleaseGC (gcurrent, current->clock.myGC);
	  new->clock.myGC = XtGetGC(gcurrent, valuemask, &myXGCV);
	  redisplay = TRUE;
          }

      if (new->core.background_pixel != current->core.background_pixel) {
          valuemask = GCForeground | GCLineWidth;
	  myXGCV.foreground = new->core.background_pixel;
	  myXGCV.line_width = 0;
	  XtReleaseGC (gcurrent, current->clock.EraseGC);
	  new->clock.EraseGC = XtGetGC((Widget)gcurrent, valuemask, &myXGCV);
	  redisplay = TRUE;
	  }
     
     return (redisplay);

}
