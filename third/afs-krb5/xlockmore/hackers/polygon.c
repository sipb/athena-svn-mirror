
#ifndef lint
static char sccsid[] = "@(#)polygon.c	3.11h 96/09/30 xlockmore";

#endif

/*-
 * polygon.c - flying and circling polygon/squares screen for xlock,
 *             the X Window System lockscreen.
 * Copyright (C) 1995 by Michael Stembera <mrbig@fc.net>.
 * Use freely anywhere except on any Microsoft platform.
 * See xlock.c for more copying information.
 * Revision History:
 * 29-Aug-95: Written.
 */

#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <sys/time.h>
#include <sys/types.h>
#include "xlock.h"

#define  SIZE   40		/* size of squares */

ModeSpecOpt polygon_opts =
{0, NULL, 0, NULL, NULL};

static GC   gcb, *gcpt, gct1, gct2;	/* GC to draw with */
static XGCValues gcv;		/* Struct for creating GC */
static float *r;
static short height, width;
static XPoint *locations, *fourpoly;
static XPoint *oldlocations, *oldfourpoly;

static unsigned long black;	/* Pixel values */
static XColor xcolor;		/* Temporary color */
static Colormap pcmap;		/* Color map to use */

static XPoint *tmp;
static short count;
static unsigned long i;
static long red, green, blue;
static float spacing;
static char *str, *str1, *str2, *str1old, *str2old;
static char *str1pta, *str2pta, *str1ptb, *str2ptb, *tmppt;
static XFontStruct *fontStruct;
static char eight_bit;
static unsigned char pixcount;
static float dspacing;
static time_t timenew, timeold;
static short maxx, maxy, clockx, clocky, text_height, cent_offset;
static unsigned short tmp_color;
static unsigned short *xcolorpt;
static short hour;

static void
initr(int batch)
{
/* set up initial square coordinates */
	int         ii;
	float       radius;

	radius = height / 2 - SIZE / 2;

	for (ii = 0; ii < batch; ii++)
		r[ii] = radius;
}

static void
initlocations(int batch)
{
	int         ii;

	spacing = (batch / 3) * (2 * M_PI) / batch;
	for (ii = 0; ii < batch; ii++) {
		locations[ii].x = (width / 2) + (int) (r[ii] * cos(ii * spacing));
		locations[ii].y = (height / 2) + (int) (r[ii] * sin(ii * spacing));
	}
}

static void
initfourpoly(int batch)
{
/* calculate initial square corners from coordinates */
	int         ii;

	for (ii = 0; ii < batch * 4; ii += 4) {
		fourpoly[ii].x = locations[ii >> 2].x - (SIZE / 2);
		fourpoly[ii].y = locations[ii >> 2].y - (SIZE / 2);
		fourpoly[ii + 1].x = fourpoly[ii].x + SIZE;
		fourpoly[ii + 1].y = fourpoly[ii].y;
		fourpoly[ii + 2].x = fourpoly[ii + 1].x;
		fourpoly[ii + 2].y = fourpoly[ii + 1].y + SIZE;
		fourpoly[ii + 3].x = fourpoly[ii + 2].x - SIZE;
		fourpoly[ii + 3].y = fourpoly[ii + 2].y;
	}
}

/* get the first value for clock */
static void
initclk(void)
{
	/* (void)time(&timenew); */
	timeold = timenew = time((long *) NULL);
	str = ctime(&timeold);

	(void) strncpy(str1, (str + 11), 8);
	hour = (short) (str1[0] - 48) * 10 + (short) (str1[1] - 48);
	if (hour > 12) {
		hour -= 12;
		(void) strcpy(str1 + 8, " PM");
	} else {
		if (hour == 0)
			hour += 12;
		(void) strcpy(str1 + 8, " AM");
	}
	str1[0] = (hour / 10) + 48;
	str1[1] = (hour % 10) + 48;
	if (str1[0] == '0')
		str1[0] = ' ';
	str1[11] = 0;		/* terminate str1 */
	str1old[11] = 0;	/* terminate str1old */

	(void) strncpy(str2, str, 11);
	(void) strncpy(str2 + 11, (str + 20), 4);
	str2[15] = 0;		/* terminate str2 */
	str2old[15] = 0;	/* terminate str2old */

	text_height = fontStruct->max_bounds.descent + fontStruct->max_bounds.ascent;
	cent_offset = (XTextWidth(fontStruct, str2, strlen(str2)) -
		       XTextWidth(fontStruct, str1, strlen(str1))) / 2;
	maxx = (width - XTextWidth(fontStruct, str2, strlen(str2)) + cent_offset);
	maxy = (height - text_height - fontStruct->max_bounds.descent);
	clockx = NRAND(maxx - cent_offset) + cent_offset;
	clockx = NRAND(maxx - cent_offset) + cent_offset;
	clocky = NRAND(maxy - fontStruct->max_bounds.ascent) +
		fontStruct->max_bounds.ascent;

	str1pta = str1;
	str2pta = str2;
	str1ptb = str1old;
	str2ptb = str2old;
}

