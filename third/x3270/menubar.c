/*
 * Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	menubar.c
 *		This module handles the menu bar.
 */

#include "globals.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Dialog.h>
#include "CmplxMenu.h"
#include "CmeBSB.h"
#include "CmeLine.h"
#include "appres.h"
#include "resources.h"

#include "actionsc.h"
#include "aboutc.h"
#include "ftc.h"
#include "keypadc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "mainc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "printc.h"
#include "savec.h"
#include "screenc.h"
#include "telnetc.h"
#include "utilc.h"

#define MACROS_MENU	"macrosMenu"

extern Widget		keypad_shell;
extern int		linemode;
extern Boolean		keypad_popped;

static struct host {
	char *name;
	char *hostname;
	enum { PRIMARY, ALIAS } entry_type;
	char *loginstring;
	struct host *next;
} *hosts, *last_host;

static struct scheme {
	char *label;
	char *scheme;
	struct scheme *next;
} *schemes, *last_scheme;
static int scheme_count;
static Widget  *scheme_widgets;

static struct charset {
	char *label;
	char *charset;
	struct charset *next;
} *charsets, *last_charset;
static int charset_count;
static Widget  *charset_widgets;

static void	scheme_init();
static void	charsets_init();
static void	options_menu_init();
static void	keypad_button_init();
static void	connect_menu_init();
static void	macros_menu_init();
static void	file_menu_init();
static void	Bye();

#include "dot.bm"
#include "arrow.bm"
#include "diamond.bm"
#include "no_diamond.bm"
#include "ky.bm"
#include "null.bm"

Boolean keypad_changed = False;


/*
 * Menu Bar
 */

static Widget   menu_bar;
static Boolean  menubar_buttons;
static Widget   disconnect_button;
static Widget   exit_button;
static Widget   exit_menu;
static Widget   macros_button;
static Widget   ft_button;
static Widget   connect_button;
static Widget   keypad_button;
static Widget   linemode_button;
static Widget   charmode_button;
static Widget   models_option;
static Widget   model_2_button;
static Widget   model_3_button;
static Widget   model_4_button;
static Widget   model_5_button;
static Widget   oversize_button;
static Widget   keypad_option_button;
static Widget	script_abort_button;
static Widget   connect_menu;
static Widget   macros_menu;

static Pixmap   arrow;
Pixmap dot;
Pixmap diamond;
Pixmap no_diamond;
Pixmap null;

static int	n_bye;

static void     toggle_init();

#define TOP_MARGIN	3
#define BOTTOM_MARGIN	3
#define LEFT_MARGIN	3
#define KEY_HEIGHT	18
#define KEY_WIDTH	70
#define BORDER		1
#define SPACING		3

#define MO_OFFSET	1
#define CN_OFFSET	2
#define MENU_BORDER	2

#define	MENU_MIN_WIDTH	(LEFT_MARGIN + 3*(KEY_WIDTH+2*BORDER+SPACING) + \
			 LEFT_MARGIN + (ky_width+8) + 2*BORDER + SPACING + \
			 2*MENU_BORDER)

/*
 * Compute the potential height of the menu bar.
 */
Dimension
menubar_qheight(container_width)
Dimension container_width;
{
	if (!appres.menubar || container_width < (unsigned) MENU_MIN_WIDTH)
		return 0;
	else
		return TOP_MARGIN + KEY_HEIGHT+2*BORDER + BOTTOM_MARGIN +
			2*MENU_BORDER;
}

/*
 * Initialize the menu bar.
 */
void
menubar_init(container, overall_width, current_width)
Widget container;
Dimension overall_width;
Dimension current_width;
{
	static Boolean ever = False;
	Dimension height;

	if (!ever) {
		scheme_init();
		charsets_init();
		ever = True;
	}

	/* Error popup */

	error_popup_init();
	info_popup_init();

	height = menubar_qheight(current_width);
	if (!height) {
		menu_bar = container;
		menubar_buttons = False;
	} else {
		height -= 2*MENU_BORDER;
		menu_bar = XtVaCreateManagedWidget(
		    "menuBarContainer", compositeWidgetClass, container,
		    XtNborderWidth, MENU_BORDER,
		    XtNwidth, overall_width - 2*MENU_BORDER,
		    XtNheight, height,
		    NULL);
		menubar_buttons = True;
	}


	/* Bitmaps */

	dot = XCreateBitmapFromData(display, root_window,
	    (char *) dot_bits, dot_width, dot_height);
	arrow = XCreateBitmapFromData(display, root_window,
	    (char *) arrow_bits, arrow_width, arrow_height);
	diamond = XCreateBitmapFromData(display, root_window,
	    (char *) diamond_bits, diamond_width, diamond_height);
	no_diamond = XCreateBitmapFromData(display, root_window,
	    (char *) no_diamond_bits, no_diamond_width, no_diamond_height);
	null = XCreateBitmapFromData(display, root_window,
	    (char *) null_bits, null_width, null_height);

	/* "File..." menu */

	file_menu_init(menu_bar,
	    LEFT_MARGIN,
	    TOP_MARGIN);

	/* "Options..." menu */

	options_menu_init(menu_bar,
	    LEFT_MARGIN + MO_OFFSET*(KEY_WIDTH+2*BORDER+SPACING),
	    TOP_MARGIN);

	/* "Connect..." menu */

	connect_menu_init(menu_bar,
	    LEFT_MARGIN + CN_OFFSET*(KEY_WIDTH+2*BORDER+SPACING),
	    TOP_MARGIN);

	/* "Macros..." menu */

	macros_menu_init(menu_bar,
	    LEFT_MARGIN + CN_OFFSET*(KEY_WIDTH+2*BORDER+SPACING),
	    TOP_MARGIN);

	/* Keypad button */

	if (menubar_buttons)
		keypad_button_init(menu_bar,
		    (Position) (current_width - LEFT_MARGIN - (ky_width+8) - 2*BORDER - 2*MENU_BORDER),
		    TOP_MARGIN);

	/* Register a grab action for the popup versions of the menus */

	XtRegisterGrabAction(HandleMenu_action, True,
	    (ButtonPressMask|ButtonReleaseMask),
	    GrabModeAsync, GrabModeAsync);
}

