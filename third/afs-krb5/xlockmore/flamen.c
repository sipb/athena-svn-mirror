
#ifndef lint
static char sccsid[] = "@(#)flamen.c	3.11h 96/09/30 xlockmore";

#endif

/*-
 * flamen.c - new recursive fractal cosmic flames.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 01-Jun-95: Updated by Scott Draves.
 * 27-Jun-91: vary number of functions used.
 * 24-Jun-91: fixed portability problem with integer mod (%).
 * 06-Jun-91: Written. (received from Scott Draves, spot@cs.cmu.edu).
 */

#include "xlock.h"
#include <math.h>

/* 
   #include <sys/time.h> #include <sys/file.h> #include <sys/types.h> */
#define MAXBATCH1	200	/* mono */
#define MAXBATCH2	20	/* color */
#define FUSE		10	/* discard this many initial iterations */
#define NMAJORVARS	7

   /* if this is defined, then instead of growing the fractals, we animate */
   /* them one at a time */
/* #define DRIFT */

	/* if this is defined then instead of a point bouncing around 
	   in a high dimensional sphere, we use lissojous figures.
	   only makes sense if DRIFT is also defined */

/* #define LISS */

#ifdef DRIFT
ModeSpecOpt drift_opts =
#else
ModeSpecOpt flamen_opts =
#endif
{0, NULL, 0, NULL, NULL};

typedef struct {
	/* shape of current flame */
	int         nxforms;
	double      f[2][3][10];	/* a bunch of non-homogeneous xforms */
	int         variation[10];	/* for each xform */
#ifdef DRIFT
	double      df[2][3][10];
	Pixmap      drawon;
#endif

	/* high-level control */
	int         mode;	/* 0->slow/single 1->fast/many */
	int         nfractals;	/* draw this many fractals */
	int         major_variation;
	int         fractal_len;	/* pts/fractal */
	int         color;
	int         rainbow;	/* more than one color per fractal
				   1-> computed by adding dimension to fractal */

	int         width, height;	/* of window */
	int         timer;

	/* draw info about current flame */
	int         fuse;	/* iterate this many before drawing */
	int         total_points;	/* draw this many pts before fractal ends */
	int         npoints;	/* how many we've computed but not drawn */
	XPoint      pts[MAXBATCH1];	/* here they are */
	unsigned long pixcol;
	/* when drawing in color, we have a buffer per color */
	int         ncpoints[NUMCOLORS];
	XPoint      cpts[NUMCOLORS][MAXBATCH2];

	double      x, y, c;

#ifdef DRIFT
} driftstruct;
static driftstruct drifts[MAXSCREENS];

#else
} flamestruct;
static flamestruct flames[MAXSCREENS];

#endif

#ifdef DRIFT
#define DECLARE_FS   driftstruct *fs = &drifts[MI_SCREEN(mi)]
#else
#define DECLARE_FS   flamestruct *fs = &flames[MI_SCREEN(mi)]
#endif


static short
halfrandom(int mv)
{
	static short lasthalf = 0;
	unsigned long r;

	if (lasthalf) {
		r = lasthalf;
		lasthalf = 0;
	} else {
		r = LRAND();
		lasthalf = r >> 16;
	}
	r = r % mv;
	return r;
}

static int
frandom(int n)
{
	static long saved_random_bits = 0;
	static int  nbits = 0;
	int         result;

	if (3 > nbits) {
		saved_random_bits = LRAND();
		nbits = 31;
	}
	switch (n) {
		case 2:
			result = saved_random_bits & 1;
			saved_random_bits >>= 1;
			nbits -= 1;
			return result;

		case 3:
			result = saved_random_bits & 3;
			saved_random_bits >>= 2;
			nbits -= 2;
			if (3 == result)
				return frandom(3);
			return result;

		case 4:
			result = saved_random_bits & 3;
			saved_random_bits >>= 2;
			nbits -= 2;
			return result;

		case 5:
			result = saved_random_bits & 7;
			saved_random_bits >>= 3;
			nbits -= 3;
			if (4 < result)
				return frandom(5);
			return result;
		default:
			(void) fprintf(stderr, "bad arg to frandom\n");
			exit(1);
	}
	return 0;
}

