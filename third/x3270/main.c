/*
 * Modifications Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 */

/*
 *	main.c
 *		A 3270 Terminal Emulator for X11
 *		Main proceudre.
 */

#include "globals.h"
#include <sys/wait.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <signal.h>
#include "appres.h"
#include "3270ds.h"
#include "resources.h"

#include "actionsc.h"
#include "ansic.h"
#include "ctlrc.h"
#include "ftc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "mainc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "savec.h"
#include "screenc.h"
#include "selectc.h"
#include "statusc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

#include "x3270.bm"
#include "wait.bm"

#define RECONNECT_MS	2000	/* 2 sec before reconnecting to host */

/* Externals */
#if defined(USE_APP_DEFAULTS) /*[*/
extern char     *app_defaults_version;
#else /*][*/
extern String	color_fallbacks[];
extern String	mono_fallbacks[];
#endif /*]*/

/* Globals */
Display        *display;
int             default_screen;
Window          root_window;
int             depth;
Widget          toplevel;
Widget          icon_shell;
XtAppContext    appcontext;
Atom            a_delete_me, a_save_yourself, a_3270, a_registry, a_iso8859,
		a_ISO8859, a_encoding, a_1, a_state;
char           *current_host;
char           *full_current_host;
unsigned short  current_port;
char           *efontname;
char		full_model_name[13] = "IBM-";
Boolean		keymap_changed = False;
char	       *model_name = &full_model_name[4];
int             model_num;
int             ov_cols, ov_rows;
char           *programname;
Pixmap          gray;
XrmDatabase     rdb;
Boolean         shifted = False;
struct trans_list *trans_list = (struct trans_list *)NULL;
static struct trans_list **last_trans = &trans_list;
AppRes          appres;
char           *charset = CN;
enum kp_placement kp_placement;
Pixmap          icon;
char           *user_title = CN;
char           *user_icon_name = CN;
struct font_list *font_list = (struct font_list *) NULL;
int             font_count = 0;
int		children = 0;

enum cstate	cstate = NOT_CONNECTED;
Boolean		ansi_host = False;
Boolean		std_ds_host = False;
Boolean		passthru_host = False;
Boolean		ever_3270 = False;
Boolean		exiting = False;
Boolean		reconnect_disabled = False;

/* Statics */
static int      net_sock = -1;
static XtInputId ns_read_id;
static XtInputId ns_exception_id;
static Boolean  reading = False;
static Boolean  excepting = False;
static Pixmap   inv_icon;
static Pixmap   wait_icon;
static Pixmap   inv_wait_icon;
static Boolean  icon_inverted = False;
static enum mcursor_state icon_cstate = NORMAL;
static struct font_list *font_last = (struct font_list *) NULL;
static Boolean  reconnecting = False;
static void     add_keymap();
static void     add_trans();
static void     peek_at_xevent();
static void     parse_font_list();
static XtErrorMsgHandler old_emh;
static void     trap_colormaps();
static Boolean  colormap_failure = False;
static void     parse_set_clear();

XrmOptionDescRec options[]= {
	{ OptActiveIcon,DotActiveIcon,	XrmoptionNoArg,		ResTrue },
	{ OptAplMode,	DotAplMode,	XrmoptionNoArg,		ResTrue },
	{ OptCharClass,	DotCharClass,	XrmoptionSepArg,	NULL },
	{ OptCharset,	DotCharset,	XrmoptionSepArg,	NULL },
	{ OptClear,	".xxx",		XrmoptionSkipArg,	NULL },
	{ OptColorScheme,DotColorScheme,XrmoptionSepArg,	NULL },
	{ OptDsTrace,	DotDsTrace,	XrmoptionNoArg,		ResTrue },
	{ OptEmulatorFont,DotEmulatorFont,XrmoptionSepArg,	NULL },
	{ OptExtended,	DotExtended,	XrmoptionNoArg,		ResTrue },
	{ OptIconName,	".iconName",	XrmoptionSepArg,	NULL },
	{ OptIconX,	".iconX",	XrmoptionSepArg,	NULL },
	{ OptIconY,	".iconY",	XrmoptionSepArg,	NULL },
	{ OptKeymap,	DotKeymap,	XrmoptionSepArg,	NULL },
	{ OptKeypadOn,	DotKeypadOn,	XrmoptionNoArg,		ResTrue },
	{ OptM3279,	DotM3279,	XrmoptionNoArg,		ResTrue },
	{ OptModel,	DotModel,	XrmoptionSepArg,	NULL },
	{ OptMono,	DotMono,	XrmoptionNoArg,		ResTrue },
	{ OptNoScrollBar,DotScrollBar,	XrmoptionNoArg,		ResFalse },
	{ OptOnce,	DotOnce,	XrmoptionNoArg,		ResTrue },
	{ OptOversize,	DotOversize,	XrmoptionSepArg,	NULL },
	{ OptPort,	DotPort,	XrmoptionSepArg,	NULL },
	{ OptReconnect,	DotReconnect,	XrmoptionNoArg,		ResTrue },
	{ OptSaveLines,	DotSaveLines,	XrmoptionSepArg,	NULL },
	{ OptScripted,	DotScripted,	XrmoptionNoArg,		ResTrue },
	{ OptScrollBar,	DotScrollBar,	XrmoptionNoArg,		ResTrue },
	{ OptSet,	".xxx",		XrmoptionSkipArg,	NULL },
	{ OptTermName,	DotTermName,	XrmoptionSepArg,	NULL },
};
int num_options = XtNumber(options);

