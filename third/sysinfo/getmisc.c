/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: getmisc.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
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
    static char 		Buf[100];

#if	defined(HAVE_GETHOSTID)
    (void) sprintf(Buf, "%08x", gethostid());
#endif	/* HAVE_GETHOSTID */

    return( (Buf[0]) ? Buf : UnSupported );
}

#if	defined(HAVE_SYSINFO) && defined(SI_HW_SERIAL)
/*
 * Get Serial Number using sysinfo()
 */
extern char *GetSerialSysinfo()
{
    static char 		buf[BUFSIZ];
    static int 			done = 0;
    static char 	       *retval;

    if (done)
	return((char *) retval);

    done = 1;
    if (sysinfo(SI_HW_SERIAL, buf, BUFSIZ) < 0)
	buf[0] = CNULL;

    return((retval = ((buf[0]) ? buf : (char *)NULL)) );
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
