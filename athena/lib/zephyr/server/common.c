/* This file is part of the Project Athena Zephyr Notification System.
 * It contains functions for general use within the Zephyr server.
 *
 *	Created by:	John T. Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/common.c,v $
 *	$Author: raeburn $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#include <zephyr/mit-copyright.h>

#ifndef lint
#ifndef SABER
static char rcsid_common_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/common.c,v 1.6 1988-10-19 23:06:35 raeburn Exp $";
#endif SABER
#endif lint

#include <stdio.h>
#include <zephyr/zsyslog.h>
#include <strings.h>

extern char *malloc();

/* common routines for the server */

/* copy the string into newly allocated area */

char *
strsave(sp)
char *sp;
{
    register char *ret;

    if((ret = malloc((unsigned) strlen(sp)+1)) == NULL) {
	    syslog(LOG_ERR, "no mem strsave'ing");
	    abort();
    }
    (void) strcpy(ret,sp);
    return(ret);
}