#define offset(field)		XtOffset(AppResptr, field)
#define toggle_offset(index)	offset(toggle[index].value)
static XtResource resources[] = {
	{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  offset(foreground), XtRString, "XtDefaultForeground" },
	{ XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	  offset(background), XtRString, "XtDefaultBackground" },
	{ ResColorBackground, ClsColorBackground, XtRString, sizeof(String),
	  offset(colorbg_name), XtRString, "black" },
	{ ResSelectBackground, ClsSelectBackground, XtRString, sizeof(String),
	  offset(selbg_name), XtRString, "dim gray" },
	{ ResNormalColor, ClsNormalColor, XtRString, sizeof(String),
	  offset(normal_name), XtRString, "green" },
	{ ResInputColor, ClsInputColor, XtRString, sizeof(String),
	  offset(select_name), XtRString, "orange" },
	{ ResBoldColor, ClsBoldColor, XtRString, sizeof(String),
	  offset(bold_name), XtRString, "cyan" },
	{ ResCursorColor, ClsCursorColor, XtRString, sizeof(String),
	  offset(cursor_color_name), XtRString, "red" },
	{ ResMono, ClsMono, XtRBoolean, sizeof(Boolean),
	  offset(mono), XtRString, ResFalse },
	{ ResExtended, ClsExtended, XtRBoolean, sizeof(Boolean),
	  offset(extended), XtRString, ResFalse },
	{ ResM3279, ClsM3279, XtRBoolean, sizeof(Boolean),
	  offset(m3279), XtRString, ResTrue },
	{ ResKeypad, ClsKeypad, XtRString, sizeof(String),
	  offset(keypad), XtRString, KpRight },
	{ ResKeypadOn, ClsKeypadOn, XtRBoolean, sizeof(Boolean),
	  offset(keypad_on), XtRString, ResFalse },
	{ ResInvertKeypadShift, ClsInvertKeypadShift, XtRBoolean, sizeof(Boolean),
	  offset(invert_kpshift), XtRString, ResFalse },
	{ ResSaveLines, ClsSaveLines, XtRInt, sizeof(int),
	  offset(save_lines), XtRString, "64" },
	{ ResMenuBar, ClsMenuBar, XtRBoolean, sizeof(Boolean),
	  offset(menubar), XtRString, ResTrue },
	{ ResActiveIcon, ClsActiveIcon, XtRBoolean, sizeof(Boolean),
	  offset(active_icon), XtRString, ResFalse },
	{ ResLabelIcon, ClsLabelIcon, XtRBoolean, sizeof(Boolean),
	  offset(label_icon), XtRString, ResFalse },
	{ ResKeypadBackground, ClsKeypadBackground, XtRString, sizeof(String),
	  offset(keypadbg_name), XtRString, "grey70" },
	{ ResEmulatorFont, ClsEmulatorFont, XtRString, sizeof(char *),
	  offset(efontname), XtRString, 0 },
	{ ResAplFont, ClsAplFont, XtRString, sizeof(char *),
	  offset(afontname), XtRString, 0 },
	{ ResVisualBell, ClsVisualBell, XtRBoolean, sizeof(Boolean),
	  offset(visual_bell), XtRString, ResFalse },
	{ ResAplMode, ClsAplMode, XtRBoolean, sizeof(Boolean),
	  offset(apl_mode), XtRString, ResFalse },
	{ ResOnce, ClsOnce, XtRBoolean, sizeof(Boolean),
	  offset(once), XtRString, ResFalse },
	{ ResScripted, ClsScripted, XtRBoolean, sizeof(Boolean),
	  offset(scripted), XtRString, ResFalse },
	{ ResModifiedSel, ClsModifiedSel, XtRBoolean, sizeof(Boolean),
	  offset(modified_sel), XtRString, ResFalse },
	{ ResUseCursorColor, ClsUseCursorColor, XtRBoolean, sizeof(Boolean),
	  offset(use_cursor_color), XtRString, ResFalse },
	{ ResReconnect, ClsReconnect, XtRBoolean, sizeof(Boolean),
	  offset(reconnect), XtRString, ResFalse },
	{ ResDoConfirms, ClsDoConfirms, XtRBoolean, sizeof(Boolean),
	  offset(do_confirms), XtRString, ResTrue },
	{ ResNumericLock, ClsNumericLock, XtRBoolean, sizeof(Boolean),
	  offset(numeric_lock), XtRString, ResFalse },
	{ ResAllowResize, ClsAllowResize, XtRBoolean, sizeof(Boolean),
	  offset(allow_resize), XtRString, ResTrue },
	{ ResSecure, ClsSecure, XtRBoolean, sizeof(Boolean),
	  offset(secure), XtRString, ResFalse },
	{ ResNoOther, ClsNoOther, XtRBoolean, sizeof(Boolean),
	  offset(no_other), XtRString, ResFalse },
	{ ResOerrLock, ClsOerrLock, XtRBoolean, sizeof(Boolean),
	  offset(oerr_lock), XtRString, ResTrue },
	{ ResTypeahead, ClsTypeahead, XtRBoolean, sizeof(Boolean),
	  offset(typeahead), XtRString, ResTrue },
	{ ResDebugTracing, ClsDebugTracing, XtRBoolean, sizeof(Boolean),
	  offset(debug_tracing), XtRString, ResTrue },
	{ ResAttnLock, ClsAttnLock, XtRBoolean, sizeof(Boolean),
	  offset(attn_lock), XtRString, ResFalse },
	{ ResBellVolume, ClsBellVolume, XtRInt, sizeof(int),
	  offset(bell_volume), XtRString, "0" },
	{ ResOversize, ClsOversize, XtRString, sizeof(char *),
	  offset(oversize), XtRString, 0 },
	{ ResCharClass, ClsCharClass, XtRString, sizeof(char *),
	  offset(char_class), XtRString, 0 },
	{ ResModel, ClsModel, XtRString, sizeof(char *),
	  offset(model), XtRString,
#if defined(RESTRICT_3279) /*[*/
	  "3279-3-E"
#else /*][*/
	  "3279-4-E"
#endif /*]*/
	  },
	{ ResKeymap, ClsKeymap, XtRString, sizeof(char *),
	  offset(key_map), XtRString, 0 },
	{ ResComposeMap, ClsComposeMap, XtRString, sizeof(char *),
	  offset(compose_map), XtRString, "latin1" },
	{ ResHostsFile, ClsHostsFile, XtRString, sizeof(char *),
	  offset(hostsfile), XtRString, 0 },
	{ ResPort, ClsPort, XtRString, sizeof(char *),
	  offset(port), XtRString, "telnet" },
	{ ResCharset, ClsCharset, XtRString, sizeof(char *),
	  offset(charset), XtRString, "bracket" },
	{ ResTermName, ClsTermName, XtRString, sizeof(char *),
	  offset(termname), XtRString, 0 },
	{ ResDebugFont, ClsDebugFont, XtRString, sizeof(char *),
	  offset(debug_font), XtRString, "3270d" },
	{ ResFontList, ClsFontList, XtRString, sizeof(char *),
	  offset(font_list), XtRString, 0 },
	{ ResIconFont, ClsIconFont, XtRString, sizeof(char *),
	  offset(icon_font), XtRString, "nil2" },
	{ ResIconLabelFont, ClsIconLabelFont, XtRString, sizeof(char *),
	  offset(icon_label_font), XtRString, "8x13" },
	{ ResBaselevelTranslations, ClsBaselevelTranslations, XtRTranslationTable,
	    sizeof(XtTranslations),
	  offset(base_translations), XtRTranslationTable, NULL },
	{ ResWaitCursor, ClsWaitCursor, XtRCursor, sizeof(Cursor),
	  offset(wait_mcursor), XtRString, "watch" },
	{ ResLockedCursor, ClsLockedCursor, XtRCursor, sizeof(Cursor),
	  offset(locked_mcursor), XtRString, "X_cursor" },
	{ ResMacros, ClsMacros, XtRString, sizeof(char *),
	  offset(macros), XtRString, 0 },
	{ ResTraceDir, ClsTraceDir, XtRString, sizeof(char *),
	  offset(trace_dir), XtRString, "/tmp" },
	{ ResColorScheme, ClsColorScheme, XtRString, sizeof(String),
	  offset(color_scheme), XtRString, "default" },
	{ ResFtCommand, ClsFtCommand, XtRString, sizeof(String),
	  offset(ft_command), XtRString, "ind$file" },
	{ ResModifiedSelColor, ClsModifiedSelColor, XtRInt, sizeof(int),
	  offset(modified_sel_color), XtRString, "10" },
	{ ResMonoCase, ClsMonoCase, XtRBoolean, sizeof(Boolean),
	  toggle_offset(MONOCASE), XtRString, ResFalse },
	{ ResAltCursor, ClsAltCursor, XtRBoolean, sizeof(Boolean),
	  toggle_offset(ALT_CURSOR), XtRString, ResFalse },
	{ ResCursorBlink, ClsCursorBlink, XtRBoolean, sizeof(Boolean),
	  toggle_offset(CURSOR_BLINK), XtRString, ResFalse },
	{ ResShowTiming, ClsShowTiming, XtRBoolean, sizeof(Boolean),
	  toggle_offset(SHOW_TIMING), XtRString, ResFalse },
	{ ResCursorPos, ClsCursorPos, XtRBoolean, sizeof(Boolean),
	  toggle_offset(CURSOR_POS), XtRString, ResTrue },
	{ ResDsTrace, ClsDsTrace, XtRBoolean, sizeof(Boolean),
	  toggle_offset(DS_TRACE), XtRString, ResFalse },
	{ ResScrollBar, ClsScrollBar, XtRBoolean, sizeof(Boolean),
	  toggle_offset(SCROLL_BAR), XtRString, ResFalse },
	{ ResLineWrap, ClsLineWrap, XtRBoolean, sizeof(Boolean),
	  toggle_offset(LINE_WRAP), XtRString, ResTrue },
	{ ResBlankFill, ClsBlankFill, XtRBoolean, sizeof(Boolean),
	  toggle_offset(BLANK_FILL), XtRString, ResFalse },
	{ ResScreenTrace, ClsScreenTrace, XtRBoolean, sizeof(Boolean),
	  toggle_offset(SCREEN_TRACE), XtRString, ResFalse },
	{ ResEventTrace, ClsEventTrace, XtRBoolean, sizeof(Boolean),
	  toggle_offset(EVENT_TRACE), XtRString, ResFalse },
	{ ResMarginedPaste, ClsMarginedPaste, XtRBoolean, sizeof(Boolean),
	  toggle_offset(MARGINED_PASTE), XtRString, ResFalse },
	{ ResRectangleSelect, ClsRectangleSelect, XtRBoolean, sizeof(Boolean),
	  toggle_offset(RECTANGLE_SELECT), XtRString, ResFalse },

	{ ResIcrnl, ClsIcrnl, XtRBoolean, sizeof(Boolean),
	  offset(icrnl), XtRString, ResTrue },
	{ ResInlcr, ClsInlcr, XtRBoolean, sizeof(Boolean),
	  offset(inlcr), XtRString, ResFalse },
	{ ResErase, ClsErase, XtRString, sizeof(char *),
	  offset(erase), XtRString, "^?" },
	{ ResKill, ClsKill, XtRString, sizeof(char *),
	  offset(kill), XtRString, "^U" },
	{ ResWerase, ClsWerase, XtRString, sizeof(char *),
	  offset(werase), XtRString, "^W" },
	{ ResRprnt, ClsRprnt, XtRString, sizeof(char *),
	  offset(rprnt), XtRString, "^R" },
	{ ResLnext, ClsLnext, XtRString, sizeof(char *),
	  offset(lnext), XtRString, "^V" },
	{ ResIntr, ClsIntr, XtRString, sizeof(char *),
	  offset(intr), XtRString, "^C" },
	{ ResQuit, ClsQuit, XtRString, sizeof(char *),
	  offset(quit), XtRString, "^\\" },
	{ ResEof, ClsEof, XtRString, sizeof(char *),
	  offset(eof), XtRString, "^D" },

#if defined(USE_APP_DEFAULTS) /*[*/
	{ ResAdVersion, ClsAdVersion, XtRString, sizeof(char *),
	  offset(ad_version), XtRString, 0 },
#endif /*]*/
};
#undef offset
#undef toggle_offset