/*
 * External entry points
 */
void
menubar_connect()	/* have connected to or disconnected from a host */
{
	if (disconnect_button)
		XtVaSetValues(disconnect_button,
		    XtNsensitive, PCONNECTED,
		    NULL);
	if (exit_button) {
		if (PCONNECTED) {
			/* Remove the immediate callback. */
			if (n_bye) {
				XtRemoveCallback(exit_button, XtNcallback,
				    Bye, 0);
				n_bye--;
			}

			/* Set pullright for extra confirmation. */
			XtVaSetValues(exit_button,
			    XtNrightBitmap, arrow,
			    XtNmenuName, "exitMenu",
			    NULL);
		} else {
			/* Install the immediate callback. */
			if (!n_bye) {
				XtAddCallback(exit_button, XtNcallback,
				    Bye, 0);
				n_bye++;
			}

			/* Remove the pullright. */
			XtVaSetValues(exit_button,
			    XtNrightBitmap, NULL,
			    XtNmenuName, NULL,
			    NULL);
		}
	}
	if (connect_menu) {
		if (PCONNECTED && connect_button)
			XtUnmapWidget(connect_button);
		else {
			connect_menu_init(menu_bar,
			    LEFT_MARGIN + CN_OFFSET*(KEY_WIDTH+2*BORDER+SPACING),
			    TOP_MARGIN);
			if (menubar_buttons &&
			    (!appres.reconnect || reconnect_disabled))
				XtMapWidget(connect_button);
		}
	}
	if (!PCONNECTED && macros_button)
		XtUnmapWidget(macros_button);
	else {
		macros_menu_init(menu_bar,
		    LEFT_MARGIN + CN_OFFSET*(KEY_WIDTH+2*BORDER+SPACING),
		    TOP_MARGIN);
		if (menubar_buttons && macros_button)
			XtMapWidget(macros_button);
	}
	if (ft_button)
		XtVaSetValues(ft_button, XtNsensitive, IN_3270, NULL);
	if (linemode_button)
		XtVaSetValues(linemode_button, XtNsensitive, IN_ANSI, NULL);
	if (charmode_button)
		XtVaSetValues(charmode_button, XtNsensitive, IN_ANSI, NULL);
	if (models_option)
		XtVaSetValues(models_option, XtNsensitive, !PCONNECTED, NULL);
	if (model_2_button)
		XtVaSetValues(model_2_button, XtNsensitive, !PCONNECTED, NULL);
	if (model_3_button)
		XtVaSetValues(model_3_button, XtNsensitive, !PCONNECTED, NULL);
	if (model_4_button)
		XtVaSetValues(model_4_button, XtNsensitive, !PCONNECTED, NULL);
	if (model_5_button)
		XtVaSetValues(model_5_button, XtNsensitive, !PCONNECTED, NULL);
	if (oversize_button)
		XtVaSetValues(oversize_button, XtNsensitive, !PCONNECTED, NULL);
}

void
menubar_keypad_changed()	/* keypad snapped on or off */
{
	if (keypad_option_button)
		XtVaSetValues(keypad_option_button,
		    XtNleftBitmap,
			appres.keypad_on || keypad_popped ? dot : None,
		    NULL);
}

void
menubar_newmode()	/* have switched telnet modes */
{
	if (linemode_button)
		XtVaSetValues(linemode_button,
		    XtNsensitive, IN_ANSI,
		    XtNleftBitmap, linemode ? diamond : no_diamond,
		    NULL);
	if (charmode_button)
		XtVaSetValues(charmode_button,
		    XtNsensitive, IN_ANSI,
		    XtNleftBitmap, linemode ? no_diamond : diamond,
		    NULL);
	if (appres.toggle[LINE_WRAP].w[0])
		XtVaSetValues(appres.toggle[LINE_WRAP].w[0], XtNsensitive,
		    IN_ANSI, NULL);
	if (appres.toggle[RECTANGLE_SELECT].w[0])
		XtVaSetValues(appres.toggle[RECTANGLE_SELECT].w[0],
		    XtNsensitive, IN_ANSI, NULL);
	if (ft_button)
		XtVaSetValues(ft_button, XtNsensitive, IN_3270, NULL);
}

void
menubar_gone()		/* container has gone away */
{
	menu_bar = (Widget) NULL;
	connect_menu = (Widget) NULL;
	connect_button = (Widget) NULL;
	disconnect_button = (Widget) NULL;
	script_abort_button = (Widget) NULL;
	macros_menu = (Widget) NULL;
	macros_button = (Widget) NULL;
}

/* Called to change the sensitivity of the "Abort Script" button. */
void
menubar_as_set(sensitive)
Boolean sensitive;
{
	if (script_abort_button != (Widget)NULL)
		XtVaSetValues(script_abort_button,
		    XtNsensitive, sensitive,
		    NULL);
}


/*
 * "File..." menu
 */
static Widget save_shell = (Widget) NULL;

/* Called from "Exit x3270" button on "File..." menu */
/*ARGSUSED*/
static void
Bye(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	x3270_exit(0);
}

/* Called from the "Disconnect" button on the "File..." menu */
/*ARGSUSED*/
static void
disconnect(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	x_disconnect(True);
	menubar_connect();
}

/* Called from the "Abort Script" button on the "File..." menu */
/*ARGSUSED*/
static void
script_abort_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	abort_script();
}

/* "About x3270" popup */
/*ARGSUSED*/
static void
show_about(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	popup_about();
}

/* Called from the "Save" button on the save options dialog */
/*ARGSUSED*/
static void
save_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *s;

	s = XawDialogGetValueString((Widget)client_data);
	if (!s || !*s)
		return;
	if (!save_options(s))
		XtPopdown(save_shell);
}

