/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * NOTE:
 *	xdvi is based on prior work as noted in the modification history, below.
 */

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
 * 4/1994	Added DPS support, bounding box
 *					--Ricardo Telichevesky
 *					  and Luis Miguel Silveira, MIT RLE.
 *
 *	Compilation options:
 *	SYSV	compile for System V
 *	VMS	compile for VMS
 *	NOTOOL	compile without toolkit
 *	BUTTONS	compile with buttons on the side of the window (needs toolkit)
 *	MSBITFIRST	store bitmaps internally with most significant bit first
 *	BMSHORT	store bitmaps in shorts instead of bytes
 *	BMLONG	store bitmaps in longs instead of bytes
 *	ALTFONT	default for -altfont option
 *	A4	use European size paper
 *	TEXXET	support reflection dvi codes (right-to-left typesetting)
 *	GREY	use grey levels to shrink fonts
 *	PS_DPS	use display postscript to render pictures/bounding boxes
 *	PS_NEWS	use the NeWS server to render pictures/bounding boxes
 *	PS_GS	use Ghostscript to render pictures/bounding boxes
 *	GS_PATH	path to call the Ghostscript interpreter by
 */

#ifndef	lint
static	char	copyright[] =
"@(#) Copyright (c) 1994 Paul Vojta.  All rights reserved.\n";
#endif

#ifndef	ALTFONT
#define	ALTFONT	"cmr10"
#endif

#if	defined(PS_GS) && !defined(GS_PATH)
#define	GS_PATH	"gs"
#endif

#ifndef	A4
#define	DEFAULT_PAPER		"us"
#else
#define	DEFAULT_PAPER		"a4"
#endif

#define	EXTERN
#define	INIT(x)	=x

#include <math.h>	/* includes atof() */
#include <ctype.h>
#include "xdvi.h"

#include "patchlevel.h"
static	_Xconst	struct {char	a[24], b, c, d;}
	version = {"This is xdvi patchlevel ", '0' + PATCHLEVEL / 10,
		'0' + PATCHLEVEL % 10, 0};

#ifdef	X_NOT_STDC_ENV
#ifndef	atof
extern	double	atof ARGS((_Xconst char *));
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
#include <X11/Xaw/Viewport.h>
#ifdef	BUTTONS
#include <X11/Xaw/Command.h>
#endif
#else	/* XtSpecificationRelease < 4 */
#define	XtPointer caddr_t
#include <X11/Viewport.h>
#ifdef	BUTTONS
#include <X11/Command.h>
#endif
#endif	/* XtSpecificationRelease */
#if	XtSpecificationRelease < 5
typedef	caddr_t	XPointer;
#endif
#else	/* !TOOLKIT */
typedef	int		Position;
#endif	/* TOOLKIT */

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
static	int	DECWCursorFont;	/* Space for the DECWindows cursor font  */
static	Pixmap	MagnifyPixmap;	/* Pixmap to hold our special mag-glass  */
#include <X11/Xresource.h>      /* Motif apparently needs this one */
#endif	/* VMS */

/*
 * Command line flags.
 */

static	Dimension	bwidth	= 2;

#define	fore_Pixel	resource._fore_Pixel
#define	back_Pixel	resource._back_Pixel
#ifdef	TOOLKIT
struct _resource	resource;
#define	brdr_Pixel	resource._brdr_Pixel
#define	hl_Pixel	resource._hl_Pixel
#define	cr_Pixel	resource._cr_Pixel
#else	/* TOOLKIT */
static	Pixel		hl_Pixel, cr_Pixel;
#endif	/* TOOLKIT */

struct	mg_size_rec	mg_size[5]	= {{200, 150}, {400, 250}, {700, 500},
					   {1000, 800}, {1200, 1200}};

static	char	*curr_page;

struct WindowRec mane	= {(Window) 0, 3, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0};
struct WindowRec alt	= {(Window) 0, 1, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0};
/*	currwin is temporary storage except for within redraw() */
struct WindowRec currwin = {(Window) 0, 3, 0, 0, 0, 0, MAXDIM, 0, MAXDIM, 0};

#ifdef	lint
#ifdef	TOOLKIT
WidgetClass	viewportWidgetClass, widgetClass;
#ifdef	BUTTONS
WidgetClass	formWidgetClass, compositeWidgetClass, commandWidgetClass;
#endif	/* BUTTONS */
#endif	/* TOOLKIT */
#endif	/* lint */

/*
 *	Data for options processing
 */

static	_Xconst	char	silent[] = " ";	/* flag value for usage() */

static	_Xconst	char	subst[]	= "x";	/* another flag value */

static	_Xconst char	*subst_val[] = {"-mgs[n] <size>"};

#ifdef	TOOLKIT

