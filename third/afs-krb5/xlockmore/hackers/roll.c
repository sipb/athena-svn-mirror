
#ifndef lint
static char sccsid[] = "@(#)roll.c 3.11h 96/09/30 xlockmore";

#endif

/*-
 * roll.c - roll for xlock, the X Window System lockscreen.
 *
 * Copyright (c) 1995 by Charles Vidal <vidalc@univ-mlv.fr>
 *         http://fillmore.univ-mlv.fr/~vidalc
 *
 * See xlock.c for copying information.
 */

#include "xlock.h"
#include <math.h>

#define MINPTS 1
#define XVAL 160.0
#define YVAL 128.0
#define DVAL 100.0
#define RVAL 50.0
#define TVAL 60.0
#define W	320
#define H	250
#define NB_POINT 625
#define FACTOR 8.0
#define FACTORINT 8
#define COEF 25

ModeSpecOpt roll_opts =
{0, NULL, 0, NULL, NULL};

typedef struct {
	double      t, u, v;
	double      t1, u1, v1;
} ptsstruct;

typedef struct {
	ptsstruct  *pts;
	XPoint     *p;
	int         maxpts, npts;
	double      alpha, theta, phi, r;
	XPoint      sphere, direct;
	int         color;
	int         width, height;
} rollstruct;

static rollstruct *rolls = NULL;

static void
creersphere(rollstruct * rp, int n1, int n2)
{
	double      i, j;
	int         n = 0;

	for (i = 0; i < FACTORINT * M_PI; i += (FACTORINT * M_PI) / n1)
		for (j = 0; j < FACTORINT * M_PI; j += (FACTORINT * M_PI) / n2) {
			rp->pts[n].t1 = rp->r * cos(i) * cos(j);
			rp->pts[n].u1 = rp->r * cos(i) * sin(j);
			rp->pts[n].v1 = rp->r * sin(i);
			n++;
		}
}

static void
rotation3d(rollstruct * rp)
{
	double      c1, c2, c3, c4, c5, c6, c7, c8, c9, x, y, z;
	double      sintheta, costheta;
	double      sinphi, cosphi;
	double      sinalpha, cosalpha;
	int         i;

	sintheta = sin(rp->theta);
	costheta = cos(rp->theta);
	sinphi = sin(rp->phi);
	cosphi = cos(rp->phi);
	sinalpha = sin(rp->alpha);
	cosalpha = cos(rp->alpha);

	c1 = cosphi * costheta;
	c2 = sinphi * costheta;
	c3 = -sintheta;

	c4 = cosphi * sintheta * sinalpha - sinphi * cosalpha;
	c5 = sinphi * sintheta * sinalpha + cosphi * cosalpha;
	c6 = costheta * sinalpha;

	c7 = cosphi * sintheta * cosalpha + sinphi * sinalpha;
	c8 = sinphi * sintheta * cosalpha - cosphi * sinalpha;
	c9 = costheta * cosalpha;
	for (i = 0; i < rp->maxpts; i++) {
		x = rp->pts[i].t;
		y = rp->pts[i].u;
		z = rp->pts[i].v;
		rp->pts[i].t = c1 * x + c2 * y + c3 * z;
		rp->pts[i].u = c4 * x + c5 * y + c6 * z;
		rp->pts[i].v = c7 * x + c8 * y + c9 * z;
	}
}

#if 0
static void
projete(rollstruct * rp)
{
	int         i;

	for (i = 0; i < rp->maxpts; i++) {
		if (rp->pts[i].v == 0.0) {
			rp->p[i].x = rp->p[i].y = 0;
		} else {
			rp->p[i].x = (short) (XVAL + DVAL *
				     (rp->pts[i].t / (rp->pts[i].v + TVAL)));
			rp->p[i].y = (short) (YVAL + DVAL *
				     (rp->pts[i].u / (rp->pts[i].v + TVAL)));
		}
	}
}

#endif

static void
projete2(rollstruct * rp)
{
	int         i;

	for (i = 0; i < rp->maxpts; i++) {
		rp->p[i].x = (short) (XVAL + 2 * rp->pts[i].t);
		rp->p[i].y = (short) (YVAL + 2 * rp->pts[i].u);
	}
}

