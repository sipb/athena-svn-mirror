/*
 * Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 * Portions of this code were taken from xterm/button.c:
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *	select.c
 *		This module handles selections.
 */

#include "globals.h"
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>
#include "3270ds.h"
#include "appres.h"
#include "ctlr.h"
#include "screen.h"
#include "cg.h"
#include "resources.h"

#include "actionsc.h"
#include "ctlrc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "screenc.h"
#include "selectc.h"
#include "tablesc.h"
#include "utilc.h"

/*
 * Mouse side.
 */

/* A button click establishes the boundaries of the 'fixed' area. */
static int      f_start = 0;	/* 'fixed' area */
static int      f_end = 0;

/* Mouse motion moves the boundaries of the 'varying' area. */
static int      v_start = 0;	/* 'varying' area */
static int      v_end = 0;

static unsigned long down_time = 0;
static unsigned long down1_time = 0;
static Dimension down1_x, down1_y;
static unsigned long up_time = 0;
static int      saw_motion = 0;
static int      num_clicks = 0;
static void     grab_sel();
#define NS		5
static Atom     want_sel[NS];
static struct {			/* owned selections */
	Atom            atom;	/* atom */
	char           *buffer;	/* buffer contents */
}               own_sel[NS];
static Boolean  cursor_moved = False;
static int      saved_cursor_addr;
static void     own_sels();
int             n_owned = -1;
Boolean         any_selected = False;

extern Widget  *screen;

#define CLICK_INTERVAL	200

#define event_x(event)		event->xbutton.x
#define event_y(event)		event->xbutton.y
#define event_time(event)	event->xbutton.time

#define BOUNDED_XY(event, x, y) {	\
	x = X_TO_COL(event_x(event));	\
	if (x < 0)			\
		x = 0;			\
	if (x >= COLS)			\
		x = COLS - 1;		\
	if (flipped)			\
		x = (COLS - x) - 1;	\
	y = Y_TO_ROW(event_y(event));	\
	if (y <= 0)			\
		y = 0;			\
	if (y >= ROWS)			\
		y = ROWS - 1;		\
}


static int char_class[256] = {
/* nul soh stx etx eot enq ack bel  bs  ht  nl  vt  np  cr  so  si */
    32,  1,  1,  1,  1,  1,  1,  1,  1, 32,  1,  1,  1,  1,  1,  1,
/* dle dc1 dc2 dc3 dc4 nak syn etb can  em sub esc  fs  gs  rs  us */
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/*  sp   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
/*   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 58, 59, 60, 61, 62, 63,
/*   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
    64, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
/*   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 91, 92, 93, 94, 48,
/*   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
    96, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
/*   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  del */
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,123,124,125,126,   1,
/* ---,---,---,---,---,---,---,---,---,---,---,---,---,---,---,--- */
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* ---,---,---,---,---,---,---,---,---,---,---,---,---,---,---,--- */
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* nob exc cen ste cur yen bro sec dia cop ord gui not hyp reg mac */
    32,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
/* deg plu two thr acu mu  par per ce  one mas gui one one thr que */
   176,177,178,179,180,181,182,183,184,185,186,178,188,189,190,191,
/* Agr Aac Aci Ati Adi Ari AE  Cce Egr Eac Eci Edi Igr Iac Ici Idi */
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
/* ETH Nti Ogr Oac Oci Oti Odi mul Oob Ugr Uac Uci Udi Yac THO ssh */
    48, 48, 48, 48, 48, 48, 48,215, 48, 48, 48, 48, 48, 48, 48, 48,
/* agr aac aci ati adi ari ae  cce egr eac eci edi igr iac ici idi */
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
/* eth nti ogr oac oci oti odi div osl ugr uac uci udi yac tho ydi */
    48, 48, 48, 48, 48, 48, 48,247, 48, 48, 48, 48, 48, 48, 48, 48
};

