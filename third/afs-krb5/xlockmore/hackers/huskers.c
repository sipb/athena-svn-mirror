
#ifndef lint
static char sccsid[] = "@(#)huskers.c	3.11h 96/09/30 xlockmore";

#endif
/*-
 * huskers.c - xpm huskers image bouncer for xlock, the X Window System
 * lockscreen.
 *
 * Copyright (c) 1995 by Skip Burrell <Skip_Burrell@sterling.com>.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 25-May-95: Written. Originally derived from image.c.
 */

#include "xlock.h"
#if defined( HAS_XPM ) || defined( HAS_XPMINC )
#if HAS_XPMINC
#include <xpm.h>
#else
#include <X11/xpm.h>		/* Normal spot */
#endif
#include "pixmaps/huskers-0.xpm"
#include "pixmaps/huskers-1.xpm"
#else
#include "bitmaps/huskers-0.xbm"
#include "bitmaps/huskers-1.xbm"
#endif

ModeSpecOpt huskers_opts =
{0, NULL, 0, NULL, NULL};

static XImage *Logo = NULL;
static XImage *Small_logo = NULL;

#if (!(defined( HAS_XPM ) || defined( HAS_XPMINC )))
static unsigned long RWcolors[2];

static XImage BmLogo =
{
	0, 0,			/* width, height */
	0, XYBitmap, 0,		/* xoffset, format, data */
	LSBFirst, 8,		/* byte-order, bitmap-unit */
	LSBFirst, 8, 1		/* bitmap-bit-order, bitmap-pad, depth */
};
static XImage BmSmall_logo =
{
	0, 0,			/* width, height */
	0, XYBitmap, 0,		/* xoffset, format, data */
	LSBFirst, 8,		/* byte-order, bitmap-unit */
	LSBFirst, 8, 1		/* bitmap-bit-order, bitmap-pad, depth */
};

#endif

#define MAXICONS 64

typedef struct {
	int         x;
	int         y;
} point;

typedef struct {
	int         width;
	int         height;
	int         nrows;
	int         ncols;
	int         xb;
	int         yb;
	int         xoff;
	int         yoff;
	int         iconmode;
	int         iconcount;
	point       icons[MAXICONS];
} imagestruct;

static imagestruct *ims = NULL;

void
init_huskers(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	imagestruct *ip;
	XImage     *logo;

#if (!(defined( HAS_XPM ) || defined( HAS_XPMINC )))
	static int  initialized = 0;

	if (!initialized) {
		XColor      xcolor, rgbcolor;
		Screen     *scr;

		scr = ScreenOfDisplay(display, MI_SCREEN(mi));
		(void) XAllocNamedColor(display, DefaultColormapOfScreen(scr),
					"Red", &xcolor, &rgbcolor);
		RWcolors[0] = xcolor.pixel;
		RWcolors[1] = MI_WIN_WHITE_PIXEL(mi);
		initialized = 1;
	}
	if (!Logo) {
		BmLogo.data = (char *) huskers1_bits;
		BmLogo.width = huskers1_width;
		BmLogo.height = huskers1_height;
		BmLogo.bytes_per_line = (huskers1_width + 7) / 8;
		Logo = &BmLogo;
	}
	if (!Small_logo) {
		BmSmall_logo.data = (char *) huskers0_bits;
		BmSmall_logo.width = huskers0_width;
		BmSmall_logo.height = huskers0_height;
		BmSmall_logo.bytes_per_line = (huskers0_width + 7) / 8;
		Small_logo = &BmSmall_logo;
	}
#else
  int         screen = MI_SCREEN(mi);
  XpmAttributes attrib;

  attrib.visual = DefaultVisual(display, screen);
  attrib.colormap = DefaultColormap(display, screen);
  attrib.depth = DefaultDepth(display, screen);
  attrib.valuemask = XpmVisual | XpmColormap | XpmDepth;

	if (!Logo)
		XpmCreateImageFromData(display, (char **) huskers1_name,
			    &Logo, (XImage **) NULL, &attrib);
	if (!Small_logo)
		XpmCreateImageFromData(display, (char **) huskers0_name,
		      &Small_logo, (XImage **) NULL, &attrib);
#endif

  if (ims == NULL) {
    if ((ims = (imagestruct *) calloc(MI_NUM_SCREENS(mi),
                sizeof (imagestruct))) == NULL)
      return;
  }
  ip = &ims[MI_SCREEN(mi)];

	ip->width = MI_WIN_WIDTH(mi);
	ip->height = MI_WIN_HEIGHT(mi);
	if ((ip->width < Logo->width) || (ip->height < Logo->height)) {
		logo = Small_logo;
	} else {
		logo = Logo;
	}

	ip->ncols = ip->width / logo->width;
	ip->nrows = ip->height / logo->height;
	ip->xoff = (ip->width - ip->ncols * logo->width) / 2;
	ip->yoff = (ip->height - ip->nrows * logo->height) / 2;
	ip->iconmode = (ip->ncols < 2 || ip->nrows < 2);
	if (ip->iconmode) {
		ip->xb = 0;
		ip->yb = 0;
		ip->iconcount = 1;	/* icon mode */
	} else {
		ip->xb = (ip->width - logo->width * ip->ncols) / 2;
		ip->yb = (ip->height - logo->height * ip->nrows) / 2;
		ip->iconcount = MI_BATCHCOUNT(mi);
		if (ip->iconcount > MAXICONS)
			ip->iconcount = 16;
	}
	XClearWindow(display, MI_WINDOW(mi));
}

void
draw_huskers(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	imagestruct *ip = &ims[MI_SCREEN(mi)];
	int         i;
	XImage     *logo;

	logo = ip->iconmode ? Small_logo : Logo;

	XSetForeground(display, gc, MI_WIN_BLACK_PIXEL(mi));
	for (i = 0; i < ip->iconcount; i++) {
		if (!ip->iconmode)
			XFillRectangle(display, MI_WINDOW(mi), gc,
			    ip->xb + logo->width * ip->icons[i].x + ip->xoff,
			   ip->yb + logo->height * ip->icons[i].y + ip->yoff,
				       logo->width, logo->height);

		ip->icons[i].x = (ip->ncols ? NRAND(ip->ncols) : 0);
		ip->icons[i].y = (ip->nrows ? NRAND(ip->nrows) : 0);
	}
	if (MI_NPIXELS(mi) <= 2)
		XSetForeground(display, gc, MI_WIN_WHITE_PIXEL(mi));
	for (i = 0; i < ip->iconcount; i++) {
#if (!(defined( HAS_XPM ) || defined( HAS_XPMINC )))
		if (MI_NPIXELS(mi) > 2)
			XSetForeground(display, gc, RWcolors[LRAND() & 1]);
#endif

		XPutImage(display, MI_WINDOW(mi), gc, logo,
			  0, 0,
			  ip->xb + logo->width * ip->icons[i].x + ip->xoff,
			  ip->yb + logo->height * ip->icons[i].y + ip->yoff,
			  logo->width, logo->height);
	}
}

void
release_huskers(ModeInfo * mi)
{
  if (ims != NULL) {
    (void) free((void *) ims);
    ims = NULL;
  }
}

void
refresh_huskers(ModeInfo * mi)
{
  /* Do nothing, it will refresh by itself */
}
