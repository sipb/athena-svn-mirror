/*------------------------------------------------------------
message-window.c: message popups for xdvi.

written by Stefan Ulrich <stefanulrich@users.sourceforge.net> 2000/02/11

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

TODO: make a Motif version (as soon as xdvik will compile with Motif,
that is).

------------------------------------------------------------*/



/*

============================================
Suggested Policy for using the GUI messages:
============================================

- Use the statusline for shorter messages, a message window for more
  important messages or such where you'd like to give further help info
  (see the `helptext' argument of do_popup_message()). When in doubt,
  prefer the statusline (excessive use of popup windows is a nuisance
  for the user).
  
- Don't use any of the GUI messages to report internal workings of
  the program; for important internal information, there should be a
  debugging setting to print it to stderr. Use the GUI messages
  only in situations such as the following:

  - to give the user feedback on actions that (s)he initiated
  
  - to indicate that an internal action causes a delay perceptible
    by the user (as a rough guide: a delay of more than half a second)
    
  - to report situations that might require new actions by the user.

*/

#define DEBUG 0

#include "xdvi-config.h"
#include "xdvi.h"
#include "my-vsnprintf.h"

#ifdef TOOLKIT	/* all the windows stuff is conditional */
/* Xaw specific stuff */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Dialog.h>
#endif /* TOOLKIT */

#include "kpathsea/c-vararg.h"

/* steps in which space for message buffer is allocated */
#define CHUNK_LEN 128

#ifdef TOOLKIT
/* have no more than MAX_POPUPS open simultaneously */
#define MAX_POPUPS 10

/* offset for cascading popups */
#define POPUP_OFFSET ((my_popup_num * 20))

/* array of active popups: */
static int g_popup_array[MAX_POPUPS];

/* arrays for saving the widgets of multiple popups;
 * same index as in g_popup_array:
 */
static Widget popup_message[MAX_POPUPS], message_box[MAX_POPUPS],
    message_text[MAX_POPUPS], message_paned[MAX_POPUPS],
    message_ok[MAX_POPUPS], message_help[MAX_POPUPS];



/*------------------------------------------------------------
 *  quit_action
 *
 *  Arguments:
 *	Widget w, XtPointer call_data
 *		- (ignored)
 *	XtPointer client_data
 *		- first free index from g_popup_array, assigned at creation time of the popup,
 *		  for popping down the correct widget
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Callback for the `OK' button
 *------------------------------------------------------------*/

/* ARGSUSED */
static void
quit_action(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cnt = (XtArgVal) client_data;
    UNUSED(w);
    UNUSED(call_data);
#if DEBUG    
    fprintf(stderr, "quit_action called for popup %d\n", cnt);
#endif
    assert(cnt >= 0 && cnt < MAX_POPUPS);
    
    /* close down window cnt */
    XtPopdown(popup_message[cnt]);
    XtDestroyWidget(popup_message[cnt]);
    /* mark this position as free */
    g_popup_array[cnt] = 0;
}


/*------------------------------------------------------------
 *  help_action
 *
 *  Arguments:
 *	Widget w, XtPointer call_data
 *		- (ignored)
 *	XtPointer client_data
 *		- the help string
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Callback for the `Help' button; opens another window
 *	containing the help text. The new window won't have
 *	another `Help' button.
 *------------------------------------------------------------*/

/* ARGSUSED */
static void
help_action(Widget w, XtPointer client_data, XtPointer call_data)
{
    UNUSED(w);
    UNUSED(call_data);
    
    /* open another window with the help text */
    do_popup_message(MSG_HELP, NULL, client_data);
}

#endif /* TOOLKIT */


/*------------------------------------------------------------
 *  create_dialogs
 *
 *  Arguments:
 *	Widget toplevel - parent for the dialog window
 *
 *	helptext	- if not-NULL, create an additional
 *			  `help' button
 *
 *	cnt		- number of current popup dialog
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Create message dialog widgets
 *------------------------------------------------------------*/

