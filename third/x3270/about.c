/*
 * Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	about.c
 *		Pop-up window with the current state of x3270.
 */

#include "globals.h"
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include "appres.h"
#include "objects.h"

#include "aboutc.h"
#include "popupsc.h"
#include "telnetc.h"
#include "utilc.h"

static Widget about_shell = NULL;
static Widget about_form;
extern time_t ns_time;
extern int ns_brcvd;
extern int ns_rrcvd;
extern int ns_bsent;
extern int ns_rsent;
extern int linemode;
extern char *build;
extern Pixmap icon;
extern struct trans_list *temp_keymaps;

/* Called when OK is pressed on the about popup */
/*ARGSUSED*/
static void
saw_about(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(about_shell);
}

/* Called when the about popup is popped down */
/*ARGSUSED*/
static void
destroy_about(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtDestroyWidget(about_shell);
	about_shell = NULL;
}

/* Return a time difference in English */
static char *
hms(ts)
time_t ts;
{
	time_t t, td;
	long hr, mn, sc;
	static char buf[128];

	(void) time(&t);

	td = t - ts;
	hr = td / 3600;
	mn = (td % 3600) / 60;
	sc = td % 60;

	if (hr > 0)
		(void) sprintf(buf, "%ld %s %ld %s %ld %s",
		    hr, (hr == 1) ?
			get_message("hour") : get_message("hours"),
		    mn, (mn == 1) ?
			get_message("minute") : get_message("minutes"),
		    sc, (sc == 1) ?
			get_message("second") : get_message("seconds"));
	else if (mn > 0)
		(void) sprintf(buf, "%ld %s %ld %s",
		    mn, (mn == 1) ?
			get_message("minute") : get_message("minutes"),
		    sc, (sc == 1) ?
			get_message("second") : get_message("seconds"));
	else
		(void) sprintf(buf, "%ld %s",
		    sc, (sc == 1) ?
			get_message("second") : get_message("seconds"));

	return buf;
}

#define MAKE_SMALL(label, n) { \
	w_prev = w; \
	w = XtVaCreateManagedWidget( \
	    ObjSmallLabel, labelWidgetClass, about_form, \
	    XtNborderWidth, 0, \
	    XtNlabel, label, \
	    XtNfromVert, w, \
	    XtNleft, XtChainLeft, \
	    XtNvertDistance, (n), \
	    NULL); \
	vd = n; \
	}

#define MAKE_LABEL(label, n) { \
	w_prev = w; \
	w = XtVaCreateManagedWidget( \
	    ObjNameLabel, labelWidgetClass, about_form, \
	    XtNborderWidth, 0, \
	    XtNlabel, label, \
	    XtNfromVert, w, \
	    XtNfromHoriz, left_anchor, \
	    XtNleft, XtChainLeft, \
	    XtNvertDistance, (n), \
	    NULL); \
	vd = n; \
	}

#define MAKE_VALUE(label) { \
	v = XtVaCreateManagedWidget( \
	    ObjDataLabel, labelWidgetClass, about_form, \
	    XtNborderWidth, 0, \
	    XtNlabel, label, \
	    XtNfromVert, w_prev, \
	    XtNfromHoriz, w, \
	    XtNhorizDistance, 0, \
	    XtNvertDistance, vd, \
	    XtNleft, XtChainLeft, \
	    NULL); \
	}

#define MAKE_LABEL2(label) { \
	w = XtVaCreateManagedWidget( \
	    ObjNameLabel, labelWidgetClass, about_form, \
	    XtNborderWidth, 0, \
	    XtNlabel, label, \
	    XtNfromVert, w_prev, \
	    XtNfromHoriz, v, \
	    XtNhorizDistance, 0, \
	    XtNvertDistance, vd, \
	    XtNleft, XtChainLeft, \
	    NULL); \
	}

