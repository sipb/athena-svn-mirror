/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: getman.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
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
    static char			buff[BUFSIZ];
#if	defined(HAVE_SYSINFO)
    register char	       *cp;

    if (sysinfo(SI_HW_PROVIDER, buff, sizeof(buff)) < 0)
	    Error("sysinfo SI_HW_PROVIDER failed.");
    else
	if (cp = strchr(buff, '_'))
	    *cp = C_NULL;
#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
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
    static char			buff[BUFSIZ];
#if	defined(HAVE_SYSINFO)
    register char	       *cp;

    if (sysinfo(SI_HW_PROVIDER, buff, sizeof(buff)) < 0)
	Error("sysinfo SI_HW_PROVIDER failed.");
    else {
	cp = buff;
	while (cp && (cp = strchr(cp, '_')))
	    *cp++ = ' ';
    }
#endif	/* HAVE_SYSINFO */

    return((buff[0]) ? buff : (char *)NULL);
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
    static char 		Buf[BUFSIZ];

    Buf[0] = CNULL;
    ms = GetManShort();
    ml = GetManLong();

    if (!ms || !ml)
	return((char *) NULL);

    if (!VL_TERSE && !VL_BRIEF)
	(void) sprintf(Buf, "%s (%s)", ms, ml);
    else
	(void) sprintf(Buf, "%s", ms);

    return(Buf);
}

