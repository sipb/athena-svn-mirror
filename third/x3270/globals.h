/*
 * Modifications Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
 * Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	globals.h
 *		Common definitions for x3270.
 */

/*
 * OS-specific #defines.
 */

/*
 * SEPARATE_SELECT_H
 *   The definitions of the data structures for select() are in <select.h>.
 * NO_SYS_TIME_H
 *   Don't #include <sys/time.h>.
 * NO_MEMORY_H
 *   Don't #include <memory.h>.
 * LOCAL_TELNET_H
 *   #include a local copy of "telnet.h" rather then <arpa/telnet.h>
 * SELECT_INT
 *   select() takes (int *) arguments rather than (fd_set *) arguments.
 * BLOCKING_CONNECT_ONLY
 *   Use only blocking sockets.
 */
#if defined(sco) /*[*/
#define BLOCKING_CONNECT_ONLY	1
#define NO_SYS_TIME_H		1
#endif /*]*/

#if defined(_IBMR2) || defined(_SEQUENT_) || defined(__QNX__) /*[*/
#define SEPARATE_SELECT_H	1
#endif /*]*/

#if defined(apollo) /*[*/
#define BLOCKING_CONNECT_ONLY	1
#define NO_MEMORY_H		1
#endif /*]*/

#if defined(hpux) /*[*/
#define SELECT_INT		1
#define LOCAL_TELNET_H		1
#endif /*]*/

/*
 * Prerequisite #includes.
 */
#include <stdio.h>			/* Unix standard I/O library */
#include <stdlib.h>			/* Other Unix library functions */
#include <unistd.h>			/* Unix system calls */
#include <ctype.h>			/* Character classes */
#include <string.h>			/* String manipulations */
#if !defined(NO_MEMORY_H) /*[*/
#include <memory.h>			/* Block moves and compares */
#endif /*]*/
#include <sys/types.h>			/* Basic system data types */
#if !defined(NO_SYS_TIME_H) /*[*/
#include <sys/time.h>			/* System time-related data types */
#endif /*]*/
#include <X11/Intrinsic.h>

/* Simple global variables */

extern int		COLS;
extern int		ROWS;
extern XtAppContext	appcontext;
extern Atom		a_delete_me;
extern Atom		a_save_yourself;
extern Atom		a_state;
extern Pixel		colorbg_pixel;
extern char		*current_host;
extern unsigned short	current_port;
extern Boolean		*debugging_font;
extern int		depth;
extern Display		*display;
extern XFontStruct      **efontinfo;
extern char		*efontname;
extern Boolean		error_popup_visible;
extern Boolean		ever_3270;
extern Boolean		exiting;
extern Boolean		*extended_3270font;
extern Boolean		flipped;
extern char		*full_current_host;
extern char		full_model_name[];
extern Pixmap		gray;
extern Pixel		keypadbg_pixel;
extern Boolean		*latin1_font;
extern int		maxCOLS;
extern int		maxROWS;
extern char		*model_name;
extern int		model_num;
extern int		ov_cols, ov_rows;
extern Boolean		passthru_host;
extern char		*programname;
extern Boolean		reconnect_disabled;
extern XrmDatabase	rdb;
extern Window		root_window;
extern Boolean		scroll_initted;
extern Boolean		shifted;
extern Boolean		*standard_font;
extern Boolean		std_ds_host;
extern char		*termtype;
extern Widget		toplevel;
extern char		*user_icon_name;
extern char		*user_title;

/* Data types and complex global variables */

/*   connection state */
enum cstate {
	NOT_CONNECTED,		/* no socket, unknown mode */
	PENDING,		/* connection pending */
	CONNECTED_INITIAL,	/* connected in ANSI mode */
	CONNECTED_3270		/* connected in 3270 mode */
};
extern enum cstate cstate;

#define PCONNECTED	((int)cstate >= (int)PENDING)
#define HALF_CONNECTED	(cstate == PENDING)
#define CONNECTED	((int)cstate >= (int)CONNECTED_INITIAL)
#define IN_3270		(cstate == CONNECTED_3270)
#define IN_ANSI		(cstate == CONNECTED_INITIAL)

/*   mouse-cursor state */
enum mcursor_state { LOCKED, NORMAL, WAIT };

/*   keyboard modifer bitmap */
#define ShiftKeyDown	0x01
#define MetaKeyDown	0x02
#define AltKeyDown	0x04

/*   toggle names */
struct toggle_name {
	char *name;
	int index;
};
extern struct toggle_name toggle_names[];

/*   extended attributes */
struct ea {
	unsigned char fg;	/* foreground color (0x00 or 0xf<n>) */
	unsigned char bg;	/* background color (0x00 or 0xf<n>) */
	unsigned char gr;	/* ANSI graphics rendition bits */
	unsigned char cs;	/* character set (GE flag, or 0..2) */
};
#define GR_BLINK	0x01
#define GR_REVERSE	0x02
#define GR_UNDERLINE	0x04
#define GR_INTENSIFY	0x08

#define CS_MASK		0x03	/* mask for specific character sets */
#define CS_GE		0x04	/* cs flag for Graphic Escape */

/*   translation lists */
struct trans_list {
	char			*name;
	struct trans_list	*next;
};
extern struct trans_list *trans_list;

/*   font list */
struct font_list {
	char			*label;
	char			*font;
	struct font_list	*next;
};
extern struct font_list *font_list;
extern int font_count;

/*   input key type */
enum keytype { KT_STD, KT_GE };

/* Shorthand macros */

#define CN	((char *) NULL)
#define PN	((XtPointer) NULL)

/* Portability macros */

/*   Replacement for memcpy that handles overlaps */

#if XtSpecificationRelease >= 5 /*[*/
#include <X11/Xfuncs.h>
#undef MEMORY_MOVE
#define MEMORY_MOVE(to,from,cnt)	bcopy(from,to,cnt)
#else /*][*/
#if !defined(MEMORY_MOVE) /*[*/
extern char *MEMORY_MOVE();
#endif /*]*/
#endif /*]*/

/*   Equivalent of setlinebuf */

#if defined(_IOLBF) /*[*/
#define SETLINEBUF(s)	setvbuf(s, (char *)NULL, _IOLBF, BUFSIZ)
#else /*][*/
#define SETLINEBUF(s)	setlinebuf(s)
#endif /*]*/

/*   Motorola version of gettimeofday */

#if defined(MOTOROLA)
#define gettimeofday(tp,tz)	gettimeofday(tp)
#endif