/* Called when the "About x3270..." button is pressed */
void
popup_about()
{
	Widget w = NULL, w_prev = NULL;
	Widget v = NULL;
	Widget left_anchor = NULL;
	int vd = 4;
	char fbuf[1024];
	char *ftype;
	char *xbuf;

	/* Create the popup */

	about_shell = XtVaCreatePopupShell(
	    "aboutPopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(about_shell, XtNpopupCallback, place_popup,
	    (XtPointer) CenterP);
	XtAddCallback(about_shell, XtNpopdownCallback, destroy_about,
	    NULL);

	/* Create a form in the popup */

	about_form = XtVaCreateManagedWidget(
	    "dialog", formWidgetClass, about_shell,
	    NULL);

	/* Pretty picture */

	left_anchor = XtVaCreateManagedWidget(
	    "icon", labelWidgetClass, about_form,
	    XtNborderWidth, 0,
	    XtNbitmap, icon,
	    XtNfromVert, w,
	    XtNleft, XtChainLeft,
	    NULL);

	/* Miscellany */

	MAKE_LABEL(build, 4);
	MAKE_LABEL(get_message("processId"), 4);
	(void) sprintf(fbuf, "%d", getpid());
	MAKE_VALUE(fbuf);
	MAKE_LABEL2(get_message("windowId"));
	(void) sprintf(fbuf, "0x%lx", XtWindow(toplevel));
	MAKE_VALUE(fbuf);

	/* Everything else at the left margin under the bitmap */
	w = left_anchor;
	left_anchor = NULL;

	MAKE_SMALL("Modifications Copyright \251 1993, 1994, 1995, 1996, 1997 by Paul Mattes.\n\
Original X11 Port Copyright \251 1990 by Jeff Sparkes.\n\
File transfer code Copyright \251 1995 by Dick Altenbern.\n\
 Permission to use, copy, modify, and distribute this software and its documentation\n\
 for any purpose and without fee is hereby granted, provided that the above copyright\n\
 notice appear in all copies and that both that copyright notice and this permission\n\
 notice appear in supporting documentation.\n\
\n\
Copyright \251 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.\n\
 All Rights Reserved.  GTRC hereby grants public use of this software.  Derivative\n\
 works based on this software must incorporate this copyright notice.", 4);

	(void) sprintf(fbuf, "%s %s: %d %s x %d %s, %s, %s",
	    get_message("model"), model_name,
	    maxCOLS, get_message("columns"),
	    maxROWS, get_message("rows"),
	    appres.mono ? get_message("mono") :
		(appres.m3279 ? get_message("fullColor") :
		    get_message("pseudoColor")),
	    (appres.extended && !std_ds_host) ? get_message("extendedDs") :
		get_message("standardDs"));
	MAKE_LABEL(fbuf, 4);

	MAKE_LABEL(get_message("terminalName"), 4);
	MAKE_VALUE(termtype);

	MAKE_LABEL(get_message("emulatorFont"), 4);
	MAKE_VALUE(efontname);
	if (*standard_font) {
		if (*latin1_font)
			ftype = get_message("fullFont");
		else
			ftype = get_message("asciiFont");
	} else
		ftype = get_message("cgFont");
	xbuf = xs_buffer("  %s", ftype);
	MAKE_LABEL(xbuf, 0);
	XtFree(xbuf);

	if (appres.charset) {
		MAKE_LABEL(get_message("characterSet"), 4);
		MAKE_VALUE(appres.charset);
	} else
		MAKE_LABEL(get_message("defaultCharacterSet"), 4);

	if (trans_list != (struct trans_list *)NULL ||
	    temp_keymaps != (struct trans_list *)NULL) {
		struct trans_list *t;

		fbuf[0] = '\0';
		for (t = trans_list; t; t = t->next) {
			if (fbuf[0])
				(void) strcat(fbuf, ",");
			(void) strcat(fbuf, t->name);
		}
		for (t = temp_keymaps; t; t = t->next) {
			if (fbuf[0])
				(void) strcat(fbuf, " ");
			(void) strcat(fbuf, "+");
			(void) strcat(fbuf, t->name);
		}
		MAKE_LABEL(get_message("keyboardMap"), 4)
		MAKE_VALUE(fbuf);
	} else
		MAKE_LABEL(get_message("defaultKeyboardMap"), 4);
	if (appres.compose_map) {
		MAKE_LABEL(get_message("composeMap"), 4);
		MAKE_VALUE(appres.compose_map);
	} else {
		MAKE_LABEL(get_message("noComposeMap"), 4);
	}

	if (appres.active_icon) {
		MAKE_LABEL(get_message("activeIcon"), 4);
		xbuf = xs_buffer("  %s", get_message("iconFont"));
		MAKE_LABEL(xbuf, 0);
		XtFree(xbuf);
		MAKE_VALUE(appres.icon_font);
		if (appres.label_icon) {
			xbuf = xs_buffer("  %s", get_message("iconLabelFont"));
			MAKE_LABEL(xbuf, 0);
			XtFree(xbuf);
			MAKE_VALUE(appres.icon_label_font);
		}
	} else
		MAKE_LABEL(get_message("staticIcon"), 4);

	if (CONNECTED) {
		MAKE_LABEL(get_message("connectedTo"), 4);
		MAKE_VALUE(current_host);
		(void) sprintf(fbuf, "  %s", get_message("port"));
		MAKE_LABEL2(fbuf);
		(void) sprintf(fbuf, "%d", current_port);
		MAKE_VALUE(fbuf);
		if (IN_ANSI) {
			if (linemode)
				ftype = get_message("lineMode");
			else
				ftype = get_message("charMode");
		} else
			ftype = get_message("dsMode");
		(void) sprintf(fbuf, "  %s, ", ftype);
		(void) strcat(fbuf, hms(ns_time));

		MAKE_LABEL(fbuf, 0);

		if (IN_3270)
			(void) sprintf(fbuf, "%s %d %s, %d %s\n%s %d %s, %d %s",
			    get_message("sent"),
			    ns_bsent, (ns_bsent == 1) ?
				get_message("byte") : get_message("bytes"),
			    ns_rsent, (ns_rsent == 1) ?
				get_message("record") : get_message("records"),
			    get_message("Received"),
			    ns_brcvd, (ns_brcvd == 1) ?
				get_message("byte") : get_message("bytes"),
			    ns_rrcvd, (ns_rrcvd == 1) ?
				get_message("record") : get_message("records"));
		else
			(void) sprintf(fbuf, "%s %d %s, %s %d %s",
			    get_message("sent"),
			    ns_bsent, (ns_bsent == 1) ?
				get_message("byte") : get_message("bytes"),
			    get_message("received"),
			    ns_brcvd, (ns_brcvd == 1) ?
				get_message("byte") : get_message("bytes"));
		MAKE_LABEL(fbuf, 4);

		if (IN_ANSI) {
			struct ctl_char *c = net_linemode_chars();
			int i;

			MAKE_LABEL(get_message("specialCharacters"), 4);
			for (i = 0; c[i].name; i++) {
				if (!i || !(i % 4)) {
					(void) sprintf(fbuf, "  %s", c[i].name);
					MAKE_LABEL(fbuf, 0);
				} else {
					MAKE_LABEL2(c[i].name);
				}
				MAKE_VALUE(c[i].value);
			}
		}
	} else if (HALF_CONNECTED) {
		MAKE_LABEL(get_message("connectionPending"), 4);
		MAKE_VALUE(current_host);
		MAKE_LABEL(hms(ns_time), 0);
	} else
		MAKE_LABEL(get_message("notConnected"), 4);

	/* Add "OK" button at the lower left */

	w = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, about_form,
	    XtNfromVert, w,
	    XtNleft, XtChainLeft,
	    XtNbottom, XtChainBottom,
	    NULL);
	XtAddCallback(w, XtNcallback, saw_about, 0);

	/* Pop it up */

	popup_popup(about_shell, XtGrabExclusive);
}

#undef MAKE_LABEL
#undef MAKE_VALUE
