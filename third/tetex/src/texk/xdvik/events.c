
/*========================================================================*\

Copyright (c) 1990-2001  Paul Vojta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
PAUL VOJTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

NOTE:
	xdvi is based on prior work, as noted in the modification history
	in xdvi.c.

\*========================================================================*/

#include "xdvi-config.h"
#include "xdvi.h"
#include "string-utils.h"

/* Xlib and Xutil are already included */

#ifdef	TOOLKIT

#  ifdef OLD_X11_TOOLKIT
#    include <X11/Atoms.h>
#  else /* not OLD_X11_TOOLKIT */
#    include <X11/Xatom.h>
#    include <X11/StringDefs.h>
#  endif /* not OLD_X11_TOOLKIT */

#  include <X11/Shell.h>	/* needed for def. of XtNiconX */

#  ifndef	XtSpecificationRelease
#    define	XtSpecificationRelease	0
#  endif

#  if XtSpecificationRelease >= 4
#    ifndef MOTIF
#      if XAW3D
#        include <X11/Xaw3d/Viewport.h>
#      else
#        include <X11/Xaw/Viewport.h>
#      endif
#      ifdef HTEX
#        include <X11/Xaw/Dialog.h>
#        include <X11/Xaw/Text.h>
#      endif
#    else /* MOTIF */
#      include <Xm/MainW.h>
#      include <Xm/ToggleB.h>
#      include <Xm/RowColumn.h>
#      include <Xm/MenuShell.h>
#    endif /* MOTIF */

#    ifdef	BUTTONS
#      ifndef MOTIF
#        if XAW3D
#          include <X11/Xaw3d/Command.h>
#        else
#          include <X11/Xaw/Command.h>
#        endif
#        define	PANEL_WIDGET_CLASS	compositeWidgetClass
#        define	BUTTON_WIDGET_CLASS	commandWidgetClass
#      else /* MOTIF */
#        include <Xm/Form.h>
#        include <Xm/BulletinB.h>
#        include <Xm/PushB.h>
#        define	PANEL_WIDGET_CLASS	xmBulletinBoardWidgetClass
#        define	BUTTON_WIDGET_CLASS	xmPushButtonWidgetClass
#      endif /* MOTIF */
#    endif /* BUTTONS */

#  else /* XtSpecificationRelease < 4 */

#    if NeedFunctionPrototypes
       typedef void *XtPointer;
#    else
       typedef char *XtPointer;
#    endif

#    include <X11/Viewport.h>
#    ifdef	BUTTONS
#      include <X11/Command.h>
#      define	PANEL_WIDGET_CLASS	compositeWidgetClass
#      define	BUTTON_WIDGET_CLASS	commandWidgetClass
#    endif

#  endif /* XtSpecificationRelease */

#else /* not TOOLKIT */

   typedef int Position;

#  define	XtPending()	XPending(DISP)

#endif /* not TOOLKIT */

#ifdef STDC_HEADERS
#include <unistd.h>
#include <fcntl.h>
#endif
#include <signal.h>
#include <sys/file.h>	/* this defines FASYNC */
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>  /* Or this might define FASYNC */
#endif
#include <sys/ioctl.h>	/* this defines SIOCSPGRP and FIOASYNC */
#include <sys/wait.h>	/* ignore HAVE_SYS_WAIT_H -- we always need WNOHANG */

/* Linux prefers O_ASYNC over FASYNC; SGI IRIX does the opposite.  */
#if !defined(FASYNC) && defined(O_ASYNC)
#define	FASYNC	O_ASYNC
#endif

#if !defined(FLAKY_SIGPOLL) && !HAVE_STREAMS && !defined(FASYNC)
#if !defined(SIOCSPGRP) || !defined(FIOASYNC)
#define	FLAKY_SIGPOLL	1
#endif
#endif

#ifndef FLAKY_SIGPOLL

#ifndef SIGPOLL
#define	SIGPOLL	SIGIO
#endif

#ifndef SA_RESTART
#define SA_RESTART 0
#endif

#if HAVE_STREAMS
#include <stropts.h>

#ifndef S_RDNORM
#define	S_RDNORM S_INPUT
#endif

#ifndef S_RDBAND
#define	S_RDBAND 0
#endif

#ifndef S_HANGUP
#define	S_HANGUP 0
#endif

#ifndef S_WRNORM
#define	S_WRNORM S_OUTPUT
#endif
#endif /* HAVE_STREAMS */

#endif /* not FLAKY_SIGPOLL */

#if HAVE_POLL
# include <poll.h>
#else
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# else
#  if HAVE_SELECT_H
#   include <select.h>
#  endif
# endif
#endif

#include <errno.h>

#ifdef	X_NOT_STDC_ENV
extern int errno;
#endif /* X_NOT_STDC_ENV */

#ifndef	X11HEIGHT
#define	X11HEIGHT	8	/* Height of server default font */
#endif

#define	MAGBORD	1	/* border size for magnifier */

/*
 * Command line flags.
 */

#define delay_rulers	resource._delay_rulers
#define tick_units	resource._tick_units
#define tick_length	resource._tick_length
#define	fore_Pixel	resource._fore_Pixel
#define	back_Pixel	resource._back_Pixel
#ifdef	TOOLKIT
extern struct _resource resource;
#define	brdr_Pixel	resource._brdr_Pixel
#endif /* TOOLKIT */

#define	clip_w	mane.width
#define	clip_h	mane.height
static Position main_x, main_y;
static Position mag_x, mag_y, new_mag_x, new_mag_y;
#if TOOLKIT && !MOTIF
static int mag_conv_x, mag_conv_y;
#else
#define	mag_conv_x	0
#define	mag_conv_y	0
#endif
static Boolean mag_moved = False;
static Boolean busycurs = False;
Boolean dragcurs = False;	/* needed for hypertex.c */

#ifndef FLAKY_SIGPOLL
static sigset_t all_signals;
#endif

static Boolean child_flag = False;	/* if SIGCHLD received */

#if HAVE_POLL
static struct pollfd fds[1] = { {0, POLLIN, 0} };
#else
static fd_set readfds;
#endif

#ifdef TOOLKIT
#define	ACTION_DECL(name)				\
		void name ARGS((Widget, XEvent *, String *, Cardinal *))

#define	ACTION(name)				\
		void				\
		name   (Widget w,		\
			XEvent *event,		\
			String *params,		\
			Cardinal *num_params)
#else /* TOOLKIT */
#define	ACTION_DECL(name)				\
		void name ARGS((XEvent *))

#define	ACTION(name)					\
		void					\
		name (XEvent *event)
#endif /* TOOLKIT */

/* ARGSUSED */
void
null_mouse(XEvent *event)
{
    UNUSED(event);
}

static ACTION_DECL(Act_digit);
static ACTION_DECL(Act_minus);
static ACTION_DECL(Act_quit);
static ACTION_DECL(Act_help);
#if 0
static ACTION_DECL(Act_print);
#endif
static ACTION_DECL(Act_goto_page);
static ACTION_DECL(Act_forward_page);
static ACTION_DECL(Act_back_page);
static ACTION_DECL(Act_declare_page_number);
static ACTION_DECL(Act_home);
static ACTION_DECL(Act_center);
static ACTION_DECL(Act_set_keep_flag);
static ACTION_DECL(Act_left);
static ACTION_DECL(Act_right);
static ACTION_DECL(Act_up);
static ACTION_DECL(Act_down);
static ACTION_DECL(Act_up_or_previous);
static ACTION_DECL(Act_down_or_next);
static ACTION_DECL(Act_set_margins);
static ACTION_DECL(Act_show_display_attributes);
static ACTION_DECL(Act_set_shrink_factor);
static ACTION_DECL(Act_shrink_to_dpi);
static ACTION_DECL(Act_set_density);
#if GREY
static ACTION_DECL(Act_set_greyscaling);
#endif
#if PS
static ACTION_DECL(Act_set_ps);
#endif
#ifdef HTEX
static ACTION_DECL(Act_htex_back);
static ACTION_DECL(Act_htex_anchorinfo);
#endif
#if PS_GS
static ACTION_DECL(Act_set_gs_alpha);
#endif
#if BUTTONS
static ACTION_DECL(Act_set_expert_mode);
#endif
#if !TOOLKIT
static ACTION_DECL(Act_redraw);
#endif
static ACTION_DECL(Act_reread_dvi_file);
#ifdef SELFILE
static ACTION_DECL(Act_select_dvi_file);
#endif
static ACTION_DECL(Act_discard_number);
static ACTION_DECL(Act_href);
static ACTION_DECL(Act_href_newwindow);
static ACTION_DECL(Act_magnifier);
static ACTION_DECL(Act_drag);
static ACTION_DECL(Act_wheel);
static ACTION_DECL(Act_motion);
static ACTION_DECL(Act_release);
static ACTION_DECL(Act_switch_magnifier_units);
#ifdef GRID
static ACTION_DECL(Act_toggle_grid_mode);
#endif
static ACTION_DECL(Act_source_special);
static ACTION_DECL(Act_show_source_specials);
static ACTION_DECL(Act_source_what_special);

#if TOOLKIT

XtActionsRec Actions[] = {
    {"digit", Act_digit},
    {"minus", Act_minus},
    {"quit", Act_quit},
    {"help", Act_help},
#if 0
    {"print", Act_print},
#endif
    {"goto-page", Act_goto_page},
    {"forward-page", Act_forward_page},
    {"back-page", Act_back_page},
    {"declare-page-number", Act_declare_page_number},
    {"home", Act_home},
    {"center", Act_center},
    {"set-keep-flag", Act_set_keep_flag},
    {"left", Act_left},
    {"right", Act_right},
    {"up", Act_up},
    {"down", Act_down},
    {"up-or-previous", Act_up_or_previous},
    {"down-or-next", Act_down_or_next},
    {"set-margins", Act_set_margins},
    {"show-display-attributes", Act_show_display_attributes},
    {"set-shrink-factor", Act_set_shrink_factor},
    {"shrink-to-dpi", Act_shrink_to_dpi},
    {"set-density", Act_set_density},
#if GREY
    {"set-greyscaling", Act_set_greyscaling},
#endif
#if PS
    {"set-ps", Act_set_ps},
#endif
#ifdef HTEX
    {"htex-back", Act_htex_back},
    {"htex-anchorinfo", Act_htex_anchorinfo},
#endif
#if PS_GS
    {"set-gs-alpha", Act_set_gs_alpha},
#endif
#if BUTTONS
    {"set-expert-mode", Act_set_expert_mode},
#endif
    {"reread-dvi-file", Act_reread_dvi_file},
#ifdef SELFILE
    {"select-dvi-file", Act_select_dvi_file},
#endif /* SELFILE */
    {"discard-number", Act_discard_number},
    {"magnifier", Act_magnifier},
    {"do-href", Act_href},
    {"do-href-newwindow", Act_href_newwindow},
    {"drag", Act_drag},
    {"wheel", Act_wheel},
    {"motion", Act_motion},
    {"release", Act_release},
    {"switch-magnifier-units", Act_switch_magnifier_units},
#ifdef GRID
    {"toggle-grid-mode", Act_toggle_grid_mode},
#endif
    {"source-special", Act_source_special},
    {"show-source-specials", Act_show_source_specials},
    {"source-what-special", Act_source_what_special},
};

Cardinal num_actions = XtNumber(Actions);


#if BUTTONS
static Widget line_widget;
static Widget panel_widget;
static int destroy_count = 0;
#endif /* BUTTONS */

#ifndef MOTIF
static Widget x_bar, y_bar;	/* horizontal and vertical scroll bars */
#endif

static Arg resize_args[] = {
    {XtNwidth, (XtArgVal) 0},
    {XtNheight, (XtArgVal) 0},
};

#define	XdviResizeWidget(widget, w, h)	\
		(width_arg.value = (XtArgVal) (w), \
		resize_args[1].value = (XtArgVal) (h), \
		XtSetValues(widget, resize_args, XtNumber(resize_args)) )

#define	width_arg	(resize_args[0])
#define	height_arg	(resize_args[1])

#if BUTTONS

#ifndef MOTIF
static Arg resizable_on[] = {
    {XtNresizable, (XtArgVal) True},
};

static Arg resizable_off[] = {
    {XtNresizable, (XtArgVal) False},
};
#endif

static Arg line_args[] = {
    {XtNbackground, (XtArgVal) 0},
    {XtNwidth, (XtArgVal) 1},
#if !MOTIF
    {XtNfromHoriz, (XtArgVal) NULL},
    {XtNborderWidth, (XtArgVal) 0},
    {XtNtop, (XtArgVal) XtChainTop},
    {XtNbottom, (XtArgVal) XtChainBottom},
    {XtNleft, (XtArgVal) XtChainRight},
    {XtNright, (XtArgVal) XtChainRight},
#else /* MOTIF */
    {XmNleftWidget, (XtArgVal) NULL},
    {XmNleftAttachment, (XtArgVal) XmATTACH_WIDGET},
    {XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNbottomAttachment, (XtArgVal) XmATTACH_FORM},
#endif /* MOTIF */
};

static Arg panel_args[] = {
#if !MOTIF
    {XtNborderWidth, (XtArgVal) 0},
    {XtNfromHoriz, (XtArgVal) NULL},
    {XtNtranslations,	(XtArgVal) NULL},
    {XtNtop, (XtArgVal) XtChainTop},
    {XtNbottom, (XtArgVal) XtChainBottom},
    {XtNleft, (XtArgVal) XtChainRight},
    {XtNright, (XtArgVal) XtChainRight},
#else /* MOTIF */
    {XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNbottomAttachment, (XtArgVal) XmATTACH_FORM},
#endif /* MOTIF */
};

static void handle_command(Widget widget, XtPointer client_data, XtPointer call_data);

static XtCallbackRec command_call[] = {
    {handle_command, NULL},
    {NULL, NULL},
};

static Arg command_args[] = {
#ifndef MOTIF
    {XtNlabel, (XtArgVal) NULL},
#else
    {XmNlabelString, (XtArgVal) NULL},
#endif
    {XtNx, (XtArgVal) 0},
    {XtNy, (XtArgVal) 0},
    {XtNborderWidth, (XtArgVal) 0},
#ifndef MOTIF
    {XtNcallback, (XtArgVal) command_call},
#else
    {XmNactivateCallback, (XtArgVal) command_call},
#endif
};


struct xdvi_action {
    XtActionProc proc;
    Cardinal num_params;
    String param;
    struct xdvi_action *next;
};

