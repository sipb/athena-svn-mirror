/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * NOTE:
 *	xdvi is based on prior work, as noted in the modification history
 *	in xdvi.c.
 */

#include <ctype.h>
#include "xdvi.h"

/* Xlib and Xutil are already included */
#ifdef	TOOLKIT
#ifdef	OLD_X11_TOOLKIT
#include <X11/Atoms.h>
#else /* not OLD_X11_TOOLKIT */
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#endif /* not OLD_X11_TOOLKIT */
#include <X11/Shell.h>	/* needed for def. of XtNiconX */
#ifndef	XtSpecificationRelease
#define	XtSpecificationRelease	0
#endif
#if	XtSpecificationRelease >= 4
#include <X11/Xaw/Viewport.h>
#ifdef	BUTTONS
#include <X11/Xaw/Command.h>
#endif
#else	/* XtSpecificationRelease < 4 */
#define	XtPointer caddr_t
#include <X11/Viewport.h>
#ifdef	BUTTONS
#include <X11/Command.h>
#endif
#endif	/* XtSpecificationRelease */
#else	/* !TOOLKIT */
typedef	int		Position;
#define	XtPending()	XPending(DISP)
#endif	/* TOOLKIT */

#if	HAS_SIGIO
#include <signal.h>
#ifndef	FASYNC
#undef	HAS_SIGIO
#define	HAS_SIGIO 0
#endif
#endif

#ifndef	X11HEIGHT
#define	X11HEIGHT	8	/* Height of server default font */
#endif

#define	MAGBORD	1	/* border size for magnifier */

/*
 * Command line flags.
 */

#define	fore_Pixel	resource._fore_Pixel
#define	back_Pixel	resource._back_Pixel
#ifdef	TOOLKIT
extern	struct _resource	resource;
#define	brdr_Pixel	resource._brdr_Pixel
#endif	/* TOOLKIT */

#define	clip_w	mane.width
#define	clip_h	mane.height
static	Position main_x, main_y;
static	Position mag_x, mag_y, new_mag_x, new_mag_y;
static	Boolean	mag_moved = False;

#ifdef	TOOLKIT
#ifdef	BUTTONS
static	Widget	line_widget, panel_widget;
#endif
static	Widget	x_bar, y_bar;	/* horizontal and vertical scroll bars */

static	Arg	resize_args[] = {
	{XtNwidth,	(XtArgVal) 0},
	{XtNheight,	(XtArgVal) 0},
};

#define	XdviResizeWidget(widget, w, h)	\
		(resize_args[0].value = (XtArgVal) (w), \
		resize_args[1].value = (XtArgVal) (h), \
		XtSetValues(widget, resize_args, XtNumber(resize_args)) )

#ifdef	BUTTONS

static	Arg	resizable_on[] = {
	{XtNresizable,	(XtArgVal) True},
};

static	Arg	resizable_off[] = {
	{XtNresizable,	(XtArgVal) False},
};

static	Arg	line_args[] = {
	{XtNbackground,	(XtArgVal) 0},
	{XtNwidth,	(XtArgVal) 1},
	{XtNheight,	(XtArgVal) 0},
	{XtNfromHoriz,	(XtArgVal) NULL},
	{XtNborderWidth, (XtArgVal) 0},
	{XtNtop,	(XtArgVal) XtChainTop},
	{XtNbottom,	(XtArgVal) XtChainBottom},
	{XtNleft,	(XtArgVal) XtChainRight},
	{XtNright,	(XtArgVal) XtChainRight},
};

static	Arg	panel_args[] = {
	{XtNfromHoriz,	(XtArgVal) NULL},
	{XtNwidth,	(XtArgVal) (XTRA_WID - 1)},
	{XtNheight,	(XtArgVal) 0},
	{XtNborderWidth, (XtArgVal) 0},
	{XtNtop,	(XtArgVal) XtChainTop},
	{XtNbottom,	(XtArgVal) XtChainBottom},
	{XtNleft,	(XtArgVal) XtChainRight},
	{XtNright,	(XtArgVal) XtChainRight},
};

static	struct {
	_Xconst	char	*label;
	_Xconst	char	*name;
	int	closure;
	int	y_pos;
	}
	command_table[] = {
		{"Quit",	"quit",		'q',		50},
		{"Shrink2",	"sh1",		2 << 8 | 's',	150},
		{"Shrink4",	"sh2",		4 << 8 | 's',	200},
		{"Shrink6",	"sh3",		6 << 8 | 's',	250},
		{"Shrink8",	"sh4",		8 << 8 | 's',	300},
		{"Page-10",	"prev10",	10 << 8 | 'p',	400},
		{"Page-5",	"prev5",	5 << 8 | 'p',	450},
		{"Prev",	"prev",		'p',		500},
		{"Next",	"next",		'n',		600},
		{"Page+5",	"next5",	5 << 8 | 'n',	650},
		{"Page+10",	"next10",	10 << 8 | 'n',	700},
#if	PS
		{"View PS",	"postscript",	'v',		750},
#endif
};

static	void	handle_command();

static	XtCallbackRec	command_call[] = {
	{handle_command, NULL},
	{NULL,		NULL},
};

static	Arg	command_args[] = {
	{XtNlabel,	(XtArgVal) NULL},
	{XtNx,		(XtArgVal) 6},
	{XtNy,		(XtArgVal) 0},
	{XtNwidth,	(XtArgVal) 64},
	{XtNheight,	(XtArgVal) 30},
	{XtNcallback,	(XtArgVal) command_call},
};

