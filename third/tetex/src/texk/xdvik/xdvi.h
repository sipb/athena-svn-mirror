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

NOTE:
	xdvi is based on prior work, as noted in the modification history
	in xdvi.c.

\*========================================================================*/

/*
 *	Written by Eric C. Cooper, CMU
 */

#ifndef	XDVI_H
#define	XDVI_H

/********************************
 *	The C environment	*
 *******************************/

/* The stuff from the path searching library.  */
/* NOTE SU: This makes the following #ifdefs partly redundant,
   but I didn't dare to touch too much of them; and we *need*
   the kpathsea settings for header compatiblility. */
#include "kpathsea/c-auto.h"
#include "kpathsea/config.h"
#include "kpathsea/hash.h" /* access to the hash functions */

#ifdef __hpux
/* On HP-UX 10.10 B and 20.10, compiling with _XOPEN_SOURCE + ..._EXTENDED
 * leads to poll() not realizing that a file descriptor is writable in psgs.c.
 */
#define	_HPUX_SOURCE	1
#else
/* SU: Don't define _XOPEN_SOURCE unconditionally, as in non-k xdvi - this
   will give problems with kpathsearch e.g. on Tru64 */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE	600
#endif
#define	_XOPEN_SOURCE_EXTENDED	1
#define	__EXTENSIONS__	1	/* needed to get struct timeval on SunOS 5.5 */
#define	_SVID_SOURCE	1	/* needed to get S_IFLNK in glibc */
#define	_BSD_SOURCE	1	/* needed to get F_SETOWN in glibc-2.1.3 */
#endif

#include "c-auto.h"
#include <errno.h>		/* FIXME: Check for availability? */

/* Some O/S dependent kludges. */
#ifdef _AIX
#define _ALL_SOURCE 1
#endif

/* just a kludge, no real portability here ... */
#define DIR_SEPARATOR '/'

#if 0
// FIXME: not in xdvi #ifdef __linux		/* needed only for kernel versions 2.1.xxx -- 2.2.8 */
// FIXME: not in xdvi #define FLAKY_SIGPOLL 1
// FIXME: not in xdvi #endif
#endif

#if STDC_HEADERS
# include <stddef.h>
# include <stdlib.h>
	/* the following works around the wchar_t problem */
# include <X11/X.h>
# if HAVE_X11_XOSDEFS_H
#  include <X11/Xosdefs.h>
# endif
# ifdef X_NOT_STDC_ENV
#  undef X_NOT_STDC_ENV
#  undef X_WCHAR
#  include <X11/Xlib.h>
#  define X_NOT_STDC_ENV
# endif
#endif

/* in case stdlib.h doesn't define these ... */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

/* Avoid name clashes with kpathsea.  */
#define xfopen xdvi_xfopen

/* For wchar_t et al., that the X files might want. */
#include "kpathsea/systypes.h"
#include "kpathsea/c-memstr.h"

#include <X11/Xlib.h>	/* include Xfuncs.h, if available */
#include <X11/Xutil.h>	/* needed for XDestroyImage */
#include <X11/Xos.h>
#undef wchar_t

#if	XlibSpecificationRelease >= 5
#include <X11/Xfuncs.h>
#endif

#ifndef	NOTOOL

#define	TOOLKIT	1

#include <X11/Intrinsic.h>
#if	(defined(VMS) && (XtSpecificationRelease <= 4)) || defined(lint)
#include <X11/IntrinsicP.h>
#endif
#if MOTIF
#include <Xm/Xm.h>
#endif

#else	/* NOTOOL */

#define	XtNumber(arr)	(sizeof(arr)/sizeof(arr[0]))
#define	XtWindow(win)	(win)
typedef	unsigned long	Pixel;
typedef	char		Boolean;
typedef	unsigned int	Dimension;
#undef	TOOLKIT
#undef	MOTIF
#undef	BUTTONS
#undef	CFG2RES

#endif	/* NOTOOL */


#ifndef MAXPATHLEN
# ifdef PATH_MAX
#  define MAXPATHLEN PATH_MAX
# else
#  define MAXPATHLEN 1024
# endif
#endif


#if defined(CFG2RES) && !defined(SELFAUTO)
#define	SELFAUTO 1
#endif

#if defined(SELFAUTO) && !defined(DEFAULT_CONFIG_PATH)
#define	DEFAULT_CONFIG_PATH	"$SELFAUTODIR:$SELFAUTOPARENT"
#endif

#undef CFGFILE				/* no cheating */

#if defined(DEFAULT_CONFIG_PATH)
#define	CFGFILE	1
#endif

typedef	char		Bool3;		/* Yes/No/Maybe */

#define	True	1
#define	False	0
#define	Maybe	2

/* for unused parameters */
#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifdef	VMS
#include <string.h>
#define	index	strchr
#define	rindex	strrchr
#define	bzero(a, b)	(void) memset ((void *) (a), 0, (size_t) (b))
#define bcopy(a, b, c)  (void) memmove ((void *) (b), (void *) (a), (size_t) (c))
#endif

#include <stdio.h>
#include <setjmp.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

/* only use definitions with prototypes now */
#define	ARGS(x)	x

#ifndef	NeedWidePrototypes
#define	NeedWidePrototypes	NeedFunctionPrototypes
#endif

#ifndef	NeedVarargsPrototypes
#define	NeedVarargsPrototypes	NeedFunctionPrototypes
#endif

#include "kpathsea/c-vararg.h"

#ifndef	_XFUNCPROTOBEGIN
#define	_XFUNCPROTOBEGIN
#define	_XFUNCPROTOEND
#endif

#include "xdvi-config.h"