/* Called from the "Save Options in File" button on the "File..." menu */
/*ARGSUSED*/
static void
do_save_options(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	extern char *profile_name;

	if (save_shell == NULL)
		save_shell = create_form_popup("SaveOptions",
		    save_button_callback, (XtCallbackProc)NULL, False);
	XtVaSetValues(XtNameToWidget(save_shell, "dialog"),
	    XtNvalue, profile_name,
	    NULL);
	popup_popup(save_shell, XtGrabExclusive);
}

static void
file_menu_init(container, x, y)
Widget container;
Dimension x, y;
{
	Widget file_menu;
	Widget w;

	file_menu = XtVaCreatePopupShell(
	    "fileMenu", complexMenuWidgetClass, container,
	    menubar_buttons ? XtNlabel : NULL, NULL,
	    NULL);
	if (!menubar_buttons)
		(void) XtCreateManagedWidget("space", cmeLineObjectClass,
		    file_menu, NULL, 0);

	/* About x3270... */
	w = XtVaCreateManagedWidget(
	    "aboutOption", cmeBSBObjectClass, file_menu,
	    NULL);
	XtAddCallback(w, XtNcallback, show_about, NULL);

	/* File Transfer */
	if (!appres.secure) {
		(void) XtCreateManagedWidget(
		    "space", cmeLineObjectClass, file_menu,
		    NULL, 0);
		ft_button = XtVaCreateManagedWidget(
		    "ftOption", cmeBSBObjectClass, file_menu,
		    XtNsensitive, IN_3270,
		    NULL);
		XtAddCallback(ft_button, XtNcallback, popup_ft, NULL);
	}

	/* Trace Data Stream
	   Trace X Events
	   Save Screen(s) in File */
	(void) XtCreateManagedWidget(
	    "space", cmeLineObjectClass, file_menu,
	    NULL, 0);
	if (appres.debug_tracing) {
		toggle_init(file_menu, DS_TRACE, "dsTraceOption", CN);
		toggle_init(file_menu, EVENT_TRACE, "eventTraceOption", CN);
	}
	toggle_init(file_menu, SCREEN_TRACE, "screenTraceOption", CN);

	/* Print Screen Text */
	(void) XtCreateManagedWidget(
	    "space", cmeLineObjectClass, file_menu,
	    NULL, 0);
	w = XtVaCreateManagedWidget(
	    "printTextOption", cmeBSBObjectClass, file_menu,
	    NULL);
	XtAddCallback(w, XtNcallback, print_text_option, NULL);

	/* Print Window Bitmap */
	w = XtVaCreateManagedWidget(
	    "printWindowOption", cmeBSBObjectClass, file_menu,
	    NULL);
	XtAddCallback(w, XtNcallback, print_window_option, NULL);

	if (!appres.secure) {

		/* Save Options */
		(void) XtCreateManagedWidget(
		    "space", cmeLineObjectClass, file_menu,
		    NULL, 0);
		w = XtVaCreateManagedWidget(
		    "saveOption", cmeBSBObjectClass, file_menu,
		    NULL);
		XtAddCallback(w, XtNcallback, do_save_options, NULL);

		/* Execute an action */
		(void) XtCreateManagedWidget(
		    "space", cmeLineObjectClass, file_menu,
		    NULL, 0);
		w = XtVaCreateManagedWidget(
		    "executeActionOption", cmeBSBObjectClass, file_menu,
		    NULL);
		XtAddCallback(w, XtNcallback, execute_action_option, NULL);
	}

	/* Abort script */
	(void) XtCreateManagedWidget(
	    "space", cmeLineObjectClass, file_menu,
	    NULL, 0);
	script_abort_button = XtVaCreateManagedWidget(
	    "abortScriptOption", cmeBSBObjectClass, file_menu,
	    XtNsensitive, sms_active(),
	    NULL);
	XtAddCallback(script_abort_button, XtNcallback, script_abort_callback,
	    0);

	/* Disconnect */
	(void) XtCreateManagedWidget(
	    "space", cmeLineObjectClass, file_menu,
	    NULL, 0);
	disconnect_button = XtVaCreateManagedWidget(
	    "disconnectOption", cmeBSBObjectClass, file_menu,
	    XtNsensitive, PCONNECTED,
	    NULL);
	XtAddCallback(disconnect_button, XtNcallback, disconnect, 0);

	/* Exit x3270 */
	if (exit_menu != (Widget)NULL)
		XtDestroyWidget(exit_menu);
	exit_menu = XtVaCreatePopupShell(
	    "exitMenu", complexMenuWidgetClass, container,
	    NULL);
	w = XtVaCreateManagedWidget(
	    "exitReallyOption", cmeBSBObjectClass, exit_menu,
	    NULL);
	XtAddCallback(w, XtNcallback, Bye, 0);
	exit_button = XtVaCreateManagedWidget(
	    "exitOption", cmeBSBObjectClass, file_menu,
	    NULL);
	XtAddCallback(exit_button, XtNcallback, Bye, 0);
	n_bye = 1;

	/* File... */
	if (menubar_buttons) {
		w = XtVaCreateManagedWidget(
		    "fileMenuButton", menuButtonWidgetClass, container,
		    XtNx, x,
		    XtNy, y,
		    XtNwidth, KEY_WIDTH,
		    XtNheight, KEY_HEIGHT,
		    XtNmenuName, "fileMenu",
		    NULL);
	}
}


/*
 * "Connect..." menu
 */

static Widget connect_shell = NULL;

/* Called from each button on the "Connect..." menu */
/*ARGSUSED*/
static void
host_connect(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	(void) x_connect(client_data);
}

/* Called from the lone "Connect" button on the connect dialog */
/*ARGSUSED*/
static void
connect_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *s;

	s = XawDialogGetValueString((Widget)client_data);
	if (!s || !*s)
		return;
	if (!x_connect(s))
		XtPopdown(connect_shell);
}

