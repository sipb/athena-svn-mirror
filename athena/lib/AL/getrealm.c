/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/AL/getrealm.c,v $
 * $Author: cfields $
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
"$Id: getrealm.c,v 4.9 1996-07-05 01:08:36 cfields Exp $";
#endif	/* lint */

#include <mit-copyright.h>
#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <krb.h>
#include <sys/param.h>

/* for Ultrix and friends ... */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

/*
 * krb_realmofhost.
 * Given a fully-qualified domain-style primary host name,
 * return the name of the Kerberos realm for the host.
 * If the hostname contains no discernable domain, or an error occurs,
 * return the local realm name, as supplied by get_krbrlm().
 * If the hostname contains a domain, but no translation is found,
 * the hostname's domain is converted to upper-case and returned.
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
int krb_ReverseResolve=1;

char *
krb_realmofhost(host)
char *host;
{
	char *domain;
	FILE *trans_file;
	char trans_host[MAXHOSTNAMELEN+1];
	char trans_realm[REALM_SZ+1];
	int retval;
	struct hostent *h;

	/* reverse-resolve the address */
	if (krb_ReverseResolve) {
	  if ((h=gethostbyname(host)) != (struct hostent *)0 ) {
	    if ((h=gethostbyaddr(h->h_addr, h->h_length, h->h_addrtype))
		!= (struct hostent *)0)
	      host = h->h_name;
	  }
	}

	domain = index(host, '.');

	/* prepare default */
	if (domain) {
		char *cp;

		strncpy(ret_realm, &domain[1], REALM_SZ);
		ret_realm[REALM_SZ] = '\0';
		/* Upper-case realm */
		for (cp = ret_realm; *cp; cp++)
			if (islower(*cp))
				*cp = toupper(*cp);
	} else {
		krb_get_lrealm(ret_realm, 1);
	}

#ifdef ATHENA_CONF_FALLBACK
	if (((trans_file = fopen(KRB_RLM_TRANS, "r")) == (FILE *) 0) &&
	    ((trans_file = fopen(KRB_FB_RLM_TRANS, "r")) == (FILE *) 0)) {
#else
	if ((trans_file = fopen(KRB_RLM_TRANS, "r")) == (FILE *) 0) {
#endif
		/* krb_errno = KRB_NO_TRANS */
		return(ret_realm);
	}
	while (1) {
		if ((retval = fscanf(trans_file, "%s %s",
				     trans_host, trans_realm)) != 2) {
			if (retval == EOF) {
				fclose(trans_file);
				return(ret_realm);
			}
			continue;	/* ignore broken lines */
		}
		trans_host[MAXHOSTNAMELEN] = '\0';
		trans_realm[REALM_SZ] = '\0';
		if (!strcasecmp(trans_host, host)) {
			/* exact match of hostname, so return the realm */
			(void) strcpy(ret_realm, trans_realm);
			fclose(trans_file);
			return(ret_realm);
		}
		if ((trans_host[0] == '.') && domain) { 
			/* this is a domain match */
			if (!strcasecmp(trans_host, domain)) {
				/* domain match, save for later */
				(void) strcpy(ret_realm, trans_realm);
				continue;
			}
		}
	}
}
