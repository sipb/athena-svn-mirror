#ifndef lint
/* @(#)mountxdr.c	2.1 86/04/14 NFSSRC */
static	char sccsid[] = "@(#)mountxdr.c 1.1 86/02/05 Copyr 1984 Sun Micro";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <rpc/rpc.h>

bool_t
xdr_path(xdrs, pathp)
	XDR *xdrs;
	char **pathp;
{
	if (xdr_string(xdrs, pathp, 1024)) {
		return(TRUE);
	}
	return(FALSE);
}