/* Called from the "Other..." button on the "Connect..." menu */
/*ARGSUSED*/
static void
do_connect_popup(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (connect_shell == NULL)
		connect_shell = create_form_popup("Connect",
		    connect_button_callback, (XtCallbackProc)NULL, False);
	popup_popup(connect_shell, XtGrabExclusive);
}

/* Called from the "Reconnect" button */
/*ARGSUSED*/
static void
do_reconnect(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	x_reconnect();
}

/*
 * Initialize the "Connect..." menu
 */
static void
connect_menu_init(container, x, y)
Widget container;
Position x, y;
{
	Widget w;
	int n_hosts = 0;
	Boolean any_hosts = False;
	struct host *h;
	Boolean need_line = False;

	/* Destroy any previous connect menu */

	if (connect_menu) {
		XtDestroyWidget(connect_menu);
		connect_menu = NULL;
	}
	if (connect_button) {
		XtDestroyWidget(connect_button);
		connect_button = NULL;
	}

	/* Create the menu */
	connect_menu = XtVaCreatePopupShell(
	    "hostMenu", complexMenuWidgetClass, container,
	    menubar_buttons ? XtNlabel : NULL, NULL,
	    NULL);
	if (!menubar_buttons)
		need_line = True;

	/* Start off with an opportunity to reconnect */

	if (current_host) {
		char *buf;

		if (need_line)
			(void) XtCreateManagedWidget("space",
			    cmeLineObjectClass, connect_menu, NULL, 0);
		buf = xs_buffer("%s %s", get_message("reconnect"),
		    current_host);
		w = XtVaCreateManagedWidget(
		    buf, cmeBSBObjectClass, connect_menu,
		    NULL);
		XtFree(buf);
		XtAddCallback(w, XtNcallback, do_reconnect, PN);
		need_line = True;
		n_hosts++;
	}
	if (appres.reconnect)
		goto make_button;

	/* Walk the host list from the file to produce the host menu */

	for (h = hosts; h; h = h->next) {
		if (h->entry_type != PRIMARY)
			continue;
		if (need_line && !any_hosts)
			(void) XtCreateManagedWidget("space",
			    cmeLineObjectClass, connect_menu, NULL, 0);
		any_hosts = True;
		w = XtVaCreateManagedWidget(
		    h->name, cmeBSBObjectClass, connect_menu,
		    NULL);
		XtAddCallback(w, XtNcallback, host_connect,
		    XtNewString(h->name));
		n_hosts++;
	}
	if (any_hosts)
		need_line = True;

	/* Add an "Other..." button at the bottom */

	if (!any_hosts || !appres.no_other) {
		if (need_line)
			(void) XtCreateManagedWidget("space", cmeLineObjectClass,
			    connect_menu, NULL, 0);
		w = XtVaCreateManagedWidget(
		    "otherHostOption", cmeBSBObjectClass, connect_menu,
		    NULL);
		XtAddCallback(w, XtNcallback, do_connect_popup, NULL);
	}

	/* Add the "Connect..." button itself to the container. */

    make_button:
	if (menubar_buttons) {
		if (appres.reconnect) {
			connect_button = XtVaCreateManagedWidget(
			    "reconnectButton", commandWidgetClass,
			    container,
			    XtNx, x,
			    XtNy, y,
			    XtNwidth, KEY_WIDTH,
			    XtNheight, KEY_HEIGHT,
			    XtNmappedWhenManaged, !PCONNECTED && reconnect_disabled,
			    NULL);
			XtAddCallback(connect_button, XtNcallback,
			    do_reconnect, NULL);
		} else if (n_hosts)
			connect_button = XtVaCreateManagedWidget(
			    "connectMenuButton", menuButtonWidgetClass,
			    container,
			    XtNx, x,
			    XtNy, y,
			    XtNwidth, KEY_WIDTH,
			    XtNheight, KEY_HEIGHT,
			    XtNmenuName, "hostMenu",
			    XtNmappedWhenManaged, !PCONNECTED,
			    NULL);
		else {
			connect_button = XtVaCreateManagedWidget(
			    "connectMenuButton", commandWidgetClass,
			    container,
			    XtNx, x,
			    XtNy, y,
			    XtNwidth, KEY_WIDTH,
			    XtNheight, KEY_HEIGHT,
			    XtNmappedWhenManaged, !PCONNECTED,
			    NULL);
			XtAddCallback(connect_button, XtNcallback,
			    do_connect_popup, NULL);
		}
	}
}

/*
 * Callback for macros
 */
/*ARGSUSED*/
static void
do_macro(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	macro_command((struct macro_def *)client_data);
}

/*
 * Initialize the "Macros..." menu
 */
static void
macros_menu_init(container, x, y)
Widget container;
Position x, y;
{
	Widget w;
	struct macro_def *m;
	Boolean any = False;

	/* Destroy any previous macros menu */

	if (macros_menu) {
		XtDestroyWidget(macros_menu);
		macros_menu = NULL;
	}
	if (macros_button) {
		XtDestroyWidget(macros_button);
		macros_button = NULL;
	}

	/* Walk the list */

	macros_init();
	for (m = macro_defs; m; m = m->next) {
		if (!any) {
			/* Create the menu */
			macros_menu = XtVaCreatePopupShell(
			    MACROS_MENU, complexMenuWidgetClass, container,
			    menubar_buttons ? XtNlabel : NULL, NULL,
			    NULL);
			if (!menubar_buttons)
				(void) XtCreateManagedWidget("space",
				    cmeLineObjectClass, macros_menu, NULL, 0);
		}
		w = XtVaCreateManagedWidget(
		    m->name, cmeBSBObjectClass, macros_menu,
		    NULL);
		XtAddCallback(w, XtNcallback, do_macro, (XtPointer)m);
		any = True;
	}

	/* Add the "Macros..." button itself to the container */

	if (any && menubar_buttons)
		macros_button = XtVaCreateManagedWidget(
		    "macrosMenuButton", menuButtonWidgetClass,
		    container,
		    XtNx, x,
		    XtNy, y,
		    XtNwidth, KEY_WIDTH,
		    XtNheight, KEY_HEIGHT,
		    XtNmenuName, MACROS_MENU,
		    XtNmappedWhenManaged, PCONNECTED,
		    NULL);
}