static struct xdvi_action *
compile_action(_Xconst char *str)
{
    _Xconst char *p, *p1, *p2;
    XtActionsRec *actp;
    struct xdvi_action *ap;

    while (*str == ' ' || *str == '\t')
	++str;

    if (*str == '\0' || *str == '\n')
	return NULL;

    p = str;
    while (((*p | ('a' ^ 'A')) >= 'a' && (*p | ('a' ^ 'A')) <= 'z')
	   || (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')
	++p;

    for (actp = Actions;; ++actp) {
	if (actp >= Actions + XtNumber(Actions)) {
	    _Xconst char *tmp = strchr(str, '\n');
	    if (tmp == NULL) {
		tmp = p;
	    }
	    fprintf(stderr, "%s: Warning: cannot compile action %.*s (skipping it).\n", prog,
		    tmp - str, str);
	    return NULL;
	}
	if (memcmp(str, actp->string, p - str) == 0
	    && actp->string[p - str] == '\0') break;
    }

    while (*p == ' ' || *p == '\t')
	++p;
    if (*p != '(') {
	while (*p != '\0' && *p != '\n')
	    ++p;
	fprintf(stderr, "%s: Warning: syntax error in action %.*s (skipping it).\n", prog, p - str, str);
	return NULL;
    }
    ++p;
    while (*p == ' ' || *p == '\t')
	++p;
    for (p1 = p;; ++p1) {
	if (*p1 == '\0' || *p1 == '\n') {
	    fprintf(stderr, "%s: Warning: syntax error in action %.*s (skipping it).\n", prog,
		    p1 - str, str);
	    return NULL;
	}
	if (*p1 == ')')
	    break;
    }

    ap = xmalloc(sizeof *ap);
    ap->proc = actp->proc;
    for (p2 = p1;; --p2)
	if (p2 <= p) {	/* if no args */
	    ap->num_params = 0;
	    ap->param = NULL;
	    break;
	}
	else if (p2[-1] != ' ' && p2[-1] != '\t') {
	    char *arg;

	    arg = xmalloc(p2 - p + 1);
	    bcopy(p, arg, p2 - p);
	    arg[p2 - p] = '\0';
	    ap->num_params = 1;
	    ap->param = arg;
	    break;
	}

    ap->next = compile_action(p1 + 1);

    return ap;
}


struct button_info {
    struct button_info *next;
    char *label;
    struct xdvi_action *action;
    Widget widget;
};

static struct button_info *b_head;

void
create_buttons(void)
{
    Dimension max_button_width;
    Dimension y_pos;
    struct button_info **bipp;
    struct button_info *bp;
    _Xconst char *cspos;
    int button_number;
    int shrink_button_number;
    int shrink_arg;
    struct xdvi_action *action;

    line_args[0].value = (XtArgVal) resource._fore_Pixel;
#if !MOTIF
    line_args[2].value = (XtArgVal) vport_widget;
    line_widget = XtCreateWidget("line", widgetClass, form_widget,
				 line_args, XtNumber(line_args));
    panel_args[1].value = (XtArgVal) line_widget;
    /* to prevent the magnifier from popping up in the panel */
    if (panel_args[2].value == (XtArgVal)NULL) {
	panel_args[2].value = (XtArgVal)XtParseTranslationTable("#augment <ButtonPress>:");
    }
    panel_widget = XtCreateWidget("panel", PANEL_WIDGET_CLASS,
				  form_widget, panel_args,
				  XtNumber(panel_args));
#else
    panel_widget = XtCreateManagedWidget("panel", PANEL_WIDGET_CLASS,
					 form_widget, panel_args,
					 XtNumber(panel_args));
    if (wheel_trans_table != NULL)
	XtOverrideTranslations(panel_widget, wheel_trans_table);
#endif

    button_number = shrink_button_number = 0;
    b_head = NULL;
    bipp = &b_head;
    command_args[1].value = resource.btn_side_spacing;
    command_args[3].value = resource.btn_border_width;
    max_button_width = 0;
    y_pos = resource.btn_top_spacing;

    for (cspos = resource.button_translations;; ++cspos) {
	Dimension w, h;
	_Xconst char *p1, *p2;
	char *label, *q;
	char name[9];
	Widget widget;
	size_t len;

	while (*cspos == ' ' || *cspos == '\t')
	    ++cspos;
	if (*cspos == '\0')
	    break;

	if (*cspos == '\n') {
	    y_pos += resource.btn_between_extra;
	    continue;
	}

	len = 0;	/* find length of actual label string */
	shrink_arg = 0;
	for (p2 = p1 = cspos; *p1 != '\0' && *p1 != ':';) {
	    if (*p1 == '\\' && p1[1] != '\0') {
		p1 += 2;
		p2 = p1;
		--len;
	    }
	    else if (*p1 == '$'
		     && (p1[1] == '#' || p1[1] == '%' || p1[1] == '_')) {
		shrink_arg = 1;
		++p1;
		if (*p1 == '%')
		    len += 2;
		else if (*p1 == '_')
		    len -= 2;
	    }
	    else {
		++p1;
		if (p1[-1] != ' ' && p1[-1] != '\t')
		    p2 = p1;
	    }
	}
	len += p2 - cspos;

	if (*p1 == '\0')
	    break;	/* if premature end of string */

	action = compile_action(p1 + 1);

	if (shrink_arg) {
	    for (;; action = action->next) {
		if (action == NULL) {
		    fprintf(stderr,
			    "Warning:  label on button number %d refers to non-existent shrink action.\n",
			    button_number + 1);
		    break;
		}
		if (action->proc == Act_set_shrink_factor
		    || action->proc == Act_shrink_to_dpi) {
		    if (shrink_button_number < 9
			&& resource.shrinkbutton[shrink_button_number] != 0) {
			shrink_arg
			    = resource.shrinkbutton[shrink_button_number];
			if (shrink_arg < 1)
			    shrink_arg = 1;
			else if (shrink_arg > 99)
			    shrink_arg = 99;
			if (action->num_params > 0)
			    free(action->param);
			action->proc = Act_set_shrink_factor;
			action->num_params = 1;
			action->param = xmalloc(4);
			sprintf(action->param, "%d", shrink_arg);
		    }
		    else {
			if (action->num_params > 0) {
			    shrink_arg = atoi(action->param);
			    if (action->proc == Act_shrink_to_dpi)
				shrink_arg = (double)pixels_per_inch
				    / shrink_arg + 0.5;
			    if (shrink_arg < 1)
				shrink_arg = 1;
			    else if (shrink_arg > 99)
				shrink_arg = 99;
			}
		    }
		    break;
		}
	    }
	    ++shrink_button_number;
	}

	label = xmalloc(len + 1);
	for (q = label; cspos < p2; ++cspos) {
	    if (*cspos == '\\') {
		if (++cspos < p2)
		    *q++ = *cspos;
	    }
	    else if (*cspos == '$'
		     && (cspos[1] == '#' || cspos[1] == '%' || cspos[1] == '_')) {
		++cspos;
		if (*cspos == '#') {
		    sprintf(q, "%d", shrink_arg);
		}
		else if (*cspos == '%') {
		    if (shrink_arg <= 15)
			sprintf(q, "%d", (int)(100.0 / shrink_arg + .5));
		    else
			sprintf(q, "%.2f", 100.0 / shrink_arg);
		}
		q += strlen(q);
	    }
	    else
		*q++ = *cspos;
	}
	*q = '\0';
	if (q > label + len)
	    oops("Internal error computing button labels");


#if !MOTIF
	command_args[0].value = (XtArgVal) label;
#else /* MOTIF */
	command_args[0].value = (XtArgVal) XmCvtCTToXmString(label);
#endif /* MOTIF */
	command_args[2].value = (XtArgVal) y_pos;
	command_call[0].closure = (XtPointer) action;
	if (++button_number > 99)
	    break;	/* if too many buttons */
	sprintf(name, "button%d", button_number);
	widget = XtCreateWidget(name, BUTTON_WIDGET_CLASS, panel_widget,
				command_args, XtNumber(command_args));
	resize_args[0].value = (XtArgVal) & w;
	resize_args[1].value = (XtArgVal) & h;
	XtGetValues(widget, resize_args, 2);

	if (w > max_button_width) {
	    max_button_width = w;
	}

	y_pos +=
	    h + resource.btn_between_spacing + 2 * resource.btn_border_width;


	bp = xmalloc(sizeof *bp);
	bp->label = label;
	bp->action = action;
	bp->widget = widget;
	*bipp = bp;
	bipp = &bp->next;

	cspos = strchr(p1 + 1, '\n');
	if (cspos == NULL)
	    break;
    }
    *bipp = NULL;

    xtra_wid = max_button_width
	+ 2 * (resource.btn_side_spacing + resource.btn_border_width);

    width_arg.value = (XtArgVal) xtra_wid;
    XtSetValues(panel_widget, &width_arg, 1);

#if !MOTIF
    ++xtra_wid;
#endif

    width_arg.value = (XtArgVal) max_button_width;
    for (bp = b_head; bp != NULL; bp = bp->next) {
	XtSetValues(bp->widget, &width_arg, 1);
	XtManageChild(bp->widget);
    }

#if MOTIF
    line_args[2].value = (XtArgVal) panel_widget;
    line_widget = XtCreateManagedWidget("line", widgetClass, form_widget,
					line_args, XtNumber(line_args));
    XtVaSetValues(vport_widget, XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, line_widget, NULL);
#endif
}

#if !MOTIF

void
set_button_panel_height(XtArgVal h)
{
    height_arg.value = h;
    XtSetValues(line_widget, &height_arg, 1);
    XtManageChild(line_widget);
    XtSetValues(panel_widget, &height_arg, 1);
    XtManageChild(panel_widget);
}

#endif /* MOTIF */

#endif /* BUTTONS */

#else /* not TOOLKIT */

static Window x_bar, y_bar;
static int x_bgn, x_end, y_bgn, y_end;	/* scrollbar positions */

static
ACTION(Act_null)
{
}

#if !GREY
#define	Act_set_greyscaling	Act_null
#endif

#if !PS
#define	Act_set_ps		Act_null
#endif

#if !HTEX
#define	Act_htex_back		Act_null
#define	Act_htex_anchorinfo	Act_null
#endif

#if !PS_GS
#define	Act_set_gs_alpha	Act_null
#endif

typedef void (*act_proc) ARGS((XEvent *));

static act_proc actions[128] = {
    Act_null, Act_null, Act_null, Act_quit,	/* NUL, ^A-^C */
    Act_quit, Act_null, Act_null, Act_null,	/* ^D-^G */
    Act_up_or_previous, Act_null, Act_forward_page,
    Act_null,	/* ^H-^K */
    Act_redraw, Act_forward_page, Act_null, Act_null,	/* ^L-^O */
    Act_show_display_attributes, Act_null, Act_null,
    Act_src_mode_toggle,	/* ^P-^S */
    Act_null, Act_null, Act_show_source_specials, Act_null,	/* ^T-^W */
    Act_source_what_special, Act_null, Act_null,
    Act_discard_number,	/* ^X-^Z, ESC */
    Act_null, Act_null, Act_null, Act_null,	/* ^{\,],^,_} */
    Act_down_or_next, Act_null, Act_null, Act_null,	/* SP,!,",# */
    Act_null, Act_null, Act_null, Act_null,	/* $,%,&,' */
    Act_null, Act_null, Act_null, Act_null,	/* (,),*,+ */
    Act_null, Act_minus, Act_null, Act_null,	/* ,,-,.,/ */
    Act_digit, Act_digit, Act_digit, Act_digit,	/* 0,1,2,3 */
    Act_digit, Act_digit, Act_digit, Act_digit,	/* 4,5,6,7 */
    Act_digit, Act_digit, Act_null, Act_null,	/* 8,9,:,; */
    Act_null, Act_null, Act_null, Act_null,	/* <,=,>,? */
    Act_null, Act_null, Act_htex_back, Act_null,	/* @,A,B,C */
    Act_null, Act_null, Act_null,
    Act_set_greyscaling,	/* D,E,F,G */
    Act_null, Act_null, Act_null, Act_null,	/* H,I,J,K */
    Act_null, Act_set_margins, Act_null, Act_null,	/* L,M,N,O */
    Act_declare_page_number, Act_null,
    Act_reread_dvi_file, Act_set_density,	/* P,Q,R,S */
    Act_null, Act_null, Act_set_gs_alpha, Act_null,	/* T,U,V,W */
    Act_null, Act_null, Act_null, Act_null,	/* X,Y,Z,[ */
    Act_null, Act_null, Act_home, Act_null,	/* \,],^,_ */
    Act_null, Act_null, Act_back_page, Act_center,	/* `,a,b,c */
    Act_down, Act_null, Act_forward_page,
    Act_goto_page,	/* d,e,f,g */
    Act_null, Act_htex_anchorinfo, Act_null,
    Act_set_keep_flag,	/* h,i,j,k */
    Act_left, Act_null, Act_forward_page, Act_null,	/* l,m,n,o */
    Act_back_page, Act_quit, Act_right,
    Act_set_shrink_factor,	/* p,q,r,s */
    Act_null, Act_up, Act_set_ps, Act_null,	/* t,u,v,w */
    Act_null, Act_null, Act_null, Act_null,	/* x,y,z,{ */
    Act_null, Act_null, Act_null, Act_up_or_previous,	/* |,},~,DEL */
};

#endif /* not TOOLKIT */

/*
 *	Mechanism to keep track of the magnifier window.  The problems are,
 *	(a) if the button is released while the window is being drawn, this
 *	could cause an X error if we continue drawing in it after it is
 *	destroyed, and
 *	(b) creating and destroying the window too quickly confuses the window
 *	manager, which is avoided by waiting for an expose event before
 *	destroying it.
 */
static short alt_stat;	/* 1 = wait for expose, */
/* -1 = destroy upon expose */
static Boolean alt_canit;	/* stop drawing this window */

/*
 *	Data for buffered events.
 */

#ifndef FLAKY_SIGPOLL
static VOLATILE short event_freq = 70;
#else
#define	event_freq	70
#endif

static void can_exposures(struct WindowRec *windowrec);

#ifdef	GREY
#define	gamma	resource._gamma

static void
mask_shifts(Pixel mask, int *pshift1, int *pshift2)
{
    int k, l;

    for (k = 0; (mask & 1) == 0; ++k)
	mask >>= 1;
    for (l = 0; (mask & 1) == 1; ++l)
	mask >>= 1;
    *pshift1 = sizeof(short) * 8 - l;
    *pshift2 = k;
}

/*
 *	Try to allocate 4 color planes for 16 colors (for GXor drawing)
 */

void
init_plane_masks(void)
{
    Pixel pixel;

    if (copy || plane_masks[0] != 0)
	return;

    if (XAllocColorCells(DISP, our_colormap, False, plane_masks, 4, &pixel, 1)) {
	/* Make sure fore_Pixel and back_Pixel are a part of the palette */
	back_Pixel = pixel;
	fore_Pixel = pixel | plane_masks[0] | plane_masks[1]
	    | plane_masks[2] | plane_masks[3];
	if (mane.win != (Window) 0)
	    XSetWindowBackground(DISP, mane.win, back_Pixel);
    }
    else {
	copy = True;
	/* FIXME: toplevel yet realized? */
	do_popup_message(MSG_WARN,
			 /* helptext */
			 "Greyscaling is running in copy mode:  your display can only display \
a limited number of colors at a time (typically 256), and other applications\
 (such as netscape) are using many of them.  Running in copy mode will \
cause overstrike characters to appear incorrectly, and may result in \
poor display quality. \n\
You can either restart xdvi with the \"-install\" option \
to allow it to install its own color map, \
or terminate other color-hungry applications before restarting xdvi. \
Please see the section ``GREYSCALING AND COLORMAPS'' in the xdvi manual page \
for more details.",
			 /* text */
			 "Couldn't allocate enough colors - expect low display quality.");
	}
}

extern double pow();

#define	MakeGC(fcn, fg, bg)	(values.function = fcn, \
	  values.foreground=fg, values.background=bg, \
	  XCreateGC(DISP, XtWindow(top_level), \
	    GCFunction | GCForeground | GCBackground, &values))

void
init_pix(void)
{
    static int shrink_allocated_for = 0;
    static float oldgamma = 0.0;
    static Pixel palette[17];
    XGCValues values;
    unsigned int i;

    if (fore_color_data.pixel == back_color_data.pixel) {
	/* get foreground and background RGB values for interpolating */
	fore_color_data.pixel = fore_Pixel;
	XQueryColor(DISP, our_colormap, &fore_color_data);
	back_color_data.pixel = back_Pixel;
	XQueryColor(DISP, our_colormap, &back_color_data);
    }

    if (our_visual->class == TrueColor) {
	/* This mirrors the non-grey code in xdvi.c */
	static int shift1_r, shift1_g, shift1_b;
	static int shift2_r, shift2_g, shift2_b;
	static Pixel set_bits;
	static Pixel clr_bits;
	unsigned int sf_squared;

	if (oldgamma == 0.0) {
	    mask_shifts(our_visual->red_mask, &shift1_r, &shift2_r);
	    mask_shifts(our_visual->green_mask, &shift1_g, &shift2_g);
	    mask_shifts(our_visual->blue_mask, &shift1_b, &shift2_b);

	    set_bits = fore_color_data.pixel & ~back_color_data.pixel;
	    clr_bits = back_color_data.pixel & ~fore_color_data.pixel;

	    if (set_bits & our_visual->red_mask)
		set_bits |= our_visual->red_mask;
	    if (clr_bits & our_visual->red_mask)
		clr_bits |= our_visual->red_mask;
	    if (set_bits & our_visual->green_mask)
		set_bits |= our_visual->green_mask;
	    if (clr_bits & our_visual->green_mask)
		clr_bits |= our_visual->green_mask;
	    if (set_bits & our_visual->blue_mask)
		set_bits |= our_visual->blue_mask;
	    if (clr_bits & our_visual->blue_mask)
		clr_bits |= our_visual->blue_mask;

	    /*
	     * Make the GCs
	     */

	    foreGC = foreGC2 = ruleGC = 0;
	    copyGC = MakeGC(GXcopy, fore_Pixel, back_Pixel);
	    if (copy || (set_bits && clr_bits)) {
		ruleGC = copyGC;
		if (!resource.thorough)
		    copy = True;
	    }
	    if (copy) {
		foreGC = ruleGC;
		if (!resource.copy)
		    do_popup_message(MSG_WARN,
				     /* helptext */
				     "Greyscaling is running in copy mode:  your display can only display \
a limited number of colors at a time (typically 256), and other applications\
 (such as netscape) are using many of them.  Running in copy mode will \
cause overstrike characters to appear incorrectly, and may result in \
poor display quality. \n\
You can either restart xdvi with the \"-install\" option \
to allow it to install its own color map, \
or terminate other color-hungry applications before restarting xdvi. \
Please see the section ``GREYSCALING AND COLORMAPS'' in the xdvi manual page \
for more details.",
			 /* text */
			 "Couldn't allocate enough colors - expect low display quality.");
	    }
	    else {
		if (set_bits)
		    foreGC = MakeGC(GXor, set_bits & fore_color_data.pixel, 0);
		if (clr_bits || !set_bits)
		    *(foreGC ? &foreGC2 : &foreGC) = MakeGC(GXandInverted,
							    clr_bits &
							    ~fore_color_data.
							    pixel, 0);
		if (!ruleGC)
		    ruleGC = foreGC;
	    }

	    oldgamma = gamma;
	}

	if (mane.shrinkfactor == 1)
	    return;
	sf_squared = mane.shrinkfactor * mane.shrinkfactor;

	if (shrink_allocated_for < mane.shrinkfactor) {
	    if (pixeltbl != NULL) {
		free((char *)pixeltbl);
		if (pixeltbl_t != NULL)
		    free((char *)pixeltbl_t);
	    }
	    pixeltbl = xmalloc((sf_squared + 1) * sizeof(Pixel));
	    if (foreGC2 != NULL)
		pixeltbl_t = xmalloc((sf_squared + 1) * sizeof(Pixel));
	    shrink_allocated_for = mane.shrinkfactor;
	}

	/*
	 * Compute pixel values directly.
	 */

#define	SHIFTIFY(x, shift1, shift2)	((((Pixel)(x)) >> (shift1)) << (shift2))

	for (i = 0; i <= sf_squared; ++i) {
	    double frac = gamma > 0 ? pow((double)i / sf_squared, 1 / gamma)
		: 1 - pow((double)(sf_squared - i) / sf_squared, -gamma);
	    unsigned int red, green, blue;
	    Pixel pixel;

	    red = frac * ((double)fore_color_data.red - back_color_data.red)
		+ back_color_data.red;
	    green = frac
		* ((double)fore_color_data.green - back_color_data.green)
		+ back_color_data.green;
	    blue = frac * ((double)fore_color_data.blue - back_color_data.blue)
		+ back_color_data.blue;

	    pixel = SHIFTIFY(red, shift1_r, shift2_r) |
		SHIFTIFY(green, shift1_g, shift2_g) |
		SHIFTIFY(blue, shift1_b, shift2_b);

	    if (copy)
		pixeltbl[i] = pixel;
	    else {
		pixeltbl[i] = set_bits ? pixel & set_bits : ~pixel & clr_bits;
		if (pixeltbl_t != NULL)
		    pixeltbl_t[i] = ~pixel & clr_bits;
	    }
	}

#undef	SHIFTIFY

	return;
    }

    /* if not TrueColor ... */

    if (gamma != oldgamma) {
	XColor color;

	if (oldgamma == 0.0)
	    init_plane_masks();

	for (i = 0; i < 16; ++i) {
	    double frac = gamma > 0 ? pow((double)i / 15, 1 / gamma)
		: 1 - pow((double)(15 - i) / 15, -gamma);

	    color.red = frac
		* ((double)fore_color_data.red - back_color_data.red)
		+ back_color_data.red;
	    color.green = frac
		* ((double)fore_color_data.green - back_color_data.green)
		+ back_color_data.green;
	    color.blue = frac
		* ((double)fore_color_data.blue - back_color_data.blue)
		+ back_color_data.blue;

	    color.pixel = back_Pixel;
	    color.flags = DoRed | DoGreen | DoBlue;

	    if (!copy) {
		if (i & 1)
		    color.pixel |= plane_masks[0];
		if (i & 2)
		    color.pixel |= plane_masks[1];
		if (i & 4)
		    color.pixel |= plane_masks[2];
		if (i & 8)
		    color.pixel |= plane_masks[3];
		XStoreColor(DISP, our_colormap, &color);
		palette[i] = color.pixel;
	    }
	    else {
		if (XAllocColor(DISP, our_colormap, &color))
		    palette[i] = color.pixel;
		else
		    palette[i] = (i * 100 >= density * 15)
			? fore_Pixel : back_Pixel;
	    }
	}

	copyGC = MakeGC(GXcopy, fore_Pixel, back_Pixel);
	foreGC = ruleGC = copy ? copyGC : MakeGC(GXor, fore_Pixel, back_Pixel);
	foreGC2 = 0;

	oldgamma = gamma;
    }

    if (mane.shrinkfactor == 1)
	return;

    if (shrink_allocated_for < mane.shrinkfactor) {
	if (pixeltbl != NULL)
	    free((char *)pixeltbl);
	pixeltbl = xmalloc((unsigned)
			   (mane.shrinkfactor * mane.shrinkfactor + 1) * sizeof(Pixel));
	shrink_allocated_for = mane.shrinkfactor;
    }

    for (i = 0; i <= (unsigned)(mane.shrinkfactor * mane.shrinkfactor); ++i)
	pixeltbl[i] = palette[(i * 30 + mane.shrinkfactor * mane.shrinkfactor)
			      / (2 * mane.shrinkfactor * mane.shrinkfactor)];
}

#undef MakeGC



/*
 *	Ruler routines
 */

static int
tick_scale(int k)
{
    if (k == 0)
	return 3;
    else if ((k % 1000) == 0)
	return 7;
    else if ((k % 500) == 0)
	return 6;
    else if ((k % 100) == 0)
	return 5;
    else if ((k % 50) == 0)
	return 4;
    else if ((k % 10) == 0)
	return 3;
    else if ((k % 5) == 0)
	return 2;
    else
	return 1;
}


static void
draw_rulers(unsigned int width, unsigned int height, GC ourGC)
{
    int k;	/* tick counter */
    double old_pixels_per_tick;
    double pixels_per_tick;
    int scale;
    int tick_offset;	/* offset along axes */
    int x;	/* coordinates of top-left popup */
    int y;	/* window corner */
    double xx;	/* coordinates of tick */
    double yy;	/* coordinates of tick */
    static char *last_tick_units = "";	/* memory of last tick units */

    if (tick_length <= 0)	/* user doesn't want tick marks */
	return;

    x = 0;	/* the pop-up window always has origin (0,0)  */
    y = 0;

    /* We need to clear the existing window to remove old rulers.  I think
       that this could be avoided if draw_rulers() could be invoked earlier.
       The expose argument in XClearArea() must be True to force redrawing
       of the text inside the popup window. Also, it would be better to draw
       the rulers before painting the text, so that rulers would not
       overwrite the text, but I haven't figured out yet how to arrange
       that. */

    XClearArea(DISP, alt.win, x, y, width, height, True);

    /* The global resource._pixels_per_inch tells us how to find the ruler
       scale.  For example, 300dpi corresponds to these TeX units:

       1 TeX point (pt)     =   4.151      pixels
       1 big point (bp)     =   4.167      pixels
       1 pica (pc)          =  49.813      pixels
       1 cicero (cc)                =  53.501      pixels
       1 didot point (dd)   =   4.442      pixels
       1 millimeter (mm)    =  11.811      pixels
       1 centimeter (cm)    = 118.110      pixels
       1 inch (in)          = 300.000      pixels
       1 scaled point (sp)  =   0.00006334 pixels

       The user can select the units via a resource (e.g. XDvi*tickUnits: bp),
       or a command-line option (e.g. -xrm '*tickUnits: cm').  The length of
       the ticks can be controlled by a resource (e.g. XDvi*tickLength: 10), or
       a command-line option (e.g. -xrm '*tickLength: 10000').  If the tick
       length exceeds the popup window size, then a graph-paper grid is drawn
       over the whole window.  Zero, or negative, tick length completely
       suppresses rulers. */

    pixels_per_tick = (double)resource._pixels_per_inch;
    if (strcmp(tick_units, "pt") == 0)
	pixels_per_tick /= 72.27;
    else if (strcmp(tick_units, "bp") == 0)
	pixels_per_tick /= 72.0;
    else if (strcmp(tick_units, "in") == 0)
	/* NO-OP */ ;
    else if (strcmp(tick_units, "cm") == 0)
	pixels_per_tick /= 2.54;
    else if (strcmp(tick_units, "mm") == 0)
	pixels_per_tick /= 25.4;
    else if (strcmp(tick_units, "dd") == 0)
	pixels_per_tick *= (1238.0 / 1157.0) / 72.27;
    else if (strcmp(tick_units, "cc") == 0)
	pixels_per_tick *= 12.0 * (1238.0 / 1157.0) / 72.27;
    else if (strcmp(tick_units, "pc") == 0)
	pixels_per_tick *= 12.0 / 72.27;
    else if (strcmp(tick_units, "sp") == 0)
	pixels_per_tick /= (65536.0 * 72.27);
    else {
	Printf("Unrecognized tickUnits [%s]: defaulting to TeX points [pt]\n",
	       tick_units);
	tick_units = "pt";
	pixels_per_tick /= 72.27;
    }

    /* To permit accurate measurement in the popup window, we can reasonably
       place tick marks about 3 to 10 pixels apart, so we scale the computed
       pixels_per_tick by a power of ten to bring it into that range. */

    old_pixels_per_tick = pixels_per_tick;	/* remember the original scale */
    while (pixels_per_tick < 3.0)
	pixels_per_tick *= 10.0;
    while (pixels_per_tick > 30.0)
	pixels_per_tick /= 10.0;
    if (strcmp(last_tick_units, tick_units) != 0) {	/* tell user what the ruler scale is, but only when it changes */
	if (old_pixels_per_tick != pixels_per_tick)
	    Printf("Ruler tick interval adjusted to represent %.2f%s\n",
		   pixels_per_tick / old_pixels_per_tick, tick_units);
	else if (debug & DBG_EVENT)
	    Printf("Ruler tick interval represents 1%s\n", tick_units);
    }

    /* In order to make the ruler as accurate as possible, given the coarse
       screen resolution, we compute tick positions in floating-point
       arithmetic, then round to nearest integer values. */

    for (k = 0, xx = 0.0; xx < (double)width; k++, xx += pixels_per_tick) {	/* draw vertical ticks on top and bottom */
	tick_offset = (int)(0.5 + xx);	/* round to nearest pixel */
	scale = tick_scale(k);
	XDrawLine(DISP, alt.win, ourGC,
		  x + tick_offset, y, x + tick_offset, y + scale * tick_length);
	XDrawLine(DISP, alt.win, ourGC,
		  x + tick_offset, y + height,
		  x + tick_offset, y + height - scale * tick_length);
    }

    for (k = 0, yy = 0.0; yy < (double)height; k++, yy += pixels_per_tick) {	/* draw horizontal ticks on left and right */
	tick_offset = (int)(0.5 + yy);	/* round to nearest pixel */
	scale = tick_scale(k);
	XDrawLine(DISP, alt.win, ourGC,
		  x, y + tick_offset, x + scale * tick_length, y + tick_offset);
	XDrawLine(DISP, alt.win, ourGC,
		  x + width, y + tick_offset,
		  x + width - scale * tick_length, y + tick_offset);
    }

    last_tick_units = tick_units;

    XFlush(DISP);	/* bring window up-to-date */
}

#endif /* GREY */

int
check_goto_page(int pageno)
{
    if (pageno < 0) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Can't go to page %d, going to first page instead", pageno + 1);
	return 0;
    }
    else if (pageno >= total_pages) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT,
			 "Can't go to page %d, going to last page (%d) instead",
			 pageno + 1, total_pages);
	return total_pages - 1;
    }
    else
	return pageno;
}

