/*
 * Hypertex modifications to DVI previewer for X.
 * This portion of xhdvi is completely in the public domain. The
 * author renounces any copyright claims. It may be freely used for
 * commercial or non-commercial purposes. The author makes no claims
 * or guarantees - use this at your own risk.
 * 
 * Arthur Smith, U. of Washington, 1994
 *
 * 5/1994       code written from scratch, probably inspired by (but
 *                incompatible with) the CERN WWW library.
 * 3/1995       CERN WWW library called to do document fetching.
 *
 * SU 2000/02/27: replaced instances of `realloc' by `xrealloc' (util.c)
 *      and the fixed-length strings by xmalloc'ed strings
 *
 * SU 2000/02/27: TODO: anchor_info is obviously only defined (in events.c)
 *	when compiling without TOOLKIT. OTOH, compiling without
 *	TOOLKIT currently seems broken (ironically because of other
 *	parts of hyperref) - this should be fixed. Currently the
 *	error messages are printed using the statusline instead if TOOLKIT is
 *	defined, so maybe one could do without the code for anchor_info
 *	entirely?
 *
 * Note: several coordinate systems are in use in this code.  Note
 * that the relation between a screen pixel and a dvi coordinate is
 * the current shrink factor times 2^16.
 * 
 * When multiple pages are allowed in one drawing area, the variables
 * xscroll_pages or yscroll_pages are set.
 *
 * Conversion between coordinates is done using the screen_to_page,
 * page_to_screen, screen_to_abs, abs_to_screen routines.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL PAUL VOJTA OR ANY OTHER AUTHOR OF THIS SOFTWARE BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "xdvi-config.h"
#if defined(XHDVI) || defined(HTEX)
#include "kpathsea/c-ctype.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-stat.h"
#define GETCWD(str,size) xgetcwd()
#include <X11/Shell.h>	/* needed for def of XtNtitle */
#include <X11/StringDefs.h>

#include "wwwconf.h"
#include "WWWLib.h"
#include "WWWInit.h"
#include "WWWCache.h"
#include "version.h"
#include "string-utils.h"

#ifdef HTEX
/* Application name and version for w3c-libwww routines.  
   This is what will show up in httpd's agent_log files. 
*/
#ifdef Omega
char *HTAppName = "oxdvik";
#else
char *HTAppName = "xdvik";
#endif
char *HTAppVersion = XDVERSION;
#endif

/* Implementation of HyperTeX through \specials */
/* One option: allow HTML tags and recognize them */
/* applicationDoSpecial strips leading blanks, so first char should
   be '<' if it's a tag */

int this_is_a_href = 0; /* for communication with dvi-init.c */

#define HTeX_A_NAME 1
#define HTeX_A_HREF 2
#define HTeX_SRC_SPECIAL 3

#define htex_shrinkfactor mane.shrinkfactor	/* Only main win has refs */

#define ANCHOR_STRING_LEN 256
#define BUFFER_STRING_LEN 512

typedef struct {
    int type;	/* type is either HTeX_A_NAME or HTeX_A_HREF */
    char *name;	/* Name string for anchor (Null by default) */
    char *href;	/* A reference from this anchor */
    int llx, lly, urx, ury;	/* Box on page where anchor located */
} HTeX_Anchor;

/* Structure to remember where we have been: */
typedef struct {
    char *refname;	/* File name (or href?) */
    int pageno;	/* Which page anchor was on */
    int x, y;	/* Approximate location on page... */
    int which;	/* Number of anchor on this page */
    int type;	/* And the other properties of this anchor */
    char *name;
    char *href;
} Anchors;

static int waiting_for_anchor = -1;	/* If waiting for anchor to be properly parsed? */
int global_x_marker, global_y_marker, global_marker_page;
static Boolean htex_goback_flag = False;
static int cur_anchor_on_page;	/* number of current anchor on page when scanning the page */

static char *anchor_name = NULL;
#define HTeX_AnchorSTEP 20	/* new memory for anchors is allocated in these steps */
static HTeX_Anchor **HTeX_anchorlist;	/* array containing the anchors */
static int *nHTeX_anchors;	/* array of numbers of anchors per page */
static int *maxHTeX_anchors;	/* array of anchors entered so far per page */


static Anchors *HTeX_visited = NULL;	/* list of anchors visited, for the `BACK' functionality */
static int nHTeX_visited = 0, maxHTeX_visited = 0;

#define HTeX_NSTACK 32	/* Maximum number of nested anchors */
static int HTeXAnest[HTeX_NSTACK];	/* number of anchor on page, per nesting level */

/* following are also used in dvi-draw.c */
int HTeXAnestlevel;	/* Current nesting level */
int HTeXreflevel;	/* 0 if not currently inside an href anchor */

int *htex_parsedpages;	/* List of all pages: 1 if already parsed,
				 * 0 if not, 2 if geometry info still missing
				 */
static int htex_total_pages;	/* remember tot. number of pages in current dvi file;
			 * for checks when dvi file changes
			 */

/* whether we need to go to an anchor in a newly opened dvi file */
static Boolean jump_to_anchor = False;

/* info about anchor we need to go to: page, number of anchor on page */
static struct jump_to_anchorinfo_t {
    int page;
    int num;
} jump_to_anchorinfo;

/* size = total_pages, current page = current_page defined in xhdvi.h */

/* prototypes; those that should be static moved from xdvi.h here. */
static int  open_www_file ARGS((Boolean, int));
static void htex_parseanchor ARGS((char *, HTeX_Anchor *));
static void htex_handletag ARGS((char *, int));
static void htex_anchor ARGS((int, char *, int));
static void htex_dohref ARGS((Boolean, char *));
static void htex_drawboxes ARGS((void));
static void htex_to_page ARGS((int));
static void htex_to_anchor ARGS((int, int, int));
static void htex_do_url ARGS((Boolean, char *));
static void htex_can_it ARGS((void));
static void extractbase ARGS((char *file));
/*
 * SU 2000/12/16: replaced retitle() by set_icon_and_title(), defined in xdvi.c
 * which also takes care of the icon name.
 */
static char *refscan ARGS((char *, char **, char **));
static void htex_loc_on_page ARGS((int, Anchors *));
static void freeHTeXAnchors ARGS((HTeX_Anchor *, int));
static void htex_img ARGS((int, char *, int));
static void htex_base ARGS((int, char *, int));


static int BEGIN = 0;
static int END = 1;

/* Dave Oliver's hypertex format: */
/* Only understand my version of anchors so far */
/* ie.: his TYPE=text for hrefs, frag for names */
static void
hy_handletag(char *cp, int pageno)
{
    int beginend = BEGIN;

    while (isspace(*cp))
	cp++;
    if (!strncasecmp(cp, "END", 3)) {
	beginend = END;
	cp += 3;
    }
    /* Leave the parsing to htex_anchor! */
    htex_anchor(beginend, cp, pageno);
}

