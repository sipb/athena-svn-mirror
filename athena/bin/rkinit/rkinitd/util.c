/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/util.c,v 1.1 1989-11-12 19:36:50 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/util.c,v $
 * $Author: qjb $
 *
 * This file contains general rkinit server utilities.
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/util.c,v 1.1 1989-11-12 19:36:50 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <rkinit.h>
#include <rkinit_err.h>
#include <rkinit_private.h>

static char errbuf[BUFSIZ];

void rpc_exchange_version_info();
void error();

int choose_version(version)
  int *version;
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
