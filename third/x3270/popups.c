/*
 * Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	popups.c
 *		This module handles pop-up dialogs: errors, host names,
 *		font names, information.
 */

#include "globals.h"
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Text.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "objects.h"

#include "actionsc.h"
#include "macrosc.h"
#include "mainc.h"
#include "popupsc.h"
#include "screenc.h"
#include "utilc.h"

static char vmsgbuf[4096];


/*
 * General popup support
 */

/* Find the parent of a window */
static Window
parent_of(w)
Window w;
{
	Window root, parent, *children;
	unsigned int nchildren;

	XQueryTree(display, w, &root, &parent, &children, &nchildren);
	XFree((char *) children);
	return parent;
}

/*
 * Find the base window (the one with the wmgr decorations) and the virtual
 * root, so we can pop up a window relative to them.
 */
void
toplevel_geometry(x, y, width, height)
Dimension *x, *y, *width, *height;
{
	Window tlw = XtWindow(toplevel);
	Window win;
	Window parent;
	int nw;
	struct {
		Window w;
		XWindowAttributes wa;
	} ancestor[10];
	XWindowAttributes wa, *base_wa, *root_wa;

	/*
	 * Trace the family tree of toplevel.
	 */
	for (win = tlw, nw = 0; ; win = parent) {
		parent = parent_of(win);
		ancestor[nw].w = parent;
		XGetWindowAttributes(display, parent, &ancestor[nw].wa);
		++nw;
		if (parent == root_window)
			break;
	}

	/*
	 * Figure out if they're running a virtual desktop, by seeing if
	 * the 1st child of root is bigger than it is.  If so, pretend that
	 * the virtual desktop is the root.
	 */
	if (nw > 1 &&
	    (ancestor[nw-2].wa.width > ancestor[nw-1].wa.width ||
	     ancestor[nw-2].wa.height > ancestor[nw-1].wa.height))
		--nw;
	root_wa = &ancestor[nw-1].wa;

	/*
	 * Now identify the base window as the window below the root
	 * window.
	 */
	if (nw >= 2) {
		base_wa = &ancestor[nw-2].wa;
	} else {
		XGetWindowAttributes(display, tlw, &wa);
		base_wa = &wa;
	}

	*x = base_wa->x + root_wa->x;
	*y = base_wa->y + root_wa->y;
	*width = base_wa->width + 2*base_wa->border_width;
	*height = base_wa->height + 2*base_wa->border_width;
}

/* Pop up a popup shell */
void
popup_popup(shell, grab)
Widget shell;
XtGrabKind grab;
{
	XtPopup(shell, grab);
	XSetWMProtocols(display, XtWindow(shell), &a_delete_me, 1);
}

static enum placement CenterD = Center;
enum placement *CenterP = &CenterD;
static enum placement BottomD = Bottom;
enum placement *BottomP = &BottomD;
static enum placement LeftD = Left;
enum placement *LeftP = &LeftD;
static enum placement RightD = Right;
enum placement *RightP = &RightD;

/* Place a popped-up shell */
/*ARGSUSED*/
void
place_popup(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Dimension width, height;
	Dimension x = 0, y = 0;
	Dimension win_width, win_height;
	Dimension popup_width, popup_height;
	enum placement p = *(enum placement *)client_data;
	extern Dimension main_width;

	/* Get and fix the popup's dimensions */
	XtRealizeWidget(w);
	XtVaGetValues(w,
	    XtNwidth, &width,
	    XtNheight, &height,
	    NULL);
	XtVaSetValues(w,
	    XtNheight, height,
	    XtNwidth, width,
	    XtNbaseHeight, height,
	    XtNbaseWidth, width,
	    XtNminHeight, height,
	    XtNminWidth, width,
	    XtNmaxHeight, height,
	    XtNmaxWidth, width,
	    NULL);

	/*
	 * Find the geometry of the parent of the toplevel window in order to
	 * place the popup window.  This is done with an Xlib call (asking the
	 * server explicitly) rather than the Xt call.  Why?  At startup,
	 * Xt may not yet know the parent window has been moved by the window
	 * manager.
	 */
	toplevel_geometry(&x, &y, &win_width, &win_height);

	switch (p) {
	    case Center:
		XtVaGetValues(w,
		    XtNwidth, &popup_width,
		    XtNheight, &popup_height,
		    NULL);
		XtVaSetValues(w,
		    XtNx, x + (win_width-popup_width) / (unsigned) 2,
		    XtNy, y + (win_height-popup_height) / (unsigned) 2,
		    NULL);
		break;
	    case Bottom:
		XtVaSetValues(w,
		    XtNx, x,
		    XtNy, y + win_height + 2,
		    NULL);
		break;
	    case Left:
		XtVaGetValues(w,
		    XtNwidth, &popup_width,
		    NULL);
		XtVaSetValues(w,
		    XtNx, x - popup_width - (win_width - main_width) - 2,
		    XtNy, y,
		    NULL);
		break;
	    case Right:
		XtVaSetValues(w,
		    XtNx, x + win_width + 2,
		    XtNy, y,
		    NULL);
		break;
	}
}