/* Parse a charClass string: [low-]high:value[,...] */
void
reclass(s)
char *s;
{
	int n;
	int low, high, value;
	int i;
	char c;

	n = -1;
	low = -1;
	high = -1;
	for (;;) {
		c = *s++;
		if (isdigit(c)) {
			if (n == -1)
				n = 0;
			n = (n * 10) + (c - '0');
			if (n > 255)
				goto fail;
		} else if (c == '-') {
			if (n == -1 || low != -1)
				goto fail;
			low = n;
			n = -1;
		} else if (c == ':') {
			if (n == -1)
				goto fail;
			high = n;
			n = -1;
		} else if (c == ',' || c == '\0') {
			if (n == -1)
				goto fail;
			value = n;
			n = -1;
			if (high == -1)
				goto fail;
			if (low == -1)
				low = high;
			if (high < low)
				goto fail;
			for (i = low; i <= high; i++)
				char_class[i] = value;
			low = -1;
			high = -1;
			if (c == '\0')
				return;
		} else
			goto fail;
	}

    fail:
	xs_warning("Error in %s string", ResCharClass);
}

static void
select_word(baddr, t)
int baddr;
Time t;
{
	unsigned char fa = *get_field_attribute(baddr);
	unsigned char ch;
	int class;

	/* Find the initial character class */
	if (FA_IS_ZERO(fa))
		ch = CG_space;
	else
		ch = screen_buf[baddr];
	class = char_class[cg2asc[ch]];

	/* Find the beginning */
	for (f_start = baddr; f_start % COLS; f_start--) {
		fa = *get_field_attribute(f_start);
		if (FA_IS_ZERO(fa))
			ch = CG_space;
		else
			ch = screen_buf[f_start];
		if (char_class[cg2asc[ch]] != class) {
			f_start++;
			break;
		}
	}

	/* Find the end */
	for (f_end = baddr; (f_end+1) % COLS; f_end++) {
		fa = *get_field_attribute(f_end);
		if (FA_IS_ZERO(fa))
			ch = CG_space;
		else
			ch = screen_buf[f_end];
		if (char_class[cg2asc[ch]] != class) {
			f_end--;
			break;
		}
	}

	v_start = f_start;
	v_end = f_end;
	grab_sel(f_start, f_end, True, t);
}

static void
select_line(baddr, t)
int baddr;
Time t;
{
	f_start = baddr - (baddr % COLS);
	f_end = f_start + COLS - 1;
	v_start = f_start;
	v_end = f_end;
	grab_sel(f_start, f_end, True, t);
}


/*
 * Start a new selection.
 * Usually bound to <Btn1Down>.
 */
/*ARGSUSED*/
void
select_start_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int x, y;
	register int baddr;

	if (w != *screen)
		return;
	action_debug(select_start_action, event, params, num_params);
	BOUNDED_XY(event, x, y);
	baddr = ROWCOL_TO_BA(y, x);
	f_start = f_end = v_start = v_end = baddr;
	down1_time = down_time = event_time(event);
	down1_x = event_x(event);
	down1_y = event_y(event);
	if (down_time - up_time > CLICK_INTERVAL) {
		num_clicks = 0;
		/* Commit any previous cursor move. */
		cursor_moved = False;
	}
	if (num_clicks == 0)
		unselect(0, ROWS*COLS);
}

/*
 * Alternate form of select_start, which combines cursor motion with selection.
 * Usually bound to <Btn1Down> in a user-specified keymap.
 */
/*ARGSUSED*/
void
move_select_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int x, y;
	register int baddr;

	if (w != *screen)
		return;
	action_debug(move_select_action, event, params, num_params);
	BOUNDED_XY(event, x, y);
	baddr = ROWCOL_TO_BA(y, x);

	f_start = f_end = v_start = v_end = baddr;
	down1_time = down_time = event_time(event);
	down1_x = event_x(event);
	down1_y = event_y(event);

	if (down_time - up_time > CLICK_INTERVAL) {
		num_clicks = 0;
		/* Commit any previous cursor move. */
		cursor_moved = False;
	}
	if (num_clicks == 0) {
		if (any_selected) {
			unselect(0, ROWS*COLS);
		} else {
			cursor_moved = True;
			saved_cursor_addr = cursor_addr;
			cursor_move(baddr);
		}
	}
}

/*
 * Begin extending the current selection.
 * Usually bound to <Btn3Down>.
 */
/*ARGSUSED*/
void
start_extend_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int x, y;
	int baddr;

	if (w != *screen)
		return;
	action_debug(start_extend_action, event, params, num_params);

	down1_time = 0L;

	BOUNDED_XY(event, x, y);
	baddr = ROWCOL_TO_BA(y, x);

	if (baddr < f_start)
		v_start = baddr;		/* extend above */
	else if (baddr > f_end)
		v_end = baddr;			/* extend below */
	else if (baddr - f_start > f_end - baddr)
		v_end = baddr;			/* shrink end */
	else
		v_start = baddr;		/* shrink start */

	grab_sel(v_start, v_end, True, event_time(event));
	saw_motion = 1;
	num_clicks = 0;
}