int
checkHyperTeX(char *cp, int pageno)
{
    Boolean htex_ref_found = False;

    if (strncasecmp(cp, "html:", 5) == 0) {
	cp += 5;
	while (isspace(*cp))
	    cp++;
	htex_ref_found = True;
    }
    if (*cp == '<') {	/* Possibly Missing the header part */
	htex_ref_found = True;
	htex_handletag(cp, pageno);
    }
    else if (strncasecmp(cp, "hyp", 3) == 0) {
	/* Dave Oliver's HyperTeX */
	htex_ref_found = True;
	cp += 4;
	hy_handletag(cp, pageno);
    }

    return htex_ref_found;
}

static void
htex_handletag(char *cp, int pageno)
{
    int beginend = BEGIN;

    if (*cp != '<')
	return;
    ++cp;
    while (isspace(*cp))
	cp++;
    if (*cp == DIR_SEPARATOR) {
	beginend = END;
	cp++;
    }
    switch (*cp) {
    case 'A':
    case 'a':	/* Anchors */
	htex_anchor(beginend, cp + 1, pageno);
	break;
    case 'b':	/* Base name? */
	htex_base(beginend, cp, pageno);
	break;
    case 'i':	/* Included images? */
	htex_img(beginend, cp, pageno);
	break;
    default:	/* Tag not implemented yet */
	break;
    }
}

/* Basically just want to parse the line... */
/* Should use WWW library stuff ? */

static void
htex_anchor(int beginend, char *cp, int pageno)
{
    int *nap, *maxp;
    int oldllx, oldlly, oldurx, oldury;
    HTeX_Anchor *HTeXAp = NULL, **HTeXApp;

    HTeXApp = HTeX_anchorlist + pageno;
    nap = nHTeX_anchors + pageno;	/* number of anchor pointers */
    maxp = maxHTeX_anchors + pageno;	/* number of pointers allocated so far */
    if (*nap >= *maxp) {
	*maxp += HTeX_AnchorSTEP;
	*HTeXApp = xrealloc(*HTeXApp, (*maxp) * sizeof(HTeX_Anchor));
    }
    if (htex_parsedpages[pageno] != 1) {	/* Only do if page not done yet */
	if (beginend == END) {	/* at end of anchor; finish up */
	    HTeXAnestlevel--;
	    if (HTeXAnestlevel < 0) {
		HTeXAnestlevel = 0;	/* ignore extra </a>'s */
	    }
	    else {
		HTeXAp = *HTeXApp + HTeXAnest[HTeXAnestlevel];
		if (HTeXAp->llx > DVI_H) {
		    HTeXAp->llx = DVI_H;
		}
		if (HTeXAp->urx < DVI_H) {
		    HTeXAp->urx = DVI_H;
		}
		if (HTeXAp->lly > DVI_V) {
		    HTeXAp->lly = DVI_V;
		}
		if (HTeXAp->ury < DVI_V) {
		    HTeXAp->ury = DVI_V;
		}
		oldllx = HTeXAp->llx;
		oldlly = HTeXAp->lly;
		oldurx = HTeXAp->urx;
		oldury = HTeXAp->ury;
		if (debug & DBG_ANCHOR) {
		    Printf("Added anchor %d, level %d:\n",
			   HTeXAnest[HTeXAnestlevel], HTeXAnestlevel);
		    if (HTeXAp->type & HTeX_A_HREF) {
			Printf("href = %s\n", HTeXAp->href);
		    }
		    if (HTeXAp->type & HTeX_A_NAME) {
			Printf("name = %s\n", HTeXAp->name);
		    }
		    Printf("box %d %d %d %d\n",
			   HTeXAp->llx, HTeXAp->lly, HTeXAp->urx, HTeXAp->ury);
		}
		/* End of debug section */

		if (waiting_for_anchor == HTeXAnest[HTeXAnestlevel]) {
		    htex_to_anchor(0, current_page, waiting_for_anchor);
		    waiting_for_anchor = -1;	/* Reset it! */
		}
		if (HTeXAnestlevel > 0) {
		    HTeXAp = *HTeXApp + HTeXAnest[HTeXAnestlevel - 1];
		    /* Check llx, lly, urx, ury info */
		    if (oldllx < HTeXAp->llx) {
			HTeXAp->llx = oldllx;
		    }
		    if (oldlly < HTeXAp->lly) {
			HTeXAp->lly = oldlly;
		    }
		    if (oldurx > HTeXAp->urx) {
			HTeXAp->urx = oldurx;
		    }
		    if (oldury > HTeXAp->ury) {
			HTeXAp->ury = oldury;
		    }
		}
	    }
	}
	else {	/* at begin of anchor */
	    HTeXAp = *HTeXApp + *nap;
	    /* Set type, and the name, href */
	    htex_parseanchor(cp, HTeXAp);
	    if (HTeXAp->type != 0) {
		cur_anchor_on_page++;	/* Increment the count of anchors here */
		if (htex_parsedpages[pageno] == 2) {
		    /* fix geometry info for stored anchors */
		    HTeXAp = *HTeXApp + cur_anchor_on_page;
		    HTeXAp->urx = HTeXAp->llx = DVI_H;	/* Current horiz pos. */
		    HTeXAp->ury = HTeXAp->lly = DVI_V;	/* Current vert. pos. */
		    if (HTeXAnestlevel >= HTeX_NSTACK) {
			/* Error - too many nested anchors! */
		    }
		    else {
			HTeXAnest[HTeXAnestlevel++] = cur_anchor_on_page;
		    }
		}
		else if (htex_parsedpages[pageno] == 0) {	/* Only do if page not done yet */
		    HTeXAp->urx = HTeXAp->llx = DVI_H;	/* Current horiz pos. */
		    HTeXAp->ury = HTeXAp->lly = DVI_V;	/* Current vert. pos. */
		    if (HTeXAnestlevel >= HTeX_NSTACK) {
			/* Error - too many nested anchors! */
		    }
		    else {
			HTeXAnest[HTeXAnestlevel++] = *nap;
		    }
		    (*nap)++;
		}
	    }
	}
    }
    else {	/* if page had been properly parsed before */
	if (beginend != END) {
	    HTeXAp = *HTeXApp + *nap;
	    /* Set type, and the name, href */
	    htex_parseanchor(cp, HTeXAp);
	}
    }
    if (beginend == END) {
	if (HTeXreflevel > 0) {
	    HTeXreflevel--;
	}
    }
    else {
	if (HTeXAp->type & HTeX_A_HREF) {
	    HTeXreflevel++;
	}
    }
}


