/* transcript.h
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Id: transcript.h,v 1.3 1999-01-22 23:11:32 ghudson Exp $
 *
 * some general global defines
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