#ifndef	_Xconst
#if	__STDC__
#define	_Xconst	const
#else	/* STDC */
#define	_Xconst
#endif	/* STDC */
#endif	/* _Xconst */

#if __STDC__ && !defined(FUNCPROTO)
/* FUNCPROTO is a bitmask specifying ANSI conformance (see Xfuncproto.h).
   The single bits specify varargs, const availability, prototypes etc.;
   we enable everything here. */
# define FUNCPROTO (-1)
#endif

#ifndef	VOLATILE
#if	__STDC__ || (defined(__stdc__) && defined(__convex__))
#define	VOLATILE	volatile
#else
#define	VOLATILE	/* nothing */
#endif
#endif

#ifndef	NORETURN
#  ifdef	__GNUC__
#    ifndef __STRICT_ANSI__
#      define	NORETURN	volatile
#    else
#      define	NORETURN
#    endif
#  else
#    define	NORETURN	/* nothing */
#  endif
#endif

#ifndef	OPEN_MODE
/*
 * SU, 2001/01/07: xdvi defines OPEN_MODE as "r" or as "r", "ctx=stm" (for VMS),
 * but we use the definition of FOPEN_R_MODE from kpathsea/c-fopen.h instead:
 */
#define OPEN_MODE FOPEN_R_MODE
#endif	/* OPEN_MODE */

#ifndef	VMS
#define	OPEN_MODE_ARGS	_Xconst char *
#else
#define	OPEN_MODE_ARGS	_Xconst char *, _Xconst char *
#endif

#define	Printf	(void) printf
#define	Puts	(void) puts
#define	Fprintf	(void) fprintf
#define	Sprintf	(void) sprintf
#define	Fseek	(void) fseek
#define	Fread	(void) fread
#define	Fputs	(void) fputs
#define	Putc	(void) putc
#define	Putchar	(void) putchar
#define	Fclose	(void) fclose
#define	Fflush	(void) fflush
#define	Strcat	(void) strcat
#define	Strcpy	(void) strcpy

#ifndef __LINE__
#define __LINE__ 0
#endif

#ifndef __FILE__
#define __FILE__ "?"
#endif

/********************************
 *	 Types and data		*
 *******************************/

#ifndef	EXTERN
#define	EXTERN	extern
#define	INIT(x)
#endif

#define	MAXDIM		32767

typedef	unsigned char	ubyte;

#if	NeedWidePrototypes
typedef	unsigned int	wide_ubyte;
typedef	int		wide_bool;
#define	WIDENINT	(int)
#else
typedef	ubyte		wide_ubyte;
typedef	Boolean		wide_bool;
#define	WIDENINT
#endif

#if defined(MAKEPK) && !defined(MKTEXPK)
#define	MKTEXPK 1
#endif

/*
 *	pixel_conv is currently used only for converting absolute positions
 *	to pixel values; although normally it should be
 *		((int) ((x) / currwin.shrinkfactor + (1 << 15) >> 16)),
 *	the rounding is achieved instead by moving the constant 1 << 15 to
 *	PAGE_OFFSET in dvi-draw.c.
 */
#define	pixel_conv(x)		((int) ((x) / currwin.shrinkfactor >> 16))
#define	pixel_round(x)		((int) ROUNDUP(x, currwin.shrinkfactor << 16))
#define	spell_conv0(n, f)	((long) (n * f))
#define	spell_conv(n)		spell_conv0(n, dimconv)

#define	BMUNIT			unsigned BMTYPE
#define	BMBITS			(8 * BMBYTES)

#define	ADD(a, b)	((BMUNIT *) (((char *) a) + b))
#define	SUB(a, b)	((BMUNIT *) (((char *) a) - b))

extern	BMUNIT	bit_masks[BMBITS + 1];

/*
 * added SU:
 * when magnifier is active, mane.shrinkfactor is still the shrinkfactor of the
 * main window, while currwin.shrinkfactor is the shrinkfactor of the magnifier.
 */
#define MAGNIFIER_ACTIVE (mane.shrinkfactor != currwin.shrinkfactor)

struct frame {
  	/* dvi_h and dvi_v is the horizontal and vertical baseline position;
	   it is the responsability of the set_char procedure to update
	   them. */
	struct framedata {
		long dvi_h, dvi_v, w, x, y, z;
		int pxl_v;
	} data;
	struct frame *next, *prev;
};

#ifndef	TEXXET
typedef	long	(*set_char_proc) ARGS((wide_ubyte));
#else
typedef	void	(*set_char_proc) ARGS((wide_ubyte, wide_ubyte));
#endif

struct drawinf {	/* this information is saved when using virtual fonts */
	struct framedata data;
	struct font	*fontp;
	set_char_proc	set_char_p;
	unsigned int	tn_table_len;
	struct font	**tn_table;
	struct tn	*tn_head;
	ubyte		*pos, *end;
	struct font	*virtual;
#ifdef	TEXXET
	int		dir;
#endif
};

EXTERN	struct drawinf	currinf;

/* Points to drawinf record containing current dvi file location (for update by
   geom_scan).  */
EXTERN	struct drawinf	*dvi_pointer_frame	INIT(NULL);

/* entries below with the characters 'dvi' in them are actually stored in
   scaled pixel units */

#define DVI_H   currinf.data.dvi_h
#define PXL_H   pixel_conv(currinf.data.dvi_h)
#define DVI_V   currinf.data.dvi_v
#define PXL_V   currinf.data.pxl_v
#define WW      currinf.data.w
#define XX      currinf.data.x
#define YY      currinf.data.y
#define ZZ      currinf.data.z
#define ROUNDUP(x,y) (((x)+(y)-1)/(y))