void
htex_open_dvi_file(void)
{
    char *cp = getenv("WWWBROWSER");
    if (cp)
	browser = cp;

    HTProfile_newPreemptiveClient(HTAppName, HTAppVersion);
    HTCacheMode_setEnabled(NO);

    if (debug & DBG_HYPER) {
	/* while working on hyperref, it's convenient to
	   see all libwww messages: */
	/*  	HTSetTraceMessageMask("*"); */
    }

    /* Open the input file.  Turn filename into URL first if needed */
    URL_aware = TRUE;

    if (!(URLbase || htex_is_url(dvi_name))) {
	char *new_name;
	size_t n;

	n = strlen(dvi_name);
	new_name = dvi_name;
	/* Find the right filename */
	if (n < sizeof(".dvi")
	    || strcmp(dvi_name + n - sizeof(".dvi") + 1, ".dvi") != 0) {
	    dvi_name = xmalloc((unsigned)n + sizeof(".dvi"));
	    Strcpy(dvi_name, new_name);
	    Strcat(dvi_name, ".dvi");
	    /* Attempt $dvi_name.dvi */
	    dvi_file = xfopen(dvi_name, OPEN_MODE);
	    if (dvi_file == NULL) {
		/* Didn't work, revert to original $dvi_name */
		free(dvi_name);
		dvi_name = new_name;
	    }
	}

	/* Turn filename into URL */
	/* Escape dangers, esp. # from emacs */
	new_name = HTEscape(dvi_name, URL_PATH);
	free(dvi_name);
	dvi_name = xmalloc((unsigned)strlen(new_name) + 6);	/* 6 for `file:', `\0' */
	strcat(strcpy(dvi_name,"file:"), new_name);
	/* Now we have the right filename, in a URL */
    }

    detach_anchor();
    if (!open_www_file(False, 0)) {	/* open, but don't set title */
	fprintf(stderr, "Can't open `%s': No such file or directory\n", global_dvi_name);
	exit(1);
    }

    URL_aware = FALSE;
}



void
htex_initpage(void)
{
    /* Starting a new page */
    if (htex_parsedpages == NULL)
	htex_reinit();
    HTeXAnestlevel = 0;	/* Start with zero nesting level for a page */
    HTeXreflevel = 0;
    cur_anchor_on_page = -1;
}

/*
 * called by put_image and put_bitmap in dvi-draw.c if HTeXAnestlevel > 0:
 * record position for current anchor
 */
void
htex_recordbits(int x, int y, int w, int h)	/* x,y are pixel positions on current page */
{
    int dvix, dviy, dvix2, dviy2;
    int ch = 0;

    dvix = x * (htex_shrinkfactor << 16);
    dviy = y * (htex_shrinkfactor << 16);
    dvix2 = (x + w) * (htex_shrinkfactor << 16);
    dviy2 = (y + h) * (htex_shrinkfactor << 16);
#if 0	/* draw marks for the coordinates found */
    XFillRectangle(DISP, mane.win, foreGC, x, y, 2, 2);
    XFillRectangle(DISP, mane.win, highGC, x + w, y + h, 2, 2);
#endif
    if (HTeXAnestlevel > 0) {
	HTeX_Anchor *HTeXAp =
	    HTeX_anchorlist[current_page] + HTeXAnest[HTeXAnestlevel - 1];
	if (HTeXAp->llx > dvix) {
	    HTeXAp->llx = dvix;
	    ch++;
	}
	if (HTeXAp->lly > dviy) {
	    HTeXAp->lly = dviy;
	    ch++;
	}
	if (HTeXAp->urx < dvix2) {
	    HTeXAp->urx = dvix2;
	    ch++;
	}
	if (HTeXAp->ury > dviy2) {
	    HTeXAp->ury = dviy2;
	    ch++;
	}
	if (debug & DBG_ANCHOR) {
	    if (ch > 0) {
		Printf
		    ("New box for anchor %d, level %d: %d %d %d %d; string: %s\n",
		     HTeXAnest[HTeXAnestlevel - 1], HTeXAnestlevel, HTeXAp->llx,
		     HTeXAp->lly, HTeXAp->urx, HTeXAp->ury, HTeXAp->name);
	    }
	}
    }
}

void
htex_donepage(int i, int pflag)	/* This page has been completed */
{
    HTeX_Anchor *HTeXAp;

    /* Finish off boxes for nested anchors not done on this page */
    while (HTeXAnestlevel > 0) {
	HTeXAnestlevel--;
	assert(HTeX_anchorlist[i] != NULL);
	HTeXAp = HTeX_anchorlist[i] + HTeXAnest[HTeXAnestlevel];

	if (HTeXAp->llx > DVI_H) {
	    HTeXAp->llx = DVI_H;
	}
	if (HTeXAp->urx < DVI_H) {
	    HTeXAp->urx = DVI_H;
	}
	if (HTeXAp->lly > DVI_V) {
	    HTeXAp->lly = DVI_V;
	}
	if (HTeXAp->ury < DVI_V) {
	    HTeXAp->ury = DVI_V;
	}
    }
    if (pflag == 1) {	/* Really parsed this page; this is true when called from dvi-draw.c after
			 * fully scanning the page.
			 */
	htex_drawboxes();	/* Draw boxes around the anchor positions */
	htex_parsedpages[i] = 1;
    }
    else {
	htex_parsedpages[i] = 2;	/* this is the case when page has been prescanned by htex_parse_page,
					 * but geometry information is still missing for the anchors.
					 */
    }
}

/* If the dvi file has changed, assume all links have changed also,
   and reset everything! */
void
htex_reinit(void)
{
    int i;

    global_x_marker = global_y_marker = 0;
    if (htex_parsedpages == NULL) {	/* First call to this routine */
	htex_parsedpages = xmalloc(total_pages * sizeof(int));
	HTeX_anchorlist = xmalloc(total_pages * sizeof(HTeX_Anchor *));
	nHTeX_anchors = xmalloc(total_pages * sizeof(int));
	maxHTeX_anchors = xmalloc(total_pages * sizeof(int));
	for (i = 0; i < total_pages; i++)
	    maxHTeX_anchors[i] = 0;
    }
    else if (htex_total_pages != total_pages) {
	htex_parsedpages = (int *)realloc(htex_parsedpages,
					  total_pages * sizeof(int));
	/* Following operates if new has fewer pages than old: */
	for (i = total_pages; i < htex_total_pages; i++) {
	    if (maxHTeX_anchors[i] > 0)
		freeHTeXAnchors(HTeX_anchorlist[i], nHTeX_anchors[i]);
	}
	HTeX_anchorlist = (HTeX_Anchor **) realloc(HTeX_anchorlist,
						   total_pages *
						   sizeof(HTeX_Anchor *));
	nHTeX_anchors = (int *)realloc(nHTeX_anchors, total_pages * sizeof(int));
	maxHTeX_anchors = (int *)realloc(maxHTeX_anchors, total_pages * sizeof(int));
	/* Following operates if new has more pages than old: */
	for (i = htex_total_pages; i < total_pages; i++)
	    maxHTeX_anchors[i] = 0;
    }
    htex_total_pages = total_pages;
    for (i = 0; i < total_pages; i++) {
	htex_parsedpages[i] = 0;
	if (maxHTeX_anchors[i] > 0) {	/* Get rid of the old anchor lists: */
	    freeHTeXAnchors(HTeX_anchorlist[i], nHTeX_anchors[i]);
	    free(HTeX_anchorlist[i]);
	}
	HTeX_anchorlist[i] = NULL;
	nHTeX_anchors[i] = 0;
	maxHTeX_anchors[i] = 0;
    }
}