/* Fallback resources. */
#if defined(USE_APP_DEFAULTS) /*[*/
static String fallbacks[] = {
	"*adVersion:	fallback",
	NULL
};
#else /*][*/
static String *fallbacks = color_fallbacks;
#endif /*]*/

struct toggle_name toggle_names[N_TOGGLES] = {
	{ ResMonoCase,        MONOCASE },
	{ ResAltCursor,       ALT_CURSOR },
	{ ResCursorBlink,     CURSOR_BLINK },
	{ ResShowTiming,      SHOW_TIMING },
	{ ResCursorPos,       CURSOR_POS },
	{ ResDsTrace,         DS_TRACE },
	{ ResScrollBar,       SCROLL_BAR },
	{ ResLineWrap,        LINE_WRAP },
	{ ResBlankFill,       BLANK_FILL },
	{ ResScreenTrace,     SCREEN_TRACE },
	{ ResEventTrace,      EVENT_TRACE },
	{ ResMarginedPaste,   MARGINED_PASTE },
	{ ResRectangleSelect, RECTANGLE_SELECT }
};


static void
usage(msg)
char *msg;
{
	if (msg != CN)
		XtWarning(msg);
	xs_error("Usage: %s [options] [[ps:]hostname [port]]", programname);
}

static void
no_minus(arg)
char *arg;
{
	if (arg[0] == '-')
	    usage(xs_buffer("Unknown or incomplete option: %s", arg));
}

