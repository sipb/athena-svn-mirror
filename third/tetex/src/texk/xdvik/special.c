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
	This module is based on prior work as noted below.

\*========================================================================*/

/*
 * Support drawing routines for TeXsun and TeX
 *
 *      Copyright, (C) 1987, 1988 Tim Morgan, UC Irvine
 *	Adapted for xdvi by Jeffrey Lee, U. of Toronto
 *
 * At the time these routines are called, the values of hh and vv should
 * have been updated to the upper left corner of the graph (the position
 * the \special appears at in the dvi file).  Then the coordinates in the
 * graphics commands are in terms of a virtual page with axes oriented the
 * same as the Imagen and the SUN normally have:
 *
 *                      0,0
 *                       +-----------> +x
 *                       |
 *                       |
 *                       |
 *                      \ /
 *                       +y
 *
 * Angles are measured in the conventional way, from +x towards +y.
 * Unfortunately, that reverses the meaning of "counterclockwise"
 * from what it's normally thought of.
 *
 * A lot of floating point arithmetic has been converted to integer
 * arithmetic for speed.  In some places, this is kind-of kludgy, but
 * it's worth it.
 */

#include "xdvi-config.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-ctype.h"
#include "kpathsea/c-stat.h"
#include "kpathsea/line.h"
#include "kpathsea/tex-file.h"
#ifndef S_IRUSR
#define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200
#endif
#include "dvi.h"	/* SU 2000/03/09: added */

# if HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
# ifndef WEXITSTATUS
#  define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
# endif
# ifndef WIFEXITED
#  define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
# endif

#define	MAXPOINTS	300	/* Max points in a path */
#define	MAX_PEN_SIZE	7	/* Max pixels of pen width */
#define	TWOPI		(3.14159265359 * 2.0)

extern double floor ARGS((double));
#define	rint(x)	floor((x) + 0.5)

static int xx[MAXPOINTS], yy[MAXPOINTS];	/* Path in milli-inches */
static int path_len = 0;	/* # points in current path */
static int pen_size = 1;	/* Pixel width of lines drawn */

static Boolean whiten = False;
static Boolean shade = False;
static Boolean blacken = False;

/* Unfortunately, these values also appear in dvisun.c */
#define	xRESOLUTION	(pixels_per_inch/currwin.shrinkfactor)
#define	yRESOLUTION	(pixels_per_inch/currwin.shrinkfactor)

# if HAVE_VFORK_H
#  include <vfork.h>
# endif

#define	CMD(x, y)	((x) << 8 | (y))

/*
 *	Issue warning messages
 */

static void
Warning(char *fmt, char *msg)
{
    Fprintf(stderr, "%s: ", prog);
    Fprintf(stderr, fmt, msg);
    (void)fputc('\n', stderr);
}


/*
 *	X drawing routines
 */

#define	toint(x)	((int) ((x) + 0.5))
#define	xconv(x)	(toint(tpic_conv*(x))/currwin.shrinkfactor + PXL_H)
#define	yconv(y)	(toint(tpic_conv*(y))/currwin.shrinkfactor + PXL_V)

/*
 *	Draw a line from (fx,fy) to (tx,ty).
 *	Right now, we ignore pen_size.
 */
static void
line_btw(int fx, int fy, int tx, int ty)
{
    int fcx = xconv(fx);
    int tcx = xconv(tx);
    int fcy = yconv(fy);
    int tcy = yconv(ty);

    if ((fcx < max_x || tcx < max_x) && (fcx >= min_x || tcx >= min_x) &&
	(fcy < max_y || tcy < max_y) && (fcy >= min_y || tcy >= min_y))
	XDrawLine(DISP, currwin.win, ruleGC,
		  fcx - currwin.base_x, fcy - currwin.base_y,
		  tcx - currwin.base_x, tcy - currwin.base_y);
}

/*
 *	Draw a dot at (x,y)
 */
static void
dot_at(int x, int y)
{
    int cx = xconv(x);
    int cy = yconv(y);

    if (cx < max_x && cx >= min_x && cy < max_y && cy >= min_y)
	XDrawPoint(DISP, currwin.win, ruleGC,
		   cx - currwin.base_x, cy - currwin.base_y);
}

/*
 *	Apply the requested attributes to the last path (box) drawn.
 *	Attributes are reset.
 *	(Not currently implemented.)
 */
/* ARGSUSED */
static void
do_attribute_path(int last_min_x, int last_max_x, int last_min_y, int last_max_y)
{
    UNUSED(last_min_x);
    UNUSED(last_min_y);
    UNUSED(last_max_x);
    UNUSED(last_max_y);
}

/*
 *	Set the size of the virtual pen used to draw in milli-inches
 */

/* ARGSUSED */
static void
set_pen_size(char *cp)
{
    int ps;

    if (sscanf(cp, " %d ", &ps) != 1) {
	Warning("invalid .ps command format: %s", cp);
	return;
    }
    pen_size = (ps * (xRESOLUTION + yRESOLUTION) + 1000) / 2000;
    if (pen_size < 1)
	pen_size = 1;
    else if (pen_size > MAX_PEN_SIZE)
	pen_size = MAX_PEN_SIZE;
}


/*
 *	Print the line defined by previous path commands
 */

