/*------------------------------------------------------------
statusline for the xdvi(k) previewer

written by Stefan Ulrich <stefanulrich@users.sourceforge.net> 2000/02/25

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
------------------------------------------------------------*/


#include "xdvi-config.h"
#include "version.h"
#include "statusline.h"

#include "kpathsea/c-vararg.h"
#include "my-vsnprintf.h"

#ifdef STATUSLINE
#ifdef	OLD_X11_TOOLKIT
#include <X11/Atoms.h>
#else /* not OLD_X11_TOOLKIT */
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#endif /* not OLD_X11_TOOLKIT */

#if XAW3D
#include <X11/Xaw3d/Viewport.h>
#include <X11/Xaw3d/Label.h>
#else
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Label.h>
#endif
#endif /* STATUSLINE */

int global_xtra_h = 20;
Widget statusline;
static XtIntervalId statusline_timer;
static Boolean initialized = False;

/*
 * only print MAX_LEN characters to the statusline
 * (it's only 1 line after all)
 */
#define MAX_LEN 512

/* for saving the statusline string if the statusline is
 * destroyed and recreated
 */
static char g_string_savebuf[MAX_LEN + 2];

#ifdef STATUSLINE



#if 0
//UNUSED /*------------------------------------------------------------
//UNUSED  *  get_statusline_defaultheight
//UNUSED  * 
//UNUSED  *  Arguments:
//UNUSED  *     void
//UNUSED  *
//UNUSED  *  Returns:
//UNUSED  *     Position <ret> - height of the statusline
//UNUSED  *              
//UNUSED  *  Purpose:
//UNUSED  *     Tries to extract the height needed for the statusline
//UNUSED  *     from the current font height.
//UNUSED  *     
//UNUSED  *------------------------------------------------------------*/
//UNUSED 
//UNUSED Position
//UNUSED get_statusline_defaultheight()
//UNUSED {
//UNUSED   Position ret;
//UNUSED   /* FIXME: always returns NULL? */
//UNUSED   XFontStruct   *font = XLoadQueryFont(DISP, XtDefaultFont);
//UNUSED   
//UNUSED   if (font != NULL) {
//UNUSED     ret =  ((font)->max_bounds.ascent + (font)->max_bounds.descent - 2);
//UNUSED     printf("=====FONT != NULL: %d\n", ret);
//UNUSED   }
//UNUSED   else {
//UNUSED     ret = 17;
//UNUSED     printf("=====FONT == NULL: %d\n", ret);
//UNUSED   }
//UNUSED   return(ret);
//UNUSED }
#endif

/*------------------------------------------------------------
 *  create_statusline
 * 
 *  Arguments:
 *	void
 *
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Create the statusline widget. To be called at the beginning
 *	of the program, and when expert mode is switched off.
 *
 *  Side effects:
 *	sets <global_xtra_h> to the height of the statusline in pixels.
 *	
 *------------------------------------------------------------*/

void
create_statusline(void)
{
    Position vport_h, clip_x, clip_w;
    static Position my_h = 0;

    /*
     * FIXME: is there a better way to set the y position depending on
     * the height of the widget?
     * It doesn't work to change the value of XtNy *after* creating
     * the widget!
     */

    if (!initialized) {
	/*
	 * determine height of statusline (depending on the font used).
	 * This is not changeable at runtime, so it's determined once and
	 * for all at program start.
	 */
	statusline = XtVaCreateWidget("statusline", labelWidgetClass, vport_widget,
				      XtNlabel, (XtArgVal) "test",
				      NULL);
	XtVaGetValues(statusline, XtNheight, &my_h, NULL);
	XtDestroyWidget(statusline);
	initialized = True;
	/* initialize g_string_savebuf */
#ifdef T1LIB
	sprintf(g_string_savebuf, "This is xdvik %s   (T1Lib rendering %s)",
		TVERSION, resource.t1lib ? "on" : "switched off");
#else
	sprintf(g_string_savebuf, "This is xdvik %s", TVERSION);
#endif	
    }
    /* determine position and width of statusline */
    XtVaGetValues(clip_widget, XtNx, &clip_x, XtNwidth, &clip_w, NULL);
    XtVaGetValues(vport_widget, XtNheight, &vport_h, NULL);
    if (vport_h - my_h <= 0) {
#ifdef DEBUG
	fprintf(stderr, "=======not printing statusline\n");
#endif /* DEBUG */
	return;
    }
    statusline = XtVaCreateManagedWidget("statusline", labelWidgetClass, vport_widget,
					 XtNlabel, (XtArgVal) g_string_savebuf,
					 XtNwidth, clip_w,
					 XtNx, clip_x - 1,	/* so that left border becomes invisible */
					 XtNy, vport_h - my_h,
					 XtNborderWidth, 1,
					 XtNjustify, XtJustifyLeft,
					 XtNborder, (XtArgVal) resource._fore_Pixel,	/* same as for the buttons line */
					 NULL);
}