/* do neccessary re-initializations when switching
   to new .dvi file (or back)
*/
static void
dvi_reinit(void)
{
    if (debug & DBG_HYPER)
	fprintf(stderr, "dvi_reinit called!\n");
    htex_reinit();
    /* we also need to re-initialize all font structures - this
       _might_ indicate that some of the font routines are buggy :-(
    */
    reset_fonts();
    free_fontlist();
    if (debug & DBG_HYPER)
	fprintf(stderr, "now decrementing dvi_time\n");
    /* let change in dvi_time trigger dvi_file_changed()
       (FIXME: in upcoming version, use new event handling for this!)
     */
    dvi_time--;
}

/* Following parses the stuff after the '<' in the html tag */
/* Only understands name and href in anchor */
/*     html: <A HREF="..." NAME="..."> */
/*     html: <A NAME="..." HREF="...> */
static void
htex_parseanchor(char *cp, HTeX_Anchor *anchor)
{
    char *ref, *str;

    anchor->type = 0;
    anchor->href = NULL;
    anchor->name = NULL;
    while (isspace(*cp))
	cp++;
    while ((*cp) && (*cp != '>')) {
	cp = refscan(cp, &ref, &str);
	if (cp == NULL)
	    break;
	if (strcasecmp(ref, "href") == 0) {
	    anchor->type |= HTeX_A_HREF;
	    MyStrAllocCopy(&(anchor->href), str);
	}
	else if (strcasecmp(ref, "name") == 0) {
	    anchor->type |= HTeX_A_NAME;
	    MyStrAllocCopy(&(anchor->name), str);
	}
    }
}

char *
MyStrAllocCopy(char **dest, char *src)
{
    if (*dest)
	free(*dest);
    if (!src)
	*dest = NULL;
    else {
	*dest = xmalloc(strlen(src) + 1);
	strcpy(*dest, src);
    }
    return *dest;
}


/* Parses cp containing 'ref="string"more', returning pointer to "more" */
static char *
refscan(char *name, char **ref, char **str)
{
    char *cp;

    *str = name;
    for (cp = name; *cp; cp++) {
	if (*cp == '=') {
	    *cp = 0;
	    *ref = name;
	    *str = cp + 1;
	    break;
	}
    }
    cp = *str;
    if (cp != name) {
	while (isspace(*cp))
	    cp++;
	if (*cp == '"') {	/* Yes, this really is a string being set */
	    *str = cp + 1;
	    while ((cp = strchr(cp + 1, '"')) != NULL) {
		if (cp[-1] != '\\')
		    break;	/* Check if quote escaped */
	    }
	    if (cp != NULL) {
		*cp = 0;
		cp++;
	    }
	}
	else {
	    cp = NULL;
	}
    }
    else {
	cp = NULL;
    }
    return cp;
}

extern int global_string_id;

/******************************
 * Function: htex_displayanchor
 * Purpose:  change the cursor to hand, and display anchor string
 *		of link under mouse cursor in the statusline, if statusline
 *		is active; else only change cursor.
 * Arguments:
 *	page - number of current page
 *	x,y  - cursor position (in pixel)
 *
 *******************************/

extern Boolean dragcurs;
extern int drag_flags;

void
htex_displayanchor(int page, int x, int y)
{
    HTeX_Anchor *HTeXAp;
    long dvix, dviy;
    int i;
    static int id_bak = -1;	/* save id of previous anchor */

    int found = 0;

    /* Don't display until we've finished with the page, or
     * if expert mode is on: */
    if (htex_parsedpages == NULL) {
	return;
    }
    /* Locate current page if we're scrolling them: */
    current_page = page;
    if (htex_parsedpages[current_page] != 1) {
	return;
    }

    dvix = x * (htex_shrinkfactor << 16);
    dviy = y * (htex_shrinkfactor << 16);
    /* Find anchor that fits current position: */
    if(HTeX_anchorlist == NULL || HTeX_anchorlist[current_page] == NULL)
	return;
    HTeXAp = HTeX_anchorlist[current_page] + nHTeX_anchors[current_page] - 1;
    for (i = nHTeX_anchors[current_page] - 1; i >= 0; i--, HTeXAp--) {
	if (dvix > HTeXAp->llx && dvix < HTeXAp->urx && dviy > HTeXAp->lly
	    && dviy < HTeXAp->ury) {
	    if (debug & DBG_ANCHOR) {
		Printf("In anchor #%d: name: %s, href: %s\n",
		       i,
		       HTeXAp->name ? HTeXAp->name : "<NULL>",
		       HTeXAp->href);
	    }
	    found = 1;
	    break;
	}
    }
    if (found) {
	XDefineCursor(DISP, mane.win, link_cursor);
#ifdef STATUSLINE
	/*
	 * If the statusline is enabled, print destination of the anchor
	 * (so we print only HTeX_A_HREF anchors).
	 * To prevent flicker, we print the anchor string only if it
	 * differs from the previous one.
	 */
	if (id_bak != i && resource.statusline && (HTeXAp->type & HTeX_A_HREF)) {
	    print_statusline(STATUS_FOREVER, "%s", HTeXAp->href);
	}
	id_bak = i;
#endif
    }
    else {	/* not found, reset cursor to normal */
	if (!dragcurs) {
	    XDefineCursor(DISP, mane.win, ready_cursor);
	}
	else {
	    XDefineCursor(DISP, mane.win, drag_cursor[drag_flags - 1]);
	}

#ifdef STATUSLINE
	/* if statusline contains anchor string, erase it and reset
	 * id_bak to initial value: */
	if (id_bak != -1 && resource.statusline) {
	    print_statusline(STATUS_FOREVER, "");
	    id_bak = -1;
	}
#endif
    }
}