#define DISTRIB_A (halfrandom(7000) + 9000)
#define DISTRIB_B ((frandom(3) + 1) * (frandom(3) + 1) * 120000)
#define LEN(x) (sizeof(x)/sizeof((x)[0]))

static void
initmode(ModeInfo * mi, int mode)
{
	DECLARE_FS;
	static      variation_distrib[] =
	{
		0, 0, 1, 1, 2, 2, 3,
		4, 4, 5, 5, 6, 6, 6};

	fs->mode = mode;

	fs->major_variation =
		variation_distrib[halfrandom(LEN(variation_distrib))];

#ifdef DRIFT
	fs->nfractals = 1;
	fs->rainbow = fs->color;
	fs->fractal_len = 2000000;
#else
	fs->rainbow = 0;
	if (mode) {
		if (!fs->color || halfrandom(8)) {
			fs->nfractals = halfrandom(30) + 5;
			fs->fractal_len = DISTRIB_A;
		} else {
			fs->nfractals = halfrandom(5) + 5;
			fs->fractal_len = DISTRIB_B;
		}
	} else {
		fs->rainbow = fs->color;
		fs->nfractals = 1;
		fs->fractal_len = DISTRIB_B;
	}

#endif
	fs->fractal_len = (fs->fractal_len * MI_BATCHCOUNT(mi)) / 20;

	XClearWindow(MI_DISPLAY(mi), MI_WINDOW(mi));
}

#ifdef DRIFT
static void
pick_df_coefs(ModeInfo * mi)
{
	int         i, j, k;
	double      r;

	DECLARE_FS;
	for (i = 0; i < fs->nxforms; i++) {

		r = 1e-6;
		for (j = 0; j < 2; j++)
			for (k = 0; k < 3; k++) {
				fs->df[j][k][i] = ((double) halfrandom(1000) / 500.0 - 1.0);
				r += fs->df[j][k][i] * fs->df[j][k][i];
			}
		r = (3 + halfrandom(5)) * 0.01 / sqrt(r);
		for (j = 0; j < 2; j++)
			for (k = 0; k < 3; k++)
				fs->df[j][k][i] *= r;
	}
}
#endif

#ifdef LISS
static int  liss_time = 0;

#endif

static void
initfractal(ModeInfo * mi)
{
	static int  xform_distrib[] =
	{2, 2, 2, 3, 3, 3, 4, 4, 5};

	DECLARE_FS;
	int         i, j, k;


	fs->fuse = FUSE;
	fs->total_points = 0;
	if (fs->rainbow)
		for (i = 0; i < MI_NPIXELS(mi); i++)
			fs->ncpoints[i] = 0;
	else
		fs->npoints = 0;
	fs->nxforms = xform_distrib[halfrandom(LEN(xform_distrib))];
	fs->c = fs->x = fs->y = 0.0;
#ifdef LISS
	if (!halfrandom(10)) {
		liss_time = 0;
	}
#endif
#ifdef DRIFT
	pick_df_coefs(mi);
#endif
	for (i = 0; i < fs->nxforms; i++) {
		if (NMAJORVARS == fs->major_variation)
			fs->variation[i] = halfrandom(NMAJORVARS);
		else
			fs->variation[i] = fs->major_variation;
		for (j = 0; j < 2; j++)
			for (k = 0; k < 3; k++) {
#ifdef LISS
				fs->f[j][k][i] = sin(liss_time * fs->df[j][k][i]);
#else
				fs->f[j][k][i] = ((double) halfrandom(1000) / 500.0 - 1.0);
#endif
			}
	}
	if (fs->color)
		fs->pixcol = MI_PIXEL(mi, halfrandom(MI_NPIXELS(mi)));
	else
		fs->pixcol = MI_WIN_WHITE_PIXEL(mi);

}


void
#ifdef DRIFT
init_drift(ModeInfo * mi)
#else
init_flamen(ModeInfo * mi)
#endif

{
	static int  first_time = 1;

	DECLARE_FS;

	fs->width = MI_WIN_WIDTH(mi);
	fs->height = MI_WIN_HEIGHT(mi);
	fs->color = MI_NPIXELS(mi) > 2;

	if (first_time)
		initmode(mi, 1);
	else
		initmode(mi, frandom(2));
	initfractal(mi);
#ifdef DRIFT
	if (!first_time)
		XFreePixmap(MI_DISPLAY(mi), fs->drawon);
	fs->drawon = XCreatePixmap(MI_DISPLAY(mi), MI_WINDOW(mi),
				   fs->width, fs->height, MI_WIN_DEPTH(mi));
#endif

	first_time = 0;
}