void
main(argc, argv)
int	argc;
char	*argv[];
{
	char	*dname;
	char	*buf;
	char	*ef, *emsg;
	int	i;
	Atom	protocols[2];
	char	*cl_hostname = CN;
	int	ovc, ovr;
	char	junk;
	int	sl;
	char	*km;

	/* Figure out who we are */
	programname = strrchr(argv[0], '/');
	if (programname)
		++programname;
	else
		programname = argv[0];

	/* Save a copy of the command-line args for merging later. */
	save_args(argc, argv);

#if !defined(USE_APP_DEFAULTS) /*[*/
	/*
	 * Figure out which fallbacks to use, based on the "-mono"
	 * switch on the command line, and the depth of the display.
	 */
	dname = CN;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-mono"))
			fallbacks = mono_fallbacks;
		else if (!strcmp(argv[i], "-display") && argc > i)
			dname = argv[i+1];
	}
	display = XOpenDisplay(dname);
	if (display == (Display *)NULL)
		XtError("Can't open display");
	if (DefaultDepthOfScreen(XDefaultScreenOfDisplay(display)) == 1)
		fallbacks = mono_fallbacks;
	XCloseDisplay(display);
#endif /*]*/

	/* Initialize. */
	toplevel = XtVaAppInitialize(
	    &appcontext,
#if defined(USE_APP_DEFAULTS) /*[*/
	    "X3270",
#else /*][*/
	    "X3270xad",	/* explicitly _not_ X3270 */
#endif /*]*/
	    options, num_options,
	    &argc, argv,
	    fallbacks,
	    XtNinput, True,
	    XtNallowShellResize, False,
	    NULL);
	display = XtDisplay(toplevel);
	rdb = XtDatabase(display);

	/* Merge in the profile. */
	merge_profile(&rdb);

	old_emh = XtAppSetWarningMsgHandler(appcontext,
	    (XtErrorMsgHandler)trap_colormaps);
	XtGetApplicationResources(toplevel, (XtPointer)&appres, resources,
	    XtNumber(resources), 0, 0);
	(void) XtAppSetWarningMsgHandler(appcontext, old_emh);

#if defined(USE_APP_DEFAULTS) /*[*/
	/* Check the app-defaults version. */
	if (!appres.ad_version)
		XtError("Outdated app-defaults file");
	else if (!strcmp(appres.ad_version, "fallback"))
		XtError("No app-defaults file");
	else if (strcmp(appres.ad_version, app_defaults_version))
		xs_error("app-defaults version mismatch: want %s, got %s",
		    app_defaults_version, appres.ad_version);
#endif /*]*/

	/* Pick out -set and -clear toggle options. */
	parse_set_clear(&argc, argv);

	/* Verify command-line syntax. */
	switch (argc) {
	    case 1:
		break;
	    case 2:
		no_minus(argv[1]);
		cl_hostname = argv[1];
		break;
	    case 3:
		no_minus(argv[1]);
		no_minus(argv[2]);
		cl_hostname = xs_buffer("%s %s", argv[1], argv[2]);
		break;
	    default:
		usage(CN);
		break;
	}

	default_screen = DefaultScreen(display);
	root_window = RootWindow(display, default_screen);
	depth = DefaultDepthOfScreen(XtScreen(toplevel));

	/* Sort out model, color and extended data stream modes. */
	appres.model = XtNewString(appres.model);
	if (!strncmp(appres.model, "3278", 4)) {
		appres.m3279 = False;
		appres.model = appres.model + 4;
		if (appres.model[0] == '-')
			++appres.model;
	} else if (!strncmp(appres.model, "3279", 4)) {
		appres.m3279 = True;
		appres.model = appres.model + 4;
		if (appres.model[0] == '-')
			++appres.model;
	}
	sl = strlen(appres.model);
	if (sl && (appres.model[sl-1] == 'E' || appres.model[sl-1] == 'e')) {
		appres.extended = True;
		appres.model[sl-1] = '\0';
		sl--;
		if (sl && appres.model[sl-1] == '-')
			appres.model[sl-1] = '\0';
	}
	if (!appres.model[0])