/* What happens when mouse is clicked: */
int
htex_handleref(Boolean invoke_new, /* True for `open new xdvi instance', False else */
	       int page,
	       int x, int y) /* current mouse location when ref clicked */
{
    HTeX_Anchor *HTeXAp;
    long dvix, dviy;
    int i, afound;

    /* Check that we've finished at least one page first! */
    if (htex_parsedpages == NULL)
	return 0;
    /* Locate current page if we're scrolling them: */
    current_page = page;
    if (htex_parsedpages[current_page] != 1)
	return 0;
    dvix = x * (htex_shrinkfactor << 16);
    dviy = y * (htex_shrinkfactor << 16);
    /* Find anchor that fits current position: */
    if(HTeX_anchorlist[current_page] == NULL)
	return 0;
    HTeXAp = HTeX_anchorlist[current_page] + nHTeX_anchors[current_page] - 1;
    afound = -1;
    for (i = nHTeX_anchors[current_page] - 1; i >= 0; i--, HTeXAp--) {
	if ((HTeXAp->type & HTeX_A_HREF) == 0)
	    continue;	/* Only ref on hrefs */
	if (HTeXAp->llx > dvix)
	    continue;
	if (HTeXAp->lly > dviy)
	    continue;
	if (HTeXAp->urx < dvix)
	    continue;
	if (HTeXAp->ury < dviy)
	    continue;
	afound = i;	/* Get the last of them in case of nesting */
	break;
    }
    if (afound == -1)
	return 0;	/* There was no href at this location */
    /* Then just do it: */
    this_is_a_href = 1;
    htex_dohref(invoke_new, HTeXAp->href);
    this_is_a_href = 0;
    return 1;
}

static void
htex_dohref(Boolean invoke_new, char *href)
{
    int i;

    /* Update the list of where we used to be: */
    if (HTeX_visited == NULL) {
	maxHTeX_visited = HTeX_AnchorSTEP;
	HTeX_visited = xmalloc(maxHTeX_visited * sizeof(Anchors));
	for (i = 0; i < maxHTeX_visited; i++) {
	    HTeX_visited[i].refname = NULL;
	    HTeX_visited[i].name = NULL;
	    HTeX_visited[i].href = NULL;
	}
    }
    else if (nHTeX_visited >= maxHTeX_visited - 1) {
	maxHTeX_visited += HTeX_AnchorSTEP;
	HTeX_visited = (Anchors *) xrealloc(HTeX_visited,
					    maxHTeX_visited * sizeof(Anchors));
	for (i = nHTeX_visited; i < maxHTeX_visited; i++) {
	    HTeX_visited[i].refname = NULL;
	    HTeX_visited[i].name = NULL;
	    HTeX_visited[i].href = NULL;
	}
    }
    MyStrAllocCopy(&(HTeX_visited[nHTeX_visited].refname), dvi_name);
    HTeX_visited[nHTeX_visited].pageno = current_page;
#ifdef HTEX
    HTeX_visited[nHTeX_visited].x = mane.base_x;
    HTeX_visited[nHTeX_visited].y = mane.base_y;
#else
    HTeX_visited[nHTeX_visited].x = mane.base_ax;
    HTeX_visited[nHTeX_visited].y = mane.base_ay;
#endif
    HTeX_visited[nHTeX_visited].type = HTeX_A_HREF;
    HTeX_visited[nHTeX_visited].which = 0;
    MyStrAllocCopy(&(HTeX_visited[nHTeX_visited].href), href);
    nHTeX_visited++;
    if (htex_is_url(href)) {
	htex_do_url(invoke_new, href);
    }
    else {
	htex_do_loc(invoke_new, href);
    }
    /* Need to handle properly when ref doesn't exist! */
}

void
check_for_anchor(void)
{
    if (debug & DBG_HYPER)
	fprintf(stderr, "check_for_anchor |%s|\n", anchor_name ? anchor_name : "<NULL>");
    if (jump_to_anchor) {
	htex_to_anchor(0, jump_to_anchorinfo.page, jump_to_anchorinfo.num);
	jump_to_anchor = False;
    }
    else if (anchor_name != NULL) {
	if (debug & DBG_HYPER)
	    fprintf(stderr, "anchor_name != NULL\n");
	htex_reinit();	/* Set up hypertext stuff */
	htex_do_loc(False, anchor_name);
	free(anchor_name);
	anchor_name = NULL;
    }
}

/* Draw boxes around the anchor positions */
static void
htex_drawboxes(void)
{
    HTeX_Anchor *HTeXAp;
    int i;
    int x, y, w, h;
    /* put_rule will replace values of 0 by 1 */
    int rule_height = (page_h / 1000.0);
    
    if (!underline_link)
	return;
    HTeXAp = HTeX_anchorlist[current_page];
    for (i = 0; i < nHTeX_anchors[current_page]; i++, HTeXAp++) {
	if ((HTeXAp->type & HTeX_A_HREF) == 0)
	    continue;	/* Only box hrefs */
	x = pixel_conv(HTeXAp->llx) - 1;
	y = pixel_conv(HTeXAp->lly) - 1;
	w = pixel_conv(HTeXAp->urx) - x + 2;
	h = pixel_conv(HTeXAp->ury) - y + 2;
	/* The first arg of put_rule is whether or not to
	   use the "highlight" graphics context. */
	put_rule(True, x + 1, y + h, w, rule_height);
    }
}

static void
invokedviviewer(char *filename, char *anchor_name, int height)
{
#define ARG_LEN 32
    int i = 0;
    char *argv[ARG_LEN];
    char *shrink_arg = NULL;
    char *file_arg = NULL;
    char *geom_arg = NULL;
#if 0
    Dimension w, h;
    _Xconst char *_Xconst geom_format = "%dx%d";
#endif /* 0 */
    UNUSED(height);
    
    if (filename == NULL)
	return;

#if 0
    /* new xdvi window should have same geometry */
    XtVaGetValues(draw_widget, XtNwidth, &w, XtNheight, &h, NULL);
    geom_arg = xmalloc(strlen(geom_format) /* too much, but never mind */
		       + length_of_int((int)w)
		       + length_of_int((int)h)
		       + 1);
    sprintf(geom_arg, geom_format, w, h);
#endif
    
    argv[i++] = program_invocation_name;
    /* FIXME: there's something broken with this and invoking xdvi.bin (wrong
       program name? How about the other command-line switches that might have
       been passed to the parent instance? How about things that have been changed
       at run-time, like shrink factor - should they be converted to command-line
       options?)
       To reproduce the problem, invoke from the shell:
       xdvi.bin -geometry 829x1172 /usr/share/texmf/doc/programs/kpathsea.dvi
       this will run at 300dpi, i.e. ignore an .Xdefaults setting as opposed to:
       xdvi.bin /usr/share/texmf/doc/programs/kpathsea.dvi
    */
#if 0
    argv[i++] = "-geometry";
    argv[i++] = geom_arg;
#endif
    argv[i++] = "-name";
    argv[i++] = "xdvi";

    argv[i++] = "-s";
    shrink_arg = xmalloc(length_of_int((int)currwin.shrinkfactor) + 1);
    sprintf(shrink_arg, "%d", currwin.shrinkfactor);
    argv[i++] = shrink_arg;

    if (anchor_name == NULL) {
	argv[i++] = filename;
    }
    else {
	file_arg = xmalloc(strlen(filename) + strlen(anchor_name) + 2);
	sprintf(file_arg, "%s#%s", filename, anchor_name);
	argv[i++] = file_arg;
    }
    argv[i++] = NULL;

    assert(i <= ARG_LEN);
    
    if (debug & DBG_HYPER) {
	fprintf(stderr, "Invoking:\n");
	for (i = 0; argv[i]; i++) {
	    fprintf(stderr, "%s\n", argv[i]);
	}
    }
    fork_process(argv[0], argv);
    free(file_arg);
    free(geom_arg);
    free(shrink_arg);
#undef ARG_LEN
}