void
init_roll(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	rollstruct *rp;

	if (rolls == NULL) {
		if ((rolls = (rollstruct *) calloc(MI_NUM_SCREENS(mi),
					       sizeof (rollstruct))) == NULL)
			return;
	}
	rp = &rolls[MI_SCREEN(mi)];

	rp->direct.x = rp->direct.y = 5;
	rp->r = RVAL;
	rp->alpha = 0;
	rp->theta = 0;
	rp->phi = 0;
	rp->width = MI_WIN_WIDTH(mi);
	rp->height = MI_WIN_HEIGHT(mi);
	rp->maxpts = MI_BATCHCOUNT(mi);
	if (rp->maxpts < -MINPTS) {
		/* if rp->maxpts is random ... the size can change */
		if (rp->pts != NULL) {
			(void) free((void *) rp->pts);
			rp->pts = NULL;
		}
		if (rp->p != NULL) {
			(void) free((void *) rp->p);
			rp->p = NULL;
		}
		rp->maxpts = NRAND(-rp->maxpts - MINPTS + 1) + MINPTS;
	} else if (rp->maxpts < MINPTS)
		rp->maxpts = MINPTS;
	rp->maxpts *= rp->maxpts;
	if (!rp->pts)
		rp->pts = (ptsstruct *) malloc(rp->maxpts * sizeof (ptsstruct));
	if (!rp->p)
		rp->p = (XPoint *) malloc(rp->maxpts * sizeof (XPoint));

	creersphere(rp, sqrt(rp->maxpts), sqrt(rp->maxpts));
	XClearWindow(display, MI_WINDOW(mi));
}

void
draw_roll(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	GC          gc = MI_GC(mi);
	rollstruct *rp = &rolls[MI_SCREEN(mi)];
	int         i;

	for (i = 0; i < rp->maxpts; i++) {
		rp->pts[i].t = rp->pts[i].t1;
		rp->pts[i].u = rp->pts[i].u1;
		rp->pts[i].v = rp->pts[i].v1;
	}
	rp->alpha += ((FACTOR * M_PI) / 200.0);
	rp->theta += ((FACTOR * M_PI) / 200.0);
	rp->phi += ((FACTOR * M_PI) / 200.0);
	if (rp->alpha > (FACTOR * M_PI))
		rp->alpha -= (FACTOR * M_PI);
	if (rp->theta > (FACTOR * M_PI))
		rp->theta -= (FACTOR * M_PI);
	if (rp->phi > (FACTOR * M_PI))
		rp->phi -= (FACTOR * M_PI);

	if (rp->npts) {
	  XSetForeground(display, gc, MI_WIN_BLACK_PIXEL(mi));
	  XDrawPoints(display, window, gc, rp->p, rp->npts, CoordModeOrigin);
  } else
		rp->p = (XPoint *) malloc(rp->maxpts * sizeof (XPoint));
	rotation3d(rp);
	projete2(rp);
	rp->npts = 0;
	for (i = 0; i < rp->maxpts; i++) {
		if (rp->pts[i].v > 0.0) {
			rp->p[rp->npts].x += rp->sphere.x;
			rp->p[rp->npts].y += rp->sphere.y;
			rp->npts++;
		}
	}
	if (MI_NPIXELS(mi) <= 2)
		XSetForeground(display, gc, MI_WIN_WHITE_PIXEL(mi));
	else {
		rp->color = (rp->color + 1) % MI_NPIXELS(mi);
		XSetForeground(display, gc, MI_PIXEL(mi, rp->color));
	}
	XDrawPoints(display, window, gc, rp->p, rp->npts, CoordModeOrigin);
	if (rp->sphere.x + XVAL + DVAL > rp->width)
		rp->direct.x = -5;
	if (rp->sphere.x < 0)
		rp->direct.x = 5;
	if (rp->sphere.y + YVAL + DVAL > rp->height)
		rp->direct.y = -5;
	if (rp->sphere.y < 0)
		rp->direct.y = 5;
	rp->sphere.x += rp->direct.x;
	rp->sphere.y += rp->direct.y;
}

void
release_roll(ModeInfo * mi)
{
	if (rolls != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			rollstruct *rp = &rolls[screen];

			if (rp->pts != NULL)
				(void) free((void *) rp->pts);
			if (rp->p != NULL)
				(void) free((void *) rp->p);
		}
		(void) free((void *) rolls);
		rolls = NULL;
	}
}

void
refresh_roll(ModeInfo * mi)
{
	/* Do nothing, it will refresh by itself */
}