/*
 *	Event-handling routines
 */

static void
expose(struct WindowRec *windowrec, int x, int y, unsigned int w, unsigned int h)
{
    if (windowrec->min_x > x)
	windowrec->min_x = x;
    if (windowrec->max_x < (int)(x + w))
	windowrec->max_x = x + w;
    if (windowrec->min_y > y)
	windowrec->min_y = y;
    if (windowrec->max_y < (int)(y + h))
	windowrec->max_y = y + h;
}

void
clearexpose(struct WindowRec *windowrec, int x, int y, unsigned int w, unsigned int h)
{
    XClearArea(DISP, windowrec->win, x, y, w, h, False);
    expose(windowrec, x, y, w, h);
}

static void
scrollwindow(struct WindowRec *windowrec, int x0, int y0)
{
    int x, y;
    int x2 = 0, y2 = 0;
    int ww, hh;

    x = x0 - windowrec->base_x;
    y = y0 - windowrec->base_y;
    ww = windowrec->width - x;
    hh = windowrec->height - y;
    windowrec->base_x = x0;
    windowrec->base_y = y0;
    if (currwin.win == windowrec->win) {
	currwin.base_x = x0;
	currwin.base_y = y0;
    }
    windowrec->min_x -= x;
    if (windowrec->min_x < 0)
	windowrec->min_x = 0;
    windowrec->max_x -= x;
    if (windowrec->max_x > (int)windowrec->width)
	windowrec->max_x = windowrec->width;
    windowrec->min_y -= y;
    if (windowrec->min_y < 0)
	windowrec->min_y = 0;
    windowrec->max_y -= y;
    if (windowrec->max_y > (int)windowrec->height)
	windowrec->max_y = windowrec->height;
    if (x < 0) {
	x2 = -x;
	x = 0;
	ww = windowrec->width - x2;
    }
    if (y < 0) {
	y2 = -y;
	y = 0;
	hh = windowrec->height - y2;
    }
    if (ww <= 0 || hh <= 0) {
	XClearWindow(DISP, windowrec->win);
	windowrec->min_x = windowrec->min_y = 0;
	windowrec->max_x = windowrec->width;
	windowrec->max_y = windowrec->height;
    }
    else {
	XCopyArea(DISP, windowrec->win, windowrec->win, copyGC,
		  x, y, (unsigned int)ww, (unsigned int)hh, x2, y2);
	if (x > 0)
	    clearexpose(windowrec, ww, 0, (unsigned int)x, windowrec->height);
	if (x2 > 0)
	    clearexpose(windowrec, 0, 0, (unsigned int)x2, windowrec->height);
	if (y > 0)
	    clearexpose(windowrec, 0, hh, windowrec->width, (unsigned int)y);
	if (y2 > 0)
	    clearexpose(windowrec, 0, 0, windowrec->width, (unsigned int)y2);
    }
}

#ifdef	TOOLKIT

/*
 *	routines for X11 toolkit
 */

static Arg arg_wh[] = {
    {XtNwidth, (XtArgVal) & window_w},
    {XtNheight, (XtArgVal) & window_h},
};

static Position window_x, window_y;
static Arg arg_xy[] = {
    {XtNx, (XtArgVal) & window_x},
    {XtNy, (XtArgVal) & window_y},
};

#define	get_xy()	XtGetValues(draw_widget, arg_xy, XtNumber(arg_xy))

#define	mane_base_x	0
#define	mane_base_y	0


#ifdef MOTIF

static int
set_bar_value(Widget bar, int value, int max)
{
    XmScrollBarCallbackStruct call_data;

    if (value > max)
	value = max;
    if (value < 0)
	value = 0;
    call_data.value = value;
    XtVaSetValues(bar, XmNvalue, value, NULL);
    XtCallCallbacks(bar, XmNvalueChangedCallback, &call_data);
    return value;
}

#endif /* MOTIF */

