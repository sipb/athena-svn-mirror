/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * RunTime functions
 */

#include "defs.h"

/*
 * Check to see if we're running with the right Root Access.
 * This should be in ../sysinfo and not in the libmcsysinfo, but we need to 
 * know about RA_LEVEL which is OS specific.
 */
extern void CheckRunTimeAccess(MsgLevel)
     int			MsgLevel;
{
    if (VL_TERSE)
	return;

#if	defined(RA_LEVEL) && RA_LEVEL == RA_ADVISED
    if (geteuid() != 0) {
	SImsg(SIM_WARN, 
"This program should be run as `root' (uid=0) to obtain\n\t\
all supported information.  Please either make setuid to\n\t\
root (chown root sysinfo;chmod u+s sysinfo) or invoke as a\n\t\
uid=0 user such as `root'.");
    }
#endif	/* RA_ADVISED */
#if	defined(RA_LEVEL) && RA_LEVEL == RA_NONE
    /* Do nothing */
    return;
#endif	/* RA_NONE */
}
