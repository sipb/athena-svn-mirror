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

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL PAUL VOJTA OR ANY OTHER AUTHOR OF THIS SOFTWARE BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

NOTE: xdvi is based on prior work as noted in the modification history, below.

\*========================================================================*/

/*
 * DVI previewer for X.
 *
 * Eric Cooper, CMU, September 1985.
 *
 * Code derived from dvi-imagen.c.
 *
 * Modification history:
 * 1/1986	Modified for X.10	--Bob Scheifler, MIT LCS.
 * 7/1988	Modified for X.11	--Mark Eichin, MIT
 * 12/1988	Added 'R' option, toolkit, magnifying glass
 *					--Paul Vojta, UC Berkeley.
 * 2/1989	Added tpic support	--Jeffrey Lee, U of Toronto
 * 4/1989	Modified for System V	--Donald Richardson, Clarkson Univ.
 * 3/1990	Added VMS support	--Scott Allendorf, U of Iowa
 * 7/1990	Added reflection mode	--Michael Pak, Hebrew U of Jerusalem
 * 1/1992	Added greyscale code	--Till Brychcy, Techn. Univ. Muenchen
 *					  and Lee Hetherington, MIT
 * 7/1992       Added extra menu buttons--Nelson H. F. Beebe <beebe@math.utah.edu>
 * 4/1994	Added DPS support, bounding box
 *					--Ricardo Telichevesky
 *					  and Luis Miguel Silveira, MIT RLE.
 * 2/1995       Added rulers support    --Nelson H. F. Beebe <beebe@math.utah.edu>
 * 1/2001	Added source specials	--including ideas from Stefan Ulrich,
 *					  U Munich
 *
 *	Compilation options:
 *	VMS	compile for VMS
 *	NOTOOL	compile without toolkit
 *	BUTTONS	compile with buttons on the side of the window (needs toolkit)
 *	WORDS_BIGENDIAN	store bitmaps internally with most significant bit first
 *	BMTYPE	store bitmaps in unsigned BMTYPE
 *	BMBYTES	sizeof(unsigned BMTYPE)
 *	ALTFONT	default for -altfont option
 *	SHRINK	default for -s option (shrink factor)
 *	MFMODE	default for -mfmode option
 *	A4	use European size paper, and change default dimension to cm
 *	TEXXET	support reflection dvi codes (right-to-left typesetting)
 *	GREY	use grey levels to shrink fonts
 *	PS_GS	use Ghostscript to render pictures/bounding boxes
 *	PS_DPS	use display postscript to render pictures/bounding boxes
 *	PS_NEWS	use the NeWS server to render pictures/bounding boxes
 *	GS_PATH	path to call the Ghostscript interpreter by
 *	GRID	grid in magnification windows enabled
 *	HTEX	hypertex enabled.  CURRENTLY DOESN't COMPILE WITHOUT.
 *		Will be fixed in next major release, so don't care for now.
 */

#if 0
static char copyright[] =
    "@(#) Copyright (c) 1994-2001 Paul Vojta.  All rights reserved.\n";
#endif

#define	EXTERN
#define	INIT(x)	=x


#include "xdvi-config.h"
#include "c-openmx.h"
#include "kpathsea/c-ctype.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-pathch.h"
#include "kpathsea/c-stat.h"
#include "kpathsea/proginit.h"
#include "kpathsea/progname.h"
#include "kpathsea/tex-file.h"
#include "kpathsea/tex-hush.h"
#include "kpathsea/tex-make.h"

/* added SU */
#include "string-utils.h"
#include "kpathsea/c-errno.h"
#include <signal.h>

#ifdef HTEX
#include "wwwconf.h"
#include "WWWLib.h"
#include "WWWInit.h"
#include "WWWCache.h"
#include "HTEscape.h"
#endif

/* alternative string definitions if string concatenation is not available */
#ifndef MAKING_HEADER
#include "krheader.h"
#endif

#include "xserver-info.h"

#include <math.h>	/* sometimes includes atof() */
#include <ctype.h>

/* #include <X11/Xmd.h>	*/ /* get WORD64 and LONG64 */

#ifndef WORD64
#ifdef LONG64
typedef unsigned int xuint32;
#else
typedef unsigned long xuint32;
#endif
#endif

#ifndef	ALTFONT
#define	ALTFONT	"cmr10"
#endif

#ifndef	SHRINK
#define	SHRINK	8
#endif

#ifndef	MFMODE
#define	MFMODE	NULL
#endif

#undef MKTEXPK
#define MKTEXPK MAKEPK

#if	defined(PS_GS) && !defined(GS_PATH)
#define	GS_PATH	"gs"
#endif

#if A4
#define	DEFAULT_PAPER		"a4"
#else
#define	DEFAULT_PAPER		"us"
#endif

#include "version.h"


#ifdef	X_NOT_STDC_ENV
#ifndef	atof
extern double atof ARGS((_Xconst char *));
#endif
#endif

/* Xlib and Xutil are already included */
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include "xdvi.icon"

#ifdef	TOOLKIT

#ifdef	OLD_X11_TOOLKIT
#include <X11/Atoms.h>
#else /* not OLD_X11_TOOLKIT */
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#endif /* not OLD_X11_TOOLKIT */

#include <X11/Shell.h>	/* needed for def. of XtNiconX */

#ifndef	XtSpecificationRelease
#define	XtSpecificationRelease	0
#endif
#if	XtSpecificationRelease >= 4

#ifndef MOTIF
# if XAW3D
#  include <X11/Xaw3d/Viewport.h>
# else
#  include <X11/Xaw/Viewport.h>
# endif
# ifdef HTEX
#  include <X11/Xaw/AsciiText.h>
#  include <X11/Xaw/Box.h>
#  include <X11/Xaw/Command.h>
#  include <X11/Xaw/Dialog.h>
#  include <X11/Xaw/Form.h>
#  include <X11/Xaw/Paned.h>
#  include <X11/Xaw/Scrollbar.h>
# endif	/* HTEX */
# define	VPORT_WIDGET_CLASS	viewportWidgetClass
# define	DRAW_WIDGET_CLASS	drawWidgetClass
#else /* MOTIF */
# include <Xm/MainW.h>
# include <Xm/ToggleB.h>
# include <Xm/RowColumn.h>
# include <Xm/MenuShell.h>
# include <Xm/DrawingA.h>
# define	VPORT_WIDGET_CLASS	xmMainWindowWidgetClass
# define	DRAW_WIDGET_CLASS	xmDrawingAreaWidgetClass
#endif /* MOTIF */


#ifdef	BUTTONS
#ifndef MOTIF
# if XAW3D
#  include <X11/Xaw3d/Command.h>
# else
#  include <X11/Xaw/Command.h>
# endif
#define	FORM_WIDGET_CLASS	formWidgetClass
#else /* MOTIF */
#include <Xm/Form.h>
#define	FORM_WIDGET_CLASS	xmFormWidgetClass
#endif /* MOTIF */
#endif /* BUTTONS */

#else /* XtSpecificationRelease < 4 */

#if NeedFunctionPrototypes
typedef void *XtPointer;
#else
typedef char *XtPointer;
#endif

#include <X11/Viewport.h>
#ifdef HTEX
#include <X11/AsciiText.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Form.h>
#include <X11/Paned.h>
#include <X11/Scroll.h>
#include <X11/VPaned.h>
#include <X11/Scrollbar.h>
#endif
#define	VPORT_WIDGET_CLASS	viewportWidgetClass
#define	DRAW_WIDGET_CLASS	drawWidgetClass
#ifdef	BUTTONS
#include <X11/Command.h>
#define	FORM_WIDGET_CLASS	formWidgetClass
#endif

#endif /* XtSpecificationRelease */

#else /* not TOOLKIT */

typedef int Position;

#endif /* not TOOLKIT */

#if XlibSpecificationRelease < 5
typedef char *XPointer;
#define	XScreenNumberOfScreen(s)	((s) - ScreenOfDisplay(DISP, 0))
#endif

/*
 *	Cursors and masks for dragging operations.
 */

#define drag_vert_width 5
#define drag_vert_height 15
#define drag_vert_x_hot 2
#define drag_vert_y_hot 7
static unsigned char drag_vert_bits[] = {
    0x04, 0x0e, 0x15, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x15, 0x0e, 0x04
};
static unsigned char drag_vert_mask[] = {
    0x0e, 0x1f, 0x1f, 0x1f, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x1f,
    0x1f, 0x1f, 0x0e
};

#define drag_horiz_width 15
#define drag_horiz_height 5
#define drag_horiz_x_hot 7
#define drag_horiz_y_hot 2
static unsigned char drag_horiz_bits[] = {
    0x04, 0x10, 0x02, 0x20, 0xff, 0x7f, 0x02, 0x20, 0x04, 0x10
};
static unsigned char drag_horiz_mask[] = {
    0x0e, 0x38, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x0e, 0x38
};

#define drag_omni_width 15
#define drag_omni_height 15
#define drag_omni_x_hot 7
#define drag_omni_y_hot 7
static unsigned char drag_omni_bits[] = {
    0x80, 0x00, 0xc0, 0x01, 0xa0, 0x02, 0x80, 0x00, 0x80, 0x00, 0x84, 0x10,
    0x82, 0x20, 0xff, 0x7f, 0x82, 0x20, 0x84, 0x10, 0x80, 0x00, 0x80, 0x00,
    0xa0, 0x02, 0xc0, 0x01, 0x80, 0x00
};
static unsigned char drag_omni_mask[] = {
    0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07, 0xf0, 0x07, 0xfc, 0x1f, 0xfe, 0x3f,
    0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf0, 0x07,
    0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01
};

#ifdef	VMS
/*
 * Magnifying glass cursor
 *
 * Developed by Tom Sawyer, April 1990
 * Contibuted by Hunter Goatley, January 1991
 *
 */

#define mag_glass_width 16
#define mag_glass_height 16
#define mag_glass_x_hot 6
#define mag_glass_y_hot 6
static char mag_glass_bits[] = {
    0xf8, 0x03, 0x0c, 0x06, 0xe2, 0x09, 0x13, 0x1a, 0x01, 0x14, 0x01, 0x14,
    0x01, 0x10, 0x01, 0x10, 0x01, 0x10, 0x03, 0x10, 0x02, 0x18, 0x0c, 0x34,
    0xf8, 0x6f, 0x00, 0xd8, 0x00, 0xb0, 0x00, 0xe0
};
#include <decw$cursor.h>	/* Include the DECWindows cursor symbols */
static int DECWCursorFont;	/* Space for the DECWindows cursor font  */
static Pixmap MagnifyPixmap;	/* Pixmap to hold our special mag-glass  */
#include <X11/Xresource.h>	/* Motif apparently needs this one */
#endif /* VMS */

/*
 * Command line flags.
 */

static Dimension bwidth = 2;

#define	fore_Pixel	resource._fore_Pixel
#define	back_Pixel	resource._back_Pixel
#ifdef	TOOLKIT
struct _resource resource;
#define	brdr_Pixel	resource._brdr_Pixel
#define	hl_Pixel	resource._hl_Pixel
#define	cr_Pixel	resource._cr_Pixel
#else /* not TOOLKIT */
static _Xconst char *brdr_color;
static _Xconst char *high_color;
static _Xconst char *curs_color;
static Pixel hl_Pixel, cr_Pixel;
static Pixel rule_pixel;
#endif /* not TOOLKIT */

struct mg_size_rec mg_size[5] = { {200, 150}, {400, 250}, {700, 500},
{1000, 800}, {1200, 1200}
};

static char *curr_page;
char *global_dvi_name;	/* save dvi file name, including eventually the path, always with .dvi extension */


struct WindowRec mane = { (Window) 0, 1, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0 };
struct WindowRec alt = { (Window) 0, 1, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0 };
/*	currwin is temporary storage except for within redraw() */
struct WindowRec currwin = { (Window) 0, 1, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0 };

#ifdef	lint
#ifdef	TOOLKIT
WidgetClass widgetClass;
#ifndef MOTIF
WidgetClass viewportWidgetClass;
#ifdef	BUTTONS
WidgetClass formWidgetClass, compositeWidgetClass, commandWidgetClass;
#endif /* BUTTONS */
#else /* MOTIF */
WidgetClass xmMainWindowWidgetClass, xmDrawingAreaWidgetClass;
#ifdef	BUTTONS
WidgetClass xmFormWidgetClass, xmBulletinBoardWidgetClass;
WidgetClass xmPushButtonWidgetClass;
#endif /* BUTTONS */
#endif /* MOTIF */
#endif /* TOOLKIT */
#endif /* lint */

static char *atom_names[] = { "XDVI WINDOWS", "DVI NAME", "SRC GOTO" };

/*
 *	Data for options processing
 */

static _Xconst char silent[] = " ";	/* flag value for usage() */

static _Xconst char subst[] = "x";	/* another flag value */

static _Xconst char *subst_val[] = {
#ifdef	BUTTONS
    "-shrinkbutton[1-9] <shrink>",
#endif
    "-mgs[n] <size>"
};

#ifdef	TOOLKIT