#if defined(RESTRICT_3279) /*[*/
		appres.model = "3";
#else /*][*/
		appres.model = "4";
#endif /*]*/
	if (appres.m3279)
		appres.extended = True;
	if (depth <= 1 || colormap_failure)
		appres.mono = True;
	if (appres.mono) {
		appres.use_cursor_color = False;
		appres.m3279 = False;
	}
	if (!appres.extended)
		appres.oversize = CN;

	a_delete_me = XInternAtom(display, "WM_DELETE_WINDOW", False);
	a_save_yourself = XInternAtom(display, "WM_SAVE_YOURSELF", False);
	a_3270 = XInternAtom(display, "3270", False);
	a_registry = XInternAtom(display, "CHARSET_REGISTRY", False);
	a_ISO8859 = XInternAtom(display, "ISO8859", False);
	a_iso8859 = XInternAtom(display, "iso8859", False);
	a_encoding = XInternAtom(display, "CHARSET_ENCODING", False);
	a_1 = XInternAtom(display, "1", False);
	a_state = XInternAtom(display, "WM_STATE", False);

	XtAppAddActions(appcontext, actions, actioncount);

	/*
	 * Font stuff.  Try the "emulatorFont" resource; if that isn't right,
	 * complain but keep going.  Try the "font" resource next, but don't
	 * say anything if it's wrong.  Finally, fall back to "fixed", and
	 * die if that won't work.
	 */
	parse_font_list();
	if (appres.apl_mode) {
		if ((appres.efontname = appres.afontname) == NULL) {
			xs_warning("No %s resource, ignoring APL mode",
			    ResAplFont);
			appres.apl_mode = False;
		}
	}
	if (!appres.efontname)
		appres.efontname = font_list->font;
	ef = appres.efontname;
	if ((emsg = load_fixed_font(ef, efontinfo))) {
		xs_warning("%s \"%s\" %s, using default font",
		    ResEmulatorFont, ef, emsg);
		if (appres.apl_mode) {
			XtWarning("Ignoring APL mode");
			appres.apl_mode = False;
		}
		ef = font_list->font;
		if (strcmp(ef, appres.efontname))
			(void) load_fixed_font(ef, efontinfo);
	}
	if (*efontinfo == NULL) {
		ef = "fixed";
		if ((emsg = load_fixed_font(ef, efontinfo)))
			xs_error("default %s \"%s\" %s", ResEmulatorFont, ef,
			    emsg);
	}

	set_font_globals(*efontinfo, ef);

#if defined(RESTRICT_3279) /*[*/
	if (appres.m3279 && !strcmp(appres.model, "4"))
		appres.model = "3";
#endif /*]*/
	if (!appres.extended || appres.oversize == CN ||
	    sscanf(appres.oversize, "%dx%d%c", &ovc, &ovr, &junk) != 2) {
		ovc = 0;
		ovr = 0;
	}
	set_rows_cols(atoi(appres.model), ovc, ovr);
	if (appres.termname != CN)
		termtype = appres.termname;
	else
		termtype = full_model_name;
	hostfile_init();

	km = appres.key_map;
	if (km == CN) {
		km = (char *)getenv("KEYMAP");
		if (km == CN)
			km = (char *)getenv("KEYBD");
	}
	setup_keymaps(km, False);

	if (appres.apl_mode) {
		appres.compose_map = Apl;
		appres.charset = Apl;
	}
	if (appres.charset != NULL) {
		buf = xs_buffer("%s.%s", ResCharset, appres.charset);
		charset = get_resource(buf);
		XtFree(buf);
		if (charset == NULL)
			xs_warning("Cannot find charset \"%s\"", appres.charset);
	}

	if (!strcmp(appres.keypad, KpLeft))
		kp_placement = kp_left;
	else if (!strcmp(appres.keypad, KpRight))
		kp_placement = kp_right;
	else if (!strcmp(appres.keypad, KpBottom))
		kp_placement = kp_bottom;
	else if (!strcmp(appres.keypad, KpIntegral))
		kp_placement = kp_integral;
	else
		xs_error("Unknown value for %s", ResKeypad);

	icon = XCreateBitmapFromData(display, root_window,
	    (char *) x3270_bits, x3270_width, x3270_height);

	if (appres.active_icon) {
		Dimension iw, ih;

		aicon_font_init();
		aicon_size(&iw, &ih);
		icon_shell =  XtVaAppCreateShell(
		    "x3270icon",
		    "X3270",
		    overrideShellWidgetClass,
		    display,
		    XtNwidth, iw,
		    XtNheight, ih,
		    XtNmappedWhenManaged, False,
		    NULL);
		XtRealizeWidget(icon_shell);
		XtVaSetValues(toplevel,
		    XtNiconWindow, XtWindow(icon_shell),
		    NULL);
	} else {
		for (i = 0; i < sizeof(x3270_bits); i++)
			x3270_bits[i] = ~x3270_bits[i];
		inv_icon = XCreateBitmapFromData(display, root_window,
		    (char *) x3270_bits, x3270_width, x3270_height);
		wait_icon = XCreateBitmapFromData(display, root_window,
		    (char *) wait_bits, wait_width, wait_height);
		for (i = 0; i < sizeof(wait_bits); i++)
			wait_bits[i] = ~wait_bits[i];
		inv_wait_icon = XCreateBitmapFromData(display, root_window,
		    (char *) wait_bits, wait_width, wait_height);
		XtVaSetValues(toplevel,
		    XtNiconPixmap, icon,
		    XtNiconMask, icon,
		    NULL);
	}

	/*
	 * If no hostname is specified on the command line, ignore certain
	 * options.
	 */
	if (argc <= 1) {
		appres.once = False;
		appres.reconnect = False;
	}

	if (appres.char_class != CN)
		reclass(appres.char_class);
	screen_init(False, True, True);
	if (appres.active_icon) {
		XtVaSetValues(icon_shell,
		    XtNbackground, appres.mono ? appres.background
					       : colorbg_pixel,
		    NULL);
	}
	protocols[0] = a_delete_me;
	protocols[1] = a_save_yourself;
	XSetWMProtocols(display, XtWindow(toplevel), protocols, 2);

	/* Save the command line. */
	save_init(argc, argv[1], argv[2]);

	/* Make sure we don't fall over any SIGPIPEs. */
	(void) signal(SIGPIPE, SIG_IGN);

	/* Respect the user's title wishes. */
	user_title = get_resource(XtNtitle);
	user_icon_name = get_resource(XtNiconName);
	if (user_icon_name)
		set_aicon_label(user_icon_name);

	/* Handle initial toggle settings. */
	if (!appres.debug_tracing) {
		appres.toggle[DS_TRACE].value = False;
		appres.toggle[EVENT_TRACE].value = False;
	}
	initialize_toggles();

	/* Connect to the host. */
	if (argc <= 1)
		x_disconnect(False);
	else if (x_connect(cl_hostname) < 0)
		x_disconnect(True);

	/* Prepare to run a peer script. */
	peer_script_init();

	/* Process X events forever. */
	while (1) {
		XEvent		event;

		while (XtAppPending(appcontext) & (XtIMXEvent | XtIMTimer)) {
			if (XtAppPeekEvent(appcontext, &event))
				peek_at_xevent(&event);
			XtAppProcessEvent(appcontext,
			    XtIMXEvent | XtIMTimer);
		}
		screen_disp();
		XtAppProcessEvent(appcontext, XtIMAll);

		if (children && waitpid(0, (int *)0, WNOHANG) > 0)
			--children;
	}
}


