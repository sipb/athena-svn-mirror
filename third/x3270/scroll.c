/*
 * Copyright 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	scroll.c
 *		Scrollbar support
 */

#include "globals.h"
#include "appres.h"
#include "ctlr.h"

#include "ctlrc.h"
#include "kybdc.h"
#include "screenc.h"
#include "scrollc.h"
#include "selectc.h"
#include "statusc.h"

/* Globals */
Boolean	scroll_initted = False;

/* Statics */
static unsigned char **buf_save = (unsigned char **) NULL;
static struct ea **ea_save = (struct ea **) NULL;
static int      n_saved = 0;
static int      scroll_next = 0;
static float    thumb_top = 0.0;
static float    thumb_top_base = 0.0;
static float    thumb_shown = 1.0;
static int      scrolled_back = 0;
static Boolean  need_saving = True;
static Boolean  vscreen_swapped = False;
static char    *sbuf = CN;
static int      sa_bufsize;
static char    *zbuf = CN;
static void     sync_scroll();

/*
 * Initialize (or re-initialize) the scrolling parameters and save area.
 */
void
scroll_init()
{
	register int i;
	int sa_size;
	unsigned char *s;

	if (appres.save_lines % maxROWS)
		appres.save_lines =
		    ((appres.save_lines+maxROWS-1)/maxROWS) * maxROWS;
	if (!appres.save_lines)
		appres.save_lines = maxROWS;
	if (sbuf != CN) {
		XtFree(sbuf);
		XtFree(zbuf);
		XtFree((char *)buf_save);
		XtFree((char *)ea_save);
	}
	sa_size = appres.save_lines + maxROWS;
	buf_save = (unsigned char **)XtCalloc(sizeof(unsigned char *), sa_size);
	ea_save = (struct ea **)XtCalloc(sizeof(struct ea *), sa_size);
	sa_bufsize = (sa_size * (sizeof(unsigned char) + sizeof(struct ea))) * maxCOLS;
	sbuf = XtMalloc(sa_bufsize);
	zbuf = XtMalloc(maxCOLS);
	(void) memset(zbuf, '\0', maxCOLS);
	s = (unsigned char *)sbuf;
	for (i = 0; i < sa_size; s += maxCOLS, i++)
		buf_save[i] = s;
	for (i = 0; i < sa_size; s += (maxCOLS * sizeof(struct ea)), i++)
		ea_save[i] = (struct ea *)s;
	scroll_reset();
	scroll_initted = True;
}

/*
 * Reset the scrolling parameters and erase the save area.
 */
void
scroll_reset()
{
	(void) memset(sbuf, 0, sa_bufsize);
	scroll_next = 0;
	n_saved = 0;
	scrolled_back = 0;
	thumb_top_base = thumb_top = 0.0;
	thumb_shown = 1.0;
	need_saving = True;
	screen_set_thumb(thumb_top, thumb_shown);
	enable_cursor(True);
}

/*
 * Save <n> lines of data from the top of the screen.
 */
void
scroll_save(n, trim_blanks)
int n;
Boolean trim_blanks;
{
	int i;

	/* Trim trailing blank lines from 'n', if requested */
	if (trim_blanks) {
		while (n) {
			if (memcmp((char *)(screen_buf+((n-1)*COLS)),
				   zbuf, COLS))
				break;
			else
				n--;
		}
		if (!n)
			return;
	}

	/* Scroll to bottom on "output". */
	if (scrolled_back)
		sync_scroll(0);

	/* Save the screen contents. */
	for (i = 0; i < n; i++) {
		if (i < COLS) {
			MEMORY_MOVE((char *)buf_save[scroll_next],
			    (char *)screen_buf+(i*COLS),
			    COLS);
			MEMORY_MOVE((char *)ea_save[scroll_next],
			    (char *)(ea_buf+(i*COLS)),
			    COLS*sizeof(struct ea));
			if (COLS < maxCOLS) {
				(void) memset((char *)(buf_save[scroll_next] + COLS), '\0',
				    maxCOLS - COLS);
				(void) memset((char *)(ea_save[scroll_next] + COLS), '\0',
				    (maxCOLS - COLS)*sizeof(struct ea));
			}
		} else {
			(void) memset((char *)buf_save[scroll_next], '\0',
			    maxCOLS);
			(void) memset((char *)ea_save[scroll_next], '\0',
			    maxCOLS*sizeof(struct ea));
		}
		scroll_next = (scroll_next + 1) % appres.save_lines;
		if (n_saved < appres.save_lines)
			n_saved++;
	}

	/* Reset the thumb. */
	thumb_top_base =
	    thumb_top =
	    ((float)n_saved / (float)(appres.save_lines + maxROWS));
	thumb_shown = 1.0 - thumb_top;
	screen_set_thumb(thumb_top, thumb_shown);
}

/*
 * Add blank lines to the scroll buffer to make it a multiple of the
 * screen size.
 */
void
scroll_round()
{
	int n;

	if (!(n_saved % maxROWS))
		return;

	/* Zero the scroll buffer. */
	for (n = maxROWS - (n_saved % maxROWS); n; n--) {
		(void) memset((char *)buf_save[scroll_next], '\0', maxCOLS);
		(void) memset((char *)ea_save[scroll_next], '\0',
			    maxCOLS*sizeof(struct ea));
		scroll_next = (scroll_next + 1) % appres.save_lines;
		if (n_saved < appres.save_lines)
			n_saved++;
	}

	/* Reset the thumb. */
	thumb_top_base =
	    thumb_top =
	    ((float)n_saved / (float)(appres.save_lines + maxROWS));
	thumb_shown = 1.0 - thumb_top;
	screen_set_thumb(thumb_top, thumb_shown);
}