#ifdef TOOLKIT
static void
create_dialogs(Widget toplevel, char *helptext, int cnt)
{
    Widget new_message_paned, new_popup_message,
	new_message_text, new_message_box, new_message_ok, new_message_help;

    new_popup_message = XtVaCreatePopupShell("xdvi", transientShellWidgetClass, toplevel,
					     XtNx, 60,
					     XtNy, 80,
					     XtNvisual, our_visual,
					     XtNcolormap, our_colormap,
					     XtNaccelerators, accelerators,
					     NULL);
    new_message_paned = XtVaCreateManagedWidget("message_paned", panedWidgetClass, new_popup_message,
					     XtNaccelerators, accelerators,
					      NULL);
    new_message_text = XtVaCreateManagedWidget("message_text", asciiTextWidgetClass, new_message_paned,
					       XtNheight, 100,
					       XtNwidth, 400,
					       /* wrap horizontally instead of scrolling 
						* TODO: this won't work for the first widget instance?
						*/
					       XtNwrap, XawtextWrapWord,
					       XtNscrollVertical, XAW_SCROLL_ALWAYS,
					       XtNeditType, XawtextRead,
					       XtNinput, True,
					       XtNdisplayCaret, False,
					       XtNleftMargin, 5,
					       XtNaccelerators, accelerators,
					       NULL);
    /* box for the OK/Cancel buttons */
    new_message_box = XtVaCreateManagedWidget("message_box", formWidgetClass, new_message_paned,
					      /* resizing by user isn't needed */
					      XtNshowGrip, False,
					      /* resizing the window shouldn't influence this box,
					       * but  only the text widget
					       */
					      XtNskipAdjust, True,
					      XtNaccelerators, accelerators,
					      NULL);

    /* insert the new widgets into the global arrays */
    popup_message[cnt] = new_popup_message;
    message_box[cnt] = new_message_box;
    message_paned[cnt] = new_message_paned;
    message_text[cnt] = new_message_text;

    /* OK button */
    new_message_ok = XtVaCreateManagedWidget("OK", commandWidgetClass, new_message_box,
					     XtNaccelerators, accelerators,
					     XtNtop, XtChainTop,
					     XtNbottom, XtChainBottom,
					     XtNleft, XtChainLeft,
					     XtNright, XtChainLeft,
					     NULL);
    /* add quit_action callback for the "OK" button */
    /* FIXME: how to make accelerators be accepted by new_popup_message as well? */
    XtInstallAllAccelerators(new_message_box, new_message_ok);
    XtAddCallback(new_message_ok, XtNcallback, quit_action, (XtPointer) cnt);
    message_ok[cnt] = new_message_ok;

    /* if helptext argument is not-NULL, create additional
     * "Help"-Button and add help_action callback:
     */
    if (helptext != NULL) {
	new_message_help = XtVaCreateManagedWidget("Help", commandWidgetClass, new_message_box,
						   XtNtop, XtChainTop,
						   XtNfromHoriz, message_ok[cnt],
						   XtNbottom, XtChainBottom,
						   XtNleft, XtChainRight,
						   XtNright, XtChainRight,
						   XtNjustify, XtJustifyRight,
						   XtNaccelerators, accelerators,
						   NULL);
	XtAddCallback(new_message_help, XtNcallback, help_action, helptext);
	message_help[cnt] = new_message_help;
    }
}
#endif /* TOOLKIT */


/*------------------------------------------------------------
 *  do_popup_message
 *
 *  Arguments:
 *	type - one of MSG_INFO, MSG_WARN or MSG_ERR,
 *	       depending on which the string for the
 *	       window title will be chosen
 *
 *	char *helptext
 *	      - if not-null, this will add a `Help'
 *		button to the message widget that pops
 *		up another message widget containing
 *		<helptext>.
 *		
 *
 *	char *msg, ...
 *	      - format string followed by a variable
 *		number of arguments to be formatted.
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Pop up a message window containing <msg>.
 *	If there are already n popups open, will open
 *	a new one unless n >= MAX_POPUPS.
 *	Without TOOLKIT, or when main window hasn't been mapped,
 *	just print <msg> to stderr.
 *------------------------------------------------------------*/