/* It's a local reference - find the anchor and go to it */
void
htex_do_loc(Boolean invoke_new, char *href)
{
    int ipage, ia = 0, reffound;
    HTeX_Anchor *HTeXAp = NULL, **HTeXApp;
    char *cp;

    if (debug & DBG_HYPER) {
	fprintf(stderr, "htex_do_loc(%s)\n", href);
    }
    if (href == NULL)
	return;	/* shouldn't happen? */
    cp = href;
    while (*cp == '#')
	cp++;
    HTeXApp = HTeX_anchorlist;
    reffound = 0;
    /* Should hash based on "name" value? - to speed this up! */
    for (ipage = 0; ipage < total_pages; ipage++, HTeXApp++) {
	if (htex_parsedpages[ipage] == 0)
	    continue;
	HTeXAp = *HTeXApp;
	for (ia = 0; ia < nHTeX_anchors[ipage]; ia++, HTeXAp++) {
	    if (debug & DBG_HYPER)
		fprintf(stderr, "checking |%s|%s|\n", HTeXAp->name, cp);
	    if ((HTeXAp->type & HTeX_A_NAME) == 0)
		continue;
	    if (!strcmp(HTeXAp->name, cp)) {
		reffound = 1;
		break;
	    }
	}
	if (reffound)
	    break;
    }
    if (reffound == 0) {	/* Need to parse remaining pages */
	if (debug & DBG_ANCHOR) {
	    Printf("Searching for remaining anchors\n");
	}
	htex_parsepages();
	/* And try again: */
	HTeXApp = HTeX_anchorlist;
	for (ipage = 0; ipage < total_pages; ipage++, HTeXApp++) {
	    if (htex_parsedpages[ipage] < 2)
		continue;
	    HTeXAp = *HTeXApp;
	    for (ia = 0; ia < nHTeX_anchors[ipage]; ia++, HTeXAp++) {
		if (debug & DBG_HYPER)
		    fprintf(stderr, "checking |%s|%s| (2)\n", HTeXAp->name, cp);
		if ((HTeXAp->type & HTeX_A_NAME) == 0)
		    continue;
		if (!strcmp(HTeXAp->name, cp)) {
		    reffound = 1;
		    break;
		}
	    }
	    if (reffound)
		break;
	}
    }
    if (reffound) {
	if (invoke_new) {
	    if (debug & DBG_HYPER)
		fprintf(stderr, "external link; invoking viewer for target: |%s|%s|\n", dvi_name, cp);
	    invokedviviewer(dvi_name, cp,
			    pixel_conv(HTeXAp->ury) - pixel_conv(HTeXAp->lly));
	}
	else {
	    if (debug & DBG_HYPER)
		fprintf(stderr, "internal link; invoking viewer for target: |%s|%s|\n", dvi_name, cp);
	    htex_to_anchor(1, ipage, ia);	/* move to anchor */
	}
    }
    else {
	if ((nHTeX_visited == 0) ||
	    (!strcmp(HTeX_visited[nHTeX_visited - 1].refname, dvi_name))) {
	    /* Really was from same file - just print error message */
	    do_popup_message(MSG_ERR,
			     NULL,
			     "Error: reference \"%s\" not found", cp);
	}
	else {
	    /* Go to page 1 and print error message */
	    do_popup_message(MSG_ERR,
			     NULL,
			     "Error: reference \"%s\" in file %s not found", cp,
			     dvi_name);
	    htex_to_page(0);	/* Go to first page! */
	}
    }
}

static void
htex_can_it(void)
{
    canit = True;
    XFlush(DISP);
}

static void
htex_to_page(int pageno)
{
    /* taken from keystroke subroutine: */
    current_page = pageno;
    htex_can_it();
}


/* TODO: might be better to draw the marker *inside* the clipping region??
   Also, maybe we should use an arrow bitmap instead?
*/
void
htex_draw_anchormarker(int x, int y)
{
    XPoint points[3];
/*     x += page_w - clip_w; */
/*     x = x + mane_base_x + clip_w; */
/*     y = y + mane_base_y + clip_h; */
    points[0].x = x + 10;
    points[1].x = x + 19;
    points[2].x = x + 10;
    points[0].y = y - 3;
    points[1].y = y + 2;
    points[2].y = y + 7;

/*     XFillRectangle(DISP, mane.win, highGC, 2 + (mane.base_x * clip_w / page_w), y, 10, 4); */
/*     XFillRectangle(DISP, mane.win, highGC, mane.base_x * clip_w / page_w, y, 10, 4); */
    XFillRectangle(DISP, mane.win, highGC, x, y, 10, 4);
    XFillPolygon(DISP, mane.win, highGC, points, 3, Convex, CoordModeOrigin);
/*     XFillRectangle(DISP, XtWindow(vport_widget), highGC, 0, 0, 30, 30); */
}

static XtIntervalId href_timeout_id = 0;

static void
htex_erase_anchormarker(XtPointer client_data, XtIntervalId * id)
{
    UNUSED(client_data);
    UNUSED(id);
    
    if (debug & DBG_EVENT) {
	fprintf(stderr, "htex_erase_anchormarker called!\n");
    }
    /* clear the mark */
    clearexpose(&mane, global_x_marker, global_y_marker - 3, 20, 10);
    global_x_marker = global_y_marker = 0;

    href_timeout_id = 0;
}


static void
htex_to_anchor(int flag, int pageno, int n)
{
    HTeX_Anchor *HTeXAp;

    if (debug & DBG_ANCHOR) {
	fprintf(stderr, "trying to jump to anchor %d on page %d (flag: %d; current page: %d)\n",
		n, pageno, flag, current_page);
	fprintf(stderr, "nHTeX_anchors on page %d: %d\n", pageno, nHTeX_anchors[pageno]);
    }


    if (pageno != current_page) {
	if (htex_parsedpages[pageno] != 1) {
	    waiting_for_anchor = n;
	}
	/* taken from keystroke subroutine: */
	current_page = pageno;
	htex_can_it();
    }
    else if (!htex_goback_flag) { /* if on same page, generate exposure event so that mark gets drawn */
	clearexpose(&mane, global_x_marker, global_y_marker - 3, 20, 10);
    }

    if(HTeX_anchorlist[pageno] == NULL) {
/* 	fprintf(stderr, "%s:%d: shouldn't happen: HTeX_anchorlist[%d] == NULL!\n", __FILE__, __LINE__, pageno); */
	return;
    }
    HTeXAp = HTeX_anchorlist[pageno] + n;

    if ((n < 0) || (n > nHTeX_anchors[pageno])) { /* shouldn't happen ... */
	fprintf(stderr, "%s:%d: Should not happen: non-existing anchor!\n", __FILE__, __LINE__);
	return;
    }
    if (!htex_goback_flag) {  /* don't mark when going back to previous anchor */
	/* erase existing marks */
	clearexpose(&mane, global_x_marker, global_y_marker - 3, 20, 10);

/*	fprintf(stderr, "setting timer for markers 2, %d\n", pixel_conv(HTeXAp->lly)); */
	if (href_timeout_id)
	    XtRemoveTimeOut(href_timeout_id);
	href_timeout_id = XtAddTimeOut(STATUS_SHORT * 1000,
		htex_erase_anchormarker, (XtPointer) NULL);

	global_x_marker = 2;
	global_y_marker = pixel_conv(HTeXAp->lly);
	global_marker_page = pageno;
	if (debug & DBG_HYPER) {
	    fprintf(stderr, "set markers to %d, %d for page %d\n",
		    global_x_marker, global_y_marker, global_marker_page);
	}
    }
    htex_goback_flag = False;
}