static void
#ifdef DRIFT
iter(driftstruct * fs)
#else
iter(flamestruct * fs)
#endif
{
	int         i = frandom(fs->nxforms);
	double      nx, ny, nc;


	if (i)
		nc = (fs->c + 1.0) / 2.0;
	else
		nc = fs->c / 2.0;

	nx = fs->f[0][0][i] * fs->x + fs->f[0][1][i] * fs->y + fs->f[0][2][i];
	ny = fs->f[1][0][i] * fs->x + fs->f[1][1][i] * fs->y + fs->f[1][2][i];


	switch (fs->variation[i]) {
		case 1:
			/* sinusoidal */
			nx = sin(nx);
			ny = sin(ny);
			break;
		case 2:
			{
				/* complex */
				double      r2 = nx * nx + ny * ny + 1e-6;

				nx = nx / r2;
				ny = ny / r2;
				break;
			}
		case 3:
			/* bent */
			if (nx < 0.0)
				nx = nx * 2.0;
			if (ny < 0.0)
				ny = ny / 2.0;
			break;
		case 4:
			{
				/* swirl */
				double      r = (nx * nx + ny * ny);	/* times k here is fun */
				double      c1 = sin(r);
				double      c2 = cos(r);
				double      t = nx;

				nx = c1 * nx - c2 * ny;
				ny = c2 * t + c1 * ny;
				break;
			}
		case 5:
			{
				/* horseshoe */
				double      r = atan2(nx, ny);	/* times k here is fun */
				double      c1 = sin(r);
				double      c2 = cos(r);
				double      t = nx;

				nx = c1 * nx - c2 * ny;
				ny = c2 * t + c1 * ny;
				break;
			}
		case 6:
			{
				/* drape */
				double      t;

				t = atan2(nx, ny) / M_PI;
				if (nx > 1e4 || nx < -1e4 || ny > 1e4 || ny < -1e4)
					ny = 1e4;
				else
					ny = sqrt(nx * nx + ny * ny) - 1.0;
				nx = t;
				break;
			}
	}

#if 0
	/* here are some others */
	{
		/* broken */
		if (nx > 1.0)
			nx = nx - 1.0;
		if (nx < -1.0)
			nx = nx + 1.0;
		if (ny > 1.0)
			ny = ny - 1.0;
		if (ny < -1.0)
			ny = ny + 1.0;
		break;
	}
	{
		/* complex sine */
		double      u = nx, v = ny;
		double      ev = exp(v);
		double      emv = exp(-v);

		nx = (ev + emv) * sin(u) / 2.0;
		ny = (ev - emv) * cos(u) / 2.0;
	}
	{

		/* polynomial */
		if (nx < 0)
			nx = -nx * nx;
		else
			nx = nx * nx;

		if (ny < 0)
			ny = -ny * ny;
		else
			ny = ny * ny;
	}
	{
		/* spherical */
		double      r = 0.5 + sqrt(nx * nx + ny * ny + 1e-6);

		nx = nx / r;
		ny = ny / r;
	}
	{
		nx = atan(nx) / M_PI_2
			ny = atan(ny) / M_PI_2
	}
#endif

	/* how to check nan too?  some machines don't have finite().
	   don't need to check ny, it'll propogate */
	if (nx > 1e4 || nx < -1e4) {
		nx = halfrandom(1000) / 500.0 - 1.0;
		ny = halfrandom(1000) / 500.0 - 1.0;
		fs->fuse = FUSE;
	}
	fs->x = nx;
	fs->y = ny;
	fs->c = nc;

}