/* not static because SELFILE stuff in dvi_init.c needs it */
void
home(wide_bool scrl)
{
#if	PS
    psp.interrupt();
#endif
    if (!scrl)
	XUnmapWindow(DISP, mane.win);
#ifndef MOTIF
    get_xy();
    if (x_bar != NULL) {
	int coord = (page_w - clip_w) / 2;

	if (coord > home_x / mane.shrinkfactor)
	    coord = home_x / mane.shrinkfactor;
	XtCallCallbacks(x_bar, XtNscrollProc, (XtPointer) (window_x + coord));
    }
    if (y_bar != NULL) {
	int coord = (page_h - clip_h) / 2;

	if (coord > home_y / mane.shrinkfactor)
	    coord = home_y / mane.shrinkfactor;
	XtCallCallbacks(y_bar, XtNscrollProc, (XtPointer) (window_y + coord));
    }
#else /* MOTIF */
    {
	int value;

	value = (page_w - clip_w) / 2;
	if (value > home_x / mane.shrinkfactor)
	    value = home_x / mane.shrinkfactor;
	(void)set_bar_value(x_bar, value, (int)(page_w - clip_w));

	value = (page_h - clip_h) / 2;
	if (value > home_y / mane.shrinkfactor)
	    value = home_y / mane.shrinkfactor;
	(void)set_bar_value(y_bar, value, (int)(page_h - clip_h));
    }
#endif /* MOTIF */
    if (!scrl) {
	XMapWindow(DISP, mane.win);
	/* Wait for the server to catch up---this eliminates flicker. */
	XSync(DISP, False);
    }
}


#ifndef MOTIF
/*ARGSUSED*/
static void
handle_destroy_bar(Widget w, XtPointer client_data, XtPointer call_data)
{
    UNUSED(call_data);
    UNUSED(w);
    *(Widget *) client_data = NULL;
}
#endif


static Boolean resized = False;

static void
get_geom(void)
{
    static Dimension new_clip_w, new_clip_h;
    static Arg arg_wh_clip[] = {
	{XtNwidth, (XtArgVal) & new_clip_w},
	{XtNheight, (XtArgVal) & new_clip_h},
    };
    int old_clip_w;

    XtGetValues(vport_widget, arg_wh, XtNumber(arg_wh));
    XtGetValues(clip_widget, arg_wh_clip, XtNumber(arg_wh_clip));
#ifndef MOTIF
    /* Note:  widgets may be destroyed but not forgotten */
    if (x_bar == NULL) {
	x_bar = XtNameToWidget(vport_widget, "horizontal");
	if (x_bar != NULL)
	    XtAddCallback(x_bar, XtNdestroyCallback, handle_destroy_bar,
			  (XtPointer) & x_bar);
    }
    if (y_bar == NULL) {
	y_bar = XtNameToWidget(vport_widget, "vertical");
	if (y_bar != NULL)
	    XtAddCallback(y_bar, XtNdestroyCallback, handle_destroy_bar,
			  (XtPointer) & y_bar);
    }
#endif
    old_clip_w = clip_w;
    /* we need to do this because */
    /* sizeof(Dimension) != sizeof(int) */
    clip_w = new_clip_w;
    clip_h = new_clip_h;
    if (old_clip_w == 0)
	home(False);
    resized = False;
}

/*
 *	callback routines
 */

/*ARGSUSED*/ void
handle_resize(Widget widget, XtPointer junk, XEvent *event, Boolean *cont)
{
    UNUSED(cont);
    UNUSED(event);
    UNUSED(junk);
    UNUSED(widget);
    resized = True;
#ifdef STATUSLINE
    handle_statusline_resize();
#endif
}

#ifdef	BUTTONS
static void
handle_command(Widget widget, XtPointer client_data, XtPointer call_data)
{
    struct xdvi_action *actp;

    UNUSED(call_data);
    for (actp = (struct xdvi_action *)client_data;
	 actp != NULL;
	 actp = actp->next)
    (actp->proc) (widget, NULL, &actp->param, &actp->num_params);
}

 /*ARGSUSED*/ static void
handle_destroy_buttons(Widget w, XtPointer client_data, XtPointer call_data)
{
    UNUSED(call_data);
    UNUSED(client_data);
    UNUSED(w);
    if (--destroy_count != 0) {
	return;
    }
#ifndef MOTIF
    XtSetValues(vport_widget, resizable_on, XtNumber(resizable_on));
    if (resource.expert) {
	/* destroy buttons */
	XtGetValues(form_widget, arg_wh, XtNumber(arg_wh));
	XdviResizeWidget(vport_widget, window_w, window_h);
    }
    else {
	create_buttons();	/* this determines xtra_wid */
	XdviResizeWidget(vport_widget, window_w -= xtra_wid, window_h);
	set_button_panel_height((XtArgVal) window_h);
    }
#else /* MOTIF */
    if (resource.expert) {
	XtVaSetValues(vport_widget,
		      XmNleftAttachment, XmATTACH_FORM, XmNleftOffset, 0, NULL);
    }
    else {
	create_buttons();
	window_w -= xtra_wid;
    }
#endif /* MOTIF */
}
#endif /* BUTTONS */

void
reconfig(void)
{
#if BUTTONS && !MOTIF
    XtSetValues(vport_widget, resizable_off, XtNumber(resizable_off));
#endif
    XdviResizeWidget(draw_widget, page_w, page_h);
#ifdef STATUSLINE
    handle_statusline_resize();
#endif
    get_geom();
}

#else /* not TOOLKIT */

/*
 *	brute force scrollbar routines
 */

static void
paint_x_bar(void)
{
    int new_x_bgn = mane.base_x * clip_w / page_w;
    int new_x_end = (mane.base_x + clip_w) * clip_w / page_w;

    if (new_x_bgn >= x_end || x_bgn >= new_x_end) {	/* no overlap */
	XClearArea(DISP, x_bar, x_bgn, 1, x_end - x_bgn, BAR_WID, False);
	XFillRectangle(DISP, x_bar, ruleGC,
		       new_x_bgn, 1, new_x_end - new_x_bgn, BAR_WID);
    }
    else {	/* this stuff avoids flicker */
	if (x_bgn < new_x_bgn)
	    XClearArea(DISP, x_bar, x_bgn, 1, new_x_bgn - x_bgn,
		       BAR_WID, False);
	else
	    XFillRectangle(DISP, x_bar, ruleGC,
			   new_x_bgn, 1, x_bgn - new_x_bgn, BAR_WID);
	if (new_x_end < x_end)
	    XClearArea(DISP, x_bar, new_x_end, 1, x_end - new_x_end,
		       BAR_WID, False);
	else
	    XFillRectangle(DISP, x_bar, ruleGC,
			   x_end, 1, new_x_end - x_end, BAR_WID);
    }
    x_bgn = new_x_bgn;
    x_end = new_x_end;
}

static void
paint_y_bar(void)
{
    int new_y_bgn = mane.base_y * clip_h / page_h;
    int new_y_end = (mane.base_y + clip_h) * clip_h / page_h;

    if (new_y_bgn >= y_end || y_bgn >= new_y_end) {	/* no overlap */
	XClearArea(DISP, y_bar, 1, y_bgn, BAR_WID, y_end - y_bgn, False);
	XFillRectangle(DISP, y_bar, ruleGC,
		       1, new_y_bgn, BAR_WID, new_y_end - new_y_bgn);
    }
    else {	/* this stuff avoids flicker */
	if (y_bgn < new_y_bgn)
	    XClearArea(DISP, y_bar, 1, y_bgn, BAR_WID, new_y_bgn - y_bgn,
		       False);
	else
	    XFillRectangle(DISP, y_bar, ruleGC,
			   1, new_y_bgn, BAR_WID, y_bgn - new_y_bgn);
	if (new_y_end < y_end)
	    XClearArea(DISP, y_bar, 1, new_y_end,
		       BAR_WID, y_end - new_y_end, False);
	else
	    XFillRectangle(DISP, y_bar, ruleGC,
			   1, y_end, BAR_WID, new_y_end - y_end);
    }
    y_bgn = new_y_bgn;
    y_end = new_y_end;
}

static void
scrollmane(int x, int y)
{
    int old_base_x = mane.base_x;
    int old_base_y = mane.base_y;

#if	PS
    psp.interrupt();
#endif
    if (x > (int)(page_w - clip_w))
	x = page_w - clip_w;
    if (x < 0)
	x = 0;
    if (y > (int)(page_h - clip_h))
	y = page_h - clip_h;
    if (y < 0)
	y = 0;
    scrollwindow(&mane, x, y);
    if (old_base_x != mane.base_x && x_bar)
	paint_x_bar();
    if (old_base_y != mane.base_y && y_bar)
	paint_y_bar();
}

void
reconfig(void)
{
    int x_thick = 0;
    int y_thick = 0;

    /* determine existence of scrollbars */
    if (window_w < page_w)
	x_thick = BAR_THICK;
    if (window_h - x_thick < page_h)
	y_thick = BAR_THICK;
    clip_w = window_w - y_thick;
    if (clip_w < page_w)
	x_thick = BAR_THICK;
    clip_h = window_h - x_thick;

    /* process drawing (clip) window */
    if (mane.win == (Window) 0) {	/* initial creation */
	XWindowAttributes attrs;

	mane.win = XCreateSimpleWindow(DISP, top_level, y_thick, x_thick,
				       (unsigned int)clip_w,
				       (unsigned int)clip_h, 0, brdr_Pixel,
				       back_Pixel);
	XSelectInput(DISP, mane.win, ExposureMask |
#ifdef HTEX
		     /* SU: need only PointerMotionHintMask? */
		     /* PointerMotionMask | PointerMotionHintMask | */
		     PointerMotionHintMask | ButtonPressMask | ButtonReleaseMask
#else
		     ButtonPressMask | ButtonMotionMask | ButtonReleaseMask
#endif
	    );
	(void)XGetWindowAttributes(DISP, mane.win, &attrs);
	backing_store = attrs.backing_store;
	XMapWindow(DISP, mane.win);
    }
    else
	XMoveResizeWindow(DISP, mane.win, y_thick, x_thick, clip_w, clip_h);

    /* process scroll bars */
    if (x_thick) {
	if (x_bar) {
	    XMoveResizeWindow(DISP, x_bar,
			      y_thick - 1, -1, clip_w, BAR_THICK - 1);
	    paint_x_bar();
	}
	else {
	    x_bar = XCreateSimpleWindow(DISP, top_level, y_thick - 1, -1,
					(unsigned int)clip_w, BAR_THICK - 1, 1,
					brdr_Pixel, back_Pixel);
	    XSelectInput(DISP, x_bar,
			 ExposureMask | ButtonPressMask | Button2MotionMask);
	    XMapWindow(DISP, x_bar);
	}
	x_bgn = mane.base_x * clip_w / page_w;
	x_end = (mane.base_x + clip_w) * clip_w / page_w;
    }
    else if (x_bar) {
	XDestroyWindow(DISP, x_bar);
	x_bar = (Window) 0;
    }

    if (y_thick) {
	if (y_bar) {
	    XMoveResizeWindow(DISP, y_bar,
			      -1, x_thick - 1, BAR_THICK - 1, clip_h);
	    paint_y_bar();
	}
	else {
	    y_bar = XCreateSimpleWindow(DISP, top_level, -1, x_thick - 1,
					BAR_THICK - 1, (unsigned int)clip_h, 1,
					brdr_Pixel, back_Pixel);
	    XSelectInput(DISP, y_bar,
			 ExposureMask | ButtonPressMask | Button2MotionMask);
	    XMapWindow(DISP, y_bar);
	}
	y_bgn = mane.base_y * clip_h / page_h;
	y_end = (mane.base_y + clip_h) * clip_h / page_h;
    }
    else if (y_bar) {
	XDestroyWindow(DISP, y_bar);
	y_bar = (Window) 0;
    }
}

void
home(wide_bool scrl)
{
    int x = 0, y = 0;

    if (page_w > clip_w) {
	x = (page_w - clip_w) / 2;
	if (x > home_x / mane.shrinkfactor)
	    x = home_x / mane.shrinkfactor;
    }
    if (page_h > clip_h) {
	y = (page_h - clip_h) / 2;
	if (y > home_y / mane.shrinkfactor)
	    y = home_y / mane.shrinkfactor;
    }
    if (scrl)
	scrollmane(x, y);
    else {
	mane.base_x = x;
	mane.base_y = y;
	if (currwin.win == mane.win) {
	    currwin.base_x = x;
	    currwin.base_y = y;
	}
	if (x_bar)
	    paint_x_bar();
	if (y_bar)
	    paint_y_bar();
    }
}

#define	get_xy()
#define	window_x 0
#define	window_y 0
#define	mane_base_x	mane.base_x
#define	mane_base_y	mane.base_y
#endif /* not TOOLKIT */


#ifdef MOTIF

struct pd_act {
    XtActionProc proc;
    Cardinal num_params;
    String param;
};

static void
do_pulldown_action(struct pd_act *actp)
{
    (actp->proc) ( /* widget */ NULL, NULL, &actp->param, &actp->num_params);
}

static struct pd_act file_pulldown_actions[] = {
    {Act_reread_dvi_file, 0, NULL},
    {Act_quit, 0, NULL}
};

/* ARGSUSED */
void
file_pulldown_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    do_pulldown_action(file_pulldown_actions + (int)client_data);
}

static struct pd_act navigate_pulldown_actions[] = {
    {Act_back_page, 1, "10"},
    {Act_back_page, 1, "5"},
    {Act_back_page, 0, NULL},
    {Act_forward_page, 0, NULL},
    {Act_forward_page, 1, "5"},
    {Act_forward_page, 1, "10"}
};

/* ARGSUSED */
void
navigate_pulldown_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    do_pulldown_action(navigate_pulldown_actions + (int)client_data);
}


static struct pd_act scale_pulldown_actions[] = {
    {Act_set_shrink_factor, 1, "1"},
    {Act_set_shrink_factor, 1, "2"},
    {Act_set_shrink_factor, 1, "3"},
    {Act_set_shrink_factor, 1, "4"}
};

/* ARGSUSED */
void
scale_pulldown_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    do_pulldown_action(scale_pulldown_actions + (int)client_data);
}

void
set_shrink_factor(int shrink)
{
    static Widget active_shrink_button = NULL;
    Widget new_shrink_button;

    mane.shrinkfactor = shrink;
    new_shrink_button = (shrink > 0 && shrink <= XtNumber(shrink_button)
			 ? shrink_button[shrink - 1] : NULL);
    if (new_shrink_button != active_shrink_button) {
	if (active_shrink_button != NULL)
	    XmToggleButtonSetState(active_shrink_button, False, False);
	if (new_shrink_button != NULL)
	    XmToggleButtonSetState(new_shrink_button, True, False);
	active_shrink_button = new_shrink_button;
    }
#ifdef STATUSLINE
    handle_statusline_resize();
#endif
}

#endif /* MOTIF */

void
showmessage(_Xconst char *message)
{
    get_xy();
    XDrawImageString(DISP, mane.win, copyGC,
		     5 - window_x, 5 + X11HEIGHT - window_y, message,
		     strlen(message));
}

/* |||
 *	Currently the event handler does not coordinate XCopyArea requests
 *	with GraphicsExpose events.  This can lead to problems if the window
 *	is partially obscured and one, for example, drags a scrollbar.
 */

/*
 *	Actions for the translation mechanism.
 */

static Boolean have_arg = False;
static int number = 0;
static int sign = 1;

#if TOOLKIT

#define	GET_ARG4(arg, param, param2, default)		\
		if (*num_params > 0)			\
		    {param;}				\
		else {					\
		    if (have_arg) {			\
			arg = (param2);			\
			have_arg = False;		\
			number = 0;			\
			sign = 1;			\
			print_statusline(STATUS_SHORT, "");\
		    }					\
		    else				\
			{default}			\
		}

#define	GET_ARG(arg, default)				\
		GET_ARG4(arg, arg = atoi(*params), sign * number, \
		  arg = (default);)

#define	GET_ARG6(arg, param, c, param_c, param2, default)\
		if (*num_params > 0) {			\
		    if (**params == (c))		\
			{param_c;}			\
		    else				\
			{param;}			\
		}					\
		else {					\
		    if (have_arg) {			\
			arg = (param2);			\
			have_arg = False;		\
			number = 0;			\
			sign = 1;			\
			print_statusline(STATUS_SHORT, "");\
		    }					\
		    else				\
			{default;}			\
		}

#define	TOGGLE(arg)							\
	if (*num_params > 0) {						\
	    if (**params != 't' && (atoi(*params) != 0) == arg)		\
		return;							\
	}								\
	else {								\
	    if (have_arg) {						\
		int	tmparg = number;				\
									\
		have_arg = False;					\
		number = 0;						\
		sign = 1;						\
									\
		if ((tmparg != 0) == arg)				\
		    return;						\
	    }								\
	}

#else /* not TOOLKIT */

static unsigned char keychar;

#define	GET_ARG4(arg, param, param2, default)		\
		if (have_arg) {				\
		    arg = (param2);			\
		    have_arg = False;			\
		    number = 0;				\
		    sign = 1;				\
		}					\
		else					\
		    {default}