void
create_buttons(h)
	XtArgVal	h;
{
	int i;

	line_args[0].value = (XtArgVal) resource._hl_Pixel;
	line_args[2].value = h;
	line_args[3].value = (XtArgVal) vport_widget;
	line_widget = XtCreateManagedWidget("line", widgetClass, form_widget,
		line_args, XtNumber(line_args));
	panel_args[0].value = (XtArgVal) line_widget;
	panel_args[2].value = h;
	panel_widget = XtCreateManagedWidget("panel", compositeWidgetClass,
		form_widget, panel_args, XtNumber(panel_args));

	command_args[2].value = (XtArgVal) vport_widget;
	for (i = 0; i < XtNumber(command_table); ++i) {
	    command_args[0].value = (XtArgVal) command_table[i].label;
	    command_args[2].value = (XtArgVal) command_table[i].y_pos;
	    command_call[0].closure = (XtPointer) &command_table[i].closure;
	    (void) XtCreateManagedWidget(command_table[i].name,
		commandWidgetClass, panel_widget,
		command_args, XtNumber(command_args));
	}
}
#endif	/* BUTTONS */

#else	/* !TOOLKIT */
static	Window	x_bar, y_bar;
static	int	x_bgn, x_end, y_bgn, y_end;	/* scrollbar positions */
#endif	/* TOOLKIT */

/*
 *	Mechanism to keep track of the magnifier window.  The problems are,
 *	(a) if the button is released while the window is being drawn, this
 *	could cause an X error if we continue drawing in it after it is
 *	destroyed, and
 *	(b) creating and destroying the window too quickly confuses the window
 *	manager, which is avoided by waiting for an expose event before
 *	destroying it.
 */
static	short	alt_stat;	/* 1 = wait for expose, */
				/* -1 = destroy upon expose */
static	Boolean	alt_canit;	/* stop drawing this window */

/*
 *	Data for buffered events.
 */

static	VOLATILE short	event_freq	= 70;

static	void	can_exposures(), keystroke();

#ifdef	GREY
#define	gamma	resource._gamma

void
init_pix(warn)
	Boolean	warn;
{
	static	int	shrink_allocated_for = 0;
	static	float	oldgamma	= 0.0;
	int	i;

	if (gamma != oldgamma) {
	    static Pixel plane_masks[4];
	    static Pixel pixel;
	    XColor color, fc, bc;
	    XGCValues	values;

	    if (!copy)
		/* allocate 4 color planes for 16 colors (for GXor drawing) */
		if (oldgamma == 0.0 && !XAllocColorCells(DISP,
			DefaultColormapOfScreen(SCRN), False, plane_masks, 4,
			&pixel, 1))
		    copy = warn = True;

	    /* get foreground and background RGB values for interpolating */
	    fc.pixel = fore_Pixel;
	    XQueryColor(DISP, DefaultColormapOfScreen(SCRN), &fc);
	    bc.pixel = back_Pixel;
	    XQueryColor(DISP, DefaultColormapOfScreen(SCRN), &bc);

	    for (i = 0; i < 16; ++i) {
		double	pow();
		double	frac = gamma > 0 ? pow((double) i / 15, 1 / gamma)
		    : 1 - pow((double) (15 - i) / 15, -gamma);

		color.red = frac * ((double) fc.red - bc.red) + bc.red;
		color.green = frac * ((double) fc.green - bc.green) + bc.green;
		color.blue = frac * ((double) fc.blue - bc.blue) + bc.blue;

		color.pixel = pixel;
		color.flags = DoRed | DoGreen | DoBlue;

		if (!copy) {
		    if (i & 1) color.pixel |= plane_masks[0];
		    if (i & 2) color.pixel |= plane_masks[1];
		    if (i & 4) color.pixel |= plane_masks[2];
		    if (i & 8) color.pixel |= plane_masks[3];
		    XStoreColor(DISP, DefaultColormapOfScreen(SCRN), &color);
		    palette[i] = color.pixel;
		}
		else {
		    if (!XAllocColor(DISP, DefaultColormapOfScreen(SCRN),
			&color))
			palette[i] = (i * 100 >= density * 15)
			    ? fore_Pixel : back_Pixel;
		    else
			palette[i] = color.pixel;
		}
	    }

	    /* Make sure fore_ and back_Pixel are a part of the palette */
	    fore_Pixel = palette[15];
	    back_Pixel = palette[0];
	    if (mane.win != (Window) 0)
		XSetWindowBackground(DISP, mane.win, palette[0]);

#define	MakeGC(fcn, fg, bg)	(values.function = fcn, values.foreground=fg,\
		values.background=bg,\
		XCreateGC(DISP, RootWindowOfScreen(SCRN),\
			GCFunction|GCForeground|GCBackground, &values))

	    foreGC = ruleGC = MakeGC(copy ? GXcopy : GXor,
		fore_Pixel, back_Pixel);
	    foreGC2 = NULL;

	    oldgamma = gamma;
	    if (resource.copy == Maybe && copy && warn)
		Puts("Note:  overstrike characters may be incorrect.");
	}
#undef	MakeGC

	if (mane.shrinkfactor == 1) return;

	if (shrink_allocated_for < mane.shrinkfactor) {
	    if (pixeltbl != NULL) free((char *) pixeltbl);
	    pixeltbl = (Pixel *) xmalloc((unsigned)
		(mane.shrinkfactor * mane.shrinkfactor + 1) * sizeof(Pixel),
		"pixel table");
	    shrink_allocated_for = mane.shrinkfactor;
	}

	for (i = 0; i <= mane.shrinkfactor * mane.shrinkfactor; ++i)
	    pixeltbl[i] =
		palette[(i * 30 + mane.shrinkfactor * mane.shrinkfactor)
		    / (2 * mane.shrinkfactor * mane.shrinkfactor)];
}
#endif	/* GREY */