static char *
strip_qual(s, ansi, std_ds, passthru)
char *s;
Boolean *ansi;
Boolean *std_ds;
Boolean *passthru;
{
	for (;;) {
		if (!strncmp(s, "a:", 2) || !strncmp(s, "A:", 2)) {
			if (ansi != (Boolean *) NULL)
				*ansi = True;
			s += 2;
			continue;
		}
		if (!strncmp(s, "s:", 2) || !strncmp(s, "S:", 2)) {
			if (std_ds != (Boolean *) NULL)
				*std_ds = True;
			s += 2;
			continue;
		}
		if (!strncmp(s, "p:", 2) || !strncmp(s, "P:", 2)) {
			if (passthru != (Boolean *) NULL)
				*passthru = True;
			s += 2;
			continue;
		}
		break;
	}
	if (*s)
		return s;
	else
		return CN;
}


/*
 * Network connect/disconnect operations, combined with X input operations.
 *
 * Returns 0 for success, -1 for error.
 * Sets 'current_host' and 'full_current_host' as side-effects.
 */
int
x_connect(n)
char	*n;
{
	char nb[2048];		/* name buffer */
	char *s;		/* temporary */
	char *target_name;
	char *ps = CN;
	char *port;
	Boolean pending;

	if (CONNECTED || reconnecting)
		return 0;

	/* Skip leading blanks. */
	while (*n == ' ')
		n++;
	if (!*n)
		return -1;

	/* Save in a modifiable buffer. */
	(void) strcpy(nb, n);

	/* Strip trailing blanks. */
	s = nb + strlen(nb) - 1;
	while (*s == ' ')
		*s-- = '\0';

	/* Strip off and remember leading qualifiers. */
	ansi_host = False;
	std_ds_host = False;
	passthru_host = False;
	if (!(s = strip_qual(nb, &ansi_host, &std_ds_host,
	    &passthru_host)))
		return -1;

	/* Look up the name in the hosts file. */
	if (hostfile_lookup(s, &target_name, &ps)) {

		/* Rescan for qualifiers. */
		(void) strcpy(nb, target_name);
		if (!(s = strip_qual(nb, &ansi_host, &std_ds_host,
		    &passthru_host)))
			return -1;
	}

	/* Split off any port number. */
	if ((port = strchr(s, ' '))) {
		*port++ = '\0';
		while (*port == ' ')
			port++;
	} else
		port = appres.port;

	/* Store the name in globals, even if we fail. */
	if (n != full_current_host) {
		if (full_current_host)
			XtFree(full_current_host);
		full_current_host = XtNewString(n);
	}
	current_host = strip_qual(full_current_host, (Boolean *) NULL,
	    (Boolean *) NULL, (Boolean *) NULL);

	/* Attempt contact. */
	ever_3270 = False;
	net_sock = net_connect(s, port, &pending);
	if (net_sock < 0) {
		reconnect_disabled = True;
		menubar_connect();
		return -1;
	}

	/* Success. */

	/* Set pending string. */
	if (ps != CN)
		login_macro(ps);

	/* Set clock ticking and set state. */
	if (pending) {
		cstate = PENDING;
		ticking_start(True);
	} else
		cstate = CONNECTED_INITIAL;

	/* Prepare Xt for I/O. */
	ns_exception_id = XtAppAddInput(appcontext, net_sock,
	    (XtPointer)XtInputExceptMask, net_exception, PN);
	excepting = True;
	ns_read_id = XtAppAddInput(appcontext, net_sock,
	    (XtPointer) XtInputReadMask, net_input, PN);
	reading = True;

	/* Tell the world. */
	menubar_connect();
	kybd_connect();
	screen_connect();
	relabel();
	ansi_init();

	return 0;
}
#undef A_COLON
#undef P_COLON

/*
 * Called from timer to attempt an automatic reconnection.
 */
/*ARGSUSED*/
static void
try_reconnect(data, id)
XtPointer data;
XtIntervalId *id;
{
	reconnecting = False;
	x_reconnect();
}

/*
 * Reconnect to the last host.
 */
void
x_reconnect()
{
	if (reconnecting || !current_host || CONNECTED || HALF_CONNECTED)
		return;
	if (x_connect(full_current_host) >= 0)
		reconnecting = False;
}

