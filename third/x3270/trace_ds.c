/*
 * Copyright 1993, 1994, 1995 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 */

/*
 *	trace_ds.c
 *		3270 data stream tracing.
 *
 */

#include "globals.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Dialog.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <fcntl.h>
#include "3270ds.h"
#include "appres.h"
#include "resources.h"
#include "ctlr.h"

#include "ctlrc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "printc.h"
#include "savec.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

/* Externals */
extern char    *build;

/* Statics */
static int      dscnt = 0;
static int      tracewindow_pid = -1;

/* Globals */
FILE           *tracef = (FILE *) 0;
struct timeval  ds_ts;
Boolean         trace_skipping = False;

char *
unknown(value)
unsigned char value;
{
	static char buf[64];

	(void) sprintf(buf, "unknown[0x%x]", value);
	return buf;
}

/* display a (row,col) */
char *
rcba(baddr)
int baddr;
{
	static char buf[16];

	(void) sprintf(buf, "(%d,%d)", baddr/COLS + 1, baddr%COLS + 1);
	return buf;
}

char *
see_ebc(ch)
unsigned char ch;
{
	static char buf[8];

	switch (ch) {
	    case FCORDER_NULL:
		return "NULL";
	    case FCORDER_SUB:
		return "SUB";
	    case FCORDER_DUP:
		return "DUP";
	    case FCORDER_FM:
		return "FM";
	    case FCORDER_FF:
		return "FF";
	    case FCORDER_CR:
		return "CR";
	    case FCORDER_NL:
		return "NL";
	    case FCORDER_EM:
		return "EM";
	    case FCORDER_EO:
		return "EO";
	}
	if (ebc2asc[ch])
		(void) sprintf(buf, "%c", ebc2asc[ch]);
	else
		(void) sprintf(buf, "\\%o", ch);
	return buf;
}

char *
see_aid(code)
unsigned char code;
{
	switch (code) {
	case AID_NO: 
		return "NoAID";
	case AID_ENTER: 
		return "Enter";
	case AID_PF1: 
		return "PF1";
	case AID_PF2: 
		return "PF2";
	case AID_PF3: 
		return "PF3";
	case AID_PF4: 
		return "PF4";
	case AID_PF5: 
		return "PF5";
	case AID_PF6: 
		return "PF6";
	case AID_PF7: 
		return "PF7";
	case AID_PF8: 
		return "PF8";
	case AID_PF9: 
		return "PF9";
	case AID_PF10: 
		return "PF10";
	case AID_PF11: 
		return "PF11";
	case AID_PF12: 
		return "PF12";
	case AID_PF13: 
		return "PF13";
	case AID_PF14: 
		return "PF14";
	case AID_PF15: 
		return "PF15";
	case AID_PF16: 
		return "PF16";
	case AID_PF17: 
		return "PF17";
	case AID_PF18: 
		return "PF18";
	case AID_PF19: 
		return "PF19";
	case AID_PF20: 
		return "PF20";
	case AID_PF21: 
		return "PF21";
	case AID_PF22: 
		return "PF22";
	case AID_PF23: 
		return "PF23";
	case AID_PF24: 
		return "PF24";
	case AID_OICR: 
		return "OICR";
	case AID_MSR_MHS: 
		return "MSR_MHS";
	case AID_SELECT: 
		return "Select";
	case AID_PA1: 
		return "PA1";
	case AID_PA2: 
		return "PA2";
	case AID_PA3: 
		return "PA3";
	case AID_CLEAR: 
		return "Clear";
	case AID_SYSREQ: 
		return "SysReq";
	case AID_QREPLY:
		return "QueryReplyAID";
	default: 
		return unknown(code);
	}
}

char *
see_attr(fa)
unsigned char fa;
{
	static char buf[256];
	char *paren = "(";

	buf[0] = '\0';

	if (fa & 0x04) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "protected");
		paren = ",";
		if (fa & 0x08) {
			(void) strcat(buf, paren);
			(void) strcat(buf, "skip");
			paren = ",";
		}
	} else if (fa & 0x08) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "numeric");
		paren = ",";
	}
	switch (fa & 0x03) {
	case 0:
		break;
	case 1:
		(void) strcat(buf, paren);
		(void) strcat(buf, "detectable");
		paren = ",";
		break;
	case 2:
		(void) strcat(buf, paren);
		(void) strcat(buf, "intensified");
		paren = ",";
		break;
	case 3:
		(void) strcat(buf, paren);
		(void) strcat(buf, "nondisplay");
		paren = ",";
		break;
	}
	if (fa & 0x20) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "modified");
		paren = ",";
	}
	if (strcmp(paren, "("))
		(void) strcat(buf, ")");
	else
		(void) strcpy(buf, "(default)");

	return buf;
}