void
do_popup_message(int type, char *helptext, char *msg, ...)
{
    char msg_type[64];
    char *msg_buf = NULL;
    int len = CHUNK_LEN, len2;	/* lengths for buffer allocation */
    int my_popup_num = 0;

    va_list argp;
    va_start(argp, msg);
    
    switch (type) {
    case MSG_HELP:
	sprintf(msg_type, "Xdvi Help");
	break;
    case MSG_INFO:
	sprintf(msg_type, "Xdvi Info");
	break;
    case MSG_WARN:
	sprintf(msg_type, "Xdvi Warning");
	break;
    case MSG_ERR:
	sprintf(msg_type, "Xdvi Error");
	/* SU: the bell is starting to annoy me (should we have a resource
	   to control it???
	*/
	/*  	XBell(DISP, 20); */
	break;
    }

    for (;;) {
	/*
	 * see man page for vsnprintf: for glibc 2.0, <len2> is
	 * -1 if <msg> had been trucated to <len>; for glibc 2.1,
	 * it's the total length of <msg>; so the following works
	 * with both versions:
	 */
	msg_buf = xrealloc(msg_buf, len);
	len2 = VSNPRINTF(msg_buf, len, msg, argp);
	if (len2 > -1 && len2 < len) {	/* <len> was large enough */
	    break;
	}
	if (len2 > -1) {	/* now len2 + 1 is the exact space needed */
	    len = len2 + 1;
	}
	else {	/* try again with larger <len> */
	    len += CHUNK_LEN;
	}
    }
    va_end(argp);

#if DEBUG
    fprintf(stderr, "do_popup_message called with prompt: \"%s\"\n", msg_buf);
#endif

#ifndef TOOLKIT
    fprintf(stderr, "\n%s:\n%s\n%s\n\n", msg_type, msg_buf, helptext ? helptext : "");
#else
    if (!XtIsRealized(top_level)) {
	/* additionally dump it to STDERR, since in some cases such as non-found
	   T1 resources, it might cause mktexpk to be run before the main window
	   comes up. */
	fprintf(stderr, "\n%s:\n%s\n%s\n\n", msg_type, msg_buf, helptext ? helptext : "");
    }
    /* search for first free position in g_popup_array */
    while (my_popup_num < MAX_POPUPS && (g_popup_array[my_popup_num] == 1)) {
	my_popup_num++;
    }
    if (my_popup_num == MAX_POPUPS) {
	/* already enough popups on screen, just dump it to stderr */
	fprintf(stderr, "%s:\n%s\n", msg_type, msg_buf);
	/* Note: If a mad function continues to open popups, this will
	 * stop after MAX_POPUPS, but open a new window for each
	 * window the user pops down. Maybe we ought to do something
	 * about this.
	 */
	return;
    }
    else {
	/* mark it as non-free */
	g_popup_array[my_popup_num] = 1;
    }
#if DEBUG
    fprintf(stderr, "first free position in g_popup_array: %d\n", my_popup_num);
#endif
    
    /* create a new set of widgets for the additional popup. */
    create_dialogs(top_level, helptext, my_popup_num);

    if (my_popup_num > 0) {
	/* some window managers position new windows exactly above the
	   existing one; to prevent this, move it with some offset
	   from the previous one: */
	Dimension x = 0, y = 0;
	XtVaGetValues(popup_message[my_popup_num-1], XtNx, &x, XtNy, &y, NULL);
	XtVaSetValues(popup_message[my_popup_num], XtNx, x, XtNy, y, NULL);
    }
    XtVaSetValues(popup_message[my_popup_num], XtNtitle, msg_type, NULL);
    XtVaSetValues(message_text[my_popup_num], XtNstring, msg_buf, NULL);
    XtPopup(popup_message[my_popup_num], XtGrabNone);
#endif /* not TOOLKIT */
    free(msg_buf);
}