/*
 * Continuously extend the current selection.
 * Usually bound to <Btn1Motion> and <Btn3Motion>.
 */
/*ARGSUSED*/
void
select_extend_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int x, y;
	int baddr;

	if (w != *screen)
		return;
	action_debug(select_extend_action, event, params, num_params);

	/* Ignore initial drag events if are too near. */
	if (down1_time != 0L &&
	    abs((int) event_x(event) - (int) down1_x) < CHAR_WIDTH &&
	    abs((int) event_y(event) - (int) down1_y) < CHAR_HEIGHT)
		return;
	else
		down1_time = 0L;

	/* If we moved the 3270 cursor on the first click, put it back. */
	if (cursor_moved) {
		cursor_move(saved_cursor_addr);
		cursor_moved = False;
	}

	BOUNDED_XY(event, x, y);
	baddr = ROWCOL_TO_BA(y, x);

	/*
	 * If baddr falls outside if the v range, open up the v range.  In
	 * addition, if we are extending one end of the v range, make sure the
	 * other end at least covers the f range.
	 */
	if (baddr <= v_start) {
		v_start = baddr;
		v_end = f_end;
	}
	if (baddr >= v_end) {
		v_end = baddr;
		v_start = f_start;
	}

	/*
	 * If baddr falls within the v range, narrow up the nearer end of the
	 * v range.
	 */
	if (baddr > v_start && baddr < v_end) {
		if (baddr - v_start < v_end - baddr)
			v_start = baddr;
		else
			v_end = baddr;
	}

	num_clicks = 0;
	saw_motion = 1;
	grab_sel(v_start, v_end, False, event_time(event));
}

/*
 * End the selection.
 * Usually bound to <BtnUp>.
 */
/*ARGSUSED*/
void
select_end_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int i;
	int x, y;

	action_debug(select_end_action, event, params, num_params);

	if (n_owned == -1) {
		for (i = 0; i < NS; i++)
			own_sel[i].atom = None;
		n_owned = 0;
	}
	for (i = 0; i < NS; i++)
		if (i < *num_params)
			want_sel[i] = XInternAtom(display, params[i], False);
		else
			want_sel[i] = None;
	if (*num_params == 0)
		want_sel[0] = XA_PRIMARY;

	BOUNDED_XY(event, x, y);
	up_time = event_time(event);

	if (up_time - down_time > CLICK_INTERVAL)
		num_clicks = 0;

	if (++num_clicks > 3)
		num_clicks = 1;

	switch (num_clicks) {
	    case 1:
		if (saw_motion) {
			f_start = v_start;
			f_end = v_end;
			grab_sel(f_start, f_end, True, event_time(event));
		}
		break;
	    case 2:
		/*
		 * If we moved the 3270 cursor on the first click, put it back.
		 */
		if (cursor_moved) {
			cursor_move(saved_cursor_addr);
			cursor_moved = False;
		}
		select_word(f_start, event_time(event));
		break;
	    case 3:
		select_line(f_start, event_time(event));
		break;
	}
	saw_motion = 0;
}

/*
 * Set the selection.
 * Usually bound to the Copy key.
 */
/*ARGSUSED*/
void
set_select_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int i;

	action_debug(set_select_action, event, params, num_params);

	if (!any_selected)
		return;
	if (n_owned == -1) {
		for (i = 0; i < NS; i++)
			own_sel[i].atom = None;
		n_owned = 0;
	}
	for (i = 0; i < NS; i++)
		if (i < *num_params)
			want_sel[i] = XInternAtom(display, params[i], False);
		else
			want_sel[i] = None;
	if (*num_params == 0)
		want_sel[0] = XA_PRIMARY;
	own_sels(event_time(event));
}

/*
 * MoveCursor action.  Not a selection operation, but it uses the same macros
 * to map the mouse position onto a cursor position.
 * Usually bound to Shift<Btn1Down>.
 */