void
x_disconnect(disable)
Boolean disable;
{
	reconnect_disabled = disable;
	ticking_stop();
	if (HALF_CONNECTED)
		status_untiming();
	if (CONNECTED || HALF_CONNECTED) {
		if (reading) {
			XtRemoveInput(ns_read_id);
			reading = False;
		}
		if (excepting) {
			XtRemoveInput(ns_exception_id);
			excepting = False;
		}
		net_disconnect();
		net_sock = -1;
		if (CONNECTED && appres.once) {
			if (error_popup_visible)
				exiting = True;
			else {
				x3270_exit(0);
				return;
			}
		}

		/* Schedule an automatic reconnection. */
		if (CONNECTED && appres.reconnect && !reconnecting &&
		    !reconnect_disabled) {
			reconnecting = True;
			(void) XtAppAddTimeOut(appcontext, RECONNECT_MS,
			    try_reconnect, PN);
		}

		/*
		 * Remember a disconnect from ANSI mode, to keep screen tracing
		 * in sync.
		 */
		if (IN_ANSI && toggled(SCREEN_TRACE))
			trace_ansi_disc();

		cstate = NOT_CONNECTED;

		/* Cancel all login strings, macros, and scripts. */
		sms_cancel_login();

		/* Cancel any file transfer in progress. */
		ft_disconnected();
	}
	menubar_connect();
	kybd_connect();
	screen_connect();
	sms_continue();	/* in case of Wait pending */
	relabel();
}

void
x_in3270(now3270)
Boolean now3270;
{
	if (now3270) {		/* ANSI -> 3270 */
		cstate = CONNECTED_3270;
		ever_3270 = True;
		menubar_newmode();
		kybd_connect();
		screen_connect();
		relabel();
		sms_continue();	/* in case of Wait pending */
	} else {	/* 3270 -> ANSI */
		cstate = CONNECTED_INITIAL;
		ever_3270 = False;
		ticking_stop();
		menubar_newmode();
		kybd_connect();
		screen_connect();
		relabel();
		ansi_init();
		ft_not3270();
	}
}

void
x_connected()
{
	cstate = CONNECTED_INITIAL;
	ticking_stop();
	menubar_connect();
	kybd_connect();
	screen_connect();
	ansi_init();
	relabel();
	sms_continue();	/* in case of Wait pending */
}


/*
 * Called when an exception is received to disable further exceptions.
 */
void
x_except_off()
{
	if (excepting) {
		XtRemoveInput(ns_exception_id);
		excepting = False;
	}
}

/*
 * Called when exception processing is complete to re-enable exceptions.
 * This includes removing and restoring reading, so the exceptions are always
 * processed first.
 */
void
x_except_on()
{
	if (excepting)
		return;
	if (reading)
		XtRemoveInput(ns_read_id);
	ns_exception_id = XtAppAddInput(appcontext, net_sock,
	    (XtPointer) XtInputExceptMask, net_exception, PN);
	excepting = True;
	if (reading)
		ns_read_id = XtAppAddInput(appcontext, net_sock,
		    (XtPointer) XtInputReadMask, net_input, PN);
}

void
relabel()
{
	char *title;
	char icon_label[8];

	if (user_title && user_icon_name)
		return;
	title = XtMalloc(10 + ((PCONNECTED || appres.reconnect) ? strlen(current_host) : 0));
	if (PCONNECTED || appres.reconnect) {
		(void) sprintf(title, "x3270-%d%s %s", model_num,
		    (IN_ANSI ? "A" : ""), current_host);
		if (!user_title)
			XtVaSetValues(toplevel, XtNtitle, title, NULL);
		if (!user_icon_name)
			XtVaSetValues(toplevel, XtNiconName, current_host, NULL);
		set_aicon_label(current_host);
	} else {
		(void) sprintf(title, "x3270-%d", model_num);
		(void) sprintf(icon_label, "x3270-%d", model_num);
		if (!user_title)
			XtVaSetValues(toplevel, XtNtitle, title, NULL);
		if (!user_icon_name)
			XtVaSetValues(toplevel, XtNiconName, icon_label, NULL);
		set_aicon_label(icon_label);
	}
	XtFree(title);
}

static void
flip_icon(inverted, mstate)
Boolean inverted;
enum mcursor_state mstate;
{
	Pixmap p;
	
	if (mstate == LOCKED)
		mstate = NORMAL;
	if (appres.active_icon
	    || (inverted == icon_inverted && mstate == icon_cstate))
		return;
	switch (mstate) {
	    case WAIT:
		if (inverted)
			p = inv_wait_icon;
		else
			p = wait_icon;
		break;
	    case LOCKED:
	    case NORMAL:
		if (inverted)
			p = inv_icon;
		else
			p = icon;
		break;
	}
	XtVaSetValues(toplevel,
	    XtNiconPixmap, p,
	    XtNiconMask, p,
	    NULL);
	icon_inverted = inverted;
	icon_cstate = mstate;
}

/*
 * Invert the icon.
 */
void
invert_icon(inverted)
Boolean inverted;
{
	flip_icon(inverted, icon_cstate);
}

/*
 * Change to the lock icon.
 */
void
lock_icon(state)
enum mcursor_state state;
{
	flip_icon(icon_inverted, state);
}

/*
 * Set up a user keymap.
 */
void
setup_keymaps(km, do_popup)
char *km;
Boolean do_popup;
{
	Boolean saw_apl_keymod = False;
	struct trans_list *t;
	struct trans_list *next;

	if (do_popup)
		keymap_changed = True;

	/* Clear out any existing translations. */
	appres.key_map = (char *)NULL;
	for (t = trans_list; t != (struct trans_list *)NULL; t = next) {
		next = t->next;
		XtFree(t->name);
		XtFree((char *)t);
	}
	trans_list = (struct trans_list *)NULL;
	last_trans = &trans_list;

	/* Build up the new list. */
	if (km != CN) {
		char *ns = XtNewString(km);
		char *comma;

		do {
			comma = strchr(ns, ',');
			if (comma)
				*comma = '\0';
			if (!strcmp(ns, Apl))
				saw_apl_keymod = True;
			add_keymap(ns, do_popup);
			if (comma)
				ns = comma + 1;
			else
				ns = NULL;
		} while (ns);
	}
	if (appres.apl_mode && !saw_apl_keymod)
		add_keymap(Apl, do_popup);
}

