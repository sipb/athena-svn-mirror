/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Current Time information
 */

#include <time.h>
#include "defs.h"

/*
 * Get Current Time
 */
extern int GetCurrentTime(Query)
     MCSIquery_t	      *Query;
{
    char		       *Str = NULL;
    time_t			Time = 0;

    /*
     * No special PSI is needed for this one.
     */
    if (Query->Op == MCSIOP_CREATE) {
	if (time(&Time))
	    if (Str = TimeToStr(Time, NULL)) {
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