static void
MovePoints(int batch, XPoint * mplocations, float *mpr)
{
	static float angle = 0, dangle = 2 * M_PI / 1440, tmpf;
	static float mpspacing = (2 * M_PI) / 3;
	static short scount2 = 0;
	static float dr = 0.1;
	static short unsigned acount = 0, tmpi;

	if (!(acount % 600))
		dangle = (NRAND(5) - 2) * 2 * M_PI / 1440;

	scount2++;

	i = (int) (acount + (batch - 1)) % batch;
	mpr[i] = mpr[(int) (acount + (batch - 2)) % batch] +
		dr;
	if (abs((int) mpr[i]) > height / 2 - SIZE / 2) {
		mpr[i] -= dr;
		scount2 += batch * 2;	/* to get the squares off the outer edge sooner */
	} else if (abs((int) mpr[i]) < 2 * SIZE)
		if (dr > 0)
			dr += .1;
		else
			dr -= .1;

	mpspacing += dspacing;	/* could be lessened as well */
/* dspacing could  also be NRAND varied */

	if (scount2 > batch * 60) {
		scount2 = 0;
		dr = NRAND(7) - 3;
	}
	for (i = 0; i < (unsigned long) batch; i++) {
		tmpf = angle + i * mpspacing;
		tmpi = (acount + i) % batch;
		mplocations[i].x = (width / 2) + (int) (mpr[tmpi] * cos(tmpf));
		mplocations[i].y = (height / 2) + (int) (mpr[tmpi] * sin(tmpf));
	}
	angle += dangle;
	acount++;
}


/* calculate square corners from center coordinates */
static void
CalcFourPoly(int batch, XPoint * cfplocations, XPoint * cfpfourpoly)
{
	static int  ii;

	for (ii = 0; ii < batch * 4; ii += 4) {
		cfpfourpoly[ii].x = cfplocations[ii >> 2].x - (SIZE / 2);
		cfpfourpoly[ii].y = cfpfourpoly[ii + 1].y = cfplocations[ii >> 2].y - (SIZE / 2);
		cfpfourpoly[ii + 1].x = cfpfourpoly[ii + 2].x = cfpfourpoly[ii].x + SIZE;
		cfpfourpoly[ii + 2].y = cfpfourpoly[ii + 3].y = cfpfourpoly[ii + 1].y + SIZE;
		cfpfourpoly[ii + 3].x = cfpfourpoly[ii + 2].x - SIZE;
	}
}

/* calculate the difference between two overlaping squares as a 6 point
   polygon  */