/*
 *	Event-handling routines
 */

static	void
expose(windowrec, x, y, w, h)
	register struct WindowRec *windowrec;
	int		x, y;
	unsigned int	w, h;
{
	if (windowrec->min_x > x) windowrec->min_x = x;
	if (windowrec->max_x < x + w)
	    windowrec->max_x = x + w;
	if (windowrec->min_y > y) windowrec->min_y = y;
	if (windowrec->max_y < y + h)
	    windowrec->max_y = y + h;
}

static	void
clearexpose(windowrec, x, y, w, h)
	struct WindowRec *windowrec;
	int		x, y;
	unsigned int	w, h;
{
	XClearArea(DISP, windowrec->win, x, y, w, h, False);
	expose(windowrec, x, y, w, h);
}

static	void
scrollwindow(windowrec, x0, y0)
	register struct WindowRec *windowrec;
	int	x0, y0;
{
	int	x, y;
	int	x2 = 0, y2 = 0;
	int	ww, hh;

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
	if (windowrec->min_x < 0) windowrec->min_x = 0;
	windowrec->max_x -= x;
	if (windowrec->max_x > windowrec->width)
	    windowrec->max_x = windowrec->width;
	windowrec->min_y -= y;
	if (windowrec->min_y < 0) windowrec->min_y = 0;
	windowrec->max_y -= y;
	if (windowrec->max_y > windowrec->height)
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
	    XCopyArea(DISP, windowrec->win, windowrec->win,
		DefaultGCOfScreen(SCRN), x, y,
		(unsigned int) ww, (unsigned int) hh, x2, y2);
	    if (x > 0)
		clearexpose(windowrec, ww, 0,
		    (unsigned int) x, windowrec->height);
	    if (x2 > 0)
		clearexpose(windowrec, 0, 0,
		    (unsigned int) x2, windowrec->height);
	    if (y > 0)
		clearexpose(windowrec, 0, hh,
		    windowrec->width, (unsigned int) y);
	    if (y2 > 0)
		clearexpose(windowrec, 0, 0,
		    windowrec->width, (unsigned int) y2);
	}
}

#ifdef	TOOLKIT
/*
 *	routines for X11 toolkit
 */

static	Arg	arg_wh[] = {
	{XtNwidth,	(XtArgVal) &window_w},
	{XtNheight,	(XtArgVal) &window_h},
};

static	Position	window_x, window_y;
static	Arg	arg_xy[] = {
	{XtNx,		(XtArgVal) &window_x},
	{XtNy,		(XtArgVal) &window_y},
};

#define	get_xy()	XtGetValues(draw_widget, arg_xy, XtNumber(arg_xy))

#define	mane_base_x	0
#define	mane_base_y	0

static	void
home(scrl)
	Boolean	scrl;
{
#if	PS
	psp.interrupt();
#endif
	if (!scrl) XUnmapWindow(DISP, mane.win);
	get_xy();
	if (x_bar != NULL) {
	    register int coord = (page_w - clip_w) / 2;
	    if (coord > home_x / mane.shrinkfactor)
		coord = home_x / mane.shrinkfactor;
	    XtCallCallbacks(x_bar, XtNscrollProc,
		(XtPointer) (window_x + coord));
	}
	if (y_bar != NULL) {
	    register int coord = (page_h - clip_h) / 2;
	    if (coord > home_y / mane.shrinkfactor)
		coord = home_y / mane.shrinkfactor;
	    XtCallCallbacks(y_bar, XtNscrollProc,
		(XtPointer) (window_y + coord));
	}
	if (!scrl) {
	    XMapWindow(DISP, mane.win);
	    /* Wait for the server to catch up---this eliminates flicker. */
	    XSync(DISP, False);
	}
}

static	Boolean	resized	= False;

static	void
get_geom()
{
	static	Dimension	new_clip_w, new_clip_h;
	static	Arg	arg_wh_clip[] = {
		{XtNwidth,	(XtArgVal) &new_clip_w},
		{XtNheight,	(XtArgVal) &new_clip_h},
	};
	register int	old_clip_w;

	XtGetValues(vport_widget, arg_wh, XtNumber(arg_wh));
	XtGetValues(clip_widget, arg_wh_clip, XtNumber(arg_wh_clip));
	/* Note:  widgets may be destroyed but not forgotten */
	x_bar = page_w <= new_clip_w ? NULL
	    : XtNameToWidget(vport_widget, "horizontal");
	y_bar = page_h <= new_clip_h ? NULL
	    : XtNameToWidget(vport_widget, "vertical");
	old_clip_w = clip_w;
			/* we need to do this because */
			/* sizeof(Dimension) != sizeof(int) */
	clip_w = new_clip_w;
	clip_h = new_clip_h;
	if (old_clip_w == 0) home(False);
	resized = False;
}

static	void
center(x, y)
	int x, y;
{
/*	We use the clip widget here because it gives a more exact value. */
	x -= clip_w/2;
	y -= clip_h/2;
	if (x_bar) XtCallCallbacks(x_bar, XtNscrollProc, (XtPointer) x);
	if (y_bar) XtCallCallbacks(y_bar, XtNscrollProc, (XtPointer) y);
	XWarpPointer(DISP, None, None, 0, 0, 0, 0, -x, -y);
}