static char *
see_highlight(setting)
unsigned char setting;
{
	switch (setting) {
	    case XAH_DEFAULT:
		return "default";
	    case XAH_NORMAL:
		return "normal";
	    case XAH_BLINK:
		return "blink";
	    case XAH_REVERSE:
		return "reverse";
	    case XAH_UNDERSCORE:
		return "underscore";
	    case XAH_INTENSIFY:
		return "intensify";
	    default:
		return unknown(setting);
	}
}

char *
see_color(setting)
unsigned char setting;
{
	static char *color_name[] = {
	    "neutralBlack",
	    "blue",
	    "red",
	    "pink",
	    "green",
	    "turquoise",
	    "yellow",
	    "neutralWhite",
	    "black",
	    "deepBlue",
	    "orange",
	    "purple",
	    "paleGreen",
	    "paleTurquoise",
	    "grey",
	    "white"
	};

	if (setting == XAC_DEFAULT)
		return "default";
	else if (setting < 0xf0 || setting > 0xff)
		return unknown(setting);
	else
		return color_name[setting - 0xf0];
}

static char *
see_transparency(setting)
unsigned char setting;
{
	switch (setting) {
	    case XAT_DEFAULT:
		return "default";
	    case XAT_OR:
		return "or";
	    case XAT_XOR:
		return "xor";
	    case XAT_OPAQUE:
		return "opaque";
	    default:
		return unknown(setting);
	}
}

static char *
see_validation(setting)
unsigned char setting;
{
	static char buf[64];
	char *paren = "(";

	(void) strcpy(buf, "");
	if (setting & XAV_FILL) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "fill");
		paren = ",";
	}
	if (setting & XAV_ENTRY) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "entry");
		paren = ",";
	}
	if (setting & XAV_TRIGGER) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "trigger");
		paren = ",";
	}
	if (strcmp(paren, "("))
		(void) strcat(buf, ")");
	else
		(void) strcpy(buf, "(none)");
	return buf;
}

static char *
see_outline(setting)
unsigned char setting;
{
	static char buf[64];
	char *paren = "(";

	(void) strcpy(buf, "");
	if (setting & XAO_UNDERLINE) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "underline");
		paren = ",";
	}
	if (setting & XAO_RIGHT) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "right");
		paren = ",";
	}
	if (setting & XAO_OVERLINE) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "overline");
		paren = ",";
	}
	if (setting & XAO_LEFT) {
		(void) strcat(buf, paren);
		(void) strcat(buf, "left");
		paren = ",";
	}
	if (strcmp(paren, "("))
		(void) strcat(buf, ")");
	else
		(void) strcpy(buf, "(none)");
	return buf;
}

char *
see_efa(efa, value)
unsigned char efa;
unsigned char value;
{
	static char buf[64];

	switch (efa) {
	    case XA_ALL:
		(void) sprintf(buf, " all(%x)", value);
		break;
	    case XA_3270:
		(void) sprintf(buf, " 3270%s", see_attr(value));
		break;
	    case XA_VALIDATION:
		(void) sprintf(buf, " validation%s", see_validation(value));
		break;
	    case XA_OUTLINING:
		(void) sprintf(buf, " outlining(%s)", see_outline(value));
		break;
	    case XA_HIGHLIGHTING:
		(void) sprintf(buf, " highlighting(%s)", see_highlight(value));
		break;
	    case XA_FOREGROUND:
		(void) sprintf(buf, " foreground(%s)", see_color(value));
		break;
	    case XA_CHARSET:
		(void) sprintf(buf, " charset(%x)", value);
		break;
	    case XA_BACKGROUND:
		(void) sprintf(buf, " background(%s)", see_color(value));
		break;
	    case XA_TRANSPARENCY:
		(void) sprintf(buf, " transparency(%s)", see_transparency(value));
		break;
	    default:
		(void) sprintf(buf, " %s[0x%x]", unknown(efa), value);
	}
	return buf;
}

