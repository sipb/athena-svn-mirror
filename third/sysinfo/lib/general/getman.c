/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
extern char *GetManShortStr()
{
    extern PSI_t	       GetManShortPSI[];

    return PSIquery(GetManShortPSI);
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
extern char *GetManLongStr()
{
    extern PSI_t	       GetManLongPSI[];

    return PSIquery(GetManLongPSI);
}

/*
 * Exported API versin of GetManLong()
 */
extern int GetManLong(Query)
     MCSIquery_t	      *Query;
{
    char		       *Str;

    if (Query->Op == MCSIOP_CREATE) {
	Str = GetManLongStr();

	if (Str) {
	    Query->Out = (Opaque_t) strdup(Str);
	    Query->OutSize = strlen(Str);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}

/*
 * Exported API versin of GetManShort()
 */
extern int GetManShort(Query)
     MCSIquery_t	      *Query;
{
    char		       *Str;

    if (Query->Op == MCSIOP_CREATE) {
	Str = GetManShortStr();

	if (Str) {
	    Query->Out = (Opaque_t) strdup(Str);
	    Query->OutSize = strlen(Str);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}

/*
 * Get the manufacturer info.
 */
extern int GetMan(Query)
     MCSIquery_t	      *Query;
{
    char 		       *ms, *ml;
    static char 		Buf[256];

    if (Query->Op == MCSIOP_CREATE) {
	if (!Buf[0]) {
	    ms = GetManShortStr();
	    ml = GetManLongStr();

	    if (!ms || !ml)
		return -1;

	    (void) snprintf(Buf, sizeof(Buf),  "%s (%s)", ms, ml);
	}

	if (Buf[0]) {
	    Query->Out = (Opaque_t) strdup(Buf);
	    Query->OutSize = strlen(Buf);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}