/*
 *	callback routines
 */

	/*ARGSUSED*/
void
handle_resize(widget, junk, event, cont)
	Widget	widget;
	XtPointer junk;
	XEvent	*event;
	Boolean	*cont;		/* unused */
{
	resized = True;
}

#ifdef	BUTTONS
	/*ARGSUSED*/
static	void
handle_command(widget, client_data_p, call_data)
	Widget	widget;
	XtPointer client_data_p;
	XtPointer call_data;
{
	int	client_data	= * (int *) client_data_p;

	keystroke((client_data) & 0xff, (client_data) >> 8,
		((client_data) >> 8) != 0, (XEvent *) NULL);
}
#endif	/* BUTTONS */

void
reconfig()
{
#ifdef	BUTTONS
	XtSetValues(vport_widget, resizable_off, XtNumber(resizable_off));
#endif
	XdviResizeWidget(draw_widget, page_w, page_h);
	get_geom();
}

#else	/* !TOOLKIT */

/*
 *	brute force scrollbar routines
 */

static	void
paint_x_bar()
{
	register int	new_x_bgn = mane.base_x * clip_w / page_w;
	register int	new_x_end = (mane.base_x + clip_w) * clip_w / page_w;

	if (new_x_bgn >= x_end || x_bgn >= new_x_end) {	/* no overlap */
	    XClearArea(DISP, x_bar, x_bgn, 1, x_end - x_bgn, BAR_WID, False);
	    XFillRectangle(DISP, x_bar, ruleGC,
		new_x_bgn, 1, new_x_end - new_x_bgn, BAR_WID);
	}
	else {		/* this stuff avoids flicker */
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

static	void
paint_y_bar()
{
	register int	new_y_bgn = mane.base_y * clip_h / page_h;
	register int	new_y_end = (mane.base_y + clip_h) * clip_h / page_h;

	if (new_y_bgn >= y_end || y_bgn >= new_y_end) {	/* no overlap */
	    XClearArea(DISP, y_bar, 1, y_bgn, BAR_WID, y_end - y_bgn, False);
	    XFillRectangle(DISP, y_bar, ruleGC,
		1, new_y_bgn, BAR_WID, new_y_end - new_y_bgn);
	}
	else {		/* this stuff avoids flicker */
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

static	void
scrollmane(x, y)
	int	x, y;
{
	register int	old_base_x = mane.base_x;
	register int	old_base_y = mane.base_y;

#if	PS
	psp.interrupt();
#endif
	if (x > (int) (page_w - clip_w)) x = page_w - clip_w;
	if (x < 0) x = 0;
	if (y > (int) (page_h - clip_h)) y = page_h - clip_h;
	if (y < 0) y = 0;
	scrollwindow(&mane, x, y);
	if (old_base_x != mane.base_x && x_bar) paint_x_bar();
	if (old_base_y != mane.base_y && y_bar) paint_y_bar();
}

void
reconfig()
{
	int	x_thick = 0;
	int	y_thick = 0;

		/* determine existence of scrollbars */
	if (window_w < page_w) x_thick = BAR_THICK;
	if (window_h - x_thick < page_h) y_thick = BAR_THICK;
	clip_w = window_w - y_thick;
	if (clip_w < page_w) x_thick = BAR_THICK;
	clip_h = window_h - x_thick;

		/* process drawing (clip) window */
	if (mane.win == (Window) 0) {	/* initial creation */
	    XWindowAttributes attrs;

	    mane.win = XCreateSimpleWindow(DISP, top_level, y_thick, x_thick,
			(unsigned int) clip_w, (unsigned int) clip_h, 0,
			brdr_Pixel, back_Pixel);
	    XSelectInput(DISP, mane.win, ExposureMask |
			ButtonPressMask | ButtonMotionMask | ButtonReleaseMask);
	    (void) XGetWindowAttributes(DISP, mane.win, &attrs);
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
				(unsigned int) clip_w, BAR_THICK - 1, 1,
				brdr_Pixel, back_Pixel);
		XSelectInput(DISP, x_bar,
			ExposureMask | ButtonPressMask | Button2MotionMask);
		XMapWindow(DISP, x_bar);
	    }
	    x_bgn = mane.base_x * clip_w / page_w;
	    x_end = (mane.base_x + clip_w) * clip_w / page_w;
	}
	else
	    if (x_bar) {
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
				BAR_THICK - 1, (unsigned int) clip_h, 1,
				brdr_Pixel, back_Pixel);
		XSelectInput(DISP, y_bar,
			ExposureMask | ButtonPressMask | Button2MotionMask);
		XMapWindow(DISP, y_bar);
	    }
	    y_bgn = mane.base_y * clip_h / page_h;
	    y_end = (mane.base_y + clip_h) * clip_h / page_h;
	}
	else
	    if (y_bar) {
		XDestroyWindow(DISP, y_bar);
		y_bar = (Window) 0;
	    }
}

static	void
home(scrl)
	Boolean	scrl;
{
	int	x = 0, y = 0;

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
	    if (x_bar) paint_x_bar();
	    if (y_bar) paint_y_bar();
	}
}

#define	get_xy()
#define	window_x 0
#define	window_y 0
#define	mane_base_x	mane.base_x
#define	mane_base_y	mane.base_y
#endif	/* TOOLKIT */