static void
GetSquareDelta(XPoint * newsquare, XPoint * presentsquare, XPoint * deltapoly)
{
	if (newsquare[0].x >= presentsquare[0].x)
		if (newsquare[0].y >= presentsquare[0].y) {
			deltapoly[0].x = newsquare[1].x;
			deltapoly[0].y = newsquare[1].y;
			deltapoly[1].x = newsquare[2].x;
			deltapoly[1].y = newsquare[2].y;
			deltapoly[2].x = newsquare[3].x;
			deltapoly[2].y = newsquare[3].y;
			deltapoly[3].x = newsquare[3].x;
			deltapoly[3].y = presentsquare[3].y;
			deltapoly[4].x = presentsquare[2].x;
			deltapoly[4].y = presentsquare[2].y;
			deltapoly[5].x = presentsquare[2].x;
			deltapoly[5].y = newsquare[1].y;
		} else {
			deltapoly[0].x = newsquare[0].x;
			deltapoly[0].y = presentsquare[0].y;
			deltapoly[1].x = newsquare[0].x;
			deltapoly[1].y = newsquare[0].y;
			deltapoly[2].x = newsquare[1].x;
			deltapoly[2].y = newsquare[1].y;
			deltapoly[3].x = newsquare[2].x;
			deltapoly[3].y = newsquare[2].y;
			deltapoly[4].x = presentsquare[1].x;
			deltapoly[4].y = newsquare[2].y;
			deltapoly[5].x = presentsquare[1].x;
			deltapoly[5].y = presentsquare[1].y;
	} else if (newsquare[0].y >= presentsquare[0].y) {
		deltapoly[0].x = newsquare[0].x;
		deltapoly[0].y = newsquare[0].y;
		deltapoly[1].x = presentsquare[0].x;
		deltapoly[1].y = newsquare[0].y;
		deltapoly[2].x = presentsquare[3].x;
		deltapoly[2].y = presentsquare[3].y;
		deltapoly[3].x = newsquare[2].x;
		deltapoly[3].y = presentsquare[2].y;
		deltapoly[4].x = newsquare[2].x;
		deltapoly[4].y = newsquare[2].y;
		deltapoly[5].x = newsquare[3].x;
		deltapoly[5].y = newsquare[3].y;
	} else {
		deltapoly[0].x = newsquare[3].x;
		deltapoly[0].y = newsquare[3].y;
		deltapoly[1].x = newsquare[0].x;
		deltapoly[1].y = newsquare[0].y;
		deltapoly[2].x = newsquare[1].x;
		deltapoly[2].y = newsquare[1].y;
		deltapoly[3].x = newsquare[1].x;
		deltapoly[3].y = presentsquare[0].y;
		deltapoly[4].x = presentsquare[0].x;
		deltapoly[4].y = presentsquare[0].y;
		deltapoly[5].x = presentsquare[0].x;
		deltapoly[5].y = newsquare[3].y;
	}
}

static void
AnimPolygons(ModeInfo * mi, int batch, XPoint * apfourpoly,
	     XPoint * apoldfourpoly)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	static short api, j, tmpx, tmpy, closest = 0;
	static short mini, temp;
	static short d;
	static unsigned char flag;
	static XPoint delta[6];

	d = SIZE * SIZE + SIZE * SIZE;

/* go through and find closest square to the current one check to see if they
   overlap if yes draw only the visible portion (6 point polygon)  if no draw
   the whole square */

	/* for(api=(batch-1)*4; api>=0; api-=4) {  */
	for (api = 0; api < batch * 4; api += 4) {
		flag = 0;
		mini = temp = d;
		tmpx = tmpy = SIZE;
		/* for(j=(batch-1)<<2; j>api; j-=4) { */
		for (j = 0; j < api; j += 4) {
			tmpx = abs(apfourpoly[api].x - apfourpoly[j].x);
			tmpy = abs(apfourpoly[api].y - apfourpoly[j].y);
			temp = (tmpx) * (tmpx) + (tmpy) * (tmpy);
			if ((temp < mini) && (tmpx < SIZE) && (tmpy < SIZE)) {
				mini = temp;
				closest = j;
				flag = 1;
			}
		}
		if (flag) {
			GetSquareDelta(apfourpoly + i, apfourpoly + closest, delta);
			/* try: 2 XDrawRectangles for speed */
			XFillPolygon(display, window, gcpt[api >> 2],
				     delta, 6, Convex, CoordModeOrigin);
		} else {
			XFillRectangle(display, window, gcpt[api >> 2],
				       apfourpoly[api].x, apfourpoly[api].y,
				       SIZE, SIZE);
		}
/* clear remainder of square from its old position */
		GetSquareDelta(apoldfourpoly + api, apfourpoly + api, delta);
		XFillPolygon(display, window, gcb, delta, 6, Convex, CoordModeOrigin);
/* try: use color of closest for clearing and use inv xor */
	}
}

