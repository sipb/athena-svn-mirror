
#ifndef lint
static char sccsid[] = "@(#)wire.c	3.11 96/09/20 xlockmore";

#endif

/*-
 * wire.c - logical circuits based on simple state-changes (wireworld)
 *           for the X Window System lockscreen.
 *
 * Copyright (c) 1996 by David Bagley.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 14-Jun-96: Coded from A.K. Dewdney's "Computer Recreations", Scientific
 *            American Magazine" Jan 1990 pp 146-148.  Used ant.c as an
 *            example.  do_gen() based on code by Kevin Dahlhausen
 *            <ap096@po.cwru.edu> and Stefan Strack
 *            <stst@vuse.vanderbilt.edu>.
 */

/*-
 * OR gate is protected by diodes
 *           XX     XX
 * Input ->XXX XX XX XXX<- Input
 *           XX  X  XX
 *               X
 *               X
 *               |
 *               V
 *             Output
 *
 *   ->XXXoOXXX-> Electron moving in a wire
 *
 * memory element, about to forget 1 and remember 0
 *         Remember 0
 *             o
 *            X O XX XX   Memory Loop
 *            X  XX X XXXX
 *            X   XX XX   X oOX-> 1 Output
 * Inputs ->XX        X    X
 *                X   X XX o
 * Inputs ->XXXXXXX XX X XO Memory of 1
 *                XX    XX
 *         Remember 1
 */
#include "xlock.h"

#define BITS(n,w,h)\
  wp->pixmaps[wp->init_bits++]=\
  XCreatePixmapFromBitmapData(display,window,(char *)n,w,h,1,0,1)

ModeSpecOpt wire_opts =
{0, NULL, 0, NULL, NULL};

#define COLORS 4
#define MINWIRES 1
#define PATTERNSIZE 8
#define MINGRIDSIZE 24
#define MINSIZE 1

static unsigned char patterns[COLORS - 1][PATTERNSIZE] =
{
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},	/* black */
	{0x00, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66, 0x00},	/* spots */
	{0xff, 0x99, 0x99, 0xff, 0xff, 0x99, 0x99, 0xff}	/* spots */
};

#define PATHDIRS 4
static int  prob_array[PATHDIRS] =
{75, 85, 90, 100};

#define SPACE 0
#define WIRE 1			/* Normal wire */
#define HEAD 2			/* electron head */
#define TAIL 3			/* electron tail */

#define REDRAWSTEP 2000		/* How much wire to draw per cycle */

/* Singly linked list */
typedef struct _WireList {
	XPoint      pt;
	struct _WireList *next;
} WireList;

typedef struct {
	int         init_bits;
	int         generation;
	int         xs, ys;
	int         xb, yb;
	int         nrows, ncols;
	int         bnrows, bncols;
	int         width, height;
	int         n;
	int         redrawing, redrawpos;
	unsigned char *world, *worldnew;
	unsigned char colors[COLORS - 1];
	int         ncells[COLORS - 1];
	WireList   *wirelist[COLORS - 1];
	GC          stippled_GC;
	Pixmap      pixmaps[COLORS - 1];
} circuitstruct;

static circuitstruct *circuits = NULL;

static void
addtolist(ModeInfo * mi, int col, int row, unsigned char state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	WireList   *current;

	current = wp->wirelist[state];
	wp->wirelist[state] = (WireList *) malloc(sizeof (WireList));
	wp->wirelist[state]->pt.x = col;
	wp->wirelist[state]->pt.y = row;
	wp->wirelist[state]->next = current;
	wp->ncells[state]++;
}

#ifdef DEBUG
static void
print_state(ModeInfo * mi, int state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	WireList   *locallist;
	int         i = 0;

	locallist = wp->wirelist[state];
	(void) printf("state %d\n", state);
	while (locallist) {
		(void) printf("%d x %d, y %d\n", i,
			      locallist->pt.x, locallist->pt.y);
		locallist = locallist->next;
		i++;
	}
}

#endif

static void
free_state(circuitstruct * wp, int state)
{
	WireList   *current;

	while (wp->wirelist[state]) {
		current = wp->wirelist[state];
		wp->wirelist[state] = wp->wirelist[state]->next;
		(void) free((void *) current);
	}
	wp->wirelist[state] = NULL;
	wp->ncells[state] = 0;
}

static void
drawcell(ModeInfo * mi, int col, int row, unsigned char state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	XGCValues   gcv;
	GC          gc;

	if (MI_NPIXELS(mi) > 2) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, MI_PIXEL(mi, wp->colors[state]));
	} else {
		gcv.stipple = wp->pixmaps[state];
		gcv.foreground = MI_WIN_WHITE_PIXEL(mi);
		gcv.background = MI_WIN_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), wp->stippled_GC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = wp->stippled_GC;
	}
	XFillRectangle(MI_DISPLAY(mi), MI_WINDOW(mi), MI_GC(mi),
	       wp->xb + wp->xs * col, wp->yb + wp->ys * row, wp->xs, wp->ys);
}

