/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/AL/getrealm.c,v $
 * $Author: steiner $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * routine to convert hostname into realm name.
 */

#ifndef	lint
static char rcsid_getrealm_c[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/lib/AL/getrealm.c,v 1.1 1988-04-25 14:11:00 steiner Exp $";
#endif	lint

#include <mit-copyright.h>
#include <strings.h>
#include <stdio.h>
#include <krb.h>
#include <sys/param.h>

/*
 * krb_getrealm.
 * Given a fully-qualified domain-style primary host name,
 * return the name of the Kerberos realm for the host.
 * If the hostname contains no discernable domain, or an error occurs,
 * return the local realm name, as supplied by get_krbrlm()
 *
 * The format of each line of the translation file is:
 * domain_name kerberos_realm
 * -or-
 * host_name kerberos_realm
 *
 * domain_name should be of the form .XXX.YYY (e.g. .LCS.MIT.EDU)
 * host names should be in the usual form (e.g. FOO.BAR.BAZ)
 */

static char ret_realm[REALM_SZ+1];

char *
krb_getrealm(host)
char *host;
{
	char *domain, *trans_idx;
	FILE *trans_file;
	char trans_host[MAXHOSTNAMELEN+1];
	char trans_realm[REALM_SZ+1];
	int retval;

	domain = index(host, '.');

	/* prepare default */
	if (domain) {
		strncpy(ret_realm, &domain[1], REALM_SZ);
		ret_realm[REALM_SZ] = '\0';
	} else {
		get_krbrlm(ret_realm, 1);
	}

	if ((trans_file = fopen(KRB_RLM_TRANS, "r")) == (FILE *) 0) {
		/* krb_errno = KRB_NO_TRANS */
		return(ret_realm);
	}
	while (1) {
		if ((retval = fscanf(trans_file, "%s %s",
				     trans_host, trans_realm)) != 2) {
			if (retval == EOF)
				return(ret_realm);
			continue;	/* ignore broken lines */
		}
		trans_host[MAXHOSTNAMELEN] = '\0';
		trans_realm[REALM_SZ] = '\0';
		if (!strcasecmp(trans_host, host)) {
			/* exact match of hostname, so return the realm */
			(void) strcpy(ret_realm, trans_realm);
			return(ret_realm);
		}
		if ((trans_host[0] = '.') && domain) { 
			/* this is a domain match */
			if (!strcasecmp(trans_host, domain)) {
				/* domain match, save for later */
				(void) strcpy(ret_realm, trans_realm);
				continue;
			}
		}
	}
}