char *
see_efa_only(efa)
unsigned char efa;
{
	switch (efa) {
	    case XA_ALL:
		return "all";
	    case XA_3270:
		return "3270";
	    case XA_VALIDATION:
		return "validation";
	    case XA_OUTLINING:
		return "outlining";
	    case XA_HIGHLIGHTING:
		return "highlighting";
	    case XA_FOREGROUND:
		return "foreground";
	    case XA_CHARSET:
		return "charset";
	    case XA_BACKGROUND:
		return "background";
	    case XA_TRANSPARENCY:
		return "transparency";
	    default:
		return unknown(efa);
	}
}

char *
see_qcode(id)
unsigned char id;
{
	static char buf[64];

	switch (id) {
	    case QR_CHARSETS:
		return "CharacterSets";
	    case QR_IMP_PART:
		return "ImplicitPartition";
	    case QR_SUMMARY:
		return "Summary";
	    case QR_USABLE_AREA:
		return "UsableArea";
	    case QR_COLOR:
		return "Color";
	    case QR_HIGHLIGHTING:
		return "Highlighting";
	    case QR_REPLY_MODES:
		return "ReplyModes";
	    case QR_ALPHA_PART:
		return "AlphanumericPartitions";
	    case QR_DDM:
		return "DistributedDataManagement";
	    default:
		(void) sprintf(buf, "unknown[0x%x]", id);
		return buf;
	}
}

/* Data Stream trace print, handles line wraps */

static char tdsbuf[4096];
#define TDS_LEN	75

static void
trace_ds_s(s)
char *s;
{
	int len = strlen(s);
	Boolean nl = False;

	if (!toggled(DS_TRACE))
		return;

	if (s && s[len-1] == '\n') {
		len--;
		nl = True;
	}
	while (dscnt + len >= 75) {
		int plen = 75-dscnt;

		(void) fprintf(tracef, "%.*s ...\n... ", plen, s);
		dscnt = 4;
		s += plen;
		len -= plen;
	}
	if (len) {
		(void) fprintf(tracef, "%.*s", len, s);
		dscnt += len;
	}
	if (nl) {
		(void) fprintf(tracef, "\n");
		dscnt = 0;
	}
}

#if defined(__STDC__)
void
trace_ds(char *fmt, ...)
#else
void
trace_ds(va_alist)
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
	/* print out remainder of message */
	(void) vsprintf(tdsbuf, fmt, args);
	trace_ds_s(tdsbuf);
	va_end(args);
}

static Widget trace_shell = (Widget)NULL;
static int trace_reason;