/*ARGSUSED*/
void
MoveCursor_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int x, y;
	register int baddr;
	int row, col;

	action_debug(MoveCursor_action, event, params, num_params);

	if (kybdlock) {
		if (*num_params == 2)
			enq_ta(MoveCursor_action, params[0], params[1]);
		return;
	}

	switch (*num_params) {
	    case 0:		/* mouse click, presumably */
		if (w != *screen)
			return;
		BOUNDED_XY(event, x, y);
		baddr = ROWCOL_TO_BA(y, x);
		cursor_move(baddr);
		break;
	    case 2:		/* probably a macro call */
		row = atoi(params[0]);
		col = atoi(params[1]);
		if (!IN_3270) {
			row--;
			col--;
		}
		if (row < 0)
			row = 0;
		if (col < 0)
			col = 0;
		baddr = ((row * COLS) + col) % (ROWS * COLS);
		cursor_move(baddr);
		break;
	    default:		/* couln't say */
		popup_an_error("%s: illegal argument count",
		    action_name(MoveCursor_action));
		break;
	}
}

/*
 * Cut action.
 * For now, merely erases all unprotected characters currently selected.
 * In future, it may interact more with selections.
 */
#define ULS	sizeof(unsigned long)
#define ULBS	(ULS * 8)

/*ARGSUSED*/
void
Cut_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	register int baddr;
	unsigned char fa = *get_field_attribute(0);
	unsigned long *target;
	register unsigned char repl;

	action_debug(Cut_action, event, params, num_params);

	target = (unsigned long *)XtCalloc(ULS, ((ROWS*COLS)+(ULBS-1))/ULBS);

	/* Identify the positions to empty. */
	for (baddr = 0; baddr < ROWS*COLS; baddr++) {
		unsigned char c = screen_buf[baddr];

		if (IS_FA(c))
			fa = c;
		else if ((IN_ANSI || !FA_IS_PROTECTED(fa)) && SELECTED(baddr))
			target[baddr/ULBS] |= 1 << (baddr%ULBS);
	}

	/* Erase them. */
	if (IN_3270)
		repl = CG_null;
	else
		repl = CG_space;
	for (baddr = 0; baddr < ROWS*COLS; baddr++)
		if (target[baddr/ULBS] & (1 << (baddr%ULBS)))
			ctlr_add(baddr, repl, 0);

	XtFree((XtPointer)target);
}


/*
 * Screen side.
 */

static char    *select_buf = CN;
static char    *sb_ptr = CN;
static int      sb_size = 0;
#define SB_CHUNK	1024

static Time     sel_time;

static void
init_select_buf()
{
	if (select_buf == CN)
		select_buf = XtMalloc(sb_size = SB_CHUNK);
	sb_ptr = select_buf;
}

static void
store_sel(c)
char c;
{
	if (sb_ptr - select_buf >= sb_size) {
		sb_size += SB_CHUNK;
		select_buf = XtRealloc(select_buf, sb_size);
		sb_ptr = select_buf + sb_size - SB_CHUNK;
	}
	*(sb_ptr++) = c;
}

static Boolean
convert_sel(w, selection, target, type, value, length, format)
Widget w;
Atom *selection, *target, *type;
XtPointer *value;
unsigned long *length;
int *format;
{
	int i;

	/* Find the right selection. */
	for (i = 0; i < NS; i++)
		if (own_sel[i].atom == *selection)
			break;
	if (i >= NS)	/* not my selection */
		return False;

	if (*target == XA_TARGETS(display)) {
		Atom* targetP;
		Atom* std_targets;
		unsigned long std_length;

		XmuConvertStandardSelection(w, sel_time, selection,
		    target, type, (caddr_t*) &std_targets, &std_length, format);
		*length = std_length + 5;
		*value = (XtPointer) XtMalloc(sizeof(Atom) * (*length));
		targetP = *(Atom**)value;
		*targetP++ = XA_STRING;
		*targetP++ = XA_TEXT(display);
		*targetP++ = XA_COMPOUND_TEXT(display);
		*targetP++ = XA_LENGTH(display);
		*targetP++ = XA_LIST_LENGTH(display);
		(void) MEMORY_MOVE((char *) targetP,
				   (char *) std_targets,
				   (int) (sizeof(Atom) * std_length));
		XtFree((char *) std_targets);
		*type = XA_ATOM;
		*format = 32;
		return True;
	}

	if (*target == XA_STRING ||
	    *target == XA_TEXT(display) ||
	    *target == XA_COMPOUND_TEXT(display)) {
		if (*target == XA_COMPOUND_TEXT(display))
			*type = *target;
		else
			*type = XA_STRING;
		*length = strlen(own_sel[i].buffer);
		*value = XtMalloc(*length);
		(void) MEMORY_MOVE(*value, own_sel[i].buffer, (int) *length);
		*format = 8;
		return True;
	}
	if (*target == XA_LIST_LENGTH(display)) {
		*value = XtMalloc(4);
		if (sizeof(long) == 4)
			*(long *)*value = 1;
		else {
			long temp = 1;
			(void) MEMORY_MOVE((char *) *value,
			                   ((char*) &temp) + sizeof(long) - 4,
					   4);
		}
		*type = XA_INTEGER;
		*length = 1;
		*format = 32;
		return True;
	}
	if (*target == XA_LENGTH(display)) {
		*value = XtMalloc(4);
		if (sizeof(long) == 4)
			*(long*)*value = strlen(own_sel[i].buffer);
		else {
			long temp = strlen(own_sel[i].buffer);
			(void) MEMORY_MOVE((char *) *value,
			                   ((char *) &temp) + sizeof(long) - 4,
					   4);
		}
		*type = XA_INTEGER;
		*length = 1;
		*format = 32;
		return True;
	}

	if (XmuConvertStandardSelection(w, sel_time, selection,
	    target, type, (caddr_t *)value, length, format))
		return True;

	/* else */
#if 0
	printf("Unknown conversion request: %s to %s\n",
	    XGetAtomName(display, *selection),
	    XGetAtomName(display, *target));
#endif
	return False;
}