static XrmOptionDescRec options[] = {
    {"-regression", ".regression", XrmoptionNoArg, (XPointer) "on"},
    {"+regression", ".regression", XrmoptionNoArg, (XPointer) "off"},
    {"-s", ".shrinkFactor", XrmoptionSepArg, (XPointer) NULL},
#ifndef	VMS
    {"-S", ".densityPercent", XrmoptionSepArg, (XPointer) NULL},
#endif
    {"-density", ".densityPercent", XrmoptionSepArg, (XPointer) NULL},
#ifdef	GREY
    {"-nogrey", ".grey", XrmoptionNoArg, (XPointer) "off"},
    {"+nogrey", ".grey", XrmoptionNoArg, (XPointer) "on"},
    {"-gamma", ".gamma", XrmoptionSepArg, (XPointer) NULL},
    {"-install", ".install", XrmoptionNoArg, (XPointer) "on"},
    {"-noinstall", ".install", XrmoptionNoArg, (XPointer) "off"},
#endif
    {"-rulecolor", ".ruleColor", XrmoptionSepArg, (XPointer) NULL},
    {"-p", ".pixelsPerInch", XrmoptionSepArg, (XPointer) NULL},
    {"-margins", ".Margin", XrmoptionSepArg, (XPointer) NULL},
    {"-sidemargin", ".sideMargin", XrmoptionSepArg, (XPointer) NULL},
    {"-topmargin", ".topMargin", XrmoptionSepArg, (XPointer) NULL},
    {"-offsets", ".Offset", XrmoptionSepArg, (XPointer) NULL},
    {"-xoffset", ".xOffset", XrmoptionSepArg, (XPointer) NULL},
    {"-yoffset", ".yOffset", XrmoptionSepArg, (XPointer) NULL},
    {"-paper", ".paper", XrmoptionSepArg, (XPointer) NULL},
    {"-altfont", ".altFont", XrmoptionSepArg, (XPointer) NULL},
#ifdef MKTEXPK
    {"-nomakepk", ".makePk", XrmoptionNoArg, (XPointer) "off"},
    {"+nomakepk", ".makePk", XrmoptionNoArg, (XPointer) "on"},
#endif
    {"-mfmode", ".mfMode", XrmoptionSepArg, (XPointer) NULL},
    {"-editor", ".editor", XrmoptionSepArg, (XPointer) NULL},
#ifdef T1LIB
    {"+not1lib", ".not1lib", XrmoptionNoArg, (XPointer) "on"},
    {"-not1lib", ".not1lib", XrmoptionNoArg, (XPointer) "off"},
#endif    
    {"-sourceposition", ".sourcePosition", XrmoptionSepArg, (XPointer) NULL},
    {"-l", ".listFonts", XrmoptionNoArg, (XPointer) "on"},
    {"+l", ".listFonts", XrmoptionNoArg, (XPointer) "off"},
#ifdef	BUTTONS
    {"-expert", ".expert", XrmoptionNoArg, (XPointer) "on"},
    {"+expert", ".expert", XrmoptionNoArg, (XPointer) "off"},
    {"+statusline", ".statusline", XrmoptionNoArg, (XPointer) "off"},
    {"-statusline", ".statusline", XrmoptionNoArg, (XPointer) "on"},
    {"-shrinkbutton1", ".shrinkButton1", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton2", ".shrinkButton2", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton3", ".shrinkButton3", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton4", ".shrinkButton4", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton5", ".shrinkButton5", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton6", ".shrinkButton6", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton7", ".shrinkButton7", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton8", ".shrinkButton8", XrmoptionSepArg, (XPointer) NULL},
    {"-shrinkbutton9", ".shrinkButton9", XrmoptionSepArg, (XPointer) NULL},
#endif
    {"-mgs", ".magnifierSize1", XrmoptionSepArg, (XPointer) NULL},
    {"-mgs1", ".magnifierSize1", XrmoptionSepArg, (XPointer) NULL},
    {"-mgs2", ".magnifierSize2", XrmoptionSepArg, (XPointer) NULL},
    {"-mgs3", ".magnifierSize3", XrmoptionSepArg, (XPointer) NULL},
    {"-mgs4", ".magnifierSize4", XrmoptionSepArg, (XPointer) NULL},
    {"-mgs5", ".magnifierSize5", XrmoptionSepArg, (XPointer) NULL},
    {"-warnspecials", ".warnSpecials", XrmoptionNoArg, (XPointer) "on"},
    {"+warnspecials", ".warnSpecials", XrmoptionNoArg, (XPointer) "off"},
    {"-hush", ".Hush", XrmoptionNoArg, (XPointer) "on"},
    {"+hush", ".Hush", XrmoptionNoArg, (XPointer) "off"},
    {"-hushchars", ".hushLostChars", XrmoptionNoArg, (XPointer) "on"},
    {"+hushchars", ".hushLostChars", XrmoptionNoArg, (XPointer) "off"},
    {"-hushchecksums", ".hushChecksums", XrmoptionNoArg, (XPointer) "on"},
    {"+hushchecksums", ".hushChecksums", XrmoptionNoArg, (XPointer) "off"},
    {"-hushstdout", ".hushStdout", XrmoptionNoArg, (XPointer) "on"},
    {"+hushstdout", ".hushStdout", XrmoptionNoArg, (XPointer) "off"},
    {"-safer", ".safer", XrmoptionNoArg, (XPointer) "on"},
    {"+safer", ".safer", XrmoptionNoArg, (XPointer) "off"},
    {"-fg", ".foreground", XrmoptionSepArg, (XPointer) NULL},
    {"-foreground", ".foreground", XrmoptionSepArg, (XPointer) NULL},
    {"-bg", ".background", XrmoptionSepArg, (XPointer) NULL},
    {"-background", ".background", XrmoptionSepArg, (XPointer) NULL},
    {"-hl", ".highlight", XrmoptionSepArg, (XPointer) NULL},
    {"-cr", ".cursorColor", XrmoptionSepArg, (XPointer) NULL},
    {"-icongeometry", ".iconGeometry", XrmoptionSepArg, (XPointer) NULL},
    {"-keep", ".keepPosition", XrmoptionNoArg, (XPointer) "on"},
    {"+keep", ".keepPosition", XrmoptionNoArg, (XPointer) "off"},
    {"-copy", ".copy", XrmoptionNoArg, (XPointer) "on"},
    {"+copy", ".copy", XrmoptionNoArg, (XPointer) "off"},
    {"-thorough", ".thorough", XrmoptionNoArg, (XPointer) "on"},
    {"+thorough", ".thorough", XrmoptionNoArg, (XPointer) "off"},
    {"-wheelunit", ".wheelUnit", XrmoptionSepArg, (XPointer) NULL},
#if	PS
    {"-nopostscript", ".postscript", XrmoptionNoArg, (XPointer) "off"},
    {"+nopostscript", ".postscript", XrmoptionNoArg, (XPointer) "on"},
    {"-noscan", ".prescan", XrmoptionNoArg, (XPointer) "off"},
    {"+noscan", ".prescan", XrmoptionNoArg, (XPointer) "on"},
    {"-allowshell", ".allowShell", XrmoptionNoArg, (XPointer) "on"},
    {"+allowshell", ".allowShell", XrmoptionNoArg, (XPointer) "off"},
#ifdef	PS_DPS
    {"-nodps", ".dps", XrmoptionNoArg, (XPointer) "off"},
    {"+nodps", ".dps", XrmoptionNoArg, (XPointer) "on"},
#endif
#ifdef	PS_NEWS
    {"-nonews", ".news", XrmoptionNoArg, (XPointer) "off"},
    {"+nonews", ".news", XrmoptionNoArg, (XPointer) "on"},
#endif
#ifdef	PS_GS
    {"-noghostscript", ".ghostscript", XrmoptionNoArg, (XPointer) "off"},
    {"+noghostscript", ".ghostscript", XrmoptionNoArg, (XPointer) "on"},
    {"-nogssafer", ".gsSafer", XrmoptionNoArg, (XPointer) "off"},
    {"+nogssafer", ".gsSafer", XrmoptionNoArg, (XPointer) "on"},
    {"-gsalpha", ".gsAlpha", XrmoptionNoArg, (XPointer) "on"},
    {"+gsalpha", ".gsAlpha", XrmoptionNoArg, (XPointer) "off"},
    {"-interpreter", ".interpreter", XrmoptionSepArg, (XPointer) NULL},
    {"-gspalette", ".palette", XrmoptionSepArg, (XPointer) NULL},
#endif
#endif /* PS */
    {"-debug", ".debugLevel", XrmoptionSepArg, (XPointer) NULL},
    {"-version", ".version", XrmoptionNoArg, (XPointer) "on"},
    {"+version", ".version", XrmoptionNoArg, (XPointer) "off"},
#ifdef HTEX
    {"-underlink", ".underLink", XrmoptionNoArg, (XPointer) "on"},
    {"+underlink", ".underLink", XrmoptionNoArg, (XPointer) "off"},
    {"-browser", ".wwwBrowser", XrmoptionSepArg, (XPointer) NULL},
    {"-base", ".urlBase", XrmoptionSepArg, (XPointer) NULL},
#endif
};


#if MAKING_HEADER
    "XDVI_CC_CONCAT_BEGIN1"
#endif

#ifdef HAVE_CC_CONCAT
static _Xconst char base_translations[] = "" /* note: Control keys always first */
    "^<Key>c:quit()\n"
    "^<Key>d:quit()\n"
    "^<Key>f:select-dvi-file()\n"
    "^<Key>m:forward-page()\n"
    "^<Key>j:forward-page()\n"
    "^<Key>l:forward-page(0)\n"
    "^<Key>h:back-page()\n"
    "^<Key>v:show-source-specials()\n"
    "^<Key>x:source-what-special()\n"
    "^<Key>p:show-display-attributes()\n"
#ifdef VMS
    "^<Key>z:quit()\n"
#endif
    "\"q\":quit()\n"
    "\"h\":help()\n"
#ifdef HTEX    
    "\"i\":htex-anchorinfo()\n"
    "\"B\":htex-back()\n"
#endif    
    "\"?\":help()\n"
    "\"d\":down()\n"
    "\"f\":forward-page()\n"
    "\"n\":forward-page()\n"
    "\" \":down-or-next()\n"
    "<Key>Return:forward-page()\n"
    "\"p\":back-page()\n"
    "\"b\":back-page()\n"
    "<Key>Delete:up-or-previous()\n"
    "<Key>BackSpace:up-or-previous()\n"
#if MOTIF
    "<Key>osfDelete:up-or-previous()\n"
    "<Key>osfBackSpace:up-or-previous()\n"
#endif
    "\"P\":declare-page-number()\n"
#ifdef GRID
    "\"D\":toggle-grid-mode()\n"
#endif
    "\"g\":goto-page()\n"
    "\">\":goto-page()\n"
    "\"<\":goto-page(1)\n"
    "\"\\^\":home()\n"
    "\"c\":center()\n"
    "\"k\":set-keep-flag()\n"
    "\"l\":left()\n"
    "\"r\":right()\n"
    "\"t\":switch-magnifier-units()\n"
    "\"u\":up()\n"
    "\"M\":set-margins()\n"
    "\"s\":set-shrink-factor()\n"
    "\"S\":set-density()\n"
    "<Key>Home:home()\n"
    "<Key>Left:left(0.015)\n"
    "<Key>Up:up(0.015)\n"
    "<Key>Right:right(0.015)\n"
    "<Key>Down:down(0.015)\n"
    "<Key>Prior:back-page()\n"
    "<Key>Next:forward-page()\n"
#ifdef XK_KP_Left
    "<Key>KP_Home:home()\n"
    "<Key>KP_Left:left()\n"
    "<Key>KP_Up:up()\n"
    "<Key>KP_Right:right()\n"
    "<Key>KP_Down:down()\n"
    "<Key>KP_Prior:back-page()\n"
    "<Key>KP_Next:forward-page()\n"
    "<Key>KP_Delete:up-or-previous()\n"
    "<Key>KP_Enter:forward-page()\n"
#endif
#if GREY
    "\"G\":set-greyscaling()\n"
#endif
#if PS
    "\"v\":set-ps()\n"
#endif
#if PS_GS
    "\"V\":set-gs-alpha()\n"
#endif /* PS_GS */
#if BUTTONS
    "\"x\":set-expert-mode()\n"
#endif
    "<Key>Escape:discard-number()\n"	/* Esc */
    "s<Btn1Down>:drag(+)\n"
    "s<Btn2Down>:drag(|)\n"
    "s<Btn3Down>:drag(-)\n"
    "^<Btn1Down>:source-special()\n"
    "<Btn1Down>:do-href()magnifier(*1)\n"
    "<Btn2Down>:do-href-newwindow()magnifier(*2)\n"
    "<Btn3Down>:magnifier(*3)\n"
    "<Btn4Down>:magnifier(*4)\n"
    "<Btn5Down>:magnifier(*5)\n"
    "\"R\":reread-dvi-file()\n"
    "";


_Xconst char default_button_config[] = ""
    "Quit:quit()\n"
#ifdef SELFILE
    "Open:select-dvi-file()\n"
#endif
    "Reread:reread-dvi-file()\n"
    "Help:help()\n\n"
    "Full size:set-shrink-factor(1)\n"
    "$%%:shrink-to-dpi(150)\n"
    "$%%:shrink-to-dpi(100)\n"
    "$%%:shrink-to-dpi(75)\n\n"
    "First:goto-page(1)\n"
    "Page-10:back-page(10)\n"
    "Page-5:back-page(5)\n"
    "Prev:back-page(1)\n\n"
    "Next:forward-page(1)\n"
    "Page+5:forward-page(5)\n"
    "Page+10:forward-page(10)\n"
    "Last:goto-page()\n\n"
#if PS
    "View PS:set-ps(toggle)\n"
#endif
#ifdef HTEX
    "Back:htex-back()\n"
#endif
    "";

#endif /* HAVE_CC_CONCAT */

#if MAKING_HEADER
    "XDVI_CC_CONCAT_END1"
#endif

#define	offset(field)	XtOffsetOf(struct _resource, field)
static int base_tick_length = 4;

static char XtRBool3[] = "Bool3";	/* resource for Bool3 */

static XtResource application_resources[] = {
#if CFGFILE
    {"name", "Name", XtRString, sizeof(char *),
     offset(progname), XtRString, (XtPointer) NULL},
#endif
    {"regression", "Regression", XtRBoolean, sizeof(Boolean),
     offset(regression), XtRString, "false"},
    {"shrinkFactor", "ShrinkFactor", XtRInt, sizeof(int),
     offset(shrinkfactor), XtRImmediate, (XtPointer) SHRINK},
/*  offset(shrinkfactor), XtRString, SHRINK}, */
    {"delayRulers", "DelayRulers", XtRBoolean, sizeof(Boolean),
     offset(_delay_rulers), XtRString, "true"}
    ,
    {"densityPercent", "DensityPercent", XtRInt, sizeof(int),
     offset(_density), XtRString, "40"},
    {"mainTranslations", "MainTranslations", XtRString, sizeof(char *),
     offset(main_translations), XtRString, (XtPointer) NULL},
    {"wheelTranslations", "WheelTranslations", XtRString, sizeof(char *),
     offset(wheel_translations), XtRString,
     (XtPointer) "<Btn4Down>:wheel(-1.)\n\
	 <Btn5Down>:wheel(1.)"},
    {"wheelUnit", "WheelUnit", XtRInt, sizeof(int),
     offset(wheel_unit), XtRImmediate, (XtPointer) 80},
#ifdef	GREY
    {"gamma", "Gamma", XtRFloat, sizeof(float),
     offset(_gamma), XtRString, "1"},
#endif
    {"pixelsPerInch", "PixelsPerInch", XtRInt, sizeof(int),
     offset(_pixels_per_inch), XtRImmediate, (XtPointer) BDPI},
    {"sideMargin", "Margin", XtRString, sizeof(char *),
     offset(sidemargin), XtRString, (XtPointer) NULL},
    {"tickLength", "TickLength", XtRInt, sizeof(int),
     offset(_tick_length), XtRInt, (XtPointer) & base_tick_length},
    {"tickUnits", "TickUnits", XtRString, sizeof(char *),
     offset(_tick_units), XtRString, "pt"},
    {"topMargin", "Margin", XtRString, sizeof(char *),
     offset(topmargin), XtRString, (XtPointer) NULL},
    {"xOffset", "Offset", XtRString, sizeof(char *),
     offset(xoffset), XtRString, (XtPointer) NULL},
    {"yOffset", "Offset", XtRString, sizeof(char *),
     offset(yoffset), XtRString, (XtPointer) NULL},
    {"paper", "Paper", XtRString, sizeof(char *),
     offset(paper), XtRString, (XtPointer) DEFAULT_PAPER},
    {"altFont", "AltFont", XtRString, sizeof(char *),
     offset(_alt_font), XtRString, (XtPointer) ALTFONT},
    {"makePk", "MakePk", XtRBoolean, sizeof(Boolean),
     offset(makepk), XtRString,
#if MAKE_TEX_PK_BY_DEFAULT
     "true"
#else
     "false"
#endif
     },
    {"mfMode", "MfMode", XtRString, sizeof(char *),
     offset(mfmode), XtRString, MFMODE},
    {"editor", "Editor", XtRString, sizeof(char *),
     offset(editor), XtRString, (XtPointer) NULL},
#ifdef T1LIB    
    {"not1lib", "T1lib", XtRBoolean, sizeof(Boolean),
     offset(t1lib), XtRString, "true"},
#endif
    {"sourcePosition", "SourcePosition", XtRString, sizeof(char *),
     offset(src_pos), XtRString, (XtPointer) NULL},
    {"listFonts", "ListFonts", XtRBoolean, sizeof(Boolean),
     offset(_list_fonts), XtRString, "false"},
    {"reverseVideo", "ReverseVideo", XtRBoolean, sizeof(Boolean),
     offset(reverse), XtRString, "false"},
    {"warnSpecials", "WarnSpecials", XtRBoolean, sizeof(Boolean),
     offset(_warn_spec), XtRString, "false"},
    {"hushLostChars", "Hush", XtRBoolean, sizeof(Boolean),
     offset(_hush_chars), XtRString, "false"},
    {"hushChecksums", "Hush", XtRBoolean, sizeof(Boolean),
     offset(_hush_chk), XtRString, "false"},
    {"hushStdout", "HushStdout", XtRBoolean, sizeof(Boolean),
     offset(_hush_stdout), XtRString, "false"},
    {"safer", "Safer", XtRBoolean, sizeof(Boolean),
     offset(safer), XtRString, "false"},
#ifdef VMS
    {"foreground", "Foreground", XtRString, sizeof(char *),
     offset(fore_color), XtRString, (XtPointer) NULL},
    {"background", "Background", XtRString, sizeof(char *),
     offset(back_color), XtRString, (XtPointer) NULL},
#endif
    {"iconGeometry", "IconGeometry", XtRString, sizeof(char *),
     offset(icon_geometry), XtRString, (XtPointer) NULL},
    {"keepPosition", "KeepPosition", XtRBoolean, sizeof(Boolean),
     offset(keep_flag), XtRString, "false"},
#if	PS
    {"postscript", "Postscript", XtRBoolean, sizeof(Boolean),
     offset(_postscript), XtRString, "true"},
    {"prescan", "Prescan", XtRBoolean, sizeof(Boolean),
     offset(prescan), XtRString, "true"},
    {"allowShell", "AllowShell", XtRBoolean, sizeof(Boolean),
     offset(allow_shell), XtRString, "false"},
#ifdef	PS_DPS
    {"dps", "DPS", XtRBoolean, sizeof(Boolean),
     offset(useDPS), XtRString, "true"},
#endif
#ifdef	PS_NEWS
    {"news", "News", XtRBoolean, sizeof(Boolean),
     offset(useNeWS), XtRString, "true"},
#endif
#ifdef	PS_GS
    {"ghostscript", "Ghostscript", XtRBoolean, sizeof(Boolean),
     offset(useGS), XtRString, "true"},
    {"gsSafer", "Safer", XtRBoolean, sizeof(Boolean),
     offset(gs_safer), XtRString, "true"},
    {"gsAlpha", "Alpha", XtRBoolean, sizeof(Boolean),
     offset(gs_alpha), XtRString, "false"},
    {"interpreter", "Interpreter", XtRString, sizeof(char *),
     offset(gs_path), XtRString, (XtPointer) GS_PATH},
    {"palette", "Palette", XtRString, sizeof(char *),
     offset(gs_palette), XtRString, (XtPointer) "Color"},
#endif
#endif /* PS */
    {"copy", "Copy", XtRBoolean, sizeof(Boolean),
     offset(copy), XtRString, "false"},
    {"thorough", "Thorough", XtRBoolean, sizeof(Boolean),
     offset(thorough), XtRString, "false"},
    {"debugLevel", "DebugLevel", XtRString, sizeof(char *),
     offset(debug_arg), XtRString, (XtPointer) NULL},
    {"version", "Version", XtRBoolean, sizeof(Boolean),
     offset(version_flag), XtRString, "false"},
#ifdef	BUTTONS
    {"expert", "Expert", XtRBoolean, sizeof(Boolean),
     offset(expert), XtRString, "false"},
    {"statusline", "Statusline", XtRBoolean, sizeof(Boolean),
     offset(statusline), XtRString, "true"},
    {"buttonTranslations", "ButtonTranslations", XtRString, sizeof(char *),
     offset(button_translations), XtRString, (XtPointer) default_button_config},
    {"shrinkButton1", "ShrinkButton1", XtRInt, sizeof(int),
     offset(shrinkbutton[0]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton2", "ShrinkButton2", XtRInt, sizeof(int),
     offset(shrinkbutton[1]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton3", "ShrinkButton3", XtRInt, sizeof(int),
     offset(shrinkbutton[2]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton4", "ShrinkButton4", XtRInt, sizeof(int),
     offset(shrinkbutton[3]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton5", "ShrinkButton5", XtRInt, sizeof(int),
     offset(shrinkbutton[4]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton6", "ShrinkButton6", XtRInt, sizeof(int),
     offset(shrinkbutton[5]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton7", "ShrinkButton7", XtRInt, sizeof(int),
     offset(shrinkbutton[6]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton8", "ShrinkButton8", XtRInt, sizeof(int),
     offset(shrinkbutton[7]), XtRImmediate, (XtPointer) 0},
    {"shrinkButton9", "ShrinkButton9", XtRInt, sizeof(int),
     offset(shrinkbutton[8]), XtRImmediate, (XtPointer) 0},
    {"buttonSideSpacing", "ButtonSpacing", XtRInt, sizeof(int),
     offset(btn_side_spacing), XtRImmediate, (XtPointer) 8},
    {"buttonTopSpacing", "ButtonSpacing", XtRInt, sizeof(int),
     offset(btn_top_spacing), XtRImmediate, (XtPointer) 30},
    {"buttonBetweenSpacing", "ButtonSpacing", XtRInt, sizeof(int),
     offset(btn_between_spacing), XtRImmediate, (XtPointer) 8},
    {"buttonBetweenExtra", "ButtonSpacing", XtRInt, sizeof(int),
     offset(btn_between_extra), XtRImmediate, (XtPointer) 24},
    {"buttonBorderWidth", "BorderWidth", XtRInt, sizeof(int),
     offset(btn_border_width), XtRImmediate, (XtPointer) 1},
#endif
    {"magnifierSize1", "MagnifierSize", XtRString, sizeof(char *),
     offset(mg_arg[0]), XtRString, (XtPointer) NULL},
    {"magnifierSize2", "MagnifierSize", XtRString, sizeof(char *),
     offset(mg_arg[1]), XtRString, (XtPointer) NULL},
    {"magnifierSize3", "MagnifierSize", XtRString, sizeof(char *),
     offset(mg_arg[2]), XtRString, (XtPointer) NULL},
    {"magnifierSize4", "MagnifierSize", XtRString, sizeof(char *),
     offset(mg_arg[3]), XtRString, (XtPointer) NULL},
    {"magnifierSize5", "MagnifierSize", XtRString, sizeof(char *),
     offset(mg_arg[4]), XtRString, (XtPointer) NULL},
#ifdef	GREY
    {"grey", "Grey", XtRBoolean, sizeof(Boolean),
     offset(_use_grey), XtRString, "true"},
    {"install", "Install", XtRBool3, sizeof(Bool3),
     offset(install), XtRString, "maybe"},
#endif
    {"ruleColor", "RuleColor", XtRPixel, sizeof(Pixel),
     offset(rule_pixel), XtRPixel, (XtPointer)&resource.rule_pixel},
    {"ruleColor", "RuleColor", XtRString, sizeof(char *),
     offset(rule_color), XtRString, (XtPointer)NULL},
#ifdef HTEX
    {"underLink", "UnderLink", XtRBoolean, sizeof(Boolean),
     offset(_underline_link), XtRString, (XtPointer) "true"},
    {"wwwBrowser", "WWWBrowser", XtRString, sizeof(char *),
     offset(_browser), XtRString, (XtPointer) NULL},
    {"urlBase", "URLBase", XtRString, sizeof(char *),
     offset(_URLbase), XtRString, (XtPointer) NULL},
#endif /* HTEX */
    {"helpTopicsButtonLabel", "HelpTopicsButtonLabel", XtRString, sizeof(char *),
     offset(_help_topics_button_label), XtRString, "Topic"},
    {"helpQuitButtonLabel", "HelpQuitButtonLabel", XtRString, sizeof(char *),
     offset(_help_quit_button_label), XtRString, "Close"},
    {"helpIntro", "HelpIntro", XtRString, sizeof(char *),
     offset(_help_intro), XtRString, NULL},
    {"helpGeneralMenulabel", "HelpGeneralMenulabel", XtRString, sizeof(char *),
     offset(_help_general_menulabel), XtRString, "General"},
    {"helpGeneral", "HelpGeneral", XtRString, sizeof(char *),
     offset(_help_general), XtRString, NULL},
    {"helpHypertexMenulabel", "HelpHypertexMenulabel", XtRString, sizeof(char *),
     offset(_help_hypertex_menulabel), XtRString, "HyperTeX commands"},
    {"helpHypertex", "HelpHypertex", XtRString, sizeof(char *),
     offset(_help_hypertex), XtRString, NULL},
    {"helpOthercommandsMenulabel", "HelpOthercommandsMenulabel", XtRString, sizeof(char *),
     offset(_help_othercommands_menulabel), XtRString, "Other Commands"},
    {"helpOthercommands", "HelpOthercommands", XtRString, sizeof(char *),
     offset(_help_othercommands), XtRString, NULL},
    {"helpPagemotionMenulabel", "HelpPagemotionMenulabel", XtRString, sizeof(char *),
     offset(_help_pagemotion_menulabel), XtRString, "Page Motion"},
    {"helpPagemotion", "HelpPagemotion", XtRString, sizeof(char *),
     offset(_help_pagemotion), XtRString, NULL},
    {"helpMousebuttonsMenulabel", "HelpMousebuttonsMenulabel", XtRString, sizeof(char *),
     offset(_help_mousebuttons_menulabel), XtRString, "Mouse bindings"},
    {"helpMousebuttons", "HelpMousebuttons", XtRString, sizeof(char *),
     offset(_help_mousebuttons), XtRString, NULL},
    {"helpSourcespecialsMenulabel", "HelpSourcespecialsMenulabel", XtRString, sizeof(char *),
     offset(_help_sourcespecials_menulabel), XtRString, "Source Specials"},
    {"helpSourcespecials", "HelpSoucespecials", XtRString, sizeof(char *),
     offset(_help_sourcespecials), XtRString, NULL},
#ifdef	GREY
};

static XtResource app_pixel_resources[] = {	/* get these later */
#endif /* GREY */
    {"foreground", "Foreground", XtRPixel, sizeof(Pixel),
     offset(_fore_Pixel), XtRString, XtDefaultForeground},
    {"background", "Background", XtRPixel, sizeof(Pixel),
     offset(_back_Pixel), XtRString, XtDefaultBackground},
    {"borderColor", "BorderColor", XtRPixel, sizeof(Pixel),
     offset(_brdr_Pixel), XtRPixel, (XtPointer) & resource._fore_Pixel},
    {"highlight", "Highlight", XtRPixel, sizeof(Pixel),
     offset(_hl_Pixel), XtRPixel, (XtPointer) & resource._fore_Pixel},
    {"cursorColor", "CursorColor", XtRPixel, sizeof(Pixel),
     offset(_cr_Pixel), XtRPixel, (XtPointer) & resource._fore_Pixel},
};
#undef	offset

static _Xconst char *usagestr[] = {
    /* shrinkFactor */ "shrink",
#ifndef	VMS
    /* S */ "density",
    /* density */ silent,
#else
    /* density */ "density",
#endif
#ifdef	GREY
    /* gamma */ "g",
#endif
    /* rule */ "color",
    /* p */ "pixels",
    /* margins */ "dimen",
    /* sidemargin */ "dimen",
    /* topmargin */ "dimen",
    /* offsets */ "dimen",
    /* xoffset */ "dimen",
    /* yoffset */ "dimen",
    /* paper */ "papertype",
    /* altfont */ "font",
    /* mfmode */ "mode-def",
    /* editor */ "editor",
    /* sourceposition */ "linenumber[ ]*filename",
    /* rv */ "^-l", "-rv",
#ifdef	BUTTONS
    /* shrinkbutton1 */ subst,
    /* shrinkbutton2 */ silent,
    /* shrinkbutton3 */ silent,
    /* shrinkbutton4 */ silent,
    /* shrinkbutton5 */ silent,
    /* shrinkbutton6 */ silent,
    /* shrinkbutton7 */ silent,
    /* shrinkbutton8 */ silent,
    /* shrinkbutton9 */ silent,
#endif
    /* mgs */ subst,
    /* mgs1 */ silent,
    /* mgs2 */ silent,
    /* mgs3 */ silent,
    /* mgs4 */ silent,
    /* mgs5 */ silent,
    /* bw */ "^-safer", "-bw <width>",
    /* fg */ "color",
    /* foreground */ silent,
    /* bg */ "color",
    /* background */ silent,
    /* hl */ "color",
    /* bd */ "^-hl", "-bd <color>",
    /* cr */ "color",
#ifndef VMS
    /* display */ "^-cr", "-display <host:display>",
#else
    /* display */ "^-cr", "-display <host::display>",
#endif
    /* geometry */ "^-cr", "-geometry <geometry>",
    /* icongeometry */ "geometry",
    /* iconic */ "^-icongeometry", "-iconic",
#ifdef	BUTTONS
    /* font */ "^-icongeometry", "-font <font>",
#endif
    /* FIXME: SU 2000/03/08: the wheel stuff is untested  */
    /* wheelunit */ "pixels",
#ifdef	PS_GS
    /* interpreter */ "path",
    /* gspalette */ "monochrome|grayscale|color",
#endif
    /* debug */ "bitmask",
#ifdef HTEX
    /* browser */ "WWWbrowser",
    /* URLbase */ "base URL",
#endif
    /* [dummy] */ "z"
};

#ifndef MOTIF

#ifdef	NOQUERY
#define	drawWidgetClass	widgetClass
#else

/* ARGSUSED */
static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *constraints, XtWidgetGeometry *reply)
{
    UNUSED(w);
    UNUSED(constraints);
    reply->request_mode = CWWidth | CWHeight;
    reply->width = page_w;
    reply->height = page_h;
    return XtGeometryAlmost;
}

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#ifdef	lint
WidgetClassRec widgetClassRec;
#endif

	/* if the following gives you trouble, just compile with -DNOQUERY */
static WidgetClassRec drawingWidgetClass = {
    {
     /* superclass         */ &widgetClassRec,
     /* class_name         */ "Draw",
     /* widget_size        */ sizeof(WidgetRec),
     /* class_initialize   */ NULL,
     /* class_part_initialize */ NULL,
     /* class_inited       */ FALSE,
     /* initialize         */ NULL,
     /* initialize_hook    */ NULL,
     /* realize            */ XtInheritRealize,
     /* actions            */ NULL,
     /* num_actions        */ 0,
     /* resources          */ NULL,
     /* num_resources      */ 0,
     /* xrm_class          */ NULLQUARK,
     /* compress_motion    */ FALSE,
     /* compress_exposure  */ TRUE,
     /* compress_enterleave */ FALSE,
     /* visible_interest   */ FALSE,
     /* destroy            */ NULL,
     /* resize             */ XtInheritResize,
     /* expose             */ XtInheritExpose,
     /* set_values         */ NULL,
     /* set_values_hook    */ NULL,
     /* set_values_almost  */ XtInheritSetValuesAlmost,
     /* get_values_hook    */ NULL,
     /* accept_focus       */ XtInheritAcceptFocus,
     /* version            */ XtVersion,
     /* callback_offsets   */ NULL,
     /* tm_table           */ XtInheritTranslations,
     /* query_geometry       */ QueryGeometry,
     /* display_accelerator  */ XtInheritDisplayAccelerator,
     /* extension            */ NULL
     }
};

#define	drawWidgetClass	&drawingWidgetClass

#endif /* NOQUERY */
#endif /* MOTIF */

static Arg vport_args[] = {
#ifndef MOTIF
#ifdef	BUTTONS
    {XtNborderWidth, (XtArgVal) 0},
    {XtNtop, (XtArgVal) XtChainTop},
    {XtNbottom, (XtArgVal) XtChainBottom},
    {XtNleft, (XtArgVal) XtChainLeft},
    {XtNright, (XtArgVal) XtChainRight},
#endif
    {XtNallowHoriz, (XtArgVal) True},
    {XtNallowVert, (XtArgVal) True},
#else /* MOTIF */
    {XmNscrollingPolicy, (XtArgVal) XmAUTOMATIC},
    {XmNborderWidth, (XtArgVal) 0},
    {XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNbottomAttachment, (XtArgVal) XmATTACH_FORM},
    {XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
#endif /* MOTIF */
};

static Arg draw_args[] = {
#ifndef MOTIF
    {XtNwidth, (XtArgVal) 0},
    {XtNheight, (XtArgVal) 0},
    {XtNx, (XtArgVal) 0},
    {XtNy, (XtArgVal) 0},
    {XtNlabel, (XtArgVal) ""},
#else /* MOTIF */
    {XmNwidth, (XtArgVal) 0},
    {XmNheight, (XtArgVal) 0},
    {XmNbottomAttachment, (XtArgVal) XmATTACH_WIDGET},
#endif /* MOTIF */
};

#ifdef	BUTTONS
static Arg form_args[] = {
#ifndef MOTIF
    {XtNdefaultDistance, (XtArgVal) 0},
#else /* MOTIF */
    {XmNhorizontalSpacing, (XtArgVal) 0},
    {XmNverticalSpacing, (XtArgVal) 0},
#endif /* MOTIF */
};
#endif /* not BUTTONS */

#else /* TOOLKIT */

static char *display;
static char *geometry;
static char *margins;
static char *offsets;
static Boolean hush;
static Boolean iconic = False;

#define	ADDR(x)	(XPointer) &resource.x

static struct option {
    _Xconst char *name;
    _Xconst char *resource;
    enum { FalseArg, TrueArg, StickyArg, SepArg } argclass;
    enum { BooleanArg, Bool3Arg, StringArg, NumberArg, FloatArg } argtype;
    int classcount;
    _Xconst char *usagestr;
    XPointer address;
} options[] = {
    {"-regression", "regression", TrueArg, BooleanArg, 1, NULL, ADDR(regression)},
    {"+regression", "regression", FalseArg, BooleanArg, 2, NULL, ADDR(regression)},
    { "+", NULL, StickyArg, StringArg, 1, NULL, (XPointer) & curr_page},
    { "-s", "shrinkFactor", SepArg, NumberArg, 1, "shrink", (XPointer) & currwin.shrinkfactor},
#ifndef VMS
    { "-S", NULL, SepArg, NumberArg, 2, "density", ADDR(_density)},
    { "-density", "densityPercent", SepArg, NumberArg, 1, silent, ADDR(_density)},
#else
    { "-density", "densityPercent", SepArg, NumberArg, 1, "density", ADDR(_density)},
#endif
#ifdef	GREY
    { "-nogrey", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(_use_grey)},
    { "+nogrey", "grey", TrueArg, BooleanArg, 1, NULL, ADDR(_use_grey)},
    { "-gamma", "gamma", SepArg, FloatArg, 1, "g", ADDR(_gamma)},
    { "-install", NULL, TrueArg, Bool3Arg, 2, NULL, ADDR(install)},
    { "-noinstall", "install", FalseArg, Bool3Arg, 1, NULL, ADDR(install)},
#endif
    { "-rulecolor", "ruleColor", SepArg, StringArg, 1, "color", ADDR(rule_color)},
    { "-p", "pixelsPerInch", SepArg, NumberArg, 1, "pixels", ADDR(_pixels_per_inch)},
    { "-margins", "Margin", SepArg, StringArg, 3, "dimen", (XPointer) & margins},
    { "-sidemargin", "sideMargin", SepArg, StringArg, 1, "dimen", ADDR(sidemargin)},
    { "-topmargin", "topMargin", SepArg, StringArg, 1, "dimen", ADDR(topmargin)},
    { "-offsets", "Offset", SepArg, StringArg, 3, "dimen", (XPointer) & offsets},
    { "-xoffset", "xOffset", SepArg, StringArg, 1, "dimen", ADDR(xoffset)},
    { "-yoffset", "yOffset", SepArg, StringArg, 1, "dimen", ADDR(yoffset)},
    { "-paper", "paper", SepArg, StringArg, 1, "papertype", ADDR(paper)},
    { "-altfont", "altFont", SepArg, StringArg, 1, "font", ADDR(_alt_font)},
#ifdef MKTEXPK
    { "-nomakepk", "makePk", FalseArg, BooleanArg, 2, NULL, ADDR(makepk)},
    { "+nomakepk", "makePk", TrueArg, BooleanArg, 1, NULL, ADDR(makepk)},
#endif
    { "-mfmode", "mfMode", SepArg, StringArg, 1, "mode-def", ADDR(mfmode)},
    { "-editor", "editor", SepArg, StringArg, 1, "editor", ADDR(editor)},
#ifdef T1LIB
    { "+not1lib", "T1lib", TrueArg, BooleanArg, 1, NULL, ADDR(t1lib)},
    { "-not1lib", "T1lib", FalseArg, BooleanArg, 1, NULL, ADDR(t1lib)},
#endif    
    { "-sourceposition", "sourcePosition", SepArg, StringArg, 1, "linenumber[ ]*filename", ADDR(src_pos)},
    { "-l", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(_list_fonts)},
    { "+l", "listFonts", FalseArg, BooleanArg, 1, NULL, ADDR(_list_fonts)},
    { "-rv", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(reverse)},
    { "+rv", "reverseVideo", FalseArg, BooleanArg, 1, NULL, ADDR(reverse)},
    { "-mgs", NULL, SepArg, StringArg, 2, subst, ADDR(mg_arg[0])},
    { "-mgs1", "magnifierSize1", SepArg, StringArg, 1, silent, ADDR(mg_arg[0])},
    { "-mgs2", "magnifierSize2", SepArg, StringArg, 1, silent, ADDR(mg_arg[1])},
    { "-mgs3", "magnifierSize3", SepArg, StringArg, 1, silent, ADDR(mg_arg[2])},
    { "-mgs4", "magnifierSize4", SepArg, StringArg, 1, silent, ADDR(mg_arg[3])},
    { "-mgs5", "magnifierSize5", SepArg, StringArg, 1, silent, ADDR(mg_arg[4])},
    { "-warnspecials", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(_warn_spec)},
    { "+warnspecials", "warnSpecials", FalseArg, BooleanArg, 1, NULL, ADDR(_warn_spec)},
    { "-hush", NULL, TrueArg, BooleanArg, 6, NULL, (XPointer) & hush},
    { "+hush", "Hush", FalseArg, BooleanArg, 5, NULL, (XPointer) & hush},
    { "-hushchars", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(_hush_chars)},
    { "+hushchars", "hushLostChars", FalseArg, BooleanArg, 1, NULL, ADDR(_hush_chars)},
    { "-hushchecksums", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(_hush_chk)},
    { "+hushchecksums", "hushChecksums", FalseArg, BooleanArg, 1, NULL, ADDR(_hush_chk)},
    { "-hushstdout", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(_hush_stdout)},
    { "+hushstdout", "hushStdout", FalseArg, BooleanArg, 1, NULL, ADDR(_hush_stdout)},
    { "-safer", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(safer)},
    { "+safer", "safer", FalseArg, BooleanArg, 1, NULL, ADDR(safer)},
    { "-bw", NULL, SepArg, NumberArg, 2, "width", (XPointer) & bwidth},
    { "-borderwidth", "borderWidth", SepArg, NumberArg, 1, silent, (XPointer) & bwidth},
    { "-fg", NULL, SepArg, StringArg, 2, "color", ADDR(fore_color)},
    { "-foreground", "foreground", SepArg, StringArg, 1, silent, ADDR(fore_color)},
    { "-bg", NULL, SepArg, StringArg, 2, "color", ADDR(back_color)}, \
    { "-background", "background", SepArg, StringArg, 1, silent, ADDR(back_color)},
    { "-hl", "highlight", SepArg, StringArg, 1, "color", (XPointer) & high_color},
    { "-bd", NULL, SepArg, StringArg, 2, "color", (XPointer) & brdr_color},
    { "-bordercolor", "borderColor", SepArg, StringArg, 1, silent, (XPointer) & brdr_color},
    { "-cr", "cursorColor", SepArg, StringArg, 1, "color", (XPointer) & curs_color},
#ifndef VMS
    { "-display", NULL, SepArg, StringArg, 1, "host:display", (XPointer) & display},
#else
    { "-display", NULL, SepArg, StringArg, 1, "host::display", (XPointer) & display},
#endif
    { "-geometry", "geometry", SepArg, StringArg, 1, "geometry", (XPointer) & geometry},
    { "-icongeometry", "iconGeometry", StickyArg, StringArg, 1, "geometry", ADDR(icon_geometry)},
    { "-iconic", NULL, TrueArg, BooleanArg, 2, NULL, (XPointer) & iconic},
    { "+iconic", "iconic", FalseArg, BooleanArg, 1, NULL, (XPointer) & iconic},
    { "-keep", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(keep_flag)},
    { "+keep", "keepPosition", FalseArg, BooleanArg, 1, NULL, ADDR(keep_flag)},
    { "-copy", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(copy)},
    { "+copy", "copy", FalseArg, BooleanArg, 1, NULL, ADDR(copy)},
    { "-thorough", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(thorough)},
    { "+thorough", "thorough", FalseArg, BooleanArg, 1, NULL, ADDR(thorough)},
    { "-wheelunit", "wheelUnit", SepArg, NumberArg, 1, "pixels", ADDR(wheel_unit)},
#if	PS
    { "-nopostscript", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(_postscript)},
    { "+nopostscript", "postscript", TrueArg, BooleanArg, 1, NULL, ADDR(_postscript)},
    { "-noscan", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(prescan)},
    { "+noscan", "prescan", TrueArg, BooleanArg, 1, NULL, ADDR(prescan)},
    { "-allowshell", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(allow_shell)},
    { "+allowshell", "allowShell", FalseArg, BooleanArg, 1, NULL, ADDR(allow_shell)},
#ifdef	PS_DPS
    { "-nodps", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(useDPS)},
    { "+nodps", "dps", TrueArg, BooleanArg, 1, NULL, ADDR(useDPS)},
#endif
#ifdef	PS_NEWS
    { "-nonews", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(useNeWS)},
    { "+nonews", "news", TrueArg, BooleanArg, 1, NULL, ADDR(useNeWS)},
#endif
#ifdef	PS_GS
    { "-noghostscript", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(useGS)},
    { "+noghostscript", "ghostscript", TrueArg, BooleanArg, 1, NULL, ADDR(useGS)},
    { "-nogssafer", NULL, FalseArg, BooleanArg, 2, NULL, ADDR(gs_safer)},
    { "+nogssafer", "gsSafer", TrueArg, BooleanArg, 1, NULL, ADDR(gs_safer)},
    { "-nogsalpha", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(gs_alpha)},
    { "+nogsalpha", "gsAlpha", FalseArg, BooleanArg, 1, NULL, ADDR(gs_alpha)},
    { "-interpreter", "interpreter", SepArg, StringArg, 1, "path", ADDR(gs_path)},
    { "-gspalette", "palette", SepArg, StringArg, 1, "monochrome|grayscale|color", ADDR(gs_palette)},
#endif
#endif /* PS */
    { "-debug", "debugLevel", SepArg, StringArg, 1, "bitmask", ADDR(debug_arg)},
    { "-version", NULL, TrueArg, BooleanArg, 2, NULL, ADDR(version_flag)},
    { "+version", "version", FalseArg, BooleanArg, 1, NULL, ADDR(version_flag)},
#ifdef HTEX
    { "-underlink", "underLink", TrueArg, BooleanArg, 2, NULL, ADDR(_underline_link)},
    { "+underlink", "underLink", FalseArg, BooleanArg, 1, NULL, ADDR(_underline_link)},
    { "-browser", "wwwBrowser", SepArg, StringArg, 1, ADDR(_browser)},
    { "-base", "urlBase", SepArg, StringArg, 1, ADDR(_URLbase)},
#endif
};

#endif /* TOOLKIT */

static NORETURN void
usage(int exitval)
{
#ifdef	TOOLKIT
    XrmOptionDescRec *opt;
    _Xconst char **usageptr = usagestr;
#else
    struct option *opt;
#endif
    _Xconst char **sv = subst_val;
    _Xconst char *str1;
    _Xconst char *str2;
    int col = 23;
    int n;

#ifdef Omega
    Fputs("Usage: oxdvi [+[<page>]]", stdout);
#else
    Fputs("Usage: xdvi [+[<page>]]", stdout);
#endif
    for (opt = options; opt < options + XtNumber(options); ++opt) {
#ifdef	TOOLKIT
	str1 = opt->option;
	if (*str1 != '-')
	    continue;
	str2 = NULL;
	if (opt->argKind != XrmoptionNoArg) {
	    str2 = *usageptr++;
	    if (str2 == silent)
		continue;
	    if (str2 == subst) {
		str1 = *sv++;
		str2 = NULL;
	    }
	}
	for (;;) {
	    n = strlen(str1) + 3;
	    if (str2 != NULL)
		n += strlen(str2) + 3;
	    if (col + n < 80)
		Putc(' ', stdout);
	    else {
		Fputs("\n\t", stdout);
		col = 8 - 1;
	    }
	    if (str2 == NULL)
		Fprintf(stdout, "[%s]", str1);
	    else
		Fprintf(stdout, "[%s <%s>]", str1, str2);
	    col += n;
	    if (**usageptr != '^' || strcmp(*usageptr + 1, opt->option) != 0)
		break;
	    ++usageptr;
	    str1 = *usageptr++;
	    str2 = NULL;
	}
#else /* not TOOLKIT */
	str1 = opt->name;
	str2 = opt->usagestr;
	if (*str1 != '-' || str2 == silent)
	    continue;
	if (str2 == subst) {
	    str1 = *sv++;
	    str2 = NULL;
	}
	n = strlen(str1) + 3;
	if (str2 != NULL)
	    n += strlen(str2) + 3;
	if (col + n < 80)
	    Putc(' ', stdout);
	else {
	    Fputs("\n\t", stdout);
	    col = 8 - 1;
	}
	if (str2 == NULL)
	    Fprintf(stdout, "[%s]", str1);
	else
	    Fprintf(stdout, "[%s <%s>]", str1, str2);
	col += n;
#endif /* not TOOLKIT */
    }
    if (col + 9 < 80)
	Putc(' ', stdout);
    else
	Fputs("\n\t", stdout);
#ifdef SELFILE
    Fputs("[dvi_file]\n", stdout);
#else
    Fputs("dvi_file\n", stdout);
#endif
    exit(exitval);
}

static int
atopix(_Xconst char *arg, Boolean allow_minus)
{
    int len = strlen(arg);
    _Xconst char *arg_end = arg;
    char tmp[11];
    double factor;

    if (allow_minus && *arg_end == '-')
	++arg_end;
    while ((*arg_end >= '0' && *arg_end <= '9') || *arg_end == '.')
	if (arg_end >= arg + XtNumber(tmp) - 1)
	    return 0;
	else
	    ++arg_end;
    bcopy(arg, tmp, arg_end - arg);
    tmp[arg_end - arg] = '\0';

#if A4
    factor = 1.0 / 2.54;	/* cm */
#else
    factor = 1.0;	/* inches */
#endif
    if (len > 2)
	switch (arg[len - 2] << 8 | arg[len - 1]) {
#if A4
	case 'i' << 8 | 'n':
	    factor = 1.0;
	    break;
#else
	case 'c' << 8 | 'm':
	    factor = 1.0 / 2.54;
	    break;
#endif
	case 'm' << 8 | 'm':
	    factor = 1.0 / 25.4;
	    break;
	case 'p' << 8 | 't':
	    factor = 1.0 / 72.27;
	    break;
	case 'p' << 8 | 'c':
	    factor = 12.0 / 72.27;
	    break;
	case 'b' << 8 | 'p':
	    factor = 1.0 / 72.0;
	    break;
	case 'd' << 8 | 'd':
	    factor = 1238.0 / 1157.0 / 72.27;
	    break;
	case 'c' << 8 | 'c':
	    factor = 12 * 1238.0 / 1157.0 / 72.27;
	    break;
	case 's' << 8 | 'p':
	    factor = 1.0 / 72.27 / 65536;
	    break;
	}

    return factor * atof(tmp) * pixels_per_inch + 0.5;
}

#ifdef GRID
/* extract the unit used in paper size specification */
/* the information is used to decide the initial grid separation */
static int
atopixunit(_Xconst char *arg)
{
    int len = strlen(arg);

    return (len > 2 && arg[len - 2] == 'c' && arg[len - 1] == 'm' ?
	    1.0 / 2.54 : 1.0) * pixels_per_inch + 0.5;
}
#endif /* GRID */

/**
 **	Main programs start here.
 **/

#ifdef	TOOLKIT

#ifdef GREY
static Arg temp_args1[] = {
    {XtNdepth, (XtArgVal) 0},
    {XtNvisual, (XtArgVal) 0},
    {XtNcolormap, (XtArgVal) 0},
};
#define	temp_args1a	(temp_args1 + 2)
#endif

static Arg temp_args2[] = {
    {XtNiconX, (XtArgVal) 0},
    {XtNiconY, (XtArgVal) 0},
};

static Arg temp_args3 = { XtNborderWidth, (XtArgVal) & bwidth };

static Pixmap icon_pm;
static Arg temp_args4 = { XtNiconPixmap, (XtArgVal) & icon_pm };

static Arg temp_args5[] = {
    {XtNtitle, (XtArgVal) 0},
    {XtNiconName, (XtArgVal) 0},
    {XtNinput, (XtArgVal) True},
};

static Arg set_wh_args[] = {
    {XtNwidth, (XtArgVal) 0},
    {XtNheight, (XtArgVal) 0},
};


#ifdef GREY

/*
 *	Alternate routine to convert color name to Pixel (needed to substitute
 *	"black" or "white" for BlackPixelOfScreen, etc., since a different
 *	visual and colormap are in use).
 */

#if XtSpecificationRelease >= 5

/*ARGSUSED*/ static Boolean
XdviCvtStringToPixel(Display *dpy,
		     XrmValuePtr args,
		     Cardinal *num_args,
		     XrmValuePtr fromVal,
		     XrmValuePtr toVal,
		     XtPointer *closure_ret)
{
    XrmValue replacement_val;
    Boolean default_is_fg;

    if ((strcmp((String) fromVal->addr, XtDefaultForeground) == 0
	 && (default_is_fg = True))
	|| (strcmp((String) fromVal->addr, XtDefaultBackground) == 0
	    && ((default_is_fg = False), True))) {
	replacement_val.size = sizeof(String);
	replacement_val.addr = (default_is_fg == resource.reverse)
	    ? "white" : "black";
	fromVal = &replacement_val;
    }

    return XtCvtStringToPixel(dpy, args, num_args, fromVal, toVal, closure_ret);
}

#else /* XtSpecificationRelease < 5 */

/*
 *	Copied from the X11R4 source code.
 */

#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

static Boolean
XdviCvtStringToPixel(Display *dpy,
		     XrmValuePtr args,
		     Cardinal *num_args,
		     XrmValuePtr fromVal,
		     XrmValuePtr toVal,
		     XtPointer *closure_ret)
{
    String str = (String) fromVal->addr;
    XColor screenColor;
    XColor exactColor;
    Screen *screen;
    Colormap colormap;
    Status status;
    String params[1];
    Cardinal num_params = 1;

    if (*num_args != 2)
	XtErrorMsg("wrongParameters", "cvtStringToPixel",
		   "XtToolkitError",
		   "String to pixel conversion needs screen and colormap arguments",
		   (String *) NULL, (Cardinal *) NULL);

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    if (strcmp(str, XtDefaultBackground) == 0) {
	*closure_ret = False;
	str = (resource.reverse ? "black" : "white");
    }
    else if (strcmp(str, XtDefaultForeground) == 0) {
	*closure_ret = False;
	str = (resource.reverse ? "white" : "black");
    }

    if (*str == '#') {	/* some color rgb definition */

	status = XParseColor(DisplayOfScreen(screen), colormap,
			     (char *)str, &screenColor);

	if (status != 0)
	    status = XAllocColor(DisplayOfScreen(screen), colormap,
				 &screenColor);
    }
    else
	/* some color name */
	status = XAllocNamedColor(DisplayOfScreen(screen), colormap,
				  (char *)str, &screenColor, &exactColor);
    if (status == 0) {
	params[0] = str;
	XtWarningMsg("noColormap", "cvtStringToPixel",
		     "XtToolkitError",
		     "Cannot allocate colormap entry for \"%s\"",
		     params, &num_params);
	return False;
    }
    else {
	*closure_ret = (char *)True;
	done(Pixel, screenColor.pixel);
    }
}

#undef	done

#endif /* XtSpecificationRelease < 5 */

/*
 *	Convert string to yes/no/maybe.  Adapted from the X toolkit.
 */

/*ARGSUSED*/ static Boolean
XdviCvtStringToBool3(Display *dpy,
		     XrmValuePtr args,
		     Cardinal *num_args,
		     XrmValuePtr fromVal,
		     XrmValuePtr toVal,
		     XtPointer *closure_ret)
{
    String str = (String) fromVal->addr;
    static Bool3 value;
    UNUSED(closure_ret);
    UNUSED(num_args);
    UNUSED(args);

    if (memicmp(str, "true", 5) == 0
	|| memicmp(str, "yes", 4) == 0
	|| memicmp(str, "on", 3) == 0 || memicmp(str, "1", 2) == 0)
	value = True;

    else if (memicmp(str, "false", 6) == 0
	     || memicmp(str, "no", 3) == 0
	     || memicmp(str, "off", 4) == 0 || memicmp(str, "0", 2) == 0)
	value = False;

    else if (memicmp(str, "maybe", 6) == 0)
	value = Maybe;

    else {
	XtDisplayStringConversionWarning(dpy, str, XtRBoolean);
	return False;
    }

    if (toVal->addr != NULL) {
	if (toVal->size < sizeof(Bool3)) {
	    toVal->size = sizeof(Bool3);
	    return False;
	}
	*(Bool3 *) (toVal->addr) = value;
    }
    else
	toVal->addr = (XPointer) & value;

    toVal->size = sizeof(Bool3);
    return True;
}

#endif /* GREY */

#else /* not TOOLKIT */

struct _resource resource = {
    /* wheel_unit          */ 80,
    /* density              */ 40,
#ifdef	GREY
    /* gamma                */ 1.0,
#endif
    /* pixels_per_inch      */ BDPI,
    /* _delay_rulers        */ True,
    /* _tick_length         */ 4,
    /* _tick_units          */ "pt",
    /* sidemargin           */ NULL,
    /* topmargin            */ NULL,
    /* xoffset              */ NULL,
    /* yoffset              */ NULL,
    /* paper                */ DEFAULT_PAPER,
    /* alt_font             */ ALTFONT,
    /* makepk               */ MAKE_TEX_PK_BY_DEFAULT,
    /* mfmode               */ MFMODE,
    /* editor               */ NULL,
    /* t1lib                */ True,
    /* src_pos              */ NULL,
    /* list_fonts           */ False,
    /* reverse              */ False,
    /* warn_spec            */ False,
    /* hush_chars           */ False,
    /* hush_chk             */ False,
    /* hush_stdout          */ False,
    /* safer                */ False,
    /* fore_color           */ NULL,
    /* back_color           */ NULL,
    /* fore_Pixel           */ (Pixel) 0,
    /* back_Pixel           */ (Pixel) 0,
    /* icon_geometry        */ NULL,
    /* keep_flag            */ False,
    /* copy                 */ False,
    /* thorough             */ False,
#if	PS
    /* postscript           */ True,
    /* prescan              */ True,
    /* allow_shell          */ False,
#ifdef	PS_DPS
    /* useDPS               */ True,
#endif
#ifdef	PS_NEWS
    /* useNeWS              */ True,
#endif
#ifdef	PS_GS
    /* useGS                */ True,
    /* gs_safer             */ True,
    /* gs_alpha             */ False,
    /* gs_path              */ GS_PATH,
    /* gs_palette           */ "Color",
#endif
#endif /* PS */
    /* debug_arg            */ NULL,
    /* version_flag         */ False,
    /* mg_arg               */ {NULL, NULL, NULL, NULL, NULL},
#ifdef	GREY
    /* use_grey             */ True,
    /* install              */ Maybe,
#endif
#ifdef GRID
    /* grid_mode            */ 0,
#endif /* GRID */
    /* rule_color          */ NULL,
#ifdef HTEX
    /* _underline_link      */ True,
    /* _browser             */ (char *)NULL,
    /* _URLbase             */ (char *)NULL,
#endif
};

static Pixel
string_to_pixel(char **strp)	/* adapted from the toolkit */
{
    char *str = *strp;
    Status status;
    XColor color, junk;

    if (*str == '#') {	/* an rgb definition */
	status = XParseColor(DISP, our_colormap, str, &color);
	if (status != 0)
	    status = XAllocColor(DISP, our_colormap, &color);
    }
    else	/* a name */
	status = XAllocNamedColor(DISP, our_colormap, str, &color, &junk);
    if (status == 0) {
#if 0      
      //FIXME: too early?
#endif      
	do_popup_message(MSG_WARN, "Cannot allocate colormap entry for \"%s\"",
			 str);
	*strp = NULL;
	return (Pixel) 0;
    }
    return color.pixel;
}

/*
 *	Process the option table.  This is not guaranteed for all possible
 *	option tables, but at least it works for this one.
 */

static void
parse_options(int argc, char **argv)
{
    char **arg;
    char **argvend = argv + argc;
    char *optstring;
    XPointer addr;
    struct option *opt, *lastopt, *candidate;
    int len1, len2, matchlen;

    /*
     * Step 1.  Process command line options.
     */
    for (arg = argv + 1; arg < argvend; ++arg) {
	len1 = strlen(*arg);
	candidate = NULL;
	matchlen = 0;
	for (opt = options; opt < options + XtNumber(options); ++opt) {
	    len2 = strlen(opt->name);
	    if (opt->argclass == StickyArg) {
		if (matchlen <= len2 && !strncmp(*arg, opt->name, len2)) {
		    candidate = opt;
		    matchlen = len2;
		}
	    }
	    else if (len1 <= len2 && matchlen <= len1 &&
		     !strncmp(*arg, opt->name, len1)) {
		if (len1 == len2) {
		    candidate = opt;
		    break;
		}
		if (matchlen < len1)
		    candidate = opt;
		else if (candidate && candidate->argclass != StickyArg)
		    candidate = NULL;
		matchlen = len1;
	    }
	}
	if (candidate == NULL) {
	    if (**arg == '-' || dvi_name)
		usage(EXIT_FAILURE);
	    else {
		/* need to make sure that dvi_name can be freed safely */
		dvi_name = xmalloc((unsigned)strlen(*arg) + 1);
		Strcpy(dvi_name, *arg);
		continue;
	    }
	}
	/* flag it for subsequent processing */
	candidate->resource = (char *)candidate;
	/* store the value */
	addr = candidate->address;
	switch (candidate->argclass) {
	case FalseArg:
	    *((Boolean *) addr) = False;
	    continue;
	case TrueArg:
	    *((Boolean *) addr) = True;
	    continue;
	case StickyArg:
	    optstring = *arg + strlen(candidate->name);
	    break;
	case SepArg:
	    ++arg;
	    if (arg >= argvend)
		usage(EXIT_FAILURE);
	    optstring = *arg;
	    break;
	}
	switch (candidate->argtype) {
	case StringArg:
	    *((char **)addr) = optstring;
	    break;
	case NumberArg:
	    *((int *)addr) = atoi(optstring);
	    break;
	case FloatArg:
	    *((float *)addr) = atof(optstring);
	    break;
	default:;
	}
    }
    /*
     * Step 2.  Propagate classes for command line arguments.  Backwards.
     */
    for (opt = options + XtNumber(options) - 1; opt >= options; --opt)
	if (opt->resource == (char *)opt) {
	    addr = opt->address;
	    lastopt = opt + opt->classcount;
	    for (candidate = opt; candidate < lastopt; ++candidate) {
		if (candidate->resource != NULL) {
		    switch (opt->argtype) {
		    case BooleanArg:
		    case Bool3Arg:	/* same type as Boolean */
			*((Boolean *) candidate->address) = *((Boolean *) addr);
			break;
		    case StringArg:
			*((char **)candidate->address) = *((char **)addr);
			break;
		    case NumberArg:
			*((int *)candidate->address) = *((int *)addr);
			break;
		    case FloatArg:
			*((float *)candidate->address) = *((float *)addr);
			break;
		    }
		    candidate->resource = NULL;
		}
	    }
	}

    if ((DISP = XOpenDisplay(display)) == NULL)
	oops("Can't open display");
    SCRN = DefaultScreenOfDisplay(DISP);

    /*
     * Step 3.  Handle resources (including classes).
     */
    for (opt = options; opt < options + XtNumber(options); ++opt)
	if (opt->resource &&
	    ((optstring = XGetDefault(DISP, prog, opt->resource)) ||
	     (optstring = XGetDefault(DISP, "XDvi", opt->resource)))) {
	    lastopt = opt + opt->classcount;
	    for (candidate = opt; candidate < lastopt; ++candidate)
		if (candidate->resource != NULL)
		    switch (opt->argtype) {
		    case Bool3Arg:
			if (memicmp(optstring, "maybe", 6) == 0) {
			    *(Bool3 *) candidate->address = Maybe;
			    break;
			}
			/* otherwise, fall through; the underlying */
			/* types of Bool3 and Boolean are the same. */
		    case BooleanArg:
			*(Boolean *) candidate->address =
			    (memicmp(optstring, "true", 5) == 0
			     || memicmp(optstring, "yes", 4) == 0
			     || memicmp(optstring, "on", 3) == 0
			     || memicmp(optstring, "1", 2) == 0);
			break;
		    case StringArg:
			*(char **)candidate->address = optstring;
			break;
		    case NumberArg:
			*(int *)candidate->address = atoi(optstring);
			break;
		    case FloatArg:
			*(float *)candidate->address = atof(optstring);
		    }
	}
}

#endif /* not TOOLKIT */

/*
 *	Routines for running as source-special client.
 *
 *	Resources are used as follows:
 *
 *	ATOM_XDVI_WINDOWS is attached to the root window of the default screen
 *		of the display; it contains a list of (hopefully active) xdvi
 *		windows.
 *	ATOM_DVI_FILE is attached to the main xdvi window; it tells the world
 *		what dvi file is being viewed.  It is set by that copy of xdvi
 *		and read by this routine.  The first 8 bytes are the inode
 *		number, and the rest is the file name.  We use 8 instead of
 *		sizeof(ino_t) because the latter may vary from machine to
 *		machine, and the format needs to be machine independent.
 *	ATOM_SRC_GOTO is attached to the main xdvi window; it tells that copy
 *		of xdvi to go to that position in the dvi file.  It is set by
 *		this routine and read by the displaying copy of xdvi.
 */

static int XdviErrorHandler(Display *, XErrorEvent *);

static unsigned long xdvi_next_request = 0;
static int xerrno;
static int (*XdviOldErrorHandler) ARGS((Display *, XErrorEvent *));

static int
XdviErrorHandler(Display *d, XErrorEvent *event)
{
    if (event->serial != xdvi_next_request || event->error_code != BadWindow)
	return XdviOldErrorHandler(d, event);

    xerrno = 1;
    return 0;
}

static int
XdviGetWindowProperty(display, w, property, long_offset, long_length, delete,
		      req_type, actual_type_return, actual_format_return,
		      nitems_return, bytes_after_return, prop_return)
    Display *display;
    Window w;
    Atom property;
    long long_offset, long_length;
    Bool delete;
    Atom req_type;
    Atom *actual_type_return;
    int *actual_format_return;
    unsigned long *nitems_return;
    unsigned long *bytes_after_return;
    unsigned char **prop_return;
{
    int retval;

    xdvi_next_request = NextRequest(display);
    xerrno = 0;

    retval = XGetWindowProperty(display, w, property, long_offset,
				long_length, delete, req_type,
				actual_type_return, actual_format_return,
				nitems_return, bytes_after_return, prop_return);

    return (xerrno != 0 ? BadWindow : retval);
}

/*
 *	src_client_check() - Check for another running copy of xdvi viewing
 *	the same file.  If one exists, return true and send that copy of xdvi
 *	the argument to -sourceposition.  If not, return false.
 */

static Boolean
src_client_check(void)
{
    unsigned char *window_list;
    size_t window_list_len;
    unsigned char *window_list_end;
    unsigned char *wp;
    unsigned char *p;
    Boolean need_rewrite;
    Boolean retval = False;

    /*
     * Get window list.  Copy it over
     * (we'll be calling property_get_data() again).
     */

    window_list_len = property_get_data(DefaultRootWindow(DISP),
					ATOM_XDVI_WINDOWS, &p,
					XGetWindowProperty);

    if (window_list_len == 0) {
	TRACE_CLIENT((stderr, "No \"xdvi windows\" property found"));
	return False;
    }

    if (window_list_len % 4 != 0) {
	TRACE_CLIENT((stderr, "\"xdvi windows\" property had incorrect size; deleting it."));
	XDeleteProperty(DISP, DefaultRootWindow(DISP), ATOM_XDVI_WINDOWS);
	return False;
    }

    window_list = xmalloc(window_list_len);
    memcpy(window_list, p, window_list_len);

    XdviOldErrorHandler = XSetErrorHandler(XdviErrorHandler);

    /* Loop over list of windows.  */

    need_rewrite = False;
    window_list_end = window_list + window_list_len;
    if (debug & DBG_CLIENT) {
	unsigned long ino;
	int i;
	
	ino = 0;
	for (i = 7; i >= 0; --i)
	    ino = (ino << 8) | dvi_property[i];
	TRACE_CLIENT((stderr, "My property: %lu `%s'", ino, dvi_property+8));
    }
    for (wp = window_list; wp < window_list_end; wp += 4) {
	Window w;
	unsigned char *buf_ret;
	size_t len;

#ifndef WORD64
	w = *((xuint32 *) wp);
#else
#if WORDS_BIGENDIAN
	w = ((unsigned long)wp[0] << 24) | ((unsigned long)wp[1] << 16)
	    | ((unsigned long)wp[2] << 8) | (unsigned long)wp[3];
#else
	w = ((unsigned long)wp[3] << 24) | ((unsigned long)wp[2] << 16)
	    | ((unsigned long)wp[1] << 8) | (unsigned long)wp[0];
#endif
#endif

	len = property_get_data(w, ATOM_DVI_FILE, &buf_ret,
				XdviGetWindowProperty);

	if (len == 0) {
	    /* not getting back info for a window usually indicates
	       that the application the window had belonged to had
	       been killed with signal 9
	    */
	    TRACE_CLIENT((stderr, "Window 0x%x: doesn't exist any more, deleting", (unsigned int)w));
	    window_list_len -= 4;
	    window_list_end -= 4;
	    bcopy(wp + 4, wp, window_list_end - wp);
	    wp -= 4;
	    need_rewrite = True;
	    continue;
	}

	if (debug & DBG_CLIENT) {
	    unsigned long ino;
	    int i;

	    ino = 0;
	    for (i = 7; i >= 0; --i)
		ino = (ino << 8) | buf_ret[i];
	    TRACE_CLIENT((stderr, "Window 0x%x: property: %lu `%s'", (unsigned int)w, ino, buf_ret + 8));
	}
	if (len == dvi_property_length
	    && memcmp(buf_ret, dvi_property, len) == 0) {
	    XChangeProperty(DISP, w,
			    ATOM_SRC_GOTO, ATOM_SRC_GOTO, 8, PropModeReplace,
			    (unsigned char *)resource.src_pos, strlen(resource.src_pos));
	    retval = True;
	    break;
	}
    }

    (void)XSetErrorHandler(XdviOldErrorHandler);

    if (need_rewrite)
	XChangeProperty(DISP, DefaultRootWindow(DISP),
			ATOM_XDVI_WINDOWS, ATOM_XDVI_WINDOWS, 32,
			PropModeReplace, (unsigned char *)window_list,
			window_list_len / 4);
    
    return retval;
}

/* Translation of valid paper types to dimensions,
   which are used internally. The newline characters are a hack
   to format the list neatly for error messages.
*/
static _Xconst char *paper_types[] = {
    "us", "8.5x11in",
    "usr", "11x8.5in",
    "legal", "8.5x14in",
    "legalr", "14x8.5in",
    "foolscap", "13.5x17.0in",	/* ??? */
    "foolscapr", "17.0x13.5in",
    "\n", "0",

    /* ISO `A' formats, Portrait */
    "a1", "59.4x84.0cm",
    "a2", "42.0x59.4cm",
    "a3", "29.7x42.0cm",
    "a4", "21.0x29.7cm",
    "a5", "14.85x21.0cm",
    "a6", "10.5x14.85cm",
    "a7", "7.42x10.5cm",
    "\n", "0",

    /* ISO `A' formats, Landscape */
    "a1r", "84.0x59.4cm",
    "a2r", "59.4x42.0cm",
    "a3r", "42.0x29.7cm",
    "a4r", "29.7x21.0cm",
    "a5r", "21.0x14.85cm",
    "a6r", "14.85x10.5cm",
    "a7r", "10.5x7.42cm",
    "\n", "0",

    /* ISO `B' formats, Portrait */
    "b1", "70.6x100.0cm",
    "b2", "50.0x70.6cm",
    "b3", "35.3x50.0cm",
    "b4", "25.0x35.3cm",
    "b5", "17.6x25.0cm",
    "b6", "13.5x17.6cm",
    "b7", "8.8x13.5cm",
    "\n", "0",

    /* ISO `B' formats, Landscape */
    "b1r", "100.0x70.6cm",
    "b2r", "70.6x50.0cm",
    "b3r", "50.0x35.3cm",
    "b4r", "35.3x25.0cm",
    "b5r", "25.0x17.6cm",
    "b6r", "17.6x13.5cm",
    "b7r", "13.5x8.8cm",
    "\n", "0",

    /* ISO `C' formats, Portrait */
    "c1", "64.8x91.6cm",
    "c2", "45.8x64.8cm",
    "c3", "32.4x45.8cm",
    "c4", "22.9x32.4cm",
    "c5", "16.2x22.9cm",
    "c6", "11.46x16.2cm",
    "c7", "8.1x11.46cm",
    "\n", "0",

    /* ISO `C' formats, Landscape */
    "c1r", "91.6x64.8cm",
    "c2r", "64.8x45.8cm",
    "c3r", "45.8x32.4cm",
    "c4r", "32.4x22.9cm",
    "c5r", "22.9x16.2cm",
    "c6r", "16.2x11.46cm",
    "c7r", "11.46x8.1cm",
};

static Boolean
set_paper_type(void)
{
    _Xconst char *arg, *arg1;
    char temp[21];
    _Xconst char **p;
    char *q;

    if (strlen(resource.paper) > sizeof(temp) - 1)
	return False;
    arg = resource.paper;
    q = temp;
    for (;;) {	/* convert to lower case */
	char c = *arg++;
	if (c >= 'A' && c <= 'Z')
	    c ^= ('a' ^ 'A');
	*q++ = c;
	if (c == '\0')
	    break;
    }
    arg = temp;
    /* perform substitutions */
    for (p = paper_types; p < paper_types + XtNumber(paper_types); p += 2)
	if (strcmp(temp, *p) == 0) {
	    arg = p[1];
	    break;
	}
    arg1 = strchr(arg, 'x');
    if (arg1 == NULL)
	return False;
    unshrunk_paper_w = atopix(arg, False);
    unshrunk_paper_h = atopix(arg1 + 1, False);
#ifdef GRID
    unshrunk_paper_unit = atopixunit(arg);
#endif /* GRID */
    return (unshrunk_paper_w != 0 && unshrunk_paper_h != 0);
}

/* Set the icon name and title name standard properties on `top_level'
   (which we don't pass in because it is a different type for TOOLKIT
   !and TOOLKIT).  We use the basename of the DVI file (without the
   .dvi), so different xdvi invocations can be distinguished, yet 
   do not use up too much real estate.  */

void
set_icon_and_title(char *dvi_name, char **icon_ret, char **title_ret, int set_std)
{
    /* Use basename of DVI file for name in icon and title.  */
    unsigned baselen;
    char *icon_name, *title_name;
    /* SU 2000/12/16: added page number information */
#ifdef Omega
    _Xconst char *_Xconst title_name_fmt = "(Omega) OXdvi:  %s   (%d page%s)";
#else
    _Xconst char *_Xconst title_name_fmt = "Xdvi:  %s   (%d page%s)";
#endif

    icon_name = strrchr(dvi_name, DIR_SEPARATOR);
    if (icon_name != NULL)
	++icon_name;
    else {
	icon_name = dvi_name;
	/*
	 * remove the `file:' prefix from the icon name; since some
	 * windowmanagers only display a prefix in window lists etc.,
	 * it's more significant this way.
	 */
	if (strncmp(icon_name, "file:", 5) == 0) {
	    icon_name += 5;
	}
    }
    baselen = strlen(icon_name);
    if (baselen >= sizeof(".dvi")
	&& strcmp(icon_name + baselen - sizeof(".dvi") + 1, ".dvi")
	== 0) {	/* remove the .dvi */
	char *p;

	baselen -= sizeof(".dvi") - 1;
	p = xmalloc(baselen + 1);
	(void)strncpy(p, icon_name, (int)baselen);
	p[baselen] = '\0';
	icon_name = p;
    }
    title_name = xmalloc(strlen(title_name_fmt)
			 + baselen + length_of_int(total_pages)
			 + 2);	/* 2 for additional plural `s' */
    Sprintf(title_name, title_name_fmt, icon_name, total_pages,
	    (total_pages > 1) ? "s" : "");

    if (icon_ret)
	*icon_ret = icon_name;
    if (title_ret)
	*title_ret = title_name;
    if (set_std) {
	Window top_window =
#ifdef TOOLKIT
	    XtWindow(top_level);
#else
	    top_level;
#endif
	XSetStandardProperties(DISP, top_window, title_name, icon_name,
			       (Pixmap) 0, NULL, 0, NULL);
    }
}

static void
display_version_info(void)
{
	Printf("xdvik version %s\n", TVERSION);
	Printf("Copyright (C) 1990-2002 Paul Vojta and others.\n\
Primary author of Xdvi: Paul Vojta;\n\
Xdvik maintainers: Nicolai Langfeldt, Stefan Ulrich\n\
Please send bug reports, feature requests etc. to one of:\n\
   http://sourceforge.net/tracker/?group_id=23164&atid=377580\n\
   tex-k@tug.org (http://tug.org/mailman/listinfo/tex-k)\n\
\n\
xdvi itself is licensed under the X Consortium license. Xdvik relies on\n\
- The kpathsea library which is covered by the GNU LIBRARY General\n\
  Public License (see COPYING.LIB for full details)\n\
- libwww which is copyrighted by the World Wide Web Consortium and CERN\n\
  (see COPYRIGH for full details).\n\
- t1lib which is (C) Rainer Menzner and licensed under the GNU LIBRARY\n\
  General Public License but are in part (C) Adobe Systems Inc., IBM\n\
  and the X11-consortium.\n\
There is NO WARRANTY of anything.\n\n"); 
}



/*
 *	main program
 */

int
main(int argc, char **argv)
{

#ifndef	TOOLKIT
    XSizeHints size_hints;
    XWMHints wmhints;
    int flag;
    int x_thick = 0;
    int y_thick = 0;
#endif /* TOOLKIT */
#ifdef MOTIF
    Widget menubar;
    Widget scale_menu;
#endif
    Dimension screen_w, screen_h;
    char *title_name;
    char *icon_name;
    int i;

    /*
     *      Step 1:  Process command-line options and resources.
     */

    /* This has to be a special case, for now. */
    
    /* This has to be a special case, for now. We need to loop through all args
     * in case xdvi is aliased, or a shell script (like in teTeX) that adds
     * things like `-name' at the beginning of the arglist.
     */
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-help") == 0
	    || strcmp(argv[i], "-h") == 0
	    || strcmp(argv[i], "+help") == 0
	    || strcmp(argv[i], "--help") == 0) {
	    /*	extern char *kpse_bug_address; */
	    printf("xdvik version %s\n", TVERSION);
	    printf("   A DVI file previewer for the X window system.\n"
		   "   Please submit bug reports to:\n"
		   "   http://sourceforge.net/tracker/?group_id=23164&atid=377580\n\n");
	    usage(0);
	}
	else if (strcmp(argv[i], "--version") == 0
		 || strcmp(argv[i], "-v") == 0) {
	    display_version_info();
	    exit(0);
	}
    }

#ifdef	VMS
    prog = strrchr(*argv, ']');
#else
    prog = strrchr(*argv, DIR_SEPARATOR);
#endif
    if (prog != NULL)
	++prog;
    else
	prog = *argv;

#ifdef	VMS
    if (strchr(prog, '.') != NULL)
	*strchr(prog, '.') = '\0';
#endif

    /* We can't initialize this at compile time, because it might be
       the result of a function call under POSIX. */
    n_files_left = OPEN_MAX;

    kpse_set_program_name(argv[0], "xdvi");
    kpse_set_program_enabled(kpse_any_glyph_format,
			     MAKE_TEX_PK_BY_DEFAULT, kpse_src_compile);

    /* get the debug value (if any) from the environment */
    /* (it's too early to get it from the command line) */
    {
	_Xconst char *dbg_str;

	dbg_str = getenv("XDVIDEBUG");
	if (dbg_str != NULL)
	    debug = atoi(dbg_str);
    }
#if	CFGFILE
#ifdef	SELFAUTO
    argv0 = argv[0];
#endif
#ifndef	CFG2RES
    readconfig();	/* read config file(s). */
#else /* CFG2RES */
    {
	static _Xconst struct cfg2res cfg2reslist[] = {
	    {"MFMODE", "mfMode", False},
	    {"PIXELSPERINCH", "pixelsPerInch", True},
	    {"SHRINKFACTOR", "shrinkFactor", True},
#ifdef	BUTTONS
	    {"SHRINKBUTTON1", "shrinkButton1", True},
	    {"SHRINKBUTTON2", "shrinkButton2", True},
	    {"SHRINKBUTTON3", "shrinkButton3", True},
	    {"SHRINKBUTTON4", "shrinkButton4", True},
	    {"SHRINKBUTTON5", "shrinkButton5", True},
	    {"SHRINKBUTTON6", "shrinkButton6", True},
	    {"SHRINKBUTTON7", "shrinkButton7", True},
	    {"SHRINKBUTTON8", "shrinkButton8", True},
	    {"SHRINKBUTTON9", "shrinkButton9", True},
#endif
	    {"PAPER", "paper", False},
	};

	readconfig(cfg2reslist, cfg2reslist + XtNumber(cfg2reslist),
		   application_resources,
		   application_resources + XtNumber(application_resources));
    }
#endif /* CFG2RES */
#endif /* CFGFILE */

#ifdef	TOOLKIT

    top_level = XtInitialize(prog, "XDvi", options, XtNumber(options),
			     &argc, argv);
    XtAddActions(Actions, num_actions);

    while (--argc > 0) {
	if (*(*++argv) == '+')
	    if (curr_page != NULL)
		usage(EXIT_FAILURE);
	    else
		curr_page = *argv + 1;
	else if (dvi_name != NULL)
	    usage(EXIT_FAILURE);
	else {
	    /* need to make sure that dvi_name can be freed safely */
	    dvi_name = xstrdup(*argv);
	    /* save a normalized representation of dvi_name to global_dvi_name,
	     * used for forward search: Without `file:' prefix, with extension
	     * and full path name.
	     */
	    global_dvi_name = normalize_and_expand_filename(dvi_name);
	}
    }

    DISP = XtDisplay(top_level);
    SCRN = XtScreen(top_level);

#ifdef GREY
    XtSetTypeConverter(XtRString, XtRBool3, XdviCvtStringToBool3,
		       NULL, 0, XtCacheNone, NULL);
#endif /* GREY */

#if !MOTIF
    accelerators = XtParseAcceleratorTable("<Key>Return:set()notify()unset()\n<Key>q:set()notify()unset()");
#endif /* !MOTIF */
    XtGetApplicationResources(top_level, (XtPointer) & resource,
			      application_resources,
			      XtNumber(application_resources), (ArgList) NULL,
			      0);
    currwin.shrinkfactor = resource.shrinkfactor;

#if CFGFILE
    if (resource.progname != NULL)
	prog = resource.progname;
#endif /* CFGFILE */

#else /* TOOLKIT */

    parse_options(argc, argv);

#endif /* TOOLKIT */

    if (resource.version_flag) {
	display_version_info();
	exit(0);
    }

    /* initialize the `debug' resource as early as possible.
       Note: earlier calls to TRACE_* or if (debug) will only
       work if the XDVIDEBUG environment variable is set!
    */
    if (resource.debug_arg != NULL) {
	debug = isdigit(*resource.debug_arg)
	    ? atoi(resource.debug_arg)
	    : DBG_ALL;
	if (debug & DBG_OPEN)
	    KPSE_DEBUG_SET(KPSE_DEBUG_FOPEN);
	if (debug & DBG_STAT)
	    KPSE_DEBUG_SET(KPSE_DEBUG_STAT);
	if (debug & DBG_HASH)
	    KPSE_DEBUG_SET(KPSE_DEBUG_HASH);
	if (debug & DBG_PATHS)
	    KPSE_DEBUG_SET(KPSE_DEBUG_PATHS);
	if (debug & DBG_EXPAND)
	    KPSE_DEBUG_SET(KPSE_DEBUG_EXPAND);
	if (debug & DBG_SEARCH)
	    KPSE_DEBUG_SET(KPSE_DEBUG_SEARCH);
    }
    
    if (resource.regression) {
	/* currently it just turns on everything; but we want only
	   output that's usable for automatic diffs (e.g. independent
	   of window manager state)	*/
	debug = DBG_ALL;
    }

    /* Check early for whether to pass off to a different xdvi process
     * (-sourceposition argument for reverse source special lookup).  */

    if (
#if XlibSpecificationRelease >= 6
	!XInternAtoms(DISP, atom_names, XtNumber(atom_names), False, atoms)
#else
	(atoms[0] = XInternAtom(DISP, atom_names[0], False)) == None
	|| (atoms[1] = XInternAtom(DISP, atom_names[1], False)) == None
	|| (atoms[2] = XInternAtom(DISP, atom_names[2], False)) == None
#endif
	) {
	oops("XtInternAtoms failed.");
    }

    if (debug & DBG_CLIENT) {
	unsigned k;
	for (k = 0; k < XtNumber(atom_names); ++k)
	    TRACE_CLIENT((stderr, "Atom(%s) = %lu", atom_names[k], atoms[k]));
    }

    if (dvi_name == NULL) {
#ifdef SELFILE
	(void) select_filename(False, False);
	/* User might have cancled  -janl */
	if (dvi_name == NULL)
	    exit(0);
#else
	usage(EXIT_FAILURE);
#endif      
    }

#ifdef HTEX
    htex_open_dvi_file();
#else
    open_dvi_file();
#endif /* not HTEX */

    if (dvi_file == NULL) {
	perror(dvi_name);
	return 1;
    }

    form_dvi_property();

    if (resource.src_pos != NULL) {
	if (src_client_check()) {
	    TRACE_CLIENT((stderr, "Match; changing property of client and exiting ..."));
	    XFlush(DISP);	/* necessary to get the property set */
	    return 0;
	}
	else {
	    TRACE_CLIENT((stderr, "no matching propery found, forking ..."));
	    /*
	     * No suitable xdvi found, so we start one by
	     * self-backgrounding.
	     */
	    Fflush(stdout);	/* these avoid double buffering */
	    Fflush(stderr);
	    XFlush(DISP);
	    if (fork())	/* if initial process (do NOT use vfork()!) */
		return 0;
	}
    }

    /* Needed for source specials and for calling ghostscript */
    xputenv("DISPLAY", XDisplayString(DISP));

    if (resource.mfmode != NULL) {
	char *p;

	p = strrchr(resource.mfmode, ':');
	if (p != NULL) {
	    unsigned int len;
	    char *p1;

	    ++p;
	    len = p - resource.mfmode;
	    p1 = xmalloc(len);
	    bcopy(resource.mfmode, p1, len - 1);
	    p1[len - 1] = '\0';
	    resource.mfmode = p1;
	    pixels_per_inch = atoi(p);
	}
    }
    if (currwin.shrinkfactor < 0 || density <= 0 || pixels_per_inch <= 0
#ifndef SELFILE
	|| dvi_name == NULL
#endif
	)
	usage(EXIT_FAILURE);
    if (currwin.shrinkfactor > 1) {
	bak_shrink = currwin.shrinkfactor;
	mane.shrinkfactor = currwin.shrinkfactor;	/* otherwise it's 1 */
    }

    if (resource.sidemargin)
	home_x = atopix(resource.sidemargin, False);
    if (resource.topmargin)
	home_y = atopix(resource.topmargin, False);
    offset_x = resource.xoffset ? atopix(resource.xoffset, True)
	: pixels_per_inch;
    offset_y = resource.yoffset ? atopix(resource.yoffset, True)
	: pixels_per_inch;
    if (!set_paper_type()) {
	_Xconst char **p;
	fprintf(stderr, "%s: Unrecognized paper type \"%s\". Legal values are:\n ", prog, resource.paper);
	for (p = paper_types; p < paper_types + XtNumber(paper_types); p += 2)
	    fprintf(stderr, "%s ", *p);
	fprintf(stderr,
		"\n(the names ending with `r' are `rotated' or `landscape' variants),\n"
		"or a specification `WIDTHxHEIGHT' followed by a dimension unit\n"
		"(pt, pc, in, bp, cm, mm, dd, cc, or sp).\n\n");
	xdvi_exit(1);
    }
    for (i = 0; i < 5; ++i)
	if (resource.mg_arg[i] != NULL) {
	    char *s;

	    mg_size[i].w = mg_size[i].h = atoi(resource.mg_arg[i]);
	    s = strchr(resource.mg_arg[i], 'x');
	    if (s != NULL) {
		mg_size[i].h = atoi(s + 1);
		if (mg_size[i].h <= 0)
		    mg_size[i].w = 0;
	    }
	}
#if	PS
    if (resource.safer) {
	resource.allow_shell = False;
#ifdef	PS_GS
	resource.gs_safer = True;
#endif
    }
#endif /* PS */
#ifdef	PS_GS
    {
	_Xconst char *CGMcgm = "CGMcgm";
	_Xconst char *cgmp;

	cgmp = strchr(CGMcgm, resource.gs_palette[0]);
	if (cgmp == NULL)
	    oops("Invalid value %s for gs palette option", resource.gs_palette);
	if (cgmp >= CGMcgm + 3) {
	    static char gsp[] = "x";

	    gsp[0] = *(cgmp - 3);
	    resource.gs_palette = gsp;
	}
    }
#endif

    kpse_init_prog("XDVI", pixels_per_inch, resource.mfmode, alt_font);

    if (debug) {
	fprintf(stderr, "Xdvi(k) %s, ", TVERSION);
	fprintf(stderr,
		"configured with: ppi=%d shrink=%d mfmode=%s alt_font=%s paper=%s\n",
		pixels_per_inch,
		currwin.shrinkfactor,
		resource.mfmode ? resource.mfmode : "<NULL>",
		alt_font,
		resource.paper);
    }

    kpse_set_program_enabled(kpse_any_glyph_format,
			     resource.makepk, kpse_src_compile);

    /* janl 16/11/98: I have changed this. The above line used to
       say the settings in resource.makepk was supplied on the
       commandline, resulting in it overriding _all other_
       settings, derived from the environment or texmf.cnf, no
       matter what the value. The value in resource.makepk could
       be the compile-time default...

       Personaly I like the environment/texmf.cnf to override
       resources and thus changed the 'level' of this setting to
       kpse_src_compile so the environment/texmf.cnf will override
       the values derived by Xt.

       Previous comment here:

       ``Let true values as an X resource/command line override false
       values in texmf.cnf/envvar.''  */

    /*
     *      Step 2:  Settle colormap issues.  This should be done before
     *      other widgets are created, so that they get the right
     *      pixel values.  (The top-level widget won't have the right
     *      values, but I don't think that makes any difference.)
     */

#ifdef XSERVER_INFO
    print_xserver_info();
#endif

#ifdef GREY

    our_depth = DefaultDepthOfScreen(SCRN);
    our_visual = DefaultVisualOfScreen(SCRN);
    our_colormap = DefaultColormapOfScreen(SCRN);
#ifdef XSERVER_INFO
    if (debug & DBG_PK)
	fprintf(stdout, "--- our_depth: %d\n", our_depth);
#endif

    if (resource.install != False && our_visual->class == PseudoColor) {
	/* look for a TrueColor visual with more bits */
	XVisualInfo template;
	XVisualInfo *list;
	int nitems_return;

#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    fprintf(stdout, "--- looking for a better TrueColor visual\n");
#endif
	template.screen = XScreenNumberOfScreen(SCRN);
	template.class = TrueColor;
	list = XGetVisualInfo(DISP, VisualScreenMask | VisualClassMask,
			      &template, &nitems_return);
	if (list != NULL) {
	    XVisualInfo *list1;
	    XVisualInfo *best = NULL;

	    for (list1 = list; list1 < list + nitems_return; ++list1) {
#ifdef XSERVER_INFO
		if (debug & DBG_PK)
		    fprintf(stdout, "--- checking %d\n", list1->depth);
#endif
		if ((unsigned int)list1->depth > our_depth
# if PS_GS
		    /* patch by Toni Ronkko <tronkko@hytti.uku.fi>, fixes bug #458057:
		     * Not all depths are supported by ghostscript; see
		     * xdev->vinfo->depth in gdevxcmp.c (ghostscript-6.51).
		     * SGI supports additional depths of 12 and 30. */
		    && (list1->depth == 1 || list1->depth == 2
			|| list1->depth == 4 || list1->depth == 8
			|| list1->depth == 15 || list1->depth == 16
			|| list1->depth == 24 || list1->depth == 32)
# endif
		    && (best == NULL || list1->depth > best->depth))
		    best = list1;
	    }
	    if (best != NULL) {
#ifdef XSERVER_INFO
		if (debug & DBG_PK)
		    fprintf(stdout, "--- best depth: %d\n", best->depth);
#endif
		our_depth = best->depth;
		our_visual = best->visual;
		our_colormap = XCreateColormap(DISP,
					       RootWindowOfScreen(SCRN),
					       our_visual, AllocNone);
		XInstallColormap(DISP, our_colormap);
#ifdef TOOLKIT
		temp_args1[0].value = (XtArgVal) our_depth;
		temp_args1[1].value = (XtArgVal) our_visual;
		temp_args1[2].value = (XtArgVal) our_colormap;
		XtSetValues(top_level, temp_args1, XtNumber(temp_args1));
		XtSetTypeConverter(XtRString, XtRPixel,
				   XdviCvtStringToPixel,
				   (XtConvertArgList) colorConvertArgs, 2,
				   XtCacheByDisplay, NULL);
#else
		/* Can't use {Black,White}PixelOfScreen() any more */
		if (!resource.fore_color)
		    resource.fore_color =
			(resource.reverse ? "white" : "black");
		if (!resource.back_color)
		    resource.back_color =
			(resource.reverse ? "black" : "white");
#endif
	    }
	    XFree(list);
	}
    }

    if (resource.install == True && our_visual->class == PseudoColor) {
	XColor tmp_color;

#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    fprintf(stdout, "--- PseudoColor, trying to install colormap\n");
#endif
	/* This next bit makes sure that the standard black and white pixels
	   are allocated in the new colormap. */

	tmp_color.pixel = BlackPixelOfScreen(SCRN);
	XQueryColor(DISP, our_colormap, &tmp_color);
	XAllocColor(DISP, our_colormap, &tmp_color);

	tmp_color.pixel = WhitePixelOfScreen(SCRN);
	XQueryColor(DISP, our_colormap, &tmp_color);
	XAllocColor(DISP, our_colormap, &tmp_color);

	our_colormap = XCopyColormapAndFree(DISP, our_colormap);
#ifdef TOOLKIT
	temp_args1a[0].value = (XtArgVal) our_colormap;
	XtSetValues(top_level, temp_args1a, 1);
#endif
    }

#ifdef TOOLKIT
    XtGetApplicationResources(top_level, (XtPointer) & resource,
			      app_pixel_resources,
			      XtNumber(app_pixel_resources), (ArgList) NULL, 0);
#endif

#endif /* GREY */

#ifndef TOOLKIT
    fore_Pixel = (resource.fore_color ? string_to_pixel(&resource.fore_color)
		  : (resource.reverse ? WhitePixelOfScreen(SCRN)
		     : BlackPixelOfScreen(SCRN)));
    back_Pixel = (resource.back_color ? string_to_pixel(&resource.back_color)
		  : (resource.reverse ? BlackPixelOfScreen(SCRN)
		     : WhitePixelOfScreen(SCRN)));
    brdr_Pixel = (brdr_color ? string_to_pixel(&brdr_color) : fore_Pixel);
    hl_Pixel = (high_color ? string_to_pixel(&high_color) : fore_Pixel);
    cr_Pixel = (curs_color ? string_to_pixel(&curs_color) : fore_Pixel);

    if (resource.rule_color)
	rule_pixel = string_to_pixel(&resource.rule_color);

#endif /* not TOOLKIT */

    copy = resource.copy;

#ifdef	GREY
    if (our_depth == 1) {
#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    fprintf(stdout, "--- using depth 1\n");
#endif
	use_grey = False;
    }

    if (use_grey && our_visual->class != TrueColor) {
#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    fprintf(stdout, "--- using grey, but not TrueColor\n");
#endif
	fore_color_data.pixel = fore_Pixel;
	XQueryColor(DISP, our_colormap, &fore_color_data);
	back_color_data.pixel = back_Pixel;
	XQueryColor(DISP, our_colormap, &back_color_data);
	init_plane_masks();
	if (!copy) {
#ifdef XSERVER_INFO
	    if (debug & DBG_PK)
		fprintf(stdout, "--- not using copy\n");
#endif
	    back_color_data.pixel = back_Pixel;
	    XStoreColor(DISP, our_colormap, &back_color_data);
	}
    }
#endif

    /*
     *  Step 3:  Initialize the dvi file and set titles.
     */

#ifdef T1LIB
    /* At this point DISP, our_visual, our_depth and our_colormap must
       be defined, and they are */
    if (resource.t1lib) {
	init_t1();
    }
#endif /* T1LIB */

    init_dvi_file();

    set_icon_and_title(dvi_name, &icon_name, &title_name, 0);

    /*
     *  Step 4:  Set initial window size.
     *  This needs to be done before colors are assigned because if
     *  -s 0 is specified, then we need to compute the shrink factor
     *  (which in turn affects whether init_pix is called).
     */

#ifdef	TOOLKIT

    /* The following code is lifted from Xterm */
    if (resource.icon_geometry != NULL) {
	int junk;

	(void)XGeometry(DISP, XScreenNumberOfScreen(SCRN),
			resource.icon_geometry, "", 0, 0, 0, 0, 0,
			(int *)&temp_args2[0].value,
			(int *)&temp_args2[1].value, &junk, &junk);
	XtSetValues(top_level, temp_args2, XtNumber(temp_args2));
    }
    /* Set icon pixmap */
    XtGetValues(top_level, &temp_args4, 1);
    if (icon_pm == (Pixmap) 0) {
	temp_args4.value = (XtArgVal) (XCreateBitmapFromData(DISP,
							     RootWindowOfScreen
							     (SCRN),
							     (_Xconst char *)
							     xdvi_bits,
							     xdvi_width,
							     xdvi_height));
	XtSetValues(top_level, &temp_args4, 1);
    }
    temp_args5[0].value = (XtArgVal) title_name;
    temp_args5[1].value = (XtArgVal) icon_name;
    XtSetValues(top_level, temp_args5, XtNumber(temp_args5));

    {
	XrmDatabase db;

	db = XtScreenDatabase(SCRN);

	XrmPutStringResource(&db,
#if !MOTIF
#if BUTTONS
			     "XDvi.form.baseTranslations",
#else
			     "XDvi.vport.baseTranslations",
#endif
#else /* MOTIF */
#if BUTTONS
			     "XDvi.form.vport.drawing.baseTranslations",
#else
			     "XDvi.vport.drawing.baseTranslations",
#endif
#endif /* MOTIF */
			     base_translations);

	if (resource.main_translations != NULL)
	    XrmPutStringResource(&db,
#if !MOTIF
#if BUTTONS
				 "XDvi.form.translations",
#else
				 "XDvi.vport.translations",
#endif
#else /* MOTIF */
#if BUTTONS
				 "XDvi.form.vport.drawing.translations",
#else
				 "XDvi.vport.drawing.translations",
#endif
#endif /* MOTIF */
				 resource.main_translations);

	/* Notwithstanding the ampersand, XrmPutStringResource() does
	 * not change db (X11R6.4 source).  */
    }

#if BUTTONS

    form_widget = XtCreateManagedWidget("form", FORM_WIDGET_CLASS,
					top_level, form_args,
					XtNumber(form_args));
#define	form_or_top	form_widget	/* for calls later on */
#define	form_or_vport	form_widget

#else /* BUTTONS */

#define	form_or_top	top_level	/* for calls later on */
#define	form_or_vport	vport_widget
#endif /* BUTTONS */

    vport_widget = XtCreateManagedWidget("vport", VPORT_WIDGET_CLASS,
					 form_or_top, vport_args,
					 XtNumber(vport_args));

#ifndef MOTIF

    clip_widget = XtNameToWidget(vport_widget, "clip");

#else /* MOTIF */

    menubar = XmVaCreateSimpleMenuBar(vport_widget, "menubar",
				      XmNdepth, (XtArgVal) our_depth,
				      XmNvisual, (XtArgVal) our_visual,
				      XmNcolormap, (XtArgVal) our_colormap,
				      XmVaCASCADEBUTTON,
				      (XtArgVal) XmCvtCTToXmString("File"), 0,
				      XmVaCASCADEBUTTON,
				      (XtArgVal) XmCvtCTToXmString("Navigate"),
				      0, XmVaCASCADEBUTTON,
				      (XtArgVal) XmCvtCTToXmString("Scale"), 0,
				      0);
    XmVaCreateSimplePulldownMenu(menubar, "file_pulldown", 0,
				 file_pulldown_callback, XmNtearOffModel,
				 (XtArgVal) XmTEAR_OFF_ENABLED, XmNdepth,
				 (XtArgVal) our_depth, XmNvisual,
				 (XtArgVal) our_visual, XmNcolormap,
				 (XtArgVal) our_colormap, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Reload"), 0, 0,
				 0, XmVaSEPARATOR, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Quit"), 0, 0, 0,
				 0);
    XmVaCreateSimplePulldownMenu(menubar, "navigate_pulldown", 1,
				 navigate_pulldown_callback, XmNtearOffModel,
				 (XtArgVal) XmTEAR_OFF_ENABLED, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Page-10"), 0, 0,
				 0, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Page-5"), 0, 0,
				 0, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Prev"), 0, 0, 0,
				 XmVaSEPARATOR, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Next"), 0, 0, 0,
				 XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Page+5"), 0, 0,
				 0, XmVaPUSHBUTTON,
				 (XtArgVal) XmCvtCTToXmString("Page+10"), 0, 0,
				 0, 0);
    scale_menu = XmVaCreateSimplePulldownMenu(menubar, "scale_pulldown", 2,
					      scale_pulldown_callback, XmNradioBehavior,
					      (XtArgVal) True, XmNtearOffModel,
					      (XtArgVal) XmTEAR_OFF_ENABLED,
					      XmVaRADIOBUTTON,
					      (XtArgVal) XmCvtCTToXmString("Shrink1"), 0,
					      0, 0, XmVaRADIOBUTTON,
					      (XtArgVal) XmCvtCTToXmString("Shrink2"), 0,
					      0, 0, XmVaRADIOBUTTON,
					      (XtArgVal) XmCvtCTToXmString("Shrink3"), 0,
					      0, 0, XmVaRADIOBUTTON,
					      (XtArgVal) XmCvtCTToXmString("Shrink4"), 0,
					      0, 0, 0);
    shrink_button[0] = XtNameToWidget(scale_menu, "button_0");
    shrink_button[1] = XtNameToWidget(scale_menu, "button_1");
    shrink_button[2] = XtNameToWidget(scale_menu, "button_2");
    shrink_button[3] = XtNameToWidget(scale_menu, "button_3");

    x_bar = XtNameToWidget(vport_widget, "HorScrollBar");
    y_bar = XtNameToWidget(vport_widget, "VertScrollBar");

#endif /* MOTIF */

    draw_args[0].value = (XtArgVal) page_w;
    draw_args[1].value = (XtArgVal) page_h;
    draw_widget = XtCreateManagedWidget("drawing", DRAW_WIDGET_CLASS,
					vport_widget, draw_args,
					XtNumber(draw_args));

#ifdef MOTIF
    XtVaSetValues(vport_widget, XmNworkWindow, draw_widget,
		  XmNmenuBar, menubar, NULL);
    XtManageChild(menubar);
    XtVaGetValues(vport_widget, XmNclipWindow, &clip_widget, NULL);
#endif /* MOTIF */

    XtOverrideTranslations(
#if !MOTIF
			   form_or_vport,
#else
			   draw_widget,
#endif
			   XtParseTranslationTable("\
\"0\":digit(0)\n\
\"1\":digit(1)\n\
\"2\":digit(2)\n\
\"3\":digit(3)\n\
\"4\":digit(4)\n\
\"5\":digit(5)\n\
\"6\":digit(6)\n\
\"7\":digit(7)\n\
\"8\":digit(8)\n\
\"9\":digit(9)\n\
\"-\":minus()\n\
<Motion>:motion()\n\
<BtnUp>:release()"));

    if (resource.wheel_unit != 0 && resource.wheel_translations != NULL) {
#if !MOTIF
	XtOverrideTranslations(form_or_vport,
			       XtParseTranslationTable(resource.
						       wheel_translations));
#else /* MOTIF */
#if !BUTTONS
	XtTranslations wheel_trans_table;
#endif

	wheel_trans_table
	    = XtParseTranslationTable(resource.wheel_translations);
	XtOverrideTranslations(draw_widget, wheel_trans_table);
#endif
    }

    /* set background colors of the drawing and clip widgets */
    {
	static Arg back_args = { XtNbackground, (XtArgVal) 0 };

	back_args.value = back_Pixel;
	XtSetValues(draw_widget, &back_args, 1);
	XtSetValues(clip_widget, &back_args, 1);
    }

    /* determine default window size */
#if BUTTONS
    if (!resource.expert)
	create_buttons();
#else
#define	xtra_wid	0
#endif /* BUTTONS */
    {
	XtWidgetGeometry constraints;
	XtWidgetGeometry reply;

	XtGetValues(top_level, &temp_args3, 1);	/* get border width */
	screen_w = WidthOfScreen(SCRN) - 2 * bwidth - xtra_wid;
	screen_h = HeightOfScreen(SCRN) - 2 * bwidth;
	for (;;) {	/* actually, at most two passes */
	    constraints.request_mode = reply.request_mode = 0;
	    constraints.width = page_w;
	    if (page_w > screen_w) {
		constraints.request_mode = CWWidth;
		constraints.width = screen_w;
	    }
	    constraints.height = page_h;
	    if (page_h > screen_h) {
		constraints.request_mode |= CWHeight;
		constraints.height = screen_h;
	    }
	    if (constraints.request_mode != 0
		&& constraints.request_mode != (CWWidth | CWHeight))
		(void)XtQueryGeometry(vport_widget, &constraints, &reply);
	    if (!(reply.request_mode & CWWidth))
		reply.width = constraints.width;
	    if (reply.width >= screen_w)
		reply.width = screen_w;
	    if (!(reply.request_mode & CWHeight))
		reply.height = constraints.height;
	    if (reply.height >= screen_h)
		reply.height = screen_h;

	    /* now reply.{width,height} contain max. usable window size */

	    if (currwin.shrinkfactor != 0)
		break;

	    currwin.shrinkfactor = ROUNDUP(unshrunk_page_w, reply.width - 2);
	    i = ROUNDUP(unshrunk_page_h, reply.height - 2);
	    if (i >= currwin.shrinkfactor)
		currwin.shrinkfactor = i;
	    if (currwin.shrinkfactor > 1)
		bak_shrink = currwin.shrinkfactor;
	    mane.shrinkfactor = currwin.shrinkfactor;
	    init_page();
	    set_wh_args[0].value = (XtArgVal) page_w;
	    set_wh_args[1].value = (XtArgVal) page_h;
	    XtSetValues(draw_widget, set_wh_args, XtNumber(set_wh_args));
	}
	set_wh_args[0].value = (XtArgVal) (reply.width + xtra_wid);
	set_wh_args[1].value = (XtArgVal) reply.height;
	XtSetValues(top_level, set_wh_args, XtNumber(set_wh_args));

#ifdef	BUTTONS
	set_wh_args[0].value -= xtra_wid;
	XtSetValues(vport_widget, set_wh_args, XtNumber(set_wh_args));
#endif /* BUTTONS */
    }
#ifdef MOTIF
    set_shrink_factor(mane.shrinkfactor);
#endif

#else /* not TOOLKIT */

    screen_w = WidthOfScreen(SCRN) - 2 * bwidth;
    screen_h = HeightOfScreen(SCRN) - 2 * bwidth;

    size_hints.flags = PMinSize;
    size_hints.min_width = size_hints.min_height = 50;
    size_hints.x = size_hints.y = 0;

    /* compute largest possible window */
    flag = 0;
    if (geometry != NULL) {
	flag = XParseGeometry(geometry, &size_hints.x, &size_hints.y,
			      &window_w, &window_h);
	if (flag & (XValue | YValue))
	    size_hints.flags |= USPosition;
	if (flag & (WidthValue | HeightValue))
	    size_hints.flags |= USSize;
    }
    if (!(flag & WidthValue))
	window_w = screen_w;
    if (!(flag & HeightValue))
	window_h = screen_h;

    if (currwin.shrinkfactor == 0) {
	/* compute best shrink factor based on window_w and window_h */
	currwin.shrinkfactor = ROUNDUP(unshrunk_page_w, window_w - 2);
	i = ROUNDUP(unshrunk_page_h, window_h - 2);
	if (i >= currwin.shrinkfactor)
	    currwin.shrinkfactor = i;
	if (currwin.shrinkfactor > 1)
	    bak_shrink = currwin.shrinkfactor;
	mane.shrinkfactor = currwin.shrinkfactor;
	init_page();
    }

    if (window_w < page_w)
	x_thick = BAR_THICK;
    if (window_h < page_h + x_thick)
	y_thick = BAR_THICK;
    if (!(flag & WidthValue)) {
	window_w = page_w + y_thick;
	if (window_w > screen_w) {
	    x_thick = BAR_THICK;
	    window_w = screen_w;
	}
	size_hints.flags |= PSize;
    }
    if (!(flag & HeightValue)) {
	window_h = page_h + x_thick;
	if (window_h > screen_h)
	    window_h = screen_h;
	size_hints.flags |= PSize;
    }

    if (flag & XNegative)
	size_hints.x += screen_w - window_w;
    if (flag & YNegative)
	size_hints.y += screen_h - window_h;

#endif /* not TOOLKIT */

    /*
     *        Step 5:  Realize the widgets (or windows).
     */

#ifdef	TOOLKIT


#if BUTTONS && !MOTIF
    if (!resource.expert) {
	set_button_panel_height(set_wh_args[1].value);
    }
#endif /* BUTTONS && !MOTIF */
#if MOTIF
    XtSetSensitive(draw_widget, TRUE);
#endif /* MOTIF */
    XtAddEventHandler(top_level, PropertyChangeMask, False,
		      handle_property_change, (XtPointer) NULL);
    XtAddEventHandler(vport_widget, StructureNotifyMask, False,
		      handle_resize, (XtPointer) NULL);
    XtAddEventHandler(draw_widget, ExposureMask, False, handle_expose,
		      (XtPointer) & mane);
    XtRealizeWidget(top_level);
#ifdef STATUSLINE
    if (resource.statusline) {
	create_statusline();
    }
#endif

#ifdef MOTIF
    XmProcessTraversal(draw_widget, XmTRAVERSE_CURRENT);
#endif /* MOTIF */

    currwin.win = mane.win = XtWindow(draw_widget);

    {
	XWindowAttributes attrs;

	(void)XGetWindowAttributes(DISP, mane.win, &attrs);
	backing_store = attrs.backing_store;
    }

#else /* TOOLKIT */

    size_hints.width = window_w;
    size_hints.height = window_h;
#ifndef GREY
    top_level = XCreateSimpleWindow(DISP, RootWindowOfScreen(SCRN),
				    size_hints.x, size_hints.y, window_w,
				    window_h, bwidth, brdr_Pixel, back_Pixel);
#else /* GREY */
    {
	XSetWindowAttributes attr;

	attr.border_pixel = brdr_Pixel;
	attr.background_pixel = back_Pixel;
	attr.colormap = our_colormap;
	top_level = XCreateWindow(DISP, RootWindowOfScreen(SCRN),
				  size_hints.x, size_hints.y, window_w,
				  window_h, bwidth, our_depth, InputOutput,
				  our_visual,
				  CWBorderPixel | CWBackPixel | CWColormap,
				  &attr);
    }
#endif /* GREY */
    XSetStandardProperties(DISP, top_level, title_name, icon_name,
			   (Pixmap) 0, argv, argc, &size_hints);

    wmhints.flags = InputHint | StateHint | IconPixmapHint;
    wmhints.input = True;	/* window manager must direct input */
    wmhints.initial_state = iconic ? IconicState : NormalState;
    wmhints.icon_pixmap = XCreateBitmapFromData(DISP,
						RootWindowOfScreen(SCRN),
						(_Xconst char *)xdvi_bits,
						xdvi_width, xdvi_height);
    if (resource.icon_geometry != NULL) {
	int junk;

	wmhints.flags |= IconPositionHint;
	(void)XGeometry(DISP, DefaultScreen(DISP), resource.icon_geometry,
			"", 0, 0, 0, 0, 0, &wmhints.icon_x, &wmhints.icon_y,
			&junk, &junk);
    }
    XSetWMHints(DISP, top_level, &wmhints);

    XSelectInput(DISP, top_level,
		 KeyPressMask | StructureNotifyMask | PropertyChangeMask);

    /* ... and off we go! */
    XMapWindow(DISP, top_level);
    XFlush(DISP);


    {
	static KeySym list[2] = { XK_Caps_Lock, XK_Num_Lock };

#define	rebindkey(ks, str)	XRebindKeysym(DISP, (KeySym) ks, \
		  (KeySym *) NULL, 0, (_Xconst ubyte *) str, 1); \
		XRebindKeysym(DISP, (KeySym) ks, \
		  list, 1, (_Xconst ubyte *) str, 1); \
		XRebindKeysym(DISP, (KeySym) ks, \
		  list + 1, 1, (_Xconst ubyte *) str, 1); \
		XRebindKeysym(DISP, (KeySym) ks, \
		  list, 2, (_Xconst ubyte *) str, 1);

	rebindkey(XK_Help, "?");
	rebindkey(XK_Cancel, "q");
	rebindkey(XK_Redo, "A");
	rebindkey(XK_Home, "^");
	rebindkey(XK_Left, "l");
	rebindkey(XK_Up, "u");
	rebindkey(XK_Right, "r");
	rebindkey(XK_Down, "d");
	rebindkey(XK_End, "g");
#ifdef  XK_Page_Up
	rebindkey(XK_Page_Up, "b");
	rebindkey(XK_Page_Down, "f");
#endif
#ifdef	XK_KP_Left
	rebindkey(XK_KP_Home, "^");
	rebindkey(XK_KP_Left, "l");
	rebindkey(XK_KP_Up, "u");
	rebindkey(XK_KP_Right, "r");
	rebindkey(XK_KP_Down, "d");
	rebindkey(XK_KP_Prior, "b");
	rebindkey(XK_KP_Next, "f");
	rebindkey(XK_KP_Delete, "\177");
#ifdef  XK_KP_Page_Up
	rebindkey(XK_KP_Page_Up, "b");
	rebindkey(XK_KP_Page_Down, "f");
#endif /* XK_KP_Page_Up */
#endif /* XK_KP_Left */
    }
#undef	rebindkey
#endif /* TOOLKIT */

    image = XCreateImage(DISP, our_visual, 1, XYBitmap, 0,
			 (char *)NULL, 0, 0, BMBITS, 0);
    image->bitmap_unit = BMBITS;
#ifdef	WORDS_BIGENDIAN
    image->bitmap_bit_order = MSBFirst;
#else
    image->bitmap_bit_order = LSBFirst;
#endif
    {
	short endian = MSBFirst << 8 | LSBFirst;
	image->byte_order = *((char *)&endian);
    }

    /* Store window id for use by src_client_check().  */

    {
#ifndef WORD64
	xuint32 data = XtWindow(top_level);

	TRACE_CLIENT((stderr, "appending my window ID: 0x%x\n", (unsigned int)data));
	XChangeProperty(DISP, DefaultRootWindow(DISP),
			ATOM_XDVI_WINDOWS, ATOM_XDVI_WINDOWS, 32,
			PropModeAppend, (unsigned char *)&data, 1);
#else
	unsigned char data[4];

#if WORDS_BIGENDIAN
	data[0] = (unsigned int)XtWindow(top_level) >> 24;
	data[1] = (unsigned int)XtWindow(top_level) >> 16;
	data[2] = (unsigned int)XtWindow(top_level) >> 8;
	data[3] = (unsigned int)XtWindow(top_level);
#else
	data[0] = (unsigned int)XtWindow(top_level);
	data[1] = (unsigned int)XtWindow(top_level) >> 8;
	data[2] = (unsigned int)XtWindow(top_level) >> 16;
	data[3] = (unsigned int)XtWindow(top_level) >> 24;
#endif
	TRACE_CLIENT((stderr, "appending my window ID: 0x%x\n", data));
	XChangeProperty(DISP, DefaultRootWindow(DISP),
			ATOM_XDVI_WINDOWS, ATOM_XDVI_WINDOWS, 32,
			PropModeAppend, data, 1);
#endif
    }
    set_dvi_property();

    /*
     *      Step 6:  Assign colors and GCs.
     *               Because of the latter, this has to go after Step 5.
     */

#define	MakeGC(fcn, fg, bg)	(values.function = fcn, \
	  values.foreground=fg, values.background=bg, \
	  XCreateGC(DISP, XtWindow(top_level), \
	    GCFunction | GCForeground | GCBackground, &values))

    if (!resource.rule_color)
	resource.rule_pixel = fore_Pixel;


#ifdef	GREY
    if (resource._gamma == 0.0)
	resource._gamma = 1.0;

    if (use_grey)
	init_pix();
    else
#endif
    {
	XGCValues values;
	Pixel set_bits = (Pixel) (fore_Pixel & ~back_Pixel);
	Pixel clr_bits = (Pixel) (back_Pixel & ~fore_Pixel);
	
	copyGC = MakeGC(GXcopy, fore_Pixel, back_Pixel);
	if (copy || (set_bits && clr_bits)) {
	    ruleGC = copyGC;
	    if (!resource.thorough)
		copy = True;
	}
	if (copy) {
	    foreGC = ruleGC;
	    if (!resource.copy)
		do_popup_message(MSG_WARN,
				     /* helptext */
				     "Greyscaling is running in copy mode:  your display can only display \
a limited number of colors at a time (typically 256), and other applications\
 (such as netscape) are using many of them.  Running in copy mode will \
cause overstrike characters to appear incorrectly, and may result in \
poor display quality. \n\
You can either restart xdvi with the \"-install\" option \
to allow it to install its own color map, \
or terminate other color-hungry applications before restarting xdvi. \
Please see the section ``GREYSCALING AND COLORMAPS'' in the xdvi manual page \
for more details.",
			 /* text */
			 "Couldn't allocate enough colors - expect low display quality.");
	}
 	else {
	    if (set_bits)
		foreGC = MakeGC(GXor, set_bits, 0);
	    if (clr_bits || !set_bits)
		*(foreGC ? &foreGC2 : &foreGC) =
		    MakeGC(GXandInverted, clr_bits, 0);
	    if (!ruleGC)
		ruleGC = foreGC;
	}
    }	/* else (use_grey) */

    {
	XGCValues values;

	highGC = ruleGC;
	if (hl_Pixel != fore_Pixel
#ifdef GREY
	    || (!copy && our_visual != DefaultVisualOfScreen(SCRN))
#endif
	    )
	highGC = MakeGC(GXcopy, hl_Pixel, back_Pixel);

	rulerGC = MakeGC(GXcopy, resource.rule_pixel, fore_Pixel);

    }
    /*
     * There's a bug in the Xaw toolkit, in which it uses the
     * DefaultGCOfScreen to do vertical scrolling in the Text widget.
     * This leads to a BadMatch error if our visual is not the default one.
     * The following kludge works around this.
     */
    
    DefaultGCOfScreen(SCRN) = copyGC;
    
#ifndef	VMS
    ready_cursor = XCreateFontCursor(DISP, XC_cross);
    redraw_cursor = XCreateFontCursor(DISP, XC_watch);
    link_cursor = XCreateFontCursor(DISP, XC_hand2);
#else
    DECWCursorFont = XLoadFont(DISP, "DECW$CURSOR");
    XSetFont(DISP, foreGC, DECWCursorFont);
    redraw_cursor = XCreateGlyphCursor(DISP, DECWCursorFont, DECWCursorFont,
				       decw$c_wait_cursor,
				       decw$c_wait_cursor + 1,
				       &resource.fore_color,
				       &resource.back_color);
    MagnifyPixmap = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN), mag_glass_bits,
					  mag_glass_width, mag_glass_height);
    ready_cursor = XCreatePixmapCursor(DISP, MagnifyPixmap, MagnifyPixmap,
				       &resource.back_color, &resource.fore_color,
				       mag_glass_x_hot, mag_glass_y_hot);
    /* SU: dunno about VMS - use the defaults for the time being: */
    link_cursor = ready_cursor;
#endif /* VMS */

    {
	XColor bg_Color, cr_Color;
	Pixmap arrow_pixmap, arrow_mask;

	bg_Color.pixel = back_Pixel;
	XQueryColor(DISP, our_colormap, &bg_Color);
	cr_Color.pixel = cr_Pixel;
	XQueryColor(DISP, our_colormap, &cr_Color);
	XRecolorCursor(DISP, ready_cursor, &cr_Color, &bg_Color);
	XRecolorCursor(DISP, redraw_cursor, &cr_Color, &bg_Color);

	arrow_pixmap = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					     (char *)drag_vert_bits, drag_vert_width,
					     drag_vert_height);
	arrow_mask = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					   (char *)drag_vert_mask, drag_vert_width,
					   drag_vert_height);
	drag_cursor[0] = XCreatePixmapCursor(DISP, arrow_pixmap, arrow_mask, &cr_Color,
					     &bg_Color, drag_vert_x_hot, drag_vert_y_hot);
	XFreePixmap(DISP, arrow_pixmap);
	XFreePixmap(DISP, arrow_mask);

	arrow_pixmap = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					     (char *)drag_horiz_bits, drag_horiz_width,
					     drag_horiz_height);
	arrow_mask = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					   (char *)drag_horiz_mask, drag_horiz_width,
					   drag_horiz_height);
	drag_cursor[1] = XCreatePixmapCursor(DISP, arrow_pixmap, arrow_mask, &cr_Color,
					     &bg_Color, drag_horiz_x_hot, drag_horiz_y_hot);
	XFreePixmap(DISP, arrow_pixmap);
	XFreePixmap(DISP, arrow_mask);

	arrow_pixmap = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					     (char *)drag_omni_bits, drag_omni_width,
					     drag_omni_height);
	arrow_mask = XCreateBitmapFromData(DISP, RootWindowOfScreen(SCRN),
					   (char *)drag_omni_mask, drag_omni_width,
					   drag_omni_height);
	drag_cursor[2] = XCreatePixmapCursor(DISP, arrow_pixmap, arrow_mask, &cr_Color,
					     &bg_Color, drag_omni_x_hot, drag_omni_y_hot);
	XFreePixmap(DISP, arrow_pixmap);
	XFreePixmap(DISP, arrow_mask);
    }

    enable_intr();
    
    if (curr_page) {
	current_page = (*curr_page ? atoi(curr_page) : total_pages) - 1;
	current_page = check_goto_page(current_page);
    }

    if (resource.src_pos != NULL) {
	if (!setjmp(canit_env))
	    source_forward_search(resource.src_pos);
    }

#ifdef PS_GS
    FIXME_ps_lock = False;
#endif
    do_pages();
    xdvi_exit(0);	/* do_pages() returns if DBG_BATCH is specified */
    /* make gcc happy */
    return 0;
}
