/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.2 $";
#endif

/*
 * Things related to running system commands.
 */

#include "defs.h"

/*
 * Set real and effective user ID.
 */
int setreuid(RealUID, EffectUID)
    uid_t			RealUID;
    uid_t			EffectUID;
{
    if (RealUID != (uid_t) -1)
	if (setuid(RealUID) < 0)
	    return(-1);

    if (EffectUID != (uid_t) -1)
	if (seteuid(EffectUID) < 0)
	    return(-1);

    return(0);
}