static void
flush_path(void)
{
    int i;
    int last_min_x, last_max_x, last_min_y, last_max_y;

    last_min_x = 30000;
    last_min_y = 30000;
    last_max_x = -30000;
    last_max_y = -30000;
    for (i = 1; i < path_len; i++) {
	if (xx[i] > last_max_x)
	    last_max_x = xx[i];
	if (xx[i] < last_min_x)
	    last_min_x = xx[i];
	if (yy[i] > last_max_y)
	    last_max_y = yy[i];
	if (yy[i] < last_min_y)
	    last_min_y = yy[i];
	line_btw(xx[i], yy[i], xx[i + 1], yy[i + 1]);
    }
    if (xx[path_len] > last_max_x)
	last_max_x = xx[path_len];
    if (xx[path_len] < last_min_x)
	last_min_x = xx[path_len];
    if (yy[path_len] > last_max_y)
	last_max_y = yy[path_len];
    if (yy[path_len] < last_min_y)
	last_min_y = yy[path_len];
    path_len = 0;
    do_attribute_path(last_min_x, last_max_x, last_min_y, last_max_y);
}


/*
 *	Print a dashed line along the previously defined path, with
 *	the dashes/inch defined.
 */

static void
flush_dashed(char *cp, Boolean dotted)
{
    int i;
    int numdots;
    int lx0, ly0, lx1, ly1;
    int cx0, cy0, cx1, cy1;
    float inchesperdash;
    double d, spacesize, a, b = 0.0, dx, dy, milliperdash;

    if (sscanf(cp, " %f ", &inchesperdash) != 1) {
	Warning("invalid format for dotted/dashed line: %s", cp);
	return;
    }
    if (path_len <= 1 || inchesperdash <= 0.0) {
	Warning("invalid conditions for dotted/dashed line", "");
	return;
    }
    milliperdash = inchesperdash * 1000.0;
    lx0 = xx[1];
    ly0 = yy[1];
    lx1 = xx[2];
    ly1 = yy[2];
    dx = lx1 - lx0;
    dy = ly1 - ly0;
    if (dotted) {
	numdots = sqrt(dx * dx + dy * dy) / milliperdash + 0.5;
	if (numdots == 0)
	    numdots = 1;
	for (i = 0; i <= numdots; i++) {
	    a = (float)i / (float)numdots;
	    cx0 = lx0 + a * dx + 0.5;
	    cy0 = ly0 + a * dy + 0.5;
	    dot_at(cx0, cy0);
	}
    }
    else {
	d = sqrt(dx * dx + dy * dy);
	numdots = d / (2.0 * milliperdash) + 1.0;
	if (numdots <= 1)
	    line_btw(lx0, ly0, lx1, ly1);
	else {
	    spacesize = (d - numdots * milliperdash) / (numdots - 1);
	    for (i = 0; i < numdots - 1; i++) {
		a = i * (milliperdash + spacesize) / d;
		b = a + milliperdash / d;
		cx0 = lx0 + a * dx + 0.5;
		cy0 = ly0 + a * dy + 0.5;
		cx1 = lx0 + b * dx + 0.5;
		cy1 = ly0 + b * dy + 0.5;
		line_btw(cx0, cy0, cx1, cy1);
		b += spacesize / d;
	    }
	    cx0 = lx0 + b * dx + 0.5;
	    cy0 = ly0 + b * dy + 0.5;
	    line_btw(cx0, cy0, lx1, ly1);
	}
    }
    path_len = 0;
}


/*
 *	Add a point to the current path
 */

static void
add_path(char *cp)
{
    int pathx, pathy;

    if (++path_len >= MAXPOINTS)
	oops("Too many points");
    if (sscanf(cp, " %d %d ", &pathx, &pathy) != 2)
	oops("Malformed path command");
    xx[path_len] = pathx;
    yy[path_len] = pathy;
}


/*
 *	Draw to a floating point position
 */

static void
im_fdraw(double x, double y)
{
    if (++path_len >= MAXPOINTS)
	oops("Too many arc points");
    xx[path_len] = x + 0.5;
    yy[path_len] = y + 0.5;
}


/*
 *	Draw an ellipse with the indicated center and radices.
 */

static void
draw_ellipse(int xc, int yc, int xr, int yr)
{
    double angle, theta;
    int n;
    int px0, py0, px1, py1;

    angle = (xr + yr) / 2.0;
    theta = sqrt(1.0 / angle);
    n = TWOPI / theta + 0.5;
    if (n < 12)
	n = 12;
    else if (n > 80)
	n = 80;
    n /= 2;
    theta = TWOPI / n;

    angle = 0.0;
    px0 = xc + xr;	/* cos(0) = 1 */
    py0 = yc;	/* sin(0) = 0 */
    while ((angle += theta) <= TWOPI) {
	px1 = xc + xr * cos(angle) + 0.5;
	py1 = yc + yr * sin(angle) + 0.5;
	line_btw(px0, py0, px1, py1);
	px0 = px1;
	py0 = py1;
    }
    line_btw(px0, py0, xc + xr, yc);
}

/*
 *	Draw an arc
 */

static void
arc(char *cp, Boolean invis)
{
    int xc, yc, xrad, yrad, n;
    float start_angle, end_angle, angle, theta, r;
    double xradius, yradius, xcenter, ycenter;

    if (sscanf(cp, " %d %d %d %d %f %f ", &xc, &yc, &xrad, &yrad,
	       &start_angle, &end_angle) != 6) {
	Warning("invalid arc specification: %s", cp);
	return;
    }

    if (invis)
	return;

    /* We have a specialized fast way to draw closed circles/ellipses */
    if (start_angle <= 0.0 && end_angle >= 6.282) {
	draw_ellipse(xc, yc, xrad, yrad);
	return;
    }
    xcenter = xc;
    ycenter = yc;
    xradius = xrad;
    yradius = yrad;
    r = (xradius + yradius) / 2.0;
    theta = sqrt(1.0 / r);
    n = 0.3 * TWOPI / theta + 0.5;
    if (n < 12)
	n = 12;
    else if (n > 80)
	n = 80;
    n /= 2;
    theta = TWOPI / n;
    flush_path();
    im_fdraw(xcenter + xradius * cos(start_angle),
	     ycenter + yradius * sin(start_angle));
    angle = start_angle + theta;
    if (end_angle < start_angle)
	end_angle += TWOPI;
    while (angle < end_angle) {
	im_fdraw(xcenter + xradius * cos(angle),
		 ycenter + yradius * sin(angle));
	angle += theta;
    }
    im_fdraw(xcenter + xradius * cos(end_angle),
	     ycenter + yradius * sin(end_angle));
    flush_path();
}


