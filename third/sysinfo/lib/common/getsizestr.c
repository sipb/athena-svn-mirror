/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

#include "defs.h"

/*
 * Get a nice size string
 */
extern char *GetSizeStr(Amt, Unit)
    Large_t			Amt;
    Large_t			Unit;
{
    static char			Buff[64];

    Buff[0] = CNULL;

    if (Unit) {
	if (Unit == GBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f GB", (float) Amt);
	else if (Unit == MBYTES) {
	    if (Amt > KBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.1f GB",
				(float) mbytes_to_gbytes(Amt));
	    else
		(void) snprintf(Buff, sizeof(Buff), "%.0f MB",
				(float) Amt);
	} else if (Unit == KBYTES) {
	    if (Amt > MBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.1f GB",
				(float) kbytes_to_gbytes(Amt));
	    else if (Amt > KBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.0f MB",
				(float) kbytes_to_mbytes(Amt));
	    else
		(void) snprintf(Buff, sizeof(Buff), "%.0f KB",
				(float) Amt);
	}
    }

    if (Buff[0] == CNULL) {
	if (Amt < KBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f Bytes", (float) Amt);
	else if (Amt < MBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f KB",
			    (float) bytes_to_kbytes(Amt));
	else if (Amt < GBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f MB",
			    (float) bytes_to_mbytes(Amt));
	else
	    (void) snprintf(Buff, sizeof(Buff), "%.1f GB",
			    (float) bytes_to_gbytes(Amt));
    }

    return(Buff);
}