/* Following goes back to previous anchor point */
void
htex_goback(void)
{
    int i;

    htex_goback_flag = True;
    global_x_marker = global_y_marker = 0;
    
    if (nHTeX_visited <= 0) {
	XBell(DISP, 10);
	print_statusline(STATUS_SHORT, "At end of history");
	return;	/* There's nowhere to go! */
    }
    if (debug & DBG_ANCHOR) {
	Printf("Currently %d anchors in sequence:\n", nHTeX_visited);
	for (i = 0; i < nHTeX_visited; i++) {
	    Printf("%d file %s, href=%s\n", i,
		   HTeX_visited[i].refname, HTeX_visited[i].href);
	}
    }
    nHTeX_visited--;
    if (strcmp(HTeX_visited[nHTeX_visited].refname, dvi_name) != 0) {
	/* Need to read in old file again! */
	MyStrAllocCopy(&dvi_name, HTeX_visited[nHTeX_visited].refname);
	i = URL_aware;
	/* FIXME: we arrive at perror(dvi_name) when going back
	   to a www-fetched file (e.g. when a link in this file fails).
	   Example:
	   from ~/test.dvi, follow link to http://localhost/test.dvi (which
	   exists) and from there to http://localhost/test.ps (which doesn't
	   exists); at this moment, xdvi will exit(1).
	 */
	URL_aware = FALSE;
	if (debug & DBG_HYPER) {
	    fprintf(stderr, "setting anchor_name to |%s| (%dnd on page %d)\n",
		    HTeX_visited[nHTeX_visited].href,
		    HTeX_visited[nHTeX_visited].which,
		    HTeX_visited[nHTeX_visited].pageno);
	}
	jump_to_anchor = True;
	jump_to_anchorinfo.page = HTeX_visited[nHTeX_visited].pageno;
	jump_to_anchorinfo.num = HTeX_visited[nHTeX_visited].which;
	open_dvi_file();
	URL_aware = i;
	if (dvi_file != NULL) {
	    set_icon_and_title(dvi_name, NULL, NULL, 1);
	    extractbase(dvi_name);
	}
	else {
	    perror(dvi_name);
	    exit(1);
	}
	htex_can_it();
	dvi_reinit();
    }
    htex_loc_on_page(2, HTeX_visited + nHTeX_visited);
    /* taken from keystroke subroutine: */
    htex_can_it();
}

/* Is this a url we recognize? */
int
htex_is_url(const char *href)
{
    /* Fix for bug #478034: don't treat shell escapes as filenames */
    if(href && href[0] == '`') {
	return 0;
    }
    /* Why reinvent the wheel?  Use libwww routines! */
    return HTURL_isAbsolute(href) == YES ? 1 : 0;
}


#if 0
/* FIXME: This appears to be unused.  Was used for retitle? -janl */
static Arg temp_args4[] = {
    {XtNtitle, (XtArgVal) 0},
    {XtNinput, (XtArgVal) True},
};
#endif

/* Can handle href's of form file:?.dvi#name */
/* Actually, supposedly we can handle anything now... */
static void
htex_do_url(Boolean invoke_new, char *href)
{
    if (href == NULL) {
	fprintf(stderr, "%s:%d: Should not happen: href == NULL!", __FILE__, __LINE__);
	return;
    }
    if (debug & DBG_HYPER) {
	fprintf(stderr, "htex_do_url(%s, %s)\n", href, invoke_new ? "extern" : "intern");
    }
    /* Have dvi_name parsed using libwww routines to make sure 
       it doesn't contain invalid characters.  Actually, we may
       assume that HTURL_isAbsolute(href) when this is called, 
       so parsing it relative to URLbase shouldn't matter.
       NOTE: HTURL_isAbsolute also treats things like `file:./btxdoc.dvi'
       as absolute (see also bug #441037).
    */

    if (URLbase != NULL) {
	MyStrAllocCopy(&dvi_name, HTParse(href, URLbase, PARSE_ALL));
    }
    else {
	MyStrAllocCopy(&dvi_name, HTParse(href, "", PARSE_ALL));
    }

    URL_aware = TRUE;
    detach_anchor();
    if (open_www_file(invoke_new, 1) == 0) {
	/* HTTP request was handled externally by invoking some viewer */
	URL_aware = FALSE;
	htex_can_it();
	htex_goback();	/* Go back to where we were! */
	return;
    }
    else {
	/* HTTP request was handled internally by calling open_dvi_file() */
	URL_aware = FALSE;
	dvi_reinit();
	
	if (anchor_name != NULL) {
	    /* This may be unnecessary, since it's already done in
	       check_for_anchor() which is called from draw_page(). */
	    htex_do_loc(invoke_new, anchor_name);
	    free(anchor_name);
	    anchor_name = NULL;
	}
	else {
	    /* This seems to be necessary if there's no anchor_name. */
	    htex_to_page(0);
	}
	return;
    }
}

/* Find the anchor pointed to by ap on the given page */

static void
htex_loc_on_page(int flag, Anchors *ap)
{
    if (debug & DBG_HYPER)
	fprintf(stderr, "htex_loc_on_page\n");
    if (htex_parsedpages[ap->pageno] == 0) {
	if (debug & DBG_HYPER)
	    fprintf(stderr, "parsing page %d\n", ap->pageno);
	if(!htex_parse_page(ap->pageno)) { /* this is a different .dvi file */
	    if (debug & DBG_HYPER)
		fprintf(stderr, "file changed?\n");	/* Parse the needed page! */
	}
    }
    if (debug & DBG_HYPER)
	fprintf(stderr, "htex_to_anchor\n");
    htex_to_anchor(flag, ap->pageno, ap->which);	/* move to anchor i */
}