EXTERN	int	current_page;
EXTERN	int	total_pages;
EXTERN	int	pageno_correct	INIT(1);
EXTERN	unsigned long	magnification;
EXTERN	double	dimconv;
EXTERN	double	tpic_conv;
EXTERN	int	n_files_left	INIT(32767);	/* for LRU closing of fonts */
EXTERN	unsigned int	page_w, page_h;

#ifndef	BDPI
#define	BDPI	600
#endif


#if	defined(GS_PATH) && !defined(PS_GS)
#define	PS_GS	1
#endif

#if	defined(PS_DPS) || defined(PS_NEWS) || defined(PS_GS)
#define	PS	1
#else
#define	PS	0
#endif

#if	PS
EXTERN	int	scanned_page;		/* last page prescanned */
EXTERN	int	scanned_page_bak;	/* actual value of the above */
EXTERN	int	scanned_page_reset;	/* number to reset the above to */
#endif

/*
 * Table of page offsets in DVI file, indexed by page number - 1.
 * Initialized in prepare_pages().
 */
EXTERN	long	*page_offset;

/*
 * Mechanism for reducing repeated warning about specials, lost characters, etc.
 */
EXTERN	Boolean	warn_spec_now;

/*
 * If we're in the middle of a PSFIG special.
 */
EXTERN	Boolean	psfig_begun	INIT(False);

/*
 * Page on which to draw box from forward source special searching.
 */
EXTERN	int	source_fwd_box_page	INIT(-1);	/* -1 means no box */


/*
 * Bitmap structure for raster ops.
 */
struct bitmap {
	unsigned short	w, h;		/* width and height in pixels */
	short		bytes_wide;	/* scan-line width in bytes */
	char		*bits;		/* pointer to the bits */
};

/*
 * Per-character information.
 * There is one of these for each character in a font (raster fonts only).
 * All fields are filled in at font definition time,
 * except for the bitmap, which is "faulted in"
 * when the character is first referenced.
 */
struct glyph {
	long addr;		/* address of bitmap in font file */
	long dvi_adv;		/* DVI units to move reference point */
	short x, y;		/* x and y offset in pixels */
	struct bitmap bitmap;	/* bitmap for character */
	short x2, y2;		/* x and y offset in pixels (shrunken bitmap) */
#ifdef	GREY
	XImage *image2;
	char *pixmap2;
	char *pixmap2_t;
#endif
	struct bitmap bitmap2;	/* shrunken bitmap for character */
};

/*
 * Per character information for virtual fonts
 */
struct macro {
	ubyte	*pos;		/* address of first byte of macro */
	ubyte	*end;		/* address of last+1 byte */
	long	dvi_adv;	/* DVI units to move reference point */
	Boolean	free_me;	/* if free(pos) should be called when */
				/* freeing space */
};

/*
 * The layout of a font information block.
 * There is one of these for every loaded font or magnification thereof.
 * Duplicates are eliminated:  this is necessary because of possible recursion
 * in virtual fonts.
 *
 * Also note the strange units.  The design size is in 1/2^20 point
 * units (also called micro-points), and the individual character widths
 * are in the TFM file in 1/2^20 ems units, i.e., relative to the design size.
 *
 * We then change the sizes to SPELL units (unshrunk pixel / 2^16).
 */

#define	NOMAGSTP (-29999)

typedef	void (*read_char_proc) ARGS((struct font *, wide_ubyte));

struct font {
  struct font *next;		/* link to next font info block */
  char *fontname;		/* name of font */
  float fsize;			/* size information (dots per inch) */
  int magstepval;		/* magstep number * two, or NOMAGSTP */
  FILE *file;			/* open font file or NULL */
  char *filename;		/* name of font file */
  long checksum;		/* checksum */
  unsigned short timestamp;	/* for LRU management of fonts */
  ubyte flags;			/* flags byte (see values below) */
#ifdef Omega
  wide_ubyte maxchar;		/* largest character code */
#else
  ubyte maxchar;		/* largest character code */
#endif
  double dimconv;		/* size conversion factor */
  set_char_proc set_char_p;	/* proc used to set char */
  /* these fields are used by (loaded) raster fonts */
  read_char_proc read_char;	/* function to read bitmap */
  struct glyph *glyph;
  /* these fields are used by (loaded) virtual fonts */
  struct font **vf_table;	/* list of fonts used by this vf */
  struct tn *vf_chain;		/* ditto, if TeXnumber >= VFTABLELEN */
  struct font *first_font;	/* first font defined */
  struct macro *macro;
  /* I suppose the above could be put into a union, but we */
  /* wouldn't save all that much space. */

  /* These were added for t1 use */
  int t1id;
  long scale;
};

#define	FONT_IN_USE	1	/* used for housekeeping */
#define	FONT_LOADED	2	/* if font file has been read */
#define	FONT_VIRTUAL	4	/* if font is virtual */

#define	TNTABLELEN	30	/* length of TeXnumber array (dvi file) */
#define	VFTABLELEN	5	/* length of TeXnumber array (virtual fonts) */

struct tn {
	struct tn *next;		/* link to next TeXnumber info block */
	unsigned int TeXnumber;			/* font number (in DVI file) */
	struct font *fontp;		/* pointer to the rest of the info */
};

EXTERN	struct font	*tn_table[TNTABLELEN];
EXTERN	struct font	*font_head	INIT(NULL);
EXTERN	struct tn	*tn_head	INIT(NULL);
#ifdef Omega
EXTERN	wide_ubyte		maxchar;
#else
EXTERN	ubyte		maxchar;
#endif
EXTERN	unsigned short	current_timestamp INIT(0);