/*------------------------------------------------------------
 *  handle_statusline_resize
 * 
 *  Arguments:
 *	void
 *
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Resize the statusline when the total window size changes.
 *	
 *------------------------------------------------------------*/

void
handle_statusline_resize(void)
{
    if (!resource.statusline) {
	return;
    }
    /* apparently the x,y values of a widget can only be set at creation time, so
     * the following won't work:
     */
#if 0
    // BROKEN  Position vport_h, clip_x, clip_w;
    // BROKEN  static Position my_h = 0;
    // BROKEN  
    // BROKEN  XtVaGetValues(clip_widget,
    // BROKEN                XtNx, &clip_x,
    // BROKEN                XtNwidth, &clip_w,
    // BROKEN                NULL);
    // BROKEN  XtVaGetValues(vport_widget,
    // BROKEN                XtNheight, &vport_h,
    // BROKEN                NULL);
    // BROKEN 
    // BROKEN  XtUnmanageChild(statusline);
    // BROKEN  XtVaSetValues(statusline,
    // BROKEN                //             XtNlabel, (XtArgVal) "",
    // BROKEN                XtNwidth, clip_w,
    // BROKEN                XtNx, clip_x - 1,
    // BROKEN                XtNy, vport_h - my_h,
    // BROKEN                XtNborderWidth, 1,
    // BROKEN                XtNjustify, XtJustifyLeft,
    // BROKEN                XtNborder, (XtArgVal) resource._fore_Pixel,
    // BROKEN                NULL);
    // BROKEN  XtManageChild(statusline);
    // BROKEN  XFlush(DISP);
#endif
    /* only this will: */
    XtDestroyWidget(statusline);
    create_statusline();
}




/*------------------------------------------------------------
 *  clear_statusline
 * 
 *  Arguments:
 *	XtPointer data - (ignored)
 *
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	clear statusline by printing an empty message to it.
 *	
 *------------------------------------------------------------*/


/* ARGSUSED */
static void
clear_statusline(XtPointer client_data, XtIntervalId *id)
{
    UNUSED(client_data);
    UNUSED(id);
    
    if (resource.statusline) {
	XtVaSetValues(statusline, XtNlabel, "", NULL);
	XFlush(DISP);
    }
}
#endif /* STATUSLINE */

/* force a statusline update, no matter how busy the application is.
   Use this with care (only for important messages).
*/
void
force_statusline_update(void)
{
#ifdef STATUSLINE
#ifdef MOTIF
    XmUpdateDisplay(top_level);
#else
    XEvent event;
    XSync(DISP, 0);
    while (XCheckMaskEvent(DISP, ExposureMask, &event))
        XtDispatchEvent(&event);
#endif
#endif    
}


/*------------------------------------------------------------
 *  print_statusline
 * 
 *  Arguments:
 *	timeout - if > 0, timeout in seconds after which the message will
 *		  be deleted again. If < 0, message will remain (until
 *		  another message overprints it)
 *	fmt     - message, a C format string
 *
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	If compiled with #define STATUSLINE and expert mode is off,
 *	print <fmt> to the statusline; else, print <fmt> to stdout
 *	(unless `hushstdout' option is specified).
 *	
 *------------------------------------------------------------*/

void
print_statusline(STATUS_TIMER timeout, _Xconst char *fmt, ...)
{
    va_list argp;
    char buf[MAX_LEN + 1];
    
    va_start(argp, fmt);

#ifdef STATUSLINE
    if (!XtIsRealized(top_level) || !initialized || !resource.statusline) {
	if (!resource._hush_stdout && strlen(fmt) > 0) {
	    fprintf(stdout, "xdvi: ");
	    (void)vfprintf(stdout, fmt, argp);
	    fprintf(stdout, "\n");
	    fflush(stdout);
	}
    }
    else {
	VSNPRINTF(buf, MAX_LEN, fmt, argp);	/* just trow away strings longer than MAX_LEN */
	/*
	 * save current contents of statusline so that toggling the statusline
	 * on and off will display the same text again
	 */
	Strcpy(g_string_savebuf, buf);
	XtVaSetValues(statusline, XtNlabel, buf, NULL);
	if (timeout > 0) {
	    timeout *= 1000;	/* convert to miliseconds */
	    statusline_timer = XtAddTimeOut(timeout,
					    clear_statusline,
					    (XtPointer) NULL);
	}
    }
#else /* STATUSLINE */
    if (strlen(fmt) > 0 && !resource._hush_stdout) {
	(void)vfprintf(stdout, fmt, argp);
	(void)fprintf(stdout, "\n");
    }
#endif /* STATUSLINE */
    va_end(argp);
}
