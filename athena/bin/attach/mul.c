/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/mul.c,v 1.8 1997-09-23 18:40:16 danw Exp $
 *
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 */

static char *rcsid_mul_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/mul.c,v 1.8 1997-09-23 18:40:16 danw Exp $";


#include "attach.h"
#include <string.h>

int mul_attach(atp, mopt, errorout)
struct _attachtab *atp;
struct mntopts *mopt;
int errorout;
{
	char mul_buf[BUFSIZ], *cp = mul_buf, *mp;
	int status;
	
	strcpy(mul_buf, atp->hostdir);
	while (mp = cp) {
		cp = strchr(mp, ',');
		if (cp)
			*cp = '\0';
		status = attach(mp);
		if (status == FAILURE)
			break;
		if (cp)
			cp++;
	}

	error_status = (status == SUCCESS) ? ERR_NONE : ERR_SOMETHING;
	return status;
}

/*
 * Detach all of the components of the MUL filesystem in the reverse
 * order that they were attached. Currently, when detach -a is done,
 * the components of the MUL are detached before the MUL, so if the
 * MUL tries to detach them, detach will complain to the user that
 * they are not attached. This is silly, so we check to make sure that
 * the components are actually still attached before we try to detach
 * them. If they are already detached, we stay quiet. Meanwhile, if
 * we try to detach a component that was there when we started, but
 * detach returns that it isn't attached, we assume that another
 * detach was racing with us and don't return failure for the
 * operation.
 */
int mul_detach(atp)
struct _attachtab *atp;
{
    int status = SUCCESS;
    int tempexp;
    char mul_buf[BUFSIZ], *cp;
    extern struct _attachtab	*attachtab_last, *attachtab_first;
    struct _attachtab *atptmp;

    lock_attachtab();
    get_attachtab();
    unlock_attachtab();

    atptmp = attachtab_first;

    tempexp = explicit;
    mul_buf[0] = ',';
    strcpy(mul_buf + 1, atp->hostdir);

    while (cp = strrchr(mul_buf, ','))
      {
	*cp++ = '\0';
	explicit = 0;
	attachtab_first = atptmp;

	if (attachtab_lookup(cp))
	  {
	    attachtab_first = NULL;
	    if (detach(cp) != SUCCESS &&
		error_status != ERR_DETACHNOTATTACHED)
	      status = FAILURE;
	  }
      }

    attachtab_first = atptmp;
    free_attachtab();
    explicit = tempexp;
    error_status = (status == SUCCESS) ? ERR_NONE : ERR_SOMETHING;
    return status;
}
