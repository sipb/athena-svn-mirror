/*
 * ksrvtgt.c
 * 
 * Copyright 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Get a ticket-granting-ticket given a service key file (srvtab)
 * The lifetime is the shortest allowed [1 five-minute interval]
 */

#include <stdio.h>
#include <string.h>
#include "krb.h"

char *progname;

Usage()
{
	fprintf(stderr, "Usage: %s [-p] name instance [[realm] srvtab]\n",
		progname);
	exit(1);
}

main(argc,argv)
    int argc;
    char **argv;
{
    char realm[REALM_SZ + 1];
    register int code;
    char *srvtab = 0;
    int use_preauth = 0;

    progname = argv[0];

    memset(realm, 0, sizeof(realm));

    if (argc == 1) Usage();

    if (strcmp(argv[1],"-p") == 0) {
	use_preauth++;
	argv++; argc--;
    }

    if (argc < 3 || argc > 5) Usage();
    
    if (argc == 4)
	srvtab = argv[3];
    
    if (argc == 5) {
	(void) strncpy(realm, argv[3], sizeof(realm) - 1);
	srvtab = argv[4];
    }

    if (srvtab == 0 || srvtab[0] == 0)
	srvtab = KEYFILE;

    if (realm[0] == 0)
	if (krb_get_lrealm(realm, 1) != KSUCCESS)
	    (void) strcpy(realm, KRB_REALM);

    if (use_preauth)
	code = krb_get_svc_in_tkt_preauth(argv[1], argv[2], realm,
					  "krbtgt", realm, 1, srvtab);
    else
	code = krb_get_svc_in_tkt(argv[1], argv[2], realm,
				  "krbtgt", realm, 1, srvtab);
    if (code)
	fprintf(stderr, "%s\n", krb_get_err_text(code));
    exit(code);
}