/*
 *	APPROXIMATE integer distance between two points
 */

#define	dist(x0, y0, x1, y1)	(abs(x0 - x1) + abs(y0 - y1))


/*
 *	Draw a spline along the previously defined path
 */

static void
flush_spline(void)
{
    int xp, yp;
    int N;
    int lastx = -1, lasty = -1;
    Boolean lastvalid = False;
    int t1, t2, t3;
    int steps;
    int j;
    int i, w;

    N = path_len + 1;
    xx[0] = xx[1];
    yy[0] = yy[1];
    xx[N] = xx[N - 1];
    yy[N] = yy[N - 1];
    for (i = 0; i < N - 1; i++) {	/* interval */
	steps = (dist(xx[i], yy[i], xx[i + 1], yy[i + 1]) +
		 dist(xx[i + 1], yy[i + 1], xx[i + 2], yy[i + 2])) / 80;
	for (j = 0; j < steps; j++) {	/* points within */
	    w = (j * 1000 + 500) / steps;
	    t1 = w * w / 20;
	    w -= 500;
	    t2 = (750000 - w * w) / 10;
	    w -= 500;
	    t3 = w * w / 20;
	    xp =
		(t1 * xx[i + 2] + t2 * xx[i + 1] + t3 * xx[i] + 50000) / 100000;
	    yp =
		(t1 * yy[i + 2] + t2 * yy[i + 1] + t3 * yy[i] + 50000) / 100000;
	    if (lastvalid)
		line_btw(lastx, lasty, xp, yp);
	    lastx = xp;
	    lasty = yp;
	    lastvalid = True;
	}
    }
    path_len = 0;
}


/*
 *	Shade the last box, circle, or ellipse
 */

static void
shade_last(void)
{
    blacken = whiten = False;
    shade = True;
}


/*
 *	Make the last box, circle, or ellipse, white inside (shade with white)
 */

static void
whiten_last(void)
{
    whiten = True;
    blacken = shade = False;
}


/*
 *	Make last box, etc, black inside
 */

static void
blacken_last(void)
{
    blacken = True;
    whiten = shade = False;
}


/*
 *	Code for PostScript<tm> specials begins here.
 */

#if	PS

/*
 *	Information on how to search for PS header and figure files.
 */

static void ps_startup ARGS((int, int, _Xconst char *));
static void ps_startup2 ARGS((void));
void NullProc
ARGS((void))
{
}


/* ARGSUSED */
static void NullProc2 ARGS((_Xconst char *));

struct psprocs psp = {	/* used for lazy startup of the ps machinery */
    /* toggle */ NullProc,
    /* destroy */ NullProc,
    /* interrupt */ NullProc,
    /* endpage */ NullProc,
    /* drawbegin */ ps_startup,
    /* drawraw */ NullProc2,
    /* drawfile */ NULL,
    /* drawend */ NullProc2,
    /* beginheader */ ps_startup2,
    /* endheader */ NullProc,
    /* newdoc */ NullProc
};

struct psprocs no_ps_procs = {	/* used if postscript is unavailable */
    /* toggle */ NullProc,
    /* destroy */ NullProc,
    /* interrupt */ NullProc,
    /* endpage */ NullProc,
    /* drawbegin */ drawbegin_none,
    /* drawraw */ NullProc2,
    /* drawfile */ NULL,
    /* drawend */ NullProc2,
    /* beginheader */ NullProc,
    /* endheader */ NullProc,
    /* newdoc */ NullProc
};

#endif /* PS */

static Boolean bbox_valid;
static unsigned int bbox_width;
static unsigned int bbox_height;
static int bbox_angle;
static int bbox_voffset;

void
draw_bbox(void)
{
    int xcorner, ycorner;

    if (bbox_valid) {

	xcorner = PXL_H - currwin.base_x;
	ycorner = PXL_V - currwin.base_y;

	if (bbox_angle == 0) {
	    ycorner -= bbox_voffset;
	    XDrawRectangle(DISP, currwin.win, ruleGC, xcorner, ycorner, bbox_width, bbox_height);
/* 	    XDrawRectangle(DISP, currwin.win, ruleGC, xcorner - 1, ycorner - 1, bbox_width + 2, bbox_height + 2); */
/* 	    XDrawLine(DISP, currwin.win, ruleGC, */
/* 		      xcorner, ycorner, xcorner + bbox_width, ycorner); */
/* 	    XDrawLine(DISP, currwin.win, ruleGC, */
/* 		      xcorner + bbox_width, ycorner, */
/* 		      xcorner + bbox_width, ycorner + bbox_height); */
/* 	    XDrawLine(DISP, currwin.win, ruleGC, */
/* 		      xcorner + bbox_width, ycorner + bbox_height, */
/* 		      xcorner, ycorner + bbox_height); */
/* 	    XDrawLine(DISP, currwin.win, ruleGC, */
/* 		      xcorner, ycorner + bbox_height, xcorner, ycorner); */
	}
	else {
	    float sin_a = sin(bbox_angle * (TWOPI / 360));
	    float cos_a = cos(bbox_angle * (TWOPI / 360));
	    float a, b, c, d;

	    a = cos_a * bbox_width;
	    b = -sin_a * bbox_width;
	    c = -sin_a * bbox_height;
	    d = -cos_a * bbox_height;

	    XDrawLine(DISP, currwin.win, ruleGC,
		      xcorner, ycorner,
		      xcorner + (int)rint(a), ycorner + (int)rint(b));
	    XDrawLine(DISP, currwin.win, ruleGC,
		      xcorner + (int)rint(a), ycorner + (int)rint(b),
		      xcorner + (int)rint(a + c), ycorner + (int)rint(b + d));
	    XDrawLine(DISP, currwin.win, ruleGC,
		      xcorner + (int)rint(a + c), ycorner + (int)rint(b + d),
		      xcorner + (int)rint(c), ycorner + (int)rint(d));
	    XDrawLine(DISP, currwin.win, ruleGC,
		      xcorner + (int)rint(c), ycorner + (int)rint(d),
		      xcorner, ycorner);
	}
	bbox_valid = False;
    }
}