/*
 * Jump to the bottom of the scroll buffer.
 */
void
scroll_to_bottom()
{
	if (scrolled_back) {
		sync_scroll(0);
		/* screen_set_thumb(thumb_top, thumb_shown); */
	}
	need_saving = True;
}

/*
 * Save the current screen image, if it hasn't been saved since last updated.
 */
void
save_image()
{
	int i;

	if (!need_saving)
		return;
	for (i = 0; i < maxROWS; i++) {
		MEMORY_MOVE((char *)buf_save[appres.save_lines+i],
		            (char *)screen_buf + (i*COLS),
		            COLS);
		MEMORY_MOVE((char *)ea_save[appres.save_lines+i],
		            (char *)(ea_buf + (i*COLS)),
		            COLS*sizeof(struct ea));
	}
	need_saving = False;
}

/*
 * Redraw the display so it begins back <sb> lines.
 */
static void
sync_scroll(sb)
int sb;
{
	int slop;
	int i;
	int scroll_first;
	float tt0;

	(void) unselect(0, ROWS*COLS);

	/*
	 * If in 3270 mode, round to a multiple of the screen size and
	 * set the scroll bar.
	 */
	if (ever_3270) {
		if ((slop = (sb % maxROWS))) {
			if (slop <= maxROWS/2)
				sb -= slop;
			else
				sb += maxROWS - slop;
		}
		if (sb)
			kybd_scroll_lock(True);
		else
			kybd_scroll_lock(False);
	}

	/* Update the status line. */
	if (ever_3270)
		status_scrolled(sb / maxROWS);
	else
		status_scrolled(0);

	/* Swap screen sizes. */
	if (sb && !scrolled_back && ((COLS < maxCOLS) || (ROWS < maxROWS))) {
		COLS = maxCOLS;
		ROWS = maxROWS;
		vscreen_swapped = True;
	} else if (!sb && scrolled_back && vscreen_swapped) {
		ctlr_shrink();
		COLS = 80;
		ROWS = 24;
		vscreen_swapped = False;
	}

	scroll_first = (scroll_next+appres.save_lines-sb) % appres.save_lines;

	/* Update the screen. */
	for (i = 0; i < maxROWS; i++)
		if (i < sb) {
			MEMORY_MOVE((char *)screen_buf + (i*COLS),
				    (char *)buf_save[(scroll_first+i) % appres.save_lines],
				    COLS);
			MEMORY_MOVE((char *)(ea_buf + (i*COLS)),
				    (char *)ea_save[(scroll_first+i) % appres.save_lines],
				    COLS*sizeof(struct ea));
		} else {
			MEMORY_MOVE((char *)screen_buf + (i*COLS),
				    (char *)buf_save[appres.save_lines+i-sb],
				    COLS);
			MEMORY_MOVE((char *)(ea_buf + (i*COLS)),
				    (char *)ea_save[appres.save_lines+i-sb],
				    COLS*sizeof(struct ea));
		}

	/* Disable the cursor if we're scrolled back, enable it if not. */
	enable_cursor(sb == 0);

	scrolled_back = sb;
	ctlr_changed(0, ROWS*COLS);
	blink_start();

	tt0 = ((float)n_saved / (float)(appres.save_lines + maxROWS));
	thumb_shown = 1.0 - tt0;
	thumb_top = ((float)(n_saved-sb) / (float)(appres.save_lines + maxROWS));
	screen_set_thumb(thumb_top, thumb_shown);
}

/*
 * Callback for "scroll" action (incrementing the thumb in one direction).
 */
void
scroll_proc(n, total)
int n;
int total;
{
	float pct;
	int nss;
	int nsr;

	if (!n_saved)
		return;
	if (n < 0)
		pct = (float)(-n) / (float)total;
	else
		pct = (float)n / (float)total;
	/*printf("scroll_proc(%d, %d) %f\n", n, total, pct);*/
	nss = pct * thumb_shown * n_saved;
	if (!nss)
		nss = 1;
	save_image();
	if (n > 0) {	/* scroll forward */
		if (nss > scrolled_back)
			sync_scroll(0);
		else {
			nsr = scrolled_back - nss;
			if (ever_3270 && (nsr % maxROWS))
				nsr -= nsr % maxROWS;
			sync_scroll(nsr);
		}
	} else {	/* scroll back */
		if (scrolled_back + nss > n_saved)
			sync_scroll(n_saved);
		else {
			nsr = scrolled_back + nss;
			if (ever_3270 && (nsr % maxROWS))
				nsr += maxROWS - (nsr % maxROWS);
			sync_scroll(nsr);
		}
	}

	screen_set_thumb((float)(n_saved - scrolled_back) / (float)(appres.save_lines + maxROWS),
	    thumb_shown);
}

/*
 * Callback for "jump" action (moving the thumb to a particular spot).
 */
void
jump_proc(top)
float top;
{
	/*printf("jump_proc(%f)\n", top);*/
	if (!n_saved) {
		screen_set_thumb(thumb_top, thumb_shown);
		return;
	}
	if (top > thumb_top_base) {	/* too far down */
		screen_set_thumb(thumb_top_base, thumb_shown);
		sync_scroll(0);
	} else {
		save_image();
		sync_scroll((int)((1.0 - top) * n_saved));
	}
}

/*
 * Resynchronize the thumb (called when the scrollbar is turned on).
 */
void
rethumb()
{
	screen_set_thumb(thumb_top, thumb_shown);
}
