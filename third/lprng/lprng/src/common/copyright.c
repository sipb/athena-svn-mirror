/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: copyright.c,v 1.1.1.4 2000-03-31 15:48:03 mwhitson Exp $";


#include "lp.h"
#include "patchlevel.h"
/**** ENDINCLUDE ****/

char *Copyright[] = {
"LPRng version " PATCHLEVEL "",
"Author: Patrick Powell, <papowell@astart.com>",
"",
"locking uses: "
#ifdef HAVE_FCNTL
		"fcntl (preferred)"
#else
#ifdef HAVE_LOCKF
            "lockf"
#else
            "flock (does NOT work over NFS)"
#endif
#endif
,
"stty uses: "
#if USE_STTY == SGTTYB
            "sgttyb"
#endif
#if USE_STTY == TERMIO
            "termio"
#endif
#if USE_STTY == TERMIOS
            "termios"
#endif
,
"",
#include "license.h"
#include "copyright.h"
0 };