/*
 *	Command line flags.
 */

extern	struct _resource {
#if TOOLKIT && CFGFILE
		_Xconst char	*progname;
#endif
		Boolean		regression;
#if TOOLKIT
		int		shrinkfactor;
		_Xconst char	*main_translations;
		_Xconst char	*wheel_translations;
#endif
		int		wheel_unit;
		unsigned int	_density;
#ifdef	GREY
		float		_gamma;
#endif
		int		_pixels_per_inch;
		Boolean		_delay_rulers;
		int		_tick_length;
		char		*_tick_units;
		_Xconst	char	*sidemargin;
		_Xconst	char	*topmargin;
		_Xconst	char	*xoffset;
		_Xconst	char	*yoffset;
		_Xconst	char	*paper;
		_Xconst	char	*_alt_font;
#ifdef MKTEXPK
		Boolean		makepk;
#endif
		_Xconst	char	*mfmode;
	    _Xconst char	*editor;
                Boolean		t1lib;
	    _Xconst char	*src_pos;
		Boolean		_list_fonts;
		Boolean		reverse;
		Boolean		_warn_spec;
		Boolean		_hush_chars;
		Boolean		_hush_chk;
		Boolean		_hush_stdout;
		Boolean		safer;
#if defined(VMS) || !defined(TOOLKIT)
		_Xconst	char	*fore_color;
		_Xconst	char	*back_color;
#endif
		Pixel		_fore_Pixel;
		Pixel		_back_Pixel;
#ifdef TOOLKIT
		Pixel		rule_Pixel;
		Pixel		_brdr_Pixel;
		Pixel		_hl_Pixel;
		Pixel		_cr_Pixel;
#endif
		_Xconst	char	*icon_geometry;
		Boolean		keep_flag;
		Boolean		copy;
		Boolean		thorough;
#if	PS
		/* default is to use DPS, then NEWS, then GhostScript;
		 * we will figure out later on which one we will use */
		Boolean		_postscript;
		Boolean		prescan;
		Boolean		allow_shell;
#ifdef	PS_DPS
		Boolean		useDPS;
#endif
#ifdef	PS_NEWS
		Boolean		useNeWS;
#endif
#ifdef	PS_GS
		Boolean		useGS;
		Boolean		gs_safer;
		Boolean		gs_alpha;
		_Xconst	char	*gs_path;
		_Xconst	char	*gs_palette;
#endif
#endif	/* PS */
		_Xconst	char	*debug_arg;
		Boolean		version_flag;
#ifdef	BUTTONS
		Boolean		expert;
		Boolean		statusline;
		_Xconst char	*button_translations;
		int		shrinkbutton[9];
		int		btn_side_spacing;
		int		btn_top_spacing;
		int		btn_between_spacing;
		int		btn_between_extra;
		int		btn_border_width;
#endif
		_Xconst	char	*mg_arg[5];
#ifdef	GREY
		Boolean		_use_grey;
		Bool3		install;
#endif
#ifdef GRID
		int _grid_mode;
#endif /* GRID */
		char *rule_color;
#ifdef	TOOLKIT
                Pixel rule_pixel;
#endif /* TOOLKIT */
#ifdef HTEX
		Boolean	_underline_link;
		char	*_browser;
		char	*_URLbase;
		char	*_scroll_pages;
#endif /* HTEX */
		char * _help_topics_button_label;
		char * _help_quit_button_label;
		char * _help_intro;
		char * _help_general_menulabel;
		char * _help_general;
		char * _help_hypertex_menulabel;
		char * _help_hypertex;
		char * _help_othercommands_menulabel;
		char * _help_othercommands;
		char * _help_pagemotion_menulabel;
		char * _help_pagemotion;
		char * _help_mousebuttons_menulabel;
		char * _help_mousebuttons;
		char * _help_sourcespecials_menulabel;
		char * _help_sourcespecials;
} resource;

/* As a convenience, we define the field names without leading underscores
 * to point to the field of the above record.  Here are the global ones;
 * the local ones are defined in each module.  */

#define	density		resource._density
#define	pixels_per_inch	resource._pixels_per_inch
#define	alt_font	resource._alt_font
#define	list_fonts	resource._list_fonts
#define	warn_spec	resource._warn_spec
#define	hush_chars	resource._hush_chars
#define	hush_chk	resource._hush_chk
#define	hush_stdout	resource._hush_stdout
#ifdef  GREY
#define	use_grey	resource._use_grey
#endif
#ifdef GRID
#define grid_mode	resource._grid_mode
#endif /* GRID */
#ifdef HTEX
#define underline_link	resource._underline_link
#define browser		resource._browser
#define URLbase		resource._URLbase
#define scroll_pages	resource._scroll_pages
#define KPSE_DEBUG_HYPER	6
#endif /* HTEX */


#ifndef TOOLKIT
EXTERN	Pixel		brdr_Pixel;
#ifdef	GRID
EXTERN	Pixel		rule_Pixel;
#endif /* GRID */
#endif



#ifdef GREY
EXTERN	Pixel		plane_masks[4];
EXTERN	XColor		fore_color_data, back_color_data;
#endif

extern	struct	mg_size_rec {
	int		w;
	int		h;
}
	mg_size[5];

EXTERN	int		debug		INIT(0);

/* for temporary debugging statements */
#ifdef MYDEBUG
#define MYTRACE(X)   do {                                         \
                fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);   \
                fprintf X;                                        \
                fprintf(stderr, "\n");                            \
     } while(0)
#else
#define MYTRACE(X)
#endif