static	XrmOptionDescRec	options[] = {
{"-s",		".shrinkFactor", XrmoptionSepArg,	(caddr_t) NULL},
#ifndef	VMS
{"-S",		".densityPercent", XrmoptionSepArg,	(caddr_t) NULL},
#endif
{"-density",	".densityPercent", XrmoptionSepArg,	(caddr_t) NULL},
#ifdef	GREY
{"-nogrey",	".grey",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nogrey",	".grey",	XrmoptionNoArg,		(caddr_t) "on"},
{"-gamma",	".gamma",	XrmoptionSepArg,	(caddr_t) NULL},
#endif
{"-p",		".pixelsPerInch", XrmoptionSepArg,	(caddr_t) NULL},
{"-margins",	".Margin",	XrmoptionSepArg,	(caddr_t) NULL},
{"-sidemargin",	".sideMargin",	XrmoptionSepArg,	(caddr_t) NULL},
{"-topmargin",	".topMargin",	XrmoptionSepArg,	(caddr_t) NULL},
{"-offsets",	".Offset",	XrmoptionSepArg,	(caddr_t) NULL},
{"-xoffset",	".xOffset",	XrmoptionSepArg,	(caddr_t) NULL},
{"-yoffset",	".yOffset",	XrmoptionSepArg,	(caddr_t) NULL},
{"-paper",	".paper",	XrmoptionSepArg,	(caddr_t) NULL},
{"-altfont",	".altFont",	XrmoptionSepArg,	(caddr_t) NULL},
#ifdef	MAKEPK
{"-nomakepk",	".makePk",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nomakepk",	".makePk",	XrmoptionNoArg,		(caddr_t) "on"},
#endif
{"-mfmode",	".mfMode",	XrmoptionSepArg,	(caddr_t) NULL},
{"-l",		".listFonts",	XrmoptionNoArg,		(caddr_t) "on"},
{"+l",		".listFonts",	XrmoptionNoArg,		(caddr_t) "off"},
#ifdef	BUTTONS
{"-expert",	".expert",	XrmoptionNoArg,		(caddr_t) "on"},
{"+expert",	".expert",	XrmoptionNoArg,		(caddr_t) "off"},
#endif
{"-mgs",	".magnifierSize1",XrmoptionSepArg,	(caddr_t) NULL},
{"-mgs1",	".magnifierSize1",XrmoptionSepArg,	(caddr_t) NULL},
{"-mgs2",	".magnifierSize2",XrmoptionSepArg,	(caddr_t) NULL},
{"-mgs3",	".magnifierSize3",XrmoptionSepArg,	(caddr_t) NULL},
{"-mgs4",	".magnifierSize4",XrmoptionSepArg,	(caddr_t) NULL},
{"-mgs5",	".magnifierSize5",XrmoptionSepArg,	(caddr_t) NULL},
{"-hush",	".Hush",	XrmoptionNoArg,		(caddr_t) "on"},
{"+hush",	".Hush",	XrmoptionNoArg,		(caddr_t) "off"},
{"-hushspecials", ".hushSpecials", XrmoptionNoArg,	(caddr_t) "on"},
{"+hushspecials", ".hushSpecials", XrmoptionNoArg,	(caddr_t) "off"},
{"-hushchars",	".hushLostChars", XrmoptionNoArg,	(caddr_t) "on"},
{"+hushchars",	".hushLostChars", XrmoptionNoArg,	(caddr_t) "off"},
{"-hushchecksums", ".hushChecksums", XrmoptionNoArg,	(caddr_t) "on"},
{"+hushchecksums", ".hushChecksums", XrmoptionNoArg,	(caddr_t) "off"},
{"-safer",	".safer",	XrmoptionNoArg,		(caddr_t) "on"},
{"+safer",	".safer",	XrmoptionNoArg,		(caddr_t) "off"},
{"-fg",		".foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-foreground",	".foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",		".background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-background",	".background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-hl",		".highlight",	XrmoptionSepArg,	(caddr_t) NULL},
{"-cr",		".cursorColor",	XrmoptionSepArg,	(caddr_t) NULL},
{"-icongeometry",".iconGeometry",XrmoptionSepArg,	(caddr_t) NULL},
{"-keep",	".keepPosition",XrmoptionNoArg,		(caddr_t) "on"},
{"+keep",	".keepPosition",XrmoptionNoArg,		(caddr_t) "off"},
{"-copy",	".copy",	XrmoptionNoArg,		(caddr_t) "on"},
{"+copy",	".copy",	XrmoptionNoArg,		(caddr_t) "off"},
{"-thorough",	".thorough",	XrmoptionNoArg,		(caddr_t) "on"},
{"+thorough",	".thorough",	XrmoptionNoArg,		(caddr_t) "off"},
#if	PS
{"-nopostscript",".postscript",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nopostscript",".postscript",	XrmoptionNoArg,		(caddr_t) "on"},
{"-noscan",	".prescan",	XrmoptionNoArg,		(caddr_t) "off"},
{"+noscan",	".prescan",	XrmoptionNoArg,		(caddr_t) "on"},
{"-allowshell",	".allowShell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+allowshell",	".allowShell",	XrmoptionNoArg,		(caddr_t) "off"},
#ifdef	PS_DPS
{"-nodps",	".dps",		XrmoptionNoArg,		(caddr_t) "off"},
{"+nodps",	".dps",		XrmoptionNoArg,		(caddr_t) "on"},
#endif
#ifdef	PS_NEWS
{"-nonews",	".news",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nonews",	".news",	XrmoptionNoArg,		(caddr_t) "on"},
#endif
#ifdef	PS_GS
{"-noghostscript",".ghostscript", XrmoptionNoArg,	(caddr_t) "off"},
{"+noghostscript",".ghostscript", XrmoptionNoArg,	(caddr_t) "on"},
{"-interpreter",".interpreter",	XrmoptionSepArg,	(caddr_t) NULL},
{"-nogssafer",	".gsSafer",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nogssafer",	".gsSafer",	XrmoptionNoArg,		(caddr_t) "on"},
{"-gspalette",	".palette",	XrmoptionSepArg,	(caddr_t) NULL},
#endif
#endif	/* PS */
{"-debug",	".debugLevel",	XrmoptionSepArg,	(caddr_t) NULL},
{"-version",	".version",	XrmoptionNoArg,		(caddr_t) "on"},
{"+version",	".version",	XrmoptionNoArg,		(caddr_t) "off"},
};

#define	offset(field)	XtOffsetOf(struct _resource, field)

static	int	basedpi	= BDPI;		/* default value for -p option */

static	char	XtRBool3[]	= "Bool3";	/* resource for Bool3 */

static	XtResource	application_resources[] = {
{"shrinkFactor", "ShrinkFactor", XtRInt, sizeof(int),
  offset(shrinkfactor), XtRString, "6"},
{"densityPercent", "DensityPercent", XtRInt, sizeof(int),
  offset(_density), XtRString, "20"},
#ifdef	GREY
{"gamma", "Gamma", XtRFloat, sizeof(float),
  offset(_gamma), XtRString, "1"},
#endif
{"pixelsPerInch", "PixelsPerInch", XtRInt, sizeof(int),
  offset(_pixels_per_inch), XtRInt, (caddr_t) &basedpi},
{"sideMargin", "Margin", XtRString, sizeof(char *),
  offset(sidemargin), XtRString, (caddr_t) NULL},
{"topMargin", "Margin", XtRString, sizeof(char *),
  offset(topmargin), XtRString, (caddr_t) NULL},
{"xOffset", "Offset", XtRString, sizeof(char *),
  offset(xoffset), XtRString, (caddr_t) NULL},
{"yOffset", "Offset", XtRString, sizeof(char *),
  offset(yoffset), XtRString, (caddr_t) NULL},
{"paper", "Paper", XtRString, sizeof(char *),
  offset(paper), XtRString, (caddr_t) DEFAULT_PAPER},
{"altFont", "AltFont", XtRString, sizeof(char *),
  offset(_alt_font), XtRString, (caddr_t) ALTFONT},
#ifdef	MAKEPK
{"makePk", "MakePk", XtRBoolean, sizeof(Boolean),
  offset(makepk), XtRString, "true"},
#endif
{"mfMode", "MfMode", XtRString, sizeof(char *),
  offset(mfmode), XtRString, NULL},
{"listFonts", "ListFonts", XtRBoolean, sizeof(Boolean),
  offset(_list_fonts), XtRString, "false"},
{"reverseVideo", "ReverseVideo", XtRBoolean, sizeof(Boolean),
  offset(reverse), XtRString, "false"},
{"hushSpecials", "Hush", XtRBoolean, sizeof(Boolean),
  offset(_hush_spec), XtRString, "false"},
{"hushLostChars", "Hush", XtRBoolean, sizeof(Boolean),
  offset(_hush_chars), XtRString, "false"},
{"hushChecksums", "Hush", XtRBoolean, sizeof(Boolean),
  offset(_hush_chk), XtRString, "false"},
{"safer", "Safer", XtRBoolean, sizeof(Boolean),
  offset(safer), XtRString, "false"},
{"foreground", "Foreground", XtRPixel, sizeof(Pixel),
  offset(_fore_Pixel), XtRPixel, (caddr_t) &resource._fore_Pixel},
{"foreground", "Foreground", XtRString, sizeof(char *),
  offset(fore_color), XtRString, (caddr_t) NULL},
{"background", "Background", XtRPixel, sizeof(Pixel),
  offset(_back_Pixel), XtRPixel, (caddr_t) &resource._back_Pixel},
{"background", "Background", XtRString, sizeof(char *),
  offset(back_color), XtRString, (caddr_t) NULL},
{"borderColor", "BorderColor", XtRPixel, sizeof(Pixel),
  offset(_brdr_Pixel), XtRPixel, (caddr_t) &resource._brdr_Pixel},
{"borderColor", "BorderColor", XtRString, sizeof(char *),
  offset(brdr_color), XtRString, (caddr_t) NULL},
{"highlight", "Highlight", XtRPixel, sizeof(Pixel),
  offset(_hl_Pixel), XtRPixel, (caddr_t) &resource._hl_Pixel},
{"highlight", "Highlight", XtRString, sizeof(char *),
  offset(high_color), XtRString, (caddr_t) NULL},
{"cursorColor", "CursorColor", XtRPixel, sizeof(Pixel),
  offset(_cr_Pixel), XtRPixel, (caddr_t) &resource._cr_Pixel},
{"cursorColor", "CursorColor", XtRString, sizeof(char *),
  offset(curs_color), XtRString, (caddr_t) NULL},
{"iconGeometry", "IconGeometry", XtRString, sizeof(char *),
  offset(icon_geometry), XtRString, (caddr_t) NULL},
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
{"interpreter", "Interpreter", XtRString, sizeof(char *),
  offset(gs_path), XtRString, (caddr_t) GS_PATH},
{"gsSafer", "Safer", XtRBoolean, sizeof(Boolean),
  offset(gs_safer), XtRString, "true"},
{"palette", "Palette", XtRString, sizeof(char *),
  offset(gs_palette), XtRString, (caddr_t) "Color"},
#endif
#endif	/* PS */
{"copy", "Copy", XtRBool3, sizeof(Bool3),
  offset(copy), XtRString, "maybe"},
{"thorough", "Thorough", XtRBoolean, sizeof(Boolean),
  offset(thorough), XtRString, "false"},
{"debugLevel", "DebugLevel", XtRString, sizeof(char *),
  offset(debug_arg), XtRString, (caddr_t) NULL},
{"version", "Version", XtRBoolean, sizeof(Boolean),
  offset(version_flag), XtRString, "false"},
#ifdef	BUTTONS
{"expert", "Expert", XtRBoolean, sizeof(Boolean),
  offset(expert), XtRString, "false"},
#endif
{"magnifierSize1", "MagnifierSize", XtRString, sizeof(char *),
  offset(mg_arg[0]), XtRString, (caddr_t) NULL},
{"magnifierSize2", "MagnifierSize", XtRString, sizeof(char *),
  offset(mg_arg[1]), XtRString, (caddr_t) NULL},
{"magnifierSize3", "MagnifierSize", XtRString, sizeof(char *),
  offset(mg_arg[2]), XtRString, (caddr_t) NULL},
{"magnifierSize4", "MagnifierSize", XtRString, sizeof(char *),
  offset(mg_arg[3]), XtRString, (caddr_t) NULL},
{"magnifierSize5", "MagnifierSize", XtRString, sizeof(char *),
  offset(mg_arg[4]), XtRString, (caddr_t) NULL},
#ifdef	GREY
{"grey", "Grey", XtRBoolean, sizeof(Boolean),
  offset(_use_grey), XtRString, "true"},
#endif
};
#undef	offset

static	_Xconst	char	*usagestr[] = {
	/* shrinkFactor */	"shrink",
#ifndef	VMS
	/* S */			"density",
	/* density */		silent,
#else
	/* density */		"density",
#endif
#ifdef	GREY
	/* gamma */		"g",
#endif
	/* p */			"pixels",
	/* margins */		"dimen",
	/* sidemargin */	"dimen",
	/* topmargin */		"dimen",
	/* offsets */		"dimen",
	/* xoffset */		"dimen",
	/* yoffset */		"dimen",
	/* paper */		"papertype",
	/* altfont */		"font",
	/* mfmode */		"mode-def",
	/* rv */		"^-l", "-rv",
	/* mgs */		subst,
	/* msg1 */		silent,
	/* msg2 */		silent,
	/* msg3 */		silent,
	/* msg4 */		silent,
	/* msg5 */		silent,
	/* fg */		"color",
	/* foreground */	silent,
	/* bg */		"color",
	/* background */	silent,
	/* hl */		"color",
	/* bd */		"^-hl", "-bd <color>",
	/* cr */		"color",
	/* bw */		"^-cr", "-bw <width>",
#ifndef VMS
	/* display */		"^-cr", "-display <host:display>",
#else
	/* display */		"^-cr", "-display <host::display>",
#endif
	/* geometry */		"^-cr", "-geometry <geometry>",
	/* icongeometry */	"geometry",
	/* iconic */		"^-icongeometry", "-iconic",
#ifdef	BUTTONS
	/* font */		"^-icongeometry", "-font <font>",
#endif
#ifdef	PS_GS
	/* interpreter */	"path",
	/* gspalette */		"monochrome|grayscale|color",
#endif
	/* debug */		"bitmask",
	/* [dummy] */		"z"
};

#ifdef	NOQUERY
#define	drawWidgetClass	widgetClass
#else

/* ARGSUSED */
static	XtGeometryResult
QueryGeometry(w, constraints, reply)
	Widget	w;
	XtWidgetGeometry *constraints, *reply;
{
	reply->request_mode = CWWidth | CWHeight;
	reply->width = page_w;
	reply->height = page_h;
	return XtGeometryAlmost;
}

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#ifdef	lint
WidgetClassRec	widgetClassRec;
#endif

	/* if the following gives you trouble, just compile with -DNOQUERY */
static	WidgetClassRec	drawingWidgetClass = {
  {
    /* superclass         */    &widgetClassRec,
    /* class_name         */    "Draw",
    /* widget_size        */    sizeof(WidgetRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/  NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    NULL,
    /* num_resources      */    0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    XtInheritTranslations,
    /* query_geometry       */  QueryGeometry,
    /* display_accelerator  */  XtInheritDisplayAccelerator,
    /* extension            */  NULL
  }
};

#define	drawWidgetClass	&drawingWidgetClass

#endif	/* NOQUERY */

static	Arg	vport_args[] = {
#ifdef	BUTTONS
	{XtNborderWidth, (XtArgVal) 0},
	{XtNtop,	(XtArgVal) XtChainTop},
	{XtNbottom,	(XtArgVal) XtChainBottom},
	{XtNleft,	(XtArgVal) XtChainLeft},
	{XtNright,	(XtArgVal) XtChainRight},
#endif
	{XtNallowHoriz,	(XtArgVal) True},
	{XtNallowVert,	(XtArgVal) True},
};

static	Arg	draw_args[] = {
	{XtNwidth,	(XtArgVal) 0},
	{XtNheight,	(XtArgVal) 0},
#ifdef	GREY
	{XtNbackground,	(XtArgVal) 0},
#endif
	{XtNx,		(XtArgVal) 0},
	{XtNy,		(XtArgVal) 0},
	{XtNlabel,	(XtArgVal) ""},
};

#ifdef	BUTTONS
static	Arg	form_args[] = {
	{XtNdefaultDistance, (XtArgVal) 0},
};
#endif

#else	/* !TOOLKIT */

static	char	*display;
static	char	*geometry;
static	char	*margins;
static	char	*offsets;
static	Boolean	hush;
static	Boolean	iconic	= False;

#define	ADDR(x)	(caddr_t) &resource.x

static	struct option {
	_Xconst	char	*name;
	_Xconst	char	*resource;
	enum {FalseArg, TrueArg, StickyArg, SepArg}
			argclass;
	enum {BooleanArg, Bool3Arg, StringArg, NumberArg, FloatArg}
			argtype;
	int		classcount;
	_Xconst	char	*usagestr;
	caddr_t		address;
}	options[] = {
{"+",		NULL,		StickyArg, StringArg, 1,
  NULL,		(caddr_t) &curr_page},
{"-s",		"shrinkFactor", SepArg, NumberArg, 1,
  "shrink",	(caddr_t)&shrink_factor},
#ifndef VMS
{"-S",		NULL,		SepArg, NumberArg, 2,
  "density",	ADDR(_density)},
{"-density",	"densityPercent", SepArg, NumberArg, 1,
  silent,	ADDR(_density)},
#else
{"-density",	"densityPercent", SepArg, NumberArg, 1,
  "density",	ADDR(_density)},
#endif
#ifdef	GREY
{"-nogrey",	NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(_use_grey)},
{"+nogrey",	"grey",		TrueArg, BooleanArg, 1,
  NULL,		ADDR(_use_grey)},
{"-gamma",	"gamma",	SepArg,	FloatArg, 1,
  "g",		ADDR(_gamma)},
#endif
{"-p",		"pixelsPerInch", SepArg, NumberArg, 1,
  "pixels",	ADDR(_pixels_per_inch)},
{"-margins",	"Margin",	SepArg,	StringArg, 3,
  "dimen",	(caddr_t) &margins},
{"-sidemargin",	"sideMargin",	SepArg,	StringArg, 1,
  "dimen",	ADDR(sidemargin)},
{"-topmargin",	"topMargin",	SepArg,	StringArg, 1,
  "dimen",	ADDR(topmargin)},
{"-offsets",	"Offset",	SepArg,	StringArg, 3,
  "dimen",	(caddr_t) &offsets},
{"-xoffset",	"xOffset",	SepArg,	StringArg, 1,
  "dimen",	ADDR(xoffset)},
{"-yoffset",	"yOffset",	SepArg,	StringArg, 1,
  "dimen",	ADDR(yoffset)},
{"-paper",	"paper",	SepArg,	StringArg, 1,
  "papertype",	ADDR(paper)},
{"-altfont",	"altFont",	SepArg,	StringArg, 1,
  "font",	ADDR(_alt_font)},
#ifdef	MAKEPK
{"-nomakepk",	"makePk",	FalseArg, BooleanArg, 2,
  NULL,		ADDR(makepk)},
{"+nomakepk",	"makePk",	TrueArg, BooleanArg, 1,
  NULL,		ADDR(makepk)},
#endif
{"-mfmode",	"mfMode",	SepArg,	StringArg, 1,
  "mode-def",	ADDR(mfmode)},
{"-l",		NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(_list_fonts)},
{"+l",		"listFonts",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(_list_fonts)},
{"-rv",		NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(reverse)},
{"+rv",		"reverseVideo",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(reverse)},
{"-mgs",	NULL,		SepArg, StringArg, 2,
  subst,	ADDR(mg_arg[0])},
{"-mgs1",	"magnifierSize1",SepArg, StringArg, 1,
  silent,	ADDR(mg_arg[0])},
{"-mgs2",	"magnifierSize2",SepArg, StringArg, 1,
  silent,	ADDR(mg_arg[1])},
{"-mgs3",	"magnifierSize3",SepArg, StringArg, 1,
  silent,	ADDR(mg_arg[2])},
{"-mgs4",	"magnifierSize4",SepArg, StringArg, 1,
  silent,	ADDR(mg_arg[3])},
{"-mgs5",	"magnifierSize5",SepArg, StringArg, 1,
  silent,	ADDR(mg_arg[4])},
{"-hush",	NULL,		TrueArg, BooleanArg, 6,
  NULL,		(caddr_t) &hush},
{"+hush",	"Hush",		FalseArg, BooleanArg, 5,
  NULL,		(caddr_t) &hush},
{"-hushspecials", NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(_hush_spec)},
{"+hushspecials", "hushSpecials", FalseArg, BooleanArg, 1,
  NULL,		ADDR(_hush_spec)},
{"-hushchars",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(_hush_chars)},
{"+hushchars",	"hushLostChars", FalseArg, BooleanArg, 1,
  NULL,		ADDR(_hush_chars)},
{"-hushchecksums", NULL,	TrueArg, BooleanArg, 2,
  NULL,		ADDR(_hush_chk)},
{"+hushchecksums","hushChecksums", FalseArg, BooleanArg, 1,
  NULL,		ADDR(_hush_chk)},
{"-safer",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(safer)},
{"+safer",	"safer",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(safer)},
{"-borderwidth", "borderWidth",	SepArg,	NumberArg, 1,
  silent,	(caddr_t) &bwidth},
{"-fg",		NULL,		SepArg,	StringArg, 2,
  "color",	ADDR(fore_color)},
{"-foreground",	"foreground",	SepArg,	StringArg, 1,
  silent,	ADDR(fore_color)},
{"-bg",		NULL,		SepArg,	StringArg, 2,
  "color",	ADDR(back_color)},
{"-background",	"background",	SepArg,	StringArg, 1,
  silent,	ADDR(back_color)},
{"-hl",		"highlight",	SepArg,	StringArg, 1,
  "color",	ADDR(high_color)},
{"-bd",		NULL,		SepArg,	StringArg, 2,
  "color",	ADDR(brdr_color)},
{"-bordercolor","borderColor",	SepArg,	StringArg, 1,
  silent,	ADDR(brdr_color)},
{"-cr",		"cursorColor",	SepArg,	StringArg, 1,
  "color",	ADDR(curs_color)},
{"-bw",		NULL,		SepArg,	NumberArg, 2,
  "width",	(caddr_t) &bwidth},
#ifndef VMS
{"-display",	NULL,		SepArg,	StringArg, 1,
  "host:display", (caddr_t) &display},
#else
{"-display",	NULL,		SepArg,	StringArg, 1,
  "host::display", (caddr_t) &display},
#endif
{"-geometry",	"geometry",	SepArg,	StringArg, 1,
  "geometry",	(caddr_t) &geometry},
{"-icongeometry","iconGeometry",StickyArg, StringArg, 1,
  "geometry",	ADDR(icon_geometry)},
{"-iconic",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		(caddr_t) &iconic},
{"+iconic",	"iconic",	FalseArg, BooleanArg, 1,
  NULL,		(caddr_t) &iconic},
{"-keep",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(keep_flag)},
{"+keep",	"keepPosition",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(keep_flag)},
{"-copy",	NULL,		TrueArg, Bool3Arg, 2,
  NULL,		ADDR(copy)},
{"+copy",	"copy",		FalseArg, Bool3Arg, 1,
  NULL,		ADDR(copy)},
{"-thorough",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(thorough)},
{"+thorough",	"thorough",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(thorough)},
#if	PS
{"-nopostscript", NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(_postscript)},
{"+nopostscript", "postscript",	TrueArg, BooleanArg, 1,
  NULL,		ADDR(_postscript)},
{"-noscan", NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(prescan)},
{"+noscan",	"prescan",	TrueArg, BooleanArg, 1,
  NULL,		ADDR(prescan)},
{"-allowshell",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(allow_shell)},
{"+allowshell",	"allowShell",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(allow_shell)},
#ifdef	PS_DPS
{"-nodps",	NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(useDPS)},
{"+nodps",	"dps",		TrueArg, BooleanArg, 1,
  NULL,		ADDR(useDPS)},
#endif
#ifdef	PS_NEWS
{"-nonews",	NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(useNeWS)},
{"+nonews",	"news",		TrueArg, BooleanArg, 1,
  NULL,		ADDR(useNeWS)},
#endif
#ifdef	PS_GS
{"-noghostscript",NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(useGS)},
{"+noghostscript","ghostscript", TrueArg, BooleanArg, 1,
  NULL,		ADDR(useGS)},
{"-interpreter", "interpreter",	SepArg,	StringArg, 1,
  "path",	ADDR(gs_path)},
{"-nogssafer",	NULL,		FalseArg, BooleanArg, 2,
  NULL,		ADDR(gs_safer)},
{"+nogssafer",	"gsSafer",	TrueArg, BooleanArg, 1,
  NULL,		ADDR(gs_safer)},
{"-gspalette",	"palette",	SepArg,	StringArg, 1,
  "monochrome|grayscale|color",	ADDR(gs_palette)},
#endif
#endif	/* PS */
{"-debug",	"debugLevel",	SepArg,	StringArg, 1,
  "bitmask",	ADDR(debug_arg)},
{"-version",	NULL,		TrueArg, BooleanArg, 2,
  NULL,		ADDR(version_flag)},
{"+version",	"version",	FalseArg, BooleanArg, 1,
  NULL,		ADDR(version_flag)},
};

#endif	/* !TOOLKIT */

static	NORETURN void
usage() {
#ifdef	TOOLKIT
	XrmOptionDescRec *opt;
	_Xconst	char	**usageptr = usagestr;
#else
	struct option	*opt;
#endif
	_Xconst char	**sv	= subst_val;
	_Xconst char	*str1;
	_Xconst char	*str2;
	int		col	= 23;
	int		n;

	Fputs("Usage: xdvi [+[<page>]]", stderr);
	for (opt = options; opt < options + XtNumber(options); ++opt) {
#ifdef	TOOLKIT
	    str1 = opt->option;
	    if (*str1 != '-') continue;
	    str2 = NULL;
	    if (opt->argKind != XrmoptionNoArg) {
		str2 = *usageptr++;
		if (str2 == silent) continue;
		if (str2 == subst) {
		    str1 = *sv++;
		    str2 = NULL;
		}
	    }
	    for (;;) {
		n = strlen(str1) + 3;
		if (str2 != NULL) n += strlen(str2) + 3;
		if (col + n < 80) Putc(' ', stderr);
		else {
		    Fputs("\n\t", stderr);
		    col = 8 - 1;
		}
		if (str2 == NULL)
		    Fprintf(stderr, "[%s]", str1);
		else
		    Fprintf(stderr, "[%s <%s>]", str1, str2);
		col += n;
		if (**usageptr != '^'
			 || strcmp(*usageptr + 1, opt->option) != 0) break;
		++usageptr;
		str1 = *usageptr++;
		str2 = NULL;
	    }
#else 	/* !TOOLKIT */
	    str1 = opt->name;
	    str2 = opt->usagestr;
	    if (*str1 != '-' || str2 == silent) continue;
	    if (str2 == subst) {
		str1 = *sv++;
		str2 = NULL;
	    }
	    n = strlen(str1) + 3;
	    if (str2 != NULL) n += strlen(str2) + 3;
	    if (col + n < 80) Putc(' ', stderr);
	    else {
		Fputs("\n\t", stderr);
		col = 8 - 1;
	    }
	    if (str2 == NULL)
		Fprintf(stderr, "[%s]", str1);
	    else
		Fprintf(stderr, "[%s <%s>]", str1, str2);
	    col += n;
#endif
	}
	if (col + 9 < 80) Putc(' ', stderr);
	else Fputs("\n\t", stderr);
	Fputs("dvi_file\n", stderr);
	exit(1);
}

static	int
atopix(arg)
	_Xconst	char	*arg;
{
	int		len	= strlen(arg);
	_Xconst char	*arg_end = arg;
	char		tmp[11];

	while ((*arg_end >= '0' && *arg_end <= '9') || *arg_end == '.')
	    if (arg_end >= arg + XtNumber(tmp) - 1) return 0;
	    else ++arg_end;
	bcopy(arg, tmp, arg_end - arg);
	tmp[arg_end - arg] = '\0';

	return (len > 2 && arg[len - 2] == 'c' && arg[len - 1] == 'm' ?
		1.0 / 2.54 : 1.0) * atof(tmp) * pixels_per_inch + 0.5;
}

/**
 **	Main programs start here.
 **/

#ifdef	TOOLKIT

static	Arg	temp_args1[] = {
	{XtNiconX,	(XtArgVal) 0},
	{XtNiconY,	(XtArgVal) 0},
};

static	Arg	temp_args2 = {XtNborderWidth,	(XtArgVal) &bwidth};

static	Pixmap	icon_pm;

static	Arg	temp_args3[] = {
	{XtNiconPixmap,	(XtArgVal) &icon_pm},
};

static	Arg	temp_args4[] = {
	{XtNtitle,	(XtArgVal) 0},
	{XtNiconName,	(XtArgVal) 0},
	{XtNinput,	(XtArgVal) True},
};

static	Arg	set_wh_args[] = {
	{XtNwidth,	(XtArgVal) 0},
	{XtNheight,	(XtArgVal) 0},
};

/*
 *	Convert string to yes/no/maybe.  Adapted from the X toolkit.
 */

/*ARGSUSED*/
Boolean XtCvtStringToBool3(dpy, args, num_args, fromVal, toVal, closure_ret)
	Display		*dpy;
	XrmValuePtr	args;
	Cardinal	*num_args;
	XrmValuePtr	fromVal;
	XrmValuePtr	toVal;
	XtPointer	*closure_ret;
{
	String		str	= (String)fromVal->addr;
	static Bool3	value;

	if (   memicmp(str, "true", 5) == 0
	    || memicmp(str, "yes", 4) == 0
	    || memicmp(str, "on", 3) == 0
	    || memicmp(str, "1", 2) == 0)	value = True;

	else if (memicmp(str, "false", 6) == 0
	    ||   memicmp(str, "no", 3) == 0
	    ||   memicmp(str, "off", 4) == 0
	    ||   memicmp(str, "0", 2) == 0)	value = False;

	else if (memicmp(str, "maybe", 6) == 0)	value = Maybe;

	else {
	    XtDisplayStringConversionWarning(dpy, str, XtRBoolean);
	    return False;
	}

	if (toVal->addr != NULL) {
	    if (toVal->size < sizeof(Bool3)) {
		toVal->size = sizeof(Bool3);
		return False;
	    }
	    *(Bool3 *)(toVal->addr) = value;
	}
	else toVal->addr = (XPointer) &value;

	toVal->size = sizeof(Bool3);
	return True;
}

#else	/* !TOOLKIT */

struct _resource	resource = {
	/* density		*/	40,
#ifdef	GREY
	/* gamma		*/	1.0,
#endif
	/* pixels_per_inch	*/	BDPI,
	/* sidemargin		*/	NULL,
	/* topmargin		*/	NULL,
	/* xoffset		*/	NULL,
	/* yoffset		*/	NULL,
	/* paper		*/	DEFAULT_PAPER,
	/* alt_font		*/	ALTFONT,
#ifdef	MAKEPK
	/* makepk		*/	True,
#endif
	/* mfmode		*/	NULL,
	/* list_fonts		*/	False,
	/* reverse		*/	False,
	/* hush_spec		*/	False,
	/* hush_chars		*/	False,
	/* hush_chk		*/	False,
	/* safer		*/	False,
	/* fore_color		*/	NULL,
	/* back_color		*/	NULL,
	/* brdr_color		*/	NULL,
	/* high_color		*/	NULL,
	/* curs_color		*/	NULL,
	/* fore_Pixel		*/	(Pixel) 0,
	/* back_Pixel		*/	(Pixel) 0,
	/* icon_geometry	*/	NULL,
	/* keep_flag		*/	False,
	/* copy			*/	Maybe,
	/* thorough		*/	False,
#if	PS
	/* postscript		*/	True,
	/* prescan		*/	True,
	/* allow_shell		*/	Maybe,
#ifdef	PS_DPS
	/* useDPS		*/	True,
#endif
#ifdef	PS_NEWS
	/* useNeWS		*/	True,
#endif
#ifdef	PS_GS
	/* useGS		*/	True,
	/* gs_path		*/	GS_PATH,
	/* gs_safer		*/	True,
	/* gs_palette		*/	"Color",
#endif
#endif	/* PS */
	/* debug_arg		*/	NULL,
	/* version_flag		*/	False,
	/* mg_arg		*/	{NULL, NULL, NULL, NULL, NULL},
#ifdef	GREY
	/* use_grey		*/	True,
#endif
};

static	Pixel
string_to_pixel(strp)		/* adapted from the toolkit */
	char	**strp;
{
	char	*str = *strp;
	Status	status;
	XColor	color, junk;

	if (*str == '#') {	/* an rgb definition */
	    status = XParseColor(DISP, DefaultColormapOfScreen(SCRN),
		str, &color);
	    if (status != 0)
		status = XAllocColor(DISP, DefaultColormapOfScreen(SCRN),
		    &color);
	}
	else	/* a name */
	    status = XAllocNamedColor(DISP, DefaultColormapOfScreen(SCRN),
		str, &color, &junk);
	if (status == 0) {
	    Fprintf(stderr, "Cannot allocate colormap entry for \"%s\"\n", str);
	    *strp = NULL;
	    return (Pixel) 0;
	}
	return color.pixel;
}

/*
 *	Process the option table.  This is not guaranteed for all possible
 *	option tables, but at least it works for this one.
 */

static	void
parse_options(argc, argv)
	int argc;
	char **argv;
{
	char	**arg;
	char	**argvend = argv + argc;
	char	*optstring;
	caddr_t	addr;
	struct option *opt, *lastopt, *candidate;
	int	len1, len2, matchlen;

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
		    if (matchlen < len1) candidate = opt;
		    else if (candidate && candidate->argclass != StickyArg)
			candidate = NULL;
		    matchlen = len1;
		}
	    }
	    if (candidate == NULL) {
		if (**arg == '-' || dvi_name) usage();
		else {
		    dvi_name = *arg;
		    continue;
		}
	    }
		/* flag it for subsequent processing */
	    candidate->resource = (char *) candidate;
		/* store the value */
	    addr = candidate->address;
	    switch (candidate->argclass) {
		case FalseArg:	*((Boolean *) addr) = False; continue;
		case TrueArg:	*((Boolean *) addr) = True; continue;
		case StickyArg:	optstring = *arg + strlen(candidate->name);
		    break;
		case SepArg:
		    ++arg;
		    if (arg >= argvend) usage();
		    optstring = *arg;
		    break;
	    }
	    switch (candidate->argtype) {
		case StringArg:	*((char **) addr) = optstring; break;
		case NumberArg:	*((int *) addr) = atoi(optstring); break;
		case FloatArg:  *((float *) addr) = atof(optstring); break;
		default:  ;
	    }
	}
	/*
	 * Step 2.  Propagate classes for command line arguments.  Backwards.
	 */
	for (opt = options + XtNumber(options) - 1; opt >= options; --opt)
	    if (opt->resource == (char *) opt) {
		addr = opt->address;
		lastopt = opt + opt->classcount;
		for (candidate = opt; candidate < lastopt; ++candidate) {
		    if (candidate->resource != NULL) {
			switch (opt->argtype) {
			    case BooleanArg:
			    case Bool3Arg:	/* same type as Boolean */
				*((Boolean *) candidate->address) =
				    *((Boolean *) addr);
				break;
			    case StringArg:
				*((char **) candidate->address) =
				    *((char **) addr);
				break;
			    case NumberArg:
				*((int *) candidate->address) = *((int *) addr);
				break;
			    case FloatArg:
				*((float *) candidate->address) =
				    *((float *) addr);
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
		    (optstring = XGetDefault(DISP, "XDvi", opt->resource))))
		{
		    lastopt = opt + opt->classcount;
		    for (candidate = opt; candidate < lastopt; ++candidate)
			if (candidate->resource != NULL) switch (opt->argtype) {
			    case Bool3Arg:
				if (       memicmp(optstring, "maybe", 6) == 0)
				{
				    * (Bool3 *) candidate->address = Maybe;
				    break;
				}
				/* otherwise, fall through; the underlying */
				/* types of Bool3 and Boolean are the same. */
			    case BooleanArg:
				* (Boolean *) candidate->address =
				    (  memicmp(optstring, "true", 5) == 0
				    || memicmp(optstring, "yes", 4) == 0
				    || memicmp(optstring, "on", 3) == 0
				    || memicmp(optstring, "1", 2) == 0);
				break;
			    case StringArg:
				* (char **) candidate->address = optstring;
				break;
			    case NumberArg:
				* (int *) candidate->address = atoi(optstring);
				break;
			    case FloatArg:
				* (float *) candidate->address =
				    atof(optstring);
			}
		}
}

#endif	/* TOOLKIT */

static	_Xconst	char	*paper_types[] = {
	"us",		"8.5x11",
	"usr",		"11x8.5",
	"legal",	"8.5x14",
	"foolscap",	"13.5x17.0",	/* ??? */

	/* ISO `A' formats, Portrait */
	"a1",		"59.4x84.0cm",
	"a2",		"42.0x59.4cm",
	"a3",		"29.7x42.0cm",
	"a4",		"21.0x29.7cm",
	"a5",		"14.85x21.0cm",
	"a6",		"10.5x14.85cm",
	"a7",		"7.42x10.5cm",

	/* ISO `A' formats, Landscape */
	"a1r",		"84.0x59.4cm",
	"a2r",		"59.4x42.0cm",
	"a3r",		"42.0x29.7cm",
	"a4r",		"29.7x21.0cm",
	"a5r",		"21.0x14.85cm",
	"a6r",		"14.85x10.5cm",
	"a7r",		"10.5x7.42cm",

	/* ISO `B' formats, Portrait */
	"b1",		"70.6x100.0cm",
	"b2",		"50.0x70.6cm",
	"b3",		"35.3x50.0cm",
	"b4",		"25.0x35.3cm",
	"b5",		"17.6x25.0cm",
	"b6",		"13.5x17.6cm",
	"b7",		"8.8x13.5cm",

	/* ISO `B' formats, Landscape */
	"b1r",		"100.0x70.6cm",
	"b2r",		"70.6x50.0cm",
	"b3r",		"50.0x35.3cm",
	"b4r",		"35.3x25.0cm",
	"b5r",		"25.0x17.6cm",
	"b6r",		"17.6x13.5cm",
	"b7r",		"13.5x8.8cm",

	/* ISO `C' formats, Portrait */
	"c1",		"64.8x91.6cm",
	"c2",		"45.8x64.8cm",
	"c3",		"32.4x45.8cm",
	"c4",		"22.9x32.4cm",
	"c5",		"16.2x22.9cm",
	"c6",		"11.46x16.2cm",
	"c7",		"8.1x11.46cm",

	/* ISO `C' formats, Landscape */
	"c1r",		"91.6x64.8cm",
	"c2r",		"64.8x45.8cm",
	"c3r",		"45.8x32.4cm",
	"c4r",		"32.4x22.9cm",
	"c5r",		"22.9x16.2cm",
	"c6r",		"16.2x11.46cm",
	"c7r",		"11.46x8.1cm",
};

static	Boolean
set_paper_type() {
	_Xconst	char	*arg, *arg1;
	char	temp[21];
	_Xconst	char	**p;
	char	*q;

	if (strlen(resource.paper) > sizeof(temp) - 1) return False;
	arg = resource.paper;
	q = temp;
	for (;;) {	/* convert to lower case */
	    char c = *arg++;
	    if (c >= 'A' && c <= 'Z') c ^= ('a' ^ 'A');
	    *q++ = c;
	    if (c == '\0') break;
	}
	arg = temp;
	/* perform substitutions */
	for (p = paper_types; p < paper_types + XtNumber(paper_types); p += 2)
	    if (strcmp(temp, *p) == 0) {
		arg = p[1];
		break;
	    }
	arg1 = index(arg, 'x');
	if (arg1 == NULL) return False;
	unshrunk_paper_w = atopix(arg);
	unshrunk_paper_h = atopix(arg1 + 1);
	return (unshrunk_paper_w != 0 && unshrunk_paper_h != 0);
}

/*
 *	main program
 */

int
main(argc, argv)
	int argc;
	char **argv;
{

#ifndef	TOOLKIT
	XSizeHints	size_hints;
	XWMHints	wmhints;
#endif	/* TOOLKIT */
	Dimension	screen_w, screen_h;
	char		*title_name;
	char		*icon_name;
	unsigned	baselen;
	int	i;

#ifndef	VMS
	prog = rindex(*argv, '/');
#else
	prog = rindex(*argv, ']');
#endif
	if (prog != NULL) ++prog; else prog = *argv;

#ifdef	VMS
	if (index(prog, '.') != NULL) *index(prog, '.') = '\0';
#endif

	/* This has to be a special case, for now. */

	if (argc == 2 && (strcmp(argv[1], "-version") == 0
		|| strcmp(argv[1], "--version") == 0)) {
	    Puts((_Xconst char *) &version);
	    exit(0);
	}

#ifdef	TOOLKIT
	top_level = XtInitialize(prog, "XDvi", options, XtNumber(options),
		&argc, argv);
	while (--argc > 0) {
	    if (*(*++argv) == '+')
		if (curr_page != NULL) usage();
		else curr_page = *argv + 1;
	    else if (dvi_name != NULL) usage();
		else dvi_name = *argv;
	}

	XtSetTypeConverter(XtRString, XtRBool3, XtCvtStringToBool3,
	    NULL, 0, XtCacheNone, NULL);

	XtGetApplicationResources(top_level, (XtPointer) &resource,
	    application_resources, XtNumber(application_resources),
	    (ArgList) NULL, 0);
	DISP = XtDisplay(top_level);
	SCRN = XtScreen(top_level);
	shrink_factor = resource.shrinkfactor;

#else	/* !TOOLKIT */

	parse_options(argc, argv);
	if (resource.fore_color)
	    fore_Pixel = string_to_pixel(&resource.fore_color);
	if (resource.back_color)
	    back_Pixel = string_to_pixel(&resource.back_color);
	if (resource.brdr_color)
	    brdr_Pixel = string_to_pixel(&resource.brdr_color);
	if (resource.high_color)
	    hl_Pixel = string_to_pixel(&resource.high_color);
	if (resource.curs_color)
	    cr_Pixel = string_to_pixel(&resource.curs_color);

#endif	/* TOOLKIT */

	if (shrink_factor <= 0 || density <= 0 || pixels_per_inch <= 0 ||
		dvi_name == NULL) usage();
	if (shrink_factor != 1) bak_shrink = shrink_factor;
	mane.shrinkfactor = shrink_factor;
	if (resource.debug_arg != NULL)
	    debug = isdigit(*resource.debug_arg) ? atoi(resource.debug_arg)
		: DBG_ALL;
	if (resource.sidemargin) home_x = atopix(resource.sidemargin);
	if (resource.topmargin) home_y = atopix(resource.topmargin);
	offset_x = resource.xoffset ? atopix(resource.xoffset)
	    : pixels_per_inch;
	offset_y = resource.yoffset ? atopix(resource.yoffset)
	    : pixels_per_inch;
	if (!set_paper_type())
	    oops("Don't recognize paper type %s", resource.paper);
	for (i = 0; i < 5; ++i)
	    if (resource.mg_arg[i] != NULL) {
		char	*s;

		mg_size[i].w = mg_size[i].h = atoi(resource.mg_arg[i]);
		s = index(resource.mg_arg[i], 'x');
		if (s != NULL) {
		    mg_size[i].h = atoi(s + 1);
		    if (mg_size[i].h <= 0) mg_size[i].w = 0;
		}
	    }
#if	PS
	if (resource.safer) {
	    resource.allow_shell = False;
#ifdef	PS_GS
	    resource.gs_safer = True;
#endif
	}
#endif	/* PS */
#ifdef	PS_GS
	{
	    _Xconst char *CGMcgm = "CGMcgm";
	    _Xconst char *cgmp;

	    cgmp = index(CGMcgm, resource.gs_palette[0]);
	    if (cgmp == NULL)
		oops("Illegal value %s for gs palette option",
		    resource.gs_palette);
	    if (cgmp >= CGMcgm + 3) resource.gs_palette[0] = *(cgmp - 3);
	}
#endif
	if (resource.version_flag) Puts((_Xconst char *) &version);

	/*
	 *	Open the dvi file and set titles
	 */

	init_font_open();
	open_dvi_file();
	if (curr_page) {
		current_page = (*curr_page ? atoi(curr_page) : total_pages) - 1;
		if (current_page < 0 || current_page >= total_pages) usage();
	}
	icon_name = rindex(dvi_name, '/');
	if (icon_name != NULL) ++icon_name; else icon_name = dvi_name;
	baselen = strlen(icon_name);
	if (baselen >= sizeof(".dvi")
		&& strcmp(icon_name + baselen - sizeof(".dvi") + 1, ".dvi")
		== 0) {		/* remove the .dvi */
	    char *p;

	    baselen -= sizeof(".dvi") - 1;
	    p = xmalloc(baselen + 1, "icon name");
	    (void) strncpy(p, icon_name, (int) baselen);
	    p[baselen] = '\0';
	    icon_name = p;
	}
	title_name = xmalloc(baselen + sizeof("Xdvi:  "), "title name");
	Strcpy(title_name, "Xdvi:  ");
	Strcat(title_name, icon_name);

	/*
	 *	Colors
	 */

	if (resource.reverse) {
	    if (!resource.fore_color) fore_Pixel = WhitePixelOfScreen(SCRN);
	    if (!resource.back_color) back_Pixel = BlackPixelOfScreen(SCRN);
	    /* Set them nonzero */
	    resource.fore_color = resource.back_color = (char *) &version;
	} else {
	    if (!resource.fore_color) fore_Pixel = BlackPixelOfScreen(SCRN);
	    if (!resource.back_color) back_Pixel = WhitePixelOfScreen(SCRN);
	}

#ifdef	GREY
	if (DefaultDepthOfScreen(SCRN) == 1)
	    use_grey = False;
#endif

	copy = resource.copy;
	if (copy == Maybe)
#ifdef	GREY
	    copy = (!resource.thorough && !use_grey
		&& DefaultDepthOfScreen(SCRN) > 1);
#else
	    copy = (!resource.thorough
		&& DefaultDepthOfScreen(SCRN) > 1);
#endif

#ifdef	GREY
	if (resource._gamma == 0.0) resource._gamma = 1.0;
	if (use_grey)
	    init_pix(True);
	else
#endif
	{
	    XGCValues	values;
	    Pixel	set_bits = (Pixel) (fore_Pixel & ~back_Pixel);
	    Pixel	clr_bits = (Pixel) (back_Pixel & ~fore_Pixel);

#define	MakeGC(fcn, fg, bg)	(values.function = fcn, values.foreground=fg,\
		values.background=bg,\
		XCreateGC(DISP, RootWindowOfScreen(SCRN),\
			GCFunction|GCForeground|GCBackground, &values))

	    if (copy || (set_bits && clr_bits))
		ruleGC = MakeGC(GXcopy, fore_Pixel, back_Pixel);
	    if (copy) foreGC = ruleGC;
	    else if (!resource.thorough && ruleGC) {
		foreGC = ruleGC;
		Puts("Note:  overstrike characters may be incorrect.");
	    }
	    else {
		if (set_bits) foreGC = MakeGC(GXor, set_bits, 0);
		if (clr_bits || !set_bits)
		    *(foreGC ? &foreGC2 : &foreGC) =
			MakeGC(GXandInverted, clr_bits, 0);
		if (!ruleGC) ruleGC = foreGC;
	    }
	}

	{
	    XGCValues	values;

	    highGC = ruleGC;
	    if (resource.high_color)
		highGC = MakeGC(GXcopy, hl_Pixel, back_Pixel);
	}

	if (!resource.brdr_color) brdr_Pixel = fore_Pixel;
	if (!resource.high_color) hl_Pixel = fore_Pixel;

#ifndef	VMS
	ready_cursor = XCreateFontCursor(DISP, XC_cross);
	redraw_cursor = XCreateFontCursor(DISP, XC_watch);
#else
	DECWCursorFont = XLoadFont(DISP, "DECW$CURSOR");
	XSetFont(DISP, foreGC, DECWCursorFont);
	redraw_cursor = XCreateGlyphCursor(DISP, DECWCursorFont, DECWCursorFont,
		decw$c_wait_cursor, decw$c_wait_cursor + 1,
		&resource.fore_color, &resource.back_color);
	MagnifyPixmap = XCreateBitmapFromData (DISP, RootWindowOfScreen(SCRN),
		mag_glass_bits, mag_glass_width, mag_glass_height);
	ready_cursor = XCreatePixmapCursor(DISP, MagnifyPixmap, MagnifyPixmap,
		&resource.back_color, &resource.fore_color,
		mag_glass_x_hot, mag_glass_y_hot);
#endif	/* VMS */

	if (!resource.curs_color) cr_Pixel = hl_Pixel;
	{
	    XColor bg_Color, cr_Color;

	    bg_Color.pixel = back_Pixel;
	    XQueryColor(DISP, DefaultColormapOfScreen(SCRN), &bg_Color);
	    cr_Color.pixel = cr_Pixel;
	    XQueryColor(DISP, DefaultColormapOfScreen(SCRN), &cr_Color);
	    XRecolorCursor(DISP, ready_cursor, &cr_Color, &bg_Color);
	    XRecolorCursor(DISP, redraw_cursor, &cr_Color, &bg_Color);
	}

#ifdef	TOOLKIT

	/*
	 *	Windows (toolkit)
	 */

		/* The following code is lifted from Xterm */
	if (resource.icon_geometry != NULL) {
	    int scr, junk;

	    for(scr = 0;	/* yyuucchh */
		SCRN != ScreenOfDisplay(DISP, scr);
		scr++);

	    (void) XGeometry(DISP, scr, resource.icon_geometry, "",
			0, 0, 0, 0, 0,
			(int *) &temp_args1[0].value,
			(int *) &temp_args1[1].value, &junk, &junk);
	    XtSetValues(top_level, temp_args1, XtNumber(temp_args1));
	}
		/* Set icon pixmap */
	XtGetValues(top_level, temp_args3, XtNumber(temp_args3));
	if (icon_pm == (Pixmap) 0) {
	    temp_args3[0].value = (XtArgVal) (XCreateBitmapFromData(DISP,
		RootWindowOfScreen(SCRN), (_Xconst char *) xdvi_bits,
		xdvi_width, xdvi_height));
	    XtSetValues(top_level, temp_args3, XtNumber(temp_args3));
	}
	temp_args4[0].value = (XtArgVal) title_name;
	temp_args4[1].value = (XtArgVal) icon_name;
	XtSetValues(top_level, temp_args4, XtNumber(temp_args4));

#ifdef	BUTTONS
	form_widget = XtCreateManagedWidget("form", formWidgetClass,
		top_level, form_args, XtNumber(form_args));

#else	/* !BUTTONS */
#define	form_widget	top_level	/* for calls to XtAddEventHandler */
#endif	/* BUTTONS */
	vport_widget = XtCreateManagedWidget("vport", viewportWidgetClass,
		form_widget, vport_args, XtNumber(vport_args));
	clip_widget = XtNameToWidget(vport_widget, "clip");
	draw_args[0].value = (XtArgVal) page_w;
	draw_args[1].value = (XtArgVal) page_h;
#ifdef	GREY
	draw_args[2].value = (XtArgVal) back_Pixel;
#endif
	draw_widget = XtCreateManagedWidget("drawing", drawWidgetClass,
		vport_widget, draw_args, XtNumber(draw_args));
	{	/* set default window size */
#ifdef	BUTTONS
	    int xtra_wid = resource.expert ? 0 : XTRA_WID;
#else
#define	xtra_wid	0
#endif
	    XtWidgetGeometry constraints;
	    XtWidgetGeometry reply;

	    XtGetValues(top_level, &temp_args2, 1);	/* get border width */
	    screen_w = WidthOfScreen(SCRN) - 2 * bwidth - xtra_wid;
	    screen_h = HeightOfScreen(SCRN) - 2 * bwidth;
	    constraints.request_mode = reply.request_mode = 0;
	    constraints.width = page_w;
	    if (page_w > screen_w) {
		constraints.request_mode = CWWidth;
		constraints.width = screen_w;
	    }
	    constraints.height = page_h;
	    if (page_h > screen_h) {
		constraints.request_mode = CWHeight;
		constraints.height = screen_h;
	    }
	    if (constraints.request_mode != 0
		    && constraints.request_mode != (CWWidth | CWHeight))
		(void) XtQueryGeometry(vport_widget, &constraints, &reply);
	    if (!(reply.request_mode & CWWidth))
		reply.width = constraints.width;
	    set_wh_args[0].value = (XtArgVal) ((reply.width < screen_w
				? reply.width : screen_w) + xtra_wid);
	    if (!(reply.request_mode & CWHeight))
		reply.height = constraints.height;
	    set_wh_args[1].value = (XtArgVal) (reply.height < screen_h
					? reply.height : screen_h);
	    XtSetValues(top_level, set_wh_args, XtNumber(set_wh_args));
#ifdef	BUTTONS
	    set_wh_args[0].value -= xtra_wid;
	    XtSetValues(vport_widget, set_wh_args, XtNumber(set_wh_args));
	    if (!resource.expert) create_buttons(set_wh_args[1].value);
#endif	/* BUTTONS */
	}
	if (resource.fore_color) {
	    static Arg fore_args = {XtNforeground, (XtArgVal) 0};

	    fore_args.value = fore_Pixel;
	    XtSetValues(draw_widget, &fore_args, 1);
	}
	if (resource.back_color) {
	    static Arg back_args = {XtNbackground, (XtArgVal) 0};

	    back_args.value = back_Pixel;
	    XtSetValues(draw_widget, &back_args, 1);
	    XtSetValues(clip_widget, &back_args, 1);
	}
	XtAddEventHandler(form_widget, KeyPressMask, False, handle_key,
		(caddr_t) NULL);
	XtAddEventHandler(vport_widget, StructureNotifyMask, False,
		handle_resize, (caddr_t) NULL);
	XtAddEventHandler(draw_widget, ExposureMask, False, handle_exp,
		(caddr_t) &mane);
	XtAddEventHandler(draw_widget, ButtonPressMask, False, handle_button,
		(caddr_t) NULL);
	XtAddEventHandler(draw_widget, ButtonMotionMask, False, handle_motion,
		(caddr_t) NULL);
	XtAddEventHandler(draw_widget, ButtonReleaseMask, False, handle_release,
		(caddr_t) NULL);
	XtRealizeWidget(top_level);

	currwin.win = mane.win = XtWindow(draw_widget);

	{
	    XWindowAttributes attrs;

	    (void) XGetWindowAttributes(DISP, mane.win, &attrs);
	    backing_store = attrs.backing_store;
	}

#else	/* !TOOLKIT */

	/*
	 *	Windows (non toolkit)
	 */

	screen_w = WidthOfScreen(SCRN) - 2*bwidth;
	screen_h = HeightOfScreen(SCRN) - 2*bwidth;
	size_hints.flags = PMinSize;
	size_hints.min_width = size_hints.min_height = 50;
	size_hints.x = size_hints.y = 0;
	if (geometry != NULL) {
	    int flag = XParseGeometry(geometry, &size_hints.x, &size_hints.y,
		&window_w, &window_h);

	    if (flag & (XValue | YValue))
		size_hints.flags |= USPosition;
	    if (flag & (WidthValue | HeightValue))
		size_hints.flags |= USSize;
	    if (flag & XNegative) size_hints.x += screen_w - window_w;
	    if (flag & YNegative) size_hints.y += screen_h - window_h;
	}
	if (!(size_hints.flags & USSize)) {
	    int x_thick = 0;
	    int y_thick = 0;
	    if (screen_w < page_w) x_thick = BAR_THICK;
	    if (screen_h < page_h + x_thick) y_thick = BAR_THICK;
	    window_w = page_w + y_thick;
	    if (window_w > screen_w) {
		x_thick = BAR_THICK;
		window_w = screen_w;
	    }
	    window_h = page_h + x_thick;
	    if (window_h > screen_h) window_h = screen_h;
	    size_hints.flags |= PSize;
	}
	size_hints.width = window_w;
	size_hints.height = window_h;
	top_level = XCreateSimpleWindow(DISP, RootWindowOfScreen(SCRN),
		size_hints.x, size_hints.y, window_w, window_h, bwidth,
		brdr_Pixel, back_Pixel);
	XSetStandardProperties(DISP, top_level, title_name, icon_name,
		(Pixmap) 0, argv, argc, &size_hints);

	wmhints.flags = InputHint | StateHint | IconPixmapHint;
	wmhints.input = True;	/* window manager must direct input */
	wmhints.initial_state = iconic ? IconicState : NormalState;
	wmhints.icon_pixmap = XCreateBitmapFromData(DISP,
	    RootWindowOfScreen(SCRN), (_Xconst char *) xdvi_bits,
	    xdvi_width, xdvi_height);
	if (resource.icon_geometry != NULL) {
	    int junk;

	    wmhints.flags |= IconPositionHint;
	    (void) XGeometry(DISP, DefaultScreen(DISP), resource.icon_geometry,
		"", 0, 0, 0, 0, 0, &wmhints.icon_x, &wmhints.icon_y,
		&junk, &junk);
	}
	XSetWMHints(DISP, top_level, &wmhints);

	XSelectInput(DISP, top_level, KeyPressMask | StructureNotifyMask);
	XMapWindow(DISP, top_level);
	XFlush(DISP);

#endif	/* TOOLKIT */

#define	rebindkey(ks, str)	XRebindKeysym(DISP, (KeySym) ks, \
		(KeySym *) NULL, 0, (_Xconst ubyte *) str, 1)
	rebindkey(XK_Home, "^");
	rebindkey(XK_Left, "l");
	rebindkey(XK_Up, "u");
	rebindkey(XK_Right, "r");
	rebindkey(XK_Down, "d");
	rebindkey(XK_Prior, "b");
	rebindkey(XK_Next, "f");
#ifdef	XK_KP_Left
	rebindkey(XK_KP_Home, "^");
	rebindkey(XK_KP_Left, "l");
	rebindkey(XK_KP_Up, "u");
	rebindkey(XK_KP_Right, "r");
	rebindkey(XK_KP_Down, "d");
	rebindkey(XK_KP_Prior, "b");
	rebindkey(XK_KP_Next, "f");
#endif	/* XK_KP_Left */
#undef	rebindkey

	image = XCreateImage(DISP, DefaultVisualOfScreen(SCRN), 1, XYBitmap, 0,
			     (char *)NULL, 0, 0, BITS_PER_BMUNIT, 0);
	image->bitmap_unit = BITS_PER_BMUNIT;
#ifndef	MSBITFIRST
	image->bitmap_bit_order = LSBFirst;
#else
	image->bitmap_bit_order = MSBFirst;
#endif
	{
	    short endian = MSBFirst << 8 | LSBFirst;
	    image->byte_order = *((char *) &endian);
	}

	do_pages();
	return 0;	/* do_pages() returns if DBG_BATCH is specified */
}