#if	PS

static XIOErrorHandler oldhandler;

static int
XDviIOErrorHandler(Display *disp)
{
    ps_destroy();
    return oldhandler(disp);
}

static void
actual_startup(void)
{

    oldhandler = XSetIOErrorHandler(XDviIOErrorHandler);

    /*
     * Figure out what we want to use to display postscript figures
     * and set at most one of the following to True:
     * resource.useGS, resource.useDPS, resource.useNeWS
     *
     * Choose DPS then NEWS then Ghostscript if they are available
     */
    if (!(
#ifdef	PS_DPS
	     (resource.useDPS && initDPS())
#if	defined(PS_NEWS) || defined(PS_GS)
	     ||
#endif
#endif /* PS_DPS */
#ifdef	PS_NEWS
	     (resource.useNeWS && initNeWS())
#ifdef	PS_GS
	     ||
#endif
#endif /* PS_NEWS */
#ifdef	PS_GS
	     (resource.useGS && initGS())
#endif
	))
	psp = no_ps_procs;
}

static void
ps_startup(int xul, int yul, _Xconst char *cp)
{
    if (!resource._postscript) {
	psp.toggle = actual_startup;
	draw_bbox();
	return;
    }
    actual_startup();
    psp.drawbegin(xul, yul, cp);
}

static void
ps_startup2(void)
{
    actual_startup();
    psp.beginheader();
}

/* ARGSUSED */
static void
NullProc2(_Xconst char *cp)
{
    UNUSED(cp);
}

static void
ps_parseraw(_Xconst char *PostScript_cmd)
{
    _Xconst char *p;

    /* dumb parsing of PostScript - search for rotation H. Zeller 1/97 */
    bbox_angle = 0;
    p = strstr(PostScript_cmd, "rotate");
    if (p != NULL) {
	while (*p != '\0' && !isdigit(*p))
	    --p;
	while (*p != '\0' && isdigit(*p))
	    --p;
	if (*p != '+' && *p != '-')
	    ++p;
	sscanf(p, "%d neg rotate", &bbox_angle);
    }
}


/* ARGSUSED */
void
drawbegin_none(int xul, int yul, _Xconst char *cp)
{
    UNUSED(xul);
    UNUSED(yul);
    UNUSED(cp);
    
    draw_bbox();
}


struct tickrec {
    struct tickrec *next;
    int pageno;
    char *command;
    char *tempname;
};

static struct tickrec *tickhead = NULL;	/* head of linked list of */
						/* cached information */
static int nticks = 0;	/* number of records total */

#ifndef	TICKCACHESIZE
#define	TICKCACHESIZE	3
#endif

#ifndef	TICKTMP
#define	TICKTMP		"/tmp"
#endif

/*
 *	cachetick() - returns:
 *		NULL		error;
 *		fp == NULL	string was not in cache, fd = file
 *		fp != NULL	string was in cache, fp = cached file
 */

static struct tickrec *
cachetick(_Xconst char *filename, kpse_file_format_type pathinfo, FILE **fp, int *fdp)
{
    struct tickrec **linkp;
    struct tickrec *tikp;
    struct tickrec **freerecp;

    linkp = &tickhead;
    freerecp = NULL;
    for (;;) {	/* see if we have it already */
	tikp = *linkp;
	if (tikp == NULL) {	/* if end of list */
	    int fd;

	    if (nticks >= TICKCACHESIZE && freerecp != NULL) {
		tikp = *freerecp;
		*freerecp = tikp->next;
		free(tikp->command);
		unlink(tikp->tempname);
		/* reuse tikp and tikp->tempname */
	    }
	    else {
		tikp = xmalloc(sizeof(struct tickrec));
		tikp->tempname = NULL;
		++nticks;
	    }
	    fd = xdvi_temp_fd(&tikp->tempname);
	    if (fd == -1) {
		perror("Cannot create temporary file");
		free(tikp->tempname);
		free(tikp);
		return NULL;
	    }
	    tikp->command = xstrdup(filename);
	    *fp = NULL;
	    *fdp = fd;
	    break;
	}
	if (strcmp(filename, tikp->command) == 0) {	/* found it */
	    *linkp = tikp->next;	/* unlink it */
	    *fp = xfopen(tikp->tempname, OPEN_MODE);
	    if (*fp == NULL) {
		perror(tikp->tempname);
		free(tikp->tempname);
		free(tikp->command);
		free(tikp);
		return NULL;
	    }
	    break;
	}
	if (tikp->pageno != current_page)
	    freerecp = linkp;
	linkp = &tikp->next;
    }
    tikp->next = tickhead;	/* link it in */
    tickhead = tikp;
    tikp->pageno = pathinfo != kpse_tex_ps_header_format ? current_page : -1;
    return tikp;
}