#define	DBG_BITMAP		1
#define	DBG_DVI			2
#define	DBG_PK			4
#define	DBG_BATCH		8
#define	DBG_EVENT		16
#define	DBG_OPEN		32
#define	DBG_PS			64
#define	DBG_STAT		128
#define	DBG_HASH		256
#define	DBG_PATHS		512
#define	DBG_EXPAND		1024
#define	DBG_SEARCH		2048
/* SU 2000/03/10: making this optional doesn't work with NOTOOL */
/* #ifdef HTEX */
#define	DBG_HYPER		4096
#define	DBG_ANCHOR		8192
/* #endif */
#define DBG_SRC_SPECIALS	16384
#define DBG_CLIENT		32768
#define DBG_T1			65536
#define DBG_T1_VERBOSE		131072
#define	DBG_ALL			(~DBG_BATCH)

#ifdef NDEBUG
#define TRACE_SRC(X)	/* as nothing */
#define TRACE_CLIENT(X)	/* as nothing */
#define TRACE_T1(X)	/* as nothing */
#define TRACE_T1_VERBOSE(X)	/* as nothing */
#define TRACE_GUI(X)
#else
#define TRACE_SRC(X)								\
    do {									\
		if (debug & DBG_SRC_SPECIALS) {					\
    		fprintf(stderr, "%s:%d: SRC: ", __FILE__, __LINE__);		\
    		fprintf X;							\
    		fprintf(stderr, "\n");						\
    	}									\
     } while(0)
#define TRACE_CLIENT(X)								\
    do {									\
		if (debug & DBG_CLIENT) {					\
    		fprintf(stderr, "%s:%d: CLIENT: ", __FILE__, __LINE__);		\
    		fprintf X;							\
    		fprintf(stderr, "\n");						\
    	}									\
     } while(0)
#define TRACE_T1(X)								\
    do {									\
		if (debug & DBG_T1) {						\
    		fprintf(stderr, "%s:%d: T1: ", __FILE__, __LINE__);		\
    		fprintf X;							\
    		fprintf(stderr, "\n");						\
    	}									\
     } while(0)
#define TRACE_T1_VERBOSE(X)							\
    do {									\
		if (debug & DBG_T1_VERBOSE) {					\
    		fprintf(stderr, "%s:%d: T1: ", __FILE__, __LINE__);		\
    		fprintf X;							\
    		fprintf(stderr, "\n");						\
    	}									\
     } while(0)
#define TRACE_GUI(X)						    \
    do {							    \
	if (debug & DBG_EVENT) { /* in HEAD we have DBG_GUI for this ...) */ \
	    fprintf(stderr, "%s:%d: GUI: ", __FILE__, __LINE__);    \
	    fprintf X;						    \
	    fprintf(stderr, "\n");				    \
	}							    \
    } while(0)
#endif

EXTERN	int		offset_x, offset_y;
EXTERN	unsigned int	unshrunk_paper_w, unshrunk_paper_h;
EXTERN	unsigned int	unshrunk_page_w, unshrunk_page_h;
#ifdef GRID
EXTERN  unsigned int	unshrunk_paper_unit;
#endif /* GRID */

EXTERN	char		*dvi_name	INIT(NULL);	/* dvi file name */
EXTERN	FILE		*dvi_file;		/* user's file */
EXTERN	time_t		dvi_time;		/* last modification time */
EXTERN	unsigned char	*dvi_property;		/* for setting in window */
EXTERN	size_t		dvi_property_length;
EXTERN	_Xconst char	*prog;
EXTERN	int		bak_shrink;		/* last shrink factor != 1 */
EXTERN	Dimension	window_w, window_h;
EXTERN	XImage		*image;
EXTERN	int		backing_store;
EXTERN	int		home_x, home_y;

EXTERN	Display		*DISP;
EXTERN	Screen		*SCRN;
#if TOOLKIT
extern	XtActionsRec	Actions[];
extern	Cardinal	num_actions;
#endif
#ifndef NOTOOL
EXTERN	XtAccelerators	accelerators;
#endif
#ifdef GREY
EXTERN	int		screen_number;
EXTERN	Visual		*our_visual;
EXTERN	unsigned int	our_depth;
EXTERN	Colormap	our_colormap;
#else
#define	our_depth	(unsigned int) DefaultDepthOfScreen(SCRN)
#define	our_visual	DefaultVisualOfScreen(SCRN)
#define	our_colormap	DefaultColormapOfScreen(SCRN)
#endif
EXTERN	GC		ruleGC;
EXTERN	GC		foreGC, highGC;
EXTERN	GC		foreGC2;
EXTERN	GC		copyGC;
EXTERN  GC      rulerGC;
EXTERN	Boolean		copy;

EXTERN	Cursor		redraw_cursor, ready_cursor, link_cursor, drag_cursor[3];

#if MOTIF && BUTTONS
EXTERN	XtTranslations	wheel_trans_table	INIT(NULL);
#endif

/* BEGIN CHUNK xdvi.h 6 */
#include "message-window.h"

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(expr) /* as nothing */
#endif

EXTERN char *global_dvi_name INIT(NULL); /* save filename argument from xdvi call, including the path  */
/* END CHUNK xdvi.h 6 */

#ifdef	GREY
EXTERN	Pixel		*pixeltbl;
EXTERN	Pixel		*pixeltbl_t;
#endif	/* GREY */

EXTERN	Boolean		canit		INIT(False);
EXTERN	jmp_buf		canit_env;
EXTERN	VOLATILE short	event_counter	INIT(0);
EXTERN	Boolean		terminate_flag	INIT(False);