static void
freeHTeXAnchors(HTeX_Anchor *HTeXAp, int nHTeX)
{
    int i;

    for (i = 0; i < nHTeX; i++) {
	if (HTeXAp[i].type & HTeX_A_NAME)
	    free(HTeXAp[i].name);
	if (HTeXAp[i].type & HTeX_A_HREF)
	    free(HTeXAp[i].href);
    }
}


static void
htex_base(int beginend, char *cp, int pageno)
{
    char *ref, *str;
    UNUSED(pageno);
    
    if (beginend == END)
	return;

    if (!strncasecmp(cp, "base", 4)) {
	cp += 4;
	cp = refscan(cp, &ref, &str);
	if (cp == NULL)
	    return;
	while (isspace(*ref))
	    ref++;
	while (isspace(*str))
	    str++;
	if (strcasecmp(ref, "href") == 0) {
	    cp = str + strlen(str) - 1;	/* Fix end of anchor */
	    while (isspace(*cp))
		cp--;
	    if (*cp == '>')
		cp--;
	    while (isspace(*cp))
		cp--;
	    cp++;
	    *cp = '\0';
	    MyStrAllocCopy(&URLbase, str);	/* Change base */
	    if (debug & DBG_HYPER) {
		Printf("Changing base name to: %s\n", URLbase);
	    }
	}
    }
}

static void
htex_img(int beginend, char *cp, int pageno)
{
    char *ref, *str;
    char fullpathname[1024];

    if (beginend == END)
	return;
    if (pageno != current_page)
	return;	/* Only do when on page */
    if (htex_parsedpages[pageno] == 1)
	return;	/* And first time through */
    if (!strncasecmp(cp, "img", 3)) {
	cp += 3;
	cp = refscan(cp, &ref, &str);
	if (cp == NULL)
	    return;
	while (isspace(*ref))
	    ref++;
	while (isspace(*str))
	    str++;
	if (strcasecmp(ref, "src") == 0) {
	    cp = str + strlen(str) - 1;	/* Fix end of anchor */
	    while (isspace(*cp))
		cp--;
	    if (*cp == '>')
		cp--;
	    while (isspace(*cp))
		cp--;
	    cp++;
	    *cp = '\0';
	    strcpy(fullpathname, str);
	    make_absolute(fullpathname, URLbase, 1024);
	    if (invokeviewer(fullpathname) != 1) {
		do_popup_message(MSG_ERR,
				 /* TODO: prompt user for a viewer name here!! */
				 "You can assign a viewer for the image type \
				 by editing the files \"mime.types\" or \
				 \"mailcap\".",
				 "No appropriate viewer found for image \"%s\"",
				 fullpathname);
	    }
	}
    }
}

#ifdef HTEX
#undef exit	/* just in case */

/* Extract the URL base name from initial file name */
static void
extractbase(char *file)
{
    char *cp;
    static char *cwd = NULL;
    int n;

    if (URLbase != NULL) {
	free(URLbase);
	URLbase = NULL;
    }

    if (strrchr(file, DIR_SEPARATOR) != NULL) {	/* If no /'s then leave dir NULL */
	n = strlen(file);
	if (htex_is_url(file)) {	/* It already is a URL */
	    URLbase = xmalloc((unsigned)(n + 1));
	    Sprintf(URLbase, "%s", file);
	}
	else {	/* Turn it into one: */
	    cwd = GETCWD(cwd, 1024);
	    URLbase = xmalloc((unsigned)(n + 6 + strlen(cwd)));
	    Sprintf(URLbase, "file:%s/%s", cwd, file);
	}
	cp = strrchr(URLbase, DIR_SEPARATOR);
	if (cp == NULL) {
	    fprintf(stderr, "extractbase: should not happen: cp == NULL!\n");
	    free(URLbase);
	    URLbase = NULL;
	    return;
	}
/*              cp[1] = '\0'; */	/* Leave it alone */
    }
}

void
detach_anchor(void)
{
    char *cp;

    cp = strchr(dvi_name, '#');	/* Handle optional # in name */
    if (cp != NULL) {
	*cp = '\0';	/* Terminate filename string */
	cp++;
	while (*cp == '#')
	    cp++;
	if (debug & DBG_HYPER)
	    fprintf(stderr, "detach_anchor: setting anchor_name: %s\n", cp);
	MyStrAllocCopy(&anchor_name, cp);
    }
}

static int
open_www_file(Boolean invoke_new, int set_title)
{
    char *url = NULL;

    if (debug & DBG_HYPER) {
	fprintf(stderr, "before url mangling: %s\n", dvi_name);
    }
    /* open_www_file() may be called from main() at program startup, in
       which case we have to turn dvi_name into an absolute URL. */

    if (URLbase && !HTURL_isAbsolute(dvi_name)) {
	MyStrAllocCopy(&url, dvi_name);
	MyStrAllocCopy(&dvi_name, HTParse(url, "", PARSE_ALL));
	free(url);
    }

    /* Otherwise, we're called with an absoulte URL in dvi_name, either
       from main() or form htex_do_url(). In both cases detach_anchor()
       has been called before open_www_file(). */

    /* FIXME: dvi_name can't contain spaces (striclty correct w.r.t. RFC,
       but wouldn't it be more user-friendly if this wasn't the case?
       probably it's impossible to do, since this is an function internal
       to wwwlib. */
    if (debug & DBG_HYPER) {
	fprintf(stderr, "dvi_name for figure_mime_type: %s\n", dvi_name);
    }
    if (strcmp(figure_mime_type(dvi_name), "application/x-dvi") != 0
	&& strcmp(figure_mime_type(dvi_name), "application/dvi") != 0) {
	/* Try other standard extensions: */
	if (!invokeviewer(dvi_name)) {
	    /* Set the WWW browser on it: */
	    if (browser) {
		int i = 0;
		char *argv[3];
		
		argv[i++] = browser;
		argv[i++] = dvi_name;
		argv[i++] = NULL;

		fork_process(argv[0], argv);
		return 0;
	    }
	    else {
		do_popup_message(MSG_ERR,
				 "Please set the environment variable WWWBROWSER \
or use the command line option \"-browser\"; otherwise URLs that cannot be handled by xdvi \
will not work. See the xdvi man page for more detailed information.",
				 "No browser specified - could not handle URL \"%s\".", dvi_name);
	    }
	}
    }	/* application/x-dvi */
    else {
	if (invoke_new) {
	    /* External viewer */
	    if (anchor_name != NULL) {
		invokedviviewer(dvi_name, anchor_name, 0);
		free(anchor_name);
		anchor_name = NULL;
	    }
	    else {
		invokedviviewer(dvi_name, NULL, 0);
	    }
	    return 0;
	}
	else {
	    /* Dvi file, ='internal viewer' */
	    open_dvi_file();
	    if (dvi_file != NULL) {
		set_icon_and_title(dvi_name, NULL, NULL, set_title);
		extractbase(dvi_name);
		return 1;
	    }
	}
    }
    return 0;
}

#endif /* HTEX */
#endif /* XHDVI || HTEX */