/* Callback for "OK" button on trace popup */
/*ARGSUSED*/
static void
tracefile_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *tfn;
	char *tracecmd;
	char *full_cmd;
	long clock;
	extern char *command_string;
	extern int children;

	if (w)
		tfn = XawDialogGetValueString((Widget)client_data);
	else
		tfn = (char *)client_data;
	tfn = do_subst(tfn, True, True);
	if (strchr(tfn, '\'') ||
	    ((int)strlen(tfn) > 0 && tfn[strlen(tfn)-1] == '\\')) {
		popup_an_error("Illegal file name: %s\n", tfn);
		XtFree(tfn);
		return;
	}
	tracef = fopen(tfn, "a");
	if (tracef == (FILE *)NULL) {
		popup_an_errno(errno, tfn);
		XtFree(tfn);
		return;
	}
	(void) SETLINEBUF(tracef);
	(void) fcntl(fileno(tracef), F_SETFD, 1);

	/* Display current status */
	clock = time((long *)0);
	(void) fprintf(tracef, "Trace started %s", ctime(&clock));
	(void) fprintf(tracef, " Version: %s\n", build);
	save_yourself();
	(void) fprintf(tracef, " Command: %s\n", command_string);
	(void) fprintf(tracef, " Model %s", model_name);
	(void) fprintf(tracef, ", %s display", 
	    appres.mono ? "monochrome" : "color");
	if (appres.extended)
		(void) fprintf(tracef, ", extended data stream");
	if (!appres.mono)
		(void) fprintf(tracef, ", %scolor",
		    appres.m3279 ? "full " : "pseudo-");
	if (appres.charset)
		(void) fprintf(tracef, ", %s charset", appres.charset);
	if (appres.apl_mode)
		(void) fprintf(tracef, ", APL mode");
	(void) fprintf(tracef, "\n");
	if (CONNECTED)
		(void) fprintf(tracef, " Connected to %s, port %u\n",
		    current_host, current_port);

	/* Start the monitor window */
	tracecmd = get_resource(ResTraceCommand);
	if (!tracecmd || !strcmp(tracecmd, "none"))
		goto done;
	switch (tracewindow_pid = fork()) {
	    case 0:	/* child process */
		full_cmd = xs_buffer("%s <'%s'", tracecmd, tfn);
		(void) execlp("xterm", "xterm", "-title", tfn,
		    "-sb", "-e", "/bin/sh", "-c", full_cmd, CN);
		(void) perror("exec(xterm)");
		_exit(1);
	    default:	/* parent */
		++children;
		break;
	    case -1:	/* error */
		popup_an_errno(errno, "fork()");
		break;
	}

    done:
	XtFree(tfn);

	/* We're really tracing, turn the flag on. */
	appres.toggle[trace_reason].value = True;
	appres.toggle[trace_reason].changed = True;
	menubar_retoggle(&appres.toggle[trace_reason]);

	if (w)
		XtPopdown(trace_shell);

	/* Snap the current TELNET options. */
	if (net_snap_options()) {
		(void) fprintf(tracef, " TELNET state:\n");
		trace_netdata('<', obuf, obptr - obuf);
	}

	/* Dump the screen contents and modes into the trace file. */
	if (CONNECTED && formatted) {
		(void) fprintf(tracef, " Screen contents:\n");
		ctlr_snap_buffer();
		space3270out(2);
		net_add_eor(obuf, obptr - obuf);
		obptr += 2;
		trace_netdata('<', obuf, obptr - obuf);
	}
	if (CONNECTED && ctlr_snap_modes()) {
		(void) fprintf(tracef, " 3270 modes:\n");
		space3270out(2);
		net_add_eor(obuf, obptr - obuf);
		obptr += 2;
		trace_netdata('<', obuf, obptr - obuf);
	}

	(void) fprintf(tracef, " Data stream:\n");
}

/* Open the trace file. */
static void
tracefile_on(reason, tt)
int reason;
enum toggle_type tt;
{
	char tracefile[256];

	if (tracef)
		return;

	(void) sprintf(tracefile, "%s/x3trc.%d", appres.trace_dir,
		getpid());
	trace_reason = reason;
	if (tt == TT_INITIAL) {
		tracefile_callback((Widget)NULL, tracefile, PN);
		return;
	}
	if (trace_shell == NULL) {
		trace_shell = create_form_popup("trace",
		    tracefile_callback, (XtCallbackProc)NULL, True);
		XtVaSetValues(XtNameToWidget(trace_shell, "dialog"),
		    XtNvalue, tracefile,
		    NULL);
	}

	/* Turn the toggle _off_ until the popup succeeds. */
	appres.toggle[reason].value = False;
	appres.toggle[reason].changed = True;

	popup_popup(trace_shell, XtGrabExclusive);
}

/* Close the trace file. */
static void
tracefile_off()
{
	long clock;

	clock = time((long *)0);
	(void) fprintf(tracef, "Trace stopped %s", ctime(&clock));
	if (tracewindow_pid != -1)
		(void) kill(tracewindow_pid, SIGTERM);
	tracewindow_pid = -1;
	(void) fclose(tracef);
	tracef = (FILE *) NULL;
}

/*ARGSUSED*/
void
toggle_dsTrace(t, tt)
struct toggle *t;
enum toggle_type tt;
{
	/* If turning on trace and no trace file, open one. */
	if (toggled(DS_TRACE) && !tracef)
		tracefile_on(DS_TRACE, tt);

	/* If turning off trace and not still tracing events, close the
	   trace file. */
	else if (!toggled(DS_TRACE) && !toggled(EVENT_TRACE))
		tracefile_off();

	if (toggled(DS_TRACE))
		(void) gettimeofday(&ds_ts, (struct timezone *)NULL);
}

/*ARGSUSED*/
void
toggle_eventTrace(t, tt)
struct toggle *t;
enum toggle_type tt;
{
	/* If turning on event debug, and no trace file, open one. */
	if (toggled(EVENT_TRACE) && !tracef)
		tracefile_on(EVENT_TRACE, tt);