void
ps_clear_cache(void)
{
    struct tickrec *tikp;

    while (tickhead != NULL) {
	tikp = tickhead;
	tickhead = tickhead->next;
	free(tikp->command);
	unlink(tikp->tempname);
	free((char *)tikp->tempname);
	free(tikp);
    }
    nticks = 0;
}

#ifndef	UNCOMPRESS
#define	UNCOMPRESS	"uncompress"
#endif

#ifndef	GUNZIP
#define	GUNZIP		"gunzip"
#endif

#ifndef	BUNZIP2
#define	BUNZIP2		"bunzip2"
#endif

static void
send_ps_file(_Xconst char *filename, kpse_file_format_type pathinfo)
{
    FILE *f;
    int fd;
    static _Xconst char *argv[] = { NULL, "-c", NULL, NULL };
    char *bufp = NULL;
    struct tickrec *volatile tikp;
    size_t len;
    char magic1, magic2, magic3;
    static Boolean warned_about_shellescape = False;

    if (psp.drawfile == NULL || !resource._postscript)
	return;

#ifdef HTEX

    filename = urlocalize((char *)filename);

    if (((URLbase != NULL) && htex_is_url(urlocalize(URLbase))) ||
	(htex_is_url(filename))) {
	/* xfopen knows how to fetch url things */
	URL_aware = 1;
	f = xfopen(filename, OPEN_MODE);
	bufp = htex_file_at_index(lastwwwopen);
	URL_aware = 0;
    }
    else
#endif
    if (filename[0] == '`') {
	if (!resource.allow_shell) {
	    if (!warned_about_shellescape) {
		do_popup_message(MSG_WARN,
				 "To enable shell escapes, use the \"-allowshell\" command \
line option.\n\nIMPORTANT NOTICE: You should NOT use shell escapes like\n`gunzip -c file.eps.gz\n\
to uncompress .eps.gz files, even though some obsolete LaTeX documentation \
seems to suggest it. Current versions of xdvi and dvips can handle .eps.gz files \
themselves, without any shell interaction.",
				 "Shell escape disallowed for special \"%s\"",
				 filename);
		warned_about_shellescape = True;
	    }
	    else {
		print_statusline(STATUS_MEDIUM,
				 "Warning: shell escape disallowed for special \"%s\"",
				 filename);
	    }
	    /* FIXME: Couldn't we try to display the .eps file anyway,
	       even if the shell escape can't be executed?
	       (Note however that `filename' only contains the shell escape).
	    */
	    return;
	}

	tikp = cachetick(filename, pathinfo, &f, &fd);
	if (tikp == NULL)	/* if error */
	    return;
	if (f == NULL) {	/* if not cached, need to create */
	    close(fd);
	    ++n_files_left;
	    len = strlen(filename) + strlen(tikp->tempname) + (4 - 1);
	    if (len > ffline_len)
		expandline(len);
	    Sprintf(ffline, "%s > %s", filename + 1, tikp->tempname);
	    (void)system(ffline);
	    f = xfopen(tikp->tempname, OPEN_MODE);
	    if (f == NULL) {
		perror(tikp->tempname);
		return;
	    }
	}
	bufp = tikp->tempname;
    }
    else {
	f = NULL;
	/* first try the same path as the dvi file */
	/* FIXME: make this, and src_find_file from dvi-draw.c, a single
	   function that can be used in both cases!
	*/
	if (filename[0] != DIR_SEPARATOR) {
	    char *p = rindex(global_dvi_name, DIR_SEPARATOR);
	    if (p == NULL) {
		bufp = (char *)filename;
	    }
	    else {
		unsigned len1, len2;
		
		len1 = ++p - global_dvi_name;
		len2 = strlen(filename) + 1;
		if (len1 + len2 > ffline_len)
		    expandline(len1 + len2);
		bcopy(global_dvi_name, ffline, len1);
		bcopy(filename, ffline + len1, len2);
		bufp = ffline;
	    }
	    if (debug & DBG_OPEN) Printf("Trying PS file %s\n", bufp);
	    f = bufp ? xfopen(bufp, OPEN_MODE) : NULL;
	}
	if (f == NULL) {
	    bufp = kpse_find_file(filename, pathinfo, true);
	    f = bufp ? xfopen(bufp, OPEN_MODE) : NULL;
	}
	/* if still no luck, complain */
	if (f == NULL) {
	    print_statusline(STATUS_MEDIUM, "Cannot find PS file \"%s\"",
			     filename);
	    draw_bbox();
	    return;
	}

	/* check for compressed files */
	len = strlen(filename);
	magic1 = '\037';
	magic3 = '\0';
	if ((len > 2 && strcmp(filename + len - 2, ".Z") == 0
	     && (argv[0] = UNCOMPRESS, magic2 = '\235', True))
	    || (len > 3 && strcmp(filename + len - 3, ".gz") == 0
		&& (argv[0] = GUNZIP, magic2 = '\213', True))
	    || (len > 4 && strcmp(filename + len - 4, ".bz2") == 0
		&& (argv[0] = BUNZIP2, magic1 = 'B', magic2 = 'Z',
		    magic3 = 'h', True))) {
	    if (getc(f) != magic1 || (char)getc(f) != magic2
		|| (magic3 != '\0' && getc(f) != magic3))
		rewind(f);
	    else {
		Fclose(f);
		++n_files_left;
		tikp = cachetick(filename, pathinfo, &f, &fd);
		if (tikp == NULL)	/* if error */
		    return;
		if (f == NULL) {	/* if not cached, need to create */
		    pid_t pid;
		    int status;

		    argv[2] = bufp;
		    Fflush(stderr);	/* avoid double flushing */
		    pid = vfork();
		    if (pid == 0) {	/* if child */
			(void)dup2(fd, 1);
			(void)execvp(argv[0], (char **)argv);
			Fprintf(stderr, "Execvp of %s failed.\n", argv[0]);
			Fflush(stderr);
			_exit(1);
		    }
		    (void)close(fd);
		    ++n_files_left;
		    for (;;) {
#if HAVE_WAITPID
			if (waitpid(pid, &status, 0) != -1)
			    break;
#else
# if HAVE_WAIT4
			if (wait4(pid, &status, 0, (struct rusage *)NULL)
			    != -1)
			    break;
# else
			int retval;

			retval = wait(&status);
			if (retval == pid)
			    break;
			if (retval != -1)
			    continue;
# endif	/* HAVE_WAIT4 */
#endif /* HAVE_WAITPID */
			if (errno == EINTR)
			    continue;
			perror("[xdvik] waitpid");
			return;
		    }
		    f = xfopen(tikp->tempname, OPEN_MODE);
		    if (f == NULL) {
			perror(tikp->tempname);
			return;
		    }
		}
		bufp = tikp->tempname;
	    }
	}
    }

    /* Success! */
    psp.drawfile(bufp, f);	/* this is supposed to fclose f */
}


