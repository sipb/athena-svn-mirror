#ifndef lint
static char sccsid[] = "@(#)cartoon.c  3.11h 96/09/30 xlockmore";

#endif

/*-
 * cartoon.c - A bouncing cartoon for xlock, the X Window System lockscreen.
 *
 * Copyright (c) 1988 by Sun Microsystems
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 *
 * 14-Oct-96:  (dhansen@metapath.com)
 *             Updated the equations for mapping the charater
 *             to follow a parabolic time curve.  Also adjust
 *             the rate based on the size (area) of the cartoon.
 *             Eliminated unused variables carried over from bounce.c
 *             Still needs a user attribute to control the rate
 *             depending on their computer and preference.
 *
 * 20-Sep-94: I done this file starting from the previous bounce.c
 *       I draw the 8 cartoons like they are.
 *       (patol@info.isbiel.ch)
 *        David monkeyed around with the cmap stuff to get it to work.
 *
 * 2-Sep-93: xlock version (David Bagley bagleyd@source.asset.com)
 * 1986: Sun Microsystems
 */


/*-
 * original copyright
 * **************************************************************************
 * Copyright 1988 by Sun Microsystems, Inc. Mountain View, CA.
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Sun or MIT not be used in advertising
 * or publicity pertaining to distribution of the software without specific
 * prior written permission. Sun and M.I.T. make no representations about the
 * suitability of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL SUN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ***************************************************************************
 */

#if defined( HAS_XPM ) || defined( HAS_XPMINC )

#include "xlock.h"
#include <math.h>
#if HAS_XPMINC
#include <xpm.h>
#else
#include <X11/xpm.h>		/* Normal spot */
#endif

#include "pixmaps/calvin.xpm"
#include "pixmaps/calvin2.xpm"
#include "pixmaps/calvin3.xpm"
#include "pixmaps/gravity.xpm"
#include "pixmaps/dragon.xpm"
#include "pixmaps/marino2.xpm"
#include "pixmaps/calvin4.xpm"
#include "pixmaps/calvinf.xpm"
#include "pixmaps/hobbes.xpm"
#include "pixmaps/cal_hob.xpm"


#define NUM_CARTOONS    10
#define MAX_X_STRENGTH  8
#define MAX_Y_STRENGTH  3


ModeSpecOpt cartoon_opts =
{0, NULL, 0, NULL, NULL};

static char **cartoonlist[NUM_CARTOONS] =
{
	calvin, calvin2, calvin3, gravity, dragon,
	marino2, calvin4, calvinf, hobbes, cal_hob
};

typedef struct {
	int         x, y, xlast, ylast;
	int         vx, vy;
	int         ncartoons;
	int         ytime, yrate, ybase;
	int         width, height;
	int         cwidth, cheight;
	GC          draw_GC, erase_GC;
	XImage     *image[NUM_CARTOONS];
	int         choice;
} cartoonstruct;

static cartoonstruct *cartoons = NULL;

static void erase_image(Display * display, Window win, GC gc,
			int x, int y, int xl, int yl, int xsize, int ysize);

static int  tbx = 0;
static int  tby = 0;

static void
put_cartoon(ModeInfo * mi)
{
	cartoonstruct *cp = &cartoons[MI_SCREEN(mi)];

	XPutImage(MI_DISPLAY(mi), MI_WINDOW(mi), cp->draw_GC,
		  cp->image[cp->choice], 0, 0,
		  cp->x - tbx, cp->y - tby,
		  cp->cwidth, cp->cheight);
	if (cp->xlast != -1) {
		erase_image(MI_DISPLAY(mi), MI_WINDOW(mi), cp->erase_GC,
			    cp->x - tbx, cp->y - tby,
			    cp->xlast, cp->ylast,
			    cp->cwidth, cp->cheight);
	}
}				/* put_cartoon */

static void
select_cartoon(ModeInfo * mi)
{
	cartoonstruct *cp = &cartoons[MI_SCREEN(mi)];
	int         old;

	old = cp->choice;
	if (cp->ncartoons >= 2) {
		while (old == cp->choice)
			cp->choice = NRAND(cp->ncartoons);
	}
	if (MI_WIN_IS_VERBOSE(mi))
		(void) printf("cartoon %d.\n", cp->choice);

	cp->cwidth = cp->image[cp->choice]->width;
	cp->cheight = cp->image[cp->choice]->height;
}				/* select_cartoon */

static void
move_cartoon(ModeInfo * mi)
{
	cartoonstruct *cp = &cartoons[MI_SCREEN(mi)];
	int         j;

	cp->xlast = cp->x - tbx;
	cp->ylast = cp->y - tby;

	cp->x += cp->vx;
	if (cp->x > (cp->width - cp->cwidth)) {
		/* Bounce off the right edge */
		cp->x = 2 * (cp->width - cp->cwidth) - cp->x;
		cp->vx = -cp->vx;
	} else if (cp->x < 0) {
		/* Bounce off the left edge */
		cp->x = -cp->x;
		cp->vx = -cp->vx;
	}
	cp->ytime++;

	cp->y += ((cp->ytime / cp->yrate) - cp->ybase);


	if (cp->y >= (cp->height + cp->cheight)) {
		/* Back at the bottom edge, time for a new cartoon */
		select_cartoon(mi);

		/* reset the direction */
		cp->vx = ((LRAND() & 1) ? -1 : 1) * NRAND(MAX_X_STRENGTH);

		cp->y = (cp->height + cp->cheight);
		cp->ytime = 0;
		j = 100000 / (cp->cwidth * cp->cheight);
		cp->yrate = (1 + NRAND(MAX_Y_STRENGTH)) * (j * j) / 4;
		cp->ybase = sqrt(((3 * cp->y / 4) + (NRAND(cp->y / 4))) *
				 2 / cp->yrate);
	}
}				/* move_cartoon */