#define	GET_ARG(arg, default)				\
		GET_ARG4(arg, NA, sign * number, arg = (default);)

#define	GET_ARG6(arg, param, c, param_c, param2, default)\
		if (have_arg) {				\
		    arg = (param2);			\
		    have_arg = False;			\
		    number = 0;				\
		    sign = 1;				\
		}					\
		else					\
		    {default;}

#define	TOGGLE(arg)					\
		if (have_arg) {				\
		    int	tmparg = number;		\
							\
		    have_arg = False;			\
		    number = 0;				\
		    sign = 1;				\
							\
		    if ((tmparg != 0) == arg)		\
			return;				\
		}

#endif /* not TOOLKIT */

static void
do_goto_page(int pageno)
{
    if (current_page != pageno) {
	current_page = pageno;
	warn_spec_now = warn_spec;
	if (!resource.keep_flag)
	    home(False);
    }
    /* Control-L (and changing the page) clears this box */
    source_fwd_box_page = -1;
    canit = True;
    XFlush(DISP);
}

static
ACTION(Act_digit)
{
    unsigned int digit;
    UNUSED(w);
    UNUSED(event);

#if TOOLKIT
    if (*num_params != 1 || (digit = **params - '0') > 9) {
	XBell(DISP, 10);
	return;
    }
#else
    digit = keychar - '0';
#endif
    have_arg = True;
    number = number * 10 + digit;
    print_statusline(STATUS_SHORT, "numerical prefix: %s%d\n", sign < 0 ? "-" : "", number);
}

static
ACTION(Act_minus)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    have_arg = True;
    sign = -sign;
    if (number > 0)
	print_statusline(STATUS_SHORT, "numerical prefix: %s%d\n", sign < 0 ? "-" : "", number);
    else
	print_statusline(STATUS_SHORT, "numerical prefix: %s\n", sign < 0 ? "-" : "");
}

static
ACTION(Act_quit)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

#if !FLAKY_SIGPOLL
    if (debug & DBG_EVENT)
	puts(event_freq < 0
	     ? "SIGPOLL is working" : "no SIGPOLL signals received");
#endif
    xdvi_exit(0);
}

static
ACTION(Act_help)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

#ifdef TOOLKIT
    show_help(top_level);
#else
    show_help();
#endif /* TOOLKIT */
}


static
ACTION(Act_goto_page)
{
    int arg;
    UNUSED(w);
    UNUSED(event);

    GET_ARG6(arg, arg = atoi(*params) - pageno_correct,
	     'e', arg = total_pages - 1,
	     sign * number - pageno_correct, arg = total_pages - 1);

    do_goto_page(check_goto_page(arg));
    return;	/* Don't use longjmp here:  it might be called from
		   * within the toolkit, and we don't want to longjmp out
		   * of Xt routines. */
}

static
ACTION(Act_forward_page)
{
    int arg, bak;
    UNUSED(w);
    UNUSED(event);

#ifdef PS_GS
    if (FIXME_ps_lock)
	return;
#endif
    
    GET_ARG(arg, 1);
    bak = arg;
    arg += current_page;

    if (current_page == total_pages - 1) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Last page of dvi file");
	return;
    }

    do_goto_page(check_goto_page(arg));
}

static
ACTION(Act_back_page)
{
    int arg, bak;
    UNUSED(w);
    UNUSED(event);

#ifdef PS_GS
    if (FIXME_ps_lock)
	return;
#endif

    GET_ARG(arg, 1);
    bak = arg;
    arg = current_page - arg;

    if (current_page == 0) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "First page of dvi file");
	return;
    }
    do_goto_page(check_goto_page(arg));
}

static
ACTION(Act_declare_page_number)
{
    int arg;
    UNUSED(w);
    UNUSED(event);

    GET_ARG(arg, 0);
    pageno_correct = arg - current_page;
    print_statusline(STATUS_SHORT, "Current page number set to %d", arg);
}

static
ACTION(Act_home)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    home(True);
}

static
ACTION(Act_center)
{
    int x, y;
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

#if BUTTONS
    if (event == NULL)
	return;	/* button actions do not provide events */
#endif

#if TOOLKIT
#if !MOTIF

    x = event->xkey.x - clip_w / 2;
    y = event->xkey.y - clip_h / 2;
    /* The clip widget gives a more exact value. */
    if (x_bar != NULL)
	XtCallCallbacks(x_bar, XtNscrollProc, (XtPointer) x);
    if (y_bar != NULL)
	XtCallCallbacks(y_bar, XtNscrollProc, (XtPointer) y);
    XWarpPointer(DISP, None, None, 0, 0, 0, 0, -x, -y);

#else /* MOTIF */

    get_xy();
    /* The clip widget gives a more exact value. */
    x = event->xkey.x - clip_w / 2;
    y = event->xkey.y - clip_h / 2;

    x = set_bar_value(x_bar, x, (int)(page_w - clip_w));
    y = set_bar_value(y_bar, y, (int)(page_h - clip_h));
    XWarpPointer(DISP, None, None, 0, 0, 0, 0, -x - window_x, -y - window_y);

#endif /* MOTIF */
#else /* not TOOLKIT */

    x = clip_w / 2 - event->xkey.x;
    if (x > mane.base_x)
	x = mane.base_x;
    y = clip_h / 2 - event->xkey.y;
    if (y > mane.base_y)
	y = mane.base_y;
    scrollwindow(&mane, mane.base_x - x, mane.base_y - y);
    if (x_bar)
	paint_x_bar();
    if (y_bar)
	paint_y_bar();
    XWarpPointer(DISP, None, None, 0, 0, 0, 0, x, y);

#endif /* not TOOLKIT */

}

static
ACTION(Act_set_keep_flag)
{
    UNUSED(w);
    UNUSED(event);
#if TOOLKIT
    if (*num_params == 0) {
#endif
	if (have_arg) {
	    resource.keep_flag = (number != 0);
	    have_arg = False;
	    number = 0;
	    sign = 1;
	}
	else {
	    resource.keep_flag = !resource.keep_flag;
	}
#if TOOLKIT
    }
    else
	resource.keep_flag = (**params == 't'
			      ? !resource.keep_flag : atoi(*params));
#endif
    if (resource.keep_flag) {
	print_statusline(STATUS_SHORT, "Keeping position when switching pages");
    }
    else {
	print_statusline(STATUS_SHORT,
			 "Not keeping position when switching pages");
    }
}

static
ACTION(Act_left)
{
    UNUSED(w);
    UNUSED(event);
#if TOOLKIT
#if !MOTIF
    if (x_bar != NULL)
	XtCallCallbacks(x_bar, XtNscrollProc,
			(XtPointer) (*num_params == 0 ? (-2 * (int)clip_w / 3)
				     : (int)(-atof(*params) * clip_w)));
    else {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Horizontal scrolling not possible");
    }
#else /* MOTIF */
    get_xy();
    (void)set_bar_value(x_bar, (*num_params == 0 ? (-2 * (int)clip_w / 3)
				: (int)(-atof(*params) * clip_w)) - window_x,
			(int)(page_w - clip_w));
#endif /* MOTIF */
#else /* not TOOLKIT */
    if (mane.base_x > 0)
	scrollmane(mane.base_x - 2 * (int)clip_w / 3, mane.base_y);
    else
	XBell(DISP, 10);
#endif /* not TOOLKIT */
}

static
ACTION(Act_right)
{
    UNUSED(w);
    UNUSED(event);
#if TOOLKIT
#if !MOTIF
    if (x_bar != NULL)
	XtCallCallbacks(x_bar, XtNscrollProc,
			(XtPointer) (*num_params == 0 ? (2 * (int)clip_w / 3)
				     : (int)(atof(*params) * clip_w)));
    else {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Horizontal scrolling not possible");
    }
#else /* MOTIF */
    get_xy();
    (void)set_bar_value(x_bar, (*num_params == 0 ? (2 * (int)clip_w / 3)
				: (int)(atof(*params) * clip_w)) - window_x,
			(int)(page_w - clip_w));
#endif /* MOTIF */
#else /* not TOOLKIT */
    if (mane.base_x < (int)page_w - (int)clip_w)
	scrollmane(mane.base_x + 2 * (int)clip_w / 3, mane.base_y);
    else
	XBell(DISP, 10);
#endif /* not TOOLKIT */
}

static
ACTION(Act_up)
{
    UNUSED(w);
    UNUSED(event);
#if TOOLKIT
#if !MOTIF
    if (y_bar != NULL)
	XtCallCallbacks(y_bar, XtNscrollProc,
			(XtPointer) (*num_params == 0 ? (-2 * (int)clip_h / 3)
				     : (int)(-atof(*params) * clip_h)));
    else {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Vertical scrolling not possible");
    }
#else /* MOTIF */
    get_xy();
    (void)set_bar_value(y_bar, (*num_params == 0 ? (-2 * (int)clip_h / 3)
				: (int)(-atof(*params) * clip_h)) - window_y,
			(int)(page_h - clip_h));
#endif /* MOTIF */
#else /* not TOOLKIT */
    if (mane.base_y > 0)
	scrollmane(mane.base_x, mane.base_y - 2 * (int)clip_h / 3);
    else
	XBell(DISP, 10);
#endif /* not TOOLKIT */
}

static
ACTION(Act_down)
{
    UNUSED(w);
    UNUSED(event);
#if TOOLKIT
#if !MOTIF
    if (y_bar != NULL)
	XtCallCallbacks(y_bar, XtNscrollProc,
			(XtPointer) (*num_params == 0 ? (2 * (int)clip_h / 3)
				     : (int)(atof(*params) * clip_h)));
    else {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "Vertical scrolling not possible");
    }
#else /* MOTIF */
    get_xy();
    (void)set_bar_value(y_bar, (*num_params == 0 ? (2 * (int)clip_h / 3)
				: (int)(atof(*params) * clip_h)) - window_y,
			(int)(page_h - clip_h));
#endif /* MOTIF */
#else /* not TOOLKIT */
    if (mane.base_y < (int)page_h - (int)clip_h)
	scrollmane(mane.base_x, mane.base_y + 2 * (int)clip_h / 3);
    else
	XBell(DISP, 10);
#endif /* not TOOLKIT */
}

static
ACTION(Act_down_or_next)
{
    UNUSED(w);
    UNUSED(event);
#ifdef PS_GS
    if (FIXME_ps_lock)
	return;
#endif

    if (!resource.keep_flag) {
#if TOOLKIT
#if !MOTIF
	if (y_bar != NULL) {
	    get_xy();
	    if (window_y > (int)clip_h - (int)page_h) {
		XtCallCallbacks(y_bar, XtNscrollProc,
				(XtPointer) (*num_params ==
					     0 ? (2 * (int)clip_h / 3)
					     : (int)(atof(*params) * clip_h)));
		return;
	    }
	}
#else /* MOTIF */
	get_xy();
	if (window_y > (int)clip_h - (int)page_h) {
	    (void)set_bar_value(y_bar, (*num_params == 0 ? (2 * (int)clip_h / 3)
					: (int)(atof(*params) * clip_h)) -
				window_y, (int)(page_h - clip_h));
	    return;
	}
#endif /* MOTIF */
#else /* not TOOLKIT */
	if (mane.base_y < (int)page_h - (int)clip_h) {
	    scrollmane(mane.base_x, mane.base_y + 2 * (int)clip_h / 3);
	    return;
	}
#endif /* not TOOLKIT */
    }	/* !keep_flag */

    if (current_page < total_pages - 1) {
	++current_page;
	warn_spec_now = warn_spec;
	if (!resource.keep_flag)
	    home(False);
	canit = True;
	XFlush(DISP);
	return;	/* Don't use longjmp here:  it might be called from
		   * within the toolkit, and we don't want to longjmp out
		   * of Xt routines. */
    }
    else {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "At bottom of last page");
    }
}


static
ACTION(Act_up_or_previous)
{
    UNUSED(w);
    UNUSED(event);
#ifdef PS_GS
    if (FIXME_ps_lock)
	return;
#endif

    if (!resource.keep_flag) {
#if TOOLKIT
#if !MOTIF
	if (y_bar != NULL) {
	    get_xy();
	    if (window_y < 0) {
		XtCallCallbacks(y_bar, XtNscrollProc,
				(XtPointer) (*num_params ==
					     0 ? (-2 * (int)clip_h / 3)
					     : (int)(-atof(*params) * clip_h)));
		return;
	    }
	}
#else /* MOTIF */
	get_xy();
	if (window_y < 0) {
	    (void)set_bar_value(y_bar,
				(*num_params == 0 ? (-2 * (int)clip_h / 3)
				 : (int)(-atof(*params) * clip_h)) - window_y,
				(int)(page_h - clip_h));
	    return;
	}
#endif /* MOTIF */
#else /* not TOOLKIT */
	if (mane.base_y > 0) {
	    scrollmane(mane.base_x, mane.base_y - 2 * (int)clip_h / 3);
	    return;
	}
#endif /* not TOOLKIT */
    }	/* !keep_flag */

    if (current_page > 0) {
	--current_page;
	warn_spec_now = warn_spec;
	if (!resource.keep_flag) {
	    /* home(False); except move to the bottom of the page */
#if TOOLKIT
#if PS
	    psp.interrupt();
#endif
	    XUnmapWindow(DISP, mane.win);
#ifndef MOTIF
	    get_xy();
	    if (x_bar != NULL) {
		int coord = (page_w - clip_w) / 2;

		if (coord > home_x / mane.shrinkfactor)
		    coord = home_x / mane.shrinkfactor;
		XtCallCallbacks(x_bar, XtNscrollProc,
				(XtPointer) (window_x + coord));
	    }
	    if (y_bar != NULL)
		XtCallCallbacks(y_bar, XtNscrollProc,
				(XtPointer) (window_y + (page_h - clip_h)));
#else /* MOTIF */
	    {
		int value;

		value = (page_w - clip_w) / 2;
		if (value > home_x / mane.shrinkfactor)
		    value = home_x / mane.shrinkfactor;
		(void)set_bar_value(x_bar, value, (int)(page_w - clip_w));

		(void)set_bar_value(y_bar, (int)(page_h - clip_h),
				    (int)(page_h - clip_h));
	    }
#endif /* MOTIF */
	    XMapWindow(DISP, mane.win);
	    /* Wait for the server to catch up---this eliminates flicker. */
	    XSync(DISP, False);
#else /* not TOOLKIT */
	    int x = 0, y = 0;

	    if (page_w > clip_w) {
		x = (page_w - clip_w) / 2;
		if (x > home_x / mane.shrinkfactor)
		    x = home_x / mane.shrinkfactor;
	    }
	    if (page_h > clip_h)
		y = page_h - clip_h;
	    mane.base_x = x;
	    mane.base_y = y;
	    if (currwin.win == mane.win) {
		currwin.base_x = x;
		currwin.base_y = y;
	    }
	    if (x_bar)
		paint_x_bar();
	    if (y_bar)
		paint_y_bar();
#endif /* not TOOLKIT */
	}	/* !keep_flag */
	canit = True;
	XFlush(DISP);
	return;	/* Don't use longjmp here:  it might be called from
		 * within the toolkit, and we don't want to longjmp out
		 * of Xt routines. */
    }
    else
	XBell(DISP, 10);
}


static
ACTION(Act_set_margins)
{
    Window ww;
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

#if BUTTONS
    if (event == NULL)
	return;	/* button actions do not provide events */
#endif

    (void)XTranslateCoordinates(DISP, event->xkey.window, mane.win,
				event->xkey.x, event->xkey.y, &home_x, &home_y, &ww);	/* throw away last argument */
    print_statusline(STATUS_SHORT, "Margins set to cursor position (%d, %d)", home_x, home_y);
    home_x *= mane.shrinkfactor;
    home_y *= mane.shrinkfactor;
}

static
ACTION(Act_show_display_attributes)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    print_statusline(STATUS_SHORT, "Unit = %d, bitord = %d, byteord = %d",
		     BitmapUnit(DISP), BitmapBitOrder(DISP),
		     ImageByteOrder(DISP));
}