void
ps_newdoc(void)
{
    struct tickrec *tikp;

    scanned_page = scanned_page_bak = scanned_page_reset =
	resource.prescan ? -1 : total_pages;
    for (tikp = tickhead; tikp != NULL; tikp = tikp->next)
	tikp->pageno = -1;
    psp.newdoc();
}


void
ps_destroy(void)
{
    struct tickrec *tikp;

    /* Note:  old NeXT systems (at least) lack atexit/on_exit.  */
    psp.destroy();
    for (tikp = tickhead; tikp != NULL; tikp = tikp->next)
	if (unlink(tikp->tempname) < 0)
	    perror(tikp->tempname);
}

#endif /* PS */


static void
psfig_special(char *cp)
{
    char *filename;
    int raww, rawh;

    if (strncmp(cp, ":[begin]", 8) == 0) {
	cp += 8;
	bbox_valid = False;
	bbox_angle = 0;
	if (sscanf(cp, "%d %d\n", &raww, &rawh) >= 2) {
	    bbox_valid = True;
	    bbox_width = pixel_conv(spell_conv(raww));
	    bbox_height = pixel_conv(spell_conv(rawh));
	    bbox_voffset = 0;
	}
	if (currwin.win == mane.win)
#if	PS
	    psp.drawbegin(PXL_H - currwin.base_x, PXL_V - currwin.base_y, cp);
#else
	    draw_bbox();
#endif
	psfig_begun = True;
    }
    else if (strncmp(cp, " plotfile ", 10) == 0) {
	cp += 10;
	while (isspace(*cp))
	    cp++;
	/* handle "`zcat file". Borrowed from dvipsk... */
	if (*cp == '"') {
	    cp++;
	    for (filename = cp; *cp && (*cp != '"'); ++cp);
	}
	else {
	    for (filename = cp; *cp && !isspace(*cp); ++cp);
	}
	*cp = '\0';
#if	PS
	if (currwin.win == mane.win)
	    send_ps_file(filename, kpse_pict_format);
#endif
    }
    else if (strncmp(cp, ":[end]", 6) == 0) {
	cp += 6;
#if	PS
	if (currwin.win == mane.win)
	    psp.drawend(cp);
#endif
	bbox_valid = False;
	psfig_begun = False;
    }
    else if (*cp == ':') {
	/* I am going to send some raw postscript stuff */
	++cp;	/* skip the colon */
#if	PS
	ps_parseraw(cp);
	if (currwin.win == mane.win)
	    psp.drawraw(cp);
#endif
    }
    else {
	/* also raw PostScript, but no extra colon to skip */
#if	PS
	if (currwin.win == mane.win) {
	    ps_parseraw(cp);
	    if (psfig_begun)
		psp.drawraw(cp);
	    else {
		psp.drawbegin(PXL_H - currwin.base_x,
			      PXL_V - currwin.base_y, cp);
		psp.drawend("");
	    }
	}
#endif
    }
}


/*	Keys for epsf specials */

static _Xconst char *keytab[] = { "clip",
    "llx",
    "lly",
    "urx",
    "ury",
    "rwi",
    "rhi",
    "hsize",
    "vsize",
    "hoffset",
    "voffset",
    "hscale",
    "vscale",
    "angle"
};

#define	KEY_LLX	keyval[0]
#define	KEY_LLY	keyval[1]
#define	KEY_URX	keyval[2]
#define	KEY_URY	keyval[3]
#define	KEY_RWI	keyval[4]
#define	KEY_RHI	keyval[5]

#define	NKEYS	(sizeof(keytab)/sizeof(*keytab))
#define	N_ARGLESS_KEYS 1