/*ARGSUSED*/
static void
lose_sel(w, selection)
Widget w;
Atom *selection;
{
	int i;

	for (i = 0; i < NS; i++)
		if (own_sel[i].atom != None && own_sel[i].atom == *selection) {
			own_sel[i].atom = None;
			XtFree(own_sel[i].buffer);
			own_sel[i].buffer = CN;
			n_owned--;
			break;
		}
	if (!n_owned)
		unselect(0, ROWS*COLS);
}

/*
 * Somewhat convoluted logic to return an ASCII character for a given screen
 * position.
 *
 * The character has to be found indirectly from screen_buf and the field
 * attirbutes, so that zero-intensity fields become blanks.
 */
static Boolean osc_valid = False;

static void
osc_start()
{
	osc_valid = False;
}

static unsigned char
onscreen_char(baddr, ge)
int baddr;
Boolean *ge;
{
	static int osc_baddr;
	static unsigned char fa;

	/* If we aren't moving forward, all bets are off. */
	if (osc_valid && baddr < osc_baddr)
		osc_valid = False;

	if (osc_valid) {
		/*
		 * Search for a new field attribute between the address we
		 * want and the last address we searched.  If we found a new
		 * field attribute, save the address for next time.
		 */
		(void) get_bounded_field_attribute(baddr, osc_baddr, &fa);
		osc_baddr = baddr;
	} else {
		/*
		 * Find the attribute the old way.
		 */
		fa = *get_field_attribute(baddr);
		osc_baddr = baddr;
		osc_valid = True;
	}

	*ge = False;
	if (FA_IS_ZERO(fa))
		return ' ';
	else {
		switch (ea_buf[baddr].cs) {
		    case 0:
		    default:
			return cg2asc[screen_buf[baddr]];
		    case 1:
			if (screen_buf[baddr] == CG_Yacute)
				return '[';
			else if (screen_buf[baddr] == CG_diaeresis)
				return ']';
			*ge = True;
			return cg2asc[screen_buf[baddr]];
		    case 2:
			return screen_buf[baddr] + 0x5f;
		}
	}
}

/*
 * Attempt to own the selections in want_sel[].
 */
static void
own_sels(t)
Time t;
{
	register int i, j;

	/*
	 * Try to grab any new selections we may want.
	 */
	for (i = 0; i < NS; i++) {
		Boolean already_own = False;

		if (want_sel[i] == None)
			continue;

		/* Check if we already own it. */
		for (j = 0; j < NS; j++)
			if (own_sel[j].atom == want_sel[i]) {
				already_own = True;
				break;
			}

		/* Find the slot for it. */
		if (!already_own) {
			for (j = 0; j < NS; j++)
				if (own_sel[j].atom == None)
					break;
			if (j >= NS)
				continue;
		}

		if (XtOwnSelection(*screen, want_sel[i], t, convert_sel,
		    lose_sel, NULL)) {
			if (!already_own) {
				n_owned++;
				own_sel[j].atom = want_sel[i];
			}
			if (own_sel[j].buffer != CN)
				XtFree(own_sel[j].buffer);
			own_sel[j].buffer = XtMalloc(strlen(select_buf) + 1);
			MEMORY_MOVE(own_sel[j].buffer, select_buf,
			    strlen(select_buf) + 1);
		} else {
			XtWarning("Could not get selection");
			if (own_sel[j].atom != None) {
				XtFree(own_sel[j].buffer);
				own_sel[j].buffer = CN;
				own_sel[j].atom = None;
				n_owned--;
			}
		}
	}
	if (!n_owned)
		unselect(0, ROWS*COLS);
	sel_time = t;
}