static int
shrink_to_fit(void)
{
    int value1;
    int value2;

    value1 = ROUNDUP(unshrunk_page_w, window_w - 2);

#if !MOTIF
    value2 = ROUNDUP(unshrunk_page_h, window_h - 2);
#else /* MOTIF */
    {	/* account for menubar */
	static Dimension new_h;

	/* get rid of scrollbar */
	XdviResizeWidget(draw_widget, 1, 1);
	XtVaGetValues(clip_widget, XtNheight, &new_h, NULL);
	value2 = ROUNDUP(unshrunk_page_h, new_h - 2);
    }
#endif /* MOTIF */

    return value1 > value2 ? value1 : value2;
}

static
ACTION(Act_set_shrink_factor)
{
    int arg;
    UNUSED(w);
    UNUSED(event);

    GET_ARG6(arg, arg = atoi(*params), 'a', arg = shrink_to_fit(),
	     number, arg = shrink_to_fit());

    if (arg <= 0) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT,
			 "set-shrink-factor requires a positive argument");
	return;
    }

    print_statusline(STATUS_SHORT, "shrink factor: %d", arg);
    if (arg == mane.shrinkfactor)
	return;
#if !MOTIF
    mane.shrinkfactor = arg;
#else
    set_shrink_factor(arg);
#endif
    init_page();
    if (arg != 1 && arg != bak_shrink) {
	bak_shrink = arg;
#if GREY
	if (use_grey)
	    init_pix();
#endif
	reset_fonts();
    }
    reconfig();
    home(False);
    canit = True;
    XFlush(DISP);
}

static
ACTION(Act_shrink_to_dpi)
{
    int arg;
    UNUSED(w);
    UNUSED(event);

    GET_ARG(arg, 0);

    if (arg > 0)
	arg = (double)pixels_per_inch / arg + 0.5;

    if (arg <= 0) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT,
			 "shrink-to-dpi requires a positive argument");
	return;
    }

    if (arg == mane.shrinkfactor)
	return;
#if !MOTIF
    mane.shrinkfactor = arg;
#else
    set_shrink_factor(arg);
#endif
    init_page();
    if (arg != 1 && arg != bak_shrink) {
	bak_shrink = arg;
#if GREY
	if (use_grey)
	    init_pix();
#endif
	reset_fonts();
    }
    reconfig();
    home(False);
    canit = True;
    XFlush(DISP);
}

static
ACTION(Act_set_density)
{
    int arg;
    unsigned int u_arg;
    UNUSED(w);
    UNUSED(event);

    GET_ARG4(arg, arg = atoi(*params), sign * number, {
	     XBell(DISP, 10);
	     return;
	     }
    );

#if GREY
    if (use_grey) {
	float newgamma = arg != 0 ? arg / 100.0 : 1.0;

	if (newgamma == gamma) {
	    print_statusline(STATUS_SHORT, "density value: %.1f", newgamma);
	    return;
	}
	gamma = newgamma;
	init_pix();
	if (our_visual->class != TrueColor) {
	    return;
	}
	reset_fonts();
	print_statusline(STATUS_SHORT, "density value: %.1f", newgamma);
    }
    else
#endif
    {
	if (arg < 0) {
	    XBell(DISP, 10);
	    print_statusline(STATUS_SHORT,
			     "set-density requires a positive value");
	    return;
	}
	u_arg = (unsigned)arg;
	print_statusline(STATUS_SHORT, "density: %u", u_arg);
	if (u_arg == density) {
	    print_statusline(STATUS_SHORT, "density value: %d", u_arg);
	    return;
	}
	density = u_arg;
	reset_fonts();
	if (mane.shrinkfactor == 1) {
	    print_statusline(STATUS_SHORT,
			     "set-density ignored with magnification 1");
	    return;
	}
	print_statusline(STATUS_SHORT, "density value: %u", u_arg);
    }
    canit = True;
    XFlush(DISP);
}

#if GREY

static
ACTION(Act_set_greyscaling)
{
    int arg;
    UNUSED(w);
    UNUSED(event);
    GET_ARG4(arg, arg = atoi(*params), sign * number, {
	     TOGGLE(use_grey); use_grey = !use_grey; if (use_grey) {
	     print_statusline(STATUS_SHORT, "greyscaling on");}
	     else {
	     print_statusline(STATUS_SHORT, "greyscaling off");}
	     canit = True; XFlush(DISP); return;}
    );

    switch (arg) {
    case 0:
	use_grey = False;
	print_statusline(STATUS_SHORT, "greyscaling off");
	break;
    case 1:
	use_grey = True;
	print_statusline(STATUS_SHORT, "greyscaling on");
	break;
    default:
	{
	    float newgamma = arg != 0 ? arg / 100.0 : 1.0;
	    use_grey = newgamma;
	    print_statusline(STATUS_SHORT, "greyscale value: %.1f", newgamma);
	}
    }
    if (use_grey)
	init_pix();
    reset_fonts();
    canit = True;
    XFlush(DISP);
}

#endif /* GREY */

#if PS

static
ACTION(Act_set_ps)
{
    UNUSED(w);
    UNUSED(event);
    TOGGLE(resource._postscript)
	resource._postscript = !resource._postscript;

    if (resource._postscript) {
 	scanned_page = scanned_page_bak;
	print_statusline(STATUS_SHORT, "postscript rendering on");
    }
    else {
	print_statusline(STATUS_SHORT, "postscript rendering off");
    }
    
    psp.toggle();

    canit = True;
    XFlush(DISP);
}

#endif /* PS */

#ifdef HTEX
static
ACTION(Act_htex_back)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    htex_goback();
}

static
ACTION(Act_htex_anchorinfo)
{
    int x, y;
    Window ww;
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    (void)XTranslateCoordinates(DISP, event->xkey.window, mane.win,
				event->xkey.x, event->xkey.y, &x, &y, &ww);
    htex_displayanchor(current_page, x, y);
}
#endif /* HTEX */

#if PS_GS
static
ACTION(Act_set_gs_alpha)
{
    UNUSED(w);
    UNUSED(event);

    TOGGLE(resource.gs_alpha)
	resource.gs_alpha = !resource.gs_alpha;

    if (resource.gs_alpha) {
	print_statusline(STATUS_SHORT, "ghostscript alpha active");
    }
    else {
	print_statusline(STATUS_SHORT, "ghostscript alpha inactive");
    }
    
    canit = True;
    XFlush(DISP);
}
#endif /* PS_GS */


#ifdef STATUSLINE
static void
do_set_statusline(void)
{
    if (!resource.statusline) {
	resource.statusline = True;
	XtSetValues(vport_widget, resizable_on, XtNumber(resizable_on));
	XdviResizeWidget(vport_widget, window_w, window_h);
	create_statusline();
    }
    else {
	resource.statusline = False;
	XtDestroyWidget(statusline);
    }
}
#endif /* STATUSLINE */


#if BUTTONS

static
ACTION(Act_set_expert_mode)
{
    int arg;
    UNUSED(w);
    UNUSED(event);
    GET_ARG(arg, -1);

#ifdef STATUSLINE
    /* with args == 1, toggles statusline only */
    if (arg == 1) {
	TOGGLE(resource.statusline);
	do_set_statusline();
	return;
    }
#endif

    TOGGLE(resource.expert);

    if (resource.expert) {	/* create buttons */
	resource.expert = False;
	if (destroy_count != 0) {
	    return;
	}
#ifndef MOTIF
	create_buttons();	/* this determines xtra_wid */

	XtSetValues(vport_widget, resizable_on, XtNumber(resizable_on));
	XdviResizeWidget(vport_widget, window_w -= xtra_wid, window_h);

	set_button_panel_height((XtArgVal) window_h);
#else
	create_buttons();
	window_w -= xtra_wid;
#endif /* not MOTIF */
    }
    else {	/* destroy buttons */
	resource.expert = True;

	if (destroy_count != 0) {
	    return;
	}
	destroy_count = 2;	/* this counts the callback calls */

	XtAddCallback(panel_widget, XtNdestroyCallback,
		      handle_destroy_buttons, (XtPointer) 0);
	XtAddCallback(line_widget, XtNdestroyCallback,
		      handle_destroy_buttons, (XtPointer) 0);
	XtDestroyWidget(panel_widget);
	XtDestroyWidget(line_widget);

	window_w += xtra_wid;
	while (b_head != NULL) {
	    struct button_info *bp = b_head;
	    struct xdvi_action *action;

	    b_head = bp->next;
	    free(bp->label);
	    /* free bp->action */
	    for (action = bp->action; action != NULL;) {
		struct xdvi_action *act2 = action;
		action = act2->next;
		if (act2->num_params > 0)
		    free(act2->param);
		free(act2);
	    }
	    free(bp);
	}
    }
}
#endif /* BUTTONS */

#if !TOOLKIT

static
ACTION(Act_redraw)
{
    canit = True;
    XFlush(DISP);
}

#endif /* not TOOLKIT */

static
ACTION(Act_reread_dvi_file)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    dvi_time = 0;

#if PS
    ps_clear_cache();
#endif
    canit = True;
    XFlush(DISP);
    print_statusline(STATUS_SHORT, "Dvi file reread");
}

static
ACTION(Act_discard_number)
{
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    have_arg = False;
    number = 0;
    sign = 1;
    print_statusline(STATUS_SHORT, "numerical prefix discarded");
}


/* Actions to support the magnifying glass.  */

static void mag_motion ARGS((XEvent *));
static void mag_release ARGS((XEvent *));

static void
compute_mag_pos(int *xp, int *yp)
{
    int t;

    t = mag_x + main_x - alt.width / 2;
    if (t > WidthOfScreen(SCRN) - (int)alt.width - 2 * MAGBORD)
	t = WidthOfScreen(SCRN) - (int)alt.width - 2 * MAGBORD;
    if (t < 0)
	t = 0;
    *xp = t;
    t = mag_y + main_y - alt.height / 2;
    if (t > HeightOfScreen(SCRN) - (int)alt.height - 2 * MAGBORD)
	t = HeightOfScreen(SCRN) - (int)alt.height - 2 * MAGBORD;
    if (t < 0)
	t = 0;
    *yp = t;
}

static Boolean skip_next_mouseevent = False;

#ifdef HTEX
static
ACTION(Act_href)
{
    int x, y;
    Window ww;
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    (void)XTranslateCoordinates(DISP, event->xkey.window, mane.win,
				event->xkey.x, event->xkey.y, &x, &y, &ww);	/* throw away last argument */
    if (htex_handleref(False, current_page, x, y) == 1)
	skip_next_mouseevent = True;
}

static
ACTION(Act_href_newwindow)
{
    int x, y;
    Window ww;
    UNUSED(w);
    UNUSED(event);
    UNUSED(params);
    UNUSED(num_params);

    (void)XTranslateCoordinates(DISP, event->xkey.window, mane.win,
				event->xkey.x, event->xkey.y, &x, &y, &ww);	/* throw away last argument */
    if (htex_handleref(True, current_page, x, y) == 1)
	skip_next_mouseevent = True;
}
#endif /* HTEX */

static
ACTION(Act_magnifier)
{
    int x, y;
    _Xconst char *p;
    XSetWindowAttributes attr;
#if TOOLKIT && !MOTIF
    Window throwaway;
#endif
    UNUSED(w);
    UNUSED(params);
    UNUSED(num_params);

    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    
#if TOOLKIT
    if (event->type != ButtonPress || mouse_release != null_mouse
	|| alt.win != (Window) 0 || mane.shrinkfactor == 1 || *num_params != 1) {
	XBell(DISP, 20);
	if (mane.shrinkfactor == 1) {
	    XBell(DISP, 20);
	    print_statusline(STATUS_SHORT,
			     "No magnification available at shrink factor 1");
	}
	return;
    }

    p = *params;
    if (*p == '*') {
	int n = atoi(p + 1) - 1;

	if (n < 0 || n >= 5 || mg_size[n].w <= 0) {
	    XBell(DISP, 20);
	    return;
	}
	alt.width = mg_size[n].w;
	alt.height = mg_size[n].h;
    }
    else {
	alt.width = alt.height = atoi(p);
	p = strchr(p, 'x');
	if (p != NULL) {
	    alt.height = atoi(p + 1);
	    if (alt.height <= 0)
		alt.width = 0;
	}
	if (alt.width <= 0) {
	    XBell(DISP, 20);
	    return;
	}
    }
#if !MOTIF
    (void)XTranslateCoordinates(DISP, event->xbutton.window, mane.win,
				0, 0, &mag_conv_x, &mag_conv_y, &throwaway);
#endif

#else /* not TOOLKIT */

    struct mg_size_rec *size_ptr = mg_size + event->xbutton.button - 1;

    if (mouse_release != null_mouse || alt.win != (Window) 0
	|| mane.shrinkfactor == 1 || size_ptr->w <= 0) {
	XBell(DISP, 20);
	return;
    }

    alt.width = size_ptr->w;
    alt.height = size_ptr->h;

#endif /* not TOOLKIT */

    mag_x = event->xbutton.x + mag_conv_x;
    mag_y = event->xbutton.y + mag_conv_y;
    main_x = event->xbutton.x_root - mag_x;
    main_y = event->xbutton.y_root - mag_y;
    compute_mag_pos(&x, &y);
    alt.base_x = (mag_x + mane_base_x) * mane.shrinkfactor - alt.width / 2;
    alt.base_y = (mag_y + mane_base_y) * mane.shrinkfactor - alt.height / 2;
    attr.save_under = True;
    attr.border_pixel = resource.rule_pixel;
    attr.background_pixel = back_Pixel;
    attr.override_redirect = True;
#ifdef GREY
    attr.colormap = our_colormap;
#endif
    alt.win = XCreateWindow(DISP, RootWindowOfScreen(SCRN),
			    x, y, alt.width, alt.height, MAGBORD,
			    our_depth, InputOutput, our_visual,
			    CWSaveUnder | CWBorderPixel | CWBackPixel |
#ifdef GREY
			    CWColormap |
#endif
			    CWOverrideRedirect, &attr);
    XSelectInput(DISP, alt.win, ExposureMask);
    XMapWindow(DISP, alt.win);

    
    /*
      This call will draw the point rulers when the magnifier first
      pops up, if the XDvi*delayRulers resource is false.  Some users
      may prefer rulers to remain invisible until the magnifier is
      moved, so the default is true.  Rulers can be suppressed entirely
      by setting the XDvi*tickLength resource to zero or negative.
      
      It is better to use highGC for draw_rulers() than foreGC,
      because this allows the ruler to be drawn in a different color,
      helping to distinguish ruler ticks from normal typeset text.
      
      This call will draw the point rulers when the magnifier first
      pops up, if the XDvi*delayRulers resource is false.  Some users
      may prefer rulers to remain invisible until the magnifier is
      moved, so the default is true.  Rulers can be suppressed entirely
      by setting the XDvi*tickLength resource to zero or negative.
      
      It is better to use highGC for draw_rulers() than foreGC,
      because this allows the ruler to be drawn in a different color,
      helping to distinguish ruler ticks from normal typeset text.
    */
    
    if (!delay_rulers)
	draw_rulers(alt.width, alt.height, rulerGC);

    alt_stat = 1;	/* waiting for exposure */
    mouse_motion = mag_motion;
    mouse_release = mag_release;
}

static void
mag_motion(XEvent *event)
{
    new_mag_x = event->xmotion.x + mag_conv_x;
    main_x = event->xmotion.x_root - new_mag_x;
    new_mag_y = event->xmotion.y + mag_conv_y;
    main_y = event->xmotion.y_root - new_mag_y;
    mag_moved = (new_mag_x != mag_x || new_mag_y != mag_y);
}

static void
mag_release(XEvent *event)
{
    UNUSED(event);
    
    if (alt.win != (Window) 0) {
	if (alt_stat)
	    alt_stat = -1;	/* destroy upon expose */
	else {
	    XDestroyWindow(DISP, alt.win);
	    if (currwin.win == alt.win)
		alt_canit = True;
	    alt.win = (Window) 0;
	    mouse_motion = mouse_release = null_mouse;
	    mag_moved = False;
	    can_exposures(&alt);
	}
    }
}

static void
movemag(int x, int y)
{
    int xx, yy;

    mag_x = x;
    mag_y = y;
    if (mag_x == new_mag_x && mag_y == new_mag_y)
	mag_moved = False;
    compute_mag_pos(&xx, &yy);
    XMoveWindow(DISP, alt.win, xx, yy);
    scrollwindow(&alt,
		 (x + mane_base_x) * mane.shrinkfactor - (int)alt.width / 2,
		 (y + mane_base_y) * mane.shrinkfactor - (int)alt.height / 2);
    draw_rulers(alt.width, alt.height, rulerGC);
}