static void
epsf_special(char *cp)
{
    char *filename;
    static char *buffer;
    static unsigned int buflen = 0;
    unsigned int len;
    char *q;
    int flags = 0;
    double keyval[6];

    filename = cp;
    if (*cp == '\'' || *cp == '"') {
	do
	    ++cp;
	while (*cp != '\0' && *cp != *filename);
	++filename;
    }
    else
	while (*cp != '\0' && *cp != ' ' && *cp != '\t')
	    ++cp;
    if (*cp != '\0')
	*cp++ = '\0';
    while (*cp == ' ' || *cp == '\t')
	++cp;
    len = strlen(cp) + NKEYS + 30;
    if (buflen < len) {
	if (buflen != 0)
	    free(buffer);
	buflen = len;
	buffer = xmalloc(buflen);
    }
    Strcpy(buffer, "@beginspecial");
    q = buffer + strlen(buffer);
    while (*cp != '\0') {
	char *p1 = cp;
	size_t keyno;

	while (*p1 != '=' && !isspace(*p1) && *p1 != '\0')
	    ++p1;
	for (keyno = 0;; ++keyno) {
	    if (keyno >= NKEYS) {
		if (warn_spec_now)
		    Fprintf(stderr,
			    "%s: unknown keyword (%.*s) in \\special will be ignored\n",
			    prog, (int)(p1 - cp), cp);
		break;
	    }
	    if (memcmp(cp, keytab[keyno], p1 - cp) == 0) {
		if (keyno >= N_ARGLESS_KEYS) {
		    while (isspace(*p1))
			++p1;
		    if (*p1 == '=') {
			++p1;
			while (isspace(*p1))
			    ++p1;
		    }
		    if (keyno < N_ARGLESS_KEYS + 6) {
			keyval[keyno - N_ARGLESS_KEYS] = atof(p1);
			flags |= (1 << (keyno - N_ARGLESS_KEYS));
		    }
		    *q++ = ' ';
		    while (!isspace(*p1) && *p1 != '\0')
			*q++ = *p1++;
		}
		*q++ = ' ';
		*q++ = '@';
		Strcpy(q, keytab[keyno]);
		q += strlen(q);
		break;
	    }
	}
	cp = p1;
	while (!isspace(*cp) && *cp != '\0')
	    ++cp;
	while (isspace(*cp))
	    ++cp;
    }
    Strcpy(q, " @setspecial\n");

    bbox_valid = False;
    if ((flags & 0x30) == 0x30 || ((flags & 0x30) && (flags & 0xf) == 0xf)) {
	bbox_valid = True;
	bbox_width = 0.1 * ((flags & 0x10) ? KEY_RWI
			    : KEY_RHI * (KEY_URX - KEY_LLX) / (KEY_URY -
							       KEY_LLY)) *
	    dimconv / currwin.shrinkfactor + 0.5;
	bbox_voffset = bbox_height =
	    0.1 * ((flags & 0x20) ? KEY_RHI : KEY_RWI * (KEY_URY - KEY_LLY) /
		   (KEY_URX - KEY_LLX))
	    * dimconv / currwin.shrinkfactor + 0.5;
    }

    if (currwin.win == mane.win) {
#if	PS
	psp.drawbegin(PXL_H - currwin.base_x, PXL_V - currwin.base_y, buffer);
	/* talk directly with the DPSHandler here */
	send_ps_file(filename, kpse_pict_format);
	psp.drawend(" @endspecial");
#else
	draw_bbox();
#endif
    }
    bbox_valid = False;
}


static void
quote_special(char *cp)
{
    bbox_valid = False;

#if	PS
    if (currwin.win == mane.win) {
	psp.drawbegin(PXL_H - currwin.base_x, PXL_V - currwin.base_y,
		      "@beginspecial @setspecial ");
	/* talk directly with the DPSHandler here */
	psp.drawraw(cp + 1);
	psp.drawend(" @endspecial");
    }
#endif

    /* nothing else to do--there's no bbox here */
}

#if	PS

static void
scan_header(char *cp)
{
    char *filename;

    filename = cp;
    if (*cp == '\'' || *cp == '"') {
	do
	    ++cp;
	while (*cp != '\0' && *cp != *filename);
	*cp = '\0';
	++filename;
    }

    psp.beginheader();
    send_ps_file(filename, kpse_tex_ps_header_format);
}

static void
scan_bang(char *cp)
{
    psp.beginheader();
    psp.drawraw(cp + 1);
}

#endif /* PS */

/*
 *	The following copyright message applies to the rest of this file.  --PV
 */

/*
 *	This program is Copyright (C) 1987 by the Board of Trustees of the
 *	University of Illinois, and by the author Dirk Grunwald.
 *
 *	This program may be freely copied, as long as this copyright
 *	message remaines affixed. It may not be sold, although it may
 *	be distributed with other software which is sold. If the
 *	software is distributed, the source code must be made available.
 *
 *	No warranty, expressed or implied, is given with this software.
 *	It is presented in the hope that it will prove useful.
 *
 *	Hacked in ignorance and desperation by jonah@db.toronto.edu
 */

/*
 *      The code to handle the \specials generated by tpic was modified
 *      by Dirk Grunwald using the code Tim Morgan at Univ. of Calif, Irvine
 *      wrote for TeXsun.
 */

static char *
endofcommand(char *cp)
{
    while (isspace(*cp))
	++cp;
    if (*cp != '=')
	return NULL;
    do
	++cp;
    while (isspace(*cp));
    return cp;
}

#define	CMD(x, y)	((x) << 8 | (y))