/* This stops some flashing, could be more efficient */
static void
erase_image(Display * display, Window win, GC gc,
	    int x, int y, int xl, int yl, int xsize, int ysize)
{
	if (y < 0)
		y = 0;
	if (x < 0)
		x = 0;
	if (xl < 0)
		xl = 0;
	if (yl < 0)
		yl = 0;

	if (xl - x > 0)
		XFillRectangle(display, win, gc, x + xsize, yl, xl - x, ysize);
	if (yl - y > 0)
		XFillRectangle(display, win, gc, xl, y + ysize, xsize, yl - y);
	if (y - yl > 0)
		XFillRectangle(display, win, gc, xl, yl, xsize, y - yl);
	if (x - xl > 0)
		XFillRectangle(display, win, gc, xl, yl, x - xl, ysize);
}				/* erase_image */

void
init_cartoon(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	cartoonstruct *cp;
	XGCValues   gcv;
	int         i;

	if (cartoons == NULL) {
		if ((cartoons = (cartoonstruct *) calloc(MI_NUM_SCREENS(mi),
					    sizeof (cartoonstruct))) == NULL)
			return;
	}
	cp = &cartoons[MI_SCREEN(mi)];

	cp->ncartoons = 0;
	if (MI_NPIXELS(mi) > 2) {
			XpmAttributes attrib;
			Display    *display = MI_DISPLAY(mi);
			int         screen = MI_SCREEN(mi);

			attrib.visual = DefaultVisual(display, screen);
			attrib.colormap = DefaultColormap(display, screen);
			attrib.depth = DefaultDepth(display, screen);
			attrib.valuemask = XpmVisual | XpmColormap | XpmDepth;

		for (i = 0; i < NUM_CARTOONS; i++) {
			if (XpmSuccess != XpmCreateImageFromData(display,
								 cartoonlist[i], &(cp->image[i]), (XImage **) NULL, &attrib))
				break;
			cp->ncartoons++;
		}
	}
	if (MI_WIN_IS_VERBOSE(mi))
		(void) printf("%d of %d cartoons loaded.\n", cp->ncartoons, NUM_CARTOONS);
	if (cp->ncartoons > 0)
	  select_cartoon(mi);

	cp->width = MI_WIN_WIDTH(mi);
	cp->height = MI_WIN_HEIGHT(mi);

	gcv.background = MI_WIN_BLACK_PIXEL(mi);
	cp->draw_GC = XCreateGC(display, window, GCForeground | GCBackground, &gcv);
	cp->erase_GC = XCreateGC(display, window, GCForeground | GCBackground, &gcv);

	/* set up intial movement */
  if (cp->cwidth < 1)
    cp->cwidth = 1;
  if (cp->cheight < 1)
    cp->cheight = 1;

	cp->vx = ((LRAND() & 1) ? -1 : 1) * NRAND(MAX_X_STRENGTH);

	cp->x = (cp->vx >= 0) ? 0 : cp->width - cp->cwidth;

	cp->y = cp->height + cp->cheight;
	cp->ytime = 0;
	i = 100000 / (cp->cwidth * cp->cheight);
	cp->yrate = (1 + NRAND(MAX_Y_STRENGTH)) * (i * i) / 4;
	cp->ybase = sqrt(((3 * cp->y / 4) +
			  (NRAND(cp->y / 4))) * 2 / cp->yrate);

	cp->xlast = -1;
	cp->ylast = 0;

	XClearWindow(display, window);
	XSetForeground(display, cp->erase_GC, MI_WIN_BLACK_PIXEL(mi));
}				/* init_cartoon */

void
draw_cartoon(ModeInfo * mi)
{
	cartoonstruct *cp = &cartoons[MI_SCREEN(mi)];

	if (cp->ncartoons > 0) {
		put_cartoon(mi);
		move_cartoon(mi);
	}
	/* else, still want to lock screen, right? */
}				/* draw_cartoon */

void
release_cartoon(ModeInfo * mi)
{
	if (cartoons != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			Display    *display = MI_DISPLAY(mi);
			cartoonstruct *cp = &cartoons[screen];
			int         i;

			if (cp->draw_GC != NULL)
				XFreeGC(display, cp->draw_GC);
			if (cp->erase_GC != NULL)
				XFreeGC(display, cp->erase_GC);
			for (i = 0; i < cp->ncartoons; i++)
				XDestroyImage(cp->image[i]);
		}
		(void) free((void *) cartoons);
		cartoons = NULL;
	}
}				/* release_cartoon */

#endif