struct	WindowRec {
	Window		win;
	int		shrinkfactor;
	int		base_x, base_y;
	unsigned int	width, height;
	int		min_x, max_x, min_y, max_y;	/* for pending expose events */
};

extern	struct WindowRec mane, alt, currwin;
EXTERN	int		min_x, max_x, min_y, max_y;

#ifdef	TOOLKIT
EXTERN	Widget		top_level	INIT(0);
EXTERN	Widget		vport_widget, draw_widget, clip_widget;
#if 0
// #ifdef HTEX
// EXTERN	Widget	pane_widget, anchor_search, anchor_info;
// #endif /* HTEX */
#endif
#ifdef MOTIF
EXTERN	Widget		shrink_button[4];
EXTERN	Widget		x_bar, y_bar;	/* horizontal and vert. scroll bars */
#endif /* MOTIF */
#ifdef	BUTTONS
#ifndef MOTIF
#define	XTRA_WID	79
#ifdef STATUSLINE
/* this is only for the initialization; the statusline will reset it to a more adequate value: */
#define	XTRA_H	17 
#endif /* STATUSLINE */
#else /* MOTIF */
#define	XTRA_WID	120
#ifdef STATUSLINE
#define	XTRA_H	17 
#endif /* STATUSLINE */
#endif /* MOTIF */
EXTERN	Widget	form_widget, paned;
EXTERN	int		xtra_wid	INIT(0);
extern	_Xconst char	default_button_config[]; /* defined in events.c */
#endif /* BUTTONS */
#else	/* !TOOLKIT */
EXTERN	Window	top_level	INIT(0);

#define	BAR_WID		12	/* width of darkened area */
#define	BAR_THICK	15	/* gross amount removed */
#endif	/* TOOLKIT */

EXTERN	jmp_buf		dvi_env;	/* mechanism to relay dvi file errors */
EXTERN	_Xconst	char	*dvi_oops_msg;	/* error message */

EXTERN	char	*ffline	INIT(NULL);	/* array used to store filenames */
EXTERN	size_t	ffline_len INIT(0);	/* current length of ffline[] */

/*
 *	Used by the geometry-scanning routines.
 *	It passes pointers to routines to be called at certain
 *	points in the dvi file, and other information.
 */

struct geom_info {
	void	(*geom_box)	ARGS((struct geom_info *,
				  long, long, long, long));
	void	(*geom_special)	ARGS((struct geom_info *, _Xconst char *));
	jmp_buf	done_env;
	void	*geom_data;
};

typedef	void	(*mouse_proc)	ARGS((XEvent *));
extern	void	null_mouse	ARGS((XEvent *));

EXTERN	mouse_proc	mouse_motion	INIT(null_mouse);
EXTERN	mouse_proc	mouse_release	INIT(null_mouse);

/* Used for source special lookup (forward search).  */

EXTERN	Atom	atoms[3];

#define	ATOM_XDVI_WINDOWS	(atoms[0])
#define	ATOM_DVI_FILE		(atoms[1])
#define	ATOM_SRC_GOTO		(atoms[2])

#ifdef	SELFAUTO
EXTERN	_Xconst	char	*argv0;		/* argv[0] */
#endif

#ifdef	CFG2RES
struct cfg2res {
	_Xconst	char	*cfgname;	/* name in config file */
	_Xconst	char	*resname;	/* name of resource */
	Boolean		numeric;	/* if numeric */
};
#endif

#if	PS
	
extern	struct psprocs	{
	void	(*toggle)	ARGS((void));
	void	(*destroy)	ARGS((void));
	void	(*interrupt)	ARGS((void));
	void	(*endpage)	ARGS((void));
	void	(*drawbegin)	ARGS((int, int, _Xconst char *));
	void	(*drawraw)	ARGS((_Xconst char *));
	void	(*drawfile)	ARGS((_Xconst char *, FILE *));
	void	(*drawend)	ARGS((_Xconst char *));
	void	(*beginheader)	ARGS((void));
	void	(*endheader)	ARGS((void));
	void	(*newdoc)	ARGS((void));
}	psp, no_ps_procs;

#endif	/* PS */

/********************************
 *	   Procedures		*
 *******************************/

_XFUNCPROTOBEGIN /* expands to `extern "C" {' in C++ mode */

extern void fork_process ARGS((_Xconst char *file, char *_Xconst argv[]));
extern void print_child_error ARGS((void));
extern int get_avg_font_width(XFontStruct *font);

#ifdef	BUTTONS
extern	void	create_buttons ARGS((void));

#ifdef STATUSLINE
extern Widget statusline;
#endif

/* statusline stuff. The following function and #defines are also
 * needed when compiling without statusline support.
 */
typedef enum STATUS_TIMER_
{
    STATUS_FOREVER = 0,
    STATUS_SHORT = 10,
    STATUS_MEDIUM = 40,
    STATUS_LONG = 90
} STATUS_TIMER;

extern void print_statusline(STATUS_TIMER, _Xconst char *fmt, ...);
extern void force_statusline_update ARGS((void));

#ifdef STATUSLINE
extern void create_statusline ARGS((void));
extern void handle_statusline_resize ARGS((void));
#endif

