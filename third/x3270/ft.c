/*
 * Modifications Copyright 1996 by Paul Mattes.
 * Copyright Octover 1995 by Dick Altenbern.
 * Based in part on code Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	ft.c
 *		This module handles the file transfer dialogs.
 */

#include "globals.h"

#include <X11/StringDefs.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/TextSrc.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/AsciiSink.h>
#include <errno.h>

#if XtSpecificationRelease < 5 /*[*/
#define TOGGLE_HACK 1
#endif /*]*/

#include "appres.h"
#include "actionsc.h"
#include "ft_cutc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "kybdc.h"
#include "objects.h"
#include "popupsc.h"
#include "telnetc.h"
#include "utilc.h"

/* Macros. */
#define eos(s)	strchr((s), '\0')

#define FILE_WIDTH	300	/* width of file name widgets */
#define MARGIN		3	/* distance from margins to widgets */
#define CLOSE_VGAP	0	/* distance between paired toggles */
#define FAR_VGAP	10	/* distance between single toggles and groups */
#define BUTTON_GAP	5	/* horizontal distance between buttons */
#define COLUMN_GAP	40	/* distance between columns */

/* Externals. */
extern Pixmap diamond;
extern Pixmap no_diamond;
extern Pixmap null;
extern Pixmap dot;

/* Globals. */
enum ft_state ft_state = FT_NONE;	/* File transfer state */
char *ft_local_filename;		/* Local file to transfer to/from */
FILE *ft_local_file = (FILE *)NULL;	/* File descriptor for local file */
Boolean ascii_flag = True;		/* Convert to ascii */
Boolean cr_flag = True;			/* Add crlf to each line */
unsigned long ft_length = 0;		/* Length of transfer */

/* Statics. */
static Widget ft_dialog, ft_shell, local_file, host_file;
static Widget lrecl_widget, blksize_widget;
static Widget primspace_widget, secspace_widget;
static Widget send_toggle, receive_toggle;
static Widget vm_toggle, tso_toggle;
static Widget ascii_toggle, binary_toggle;
static Widget cr_widget;

static Boolean receive_flag = True;	/* Current transfer is receive */
static Boolean append_flag = False;	/* Append transfer */
static Boolean vm_flag = False;		/* VM Transfer flag */

struct toggle_list {			/* List of toggle widgets */
	Widget *widgets;
};
static Widget recfm_options[5];
static Widget units_options[5];
static struct toggle_list recfm_toggles = { recfm_options };
static struct toggle_list units_toggles = { units_options };

static enum recfm {
	DEFAULT_RECFM, FIXED, VARIABLE, UNDEFINED
} recfm = DEFAULT_RECFM;
static Boolean recfm_default = True;
static enum recfm r_default_recfm = DEFAULT_RECFM;
static enum recfm r_fixed = FIXED;
static enum recfm r_variable = VARIABLE;
static enum recfm r_undefined = UNDEFINED;

static enum units {
	DEFAULT_UNITS, TRACKS, CYLINDERS, AVBLOCK
} units = DEFAULT_UNITS;
static Boolean units_default = True;
static enum units u_default_units = DEFAULT_UNITS;
static enum units u_tracks = TRACKS;
static enum units u_cylinders = CYLINDERS;
static enum units u_avblock = AVBLOCK;

typedef enum { T_NUMERIC, T_HOSTFILE, T_UNIXFILE } text_t;
static text_t t_numeric = T_NUMERIC;
static text_t t_hostfile = T_HOSTFILE;
static text_t t_unixfile = T_UNIXFILE;

static Boolean s_true = True;
static Boolean s_false = False;
static Boolean allow_overwrite = False;
typedef struct sr {
	struct sr *next;
	Widget w;
	Boolean *bvar1;
	Boolean bval1;
	Boolean *bvar2;
	Boolean bval2;
	Boolean *bvar3;
	Boolean bval3;
	Boolean is_value;
	Boolean has_focus;
} sr_t;
sr_t *sr = (sr_t *)NULL;
sr_t *sr_last = (sr_t *)NULL;

static Widget progress_shell, from_file, to_file;
static Widget ft_status, waiting, aborting;
static String status_string;
static struct timeval t0;		/* Starting time */
static Boolean ft_is_cut;		/* File transfer is CUT-style */

static Widget overwrite_shell;

static void apply_bitmap();
static void check_sensitivity();
static void flip_toggles();
static void focus_next();
static void ft_cancel();
static void ft_popup_callback();
static void ft_popup_init();
static int ft_start();
static void ft_start_callback();
static void match_dimension();
static void overwrite_cancel_callback();
static void overwrite_okay_callback();
static void overwrite_popdown();
static void overwrite_popup_init();
static void popup_overwrite();
static void popup_progress();
static void progress_cancel_callback();
static void progress_popup_callback();
static void progress_popup_init();
static void recfm_callback();
static void register_sensitivity();
static void text_callback();
static void toggle_append();
static void toggle_ascii();
static void toggle_cr();
static void toggle_receive();
static void toggle_vm();
static void units_callback();

/* Main external entry point. */

/* "File Transfer" dialog. */

/*
 * Pop up the "Transfer" menu.
 * Called back from the "File Transfer" option on the File menu.
 */
void
popup_ft(w, call_parms, call_data)
Widget w;
XtPointer call_parms;
XtPointer call_data;
{
	/* Initialize it. */
	if (ft_shell == (Widget)NULL)
		ft_popup_init();

	/* Pop it up. */
	popup_popup(ft_shell, XtGrabNone);
}


