#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/map.c,v 1.1.1.1 1996-10-07 20:25:49 ghudson Exp $";
#endif
/* map.c
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * front end to mapname -- font mapping for users
 *
 * for non-4.2bsd systems (e.g., System V) which do not
 * allow long Unix file names
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 3.1  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.0  1991/06/17  16:50:45  snichols
 * Release 3.0
 *
 * Revision 2.6  1991/03/01  14:04:20  snichols
 * Use default resource path in ListPSResourceFiles.
 *
 * Revision 2.5  91/02/25  17:03:35  snichols
 * Use PSres package for mapping, for all versions of Unix.
 * 
 * Revision 2.4  91/01/04  09:17:31  snichols
 * put #ifdef FOURTEEN around everything, so this is only compiled in if 
 * necessary.
 * 
 * Revision 2.3  90/12/12  13:58:27  snichols
 * new configuration stuff.
 * 
 * Revision 2.2  87/11/17  16:49:57  byron
 * *** empty log message ***
 * 
 * Revision 2.2  87/11/17  16:49:57  byron
 * Release 2.1
 * 
 * Revision 2.1.1.2  87/11/12  13:40:11  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.1  87/04/23  10:25:24  byron
 * Copyright notice.
 * 
 * Revision 2.2  86/11/02  14:15:21  shore
 * Product Update
 * 
 * Revision 2.1  85/11/24  11:49:13  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  00:14:39  shore
 * Initial revision
 * 
 *
 */
#include <stdio.h>
#include "transcript.h"
#include "config.h"
#include "PSres.h"

main(argc,argv)
int argc;
char **argv;
{
    char result[128];
    int count;
    char **files;
    char **names;

    if (argc != 2) exit(1);

    count = ListPSResourceFiles(NULL, ResourceDir, PSResFontAFM, argv[1],
				&names, &files);
    if (count == 0) exit(1);
    printf("%s", files[0]);
    exit(0);
}