static void
#ifdef DRIFT
draw(ModeInfo * mi, driftstruct * fs, Drawable d)
#else
draw(ModeInfo * mi, flamestruct * fs, Drawable d)
#endif
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	double      x = fs->x;
	double      y = fs->y;
	int         fixed_x, fixed_y, npix, c, n;

	if (fs->fuse) {
		fs->fuse--;
		return;
	}
	if (!(x > -1.0 && x < 1.0 && y > -1.0 && y < 1.0))
		return;

	fixed_x = (int) ((fs->width / 2) * (x + 1.0));
	fixed_y = (int) ((fs->height / 2) * (y + 1.0));

	if (!fs->rainbow) {

		fs->pts[fs->npoints].x = fixed_x;
		fs->pts[fs->npoints].y = fixed_y;
		fs->npoints++;
		if (fs->npoints == MAXBATCH1) {
			XSetForeground(display, gc, fs->pixcol);
			XDrawPoints(display, d, gc, fs->pts, fs->npoints, CoordModeOrigin);
			fs->npoints = 0;
		}
	} else {

		npix = MI_NPIXELS(mi);
		c = (int) (fs->c * npix);

		if (c < 0)
			c = 0;
		if (c >= npix)
			c = npix - 1;
		n = fs->ncpoints[c];
		fs->cpts[c][n].x = fixed_x;
		fs->cpts[c][n].y = fixed_y;
		if (++fs->ncpoints[c] == MAXBATCH2) {
			XSetForeground(display, gc, MI_PIXEL(mi, c));
			XDrawPoints(display, d, gc, fs->cpts[c],
				    fs->ncpoints[c], CoordModeOrigin);
			fs->ncpoints[c] = 0;
		}
	}
}

static void
#ifdef DRIFT
draw_flush(ModeInfo * mi, driftstruct * fs, Drawable d)
#else
draw_flush(ModeInfo * mi, flamestruct * fs, Drawable d)
#endif
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);

	if (fs->rainbow) {
		int         npix = MI_NPIXELS(mi);
		int         i;

		for (i = 0; i < npix; i++) {
			if (fs->ncpoints[i]) {
				XSetForeground(display, gc, MI_PIXEL(mi, i));
				XDrawPoints(display, d, gc, fs->cpts[i],
					    fs->ncpoints[i], CoordModeOrigin);
				fs->ncpoints[i] = 0;
			}
		}
	} else {
		if (fs->npoints)
			XSetForeground(display, gc, fs->pixcol);
		XDrawPoints(display, d, gc, fs->pts,
			    fs->npoints, CoordModeOrigin);
		fs->npoints = 0;
	}
}


void
#ifdef DRIFT
draw_drift(ModeInfo * mi)
#else
draw_flamen(ModeInfo * mi)
#endif

{
	Window      window = MI_WINDOW(mi);

	DECLARE_FS;

#ifdef DRIFT
	XClearWindow(MI_DISPLAY(mi), window);
#endif

	fs->timer = MI_BATCHCOUNT(mi) * 1000;

	while (fs->timer) {
		iter(fs);
#ifdef DRIFT
		draw(mi, fs, fs->drawon);
#else
		draw(mi, fs, window);
#endif
		if (fs->total_points++ > fs->fractal_len) {
#ifdef DRIFT
			draw_flush(mi, fs, fs->drawon);
#else
			draw_flush(mi, fs, window);
#endif
			if (0 == --fs->nfractals)
				initmode(mi, frandom(2));
			initfractal(mi);
		}
		fs->timer--;
	}
#ifdef DRIFT
	{
		int         i, j, k;

		draw_flush(mi, fs, fs->drawon);
		XCopyArea(MI_DISPLAY(mi), fs->drawon, window, MI_GC(mi),
			  0, 0, fs->width, fs->height, 0, 0);
#ifdef LISS
		liss_time++;
#endif
		for (i = 0; i < fs->nxforms; i++)
			for (j = 0; j < 2; j++)
				for (k = 0; k < 3; k++) {
#ifdef LISS
					fs->f[j][k][i] = sin(liss_time * fs->df[j][k][i]);
#else
					double      t = fs->f[j][k][i] += fs->df[j][k][i];

					if (t < -1.0 || 1.0 < t)
						fs->df[j][k][i] *= -1.0;
#endif
				}
	}
#endif
}

void
#ifdef DRIFT
release_drift(ModeInfo * mi)
#else
release_flamen(ModeInfo * mi)
#endif
{
}

void
#ifdef DRIFT
refresh_drift(ModeInfo * mi)
#else
refresh_flamen(ModeInfo * mi)
#endif
{
	XClearWindow(MI_DISPLAY(mi), MI_WINDOW(mi));
}