/* Initialize the transfer pop-up. */
static void
ft_popup_init()
{
	Widget w;
	Widget cancel_button;
	Widget local_label, host_label;
	Widget append_widget;
	Widget lrecl_label, blksize_label, primspace_label, secspace_label;
	Widget h_ref = (Widget)NULL;
	Dimension d1;
	Dimension maxw = 0;
	Widget recfm_label, units_label;
	Widget start_button;

	/* Create the menu shell. */
	ft_shell = XtVaCreatePopupShell(
	    "ftPopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(ft_shell, XtNpopupCallback, place_popup,
	    (XtPointer)CenterP);
	XtAddCallback(ft_shell, XtNpopupCallback, ft_popup_callback,
	    (XtPointer)NULL);

	/* Create the form within the shell. */
	ft_dialog = XtVaCreateManagedWidget(
	    "dialog", formWidgetClass, ft_shell,
	    NULL);

	/* Create the file name widgets. */
	local_label = XtVaCreateManagedWidget(
	    "local", labelWidgetClass, ft_dialog,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	local_file = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNeditType, XawtextEdit,
	    XtNwidth, FILE_WIDTH,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, local_label,
	    XtNhorizDistance, 0,
	    NULL);
	match_dimension(local_label, local_file, XtNheight);
	w = XawTextGetSource(local_file);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_unixfile);
	register_sensitivity(local_file,
	    NULL, NULL,
	    NULL, NULL,
	    NULL, NULL);

	host_label = XtVaCreateManagedWidget(
	    "host", labelWidgetClass, ft_dialog,
	    XtNfromVert, local_label,
	    XtNvertDistance, 3,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	host_file = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNeditType, XawtextEdit,
	    XtNwidth, FILE_WIDTH,
	    XtNdisplayCaret, False,
	    XtNfromVert, local_label,
	    XtNvertDistance, 3,
	    XtNfromHoriz, host_label,
	    XtNhorizDistance, 0,
	    NULL);
	match_dimension(host_label, host_file, XtNheight);
	match_dimension(local_label, host_label, XtNwidth);
	w = XawTextGetSource(host_file);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_hostfile);
	register_sensitivity(host_file,
	    NULL, NULL,
	    NULL, NULL,
	    NULL, NULL);

	/* Create the left column. */

	/* Create send/receive toggles. */
	send_toggle = XtVaCreateManagedWidget(
	    "send", commandWidgetClass, ft_dialog,
	    XtNfromVert, host_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(send_toggle, receive_flag ? no_diamond : diamond);
	XtAddCallback(send_toggle, XtNcallback, toggle_receive,
	    (XtPointer)&s_false);
	receive_toggle = XtVaCreateManagedWidget(
	    "receive", commandWidgetClass, ft_dialog,
	    XtNfromVert, send_toggle,
	    XtNvertDistance, CLOSE_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(receive_toggle, receive_flag ? diamond : no_diamond);
	XtAddCallback(receive_toggle, XtNcallback, toggle_receive,
	    (XtPointer)&s_true);

	/* Create ASCII/binary toggles. */
	ascii_toggle = XtVaCreateManagedWidget(
	    "ascii", commandWidgetClass, ft_dialog,
	    XtNfromVert, receive_toggle,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(ascii_toggle, ascii_flag ? diamond : no_diamond);
	XtAddCallback(ascii_toggle, XtNcallback, toggle_ascii,
	    (XtPointer)&s_true);
	binary_toggle = XtVaCreateManagedWidget(
	    "binary", commandWidgetClass, ft_dialog,
	    XtNfromVert, ascii_toggle,
	    XtNvertDistance, CLOSE_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(binary_toggle, ascii_flag ? no_diamond : diamond);
	XtAddCallback(binary_toggle, XtNcallback, toggle_ascii,
	    (XtPointer)&s_false);

	/* Create append toggle. */
	append_widget = XtVaCreateManagedWidget(
	    "append", commandWidgetClass, ft_dialog,
	    XtNfromVert, binary_toggle,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(append_widget, append_flag ? dot : null);
	XtAddCallback(append_widget, XtNcallback, toggle_append, NULL);

	/* Set up the recfm group. */
	recfm_label = XtVaCreateManagedWidget(
	    "file", labelWidgetClass, ft_dialog,
	    XtNfromVert, append_widget,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	register_sensitivity(recfm_label,
	    &receive_flag, False,
	    NULL, NULL,
	    NULL, NULL);

	recfm_options[0] = XtVaCreateManagedWidget(
	    "recfmDefault", commandWidgetClass, ft_dialog,
	    XtNfromVert, recfm_label,
	    XtNvertDistance, 3,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(recfm_options[0],
	    (recfm == DEFAULT_RECFM) ? diamond : no_diamond);
	XtAddCallback(recfm_options[0], XtNcallback, recfm_callback,
	    (XtPointer)&r_default_recfm);
	register_sensitivity(recfm_options[0],
	    &receive_flag, False,
	    NULL, NULL,
	    NULL, NULL);

	recfm_options[1] = XtVaCreateManagedWidget(
	    "fixed", commandWidgetClass, ft_dialog,
	    XtNfromVert, recfm_options[0],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(recfm_options[1],
	    (recfm == FIXED) ? diamond : no_diamond);
	XtAddCallback(recfm_options[1], XtNcallback, recfm_callback,
	    (XtPointer)&r_fixed);
	register_sensitivity(recfm_options[1],
	    &receive_flag, False,
	    NULL, NULL,
	    NULL, NULL);

	recfm_options[2] = XtVaCreateManagedWidget(
	    "variable", commandWidgetClass, ft_dialog,
	    XtNfromVert, recfm_options[1],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(recfm_options[2],
	    (recfm == VARIABLE) ? diamond : no_diamond);
	XtAddCallback(recfm_options[2], XtNcallback, recfm_callback,
	    (XtPointer)&r_variable);
	register_sensitivity(recfm_options[2],
	    &receive_flag, False,
	    NULL, NULL,
	    NULL, NULL);

	recfm_options[3] = XtVaCreateManagedWidget(
	    "undefined", commandWidgetClass, ft_dialog,
	    XtNfromVert, recfm_options[2],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(recfm_options[3],
	    (recfm == UNDEFINED) ? diamond : no_diamond);
	XtAddCallback(recfm_options[3], XtNcallback, recfm_callback,
	    (XtPointer)&r_undefined);
	register_sensitivity(recfm_options[3],
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	lrecl_label = XtVaCreateManagedWidget(
	    "lrecl", labelWidgetClass, ft_dialog,
	    XtNfromVert, recfm_options[3],
	    XtNvertDistance, 3,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	register_sensitivity(lrecl_label,
	    &receive_flag, False,
	    &recfm_default, False,
	    NULL, NULL);
	lrecl_widget = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNfromVert, recfm_options[3],
	    XtNvertDistance, 3,
	    XtNfromHoriz, lrecl_label,
	    XtNhorizDistance, MARGIN,
	    XtNwidth, 100,
	    XtNeditType, XawtextEdit,
	    XtNdisplayCaret, False,
	    NULL);
	match_dimension(lrecl_label, lrecl_widget, XtNheight);
	w = XawTextGetSource(lrecl_widget);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_numeric);
	register_sensitivity(lrecl_widget,
	    &receive_flag, False,
	    &recfm_default, False,
	    NULL, NULL);

	blksize_label = XtVaCreateManagedWidget(
	    "blksize", labelWidgetClass, ft_dialog,
	    XtNfromVert, lrecl_widget,
	    XtNvertDistance, 3,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	match_dimension(blksize_label, lrecl_label, XtNwidth);
	register_sensitivity(blksize_label,
	    &receive_flag, False,
	    &recfm_default, False,
	    NULL, NULL);
	blksize_widget = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNfromVert, lrecl_widget,
	    XtNvertDistance, 3,
	    XtNfromHoriz, blksize_label,
	    XtNhorizDistance, MARGIN,
	    XtNwidth, 100,
	    XtNeditType, XawtextEdit,
	    XtNdisplayCaret, False,
	    NULL);
	match_dimension(blksize_label, blksize_widget, XtNheight);
	w = XawTextGetSource(blksize_widget);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_numeric);
	register_sensitivity(blksize_widget,
	    &receive_flag, False,
	    &recfm_default, False,
	    NULL, NULL);


	/* Find the widest widget in the left column. */
	XtVaGetValues(send_toggle, XtNwidth, &maxw, NULL);
	h_ref = send_toggle;
#define REMAX(w) { \
		XtVaGetValues((w), XtNwidth, &d1, NULL); \
		if (d1 > maxw) { \
			maxw = d1; \
			h_ref = (w); \
		} \
	}
	REMAX(receive_toggle);
	REMAX(ascii_toggle);
	REMAX(binary_toggle);
	REMAX(append_widget);
#undef REMAX

	/* Create the right column buttons. */

	/* Create VM/TSO toggle. */
	vm_toggle = XtVaCreateManagedWidget(
	    "vm", commandWidgetClass, ft_dialog,
	    XtNfromVert, host_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(vm_toggle, vm_flag ? diamond : no_diamond);
	XtAddCallback(vm_toggle, XtNcallback, toggle_vm, (XtPointer)&s_true);
	tso_toggle =  XtVaCreateManagedWidget(
	    "tso", commandWidgetClass, ft_dialog,
	    XtNfromVert, vm_toggle,
	    XtNvertDistance, CLOSE_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(tso_toggle, vm_flag ? no_diamond : diamond);
	XtAddCallback(tso_toggle, XtNcallback, toggle_vm, (XtPointer)&s_false);

	/* Create CR toggle. */
	cr_widget = XtVaCreateManagedWidget(
	    "cr", commandWidgetClass, ft_dialog,
	    XtNfromVert, tso_toggle,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(cr_widget, cr_flag ? dot : null);
	XtAddCallback(cr_widget, XtNcallback, toggle_cr, 0);
	register_sensitivity(cr_widget,
	    &ascii_flag, True,
	    NULL, NULL,
	    NULL, NULL);

	/* Set up the Units group. */
	units_label = XtVaCreateManagedWidget(
	    "units", labelWidgetClass, ft_dialog,
	    XtNfromVert, append_widget,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	register_sensitivity(units_label,
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	units_options[0] = XtVaCreateManagedWidget(
	    "spaceDefault", commandWidgetClass, ft_dialog,
	    XtNfromVert, units_label,
	    XtNvertDistance, 3,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(units_options[0],
	    (units == DEFAULT_UNITS) ? diamond : no_diamond);
	XtAddCallback(units_options[0], XtNcallback,
	    units_callback, (XtPointer)&u_default_units);
	register_sensitivity(units_options[0],
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	units_options[1] = XtVaCreateManagedWidget(
	    "tracks", commandWidgetClass, ft_dialog,
	    XtNfromVert, units_options[0],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(units_options[1],
	    (units == TRACKS) ? diamond : no_diamond);
	XtAddCallback(units_options[1], XtNcallback,
	    units_callback, (XtPointer)&u_tracks);
	register_sensitivity(units_options[1],
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	units_options[2] = XtVaCreateManagedWidget(
	    "cylinders", commandWidgetClass, ft_dialog,
	    XtNfromVert, units_options[1],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(units_options[2],
	    (units == CYLINDERS) ? diamond : no_diamond);
	XtAddCallback(units_options[2], XtNcallback,
	    units_callback, (XtPointer)&u_cylinders);
	register_sensitivity(units_options[2],
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	units_options[3] = XtVaCreateManagedWidget(
	    "avblock", commandWidgetClass, ft_dialog,
	    XtNfromVert, units_options[2],
	    XtNvertDistance, CLOSE_VGAP,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	apply_bitmap(units_options[3],
	    (units == AVBLOCK) ? diamond : no_diamond);
	XtAddCallback(units_options[3], XtNcallback,
	    units_callback, (XtPointer)&u_avblock);
	register_sensitivity(units_options[3],
	    &receive_flag, False,
	    &vm_flag, False,
	    NULL, NULL);

	primspace_label = XtVaCreateManagedWidget(
	    "primspace", labelWidgetClass, ft_dialog,
	    XtNfromVert, units_options[3],
	    XtNvertDistance, 3,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	register_sensitivity(primspace_label,
	    &receive_flag, False,
	    &vm_flag, False,
	    &units_default, False);
	primspace_widget = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNfromVert, units_options[3],
	    XtNvertDistance, 3,
	    XtNfromHoriz, primspace_label,
	    XtNhorizDistance, 0,
	    XtNwidth, 100,
	    XtNeditType, XawtextEdit,
	    XtNdisplayCaret, False,
	    NULL);
	match_dimension(primspace_label, primspace_widget, XtNheight);
	w = XawTextGetSource(primspace_widget);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_numeric);
	register_sensitivity(primspace_widget,
	    &receive_flag, False,
	    &vm_flag, False,
	    &units_default, False);

	secspace_label = XtVaCreateManagedWidget(
	    "secspace", labelWidgetClass, ft_dialog,
	    XtNfromVert, primspace_widget,
	    XtNvertDistance, 3,
	    XtNfromHoriz, h_ref,
	    XtNhorizDistance, COLUMN_GAP,
	    XtNborderWidth, 0,
	    NULL);
	match_dimension(primspace_label, secspace_label, XtNwidth);
	register_sensitivity(secspace_label,
	    &receive_flag, False,
	    &vm_flag, False,
	    &units_default, False);
	secspace_widget = XtVaCreateManagedWidget(
	    "value", asciiTextWidgetClass, ft_dialog,
	    XtNfromVert, primspace_widget,
	    XtNvertDistance, 3,
	    XtNfromHoriz, secspace_label,
	    XtNhorizDistance, 0,
	    XtNwidth, 100,
	    XtNeditType, XawtextEdit,
	    XtNdisplayCaret, False,
	    NULL);
	match_dimension(secspace_label, secspace_widget, XtNheight);
	w = XawTextGetSource(secspace_widget);
	if (w == NULL)
		XtWarning("Cannot find text source in dialog");
	else
		XtAddCallback(w, XtNcallback, text_callback,
		    (XtPointer)&t_numeric);
	register_sensitivity(secspace_widget,
	    &receive_flag, False,
	    &vm_flag, False,
	    &units_default, False);

	/* Set up the buttons at the bottom. */
	start_button = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, ft_dialog,
	    XtNfromVert, blksize_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    NULL);
	XtAddCallback(start_button, XtNcallback, ft_start_callback,
	    (XtPointer)NULL);

	cancel_button = XtVaCreateManagedWidget(
	    ObjCancelButton, commandWidgetClass, ft_dialog,
	    XtNfromVert, blksize_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, start_button,
	    XtNhorizDistance, BUTTON_GAP,
	    NULL);
	XtAddCallback(cancel_button, XtNcallback, ft_cancel, 0);
}



/* Callbacks for all the transfer widgets. */

/* Transfer pop-up popping up. */
static void
ft_popup_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Set the focus to the local file widget. */
	dialog_focus_action(local_file, (XEvent *)NULL, (String *)NULL,
	    (Cardinal *)NULL);

	/* Disallow overwrites. */
	allow_overwrite = False;
}

/* Cancel button pushed. */
static void
ft_cancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(ft_shell);
}

/* recfm options. */
static void
recfm_callback(w, user_data, call_data)
Widget w;
XtPointer user_data;
XtPointer call_data;
{
	recfm = *(enum recfm *)user_data;
	recfm_default = (recfm == DEFAULT_RECFM);
	check_sensitivity(&recfm_default);
	flip_toggles(&recfm_toggles, w);
}

/* Units options. */
static void
units_callback(w, user_data, call_data)
Widget w;
XtPointer user_data;
XtPointer call_data;
{
	units = *(enum units *)user_data;
	units_default = (units == DEFAULT_UNITS);
	check_sensitivity(&units_default);
	flip_toggles(&units_toggles, w);
}

/* OK button pushed. */
static void
ft_start_callback(w, call_parms, call_data)
Widget w;
XtPointer call_parms;
XtPointer call_data;
{
	if (ft_start()) {
		XtPopdown(ft_shell);
		popup_progress();
	}
}

/* Mark a toggle. */
static void
mark_toggle(w, p)
Widget w;
Pixmap p;
{
#if defined(TOGGLE_HACK) /*[*/
	String l, nl;

	XtVaGetValues(w, XtNlabel, &l, NULL);
	nl = XtNewString(l);
	if (p == diamond)
		nl[0] = '+';
	else if (p == no_diamond)
		nl[0] = '-';
	else if (p == dot)
		nl[0] = '*';
	else if (p == null)
		nl[0] = ' ';
	XtVaSetValues(w, XtNlabel, nl, NULL);
	XtFree(nl);
#else /*][*/
	XtVaSetValues(w, XtNleftBitmap, p, NULL);
#endif /*]*/
}

/* Send/receive options. */
static void
toggle_receive(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Toggle the flag */
	receive_flag = *(Boolean *)client_data;

	/* Change the widget states. */
	mark_toggle(receive_toggle, receive_flag ? diamond : no_diamond);
	mark_toggle(send_toggle, receive_flag ? no_diamond : diamond);
	check_sensitivity(&receive_flag);
}

/* Ascii/binary options. */
static void
toggle_ascii(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Toggle the flag. */
	ascii_flag = *(Boolean *)client_data;

	/* Change the widget states. */
	mark_toggle(ascii_toggle, ascii_flag ? diamond : no_diamond);
	mark_toggle(binary_toggle, ascii_flag ? no_diamond : diamond);
	cr_flag = ascii_flag;
	mark_toggle(cr_widget, cr_flag ? dot : null);
	check_sensitivity(&ascii_flag);
}

/* CR option. */
static void
toggle_cr(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Toggle the cr flag */
	cr_flag = !cr_flag;

	mark_toggle(w, cr_flag ? dot : null);
}

/* Append option. */
static void
toggle_append(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Toggle Append Flag */
	append_flag = !append_flag;

	mark_toggle(w, append_flag ? dot : null);
}

/* TSO/VM option. */
static void
toggle_vm(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* Toggle the flag. */
	vm_flag = *(Boolean *)client_data;

	/* Change the widget states. */
	mark_toggle(vm_toggle, vm_flag ? diamond : no_diamond);
	mark_toggle(tso_toggle, vm_flag ? no_diamond : diamond);

	if (vm_flag) {
		if (recfm == UNDEFINED) {
			recfm = DEFAULT_RECFM;
			recfm_default = True;
			flip_toggles(&recfm_toggles,
			    recfm_toggles.widgets[0]);
		}
	}
	check_sensitivity(&vm_flag);
}

/*
 * Begin the transfer.
 * Returns 1 if the transfer has started, 0 otherwise.
 */
static int
ft_start()
{
	char opts[80];
	char *op = opts + 1;
	char *cmd;
	String lrecl, blksize, primspace, secspace;
	int flen;
	char *host_filename;

	/* Get the host file from its widget */
	XtVaGetValues(host_file, XtNstring, &host_filename, NULL);
	if (!*host_filename)
		return 0;
	/* XXX: probably more validation to do here */

	/* Get the local file from it widget */
	XtVaGetValues(local_file, XtNstring,  &ft_local_filename, NULL);
	if (!*ft_local_filename)
		return 0;

	/* See if the local file can be overwritten. */
	if (receive_flag && !append_flag && !allow_overwrite) {
		ft_local_file = fopen(ft_local_filename, "r");
		if (ft_local_file != (FILE *)NULL) {
			(void) fclose(ft_local_file);
			ft_local_file = (FILE *)NULL;
			popup_overwrite();
			return 0;
		}
	}

	/* Open the local file. */
	ft_local_file = fopen(ft_local_filename,
	    receive_flag ?
		(append_flag ? "a" : "w" ) :
		"r");
	if (ft_local_file == (FILE *)NULL) {
		allow_overwrite = False;
		popup_an_errno(errno, "Open(%s)", ft_local_filename);
		return 0;
	}

	/* Build the ind$file command */
	op[0] = '\0';
	if (ascii_flag)
		strcat(op, " ascii");
	if (cr_flag)
		strcat(op, " crlf");
	if (append_flag && !receive_flag)
		strcat(op, " append");
	if (!receive_flag) {
		if (!vm_flag) {
			if (recfm != DEFAULT_RECFM) {
				/* RECFM Entered, process */
				strcat(op, " recfm(");
				switch (recfm) {
				    case FIXED:
					strcat(op, "f");
					break;
				    case VARIABLE:
					strcat(op, "v");
					break;
				    case UNDEFINED:
					strcat(op, "u");
					break;
				    default:
					break;
				};
				strcat(op, ")");
				XtVaGetValues(lrecl_widget,
				    XtNstring, &lrecl,
				    NULL);
				if (strlen(lrecl) > 0)
					sprintf(eos(op), " lrecl(%s)", lrecl);
				XtVaGetValues(blksize_widget,
				    XtNstring, &blksize,
				    NULL);
				if (strlen(blksize) > 0)
					sprintf(eos(op), " blksize(%s)",
					    blksize);
			}
			if (units != DEFAULT_UNITS) {
				/* Space Entered, processs it */
				switch (units) {
				    case TRACKS:
					strcat(op, " tracks");
					break;
				    case CYLINDERS:
					strcat(op, " cylinders");
					break;
				    case AVBLOCK:
					strcat(op, " avblock");
					break;
				    default:
					break;
				};
				XtVaGetValues(primspace_widget, XtNstring,
				    &primspace, NULL);
				if (strlen(primspace) > 0) {
					sprintf(eos(op), " space(%s",
					    primspace);
					XtVaGetValues(secspace_widget,
					    XtNstring, &secspace,
					    NULL);
					if (strlen(secspace) > 0)
						sprintf(eos(op), ",%s",
						    secspace);
					strcat(op, ")");
				}
			}
		} else {
			if (recfm != DEFAULT_RECFM) {
				strcat(op, " recfm ");
				switch (recfm) {
				    case FIXED:
					strcat(op, "f");
					break;
				    case VARIABLE:
					strcat(op, "v");
					break;
				    default:
					break;
				};

				XtVaGetValues(lrecl_widget,
				    XtNstring, &lrecl,
				    NULL);
				if (strlen(lrecl) > 0)
					sprintf(eos(op), " lrecl %s", lrecl);
			}
		}
	}

	/* Insert the '(' for VM options. */
	if (strlen(op) > 0 && vm_flag) {
		opts[0] = ' ';
		opts[1] = '(';
		op = opts;
	}

	/* Build the whole command. */
	cmd = xs_buffer("%s %s %s%s\\n",
	    appres.ft_command,
	    receive_flag ? "get" : "put", host_filename, op);

	/* Erase the line and enter the command. */
	flen = kybd_prime();
	if (!flen || flen < strlen(cmd) - 1) {
		XtFree(cmd);
		popup_an_error(get_message("ftUnable"));
		allow_overwrite = False;
		return 0;
	}
	(void) emulate_input(cmd, strlen(cmd), False);
	XtFree(cmd);

	/* Get this thing started. */
	ft_state = FT_AWAIT_ACK;
	ft_is_cut = False;

	return 1;
}

/* "Transfer in Progress" pop-up. */

/* Pop up the "in progress" pop-up. */
static void
popup_progress()
{
	/* Initialize it. */
	if (progress_shell == (Widget)NULL)
		progress_popup_init();

	/* Pop it up. */
	popup_popup(progress_shell, XtGrabNone);
}

/* Initialize the "in progress" pop-up. */
static void
progress_popup_init()
{
	Widget progress_pop, from_label, to_label, cancel_button;

	/* Create the shell. */
	progress_shell = XtVaCreatePopupShell(
	    "ftProgressPopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(progress_shell, XtNpopupCallback, place_popup,
	    (XtPointer)CenterP);
	XtAddCallback(progress_shell, XtNpopupCallback,
	    progress_popup_callback, (XtPointer)NULL);

	/* Create a form structure to contain the other stuff */
	progress_pop = XtVaCreateManagedWidget(
	    "dialog", formWidgetClass, progress_shell,
	    NULL);

	/* Create the widgets. */
	from_label = XtVaCreateManagedWidget(
	    "fromLabel", labelWidgetClass, progress_pop,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	from_file = XtVaCreateManagedWidget(
	    "filename", labelWidgetClass, progress_pop,
	    XtNwidth, FILE_WIDTH,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, from_label,
	    XtNhorizDistance, 0,
	    NULL);
	match_dimension(from_label, from_file, XtNheight);

	to_label = XtVaCreateManagedWidget(
	    "toLabel", labelWidgetClass, progress_pop,
	    XtNfromVert, from_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    NULL);
	to_file = XtVaCreateManagedWidget(
	    "filename", labelWidgetClass, progress_pop,
	    XtNwidth, FILE_WIDTH,
	    XtNfromVert, from_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, to_label,
	    XtNhorizDistance, 0,
	    NULL);
	match_dimension(to_label, to_file, XtNheight);

	match_dimension(from_label, to_label, XtNwidth);

	waiting = XtVaCreateManagedWidget(
	    "waiting", labelWidgetClass, progress_pop,
	    XtNfromVert, to_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    XtNmappedWhenManaged, False,
	    NULL);

	ft_status = XtVaCreateManagedWidget(
	    "status", labelWidgetClass, progress_pop,
	    XtNfromVert, to_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    XtNresizable, True,
	    XtNmappedWhenManaged, False,
	    NULL);
	XtVaGetValues(ft_status, XtNlabel, &status_string, NULL);
	status_string = XtNewString(status_string);

	aborting = XtVaCreateManagedWidget(
	    "aborting", labelWidgetClass, progress_pop,
	    XtNfromVert, to_label,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    XtNmappedWhenManaged, False,
	    NULL);

	cancel_button = XtVaCreateManagedWidget(
	    ObjCancelButton, commandWidgetClass, progress_pop,
	    XtNfromVert, ft_status,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    NULL);
	XtAddCallback(cancel_button, XtNcallback, progress_cancel_callback,
	    NULL);
}

/* Callbacks for the "in progress" pop-up. */

/* In-progress pop-up popped up. */
static void
progress_popup_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	String hf, lf;

	XtVaGetValues(host_file, XtNstring, &hf, NULL);
	XtVaGetValues(local_file, XtNstring, &lf, NULL);

	XtVaSetValues(from_file, XtNlabel, receive_flag ? hf : lf, NULL);
	XtVaSetValues(to_file, XtNlabel, receive_flag ? lf : hf, NULL);

	switch (ft_state) {
	    case FT_AWAIT_ACK:
		XtUnmapWidget(ft_status);
		XtUnmapWidget(aborting);
		XtMapWidget(waiting);
		break;
	    case FT_RUNNING:
		XtUnmapWidget(waiting);
		XtUnmapWidget(aborting);
		XtMapWidget(ft_status);
		break;
	    case FT_ABORT_WAIT:
	    case FT_ABORT_SENT:
		XtUnmapWidget(waiting);
		XtUnmapWidget(ft_status);
		XtMapWidget(aborting);
		break;
	    default:
		break;
	}
}

/* In-progress "cancel" button. */
static void
progress_cancel_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (ft_state == FT_RUNNING) {
		ft_state = FT_ABORT_WAIT;
		XtUnmapWidget(waiting);
		XtUnmapWidget(ft_status);
		XtMapWidget(aborting);
	} else {
		/* Impatient user or hung host -- just clean up. */
		ft_complete(get_message("ftUserCancel"));
	}
}

/* "Overwrite existing?" pop-up. */

/* Pop up the "overwrite" pop-up. */
static void
popup_overwrite()
{
	/* Initialize it. */
	if (overwrite_shell == (Widget)NULL)
		overwrite_popup_init();

	/* Pop it up. */
	popup_popup(overwrite_shell, XtGrabExclusive);
}

/* Initialize the "overwrite" pop-up. */
static void
overwrite_popup_init()
{
	Widget overwrite_pop, overwrite_name, okay_button, cancel_button;
	String overwrite_string, label, lf;
	Dimension d;

	/* Create the shell. */
	overwrite_shell = XtVaCreatePopupShell(
	    "ftOverwritePopup", transientShellWidgetClass, toplevel,
	    NULL);
	XtAddCallback(overwrite_shell, XtNpopupCallback, place_popup,
	    (XtPointer)CenterP);
	XtAddCallback(overwrite_shell, XtNpopdownCallback, overwrite_popdown,
	    (XtPointer)NULL);

	/* Create a form structure to contain the other stuff */
	overwrite_pop = XtVaCreateManagedWidget(
	    "dialog", formWidgetClass, overwrite_shell,
	    NULL);

	/* Create the widgets. */
	overwrite_name = XtVaCreateManagedWidget(
	    "overwriteName", labelWidgetClass, overwrite_pop,
	    XtNvertDistance, MARGIN,
	    XtNhorizDistance, MARGIN,
	    XtNborderWidth, 0,
	    XtNresizable, True,
	    NULL);
	XtVaGetValues(overwrite_name, XtNlabel, &overwrite_string, NULL);
	XtVaGetValues(local_file, XtNstring, &lf, NULL);
	label = xs_buffer(overwrite_string, lf);
	XtVaSetValues(overwrite_name, XtNlabel, label, NULL);
	XtFree(label);
	XtVaGetValues(overwrite_name, XtNwidth, &d, NULL);
	if ((Dimension)(d + 20) < 400)
		d = 400;
	else
		d += 20;
	XtVaSetValues(overwrite_name, XtNwidth, d, NULL);
	XtVaGetValues(overwrite_name, XtNheight, &d, NULL);
	XtVaSetValues(overwrite_name, XtNheight, d + 10, NULL);

	okay_button = XtVaCreateManagedWidget(
	    ObjConfirmButton, commandWidgetClass, overwrite_pop,
	    XtNfromVert, overwrite_name,
	    XtNvertDistance, FAR_VGAP,
	    XtNhorizDistance, MARGIN,
	    NULL);
	XtAddCallback(okay_button, XtNcallback, overwrite_okay_callback,
	    NULL);

	cancel_button = XtVaCreateManagedWidget(
	    ObjCancelButton, commandWidgetClass, overwrite_pop,
	    XtNfromVert, overwrite_name,
	    XtNvertDistance, FAR_VGAP,
	    XtNfromHoriz, okay_button,
	    XtNhorizDistance, BUTTON_GAP,
	    NULL);
	XtAddCallback(cancel_button, XtNcallback, overwrite_cancel_callback,
	    NULL);
}

/* Overwrite "okay" button. */
static void
overwrite_okay_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(overwrite_shell);

	allow_overwrite = True;
	if (ft_start()) {
		XtPopdown(ft_shell);
		popup_progress();
	}
}

/* Overwrite "cancel" button. */
static void
overwrite_cancel_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtPopdown(overwrite_shell);
}

/* Overwrite pop-up popped down. */
static void
overwrite_popdown(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtDestroyWidget(overwrite_shell);
	overwrite_shell = (Widget)NULL;
}

/* External entry points called by ft_dft and ft_cut. */

/* Pop up a message, end the transfer. */
void
ft_complete(errmsg)
String errmsg;
{
	/* Close the local file. */
	if (ft_local_file != (FILE *)NULL && fclose(ft_local_file) < 0)
		popup_an_errno(errno, "close(%s)", ft_local_filename);
	ft_local_file = (FILE *)NULL;

	/* Clean up the state. */
	ft_state = FT_NONE;

	/* Pop down the in-progress shell. */
	XtPopdown(progress_shell);

	/* Pop up the text. */
	if (errmsg != (String)NULL) {
		/* Make sure the error message will fit on the display. */
		if (strlen(errmsg) > 50 && strchr(errmsg, '\n') == CN) {
			char *s = errmsg + 50;

			while (s > errmsg && *s != ' ')
				s--;
			if (s > errmsg)
				*s = '\n';
		}
		popup_an_error(errmsg);
		XtFree(errmsg);
	} else {
		struct timeval t1;
		double kbytes_sec;
		char *buf;

		(void) gettimeofday(&t1, (struct timezone *)NULL);
		kbytes_sec = (double)ft_length / 1024.0 /
			((double)(t1.tv_sec - t0.tv_sec) + 
			 (double)(t1.tv_usec - t0.tv_usec) / 1.0e6);
		buf = XtMalloc(256);
		(void) sprintf(buf, get_message("ftComplete"), ft_length,
		    kbytes_sec, ft_is_cut ? "CUT" : "DFT");
		popup_an_info(buf);
		XtFree(buf);
	}
}

/* Update the bytes-transferred count on the progress pop-up. */
void
ft_update_length()
{
	char text_string[80];

	/* Format the message */
	sprintf(text_string, status_string, ft_length);

	XtVaSetValues(ft_status, XtNlabel, text_string, NULL);
}

/* Process a transfer acknowledgement. */
void
ft_running(is_cut)
Boolean is_cut;
{
	if (ft_state == FT_AWAIT_ACK)
		ft_state = FT_RUNNING;
	ft_is_cut = is_cut;
	(void) gettimeofday(&t0, (struct timezone *)NULL);
	ft_length = 0;

	XtUnmapWidget(waiting);
	ft_update_length();
	XtMapWidget(ft_status);
}

/* Process a protocol-generated abort. */
void
ft_aborting()
{
	if (ft_state == FT_RUNNING || ft_state == FT_ABORT_WAIT) {
		ft_state = FT_ABORT_SENT;
		XtUnmapWidget(waiting);
		XtUnmapWidget(ft_status);
		XtMapWidget(aborting);
	}
}

/* Process a disconnect abort. */
void
ft_disconnected()
{
	if (ft_state != FT_NONE)
		ft_complete(get_message("ftDisconnected"));
}

/* Process an abort from no longer being in 3270 mode. */
void
ft_not3270()
{
	if (ft_state != FT_NONE)
		ft_complete(get_message("ftNot3270"));
}

/* Support functions for dialogs. */

/* Match one dimension of two widgets. */
static void
match_dimension(w1, w2, n)
Widget w1;
Widget w2;
String n;
{
	Dimension h1, h2;
	Dimension b1, b2;

	XtVaGetValues(w1, n, &h1, XtNborderWidth, &b1, NULL);
	XtVaGetValues(w2, n, &h2, XtNborderWidth, &b2, NULL);
	h1 += 2 * b1;
	h2 += 2 * b2;
	if (h1 > h2)
		XtVaSetValues(w2, n, h1 - (2 * b2), NULL);
	else if (h2 > h1)
		XtVaSetValues(w1, n, h2 - (2 * b1), NULL);
}

/* Apply a bitmap to a widget. */
static void
apply_bitmap(w, p)
Widget w;
Pixmap p;
{
	Dimension d1;
#if defined(TOGGLE_HACK) /*[*/
	String l, nl;
#endif /*]*/

	XtVaGetValues(w, XtNheight, &d1, NULL);
	if (d1 < 10)
		XtVaSetValues(w, XtNheight, 10, NULL);
#if defined(TOGGLE_HACK) /*[*/
	XtVaGetValues(w, XtNlabel, &l, NULL);
	nl = XtMalloc(strlen(l) + 6);
	if (p == diamond)
		(void) sprintf(nl, "+  %s", l);
	else if (p == dot)
		(void) sprintf(nl, "*  %s", l);
	else if (p == no_diamond)
		(void) sprintf(nl, "-  %s", l);
	else if (p == null)
		(void) sprintf(nl, "   %s", l);
	XtVaSetValues(w,
	    XtNlabel, nl,
	    NULL);
	XtFree(nl);
#else /*][*/
	XtVaSetValues(w, XtNleftBitmap, p, NULL);
#endif /*]*/
}

/* Flip a multi-valued toggle. */
static void
flip_toggles(toggle_list, w)
struct toggle_list *toggle_list;
Widget w;
{
	int i;

	/* Flip the widget w to on, and the rest to off. */
	for (i = 0; toggle_list->widgets[i] != (Widget)NULL; i++) {
		/* Process each widget in the list */
		mark_toggle(*(toggle_list->widgets+i),
		    (*(toggle_list->widgets+i) == w) ? diamond : no_diamond);
	}
}

/*
 * Callback for text source changes.  Edits the text to ensure it meets the
 * specified criteria.
 */
static void
text_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XawTextBlock b;		/* firstPos, length, ptr, format */
	static XawTextBlock nullb = { 0, 0, NULL };
	XawTextPosition pos = 0;
	int i;
	text_t t = *(text_t *)client_data;
	static Boolean called_back = False;

	if (called_back)
		return;
	else
		called_back = True;

	while (1) {
		Boolean replaced = False;

		XawTextSourceRead(w, pos, &b, 1024);
		if (b.length <= 0)
			break;
		nullb.format = b.format;
		for (i = 0; i < b.length; i++) {
			Boolean bad = False;
			char c = *(b.ptr + i);

			switch (t) {
			    case T_NUMERIC:
				/* Only numbers. */
				bad = !isdigit(c);
				break;
			    case T_HOSTFILE:
				/*
				 * Only printing characters and spaces; no
				 * leading or trailing blanks.
				 */
				bad = !isprint(c) || (!pos && !i && c == ' ');
				break;
			    case T_UNIXFILE:
				/* Only printing characters. */
				bad = !isgraph(c);
				break;
			}
			if (bad) {
				XawTextSourceReplace(w, pos + i, pos + i + 1,
				    &nullb);
				pos = 0;
				replaced = True;
				break;
			}
		}
		if (replaced)
			continue; /* rescan the same block */
		pos += b.length;
		if (b.length < 1024)
			break;
	}
	called_back = False;
}

/* Register widget sensitivity, based on zero to three Booleans. */
static void
register_sensitivity(w, bvar1, bval1, bvar2, bval2, bvar3, bval3)
Widget w;
Boolean *bvar1;
Boolean bval1;
Boolean *bvar2;
Boolean bval2;
Boolean *bvar3;
Boolean bval3;
{
	sr_t *s;
	Boolean f;

	/* Allocate a structure. */
	s = (sr_t *)XtMalloc(sizeof(sr_t));
	s->w = w;
	s->bvar1 = bvar1;
	s->bval1 = bval1;
	s->bvar2 = bvar2;
	s->bval2 = bval2;
	s->bvar3 = bvar3;
	s->bval3 = bval3;
	s->is_value = !strcmp(XtName(w), "value");
	s->has_focus = False;

	/* Link it onto the chain. */
	s->next = (sr_t *)NULL;
	if (sr_last != (sr_t *)NULL)
		sr_last->next = s;
	else
		sr = s;
	sr_last = s;

	/* Set up the initial widget sensitivity. */
	if (bvar1 == (Boolean *)NULL)
		f = True;
	else {
		f = (*bvar1 == bval1);
		if (bvar2 != (Boolean *)NULL)
			f &= (*bvar2 == bval2);
		if (bvar3 != (Boolean *)NULL)
			f &= (*bvar3 == bval3);
	}
	XtVaSetValues(w, XtNsensitive, f, NULL);
}

/* Scan the list of registered widgets for a sensitivity change. */
static void
check_sensitivity(bvar)
Boolean *bvar;
{
	sr_t *s;

	for (s = sr; s != (sr_t *)NULL; s = s->next) {
		if (s->bvar1 == bvar || s->bvar2 == bvar || s->bvar3 == bvar) {
			Boolean f;

			f = (s->bvar1 != (Boolean *)NULL &&
			     (*s->bvar1 == s->bval1));
			if (s->bvar2 != (Boolean *)NULL)
				f &= (*s->bvar2 == s->bval2);
			if (s->bvar3 != (Boolean *)NULL)
				f &= (*s->bvar3 == s->bval3);
			XtVaSetValues(s->w, XtNsensitive, f, NULL);

			/* If it is now insensitive, move the focus. */
			if (!f && s->is_value && s->has_focus)
				focus_next(s);
		}
	}
}

/* Move the input focus to the next sensitive value field. */
static void
focus_next(s)
sr_t *s;
{
	sr_t *t;
	Boolean sen;

	/* Defocus this widget. */
	s->has_focus = False;
	XawTextDisplayCaret(s->w, False);

	/* Search after. */
	for (t = s->next; t != (sr_t *)NULL; t = t->next) {
		if (t->is_value) {
			XtVaGetValues(t->w, XtNsensitive, &sen, NULL);
			if (sen)
				break;
		}
	}

	/* Wrap and search before. */
	if (t == (sr_t *)NULL)
		for (t = sr; t != s && t != (sr_t *)NULL; t = t->next) {
			if (t->is_value) {
				XtVaGetValues(t->w, XtNsensitive, &sen, NULL);
				if (sen)
					break;
			}
		}

	/* Move the focus. */
	if (t != (sr_t *)NULL && t != s) {
		t->has_focus = True;
		XawTextDisplayCaret(t->w, True);
		XtSetKeyboardFocus(ft_dialog, t->w);
	}
}

/* Dialog action procedures. */

/* Proceed to the next input field. */
void
dialog_next_action(w, event, parms, num_parms)
Widget w;
XEvent *event;
String *parms;
Cardinal *num_parms;
{
	sr_t *s;

	for (s = sr; s != (sr_t *)NULL; s = s->next) {
		if (s->w == w) {
			focus_next(s);
			return;
		}
	}
}

/* Set keyboard focus to an input field. */
void
dialog_focus_action(w, event, parms, num_parms)
Widget w;
XEvent *event;
String *parms;
Cardinal *num_parms;
{
	sr_t *s;

	/* Remove the focus from the widget that has it now. */
	for (s = sr; s != (sr_t *)NULL; s = s->next) {
		if (s->has_focus) {
			if (s->w == w)
				return;
			s->has_focus = False;
			XawTextDisplayCaret(s->w, False);
			break;
		}
	}

	/* Find this object. */
	for (s = sr; s != (sr_t *)NULL; s = s->next) {
		if (s->w == w)
			break;
	}
	if (s == (sr_t *)NULL)
		return;

	/* Give it the focus. */
	s->has_focus = True;
	XawTextDisplayCaret(w, True);
	XtSetKeyboardFocus(ft_dialog, w);
}
