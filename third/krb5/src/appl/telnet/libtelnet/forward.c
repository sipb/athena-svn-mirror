/*
 * appl/telnet/libtelnet/forward.c
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


/* General-purpose forwarding routines. These routines may be put into */
/* libkrb5.a to allow widespread use */ 

#if defined(KERBEROS) || defined(KRB5)
#include <stdio.h>
#include <pwd.h>
#include <netdb.h>
#include <krb.h>
 
#include "k5-int.h"
 
extern char *line;		/* see sys_term.c */
extern char *UserNameRequested;
extern int k5_haveauth;

/* Decode, decrypt and store the forwarded creds in the local ccache. */
krb5_error_code
rd_and_store_for_creds(context, auth_context, inbuf, ticket, lusername)
    krb5_context context;
    krb5_auth_context auth_context;
    krb5_data *inbuf;
    krb5_ticket *ticket;
    char *lusername;
{
    krb5_creds **creds;
    krb5_error_code retval;
    char ccname[35];
    krb5_ccache ccache = NULL;
    struct passwd *pwd;
    char *tty;

    k5_haveauth = 0;

    if (!UserNameRequested)
	return -1;

    if (retval = krb5_rd_cred(context, auth_context, inbuf, &creds, NULL)) 
	return(retval);

    sprintf(ccname, "FILE:/tmp/krb5cc_p%d", getpid());
    unlink(ccname + 5);
    setenv(KRB5_ENV_CCNAME, ccname, 1);

    if (retval = krb5_cc_resolve(context, ccname, &ccache))
	goto cleanup;

    if (retval = krb5_cc_initialize(context, ccache, ticket->enc_part2->client))
	goto cleanup;

    if (retval = krb5_cc_store_cred(context, ccache, *creds)) 
	goto cleanup;

    tty = strrchr(line, '/') + 1;
    if (!tty)
	tty = line;
    sprintf(ccname, "/tmp/tkt_%s", tty);
    unlink(ccname);
    setenv("KRBTKFILE", ccname, 1);
    if (retval = try_convert524(context, ccache))
	goto cleanup;

    k5_haveauth = 1;

cleanup:
    krb5_free_creds(context, *creds);
    if (ccache && !k5_haveauth)
	krb5_cc_destroy(context, ccache);
    return retval;
}

int try_convert524(kcontext, ccache)
     krb5_context kcontext;
     krb5_ccache ccache;
{
    krb5_principal me, kpcserver;
    krb5_error_code kpccode;
    int kpcval;
    krb5_creds increds, *v5creds;
    CREDENTIALS v4creds;
    char tkname[35];

    krb524_init_ets(kcontext);

    if ((kpccode = krb5_cc_get_principal(kcontext, ccache, &me)))
	return kpccode;

    /* cc->ccache, already set up */
    /* client->me, already set up */
    if ((kpccode = krb5_build_principal(kcontext,
				        &kpcserver, 
				        krb5_princ_realm(kcontext, me)->length,
				        krb5_princ_realm(kcontext, me)->data,
				        "krbtgt",
				        krb5_princ_realm(kcontext, me)->data,
					NULL)))
      return kpccode;

    memset((char *) &increds, 0, sizeof(increds));
    increds.client = me;
    increds.server = kpcserver;
    increds.times.endtime = 0;
    increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
    if ((kpccode = krb5_get_credentials(kcontext, 0, 
					ccache,
					&increds, 
					&v5creds)))
	return kpccode;
    if ((kpccode = krb524_convert_creds_kdc(kcontext, 
					    v5creds,
					    &v4creds)))
	return kpccode;
    /* this is stolen from the v4 kinit */
    /* initialize ticket cache */
    if ((kpcval = in_tkt(v4creds.pname,v4creds.pinst)
	 != KSUCCESS))
	return kpcval;
    /* stash ticket, session key, etc. for future use */
    if ((kpcval = krb_save_credentials(v4creds.service,
				       v4creds.instance,
				       v4creds.realm, 
				       v4creds.session,
				       v4creds.lifetime,
				       v4creds.kvno,
				       &(v4creds.ticket_st), 
				       v4creds.issue_date)))
	return kpcval;

    return 0;
}

#endif /* defined(KRB5) && defined(FORWARD) */