static void
ChangeColor(ModeInfo * mi, int batch)
{
	Display    *display = MI_DISPLAY(mi);
	static short ii, coin, delta;
	static char rflag = 1, gflag = 0, bflag = 1;

/* try: use % instead of loop */
	for (ii = 0; ii < batch - 1; ii++)
		XCopyGC(display, gcpt[ii + 1], (GCForeground), gcpt[ii]);

	if (eight_bit) {
		pixcount += (LRAND() & 1);
		gcv.foreground = MI_PIXEL(mi, pixcount % 64);
		XChangeGC(display, gcpt[batch - 1], (GCForeground), &gcv);
	} else {
/* increment cycle through one of RGB by no more than 4% */
		delta = NRAND(2621);	/* max 4% change, avg 2% change */
		coin = NRAND(3);
		switch (coin) {
			case 0:
				if (rflag) {
					red += delta;
					if (red > 65535) {
						red = 65535;
						rflag = 0;
					}
				} else {
					red -= delta;
					if (red < 655) {
						red = 655;
						rflag = 1;
					}
				}
				break;
			case 1:
				if (gflag) {
					green += delta;
					if (green > 65535) {
						green = 65535;
						gflag = 0;
					}
				} else {
					green -= delta;
					if (green < 655) {
						green = 655;
						gflag = 1;
					}
				}
				break;
			case 2:
				if (bflag) {
					blue += delta;
					if (blue > 65535) {
						blue = 65535;
						bflag = 0;
					}
				} else {
					blue -= delta;
					if (blue < 655) {
						blue = 655;
						bflag = 1;
					}
				}
		}
		xcolor.red = red;
		xcolor.green = green;
		xcolor.blue = blue;
		(void) XAllocColor(display, pcmap, &xcolor);
		gcv.foreground = xcolor.pixel;
		XChangeGC(display, gcpt[batch - 1], (GCForeground), &gcv);
	}

	if (eight_bit) {
		gcv.foreground = MI_PIXEL(mi, (int) (pixcount + 32) % 64);
	} else {
		XGetGCValues(display, gcpt[batch / 2], (GCForeground), &gcv);
		xcolor.pixel = gcv.foreground;
		XQueryColor(display, pcmap, &xcolor);

/* find the most "distant" color from middle square in the RGB space  for
   clock  */
		tmp_color = xcolor.red;
		xcolorpt = &xcolor.red;
		if (tmp_color > xcolor.green) {
			tmp_color = xcolor.green;
			xcolorpt = &xcolor.green;
		}
		if (tmp_color > xcolor.blue) {
			tmp_color = xcolor.blue;
			xcolorpt = &xcolor.blue;
		}
		if (xcolor.red > 32768)
			xcolor.red = 0;
		else
			xcolor.red = 65535;
		if (xcolor.green > 32768)
			xcolor.green = 0;
		else
			xcolor.green = 65535;
		if (xcolor.blue > 32768)
			xcolor.blue = 0;
		else
			xcolor.blue = 65535;
/* this to make sure we don't get a black clock */
		if (tmp_color >= 32768)
			*xcolorpt = 65535;

		(void) XAllocColor(display, pcmap, &xcolor);
		gcv.foreground = xcolor.pixel;
	}
	XChangeGC(display, gct1, (GCForeground), &gcv);
}

static void
DrawClock(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	static short xold, yold, dx = 1, dy = 1;

	xold = clockx;
	yold = clocky;
	clockx += dx;
	clocky += dy;

	if (clockx > maxx || clockx < cent_offset) {
		dx = -dx;
		clockx += dx;
	}
	if (clocky > maxy || clocky < fontStruct->max_bounds.ascent) {
		dy = -dy;
		clocky += dy;
	}
	if (timeold != (timenew = time((long *) NULL))) {	/* only parse if time has changed */
		timeold = timenew;
		str = ctime(&timeold);

		/* keep last disp time so it can be cleared even if it changed */
		tmppt = str1ptb;
		str1ptb = str1pta;
		str1pta = tmppt;
		tmppt = str2ptb;
		str2ptb = str2pta;
		str2pta = tmppt;

		/* copy the hours portion for 24 to 12 hr conversion */
		(void) strncpy(str1pta, (str + 11), 8);
		hour = (short) (str1pta[0] - 48) * 10 + (short) (str1pta[1] - 48);
		if (hour > 12) {
			hour -= 12;
			(void) strcpy(str1pta + 8, " PM");
		} else {
			if (hour == 0)
				hour += 12;
			(void) strcpy(str1pta + 8, " AM");
		}
		str1pta[0] = (hour / 10) + 48;
		str1pta[1] = (hour % 10) + 48;
		if (str1pta[0] == '0')
			str1pta[0] = ' ';

		/* copy day month */
		(void) strncpy(str2pta, str, 11);
		/* copy year */
		(void) strncpy(str2pta + 11, (str + 20), 4);

		XDrawString(display, window, gct2, xold, yold,
			    str1ptb, strlen(str1ptb));	/* erase */
		XDrawString(display, window, gct1, clockx, clocky,
			    str1pta, strlen(str1pta));	/* draw */

		XDrawString(display, window,
			    gct2, xold - cent_offset, yold + text_height,
			    str2ptb, strlen(str2ptb));	/* erase */
		XDrawString(display, window,
			    gct1, clockx - cent_offset, clocky + text_height,
			    str2pta, strlen(str2pta));	/* draw */
	} else {		/* do this if time is the same */
		XDrawString(display, window, gct2, xold, yold,
			    str1pta, strlen(str1pta));	/* erase */
		XDrawString(display, window, gct1, clockx, clocky,
			    str1pta, strlen(str1pta));	/* draw */

		XDrawString(display, window, gct2,
			    xold - cent_offset, yold + text_height,
			    str2pta, strlen(str2pta));	/* erase */
		XDrawString(display, window, gct1,
			    clockx - cent_offset, clocky + text_height,
			    str2pta, strlen(str2pta));	/* draw */
	}
}