/* Actions to support dragging the image.  */

static int drag_last_x, drag_last_y;	/* last position of cursor */
int drag_flags;	/* 1 = vert, 2 = horiz; hypertex.c needs this to be non-static */

static void drag_motion ARGS((XEvent *));
static void drag_release ARGS((XEvent *));

static
ACTION(Act_drag)
{
    UNUSED(w);

    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    if (mouse_release != null_mouse && mouse_release != drag_release)
	return;
    
#if TOOLKIT

    if (*num_params != 1)
	return;
    switch (**params) {
    case '|':
	drag_flags = 1;
	break;
    case '-':
	drag_flags = 2;
	break;
    case '+':
	drag_flags = 3;
	break;
    default:
	return;
    }

#else /* not TOOLKIT */

    drag_flags = event->xbutton.button;
    if (drag_flags <= 0 || drag_flags > 3)
	return;
    drag_flags = drag_flags ^ (drag_flags >> 1);

#endif /* not TOOLKIT */

    if (mouse_release == null_mouse) {
	mouse_motion = drag_motion;
	mouse_release = drag_release;
	drag_last_x = event->xbutton.x_root;
	drag_last_y = event->xbutton.y_root;
    }
    else
	drag_motion(event);

    XDefineCursor(DISP, mane.win, drag_cursor[drag_flags - 1]);
    XFlush(DISP);
    dragcurs = True;
}


static void
drag_motion(XEvent *event)
{
#if MOTIF
    get_xy();
#endif

    if (drag_flags & 2) {	/* if horizontal motion */
#if TOOLKIT
#if !MOTIF
	if (x_bar != NULL)
	    XtCallCallbacks(x_bar, XtNscrollProc,
			    (XtPointer) (drag_last_x - event->xbutton.x_root));
#else /* MOTIF */
	(void)set_bar_value(x_bar,
			    drag_last_x - event->xbutton.x_root - window_x,
			    (int)(page_w - clip_w));
#endif /* MOTIF */
#else /* not TOOLKIT */
	scrollmane(mane.base_x + drag_last_x - event->xbutton.x_root,
		   mane.base_y);
#endif /* not TOOLKIT */
	drag_last_x = event->xbutton.x_root;
    }

    if (drag_flags & 1) {	/* if vertical motion */
#if TOOLKIT
#if !MOTIF
	if (y_bar != NULL)
	    XtCallCallbacks(y_bar, XtNscrollProc,
			    (XtPointer) (drag_last_y - event->xbutton.y_root));
#else /* MOTIF */
	(void)set_bar_value(y_bar,
			    drag_last_y - event->xbutton.y_root - window_y,
			    (int)(page_h - clip_h));
#endif /* MOTIF */
#else /* not TOOLKIT */
	scrollmane(mane.base_x,
		   mane.base_y + drag_last_y - event->xbutton.y_root);
#endif /* not TOOLKIT */
	drag_last_y = event->xbutton.y_root;
    }
}


static void
drag_release(XEvent *event)
{
    drag_motion(event);
    mouse_motion = mouse_release = null_mouse;

    XDefineCursor(DISP, mane.win, ready_cursor);

    XFlush(DISP);
    dragcurs = False;
}



/* Wheel support.  */
/* SU 2000/03/08: TODO: I couldn't test anything of this,
 * since I don't have a wheel mouse ...
 */

static int wheel_button = -1;

static
ACTION(Act_wheel)
{
#if TOOLKIT
    int dist;
    UNUSED(w);

    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    if (*num_params == 0) {
	XBell(DISP, 20);
	return;
    }
    dist = (strchr(*params, '.') == NULL) ? atoi(*params)
	: (int)(atof(*params) * resource.wheel_unit);
#if !MOTIF
    if (y_bar != NULL)
	XtCallCallbacks(y_bar, XtNscrollProc, (XtPointer) dist);
#else /* MOTIF */
    get_xy();
    (void)set_bar_value(y_bar, dist - window_y, (int)(page_h - clip_h));
#endif /* MOTIF */
#else /* not TOOLKIT */
    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    scrollmane(mane.base_x,
	       mane.base_y + (event->xbutton.button == 5
			      ? resource.wheel_unit : -resource.wheel_unit));
#endif /* not TOOLKIT */

    wheel_button = event->xbutton.button;
}


/* Internal mouse actions.  */

#if TOOLKIT

static
ACTION(Act_motion)
{
#ifdef HTEX
    int x, y;
    UNUSED(w);
    UNUSED(params);
    UNUSED(num_params);

    if (pointerlocate(&x, &y)) {
	htex_displayanchor(current_page, x, y);
    }
#endif
    if ((int)event->xbutton.button != wheel_button) {
	mouse_motion(event);
    }
}


static
ACTION(Act_release)
{
    UNUSED(w);
    UNUSED(params);
    UNUSED(num_params);

    if ((int)event->xbutton.button == wheel_button) {
	wheel_button = -1;
	return;
    }

    mouse_release(event);
}

static
ACTION(Act_switch_magnifier_units)
{
    size_t k = 0;
    static char *TeX_units[] = {
	"bp", "cc", "cm", "dd", "in", "mm", "pc", "pt", "sp",
    };
    UNUSED(w);
    UNUSED(params);
    UNUSED(event);
    UNUSED(num_params);

    for (k = 0; k < sizeof(TeX_units) / sizeof(TeX_units[0]); ++k)
	if (strcmp(tick_units, TeX_units[k]) == 0)
	    break;
    k++;
    if (k >= sizeof(TeX_units) / sizeof(TeX_units[0]))
	k = 0;
    tick_units = TeX_units[k];
    print_statusline(STATUS_SHORT, "Ruler units: %s\n", tick_units);
}

#ifdef GRID
static
ACTION(Act_toggle_grid_mode)
{
    int arg;
    UNUSED(w);
    UNUSED(params);
    UNUSED(event);
    UNUSED(num_params);

    GET_ARG(arg, -1);
    switch (arg) {
    case -1:
	grid_mode = !grid_mode;
	if (grid_mode) {
	    print_statusline(STATUS_SHORT, "grid mode on");
	}
	else {
	    print_statusline(STATUS_SHORT, "grid mode off");
	}
	break;
    case 1:
    case 2:
    case 3:
	grid_mode = arg;
	print_statusline(STATUS_SHORT, "grid mode %d", arg);
	break;
    default:
	XBell(DISP, 20);
	print_statusline(STATUS_SHORT,
			 "Valid arguments for grid mode are: none (toggles), 1, 2, 3");
	return;
    }
    init_page();
    reconfig();
    canit = True;
    XFlush(DISP);
}
#endif /* GRID */

#endif /* TOOLKIT */


/* Actions for source specials.  */

ACTION(Act_source_special)
{
    int x, y;
    Window ww;
    UNUSED(w);
    UNUSED(params);
    UNUSED(num_params);

    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    
    if ((event->type == ButtonPress && mouse_release != null_mouse)
	|| alt.win != (Window) 0) {
	XBell(DISP, 20);
	return;
    }

    x = event->xbutton.x;
    y = event->xbutton.y;
    if (event->xbutton.window != mane.win)
	(void)XTranslateCoordinates(DISP,
				    RootWindowOfScreen(SCRN), mane.win,
				    event->xbutton.x_root,
				    event->xbutton.y_root, &x, &y, &ww);	/* throw away last argument */

    x = (x + mane_base_x) * mane.shrinkfactor;
    y = (y + mane_base_y) * mane.shrinkfactor;

    source_reverse_search(x, y, True);
}

ACTION(Act_show_source_specials)
{
    int arg;
    GET_ARG(arg, -1);
    UNUSED(w);

    if ((event->type == ButtonPress && mouse_release != null_mouse)
	|| alt.win != (Window) 0) {
	XBell(DISP, 20);
	return;
    }

    source_special_show(arg == 1 ? True : False);
}

ACTION(Act_source_what_special)
{
    int my_x, my_y;
    Window ww;
    UNUSED(w);
    UNUSED(params);
    UNUSED(num_params);

    if (skip_next_mouseevent) {
	skip_next_mouseevent = False;
	return;
    }
    
    (void)XTranslateCoordinates(DISP, event->xkey.window, mane.win,
				event->xkey.x, event->xkey.y, &my_x, &my_y, &ww);	/* throw away last argument */
    my_x = (my_x + mane_base_x) * mane.shrinkfactor;
    my_y = (my_y + mane_base_y) * mane.shrinkfactor;

    source_reverse_search(my_x, my_y, False);
}

#ifdef SELFILE
static
ACTION(Act_select_dvi_file)
{
    UNUSED(w);
    UNUSED(params);
    UNUSED(event);
    UNUSED(num_params);

    /*
     * signal we want a new file in dvi_file_changed; for this, dvi_time
     * needs to be increased
     */
    ++dvi_time;
    canit = True;
    XFlush(DISP);
}
#endif

#undef	ACTION
#undef	GET_ARG4
#undef	GET_ARG
#undef	TOGGLE

/* APS Pointer locator: */

int
pointerlocate(int *xpos, int *ypos)	/* Return screen positions */
{
    Window root, child;
    int root_x, root_y;
    unsigned int keys_buttons;

    return XQueryPointer(DISP, mane.win, &root, &child,
			 &root_x, &root_y, xpos, ypos, &keys_buttons);
}


#if TOOLKIT

/*ARGSUSED*/ void
handle_expose(Widget widget, XtPointer closure, XEvent *ev, Boolean *cont)
{
    struct WindowRec *windowrec = (struct WindowRec *)closure;
    UNUSED(widget);
    UNUSED(cont);

    if (windowrec == &alt) {
	if (alt_stat < 0) {	/* destroy upon exposure */
	    alt_stat = 0;
	    mag_release(ev);
	    return;
	}
	else
	    alt_stat = 0;
    }

#define	event	(&(ev->xexpose))
    expose(windowrec, event->x, event->y,
	   (unsigned int)event->width, (unsigned int)event->height);
}
#undef	event
#endif /* TOOLKIT */

void
handle_property_change(
#ifdef TOOLKIT
		       Widget widget, XtPointer junk, XEvent *ev, Boolean *cont
#else /* !TOOLKIT */
		       XEvent *ev
#endif
		       )
#define	event	(&(ev->xproperty))
{
    char *src_goto_property;
    size_t src_goto_len;

    UNUSED(widget);
    UNUSED(junk);
    UNUSED(cont);

    if (event->window != XtWindow(top_level)
	|| event->atom != ATOM_SRC_GOTO)	/* if spurious event */
	return;

    /*
       * Retrieve the data from our window property.
     */

    src_goto_len = property_get_data(XtWindow(top_level), ATOM_SRC_GOTO,
				     (unsigned char **)&src_goto_property, XGetWindowProperty);

    if (src_goto_len == 0) {
	if (debug & DBG_CLIENT)
	    puts("SRC_GOTO gave us nothing");
	return;
    }

    source_forward_search(src_goto_property);
}

#undef	event


/*
 *	Since redrawing the screen is (potentially) a slow task, xdvi checks
 *	for incoming events while this is occurring.  It does not register
 *	a work proc that draws and returns every so often, as the toolkit
 *	documentation suggests.  Instead, it checks for events periodically
 *	(or not, if SIGPOLL can be used instead) and processes them in
 *	a subroutine called by the page drawing routine.  This routine (below)
 *	checks to see if anything has happened and processes those events and
 *	signals.  (Or, if it is called when there is no redrawing that needs
 *	to be done, it blocks until something happens.)
 */

extern void print_child_error ARGS((void));

void
#if	PS
ps_read_events(wide_bool wait, wide_bool allow_can)
#else
read_events(wide_bool wait)
#define	allow_can	True
#endif
{
    XEvent event;

    alt_canit = False;
    for (;;) {
	event_counter = event_freq;
	/*
	 * The above line clears the flag indicating that an event is
	 * pending.  So if an event comes in right now, the flag will be
	 * set again needlessly, but we just end up making an extra call.
	 * Also, be careful about destroying the magnifying glass while
	 * drawing on it.
	 */

#ifndef FLAKY_SIGPOLL

	if (event_freq < 0) {	/* if SIGPOLL works */

	    if (!XtPending()) {
		sigset_t oldsig;

		(void)sigprocmask(SIG_BLOCK, &all_signals, &oldsig);
		for (;;) {

		    if (terminate_flag)
			xdvi_exit(0);

		    if (child_flag) {	/* zombie prevention */
			child_flag = False;
#if ! HAVE_SIGACTION
			/* reset the signal */
			(void)signal(SIGCHLD, handle_sigchld);
#endif
			for (;;) {
			    pid_t pid;
			    int status;
#if HAVE_WAITPID
			    pid = waitpid(-1, &status, WNOHANG);
#else
			    pid = wait3(NULL, WNOHANG, NULL);
#endif
			    if (WIFEXITED(status)) {
				/* if child exited with error, print error text */
				if (WEXITSTATUS(status) != 0) {
				    print_child_error();
				    break;
				}
			    }

			    if (pid == 0)
				break;
			    if (pid != -1 || errno == EINTR)
				continue;
			    if (errno == ECHILD)
				break;
#if HAVE_WAITPID
			    perror("xdvi: waitpid");
#else
			    perror("xdvi: wait3");
#endif
			    break;
			}
		    }

		    if (XtPending())
			break;

		    /*
		     * The following code eliminates unnecessary calls to
		     * XDefineCursor, since this is a slow operation on some
		     * hardware (e.g., S3 chips).
		     */
		    if (busycurs && wait && !canit && !mag_moved
			&& alt.min_x == MAXDIM && mane.min_x == MAXDIM) {
			if (!dragcurs) {
			    XSync(DISP, False);
			    if (XtPending())
				break;

			    XDefineCursor(DISP, mane.win, ready_cursor);

			    XFlush(DISP);
			}
			busycurs = False;
		    }
		    if (!wait && (canit | alt_canit)) {
#if	PS
			psp.interrupt();
#endif
			if (allow_can) {
			    (void)sigprocmask(SIG_SETMASK, &oldsig,
					      (sigset_t *) NULL);
			    longjmp(canit_env, 1);
			}
		    }
		    if (!wait || canit || mane.min_x < MAXDIM
			|| alt.min_x < MAXDIM || mag_moved) {
			(void)sigprocmask(SIG_SETMASK, &oldsig,
					  (sigset_t *) NULL);
			return;
		    }
		    (void)sigsuspend(&oldsig);
		}
		(void)sigprocmask(SIG_SETMASK, &oldsig, (sigset_t *) NULL);
	    }
	}
	else
#endif /* not FLAKY_SIGPOLL */

	{
	    for (;;) {
#if HAVE_POLL
		int retval;
#endif

		if (terminate_flag)
		    xdvi_exit(0);

		if (child_flag) {	/* zombie prevention */
		    child_flag = False;
#if ! HAVE_SIGACTION
		    /* reset the signal */
		    (void)signal(SIGCHLD, handle_sigchld);
#endif
		    for (;;) {
			pid_t pid;
#if HAVE_WAITPID
			pid = waitpid(-1, NULL, WNOHANG);
#else
			pid = wait3(NULL, WNOHANG, NULL);
#endif
			if (pid == 0)
			    break;
			if (pid != -1 || errno == EINTR)
			    continue;
			if (errno == ECHILD)
			    break;
#if HAVE_WAITPID
			perror("xdvi: waitpid");
#else
			perror("xdvi: wait3");
#endif
			break;
		    }
		}

		if (XtPending())
		    break;

		/*
		 * The following code eliminates unnecessary calls to
		 * XDefineCursor, since this is a slow operation on some
		 * hardware (e.g., S3 chips).
		 */
		if (busycurs && wait && !canit && !mag_moved
		    && alt.min_x == MAXDIM && mane.min_x == MAXDIM) {
		    if (!dragcurs) {
			XSync(DISP, False);
			if (XtPending())
			    break;

			XDefineCursor(DISP, mane.win, ready_cursor);

			XFlush(DISP);
		    }
		    busycurs = False;
		}
		if (!wait && (canit | alt_canit)) {
#if	PS
		    psp.interrupt();
#endif
		    if (allow_can)
			longjmp(canit_env, 1);
		}
		if (!wait || canit
		    || mane.min_x < MAXDIM || alt.min_x < MAXDIM || mag_moved)
		    return;
		/* If a SIGUSR1 signal comes right now, then it will wait
		   until an X event or another SIGUSR1 signal arrives. */

#if HAVE_POLL
		do {
		    retval = poll(fds, XtNumber(fds), -1);
		} while (retval < 0 && errno == EAGAIN);

		if (retval < 0 && errno != EINTR)
		    perror("poll (xdvi read_events)");
#else
		FD_SET(ConnectionNumber(DISP), &readfds);
		if (select(ConnectionNumber(DISP) + 1, &readfds,
			   (fd_set *) NULL, (fd_set *) NULL,
			   (struct timeval *)NULL) < 0 && errno != EINTR)
		    perror("select (xdvi read_events)");
#endif
	    }
	}

#ifdef	TOOLKIT

	XtNextEvent(&event);
	if (resized)
	    get_geom();
	if (event.xany.window == alt.win && event.type == Expose) {
	    handle_expose((Widget) NULL, (XtPointer) & alt, &event,
			  (Boolean *) NULL);
	    continue;
	}
	(void)XtDispatchEvent(&event);

#else /* not TOOLKIT */

	XNextEvent(DISP, &event);
	if (event.xany.window == mane.win || event.xany.window == alt.win) {
	    struct WindowRec *wr = &mane;

	    if (event.xany.window == alt.win) {
		wr = &alt;
		/* check in case we already destroyed the window */
		if (alt_stat < 0) {	/* destroy upon exposure */
		    alt_stat = 0;
		    mag_release(&event);
		    continue;
		}
		else
		    alt_stat = 0;
	    }
	    switch (event.type) {
	    case GraphicsExpose:
	    case Expose:
		expose(wr, event.xexpose.x, event.xexpose.y,
		       event.xexpose.width, event.xexpose.height);
		break;

	    case ButtonPress:
		if (resource.wheel_unit != 0 && (event.xbutton.button == 4
						 || event.xbutton.button == 5))
		    Act_wheel(&event);
		else if (event.xbutton.state & ControlMask) {
		    switch (event.xbutton.button) {
		    case 1:
			Act_source_special(&event);
			break;
		    }
		}
		else if (event.xbutton.state & ShiftMask)
		    Act_drag(&event);
		else
		    Act_magnifier(&event);
		break;

	    case MotionNotify:
		mouse_motion(&event);
		break;

	    case ButtonRelease:
		if ((int)event.xbutton.button == wheel_button)
		    wheel_button = -1;
		else
		    mouse_release(&event);
		break;
	    }	/* end switch */
	}	/* end if window == {mane,alt}.win */

	else if (event.xany.window == x_bar) {
	    if (event.type == Expose)
		XFillRectangle(DISP, x_bar, ruleGC,
			       x_bgn, 1, x_end - x_bgn, BAR_WID);
	    else if (event.type == MotionNotify)
		scrollmane(event.xmotion.x * page_w / clip_w, mane.base_y);
	    else
		switch (event.xbutton.button) {
		case 1:
		    scrollmane(mane.base_x + event.xbutton.x, mane.base_y);
		    break;
		case 2:
		    scrollmane(event.xbutton.x * page_w / clip_w, mane.base_y);
		    break;
		case 3:
		    scrollmane(mane.base_x - event.xbutton.x, mane.base_y);
		}
	}

	else if (event.xany.window == y_bar) {
	    if (event.type == Expose)
		XFillRectangle(DISP, y_bar, ruleGC,
			       1, y_bgn, BAR_WID, y_end - y_bgn);
	    else if (event.type == MotionNotify)
		scrollmane(mane.base_x, event.xmotion.y * page_h / clip_h);
	    else
		switch (event.xbutton.button) {
		case 1:
		    scrollmane(mane.base_x, mane.base_y + event.xbutton.y);
		    break;
		case 2:
		    scrollmane(mane.base_x, event.xbutton.y * page_h / clip_h);
		    break;
		case 3:
		    scrollmane(mane.base_x, mane.base_y - event.xbutton.y);
		}
	}

	else if (event.xany.window == top_level)
	    switch (event.type) {
	    case ConfigureNotify:
		if (event.xany.window == top_level &&
		    (event.xconfigure.width != window_w ||
		     event.xconfigure.height != window_h)) {
		    Window old_mane_win = mane.win;

		    window_w = event.xconfigure.width;
		    window_h = event.xconfigure.height;
		    reconfig();
		    if (old_mane_win == (Window) 0)
			home(False);
		}
		break;

	    case MapNotify:	/* if running w/o WM */
		if (mane.win == (Window) 0) {
		    reconfig();
		    home(False);
		}
		break;

	    case KeyPress:
		{
#define			TRSIZE	4

		    char trbuf[TRSIZE];

		    if (XLookupString(&event.xkey, trbuf, TRSIZE,
				      (KeySym *) NULL,
				      (XComposeStatus *) NULL) == 1
			&& (keychar = *trbuf) < 128)
			(actions[keychar]) (&event);
		}
		break;

	    case PropertyNotify:
		handle_property_change(&event);
		break;
	    }

#endif /* not TOOLKIT */

    }
}

