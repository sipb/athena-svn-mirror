/*
 * kdestroy.c
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * This program causes Kerberos tickets to be destroyed.
 * Options are: 
 *
 *   -q[uiet]	- no bell even if tickets not destroyed
 *   -f[orce]	- no message printed at all 
 */

#include "mit-copyright.h"

#include <stdio.h>
#include "krb.h"
#include <string.h>

#include <krb5.h>

static char *pname;

static usage()
{
    fprintf(stderr, "Usage: %s [-f] [-q]\n", pname);
    exit(1);
}

krb5_error_code do_v5_kdestroy(cachename)
	char	*cachename;
{
	krb5_context context;
	krb5_error_code retval;
	krb5_ccache cache;

	retval = krb5_init_context(&context);
	if (retval)
		return retval;

	if (!cachename)
		cachename = krb5_cc_default_name(context);

	krb5_init_ets(context);

	retval = krb5_cc_resolve (context, cachename, &cache);
	if (retval) {
		krb5_free_context(context);
		return retval;
	}

	retval = krb5_cc_destroy(context, cache);

	krb5_free_context(context);
	return retval;
}

int main(argc, argv)
    int     argc;
    char   *argv[];
{
    int     fflag=0, qflag=0, k_errno, k5_errno, retval=0;
    register char *cp;

    cp = strrchr (argv[0], '/');
    if (cp == NULL)
	pname = argv[0];
    else
	pname = cp+1;

    if (argc > 2)
	usage();
    else if (argc == 2) {
	if (!strcmp(argv[1], "-f"))
	    ++fflag;
	else if (!strcmp(argv[1], "-q"))
	    ++qflag;
	else usage();
    }

    k_errno = dest_tkt();
    k5_errno = do_v5_kdestroy(0);

    if (k_errno != 0 && k_errno != RET_TKFIL) {
	retval = 1;
	if (!fflag) fprintf(stderr, "V4 tickets NOT destroyed.\n");
    }
    if (k5_errno != 0 && k5_errno != KRB5_FCC_NOFILE) {
	retval = 1;
	if (!fflag) fprintf(stderr, "V5 credentials NOT destroyed.\n");
    }
    if (fflag) return retval;

    if (retval) {
	if (!qflag) fprintf(stderr, "\007");
    } else {
	if (k_errno && k5_errno) fprintf(stderr, "No tickets to destroy.\n");
	else fprintf(stderr, "Tickets destroyed.\n");
    }
    return retval;
}