/*
 * Copy the selected area on the screen into a buffer and attempt to
 * own the selections in want_sel[].
 */
static void
grab_sel(start, end, really, t)
int start, end;
Boolean really;
Time t;
{
	register int i;
	int start_row, end_row;
	unsigned char osc;
	Boolean ge;

	unselect(0, ROWS*COLS);

	if (start > end) {
		int exch = end;

		end = start;
		start = exch;
	}

	start_row = start / COLS;
	end_row = end / COLS;

	init_select_buf();	/* prime the store_sel() routine */
	osc_start();		/* prime the onscreen_char() routine */

	if (!ever_3270 && !toggled(RECTANGLE_SELECT)) {
		/* Continuous selections */
		for (i = start; i <= end; i++) {
			SET_SELECT(i);
			if (really) {
				if (i != start && !(i % COLS))
					store_sel('\n');
				osc = onscreen_char(i, &ge);
				if (ge)
					store_sel('\033');
				store_sel((char)osc);
			}
		}
		/* Check for newline extension */
		if ((end % COLS) != (COLS - 1)) {
			Boolean all_blank = True;

			for (i = end; i < end + (COLS - (end % COLS)); i++)
				if (onscreen_char(i, &ge) != ' ') {
					all_blank = False;
					break;
				}
			if (all_blank) {
				for (i = end; i < end + (COLS - (end % COLS)); i++)
					SET_SELECT(i);
				store_sel('\n');
			}
		}
	} else {
		/* Rectangular selections */
		if (start_row == end_row) {
			for (i = start; i <= end; i++) {
				SET_SELECT(i);
				if (really) {
					osc = onscreen_char(i, &ge);
					if (ge)
						store_sel('\033');
					store_sel((char)osc);
				}
			}
			if (really && (end % COLS == COLS - 1))
				store_sel('\n');
		} else {
			int row, col;
			int start_col = start % COLS;
			int end_col = end % COLS;

			if (start_col > end_col) {
				int exch = end_col;

				end_col = start_col;
				start_col = exch;
			}

			for (row = start_row; row <= end_row; row++) {
				for (col = start_col; col <= end_col; col++) {
					SET_SELECT(row*COLS + col);
					if (really) {
						osc = onscreen_char(row*COLS + col, &ge);
						if (ge)
							store_sel('\033');
						store_sel((char)osc);
					}
				}
				if (really)
					store_sel('\n');
			}
		}
	}

	/* Trim trailing blanks on each line */
	if (really) {
		int k, l;
		char c;
		int nb = 0;
		int iy = 0;

		store_sel('\0');
		for (k = 0; (c = select_buf[k]); k++) {
			if (c == ' ')
				nb++;
			else {
				if (c != '\n') {
					for (l = 0; l < nb; l++)
						select_buf[iy++] = ' ';
				}
				select_buf[iy++] = c;
				nb = 0;
			}
		}
		select_buf[iy] = '\0';
	}
	any_selected = True;
	ctlr_changed(0, ROWS*COLS);
	if (really)
		own_sels(t);
}

/*
 * Check if any character in a given region is selected.
 */
Boolean
area_is_selected(baddr, len)
int baddr, len;
{
	register int i;

	for (i = 0; i < len; i++)
		if (SELECTED(baddr+i))
			return True;
	return False;
}

/*
 * Unhighlight the region of selected text -- but don't give up the selection.
 */
/*ARGSUSED*/
void
unselect(baddr, len)
int baddr;
int len;
{
	if (any_selected) {
		(void) memset((char *) selected, 0, (ROWS*COLS + 7) / 8);
		ctlr_changed(0, ROWS*COLS);
		any_selected = False;
	}
}