static void
draw_state(ModeInfo * mi, int state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	GC          gc;
	XRectangle *rects;
	XGCValues   gcv;
	WireList   *current;

	if (MI_NPIXELS(mi) > 2) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, MI_PIXEL(mi, wp->colors[state]));
	} else {
		gcv.stipple = wp->pixmaps[state];
		gcv.foreground = MI_WIN_WHITE_PIXEL(mi);
		gcv.background = MI_WIN_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), wp->stippled_GC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = wp->stippled_GC;
	}

	{
		/* Take advantage of XDrawRectangles */
		int         ncells = 0;

		/* Create Rectangle list from part of the wirelist */
		rects = (XRectangle *) malloc(wp->ncells[state] * sizeof (XRectangle));
		current = wp->wirelist[state];
		while (current) {
			rects[ncells].x = wp->xb + current->pt.x * wp->xs;
			rects[ncells].y = wp->yb + current->pt.y * wp->ys;
			rects[ncells].width = wp->xs;
			rects[ncells].height = wp->ys;
			current = current->next;
			ncells++;
		}
		/* Finally get to draw */
		XFillRectangles(MI_DISPLAY(mi), MI_WINDOW(mi), gc, rects, ncells);
		/* Free up rects list and the appropriate part of the wirelist */
		(void) free((void *) rects);
	}
	free_state(wp, state);

	if (wp->redrawing) {
		int         i;

		for (i = 0; i < REDRAWSTEP; i++) {
			if ((*(wp->world + wp->redrawpos))) {
				drawcell(mi, wp->redrawpos % wp->bncols - 1,
					 wp->redrawpos / wp->bncols - 1, *(wp->world + wp->redrawpos) - 1);
			}
			if (++(wp->redrawpos) >= wp->bncols * (wp->bnrows - 1)) {
				wp->redrawing = 0;
				break;
			}
		}
	}
}

#if 0
static void
RandomSoup(circuitstruct * wp)
{
	int         i, j;

	for (j = 1; j < wp->bnrows - 1; j++)
		for (i = 1; i < wp->bncols - 1; i++) {
			*(wp->worldnew + i + j * wp->bncols) =
				(NRAND(100) > wp->n) ? SPACE : (NRAND(4)) ? WIRE : (NRAND(2)) ?
				HEAD : TAIL;
		}
}

#endif


static void
create_path(circuitstruct * wp, int n)
{
	int         col, row;
	int         count = 0;
	int         dir, prob;
	int         nextcol = 0, nextrow = 0, i;

#ifdef RANDOMSTART
	/* Path usually "mushed" in a corner */
	col = NRAND(wp->ncols) + 1;
	row = NRAND(wp->nrows) + 1;
#else
	/* Start from center */
	col = wp->ncols / 2;
	row = wp->nrows / 2;
#endif
	dir = NRAND(PATHDIRS);
	*(wp->worldnew + col + row * wp->bncols) = HEAD;
	while (++count < n) {
		prob = NRAND(prob_array[PATHDIRS - 1]);
		i = 0;
		while (prob > prob_array[i])
			i++;
		dir = (dir + i) % PATHDIRS;
		switch (dir) {
			case 0:
				nextcol = col;
				nextrow = row - 1;
				break;
			case 1:
				nextcol = col + 1;
				nextrow = row;
				if (!NRAND(10))
					nextrow += ((LRAND() & 1) ? -1 : 1);
				break;
			case 2:
				nextcol = col;
				nextrow = row + 1;
				break;
			case 3:
				nextcol = col - 1;
				nextrow = row;
				if (!NRAND(10))
					nextrow += ((LRAND() & 1) ? -1 : 1);
				break;
		}
		if (nextrow > 0 && nextrow < wp->bnrows - 1 &&
		    nextcol > 0 && nextcol < wp->bncols - 1) {
			col = nextcol;
			row = nextrow;
			if (!*(wp->worldnew + col + row * wp->bncols))
				*(wp->worldnew + col + row * wp->bncols) = WIRE;
		} else
			dir = (dir + PATHDIRS / 2) % PATHDIRS;
	}
	*(wp->worldnew + col + row * wp->bncols) = HEAD;
}

static void
do_gen(circuitstruct * wp)
{
	int         x, y;
	unsigned char *z;
	int         count;

#define loc(X, Y) (*(wp->world + (X) + ((Y) * wp->bncols)))
#define add(X, Y) if (loc(X, Y) == HEAD) count++

	for (y = 1; y < wp->bnrows - 1; y++) {
		for (x = 1; x < wp->bncols - 1; x++) {
			z = wp->worldnew + x + y * wp->bncols;
			switch (loc(x, y)) {
				case SPACE:
					*z = SPACE;
					break;
				case TAIL:
					*z = WIRE;
					break;
				case HEAD:
					*z = TAIL;
					break;
				case WIRE:
					count = 0;
					add(x - 1, y);
					add(x + 1, y);
					add(x - 1, y - 1);
					add(x, y - 1);
					add(x + 1, y - 1);
					add(x - 1, y + 1);
					add(x, y + 1);
					add(x + 1, y + 1);
					if (count == 1 || count == 2)
						*z = HEAD;
					else
						*z = WIRE;
					break;
				default:
					(void) fprintf(stderr,
						       "oops, bad internal character %d at %d,%d\n",
						       (int) loc(x, y), x, y);
					exit(1);
			}
		}
	}
}