void
init_polygon(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	char        rflag = 1, gflag = 0, bflag = 1;	/* RGB flags */

	short       coin, delta;

	dspacing = ((2 * M_PI) / MI_BATCHCOUNT(mi)) / 3000;

	XClearWindow(display, window);
/* check for color depth */
	if (MI_WIN_DEPTH(mi) > 8)
		eight_bit = 0;
	else
		eight_bit = 1;

	r = (float *) malloc(MI_BATCHCOUNT(mi) * sizeof (float));

	locations = (XPoint *) malloc(MI_BATCHCOUNT(mi) *
				      sizeof (XPoint));		/* hold x,y coord. */
	fourpoly = (XPoint *) malloc(4 * MI_BATCHCOUNT(mi) *
				     sizeof (XPoint));	/* hold corner coord. */
	oldlocations = (XPoint *) malloc(MI_BATCHCOUNT(mi) *
					 sizeof (XPoint));
	oldfourpoly = (XPoint *) malloc(4 * MI_BATCHCOUNT(mi) * sizeof (XPoint));
	gcpt = (GC *) malloc(MI_BATCHCOUNT(mi) * sizeof (GC));
	str = (char *) malloc(40 * sizeof (char));	/* to hold time & date string */
	str1 = (char *) malloc(20 * sizeof (char));	/* to hold time */
	str2 = (char *) malloc(20 * sizeof (char));	/* to hold date */
	str1old = (char *) malloc(20 * sizeof (char));
	str2old = (char *) malloc(20 * sizeof (char));

	initr(MI_BATCHCOUNT(mi));
	initlocations(MI_BATCHCOUNT(mi));
	initfourpoly(MI_BATCHCOUNT(mi));

	pcmap = XCreateColormap(display, window, MI_VISUAL(mi), AllocNone);
#if 0
	XSetWindowColormap(display, window, pcmap);
	(void) XSetWMColormapWindows(display, window, &window, 1);
	XInstallColormap(display, pcmap);
#endif
	width = MI_WIN_WIDTH(mi);
	height = MI_WIN_HEIGHT(mi);

	gcv.foreground = black;
	gcv.background = black;
	gcv.function = GXcopy;
	gcb = XCreateGC(display, window,
			(GCForeground | GCBackground | GCFunction), &gcv);
	gcv.function = GXcopy;

	if (eight_bit) {
		pixcount = NRAND(MI_NPIXELS(mi));
		for (i = 0; i < (unsigned long) MI_BATCHCOUNT(mi); i++) {
			pixcount += (LRAND() & 1);
			gcv.foreground = MI_PIXEL(mi, pixcount % 64);
			gcpt[i] = XCreateGC(display, window, (GCForeground | GCBackground
							| GCFunction), &gcv);
		}
	} else {
		red = NRAND(65536);
		green = NRAND(65536);
		blue = NRAND(65536);
		xcolor.red = red;
		xcolor.green = green;
		xcolor.blue = blue;
		(void) XAllocColor(display, pcmap, &xcolor);
		gcv.foreground = xcolor.pixel;

/* increment cycle through one of RGB by no more than 4% */
		for (i = 0; i < (unsigned long) MI_BATCHCOUNT(mi); i++) {
			gcpt[i] = XCreateGC(display, window, (GCForeground | GCBackground
							| GCFunction), &gcv);
			delta = NRAND(2621);	/* max 4% change, avg 2% change */
			coin = NRAND(3);
			switch (coin) {
				case 0:
					if (rflag) {
						red += delta;
						if (red > 65535) {
							red = 65535;
							rflag = 0;
						}
					} else {
						red -= delta;
						if (red < 655) {
							red = 655;
							rflag = 1;
						}
					}
					break;
				case 1:
					if (gflag) {
						green += delta;
						if (green > 65535) {
							green = 65535;
							gflag = 0;
						}
					} else {
						green -= delta;
						if (green < 655) {
							green = 655;
							gflag = 1;
						}
					}
					break;
				case 2:
					if (bflag) {
						blue += delta;
						if (blue > 65535) {
							blue = 65535;
							bflag = 0;
						}
					} else {
						blue -= delta;
						if (blue < 655) {
							blue = 655;
							bflag = 1;
						}
					}
			}
			xcolor.red = red;
			xcolor.green = green;
			xcolor.blue = blue;
			(void) XAllocColor(display, pcmap, &xcolor);
			gcv.foreground = xcolor.pixel;
		}
	}

/* try to get a font for clock (preferably large) */
	fontStruct = XLoadQueryFont(display,
	    "-b&h-lucida-bold-r-normal-sans-34-240-100-100-p-216-iso8859-1");
	if (fontStruct == NULL)
		fontStruct = XLoadQueryFont(display,
					    "-adobe-helvetica-bold-r-normal--34-240-100-100-p-182-iso8859-1");
	if (fontStruct == NULL)
		fontStruct = XLoadQueryFont(display,
					    "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1");
	if (fontStruct == NULL)
		fontStruct = XLoadQueryFont(display,
					    "12x24");
	if (fontStruct == NULL)
		fontStruct = XLoadQueryFont(display,
					    "fixed");
	if (fontStruct == NULL) {
		(void) printf("Could not get font.\n");
		exit(1);
	}
	gcv.font = fontStruct->fid;
	gcv.function = GXcopy;
	gct1 = XCreateGC(display, window, (GCForeground | GCBackground |
					   GCFont | GCFunction), &gcv);
	/* gcv.function=GXxor; */
	gcv.foreground = black;
	gcv.background = black;
	gcv.function = GXcopy;
	gct2 = XCreateGC(display, window, (GCForeground | GCBackground |
					   GCFont | GCFunction), &gcv);

/* pick the most "distant" color in the RGB space for clock */
	if (eight_bit) {
		gcv.foreground = MI_PIXEL(mi, (int) (pixcount + 32) % 64);
	} else {
		XGetGCValues(display, gcpt[MI_BATCHCOUNT(mi) / 2], (GCForeground), &gcv);
		xcolor.pixel = gcv.foreground;
		XQueryColor(display, pcmap, &xcolor);

		tmp_color = xcolor.red;
		xcolorpt = &xcolor.red;
		if (tmp_color > xcolor.green) {
			tmp_color = xcolor.green;
			xcolorpt = &xcolor.green;
		}
		if (tmp_color > xcolor.blue) {
			tmp_color = xcolor.blue;
			xcolorpt = &xcolor.blue;
		}
		if (xcolor.red > 32768)
			xcolor.red = 0;
		else
			xcolor.red = 65535;
		if (xcolor.green > 32768)
			xcolor.green = 0;
		else
			xcolor.green = 65535;
		if (xcolor.blue > 32768)
			xcolor.blue = 0;
		else
			xcolor.blue = 65535;
/* this to make sure we don't get a black clock for clock */
		if (tmp_color >= 32768)
			*xcolorpt = 65535;
		(void) XAllocColor(display, pcmap, &xcolor);
		gcv.foreground = xcolor.pixel;

	}
	XChangeGC(display, gct1, (GCForeground), &gcv);
/* XChangeGC(display, gct2, (GCForeground), &gcv);  */

	initclk();
	count = 0;
}


void
draw_polygon(ModeInfo * mi)
{
	/* static long unsigned tmpfirst; */

	/* if(!count) tmpfirst=time(t)-1; */

	MovePoints(MI_BATCHCOUNT(mi), oldlocations, r);
	tmp = oldlocations;
	oldlocations = locations;
	locations = tmp;
	CalcFourPoly(MI_BATCHCOUNT(mi), locations, oldfourpoly);
	tmp = oldfourpoly;
	oldfourpoly = fourpoly;
	fourpoly = tmp;
	AnimPolygons(mi, MI_BATCHCOUNT(mi), fourpoly, oldfourpoly);
	DrawClock(mi);

	count++;
	if (!(count % 30)) {
		ChangeColor(mi, MI_BATCHCOUNT(mi));
		/*   (void) printf("%f tics/sec\n",(float)(count/(time(t)-tmpfirst)));  */
	}
}

void
release_polygon(ModeInfo * mi)
{
}
