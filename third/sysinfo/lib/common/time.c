/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Time functions
 */

#include <time.h>
#include "defs.h"

/*
 * Convert TimeVal to a string
 */
extern char *TimeToStr(TimeVal, Format)
     time_t			TimeVal;
     char		       *Format;
{
    struct tm		       *tm;
    static char			Buff[128];
    char		       *String;
    char		       *cp;
    char		       *Fmt;

    if (Format)
	Fmt = Format;
    else
	/* Use %H instead of %k as %H is more portable */
	Fmt = "%a %b %e %H:%M:%S %Y %Z";

#if	defined(HAVE_STRFTIME)
    if (tm = localtime(&TimeVal))
	if (strftime(Buff, sizeof(Buff), Fmt, tm) > 0)
	    return Buff;
#endif	/* HAVE_STRFTIME */

#if	defined(HAVE_ASCTIME)
    if (tm = localtime(&TimeVal))
	if (String = asctime(tm)) {
	    if (cp = strchr(String, '\n'))
		*cp = CNULL;
	    return String;
	}
#endif	/* HAVE_ASCTIME */

    SImsg(SIM_DBG, "TimeToStr() failed - No conversion func defined?");

    return (char *) NULL;
}
