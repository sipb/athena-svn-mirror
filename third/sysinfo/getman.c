/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

/*

 * Get architecture information
 */

#include "defs.h"

/*
 * Return the predefined symbol MAN_SHORT
 */
extern char *GetManShortDef()
{
#if	defined(MAN_SHORT)
    return(MAN_SHORT);
#else	/* !MAN_SHORT */
    return((char *) NULL);
#endif	/* MAN_SHORT */
}

/*
 * Try the sysinfo() call.
 */
extern char *GetManShortSysinfo()
{
    static char			Buff[256];
#if	defined(HAVE_SYSINFO)
    register char	       *cp;

    if (sysinfo(SI_HW_PROVIDER, Buff, sizeof(Buff)) < 0)
	    SImsg(SIM_GERR, "sysinfo SI_HW_PROVIDER failed.");
    else
	if (cp = strchr(Buff, '_'))
	    *cp = C_NULL;
#endif	/* HAVE_SYSINFO */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get the short manufacturer name
 */
extern char *GetManShort()
{
    extern PSI_t	       GetManShortPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetManShortPSI));
}

/*
 * Return the predefined symbol MAN_LONG
 */
extern char *GetManLongDef()
{
#if	defined(MAN_LONG)
    return(MAN_LONG);
#else	/* !MAN_LONG */
    return((char *) NULL);
#endif	/* MAN_LONG */
}

/*
 * Try the sysinfo() call
 */
extern char *GetManLongSysinfo()
{
    static char			Buff[256];
#if	defined(HAVE_SYSINFO)
    register char	       *cp;

    if (sysinfo(SI_HW_PROVIDER, Buff, sizeof(Buff)) < 0)
	SImsg(SIM_GERR, "sysinfo SI_HW_PROVIDER failed.");
    else {
	cp = Buff;
	while (cp && (cp = strchr(cp, '_')))
	    *cp++ = ' ';
    }
#endif	/* HAVE_SYSINFO */

    return((Buff[0]) ? Buff : (char *)NULL);
}

/*
 * Get the long manufacturer name
 */
extern char *GetManLong()
{
    extern PSI_t	       GetManLongPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetManLongPSI));
}

/*
 * Get the manufacturer info.
 */
extern char *GetMan()
{
    char 		       *ms, *ml;
    static char 		Buf[256];

    Buf[0] = CNULL;
    ms = GetManShort();
    ml = GetManLong();

    if (!ms || !ml)
	return((char *) NULL);

    if (!VL_TERSE && !VL_BRIEF)
	(void) snprintf(Buf, sizeof(Buf),  "%s (%s)", ms, ml);
    else
	(void) snprintf(Buf, sizeof(Buf),  "%s", ms);

    return(Buf);
}