static void
redraw(struct WindowRec *windowrec)
{
    static Boolean do_clear = False;

    currwin = *windowrec;
    min_x = currwin.min_x + currwin.base_x;
    min_y = currwin.min_y + currwin.base_y;
    max_x = currwin.max_x + currwin.base_x;
    max_y = currwin.max_y + currwin.base_y;
    can_exposures(windowrec);

    if (debug & DBG_EVENT)
	Printf("Redraw %d x %d at (%d, %d) (base=%d,%d)\n", max_x - min_x,
	       max_y - min_y, min_x, min_y, currwin.base_x, currwin.base_y);
    if (!busycurs) {
	if (!dragcurs) {
	    XDefineCursor(DISP, mane.win, redraw_cursor);
	    XFlush(DISP);
	}
	busycurs = True;
    }
    if (setjmp(dvi_env)) {
	/* SU 2000/03/14: Clearing the window (which is generally quite annoying
	 * for users, since readable text is lost and the window essentially becomes useless)
	 * wouldn't be needed if xdvi would redraw itself after
	 * the file had been reopened succesfully.
	 */
	XClearWindow(DISP, mane.win);
	/* print to the window if statusline is not enabled, since these
	 * are important messages like `dvi file corrupted' etc.
	 */
	if (resource.statusline) {
	    print_statusline(STATUS_SHORT, (char *)dvi_oops_msg);
	}
	else {
	    showmessage(dvi_oops_msg);
	}
	/* this will delete dvi_oops_msg from the statusline
	 * when the file is reloaded
	 */
	do_clear = True;
	if (dvi_file) {
	    Fclose(dvi_file);
	    dvi_file = NULL;
	}
    }
    else {
	draw_page();
	if (do_clear) {
	    print_statusline(STATUS_FOREVER, "");
	}
	do_clear = False;
	warn_spec_now = False;
    }
}

void
redraw_page(void)
{
    if (debug & DBG_EVENT)
	Fputs("Redraw page:  ", stdout);
    XClearWindow(DISP, mane.win);
    if (backing_store != NotUseful) {
	mane.min_x = mane.min_y = 0;
	mane.max_x = page_w;
	mane.max_y = page_h;
    }
    else {
	get_xy();
	mane.min_x = -window_x;
	mane.max_x = -window_x + clip_w;
	mane.min_y = -window_y;
	mane.max_y = -window_y + clip_h;
    }
    redraw(&mane);
}

/*
 *	Interrupt system for receiving events.  The program sets a flag
 *	whenever an event comes in, so that at the proper time (i.e., when
 *	reading a new dvi item), we can check incoming events to see if we
 *	still want to go on printing this page.  This way, one can stop
 *	displaying a page if it is about to be erased anyway.  We try to read
 *	as many events as possible before doing anything and base the next
 *	action on all events read.
 *	Note that the Xlib and Xt routines are not reentrant, so the most we
 *	can do is set a flag in the interrupt routine and check it later.
 *	Also, sometimes the interrupts are not generated (some systems only
 *	guarantee that SIGIO is generated for terminal files, and on the system
 *	I use, the interrupts are not generated if I use "(xdvi foo &)" instead
 *	of "xdvi foo").  Therefore, there is also a mechanism to check the
 *	event queue every 70 drawing operations or so.  This mechanism is
 *	disabled if it turns out that the interrupts do work.
 *	For a fuller discussion of some of the above, see xlife in
 *	comp.sources.x.
 */

static void
can_exposures(struct WindowRec *windowrec)
{
    windowrec->min_x = windowrec->min_y = MAXDIM;
    windowrec->max_x = windowrec->max_y = 0;
}

#ifndef FLAKY_SIGPOLL
/* ARGSUSED */
static RETSIGTYPE
handle_sigpoll(int signo)
{
    UNUSED(signo);
    event_counter = 1;
    event_freq = -1;	/* forget Plan B */
#if !HAVE_SIGACTION
    (void)signal(SIGPOLL, handle_sigpoll);	/* reset the signal */
#endif
}
#endif /* not FLAKY_SIGPOLL */

/* ARGSUSED */
static RETSIGTYPE
handle_sigusr(int signo)
{
    UNUSED(signo);
    event_counter = 1;
    canit = True;
    dvi_time = 0;
#if ! HAVE_SIGACTION
    (void)signal(SIGUSR1, handle_sigusr);	/* reset the signal */
#endif
}

/* ARGSUSED */
static RETSIGTYPE
handle_sigchld(int signo)
{
    UNUSED(signo);
    child_flag = True;
}

/* ARGSUSED */
static RETSIGTYPE
handle_sigterm(int signo)
{
    UNUSED(signo);
    terminate_flag = True;
}

/* ARGSUSED */
static RETSIGTYPE
handle_sigsegv(int signo)
{
    UNUSED(signo);
    fprintf(stderr, "xdvik: caught segmentation fault - trying to clean up and aborting ...\n");
    xdvi_abort();
}

void
enable_intr(void)
{
#ifndef FLAKY_SIGPOLL
    int sock_fd = ConnectionNumber(DISP);
#endif
#if HAVE_SIGACTION
    struct sigaction a;
#endif

#ifndef FLAKY_SIGPOLL
#if HAVE_SIGACTION
    /* Subprocess handling, e.g., MakeTeXPK, fails on the Alpha without
       this, because SIGPOLL interrupts the call of system(3), since OSF/1
       doesn't retry interrupted wait calls by default.  From code by
       maj@cl.cam.ac.uk.  */
    a.sa_handler = handle_sigpoll;
    (void)sigemptyset(&a.sa_mask);
    (void)sigaddset(&a.sa_mask, SIGPOLL);
    a.sa_flags = SA_RESTART;
    sigaction(SIGPOLL, &a, NULL);
#else /* not HAVE_SIGACTION */
    (void)signal(SIGPOLL, handle_sigpoll);
#endif /* not HAVE_SIGACTION */

#if HAVE_STREAMS
    if (isastream(sock_fd) > 0) {
	if (ioctl(sock_fd, I_SETSIG,
		  S_RDNORM | S_RDBAND | S_HANGUP | S_WRNORM) == -1)
	    perror("ioctl I_SETSIG (xdvi)");
    }
    else
#endif
    {
#ifdef FASYNC
	if (fcntl(sock_fd, F_SETOWN, getpid()) == -1)
	    perror("fcntl F_SETOWN (xdvi)");
	if (fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL, 0) | FASYNC)
	    == -1)
	    perror("fcntl F_SETFL (xdvi)");
#elif defined(SIOCSPGRP) && defined(FIOASYNC)
	/* For HP-UX B.10.10 and maybe others.  See "man 7 socket".  */
	int arg;

	arg = getpid();
	if (ioctl(sock_fd, SIOCSPGRP, &arg) == -1)
	    perror("ioctl SIOCSPGRP (xdvi)");
	arg = 1;
	if (ioctl(sock_fd, FIOASYNC, &arg) == -1)
	    perror("ioctl FIOASYNC (xdvi)");
#endif
    }
#endif /* not FLAKY_SIGPOLL */

#if HAVE_SIGACTION
    a.sa_handler = handle_sigusr;
    (void)sigemptyset(&a.sa_mask);
    (void)sigaddset(&a.sa_mask, SIGUSR1);
    a.sa_flags = 0;
    sigaction(SIGUSR1, &a, NULL);
#else /* not HAVE_SIGACTION */
    (void)signal(SIGUSR1, handle_sigusr);
#endif /* not HAVE_SIGACTION */

#if HAVE_SIGACTION
    a.sa_handler = handle_sigchld;
    (void)sigemptyset(&a.sa_mask);
    (void)sigaddset(&a.sa_mask, SIGCHLD);
    a.sa_flags = 0;
    sigaction(SIGCHLD, &a, NULL);
#else /* not HAVE_SIGACTION */
    (void)signal(SIGCHLD, handle_sigchld);
#endif /* not HAVE_SIGACTION */

#if HAVE_SIGACTION
    a.sa_handler = handle_sigterm;
    (void)sigemptyset(&a.sa_mask);
    (void)sigaddset(&a.sa_mask, SIGINT);
    (void)sigaddset(&a.sa_mask, SIGQUIT);
    (void)sigaddset(&a.sa_mask, SIGTERM);
    a.sa_flags = 0;
    sigaction(SIGINT, &a, NULL);
    sigaction(SIGQUIT, &a, NULL);
    sigaction(SIGTERM, &a, NULL);
    a.sa_handler = handle_sigsegv;
    (void)sigemptyset(&a.sa_mask);
    (void)sigaddset(&a.sa_mask, SIGSEGV);
    a.sa_flags = 0;
    sigaction(SIGSEGV, &a, NULL);
#else /* not HAVE_SIGACTION */
    (void)signal(SIGINT, handle_sigterm);
    (void)signal(SIGQUIT, handle_sigterm);
    (void)signal(SIGTERM, handle_sigterm);
    (void)signal(SIGSEGV, handle_sigsegv);
#endif /* not HAVE_SIGACTION */

#ifndef FLAKY_SIGPOLL

    (void)sigemptyset(&all_signals);
    (void)sigaddset(&all_signals, SIGPOLL);
    (void)sigaddset(&all_signals, SIGUSR1);
    (void)sigaddset(&all_signals, SIGCHLD);
    (void)sigaddset(&all_signals, SIGINT);
    (void)sigaddset(&all_signals, SIGQUIT);
    (void)sigaddset(&all_signals, SIGTERM);
    (void)sigaddset(&all_signals, SIGSEGV);

#endif

#if HAVE_POLL
    fds[0].fd = ConnectionNumber(DISP);
#else
    FD_ZERO(&readfds);
#endif

}

void
do_pages(void)
{
    if (debug & DBG_BATCH) {
#ifdef	TOOLKIT
	while (mane.min_x == MAXDIM)
	    read_events(True);
#else /* !TOOLKIT */
	while (mane.min_x == MAXDIM)
	    if (setjmp(canit_env))
		break;
	    else
		read_events(True);
#endif /* TOOLKIT */
	for (current_page = 0; current_page < total_pages; ++current_page) {
#ifdef	__convex__
	    /* convex C turns off optimization for the entire function
	       if setjmp return value is discarded. */
	    if (setjmp(canit_env))	/*optimize me */
		;
#else
	    (void)setjmp(canit_env);
#endif
	    canit = False;
	    redraw_page();
	}
    }
    else {	/* normal operation */
#ifdef	__convex__
	/* convex C turns off optimization for the entire function
	   if setjmp return value is discarded. */
	if (setjmp(canit_env))	/*optimize me */
	    ;
#else
	(void)setjmp(canit_env);
#endif
	dvi_pointer_frame = NULL;	/* clear defunct value (see geom_*()) */
	for (;;) {
	    read_events(True);
	    if (canit) {
		canit = False;
		can_exposures(&mane);
		can_exposures(&alt);
		if (dvi_file != NULL)
		    redraw_page();
		else
		    (void)dvi_file_changed();
	    }
	    else if (mag_moved) {
		if (alt.win == (Window) 0)
		    mag_moved = False;
		else if (abs(new_mag_x - mag_x) > 2 * abs(new_mag_y - mag_y))
		    movemag(new_mag_x, mag_y);
		else if (abs(new_mag_y - mag_y) > 2 * abs(new_mag_x - mag_x))
		    movemag(mag_x, new_mag_y);
		else
		    movemag(new_mag_x, new_mag_y);
	    }
	    else if (alt.min_x < MAXDIM) {
		redraw(&alt);
	    }
	    else if (mane.min_x < MAXDIM) {
		redraw(&mane);
	    }
	    XFlush(DISP);
	}
    }
}