static void
free_list(circuitstruct * wp)
{
	int         state;

	for (state = 0; state < COLORS - 1; state++)
		free_state(wp, state);
}

void
init_wire(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	int         size = MI_SIZE(mi);
	circuitstruct *wp;
	XGCValues   gcv;
	int         i;

	if (circuits == NULL) {
		if ((circuits = (circuitstruct *) calloc(MI_NUM_SCREENS(mi),
					    sizeof (circuitstruct))) == NULL)
			return;
	}
	wp = &circuits[MI_SCREEN(mi)];
	wp->redrawing = 0;
	if ((MI_NPIXELS(mi) <= 2) && (wp->init_bits == 0)) {
		gcv.fill_style = FillOpaqueStippled;
		wp->stippled_GC = XCreateGC(display, window, GCFillStyle, &gcv);
		for (i = 0; i < COLORS - 1; i++)
			BITS(patterns[i], PATTERNSIZE, PATTERNSIZE);
	}
	if (MI_NPIXELS(mi) > 2) {
		wp->colors[0] = (NRAND(MI_NPIXELS(mi)));
		wp->colors[1] = (wp->colors[0] + MI_NPIXELS(mi) / 6 +
				 NRAND(MI_NPIXELS(mi) / 4)) % MI_NPIXELS(mi);
		wp->colors[2] = (wp->colors[1] + MI_NPIXELS(mi) / 6 +
				 NRAND(MI_NPIXELS(mi) / 4)) % MI_NPIXELS(mi);
	}
	free_list(wp);
	wp->generation = 0;
	wp->n = MI_BATCHCOUNT(mi);
	if (wp->n < -MINWIRES) {
		wp->n = NRAND(-wp->n - MINWIRES + 1) + MINWIRES;
	} else if (wp->n < MINWIRES)
		wp->n = MINWIRES;

	wp->width = MI_WIN_WIDTH(mi);
	wp->height = MI_WIN_HEIGHT(mi);

	if (size < -MINSIZE)
		wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
	else if (size < MINSIZE) {
		if (!size)
			wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
		else
			wp->ys = MINSIZE;
	} else
		wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				       MINGRIDSIZE));
	wp->xs = wp->ys;
	wp->ncols = MAX(wp->width / wp->xs, 2);
	wp->nrows = MAX(wp->height / wp->ys, 2);
	wp->xb = (wp->width - wp->xs * wp->ncols) / 2;
	wp->yb = (wp->height - wp->ys * wp->nrows) / 2;

	wp->bncols = wp->ncols + 2;
	wp->bnrows = wp->nrows + 2;
	XClearWindow(display, MI_WINDOW(mi));

	if (wp->world != NULL)
		(void) free((void *) wp->world);
	wp->world = (unsigned char *)
		calloc(wp->bncols * wp->bnrows, sizeof (unsigned char));

	if (wp->worldnew != NULL)
		(void) free((void *) wp->worldnew);
	wp->worldnew = (unsigned char *)
		calloc(wp->bncols * wp->bnrows, sizeof (unsigned char));

	create_path(wp, wp->n);
}

void
draw_wire(ModeInfo * mi)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	int         offset, i, j;
	unsigned char *z, *znew;

	for (j = 1; j < wp->bnrows - 1; j++) {
		for (i = 1; i < wp->bncols - 1; i++) {
			offset = j * wp->bncols + i;
			z = wp->world + offset;
			znew = wp->worldnew + offset;
			if (*z != *znew) {	/* Counting on once a space always a space */
				*z = *znew;
				addtolist(mi, i - 1, j - 1, *znew - 1);
			}
		}
	}
	for (i = 0; i < COLORS - 1; i++)
		draw_state(mi, i);
	if (++wp->generation > MI_CYCLES(mi))
		init_wire(mi);
	else
		do_gen(wp);
}

void
release_wire(ModeInfo * mi)
{
	if (circuits != NULL) {
		int         screen, shade;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			circuitstruct *wp = &circuits[screen];

			free_list(wp);
			if (wp->stippled_GC != NULL)
				XFreeGC(MI_DISPLAY(mi), wp->stippled_GC);
			if (wp->init_bits != 0)
				for (shade = 0; shade < COLORS - 1; shade++)
					XFreePixmap(MI_DISPLAY(mi), wp->pixmaps[shade]);
			if (wp->world != NULL)
				(void) free((void *) wp->world);
			if (wp->worldnew != NULL)
				(void) free((void *) wp->worldnew);
		}
		(void) free((void *) circuits);
		circuits = NULL;
	}
}

void
refresh_wire(ModeInfo * mi)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];

	wp->redrawing = 1;
	wp->redrawpos = wp->ncols + 1;
}