/* Called toggle the keypad */
/*ARGSUSED*/
static void
toggle_keypad(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	switch (kp_placement) {
	    case kp_integral:
		screen_showikeypad(appres.keypad_on = !appres.keypad_on);
		break;
	    case kp_left:
	    case kp_right:
	    case kp_bottom:
		keypad_popup_init();
		if (keypad_popped)
			XtPopdown(keypad_shell);
		else
			popup_popup(keypad_shell, XtGrabNone);
		break;
	}
	menubar_keypad_changed();
	keypad_changed = True;
}

static void
keypad_button_init(container, x, y)
Widget container;
Position x, y;
{
	Pixmap pixmap;

	pixmap = XCreateBitmapFromData(display, root_window,
	    (char *) ky_bits, ky_width, ky_height);

	keypad_button = XtVaCreateManagedWidget(
	    "keypadButton", commandWidgetClass, container,
	    XtNbitmap, pixmap,
	    XtNx, x,
	    XtNy, y,
	    XtNwidth, ky_width+8,
	    XtNheight, KEY_HEIGHT,
	    NULL);
	XtAddCallback(keypad_button, XtNcallback, toggle_keypad, NULL);
}

void
menubar_resize(width)
Dimension width;
{
	if (menubar_buttons) {
		XtDestroyWidget(keypad_button);
		keypad_button_init(menu_bar,
		    (Position) (width - LEFT_MARGIN - (ky_width+8) - 2*BORDER),
		    TOP_MARGIN);
	}
}


/*
 * "Options..." menu
 */

/*ARGSUSED*/
void
toggle_callback(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	struct toggle *t = (struct toggle *) userdata;

	/*
	 * If this is a two-button radio group, rather than a simple toggle,
	 * there is nothing to do if they are clicking on the current value.
	 *
	 * t->w[0] is the "toggle true" button; t->w[1] is "toggle false".
	 */
	if (t->w[1] != 0 && w == t->w[!t->value])
		return;

	do_toggle(t - appres.toggle);
}

Widget oversize_shell = NULL;

/* Called from the "Change" button on the oversize dialog */
/*ARGSUSED*/
static void
oversize_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *s;
	int ovc, ovr;
	char junk;

	s = XawDialogGetValueString((Widget)client_data);
	if (!s || !*s)
		return;
	if (sscanf(s, "%dx%d%c", &ovc, &ovr, &junk) == 2 &&
	    ovc * ovr < 0x4000) {
		XtPopdown(oversize_shell);
		screen_change_model(model_num, ovc, ovr);
	} else
		popup_an_error("Illegal size: %s", s);
}

/* Called from the "Oversize..." button on the "Models..." menu */
/*ARGSUSED*/
static void
do_oversize_popup(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (oversize_shell == NULL)
		oversize_shell = create_form_popup("Oversize",
		    oversize_button_callback, (XtCallbackProc)NULL, True);
	popup_popup(oversize_shell, XtGrabExclusive);
}

/* Init a toggle, menu-wise */
static void
toggle_init(menu, index, name1, name2)
Widget menu;
int index;
char *name1;
char *name2;
{
	struct toggle *t = &appres.toggle[index];

	t->label[0] = name1;
	t->label[1] = name2;
	t->w[0] = XtVaCreateManagedWidget(
	    name1, cmeBSBObjectClass, menu,
	    XtNleftBitmap,
	     t->value ? (name2 ? diamond : dot) : (name2 ? no_diamond : None),
	    NULL);
	XtAddCallback(t->w[0], XtNcallback, toggle_callback, (XtPointer) t);
	if (name2 != NULL) {
		t->w[1] = XtVaCreateManagedWidget(
		    name2, cmeBSBObjectClass, menu,
		    XtNleftBitmap, t->value ? no_diamond : diamond,
		    NULL);
		XtAddCallback(t->w[1], XtNcallback, toggle_callback, (XtPointer) t);
	} else
		t->w[1] = NULL;
}

Widget *font_widgets;
Widget other_font;
Widget font_shell = NULL;

/*ARGSUSED*/
void
do_newfont(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	screen_newfont((char *)userdata, True);
}

/* Called from the "Select Font" button on the font dialog */
/*ARGSUSED*/
static void
font_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *s;

	s = XawDialogGetValueString((Widget)client_data);
	if (!s || !*s)
		return;
	XtPopdown(font_shell);
	do_newfont(w, s, PN);
}

/*ARGSUSED*/
static void
do_otherfont(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	if (font_shell == NULL)
		font_shell = create_form_popup("Font", font_button_callback,
						(XtCallbackProc)NULL, True);
	popup_popup(font_shell, XtGrabExclusive);
}

/* Initialze the color scheme list. */
static void
scheme_init()
{
	char *cm;
	char *label;
	char *scheme;
	struct scheme *s;

	if (!appres.m3279)
		return;

	cm = get_resource(ResSchemeList);
	if (cm == CN)
		return;

	scheme_count = 0;
	while (split_dresource(&cm, &label, &scheme) == 1) {
		s = (struct scheme *)XtMalloc(sizeof(struct scheme));
		s->label = label;
		s->scheme = scheme;
		s->next = (struct scheme *)NULL;
		if (last_scheme != (struct scheme *)NULL)
			last_scheme->next = s;
		else
			schemes = s;
		last_scheme = s;
		scheme_count++;
	}
}

/*ARGSUSED*/
static void
do_newscheme(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	screen_newscheme((char *)userdata);
}

