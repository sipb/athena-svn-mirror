/* Copyright 1989,1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file contains general rkinit server utilities. */

static const char rcsid[] = "$Id: util.c,v 1.2 1999-12-09 22:24:01 danw Exp $";

#include <stdio.h>
#include <rkinit.h>
#include <rkinit_err.h>

#include "rkinitd.h"

static char errbuf[BUFSIZ];

int choose_version(int *version)
{
    int c_lversion;		/* lowest version number client supports */
    int c_hversion;		/* highest version number client supports */
    int status = RKINIT_SUCCESS;

    rpc_exchange_version_info(&c_lversion, &c_hversion,
				  RKINIT_LVERSION, RKINIT_HVERSION);

    *version = min(RKINIT_HVERSION, c_hversion);
    if (*version < max(RKINIT_LVERSION, c_lversion)) {
	sprintf(errbuf,
		"Can't run version %d client against version %d server.",
		c_hversion, RKINIT_HVERSION);
	rkinit_errmsg(errbuf);
	return(RKINIT_VERSION);
    }

    return(status);
}
