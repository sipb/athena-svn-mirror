/* transcript.h
 *
 * Copyright (C) 1985,1987,1991 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/transcript.h,v 1.1.1.1 1996-10-07 20:25:53 ghudson Exp $
 *
 * some general global defines
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 3.1  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.0  1991/06/17  17:01:02  snichols
 * Release 3.0
 *
 * Revision 2.12  1991/06/17  17:00:25  snichols
 * remove declarations of time and ctime.
 *
 * Revision 2.11  1991/03/28  23:47:32  snichols
 * isolated code for finding PPD files to one routine, in psutil.c.
 *
 * Revision 2.10  1991/03/26  20:42:47  snichols
 * only define True and FALSe if not defined.
 *
 * Revision 2.9  1991/03/25  21:45:29  snichols
 * compat.h should be in quotes.
 *
 * Revision 2.8  1991/03/25  21:42:00  snichols
 * garbage at end.
 *
 * Revision 2.7  1991/03/25  21:27:44  snichols
 * got rid of old garbage, and added inclusion of compat.h for non-XPG3
 * systems.
 *
 * Revision 2.6  1991/01/16  14:14:17  snichols
 * Added support for LZW compression and ascii85 encoding for Level 2 printers.
 *
 * Revision 2.5  91/01/04  16:02:28  snichols
 * use stdlib.h in XPG3 systems rather than declaring malloc, etc.
 * 
 * Revision 2.4  90/12/12  09:38:13  snichols
 * added new extern's from config.h, and defined TSPATHSIZE.
 * 
 * Revision 2.3  90/10/11  15:09:17  snichols
 * changed COMMENTVERSION to 3.0
 * 
 * Revision 2.2  87/11/17  16:52:57  byron
 * *** empty log message ***
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

#ifdef XPG3
#include <stdlib.h>
#else
#include "compat.h"
#endif
#include <stdio.h>

#define private static

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define COMMENTVERSION	"PS-Adobe-3.0"

#define VOID void
#define VOIDC (void)

/* extern globals from config.h */

extern char *PSLibDir;
extern char *PPDDir;
extern char *TroffFontDir;
extern char *ResourceDir;
extern char *DitDir;
extern char *TempDir;
extern char *lp;
extern char *bindir;


/* pathname limit */
#define TSPATHSIZE 1024

/* buffer size, used in numerous places, except where parsing is happening. */
#define TSBUFSIZE 20480

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


#define PSPLOT		"psplot"
#define PSCAT		"pscat"
#define PSDIT		"psdit"
#define PSIF		"psif"

/* psutil functions */
extern char *mstrcat();
extern char *envget();
extern VOID pexit();
extern VOID pexit2();
extern char *parseppd();
extern FILE *GetPPD();

extern char *mapname();

/* system functions */
extern VOID perror();

extern char *optarg;
extern int optind;