void
applicationDoSpecial(char *cp)
{
    char *p;

    /* Skip white space */
    while (*cp == ' ' || *cp == '\t')
	++cp;

    /* PostScript specials */

    if (*cp == '"') {
	quote_special(cp);
	return;
    }
    if (memicmp(cp, "ps:", 3) == 0) {
	psfig_special(cp + 3);
	return;
    }
    if (memicmp(cp, "psfile", 6) == 0 && (p = endofcommand(cp + 6)) != NULL) {
	epsf_special(p);
	return;
    }

    /* these should have been scanned */

    if (*cp == '!'
	|| (memicmp(cp, "header", 6) == 0 && endofcommand(cp + 6) != NULL)) {
#if PS
	if (resource._postscript && scanned_page_reset >= 0) {
	    /* turn on scanning and redraw the page */
	    scanned_page = scanned_page_bak = scanned_page_reset = -1;
	    psp.interrupt();
	    canit = True;
	    longjmp(canit_env, 1);
	}
#endif
	return;
    }

#ifdef HTEX
    if (checkHyperTeX(cp, current_page))
	return;
#endif

    /* tpic specials */

    if (*cp >= 'a' && *cp <= 'z' && cp[1] >= 'a' && cp[1] <= 'z' &&
	(isspace(cp[2]) || cp[2] == '\0')) {
	switch (CMD(*cp, cp[1])) {
	case CMD('p', 'n'):
	    set_pen_size(cp + 2);
	    return;
	case CMD('f', 'p'):
	    flush_path();
	    return;
	case CMD('d', 'a'):
	    flush_dashed(cp + 2, False);
	    return;
	case CMD('d', 't'):
	    flush_dashed(cp + 2, True);
	    return;
	case CMD('p', 'a'):
	    add_path(cp + 2);
	    return;
	case CMD('a', 'r'):
	    arc(cp + 2, False);
	    return;
	case CMD('i', 'a'):
	    arc(cp + 2, True);
	    return;
	case CMD('s', 'p'):
	    flush_spline();
	    return;
	case CMD('s', 'h'):
	    shade_last();
	    return;
	case CMD('w', 'h'):
	    whiten_last();
	    return;
	case CMD('b', 'k'):
	    blacken_last();
	    return;
	case CMD('i', 'p'):	/* throw away the path -- jansteen */
	    path_len = 0;
	    return;
	}
    }

    if (warn_spec_now && memcmp(cp, "src:", 4) != 0)
	Fprintf(stderr, "%s:  special \"%s\" not implemented\n", prog, cp);
}

#undef	CMD

#if	PS
void
scan_special(char *cp)
{
    char *p;

    /* Skip white space */
    while (*cp == ' ' || *cp == '\t')
	++cp;

    if (debug & DBG_PS)
	Printf("Scanning special `%s'.\n", cp);

	    if (*cp == '!')
		scan_bang(cp);
	    else if (memicmp(cp, "header", 6) == 0
		     && (p = endofcommand(cp + 6)) != NULL) scan_header(p);
}
#endif /* PS */


#define	xspell_conv(n)	spell_conv0(n, current_dimconv)
#define	xpixel_conv(x)	((int) ((x) >> 16))
#define	G_PXL_H		xpixel_conv(currinf.data.dvi_h)

/* ARGSUSED */
void
geom_do_special(struct geom_info *g_info, char *cp, double current_dimconv)
{
    _Xconst char *p;
    UNUSED(current_dimconv);
    
    if (memicmp(cp, "psfile", 6) == 0 && (p = endofcommand(cp + 6)) != NULL) {
	/* compute epsf bounding box */
	char c;
	int flags = 0;
	double keyval[6];

	c = *p;
	if (c == '\'' || c == '"') {
	    do
		++p;
	    while (*p != '\0' && *p != c);
	}
	else
	    while (*p != '\0' && *p != ' ' && *p != '\t')
		++p;
	while (*p == ' ' || *p == '\t')
	    ++p;
	while (*p != '\0') {
	    _Xconst char *p1 = p;
	    size_t keyno;

	    while (*p1 != '=' && !isspace(*p1) && *p1 != '\0')
		++p1;
	    for (keyno = 0; keyno < NKEYS; ++keyno) {
		if (memcmp(p, keytab[keyno], p1 - p) == 0) {
		    if (keyno >= N_ARGLESS_KEYS) {
			while (isspace(*p1))
			    ++p1;
			if (*p1 == '=') {
			    ++p1;
			    while (isspace(*p1))
				++p1;
			}
			if (keyno < N_ARGLESS_KEYS + 6) {
			    keyval[keyno - N_ARGLESS_KEYS] = atof(p1);
			    flags |= (1 << (keyno - N_ARGLESS_KEYS));
			}
			while (!isspace(*p1) && *p1 != '\0')
			    ++p1;
		    }
		    break;
		}
	    }
	    p = p1;
	    while (!isspace(*p) && *p != '\0')
		++p;
	    while (isspace(*p))
		++p;
	}

	if ((flags & 0x30) == 0x30 || ((flags & 0x30) && (flags & 0xf) == 0xf)) {
	    long x = G_PXL_H;
	    long y = PXL_V;
	    long bbox_w;
	    long bbox_h;

	    bbox_w = 0.1 * ((flags & 0x10) ? KEY_RWI
			    : KEY_RHI * (KEY_URX - KEY_LLX) / (KEY_URY -
							       KEY_LLY)) *
		dimconv + 0.5;
	    bbox_h =
		0.1 * ((flags & 0x20) ? KEY_RHI : KEY_RWI *
		       (KEY_URY - KEY_LLY) / (KEY_URX - KEY_LLX))
		* dimconv + 0.5;

	    g_info->geom_box(g_info, x, y - bbox_h, x + bbox_w, y);
	}
    }
    else if (memicmp(cp, "ps::[begin]", 11) == 0) {
	/* compute psfig bounding box */
	long bbox_w, bbox_h;

	if (sscanf(cp + 11, "%ld %ld\n", &bbox_w, &bbox_h) >= 2) {
	    long x = G_PXL_H;
	    long y = PXL_V;

	    bbox_w = xpixel_conv(spell_conv(bbox_w));
	    bbox_h = xpixel_conv(spell_conv(bbox_h));

	    g_info->geom_box(g_info, x, y, x + bbox_w, y + bbox_h);
	}
    }
}