static	void
compute_mag_pos(xp, yp)
	int	*xp, *yp;
{
	register int t;

	t = mag_x + main_x - alt.width/2;
	if (t > WidthOfScreen(SCRN) - (int) alt.width - 2*MAGBORD)
	    t = WidthOfScreen(SCRN) - (int) alt.width - 2*MAGBORD;
	if (t < 0) t = 0;
	*xp = t;
	t = mag_y + main_y - alt.height/2;
	if (t > HeightOfScreen(SCRN) - (int) alt.height - 2*MAGBORD)
	    t = HeightOfScreen(SCRN) - (int) alt.height - 2*MAGBORD;
	if (t < 0) t = 0;
	*yp = t;
}


#define	TRSIZE	100

#ifdef	TOOLKIT
	/*ARGSUSED*/
void
handle_key(widget, junk, eventp, cont)
	Widget	widget;
	XtPointer junk;
	XEvent	*eventp;
	Boolean	*cont;		/* unused */
#else	/* !TOOLKIT */
void
handle_key(eventp)
	XEvent *eventp;
#endif	/* TOOLKIT */
{
	static	Boolean	has_arg		= False;
	static	int	number		= 0;
	static	int	sign		= 1;
	char	ch;
	Boolean	arg0;
	int	number0;
	char	trbuf[TRSIZE];
	int	nbytes;

	nbytes = XLookupString(&eventp->xkey, trbuf, TRSIZE, (KeySym *) NULL,
	    (XComposeStatus *) NULL);
	if (nbytes == 0) return;
	ch = '\0';
	if (nbytes == 1) ch = *trbuf;
	if (ch >= '0' && ch <= '9') {
	    has_arg = True;
	    number = number * 10 + sign * (ch - '0');
	    return;
	}
	else if (ch == '-') {
	    has_arg = True;
	    sign = -1;
	    number = 0;
	    return;
	}
	number0 = number;
	number = 0;
	sign = 1;
	arg0 = has_arg;
	has_arg = False;
	keystroke(ch, number0, arg0, eventp);
}

#ifdef	TOOLKIT
	/*ARGSUSED*/
void
handle_button(widget, junk, ev, cont)
	Widget	widget;
	XtPointer junk;
	XEvent *ev;
#define	event	(&(ev->xbutton))
	Boolean	*cont;		/* unused */
#else	/* !TOOLKIT */
void
handle_button(event)
	XButtonEvent *event;
#endif	/* TOOLKIT */
{
	int	x, y;
	struct mg_size_rec	*size_ptr = mg_size + event->button - 1;
	XSetWindowAttributes attr;

	if (alt.win != (Window) 0 || mane.shrinkfactor == 1 || size_ptr->w <= 0)
	    XBell(DISP, 20);
	else {
	    mag_x = event->x;
	    mag_y = event->y;
	    alt.width = size_ptr->w;
	    alt.height = size_ptr->h;
	    main_x = event->x_root - mag_x;
	    main_y = event->y_root - mag_y;
	    compute_mag_pos(&x, &y);
	    alt.base_x = (event->x + mane_base_x) * mane.shrinkfactor -
		alt.width/2;
	    alt.base_y = (event->y + mane_base_y) * mane.shrinkfactor -
		alt.height/2;
	    attr.save_under = True;
	    attr.border_pixel = brdr_Pixel;
	    attr.background_pixel = back_Pixel;
	    attr.override_redirect = True;
	    alt.win = XCreateWindow(DISP, RootWindowOfScreen(SCRN),
			x, y, alt.width, alt.height, MAGBORD,
			0,	/* depth from parent */
			InputOutput, (Visual *) CopyFromParent,
			CWSaveUnder | CWBorderPixel | CWBackPixel |
			CWOverrideRedirect, &attr);
	    XSelectInput(DISP, alt.win, ExposureMask);
	    XMapWindow(DISP, alt.win);
	    alt_stat = 1;	/* waiting for exposure */
	}
}

#ifdef	TOOLKIT
#undef	event

	/*ARGSUSED*/
void
handle_motion(widget, junk, ev, cont)
	Widget	widget;
	XtPointer junk;
	XEvent *ev;
#define	event	(&(ev->xmotion))
	Boolean	*cont;		/* unused */
{
	new_mag_x = event->x;
	main_x = event->x_root - new_mag_x;
	new_mag_y = event->y;
	main_y = event->y_root - new_mag_y;
	mag_moved = (new_mag_x != mag_x || new_mag_y != mag_y);
}

#undef	event
#endif	/* TOOLKIT */

static	void
movemag(x, y)
	int	x, y;
{
	int	xx, yy;

	mag_x = x;
	mag_y = y;
	if (mag_x == new_mag_x && mag_y == new_mag_y) mag_moved = False;
	compute_mag_pos(&xx, &yy);
	XMoveWindow(DISP, alt.win, xx, yy);
	scrollwindow(&alt,
	    (x + mane_base_x) * mane.shrinkfactor - (int) alt.width/2,
	    (y + mane_base_y) * mane.shrinkfactor - (int) alt.height/2);
}

#ifdef	TOOLKIT
	/*ARGSUSED*/
void
handle_release(widget, junk, ev, cont)
	Widget	widget;
	XtPointer junk;
	XEvent *ev;
#define	event	(&(ev->xbutton))
	Boolean	*cont;		/* unused */
#else	/* !TOOLKIT */
void
handle_release()
#endif	/* TOOLKIT */
{
	if (alt.win != (Window) 0)
	    if (alt_stat) alt_stat = -1;	/* destroy upon expose */
	    else {
		XDestroyWindow(DISP, alt.win);
		if (currwin.win == alt.win) alt_canit = True;
		alt.win = (Window) 0;
		mag_moved = False;
		can_exposures(&alt);
	    }
}