#if !MOTIF
extern	void	set_button_panel_height ARGS((XtArgVal));
#endif /* !MOTIF */
#endif /* BUTTONS */
#ifdef	GREY
extern	void	init_plane_masks ARGS((void));
extern	void	init_pix ARGS((void));
#endif /* GREY */
extern int check_goto_page(int pageno);
extern	void	reconfig ARGS((void));
#ifdef	TOOLKIT
extern	void	handle_resize ARGS((Widget, XtPointer, XEvent *, Boolean *));
extern	void	handle_expose ARGS((Widget, XtPointer, XEvent *, Boolean *));
extern  void	handle_property_change ARGS((Widget, XtPointer, XEvent *, Boolean *));
#endif /* TOOLKIT */
#ifdef MOTIF
extern	void	file_pulldown_callback ARGS((Widget, XtPointer, XtPointer));
extern	void	navigate_pulldown_callback ARGS((Widget, XtPointer, XtPointer));
extern	void	scale_pulldown_callback ARGS((Widget, XtPointer, XtPointer));
extern	void	set_shrink_factor ARGS((int));
#endif /* MOTIF */
extern	void	showmessage ARGS((_Xconst char *));
#if	PS
extern	void	ps_read_events ARGS((wide_bool, wide_bool));
#define	read_events(wait)	ps_read_events(wait, True)
#else /* PS */
extern	void	read_events ARGS((wide_bool));
#endif /* PS */
extern	void	redraw_page ARGS((void));
extern	void	enable_intr ARGS((void));
extern	void	do_pages ARGS((void));
extern	void	free_fontlist ARGS((void));
extern	void	reset_fonts ARGS((void));
extern	void	realloc_font ARGS((struct font *, wide_ubyte));
extern	void	realloc_virtual_font ARGS((struct font *, wide_ubyte));
extern	Boolean	load_font ARGS((struct font *));
extern	struct font	*define_font ARGS((FILE *, wide_ubyte,
			struct font *, struct font **, unsigned int,
			struct tn **, Boolean *not_found_flag));
extern	void	init_page ARGS((void));
extern	void	open_dvi_file ARGS((void));
extern	void	form_dvi_property ARGS((void));
extern	void	init_dvi_file ARGS((void));
extern	void	set_dvi_property ARGS((void));
extern	int dvi_file_changed ARGS((void));
#ifdef GRID
extern	void	put_grid ARGS((int, int, unsigned int, unsigned int, unsigned int, GC));
#endif /* GRID */
#ifndef	TEXXET
extern	long	set_char ARGS((wide_ubyte));
extern	long	load_n_set_char ARGS((wide_ubyte));
extern	long	set_vf_char ARGS((wide_ubyte));
extern	long	set_t1_char ARGS((wide_ubyte));
#else /* TEXXET */
extern	void	set_char ARGS((wide_ubyte, wide_ubyte));
extern	void	load_n_set_char ARGS((wide_ubyte, wide_ubyte));
extern	void	set_vf_char ARGS((wide_ubyte, wide_ubyte));
extern	void	set_t1_char ARGS((wide_ubyte, wide_ubyte));
#endif /* TEXXET */

#ifdef T1LIB
extern	int	tfmload ARGS((char *, long *, long *));
extern	void	read_T1_char ARGS((struct font *, wide_ubyte));
extern  void	add_T1_mapentry ARGS((int, char *, const char *, char *, char *, char *));
extern  int	find_T1_font ARGS((const char *));
#endif /* T1LIB */
extern  int	getpsinfo ARGS((char *));
extern	void	draw_page ARGS((void));
extern	void	source_reverse_search ARGS((int, int, wide_bool));
extern	void	source_special_show ARGS((wide_bool));
extern	void	source_forward_search ARGS((_Xconst char *));
#ifdef T1LIB
extern  void	init_t1 ARGS((void));
#endif
#if	CFGFILE
#ifndef	CFG2RES
extern	void	readconfig ARGS((void));
#else /* CFG2RES */
extern	void	readconfig ARGS((_Xconst struct cfg2res *,
		  _Xconst struct cfg2res *, XtResource *, XtResource *));
#endif	/* CFG2RES */
#endif	/* CFGFILE */
extern	void	init_font_open ARGS((void));
#ifdef T1LIB
extern	FILE	*font_open ARGS((char *, char **, double, int *, int, char **,int *));
#else
extern	FILE	*font_open ARGS((char *, char **, double, int *, int, char **));
#endif /* T1LIB */

#if	PS
extern	void	ps_clear_cache ARGS((void));
extern	void	ps_newdoc ARGS((void));
extern	void	ps_destroy ARGS((void));
extern	void	scan_special ARGS((char *));
extern	void	drawbegin_none ARGS((int, int, _Xconst char *));
extern	void	draw_bbox ARGS((void));
extern	void	NullProc ARGS((void));
#ifdef	PS_DPS
extern	Boolean	initDPS ARGS((void));
#endif /* PS_DPS */
#ifdef	PS_NEWS
extern	Boolean	initNeWS ARGS((void));
#endif /* PS_NEWS */
#ifdef	PS_GS
extern Boolean FIXME_ps_lock;
extern	Boolean	initGS ARGS((void));
#endif /* PS_GS */
#endif /* PS */

extern	void	applicationDoSpecial ARGS((char *));

extern	NORETURN void	xdvi_exit ARGS((int));
extern	NORETURN void	xdvi_abort ARGS((void));

extern	void	geom_do_special ARGS((struct geom_info *, char *, double));

extern NORETURN void oops(_Xconst char *fmt, ...);


extern char *my_realpath(const char *path, char *real);
#if HAVE_REALPATH
#include <limits.h>
#include <stdlib.h>
# define REALPATH realpath
#else
# define REALPATH my_realpath
#endif


#ifndef KPATHSEA
extern	void	*xmalloc ARGS((unsigned));
extern	void	*xrealloc ARGS((void *, unsigned));
extern	char	*xstrdup ARGS((_Xconst char *));
extern	char	*xmemdump ARGS((_Xconst char *, size_t));
extern	void	xputenv ARGS((_Xconst char *, _Xconst char *));
#endif /* KPATHSEA */
extern	void	expandline ARGS((size_t));
extern	void	alloc_bitmap ARGS((struct bitmap *));