/* Action called when "Return" is pressed in data entry popup */
void
confirm_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	Widget w2;

	/* Find the Confirm or Okay button */
	w2 = XtNameToWidget(XtParent(w), ObjConfirmButton);
	if (w2 == NULL)
		w2 = XtNameToWidget(w, ObjConfirmButton);
	if (w2 == NULL) {
		xs_warning("confirm: cannot find %s", ObjConfirmButton);
		return;
	}

	/* Call its "notify" event */
	XtCallActionProc(w2, "set", event, params, *num_params);
	XtCallActionProc(w2, "notify", event, params, *num_params);
	XtCallActionProc(w2, "unset", event, params, *num_params);
}

/* Callback for "Cancel" button in data entry popup */
/*ARGSUSED*/
static void
cancel_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown((Widget) client_data);
}

/*
 * Callback for text source changes.  Ensures that the dialog text does not
 * contain white space -- especially newlines.
 */
/*ARGSUSED*/
static void
popup_dialog_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	static Boolean called_back = False;
	XawTextBlock b, nullb;	/* firstPos, length, ptr, format */
	XawTextPosition pos = 0;
	int front_len = 0;
	int end_len = 0;
	int end_pos = 0;
	int i;
	enum { FRONT, MIDDLE, END } place = FRONT;

	if (called_back)
		return;
	else
		called_back = True;

	nullb.firstPos = 0;
	nullb.length = 0;
	nullb.ptr = NULL;

	/*
	 * Scan the text for whitespace.  Leading whitespace is deleted;
	 * embedded whitespace causes the rest of the text to be deleted.
	 */
	while (1) {
		XawTextSourceRead(w, pos, &b, 1024);
		if (b.length <= 0)
			break;
		nullb.format = b.format;
		if (place == END) {
			end_len += b.length;
			continue;
		}
		for (i = 0; i < b.length; i++)
			if (isspace(*(b.ptr + i))) {
				if (place == FRONT) {
					front_len++;
					continue;
				} else {
					end_pos = b.firstPos + i;
					end_len = b.length - i;
					place = END;
					break;
				}
			} else
				place = MIDDLE;
		pos += b.length;
		if (b.length < 1024)
			break;
	}
	if (front_len)
		XawTextSourceReplace(w, 0, front_len, &nullb);
	if (end_len)
		XawTextSourceReplace(w, end_pos - front_len,
		    end_pos - front_len + end_len, &nullb);
	called_back = False;
}

/* Create a simple data entry popup */
Widget
create_form_popup(name, callback, callback2, no_spaces)
char *name;
XtCallbackProc callback;
XtCallbackProc callback2;
Boolean no_spaces;
{
	char *widgetname;
	Widget shell;
	Widget dialog;
	Widget w;

	/* Create the popup shell */

	widgetname = xs_buffer("%sPopup", name);
	if (isupper(widgetname[0]))
		widgetname[0] = tolower(widgetname[0]);
	shell = XtVaCreatePopupShell(
	    widgetname, transientShellWidgetClass, toplevel,
	    NULL);
	XtFree(widgetname);
	XtAddCallback(shell, XtNpopupCallback, place_popup, (XtPointer) CenterP);

	/* Create a dialog in the popup */

	dialog = XtVaCreateManagedWidget(
	    ObjDialog, dialogWidgetClass, shell,
	    XtNvalue, "",
	    NULL);
	XtVaSetValues(XtNameToWidget(dialog, XtNlabel),
	    NULL);

	/* Add "Confirm" and "Cancel" buttons to the dialog */

	w = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, dialog,
	    NULL);
	XtAddCallback(w, XtNcallback, callback, (XtPointer)dialog);
	if (callback2) {
		w = XtVaCreateManagedWidget(
			ObjConfirm2Button, commandWidgetClass, dialog,
			NULL);
		XtAddCallback(w, XtNcallback, callback2, (XtPointer)dialog);
	}
	w = XtVaCreateManagedWidget(
	    ObjCancelButton, commandWidgetClass, dialog,
	    NULL);
	XtAddCallback(w, XtNcallback, cancel_button_callback, (XtPointer) shell);

	if (!no_spaces)
		return shell;

	/* Modify the translations for the objects in the dialog */

	w = XtNameToWidget(dialog, XtNvalue);
	if (w == NULL)
		xs_warning("Cannot find \"%s\" in dialog", XtNvalue);

	/* Set a callback for text modifications */
	w = XawTextGetSource(w);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, popup_dialog_callback, NULL);

	return shell;
}


/*
 * Error popup
 */

static Widget error_shell = NULL;
static Widget error_form;
Boolean error_popup_visible;

/* Called when OK is pressed on the error popup */
/*ARGSUSED*/
static void
saw_error(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(error_shell);
}