/*
 * Add to the list of user-specified keymap translations, finding both the
 * system and user versions of a keymap.
 */
static void
add_keymap(name, do_popup)
char *name;
Boolean do_popup;
{
	char *translations;
	char *buf;
	int any = 0;

	if (appres.key_map == (char *)NULL)
		appres.key_map = XtNewString(name);
	else {
		char *t = xs_buffer("%s,%s", appres.key_map, name);

		XtFree(appres.key_map);
		appres.key_map = t;
	}

	buf = xs_buffer("%s.%s", ResKeymap, name);
	if ((translations = get_resource(buf))) {
		add_trans(name, translations);
		any++;
	}
	XtFree(buf);
	buf = xs_buffer("%s.%s.%s", ResKeymap, name, ResUser);
	if ((translations = get_resource(buf))) {
		add_trans(buf + 7, translations);
		any++;
	}
	XtFree(buf);
	if (!any) {
		if (do_popup)
			popup_an_error("Cannot find %s \"%s\"", ResKeymap,
			    name);
		else
			xs_warning("Cannot find %s \"%s\"", ResKeymap, name);
	}
}

/*
 * Add a single keymap name and translation to the translation list.
 */
static void
add_trans(name, translations)
char *name;
char *translations;
{
	struct trans_list *t;

	t = (struct trans_list *)XtMalloc(sizeof(*t));
	t->name = XtNewString(name);
	(void) lookup_tt(name, translations);
	t->next = NULL;
	*last_trans = t;
	last_trans = &t->next;
}

/*
 * Peek at X events before Xt does, calling KeymapEvent_action if we see a
 * KeymapEvent.  This is to get around an (apparent) server bug that causes
 * Keymap events to come in with a window id of 0, so Xt never calls our
 * event handler.
 *
 * If the bug is ever fixed, this code will be redundant but harmless.
 */
static void
peek_at_xevent(e)
XEvent *e;
{
	static Cardinal zero = 0;

	if (e->type == KeymapNotify) {
		ia_cause = IA_PEEK;
		KeymapEvent_action((Widget)NULL, e, (String *)NULL, &zero);
	}
}


/*
 * Application exit, with cleanup.
 */
void
x3270_exit(n)
int n;
{
	static Boolean already_exiting = 0;

	/* Handle unintentional recursion. */
	if (already_exiting)
		return;
	already_exiting = True;

	/* Turn off toggle-related activity. */
	shutdown_toggles();

	/* Shut down the socket gracefully. */
	x_disconnect(False);

	exit(n);
}


/*
 * Font list parser.
 */
static void
parse_font_list()
{
	char *s, *label, *font;
	struct font_list *f;

	if (!appres.font_list)
		xs_error("No %s resource", ResFontList);
	s = XtNewString(appres.font_list);
	while (split_dresource(&s, &label, &font) == 1) {
		f = (struct font_list *)XtMalloc(sizeof(*f));
		f->label = label;
		f->font = font;
		f->next = (struct font_list *)NULL;
		if (font_list)
			font_last->next = f;
		else
			font_list = f;
		font_last = f;
		font_count++;
	}
	if (!font_count)
		xs_error("Invalid %s resource", ResFontList);
}


/*
 * Warning message trap, for catching colormap failures.
 */
static void
trap_colormaps(name, type, class, defaultp, params, num_params)
String name;
String type;
String class;
String defaultp;
String *params;
Cardinal *num_params;
{
    if (!strcmp(type, "cvtStringToPixel"))
	    colormap_failure = True;
    (*old_emh)(name, type, class, defaultp, params, num_params);
}

/*
 * Pick out -set and -clear toggle options.
 */
static void
parse_set_clear(argcp, argv)
int *argcp;
char **argv;
{
	int i, j;
	int argc_out = 0;
	char **argv_out = (char **) XtMalloc((*argcp + 1) * sizeof(char *));

	argv_out[argc_out++] = argv[0];

	for (i = 1; i < *argcp; i++) {
		Boolean is_set = False;

		if (!strcmp(argv[i], OptSet))
			is_set = True;
		else if (strcmp(argv[i], OptClear)) {
			argv_out[argc_out++] = argv[i];
			continue;
		}

		if (i == *argcp - 1)	/* missing arg */
			continue;

		/* Delete the argument. */
		i++;

		for (j = 0; j < N_TOGGLES; j++)
			if (!strcmp(argv[i], toggle_names[j].name)) {
				appres.toggle[toggle_names[j].index].value =
				    is_set;
				break;
			}
		if (j >= N_TOGGLES)
			usage("Unknown toggle name");

	}
	*argcp = argc_out;
	argv_out[argc_out] = CN;
	(void) memcpy((char *)argv, (char *)argv_out,
	    (argc_out + 1) * sizeof(char *));
	XtFree((XtPointer)argv_out);
}

/*
 * Safe routine for querying window attributes
 */
/*ARGSUSED*/
static int
dummy_error_handler(d, e)
Display *d;
XErrorEvent *e;
{
	return 0;
}

Status
x_get_window_attributes(w, wa)
Window w;
XWindowAttributes *wa;
{
	int (*old_handler)();
	Status s;

	old_handler = XSetErrorHandler(dummy_error_handler);

	s = XGetWindowAttributes(display, w, wa);
	if (!s)
		(void) fprintf(stderr, "Error: querying bad window 0x%lx\n", w);

	(void) XSetErrorHandler(old_handler);

	return s;
}