/* Initialze the character set list. */
static void
charsets_init()
{
	char *cm;
	char *label;
	char *charset;
	struct charset *s;

	cm = get_resource(ResCharsetList);
	if (cm == CN)
		return;

	charset_count = 0;
	while (split_dresource(&cm, &label, &charset) == 1) {
		s = (struct charset *)XtMalloc(sizeof(struct charset));
		s->label = label;
		s->charset = charset;
		s->next = (struct charset *)NULL;
		if (last_charset != (struct charset *)NULL)
			last_charset->next = s;
		else
			charsets = s;
		last_charset = s;
		charset_count++;
	}
}

/*ARGSUSED*/
static void
do_newcharset(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	struct charset *s;
	int i;

	/* Change the character set. */
	screen_newcharset((char *)userdata);

	/* Update the menu. */
	for (i = 0, s = charsets; i < charset_count; i++, s = s->next)
		XtVaSetValues(charset_widgets[i],
		    XtNleftBitmap,
			((appres.charset == CN) ||
			 strcmp(appres.charset, s->charset)) ?
			    no_diamond : diamond,
		    NULL);
}

Widget keymap_shell = NULL;

/* Called from the "Set Keymap" button on the keymap dialog */
/*ARGSUSED*/
static void
keymap_button_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *s;

	s = XawDialogGetValueString((Widget)client_data);
	if (!s || !*s)
		return;
	XtPopdown(keymap_shell);
	setup_keymaps(s, True);
	screen_set_keymap();
	keypad_set_keymap();
}

/* Callback from the "Keymap" menu option */
/*ARGSUSED*/
static void
do_keymap(w, userdata, calldata)
Widget w;
XtPointer userdata;
XtPointer calldata;
{
	if (keymap_shell == NULL)
		keymap_shell = create_form_popup("Keymap",
		    keymap_button_callback, (XtCallbackProc)NULL, True);
	popup_popup(keymap_shell, XtGrabExclusive);
}

/* Called to change telnet modes */
/*ARGSUSED*/
static void
linemode_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	net_linemode();
}

/*ARGSUSED*/
static void
charmode_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	net_charmode();
}

/* Called to change models */
/*ARGSUSED*/
static void
change_model_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	int m;

	m = atoi(client_data);
	switch (model_num) {
	    case 2:
		XtVaSetValues(model_2_button, XtNleftBitmap, no_diamond, NULL);
		break;
	    case 3:
		XtVaSetValues(model_3_button, XtNleftBitmap, no_diamond, NULL);
		break;
	    case 4:
		XtVaSetValues(model_4_button, XtNleftBitmap, no_diamond, NULL);
		break;
	    case 5:
		XtVaSetValues(model_5_button, XtNleftBitmap, no_diamond, NULL);
		break;
	}
	XtVaSetValues(w, XtNleftBitmap, diamond, NULL);
	screen_change_model(m, 0, 0);
}