/* Called when the error popup is closed */
/*ARGSUSED*/
static void
error_popdown(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	error_popup_visible = False;
	if (exiting)
		x3270_exit(1);
}

void
error_popup_init()
{
	Widget w;

	if (error_shell != NULL)
		return;

	error_shell = XtVaCreatePopupShell(
	    "errorPopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(error_shell, XtNpopupCallback, place_popup,
	    (XtPointer) CenterP);
	XtAddCallback(error_shell, XtNpopdownCallback, error_popdown, PN);

	/* Create a dialog in the popup */

	error_form = XtVaCreateManagedWidget(
	    "dialog", dialogWidgetClass, error_shell,
	    NULL);
	XtVaSetValues(XtNameToWidget(error_form, XtNlabel),
	    XtNlabel, "first line\nsecond line",
	    NULL);

	/* Add "OK" button to the dialog */

	w = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, error_form,
	    NULL);
	XtAddCallback(w, XtNcallback, saw_error, 0);

	/* Force it into existence so it sizes itself with 2-line text */

	XtRealizeWidget(error_shell);
}

/* Pop up an error dialog. */
void
#if defined(__STDC__)
popup_an_error(char *fmt, ...)
#else
popup_an_error(va_alist)
va_dcl
#endif
{
	va_list args;

#if defined(__STDC__)
	va_start(args, fmt);
#else
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	(void) vsprintf(vmsgbuf, fmt, args);
	if (!error_shell) {
		(void) fprintf(stderr, "%s: %s\n", programname, vmsgbuf);
		exit(1);
	}

	if (sms_redirect()) {
		sms_error(vmsgbuf);
		return;
	}

	XtVaSetValues(error_form, XtNlabel, vmsgbuf, NULL);
	if (!error_popup_visible) {
		ring_bell();
		error_popup_visible = True;
		popup_popup(error_shell, XtGrabExclusive);
	}
}

/* Pop up an error dialog, based on an error number. */
void
#if defined(__STDC__)
popup_an_errno(int errn, char *fmt, ...)
#else
popup_an_errno(va_alist)
va_dcl
#endif
{
	va_list args;
	char *s;
	extern int sys_nerr;
#if !defined(__FreeBSD__)
	extern char *sys_errlist[];
#endif

#if defined(__STDC__)
	va_start(args, fmt);
#else
	int errn;
	char *fmt;
	va_start(args);
	errn = va_arg(args, int);
	fmt = va_arg(args, char *);
#endif
	(void) vsprintf(vmsgbuf, fmt, args);
	s = XtNewString(vmsgbuf);

	if (errn > 0) {
		if (errn < sys_nerr)
			popup_an_error("%s:\n%s", s, sys_errlist[errn]);
		else
			popup_an_error("%s:\nError %d", s, errn);
	} else
		popup_an_error(s);
	XtFree(s);
}


/*
 * Info popup
 */

static Widget info_shell = NULL;
static Widget info_form;
Boolean info_popup_visible;

/* Called when OK is pressed on the info popup */
/*ARGSUSED*/
static void
saw_info(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(info_shell);
}

/* Called when the info popup is closed */
/*ARGSUSED*/
static void
info_popdown(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	info_popup_visible = False;
}

void
info_popup_init()
{
	Widget w;

	if (info_shell != NULL)
		return;

	info_shell = XtVaCreatePopupShell(
	    "infoPopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(info_shell, XtNpopupCallback, place_popup,
	    (XtPointer) CenterP);
	XtAddCallback(info_shell, XtNpopdownCallback, info_popdown, PN);

	/* Create a dialog in the popup */

	info_form = XtVaCreateManagedWidget(
	    "dialog", dialogWidgetClass, info_shell,
	    NULL);
	XtVaSetValues(XtNameToWidget(info_form, XtNlabel),
	    XtNlabel, "first line\nsecond line",
	    NULL);

	/* Add "OK" button to the dialog */

	w = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, info_form,
	    NULL);
	XtAddCallback(w, XtNcallback, saw_info, 0);

	/* Force it into existence so it sizes itself with 2-line text */

	XtRealizeWidget(info_shell);
}

/* Pop up an info dialog. */
void
#if defined(__STDC__)
popup_an_info(char *fmt, ...)
#else
popup_an_info(va_alist)
va_dcl
#endif
{
	va_list args;

#if defined(__STDC__)
	va_start(args, fmt);
#else
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	(void) vsprintf(vmsgbuf, fmt, args);
	if (!info_shell) {
		(void) printf("%s: %s\n", programname, vmsgbuf);
		return;
	}

	XtVaSetValues(info_form, XtNlabel, vmsgbuf, NULL);
	if (!info_popup_visible) {
		info_popup_visible = True;
		popup_popup(info_shell, XtGrabNonexclusive);
	}
}


/*
 * Script actions
 */
/*ARGSUSED*/
void
Info_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_debug(Info_action, event, params, num_params);
	if (check_usage(Info_action, *num_params, 1, 1) < 0)
		return;
	popup_an_info(params[0]);
}
