/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/common.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/common.c,v 1.1 1987-03-18 17:47:50 jtkohl Exp $
 */

#ifndef lint
static char *rcsid_common_c = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/common.c,v 1.1 1987-03-18 17:47:50 jtkohl Exp $";
#endif lint

/* common routines */

#include <zephyr/mit-copyright.h>
#include <stdio.h>

char *
strsave(sp) char *sp;
{
    register char *ret;

    if((ret = (char *) malloc(strlen(sp)+1)) == NULL) {
	error("out of memory in strsave()\n");
	return(NULL);
    }
    strcpy(ret,sp);
    return(ret);
}