static void
options_menu_init(container, x, y)
Widget container;
Position x, y;
{
	Widget m, t, w;
	struct font_list *f;
	int ix;

	/* Create the shell */
	m = XtVaCreatePopupShell(
	    "optionsMenu", complexMenuWidgetClass, container,
	    menubar_buttons ? XtNlabel : NULL, NULL,
	    NULL);
	if (!menubar_buttons)
		(void) XtCreateManagedWidget("space", cmeLineObjectClass,
		    m, NULL, 0);

	/* Create the "toggles" pullright */
	t = XtVaCreatePopupShell(
	    "togglesMenu", complexMenuWidgetClass, container,
	    NULL);
	if (!menubar_buttons) {
		keypad_option_button = XtVaCreateManagedWidget(
		    "keypadOption", cmeBSBObjectClass, t,
		    XtNleftBitmap, appres.keypad_on || keypad_popped ? dot : None,
		    NULL);
		XtAddCallback(keypad_option_button, XtNcallback, toggle_keypad,
		    PN);
		(void) XtCreateManagedWidget("space", cmeLineObjectClass,
		    t, NULL, 0);
	}
	toggle_init(t, MONOCASE, "monocaseOption", CN);
	toggle_init(t, CURSOR_BLINK, "cursorBlinkOption", CN);
	toggle_init(t, BLANK_FILL, "blankFillOption", CN);
	toggle_init(t, SHOW_TIMING, "showTimingOption", CN);
	toggle_init(t, CURSOR_POS, "cursorPosOption", CN);
	toggle_init(t, SCROLL_BAR, "scrollBarOption", CN);
	toggle_init(t, LINE_WRAP, "lineWrapOption", CN);
	toggle_init(t, MARGINED_PASTE, "marginedPasteOption", CN);
	toggle_init(t, RECTANGLE_SELECT, "rectangleSelectOption", CN);
	(void) XtCreateManagedWidget("space", cmeLineObjectClass, t, NULL, 0);
	toggle_init(t, ALT_CURSOR, "underlineCursorOption",
	    "blockCursorOption");
	(void) XtCreateManagedWidget("space", cmeLineObjectClass, t, NULL, 0);
	linemode_button = XtVaCreateManagedWidget(
	    "lineModeOption", cmeBSBObjectClass, t,
	    XtNleftBitmap, linemode ? diamond : no_diamond,
	    XtNsensitive, IN_ANSI,
	    NULL);
	XtAddCallback(linemode_button, XtNcallback, linemode_callback, NULL);
	charmode_button = XtVaCreateManagedWidget(
	    "characterModeOption", cmeBSBObjectClass, t,
	    XtNleftBitmap, linemode ? no_diamond : diamond,
	    XtNsensitive, IN_ANSI,
	    NULL);
	XtAddCallback(charmode_button, XtNcallback, charmode_callback, NULL);
	(void) XtVaCreateManagedWidget(
	    "togglesOption", cmeBSBObjectClass, m,
	    XtNrightBitmap, arrow,
	    XtNmenuName, "togglesMenu",
	    NULL);

	(void) XtCreateManagedWidget("space", cmeLineObjectClass, m, NULL, 0);

	/* Create the "fonts" pullright */
	t = XtVaCreatePopupShell(
	    "fontsMenu", complexMenuWidgetClass, container,
	    NULL);
	font_widgets = (Widget *)XtCalloc(font_count, sizeof(Widget));
	for (f = font_list, ix = 0; f; f = f->next, ix++) {
		font_widgets[ix] = XtVaCreateManagedWidget(
		    f->label, cmeBSBObjectClass, t,
		    XtNleftBitmap, !strcmp(efontname, f->font) ? diamond : no_diamond,
		    XtNsensitive, appres.apl_mode ? False : True,
		    NULL);
		XtAddCallback(font_widgets[ix], XtNcallback, do_newfont,
		    f->font);
	}
	if (!appres.no_other) {
		other_font = XtVaCreateManagedWidget(
		    "otherFontOption", cmeBSBObjectClass, t,
		    XtNsensitive, appres.apl_mode ? False : True,
		    NULL);
		XtAddCallback(other_font, XtNcallback, do_otherfont, NULL);
	}
	(void) XtVaCreateManagedWidget(
	    "fontsOption", cmeBSBObjectClass, m,
	    XtNrightBitmap, arrow,
	    XtNmenuName, "fontsMenu",
	    NULL);

	(void) XtCreateManagedWidget("space", cmeLineObjectClass, m, NULL, 0);

	/* Create the "models" pullright */
	t = XtVaCreatePopupShell(
	    "modelsMenu", complexMenuWidgetClass, container,
	    NULL);
	model_2_button = XtVaCreateManagedWidget(
	    "model2Option", cmeBSBObjectClass, t,
	    XtNleftBitmap, model_num == 2 ? diamond : no_diamond,
	    XtNsensitive, !PCONNECTED,
	    NULL);
	XtAddCallback(model_2_button, XtNcallback, change_model_callback, "2");
	model_3_button = XtVaCreateManagedWidget(
	    "model3Option", cmeBSBObjectClass, t,
	    XtNleftBitmap, model_num == 3 ? diamond : no_diamond,
	    XtNsensitive, !PCONNECTED,
	    NULL);
	XtAddCallback(model_3_button, XtNcallback, change_model_callback, "3");
#if defined(RESTRICT_3279) /*[*/
	if (!appres.m3279) {
#endif /*]*/
		model_4_button = XtVaCreateManagedWidget(
		    "model4Option", cmeBSBObjectClass, t,
		    XtNleftBitmap, model_num == 4 ? diamond : no_diamond,
		    XtNsensitive, !PCONNECTED,
		    NULL);
		XtAddCallback(model_4_button, XtNcallback,
		    change_model_callback, "4");
		model_5_button = XtVaCreateManagedWidget(
		    "model5Option", cmeBSBObjectClass, t,
		    XtNleftBitmap, model_num == 5 ? diamond : no_diamond,
		    XtNsensitive, !PCONNECTED,
		    NULL);
		XtAddCallback(model_5_button, XtNcallback,
		    change_model_callback, "5");
#if defined(RESTRICT_3279) /*[*/
	}
#endif /*]*/
	if (appres.extended) {
		oversize_button = XtVaCreateManagedWidget(
		    "oversizeOption", cmeBSBObjectClass, t,
		    XtNsensitive, !PCONNECTED,
		    NULL);
		XtAddCallback(oversize_button, XtNcallback, do_oversize_popup,
		    NULL);
	}
	models_option = XtVaCreateManagedWidget(
	    "modelsOption", cmeBSBObjectClass, m,
	    XtNrightBitmap, arrow,
	    XtNmenuName, "modelsMenu",
	    XtNsensitive, !PCONNECTED,
	    NULL);

	/* Create the "colors" pullright */
	if (scheme_count) {
		struct scheme *s;
		int i;

		(void) XtCreateManagedWidget("space", cmeLineObjectClass, m,
		    NULL, 0);

		scheme_widgets = (Widget *)XtCalloc(scheme_count,
		    sizeof(Widget));
		t = XtVaCreatePopupShell(
		    "colorsMenu", complexMenuWidgetClass, container,
		    NULL);
		s = schemes;
		for (i = 0, s = schemes; i < scheme_count; i++, s = s->next) {
			scheme_widgets[i] = XtVaCreateManagedWidget(
			    s->label, cmeBSBObjectClass, t,
			    XtNleftBitmap,
				!strcmp(appres.color_scheme, s->scheme) ?
				    diamond : no_diamond,
			    NULL);
			XtAddCallback(scheme_widgets[i], XtNcallback,
			    do_newscheme, s->scheme);
		}
		(void) XtVaCreateManagedWidget(
		    "colorsOption", cmeBSBObjectClass, m,
		    XtNrightBitmap, arrow,
		    XtNmenuName, "colorsMenu",
		    NULL);
	}

	/* Create the "character set" pullright */
	if (charset_count) {
		struct charset *s;
		int i;

		(void) XtCreateManagedWidget("space", cmeLineObjectClass, m,
		    NULL, 0);

		charset_widgets = (Widget *)XtCalloc(charset_count,
		    sizeof(Widget));
		t = XtVaCreatePopupShell(
		    "charsetMenu", complexMenuWidgetClass, container,
		    NULL);
		for (i = 0, s = charsets; i < charset_count; i++, s = s->next) {
			charset_widgets[i] = XtVaCreateManagedWidget(
			    s->label, cmeBSBObjectClass, t,
			    XtNleftBitmap,
				((appres.charset == CN) ||
				 strcmp(appres.charset, s->charset)) ?
				    no_diamond : diamond,
			    NULL);
			XtAddCallback(charset_widgets[i], XtNcallback,
			    do_newcharset, s->charset);
		}
		(void) XtVaCreateManagedWidget(
		    "charsetOption", cmeBSBObjectClass, m,
		    XtNrightBitmap, arrow,
		    XtNmenuName, "charsetMenu",
		    NULL);
	}

	/* Create the "keymap" option */
	if (!appres.no_other) {
		(void) XtCreateManagedWidget("space", cmeLineObjectClass, m,
		    NULL, 0);
		w = XtVaCreateManagedWidget(
		    "keymapOption", cmeBSBObjectClass, m,
		    NULL);
		XtAddCallback(w, XtNcallback, do_keymap, NULL);
	}

	if (menubar_buttons) {
		(void) XtVaCreateManagedWidget(
		    "optionsMenuButton", menuButtonWidgetClass, container,
		    XtNx, x,
		    XtNy, y,
		    XtNwidth, KEY_WIDTH,
		    XtNheight, KEY_HEIGHT,
		    XtNmenuName, "optionsMenu",
		    NULL);
		keypad_option_button = NULL;
	}
}

