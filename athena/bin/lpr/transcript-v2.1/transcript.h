/* transcript.h
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/transcript.h,v 1.2 1995-07-11 21:24:25 miki Exp $
 *
 * some general global defines
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  94/09/15  12:01:25  probe
 * POSIX compliance.
 * 
 * Revision 1.2  94/09/14  13:59:22  probe
 * POSIX support added.
 * Assume everything has strchr/strrchr (even BSD did)
 * 
 * Revision 1.1  94/02/22  15:52:07  probe
 * Initial revision
 * 
 * Revision 2.2  87/11/17  16:52:57  byron
 * Release 2.1
 * 
 * Revision 2.1.1.2  87/11/12  13:42:14  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.1  87/04/23  10:27:01  byron
 * Copyright notice.
 * 
 * Revision 2.2  86/11/02  14:13:42  shore
 * Product Update
 * 
 * Revision 2.1  85/11/24  11:51:21  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:58:05  shore
 * changes for Release 2 with System V support
 * 
 * Revision 1.2  85/05/14  11:31:52  shore
 * 
 * 
 *
 */

#define private static

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif
#define TRUE	(1)
#define FALSE	(0)

#define PSVERSION 	23.0
#define COMMENTVERSION	"PS-Adobe-1.0"

#define VOID void
#define VOIDC (void)

#define INDEX strchr
#define RINDEX strrchr

/* external globals (from config.c) */

extern char LibDir[];
extern char TroffFontDir[];
extern char DitDir[];
extern char TempDir[];

/* definitions of basenames of various prologs, filters, etc */
/* all of these get concatenated as LibDir/filename */

#define BANNERPRO	"/banner.pro"
#define PLOTPRO		"/psplot.pro"
#define CATPRO		"/pscat.pro"
#define TEXTPRO		"/pstext.pro"
#define ENSCRIPTPRO	"/enscript.pro"
#define PS4014PRO	"/ps4014.pro"
#define PS630PRO	"/ps630.pro"
#define PSDITPRO	"/psdit.pro"
#define PSSUNPRO	"/pssun.pro"
#define FONTMAP		"/font.map"

#define ENSCRIPTTEMP	"/ESXXXXXX"
#define REVTEMP		"/RVXXXXXX"

#define PSTEXT		"pstext"
#define PSPLOT		"psplot"
#define PSCAT		"pscat"
#define PSDIT		"psdit"
#define PSIF		"psif"
#define PSSUN		"pssun"

/* psutil functions */
extern char *mstrcat();
extern char *envget();
extern VOID pexit();
extern VOID pexit2();

extern char *mapname();

/* system functions */
#ifdef POSIX
#include <unistd.h>
#include <stdlib.h>
#else
#include <sys/types.h>
extern VOID perror();
extern VOID exit();
extern char *ctime();
extern time_t time();
extern char *mktemp();
extern char *gets();
extern char *malloc();
#endif