#ifndef HAVE_MEMICMP
extern	int	memicmp ARGS((_Xconst char *, _Xconst char *, size_t));
#endif

extern	FILE	*xfopen ARGS((_Xconst char *, OPEN_MODE_ARGS));
#ifdef HTEX
extern	FILE	*xfopen_local ARGS((_Xconst char *, OPEN_MODE_ARGS));
#else
#define xfopen_local	xfopen
#endif

extern	int	xpipe ARGS((int *));
extern	DIR	*xopendir ARGS((_Xconst char *));
extern	_Xconst	struct passwd *ff_getpw ARGS((_Xconst char **, _Xconst char *));
extern	unsigned long	num ARGS((FILE *, int));
extern	long	snum ARGS((FILE *, int));
extern	size_t	property_get_data ARGS((Window, Atom, unsigned char **,
		  int (*property_get_data) (Display *, Window, Atom, long,
		  long, Bool, Atom, Atom *, int *, unsigned long *,
		  unsigned long *, unsigned char **)));
#if PS
extern	int	xdvi_temp_fd ARGS((char **));
#endif
extern	void	read_PK_index ARGS((struct font *, wide_bool));
extern	void	read_GF_index ARGS((struct font *, wide_bool));
#ifdef Omega
extern	unsigned long read_VF_index ARGS((struct font *, wide_bool));
#else /* OMEGA */
extern	void	read_VF_index ARGS((struct font *, wide_bool));
#endif /* OMEGA */

/* SU 2000/03/10: removed the HTEX condition for next declaration */
extern	int	pointerlocate ARGS((int *, int *));

#ifdef HTEX
char *urlocalize ARGS((char *filename));
extern int lastwwwopen;

EXTERN	Boolean	URL_aware	INIT(False);
extern int HTeXAnestlevel;	/* Hypertext nesting level */
extern int HTeXSrc;	/* Hypertext nesting level */
extern int HTeXreflevel;	/* flag for whether we are inside an href */

extern int global_x_marker, global_y_marker, global_marker_page;
extern  void	htex_draw_anchormarker ARGS((int, int));
extern  char    *htex_file_at_index ARGS((int));
extern  char    *htex_url_at_index ARGS((int));
extern	void	search_callback ARGS((Widget, XtPointer, XtPointer));
extern	void	detach_anchor ARGS((void));
extern	char	*MyStrAllocCopy ARGS((char **, char *));
extern  void	htex_open_dvi_file ARGS((void));
extern	void	htex_recordbits ARGS((int, int, int, int));
extern	void	htex_initpage ARGS((void));
extern	void	htex_donepage ARGS((int, int));
extern	void	htex_parsepages ARGS((void));
extern	Boolean	htex_parse_page ARGS((int));
extern	void	check_for_anchor ARGS((void));
extern	void	htex_scanpage ARGS((int));
extern	void	htex_dospecial ARGS((long, int));
extern	void	htex_reinit ARGS((void));
extern	void	htex_do_loc ARGS((Boolean, char *));
extern	void	add_search ARGS((char *, int));
extern	int	htex_handleref ARGS((Boolean, int, int, int));
extern	void	htex_displayanchor ARGS((int, int, int));
extern	void	htex_goback ARGS((void));
extern	int	checkHyperTeX ARGS((char *, int));
extern	int	htex_is_url ARGS((const char *));
extern  void 	paint_anchor ARGS((char *));
extern	int	fetch_relative_url ARGS((char *, _Xconst char *));
extern	void	wait_for_urls ARGS((void));
extern  char	*figure_mime_type ARGS((char *));
extern	void	htex_cleanup ARGS((void));
#endif
extern void	set_icon_and_title ARGS((char *dvi_name, char **icon_ret, char **title_ret, int set));
extern void	put_rule ARGS((Boolean highlight, int x, int y, unsigned int w, unsigned int h));
extern void	home ARGS((wide_bool go_home));
#ifdef SELFILE
extern FILE	*select_filename ARGS((int open, int move_home));
#endif
extern void	clearexpose ARGS((struct WindowRec *, int, int, unsigned int, unsigned int));

#ifdef HTEX
extern void	make_absolute ARGS((char *rel, char *base, int len));
extern int	invokeviewer ARGS((char *filename));
#endif /* HTEX */


#ifdef TOOLKIT
extern void show_help ARGS((Widget));
extern void do_popup_message(int type, char *helptext, char *msg, ...);
#define MSG_HELP 0
#define MSG_INFO 1
#define MSG_WARN 2
#define MSG_ERR  3
#else
extern void show_help ARGS((void));
#endif /* TOOLKIT */


_XFUNCPROTOEND

#define one(fp)		((unsigned char) getc(fp))
#define sone(fp)	((long) one(fp))
#define two(fp)		num (fp, 2)
#define stwo(fp)	snum(fp, 2)
#define four(fp)	num (fp, 4)
#define sfour(fp)	snum(fp, 4)
/*
  hashtable wrapper functions, mostly used by dvi-draw.c to
  map filenames to integers. This uses the hashtable implementation
  from kpathsea, which is reasonably fast.
*/

/*
  We use this dummy wrapper stuct, which we cast to void *, to get integer
  values into/from the hashtable (natively, kpahtsea only supports string
  values).
*/
struct str_int_hash_item {
    int value;
};

extern Boolean find_str_int_hash(hash_table_type *hashtable, const char *key, int *val);
void put_str_int_hash(hash_table_type *hashtable, const char *key, int val);

#endif	/* XDVI_H */