#ifdef	TOOLKIT
#undef	event

	/*ARGSUSED*/
void
handle_exp(widget, closure, ev, cont)
	Widget	widget;
	XtPointer closure;
	register XEvent *ev;
#define	event	(&(ev->xexpose))
	Boolean	*cont;		/* unused */
{
	struct WindowRec *windowrec = (struct WindowRec *) closure;

	if (windowrec == &alt)
	    if (alt_stat < 0) {	/* destroy upon exposure */
		alt_stat = 0;
		handle_release(widget, (caddr_t) NULL, ev, (Boolean *) NULL);
		return;
	    }
	    else
		alt_stat = 0;
	expose(windowrec, event->x, event->y,
	    (unsigned int) event->width, (unsigned int) event->height);
}

#undef	event
#endif	/* TOOLKIT */

void
showmessage(message)
	_Xconst	char	*message;
{
	get_xy();
	XDrawImageString(DISP, mane.win, foreGC,
	    5 - window_x, 5 + X11HEIGHT - window_y, message, strlen(message));
}

/* |||
 *	Currently the event handler does not coordinate XCopyArea requests
 *	with GraphicsExpose events.  This can lead to problems if the window
 *	is partially obscured and one, for example, drags a scrollbar.
 */

static	void
keystroke(ch, number0, arg0, eventp)
	char	ch;
	int	number0;
	Boolean	arg0;
	XEvent	*eventp;
{
	int	next_page;
#ifdef	TOOLKIT
	Window	ww;
#endif

	next_page = current_page;
	switch (ch) {
	    case 'q':
	    case '\003':	/* control-C */
	    case '\004':	/* control-D */
#ifdef	VMS
	    case '\032':	/* control-Z */
#endif
#if	PS
		ps_destroy();
#endif
		exit(0);

	    case 'n':
	    case 'f':
	    case ' ':
	    case '\r':
	    case '\n':
		/* scroll forward; i.e. go to relative page */
		next_page = current_page + (arg0 ? number0 : 1);
		break;
	    case 'p':
	    case 'b':
	    case '\b':
	    case '\177':	/* Del */
		/* scroll backward */
		next_page = current_page - (arg0 ? number0 : 1);
		break;
	    case 'g':
		/* go to absolute page */
		next_page = (arg0 ? number0 - pageno_correct :
		    total_pages - 1);
		break;
	    case 'P':		/* declare current page */
		pageno_correct = arg0 * number0 - current_page;
		return;
	    case 'k':		/* toggle keep-position flag */
		resource.keep_flag = (arg0 ? number0 : !resource.keep_flag);
		return;
	    case '\f':
		/* redisplay current page */
		break;
	    case '^':
		home(True);
		return;
#ifdef	TOOLKIT
	    case 'l':
		if (!x_bar) goto bad;
		XtCallCallbacks(x_bar, XtNscrollProc,
		    (XtPointer) (-2 * (int) clip_w / 3));
		return;
	    case 'r':
		if (!x_bar) goto bad;
		XtCallCallbacks(x_bar, XtNscrollProc,
		    (XtPointer) (2 * (int) clip_w / 3));
		return;
	    case 'u':
		if (!y_bar) goto bad;
		XtCallCallbacks(y_bar, XtNscrollProc,
		    (XtPointer) (-2 * (int) clip_h / 3));
		return;
	    case 'd':
		if (!y_bar) goto bad;
		XtCallCallbacks(y_bar, XtNscrollProc,
		    (XtPointer) (2 * (int) clip_h / 3));
		return;
	    case 'c':
		center(eventp->xkey.x, eventp->xkey.y);
		return;
	    case 'M':
		(void) XTranslateCoordinates(DISP, eventp->xkey.window,
			mane.win, eventp->xkey.x, eventp->xkey.y,
			&home_x, &home_y, &ww);	/* throw away last argument */
		home_x *= mane.shrinkfactor;
		home_y *= mane.shrinkfactor;
		return;
#ifdef	BUTTONS
	    case 'x':
		if (arg0 && resource.expert == (number0 != 0)) return;
		XtSetValues(vport_widget, resizable_on,
		    XtNumber(resizable_off));
		if (resource.expert) {	/* create buttons */
		    XdviResizeWidget(vport_widget,
			window_w -= XTRA_WID, window_h);
		    create_buttons((XtArgVal) window_h);
		    resource.expert = False;
		}
		else {		/* destroy buttons */
		    XtDestroyWidget(panel_widget);
		    XtDestroyWidget(line_widget);
		    XdviResizeWidget(vport_widget,
			window_w += XTRA_WID, window_h);
		    resource.expert = True;
		}
		return;
#endif	/* BUTTONS */
#else	/* !TOOLKIT */
	    case 'l':
		if (mane.base_x <= 0) goto bad;
		scrollmane(mane.base_x - 2 * (int) clip_w / 3, mane.base_y);
		return;
	    case 'r':
		if (mane.base_x >= page_w - clip_w) goto bad;
		scrollmane(mane.base_x + 2 * (int) clip_w / 3, mane.base_y);
		return;
	    case 'u':
		if (mane.base_y <= 0) goto bad;
		scrollmane(mane.base_x, mane.base_y - 2 * (int) clip_h / 3);
		return;
	    case 'd':
		if (mane.base_y >= page_h - clip_h) goto bad;
		scrollmane(mane.base_x, mane.base_y + 2 * (int) clip_h / 3);
		return;
	    case 'c':	/* unchecked scrollmane() */
		scrollwindow(&mane, mane.base_x + eventp->xkey.x - clip_w/2,
		    mane.base_y + eventp->xkey.y - clip_h/2);
		if (x_bar) paint_x_bar();
		if (y_bar) paint_y_bar();
		XWarpPointer(DISP, None, None, 0, 0, 0, 0,
		    clip_w/2 - eventp->xkey.x, clip_h/2 - eventp->xkey.y);
		return;
	    case 'M':
		home_x = (eventp->xkey.x - (y_bar ? BAR_THICK : 0)
		    + mane.base_x) * mane.shrinkfactor;
		home_y = (eventp->xkey.y - (x_bar ? BAR_THICK : 0)
		    + mane.base_y) * mane.shrinkfactor;
		return;
#endif	/* TOOLKIT */

	    case '\020':	/* Control P */
		Printf("Unit = %d, bitord = %d, byteord = %d\n",
		    BitmapUnit(DISP), BitmapBitOrder(DISP),
		    ImageByteOrder(DISP));
		return;
	    case 's':
		if (!arg0) {
		    int temp;
		    number0 = ROUNDUP(unshrunk_page_w, window_w - 2);
		    temp = ROUNDUP(unshrunk_page_h, window_h - 2);
		    if (number0 < temp) number0 = temp;
		}
		if (number0 <= 0) goto bad;
		if (number0 == mane.shrinkfactor) return;
		mane.shrinkfactor = number0;
		init_page();
		if (number0 != 1 && number0 != bak_shrink) {
		    bak_shrink = number0;
#ifdef	GREY
		    if (use_grey) init_pix(False);
#endif
		    reset_fonts();
		}
		reconfig();
		home(False);
		break;
	    case 'S':
		if (!arg0) goto bad;
#ifdef	GREY
		if (use_grey) {
		    float newgamma = number0 != 0 ? number0 / 100.0 : 1.0;

		    if (newgamma == gamma) return;
		    gamma = newgamma;
		    init_pix(False);
		    return;
		}
#endif
		if (number0 < 0) goto bad;
		if (number0 == density) return;
		density = number0;
		reset_fonts();
		if (mane.shrinkfactor == 1) return;
		break;

#ifdef	GREY
	    case 'G':
		use_grey = (arg0 ? number0 : !use_grey);
		if (use_grey) init_pix(False);
		reset_fonts();
		break;
#endif

#if	PS
	    case 'v':
		if (!arg0 || resource._postscript != !number0) {
		    resource._postscript = !resource._postscript;
		    if (resource._postscript) scanned_page = scanned_page_bak;
		    psp.toggle();
		}
		break;
#endif

	    case 'R':
		/* reread DVI file */
		--dvi_time;	/* then it will notice a change */
		break;
	    default:
		goto bad;
	}
	if (0 <= next_page && next_page < total_pages) {
	    if (current_page != next_page) {
		current_page = next_page;
		hush_spec_now = hush_spec;
		if (!resource.keep_flag) home(False);
	    }
	    canit = True;
	    XFlush(DISP);
	    return;	/* Don't use longjmp here:  it might be called from
			 * within the toolkit, and we don't want to longjmp out
			 * of Xt routines. */
	}
	bad:  XBell(DISP, 10);
}

