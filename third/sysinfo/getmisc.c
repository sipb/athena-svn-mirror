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
 * Miscellaneous Get*() functions
 */

#include "defs.h"

/*
 * Get host ID
 */
extern char *GetHostID()
{
    static char 		Buff[100];

#if	defined(HAVE_GETHOSTID)
    (void) snprintf(Buff, sizeof(Buff),  "%08x", gethostid());
#endif	/* HAVE_GETHOSTID */

    return( (Buff[0]) ? Buff : (char *)NULL );
}

#if	defined(HAVE_SYSINFO) && defined(SI_HW_SERIAL)
/*
 * Get Serial Number using sysinfo()
 */
extern char *GetSerialSysinfo()
{
    static char 		Buff[128];
    static int 			done = 0;
    static char 	       *retval;

    if (done)
	return((char *) retval);

    done = 1;
    if (sysinfo(SI_HW_SERIAL, Buff, sizeof(Buff)) < 0)
	Buff[0] = CNULL;

    return((retval = ((Buff[0]) ? Buff : (char *)NULL)) );
}
#endif	/* HAVE_SYSINFO ... */

/*
 * Get serial number
 */
extern char *GetSerial()
{
    extern PSI_t	       GetSerialPSI[];

    return(PSIquery(GetSerialPSI));
}

/*
 * Get ROM Version
 */
extern char *GetRomVer()
{
    extern PSI_t	       GetRomVerPSI[];

    return(PSIquery(GetRomVerPSI));
}
