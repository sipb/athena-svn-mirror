/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/mul.c,v 1.5 1994-03-25 15:58:31 miki Exp $
 *
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 */

static char *rcsid_mul_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/mul.c,v 1.5 1994-03-25 15:58:31 miki Exp $";


#include "attach.h"
#include <string.h>

int mul_attach(atp, mopt, errorout)
struct _attachtab *atp;
struct mntopts *mopt;
int errorout;
{
	char mul_buf[BUFSIZ], *cp = mul_buf, *mp;
	
	strcpy(mul_buf, atp->hostdir);
	while (mp = cp) {
		cp = strchr(mp, ',');
		if (cp)
			*cp = '\0';
		attach(mp);
		if (cp)
			cp++;
	}
	return SUCCESS;
}

int mul_detach(atp)
struct _attachtab *atp;
{
    int status = SUCCESS;
    char mul_buf[BUFSIZ], *cp;
    
    strcpy(mul_buf, atp->hostdir);
    cp = &mul_buf[strlen(mul_buf)];
    while (cp-- >= mul_buf) {
	explicit = 0;
	if (cp < mul_buf || *cp == ',') {
	    if (attachtab_lookup(cp+1)
		&& detach(cp+1) != SUCCESS
		&& error_status != ERR_DETACHNOTATTACHED)
		status = FAILURE;
	    if (cp >= mul_buf)
		*cp = '\0';
	}
    }

    error_status = (status == SUCCESS) ? ERR_NONE : ERR_SOMETHING;
    return status;
}