void
read_events(wait)
	wide_bool	wait;
{
	XEvent	event;

	alt_canit = False;
	for (;;) {
	    event_counter = event_freq;
	    /*
	     * The above line clears the flag indicating that an event is
	     * pending.  So if an event comes in right now, the flag will be
	     * set again needlessly, but we just end up making an extra call.
	     * Also, be careful about destroying the magnifying glass while
	     * writing it.
	     */
	    if (!XtPending() && (!wait || canit || mane.min_x < MAXDIM ||
		    alt.min_x < MAXDIM || mag_moved)) {
		if (!wait && (canit | alt_canit)) {
#if	PS
		    psp.interrupt();
#endif
		    if (allow_can) longjmp(canit_env, 1);
		}
		return;
	    }
#ifdef	TOOLKIT
	    XtNextEvent(&event);
	    if (resized) get_geom();
	    if (event.xany.window == alt.win && event.type == Expose) {
		handle_exp((Widget) NULL, (XtPointer) &alt, &event,
		    (Boolean *) NULL);
		continue;
	    }
	    (void) XtDispatchEvent(&event);
#else	/* !TOOLKIT */

	    XNextEvent(DISP, &event);
	    if (event.xany.window == mane.win || event.xany.window == alt.win) {
		struct WindowRec *wr = &mane;

		if (event.xany.window == alt.win) {
		    wr = &alt;
		    /* check in case we already destroyed the window */
		    if (alt_stat < 0) { /* destroy upon exposure */
			alt_stat = 0;
			handle_release();
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

		case MotionNotify:
		    new_mag_x = event.xmotion.x;
		    new_mag_y = event.xmotion.y;
		    mag_moved = (new_mag_x != mag_x || new_mag_y != mag_y);
		    break;

		case ButtonPress:
		    handle_button(&event.xbutton);
		    break;

		case ButtonRelease:
		    handle_release();
		    break;
		}	/* end switch */
	    }	/* end if window == {mane,alt}.win */

	    else if (event.xany.window == x_bar) {
		if (event.type == Expose)
		    XFillRectangle(DISP, x_bar, ruleGC,
			x_bgn, 1, x_end - x_bgn, BAR_WID);
		else if (event.type == MotionNotify)
		    scrollmane(event.xmotion.x * page_w / clip_w,
			mane.base_y);
		else switch (event.xbutton.button)
		{
		    case 1:
			scrollmane(mane.base_x + event.xbutton.x, mane.base_y);
			break;
		    case 2:
			scrollmane(event.xbutton.x * page_w / clip_w,
			    mane.base_y);
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
		    scrollmane(mane.base_x,
			event.xmotion.y * page_h / clip_h);
		else switch (event.xbutton.button)
		{
		    case 1:
			scrollmane(mane.base_x, mane.base_y + event.xbutton.y);
			break;
		    case 2:
			scrollmane(mane.base_x,
			    event.xbutton.y * page_h / clip_h);
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
			    register Window old_mane_win = mane.win;

			    window_w = event.xconfigure.width;
			    window_h = event.xconfigure.height;
			    reconfig();
			    if (old_mane_win == (Window) 0) home(False);
		    }
		    break;

		case MapNotify:		/* if running w/o WM */
		    if (mane.win == (Window) 0) {
			reconfig();
			home(False);
		    }
		    break;

		case KeyPress:
		    handle_key(&event);
		    break;
		}
#endif	/* TOOLKIT */
	}
}

static	void
redraw(windowrec)
	struct WindowRec *windowrec;
{

	currwin = *windowrec;
	min_x = currwin.min_x + currwin.base_x;
	min_y = currwin.min_y + currwin.base_y;
	max_x = currwin.max_x + currwin.base_x;
	max_y = currwin.max_y + currwin.base_y;
	can_exposures(windowrec);

	if (debug & DBG_EVENT)
	    Printf("Redraw %d x %d at (%d, %d) (base=%d,%d)\n", max_x - min_x,
		max_y - min_y, min_x, min_y, currwin.base_x, currwin.base_y);
	XDefineCursor(DISP, mane.win, redraw_cursor);
	XFlush(DISP);
	if (setjmp(dvi_env)) {
	    XClearWindow(DISP, mane.win);
	    showmessage(dvi_oops_msg);
	    if (dvi_file) {
		Fclose(dvi_file);
		dvi_file = NULL;
	    }
	}
	else {
	    draw_page();
	    hush_spec_now = True;
	}
}

void
redraw_page()
{
	if (debug & DBG_EVENT) Fputs("Redraw page:  ", stdout);
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

static	void
can_exposures(windowrec)
	struct WindowRec *windowrec;
{
	windowrec->min_x = windowrec->min_y = MAXDIM;
	windowrec->max_x = windowrec->max_y = 0;
}

#if	HAS_SIGIO
/* ARGSUSED */
static	void
handle_intr(signo)
	int	signo;
{
	event_counter = 1;
	event_freq = -1;	/* forget Plan B */
}

static	void
enable_intr() {
	int	socket	= ConnectionNumber(DISP);

#ifdef SA_RESTART
	/* Subprocess handling, e.g., MakeTeXPK, fails on the Alpha without
	   this, because SIGIO interrupts the call of system(3), since OSF/1
	   doesn't retry interrupted wait calls by default.  From code by
	   maj@cl.cam.ac.uk.  */
	{
	    struct sigaction a;
	    a.sa_handler = handle_intr;
	    sigemptyset(&a.sa_mask);
	    sigaddset(&a.sa_mask, SIGIO);
	    a.sa_flags = SA_RESTART;
	    sigaction(SIGIO, &a, NULL);
	}
#else
	(void) signal(SIGIO, handle_intr);
#endif	/* SA_RESTART */
	(void) fcntl(socket, F_SETOWN, getpid());
	(void) fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | FASYNC);
}
#endif	/* HAS_SIGIO */

void
do_pages()
{
	if (debug & DBG_BATCH) {
#ifdef	TOOLKIT
	    while (mane.min_x == MAXDIM) read_events(True);
#else	/* !TOOLKIT */
	    while (mane.min_x == MAXDIM)
		if (setjmp(canit_env)) break;
		else read_events(True);
#endif	/* TOOLKIT */
	    for (current_page = 0; current_page < total_pages; ++current_page) {
#ifdef	__convex__
		/* convex C turns off optimization for the entire function
		   if setjmp return value is discarded.*/
		if (setjmp(canit_env))	/*optimize me*/;
#else
		(void) setjmp(canit_env);
#endif
		canit = False;
		redraw_page();
	    }
	}
	else {	/* normal operation */
#if	HAS_SIGIO
	    enable_intr();
#endif
#ifdef	__convex__
	    /* convex C turns off optimization for the entire function
	       if setjmp return value is discarded.*/
	    if (setjmp(canit_env))	/*optimize me*/;
#else
	    (void) setjmp(canit_env);
#endif
	    for (;;) {
		if (mane.win != (Window) 0)
		    XDefineCursor(DISP, mane.win, ready_cursor);
		read_events(True);
		if (canit) {
		    canit = False;
		    can_exposures(&mane);
		    can_exposures(&alt);
		    redraw_page();
		}
		else if (mag_moved) {
		    if (alt.win == (Window) 0) mag_moved = False;
		    else if (abs(new_mag_x - mag_x) >
			2 * abs(new_mag_y - mag_y))
			    movemag(new_mag_x, mag_y);
		    else if (abs(new_mag_y - mag_y) >
			2 * abs(new_mag_x - mag_x))
			    movemag(mag_x, new_mag_y);
		    else movemag(new_mag_x, new_mag_y);
		}
		else if (alt.min_x < MAXDIM) redraw(&alt);
		else if (mane.min_x < MAXDIM) redraw(&mane);
		XFlush(DISP);
	    }
	}
}
