/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: setreuid.c,v 1.1.1.1 1996-10-07 20:16:52 ghudson Exp $";
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
