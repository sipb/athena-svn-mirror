
#ifndef lint
static char sccsid[] = "@(#)julia.c	3.11h 96/09/30 xlockmore";

#endif

/*- 
 * julia.c - continuously varying Julia set for xlock
 * 
 * Copyright (c) 1995 Sean McCullough <bankshot@mailhost.nmt.edu>.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 2-Dec-95: snagged boilerplate from hop.c
 *           used ifs {w0 = sqrt(x-c), w1 = -sqrt(x-c)} with random iteration 
 *           to plot the julia set, and sinusoidially varied parameter for set 
 *	     and plotted parameter with a circle.
 */
/*-
 * One thing to note is that batchcount is the *depth* of the search tree,
 * so the number of points computed is 2^batchcount - 1.  I use 8 or 9
 * on a dx266 and it looks okay.  The sinusoidal variation of the parameter
 * might not be as interesting as it could, but it still gives an idea of
 * the effect of the parameter.
 */
#include "xlock.h"
#include <math.h>

#define numpoints ((0x2<<jp->depth)-1)
ModeSpecOpt julia_opts =
{0, NULL, 0, NULL, NULL};

typedef struct {
	int         centerx;
	int         centery;	/* center of the screen */
	double      cr;
	double      ci;		/* julia params */
	int         depth;
	int         inc;
	int         pix;
	long        itree;
} julstruct;

static julstruct *juls = NULL;
static XPoint *pointBuffer = 0;	/* pointer for XDrawPoints */

static void
apply(ModeInfo * mi, register double xr, register double xi, int d)
{
	julstruct  *jp = &juls[MI_SCREEN(mi)];
	double      theta, r;

	pointBuffer[jp->itree].x = (int) (0.5 * xr * jp->centerx + jp->centerx);
	pointBuffer[jp->itree].y = (int) (0.5 * xi * jp->centery + jp->centery);
	jp->itree++;

	if (d > 0) {
		xi -= jp->ci;
		xr -= jp->cr;

		theta = atan2(xi, xr) / 2.0;

		/*r = pow(xi * xi + xr * xr, 0.25); */
		r = sqrt(sqrt(xi * xi + xr * xr));	/* 3 times faster */

		xr = r * cos(theta);
		xi = r * sin(theta);

		d--;
		apply(mi, xr, xi, d);
		apply(mi, -xr, -xi, d);
	}
}

void
init_julia(ModeInfo * mi)
{
	julstruct  *jp;

	if (juls == NULL) {
		if ((juls = (julstruct *) calloc(MI_NUM_SCREENS(mi),
						 sizeof (julstruct))) == NULL)
			return;
	}
	jp = &juls[MI_SCREEN(mi)];

	jp->centerx = MI_WIN_WIDTH(mi) / 2;
	jp->centery = MI_WIN_HEIGHT(mi) / 2;

	jp->depth = MI_BATCHCOUNT(mi);
	if (jp->depth > 10)
		jp->depth = 10;

	jp->pix = 0;

	jp->cr = 1.5 * sin(M_PI * (jp->inc / 300.0)) * sin(jp->inc * M_PI / 200.0);
	jp->ci = 1.5 * cos(M_PI * (jp->inc / 300.0)) * cos(jp->inc * M_PI / 200.0);

	jp->cr += 0.5 * cos(M_PI * jp->inc / 400.0);
	jp->ci += 0.5 * sin(M_PI * jp->inc / 400.0);

	if (!pointBuffer)
		pointBuffer = (XPoint *) malloc(numpoints * sizeof (XPoint));
}

void
draw_julia(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	GC          gc = MI_GC(mi);
	julstruct  *jp = &juls[MI_SCREEN(mi)];
	double      r, theta;
	register double xr = 0.0, xi = 0.0;
	int         k = 64, rnd = 0;
	XPoint     *xp = pointBuffer;

	jp->inc++;
	if (MI_NPIXELS(mi) > 2) {
		XSetForeground(display, gc, MI_PIXEL(mi, jp->pix));
		if (++jp->pix >= MI_NPIXELS(mi))
			jp->pix = 0;
	}
	while (k--) {

		/* save calls to LRAND by using bit shifts over and over on the same
		   int for 32 iterations, then get a new random int */
		if (!(k % 32))
			rnd = LRAND();

		/* complex sqrt: x^0.5 = radius^0.5*(cos(theta/2) + i*sin(theta/2)) */

		xi -= jp->ci;
		xr -= jp->cr;

		theta = atan2(xi, xr) / 2.0;

		/*r = pow(xi * xi + xr * xr, 0.25); */
		r = sqrt(sqrt(xi * xi + xr * xr));	/* 3 times faster */

		xr = r * cos(theta);
		xi = r * sin(theta);

		if ((rnd >> (k % 32)) & 0x1) {
			xi = -xi;
			xr = -xr;
		}
		xp->x = jp->centerx + (int) ((jp->centerx >> 1) * xr);
		xp->y = jp->centery + (int) ((jp->centery >> 1) * xi);
		xp++;
	}

	jp->itree = 0;
	apply(mi, xr, xi, jp->depth);

	XSetForeground(display, gc, MI_WIN_BLACK_PIXEL(mi));
	XFillRectangle(display, window, gc, 0, 0,
		       jp->centerx * 2, jp->centery * 2);

	XSetForeground(display, gc, MI_WIN_WHITE_PIXEL(mi));
	XDrawPoints(display, window, gc,
		    pointBuffer, numpoints, CoordModeOrigin);

	/* draw a circle at the c-parameter so you can see it's effect on the
	   structure of the julia set */
	XFillArc(display, window, gc,
		 (int) (jp->centerx * jp->cr / 2) + jp->centerx - 2,
		 (int) (jp->centery * jp->ci / 2) + jp->centery - 2, 8, 8, 0, 360 * 64);

	init_julia(mi);
}

void
release_julia(ModeInfo * mi)
{
	if (juls != NULL) {
		(void) free((void *) juls);
		juls = NULL;
	}
	if (pointBuffer) {
		(void) free((void *) pointBuffer);
		pointBuffer = NULL;
	}
}

void
refresh_julia(ModeInfo * mi)
{
	/* Do nothing, it will refresh by itself */
}