/*
 * Change a menu checkmark
 */
void
menubar_retoggle(t)
struct toggle *t;
{
	XtVaSetValues(t->w[0],
	    XtNleftBitmap, t->value ? (t->w[1] ? diamond : dot) : None,
	    NULL);
	if (t->w[1] != NULL)
		XtVaSetValues(t->w[1],
		    XtNleftBitmap, t->value ? no_diamond : diamond,
		    NULL);
}

/*ARGSUSED*/
void
HandleMenu_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	String p;

	action_debug(HandleMenu_action, event, params, num_params);
	if (check_usage(HandleMenu_action, *num_params, 1, 2) < 0)
		return;
	if (!CONNECTED || *num_params == 1)
		p = params[0];
	else
		p = params[1];
	if (!XtNameToWidget(menu_bar, p)) {
		if (strcmp(p, MACROS_MENU))
			popup_an_error("%s: cannot find menu %s",
			    action_name(HandleMenu_action), p);
		return;
	}
	XtCallActionProc(menu_bar, "XawPositionComplexMenu", event, &p, 1);
	XtCallActionProc(menu_bar, "MenuPopup", event, &p, 1);
}



/*
 * Host file support
 */

static char *
stoken(s)
char **s;
{
	char *r;
	char *ss = *s;

	if (!*ss)
		return NULL;
	r = ss;
	while (*ss && *ss != ' ' && *ss != '\t')
		ss++;
	if (*ss) {
		*ss++ = '\0';
		while (*ss == ' ' || *ss == '\t')
			ss++;
	}
	*s = ss;
	return r;
}

/*
 * Read the host file
 */
void
hostfile_init()
{
	FILE *hf;
	char buf[1024];

	if (appres.hostsfile == CN)
		appres.hostsfile = xs_buffer("%s/ibm_hosts", LIBX3270DIR);
	hf = fopen(appres.hostsfile, "r");
	if (!hf)
		return;

	while (fgets(buf, 1024, hf)) {
		char *s = buf;
		char *name, *entry_type, *hostname;
		struct host *h;
		char *slash;

		if (strlen(buf) > (unsigned) 1)
			buf[strlen(buf) - 1] = '\0';
		while (isspace(*s))
			s++;
		if (!*s || *s == '#')
			continue;
		name = stoken(&s);
		entry_type = stoken(&s);
		hostname = stoken(&s);
		if (!name || !entry_type || !hostname) {
			xs_warning("Bad %s syntax, entry skipped",
			    ResHostsFile);
			continue;
		}
		h = (struct host *)XtMalloc(sizeof(*h));
		h->name = XtNewString(name);
		h->hostname = XtNewString(hostname);

		/*
		 * Quick syntax extension to allow the hosts file to
		 * specify a port as host/port.
		 */
		if ((slash = strchr(h->hostname, '/')))
			*slash = ' ';

		if (!strcmp(entry_type, "primary"))
			h->entry_type = PRIMARY;
		else
			h->entry_type = ALIAS;
		if (*s)
			h->loginstring = XtNewString(s);
		else
			h->loginstring = CN;
		h->next = (struct host *)0;
		if (last_host)
			last_host->next = h;
		else
			hosts = h;
		last_host = h;
	}
	(void) fclose(hf);
}

/*
 * Look up a host in the list.  Turns aliases into real hostnames, and
 * finds loginstrings.
 */
int
hostfile_lookup(name, hostname, loginstring)
char *name;
char **hostname;
char **loginstring;
{
	struct host *h;

	for (h = hosts; h; h = h->next)
		if (! strcmp(name, h->name)) {
			*hostname = h->hostname;
			*loginstring = h->loginstring;
			return 1;
		}
	return 0;
}

/* Explicit connect/disconnect actions. */

/*ARGSUSED*/
void
Connect_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_debug(Connect_action, event, params, num_params);
	if (check_usage(Connect_action, *num_params, 1, 1) < 0)
		return;
	if (CONNECTED || HALF_CONNECTED) {
		popup_an_error("Already connected");
		return;
	}
	(void) x_connect(params[0]);

	/*
	 * If called from a script and the connection was successful (or
	 * half-successful), pause the script until we are connected and
	 * we have identified the host type.
	 */
	if (!w && (CONNECTED || HALF_CONNECTED))
		sms_connect_wait();
}

/*ARGSUSED*/
void
Reconnect_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_debug(Reconnect_action, event, params, num_params);
	if (check_usage(Reconnect_action, *num_params, 0, 0) < 0)
		return;
	if (CONNECTED || HALF_CONNECTED) {
		popup_an_error("Already connected");
		return;
	}
	if (!current_host) {
		popup_an_error("No previous host to connect to");
		return;
	}
	x_reconnect();

	/*
	 * If called from a script and the connection was successful (or
	 * half-successful), pause the script until we are connected and
	 * we have identified the host type.
	 */
	if (!w && (CONNECTED || HALF_CONNECTED))
		sms_connect_wait();
}

/*ARGSUSED*/
void
Disconnect_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_debug(Disconnect_action, event, params, num_params);
	if (check_usage(Disconnect_action, *num_params, 0, 0) < 0)
		return;
	x_disconnect(False);
}