	/* If turning off event debug, and not tracing the data stream,
	   close the trace file. */
	else if (!toggled(EVENT_TRACE) && !toggled(DS_TRACE))
		tracefile_off();
}

/* Screen trace file support. */

static Widget screentrace_shell = (Widget)NULL;
static FILE *screentracef = (FILE *)0;

/*
 * Screen trace function, called when the host clears the screen.
 */
static void
do_screentrace()
{
	register int i;

	if (fprint_screen(screentracef, False)) {
		for (i = 0; i < COLS; i++)
			(void) fputc('=', screentracef);
		(void) fputc('\n', screentracef);
	}
}

void
trace_screen()
{
	trace_skipping = False;

	if (!toggled(SCREEN_TRACE) | !screentracef)
		return;
	do_screentrace();
}

/* Called from ANSI emulation code to log a single character. */
void
trace_char(c)
char c;
{
	if (!toggled(SCREEN_TRACE) | !screentracef)
		return;
	(void) fputc(c, screentracef);
}

/*
 * Called when disconnecting in ANSI mode, to finish off the trace file
 * and keep the next screen clear from re-recording the screen image.
 * (In a gross violation of data hiding and modularity, trace_skipping is
 * manipulated directly in ctlr_clear()).
 */
void
trace_ansi_disc()
{
	int i;

	(void) fputc('\n', screentracef);
	for (i = 0; i < COLS; i++)
		(void) fputc('=', screentracef);
	(void) fputc('\n', screentracef);

	trace_skipping = True;
}

/* Callback for "OK" button on screentrace popup */
/*ARGSUSED*/
static void
screentrace_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *tfn;

	if (w)
		tfn = XawDialogGetValueString((Widget)client_data);
	else
		tfn = (char *)client_data;
	tfn = do_subst(tfn, True, True);
	screentracef = fopen(tfn, "a");
	if (screentracef == (FILE *)NULL) {
		popup_an_errno(errno, tfn);
		XtFree(tfn);
		return;
	}
	XtFree(tfn);
	(void) SETLINEBUF(screentracef);
	(void) fcntl(fileno(screentracef), F_SETFD, 1);

	/* We're really tracing, turn the flag on. */
	appres.toggle[SCREEN_TRACE].value = True;
	appres.toggle[SCREEN_TRACE].changed = True;
	menubar_retoggle(&appres.toggle[SCREEN_TRACE]);

	if (w)
		XtPopdown(screentrace_shell);
}

/* Callback for second "OK" button on screentrace popup */
/*ARGSUSED*/
static void
onescreen_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *tfn;

	if (w)
		tfn = XawDialogGetValueString((Widget)client_data);
	else
		tfn = (char *)client_data;
	tfn = do_subst(tfn, True, True);
	screentracef = fopen(tfn, "a");
	if (screentracef == (FILE *)NULL) {
		popup_an_errno(errno, tfn);
		XtFree(tfn);
		return;
	}
	(void) fcntl(fileno(screentracef), F_SETFD, 1);
	XtFree(tfn);

	/* Save the current image, once. */
	do_screentrace();

	/* Close the file, we're done. */
	(void) fclose(screentracef);
	screentracef = (FILE *)NULL;

	if (w)
		XtPopdown(screentrace_shell);
}

/*ARGSUSED*/
void
toggle_screenTrace(t, tt)
struct toggle *t;
enum toggle_type tt;
{
	char tracefile[256];

	if (toggled(SCREEN_TRACE)) {
		(void) sprintf(tracefile, "%s/x3scr.%d", appres.trace_dir,
			getpid());
		if (tt == TT_INITIAL) {
			screentrace_callback((Widget)NULL, tracefile, PN);
			return;
		}
		if (screentrace_shell == NULL) {
			screentrace_shell = create_form_popup("screentrace",
			    screentrace_callback, onescreen_callback, True);
			XtVaSetValues(XtNameToWidget(screentrace_shell, "dialog"),
			    XtNvalue, tracefile,
			    NULL);
		}
		appres.toggle[SCREEN_TRACE].value = False;
		appres.toggle[SCREEN_TRACE].changed = True;
		popup_popup(screentrace_shell, XtGrabExclusive);
	} else {
		if (ctlr_any_data() && !trace_skipping)
			do_screentrace();
		(void) fclose(screentracef);
	}
}
