/*
 * clients/kinit/kinit.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Initialize a credentials cache.
 */

#include "k5-int.h"
#include "com_err.h"
#include "adm_proto.h"
#ifdef KRB5_KRB4_COMPAT
#include <kerberosIV/krb.h>
#include <kerberosIV/krb4-proto.h>
#endif

#include <stdio.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#define KRB5_DEFAULT_OPTIONS (KDC_OPT_PROXIABLE|KDC_OPT_FORWARDABLE)
#define KRB5_DEFAULT_LIFE 60*60*10 /* 10 hours */

extern int optind;
extern char *optarg;

krb5_data tgtname = {
    0,
    KRB5_TGS_NAME_SIZE,
    KRB5_TGS_NAME
};

/*
 * Try no preauthentication first; then try the encrypted timestamp
 */
krb5_preauthtype * preauth = NULL;
krb5_preauthtype preauth_list[2] = { 0, -1 };

char *progname;

void
main(argc, argv)
    int argc;
    char **argv;
{
    krb5_context kcontext;
    krb5_ccache ccache = NULL;
    char *cache_name = NULL;		/* -f option */
    char *keytab_name = NULL;		/* -t option */
    char *service_name = NULL;		/* -s option */
    krb5_deltat lifetime = KRB5_DEFAULT_LIFE;	/* -l option */
    krb5_timestamp starttime = 0;
    krb5_deltat rlife = 0;
    int options = KRB5_DEFAULT_OPTIONS;
    int option;
    int errflg = 0;
    krb5_error_code code;
    krb5_principal me;
    krb5_principal server;
    krb5_creds my_creds;
    krb5_timestamp now;
    krb5_address *null_addr = (krb5_address *)0;
    krb5_address **addrs = (krb5_address **)0;
    int use_keytab = 0;			/* -k option */
    krb5_keytab keytab = NULL;
    struct passwd *pw = 0;
    int pwsize;
    char password[255], *client_name, prompt[255];
    int got_krb4 = 0;

    code = krb5_init_context(&kcontext);
    if (code) {
	    com_err(argv[0], code, "while initializing krb5");
	    exit(1);
    }

    if ((code = krb5_timeofday(kcontext, &now))) {
	com_err(argv[0], code, "while getting time of day");
	exit(1);
    }

    if (strrchr(argv[0], '/'))
	argv[0] = strrchr(argv[0], '/')+1;
    progname = argv[0];

    while ((option = getopt(argc, argv, "r:Rfpl:s:c:kt:vS:")) != EOF) {
	switch (option) {
	case 'r':
	    options |= KDC_OPT_RENEWABLE;
	    code = krb5_string_to_deltat(optarg, &rlife);
	    if (code != 0 || rlife == 0) {
		fprintf(stderr, "Bad lifetime value %s\n", optarg);
		errflg++;
	    }
	    break;
	case 'R':
	    /* renew the ticket */
	    options |= KDC_OPT_RENEW;
	    break;
	case 'v':
	    /* validate the ticket */
	    options |= KDC_OPT_VALIDATE;
	    break;
        case 'S':
	    service_name = optarg;
	    break;
	case 'p':
	    options |= KDC_OPT_PROXIABLE;
	    break;
	case 'f':
	    options |= KDC_OPT_FORWARDABLE;
	    break;
#ifndef NO_KEYTAB
       case 'k':
	    use_keytab = 1;
	    break;
       case 't':
	    if (keytab == NULL) {
		 keytab_name = optarg;

		 code = krb5_kt_resolve(kcontext, keytab_name, &keytab);
		 if (code != 0) {
		      com_err(argv[0], code, "resolving keytab %s",
			      keytab_name);
		 errflg++;
		 }
	    } else {
		 fprintf(stderr, "Only one -t option allowed.\n");
		 errflg++;
	    }
	    break;
#endif
       case 'l':
	    code = krb5_string_to_deltat(optarg, &lifetime);
	    if (code != 0 || lifetime == 0) {
		fprintf(stderr, "Bad lifetime value %s\n", optarg);
		errflg++;
	    }
	    break;
       case 's':
	    code = krb5_string_to_timestamp(optarg, &starttime);
	    if (code != 0 || starttime == 0) {
	      krb5_deltat ktmp;
	      code = krb5_string_to_deltat(optarg, &ktmp);
	      if (code == 0 && ktmp != 0) {
		starttime = now + ktmp;
		options |= KDC_OPT_POSTDATED;
	      } else {
		fprintf(stderr, "Bad postdate start time value %s\n", optarg);
		errflg++;
	      }
	    } else {
	      options |= KDC_OPT_POSTDATED;
	    }
	    break;
       case 'c':
	    if (ccache == NULL) {
		cache_name = optarg;
		
		code = krb5_cc_resolve (kcontext, cache_name, &ccache);
		if (code != 0) {
		    com_err (argv[0], code, "resolving ccache %s",
			     cache_name);
		    errflg++;
		}
	    } else {
		fprintf(stderr, "Only one -c option allowed\n");
		errflg++;
	    }
	    break;
	case '?':
	default:
	    errflg++;
	    break;
	}
    }

    if (argc - optind > 1) {
	fprintf(stderr, "Extra arguments (starting with \"%s\").\n",
		argv[optind+1]);
	errflg++;
    }

    if (errflg) {
	fprintf(stderr, "Usage: %s [-r time] [-R] [-s time] [-v] [-puf] [-l lifetime] [-c cachename] [-k] [-t keytab] [-S target_service] [principal]\n", argv[0]);
	exit(2);
    }

    if (ccache == NULL) {
	 if ((code = krb5_cc_default(kcontext, &ccache))) {
	      com_err(argv[0], code, "while getting default ccache");
	      exit(1);
	 }
    }

    if (optind != argc-1) {       /* No principal name specified */
#ifndef NO_KEYTAB
	 if (use_keytab) {
	      /* Use the default host/service name */
	      code = krb5_sname_to_principal(kcontext, NULL, NULL,
					     KRB5_NT_SRV_HST, &me);
	      if (code) {
		   com_err(argv[0], code,
			   "when creating default server principal name");
		   exit(1);
	      }
	 } else
#endif
	 {
	      /* Get default principal from cache if one exists */
	      code = krb5_cc_get_principal(kcontext, ccache, &me);
	      if (code) {
#ifdef HAVE_PWD_H
		   /* Else search passwd file for client */
		   pw = getpwuid((int) getuid());
		   if (pw) {
			if ((code = krb5_parse_name(kcontext,pw->pw_name,
						    &me))) {
			     com_err (argv[0], code, "when parsing name %s",
				      pw->pw_name);
			     exit(1);
			}
		   } else {
			fprintf(stderr, 
			"Unable to identify user from password file\n");
			exit(1);
		   }
#else /* HAVE_PWD_H */
		   fprintf(stderr, "Unable to identify user\n");
		   exit(1);
#endif /* HAVE_PWD_H */
	      }
	 }
    } /* Use specified name */	 
    else if ((code = krb5_parse_name (kcontext, argv[optind], &me))) {
	 com_err (argv[0], code, "when parsing name %s",argv[optind]);
	 exit(1);
    }
    
    if ((code = krb5_unparse_name(kcontext, me, &client_name))) {
	com_err (argv[0], code, "when unparsing name");
	exit(1);
    }

    memset((char *)&my_creds, 0, sizeof(my_creds));
    
    my_creds.client = me;

    if (service_name == NULL) {
	 if((code = krb5_build_principal_ext(kcontext, &server,
					     krb5_princ_realm(kcontext, me)->length,
					     krb5_princ_realm(kcontext, me)->data,
					     tgtname.length, tgtname.data,
					     krb5_princ_realm(kcontext, me)->length,
					     krb5_princ_realm(kcontext, me)->data,
					     0))) {
	      com_err(argv[0], code, "while building server name");
	      exit(1);
	 }
    } else {
	 if (code = krb5_parse_name(kcontext, service_name, &server)) {
	      com_err(argv[0], code, "while parsing service name %s",
		      service_name);
	      exit(1);
	 }
    }
	
    my_creds.server = server;

    if (options & KDC_OPT_POSTDATED) {
      my_creds.times.starttime = starttime;
      my_creds.times.endtime = starttime + lifetime;
    } else {
      my_creds.times.starttime = 0;	/* start timer when request
					   gets to KDC */
      my_creds.times.endtime = now + lifetime;
    }
    if (options & KDC_OPT_RENEWABLE) {
	my_creds.times.renew_till = now + rlife;
    } else
	my_creds.times.renew_till = 0;

    if (options & KDC_OPT_VALIDATE) {
        /* don't use get_in_tkt, just use mk_req... */
        krb5_data outbuf;

        code = krb5_validate_tgt(kcontext, ccache, server, &outbuf);
	if (code) {
	  com_err (argv[0], code, "validating tgt");
	  exit(1);
	}
	/* should be done... */
	exit(0);
    }

    if (options & KDC_OPT_RENEW) {
        /* don't use get_in_tkt, just use mk_req... */
        krb5_data outbuf;

        code = krb5_renew_tgt(kcontext, ccache, server, &outbuf);
	if (code) {
	  com_err (argv[0], code, "renewing tgt");
	  exit(1);
	}
	/* should be done... */
	exit(0);
    }
#ifndef NO_KEYTAB
    if (!use_keytab)
#endif
    {
	 (void) sprintf(prompt,"Password for %s: ", (char *) client_name);

	 pwsize = sizeof(password);

	 code = krb5_read_password(kcontext, prompt, 0, password, &pwsize);
	 if (code || pwsize == 0) {
	      fprintf(stderr, "Error while reading password for '%s'\n",
		      client_name);
	      memset(password, 0, sizeof(password));
	      exit(1);
	 }

	 code = krb5_get_in_tkt_with_password(kcontext, options, addrs,
					      NULL, preauth, password, 0,
					      &my_creds, 0);
#ifdef KRB5_KRB4_COMPAT
	 got_krb4 = try_krb4(kcontext, me, password, lifetime);
#endif
	 memset(password, 0, sizeof(password));
#ifndef NO_KEYTAB
    } else {
	 code = krb5_get_in_tkt_with_keytab(kcontext, options, addrs,
					    NULL, preauth, keytab, 0,
					    &my_creds, 0);
#endif
    }
    
    if (code) {
	if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
	    fprintf (stderr, "%s: Password incorrect\n", argv[0]);
	else
	    com_err (argv[0], code, "while getting initial credentials");
	exit(1);
    }

    code = krb5_cc_initialize (kcontext, ccache, me);
    if (code != 0) {
	com_err (argv[0], code, "when initializing cache %s",
		 cache_name?cache_name:"");
	exit(1);
    }

    code = krb5_cc_store_cred(kcontext, ccache, &my_creds);
    if (code) {
	com_err (argv[0], code, "while storing credentials");
	exit(1);
    }

#ifdef KRB5_KRB4_COMPAT
    if (!got_krb4)
	try_convert524(kcontext, ccache);
#endif

    /* my_creds is pointing at server */
    krb5_free_principal(kcontext, server);

    krb5_free_context(kcontext);
    
    exit(0);
}

#define VALIDATE 0
#define RENEW 1

/* stripped down version of krb5_mk_req */
krb5_error_code krb5_validate_tgt(context, ccache, server, outbuf)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal	  server; /* tgtname */
     krb5_data *outbuf;
{
	return krb5_tgt_gen(context, ccache, server, outbuf, VALIDATE);
}

/* stripped down version of krb5_mk_req */
krb5_error_code krb5_renew_tgt(context, ccache, server, outbuf)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal	  server; /* tgtname */
     krb5_data *outbuf;
{
	return krb5_tgt_gen(context, ccache, server, outbuf, RENEW);
}


/* stripped down version of krb5_mk_req */
krb5_error_code krb5_tgt_gen(context, ccache, server, outbuf, opt)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal	  server; /* tgtname */
     krb5_data *outbuf;
     int opt;
{
    krb5_auth_context   * auth_context = 0;
    const krb5_flags      ap_req_options;
    krb5_data           * in_data;

    krb5_error_code 	  retval;
    krb5_creds 		* credsp;
    krb5_creds 		  creds;

    /* obtain ticket & session key */
    memset((char *)&creds, 0, sizeof(creds));
    if ((retval = krb5_copy_principal(context, server, &creds.server)))
	goto cleanup;

    if ((retval = krb5_cc_get_principal(context, ccache, &creds.client)))
	goto cleanup_creds;

    if(opt == VALIDATE) {
	    if ((retval = krb5_get_credentials_validate(context, 0,
							ccache, &creds, &credsp)))
		    goto cleanup_creds;
    } else {
	    if ((retval = krb5_get_credentials_renew(context, 0,
							ccache, &creds, &credsp)))
		    goto cleanup_creds;
    }

    /* we don't actually need to do the mk_req, just get the creds. */
cleanup_creds:
    krb5_free_cred_contents(context, &creds);

cleanup:

    return retval;
}


#ifdef KRB5_KRB4_COMPAT
int try_krb4(kcontext, me, password, lifetime)
    krb5_context kcontext;
    krb5_principal me;
    char *password;
    krb5_deltat lifetime;
{
    krb5_error_code code;
    int krbval, kpass_ok = 0;
    char v4name[ANAME_SZ], v4inst[INST_SZ], v4realm[REALM_SZ], v4key[8];
    int v4life;

    /* Translate to a Kerberos 4 principal. */
    code = krb5_524_conv_principal(kcontext, me, v4name, v4inst, v4realm);
    if (code)
	return(code);

    v4life = lifetime / (5 * 60);
    if (v4life < 1)
	v4life = 1;
    if (v4life > 255)
	v4life = 255;

    krbval = krb_get_pw_in_tkt(v4name, v4inst, v4realm, "krbtgt", v4realm,
			       v4life, password);

    if (krbval != INTK_OK) {
	fprintf(stderr, "Kerberos 4 error: %s\n",
		krb_get_err_text(krbval));
	return 0;
    }
    return 1;
}

/* Convert krb5 tickets to krb4. */
int try_convert524(kcontext, ccache)
     krb5_context kcontext;
     krb5_ccache ccache;
{
    krb5_principal me, kpcserver;
    krb5_error_code kpccode;
    int kpcval;
    krb5_creds increds, *v5creds;
    CREDENTIALS v4creds;

    /* or do this directly with krb524_convert_creds_kdc */
    krb524_init_ets(kcontext);

    if ((kpccode = krb5_cc_get_principal(kcontext, ccache, &me))) {
	com_err(progname, kpccode, "while getting principal name");
	return 0;
    }

    /* cc->ccache, already set up */
    /* client->me, already set up */
    if ((kpccode = krb5_build_principal(kcontext,
				        &kpcserver, 
				        krb5_princ_realm(kcontext, me)->length,
				        krb5_princ_realm(kcontext, me)->data,
				        "krbtgt",
				        krb5_princ_realm(kcontext, me)->data,
					NULL))) {
      com_err(progname, kpccode,
	      "while creating service principal name");
      return 0;
    }

    memset((char *) &increds, 0, sizeof(increds));
    increds.client = me;
    increds.server = kpcserver;
    increds.times.endtime = 0;
    increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
    if ((kpccode = krb5_get_credentials(kcontext, 0, 
					ccache,
					&increds, 
					&v5creds))) {
	com_err(progname, kpccode,
		"getting V5 credentials");
	return 0;
    }
    if ((kpccode = krb524_convert_creds_kdc(kcontext, 
					    v5creds,
					    &v4creds))) {
	com_err(progname, kpccode, 
		"converting to V4 credentials");
	return 0;
    }
    /* this is stolen from the v4 kinit */
    /* initialize ticket cache */
    if ((kpcval = in_tkt(v4creds.pname,v4creds.pinst)
	 != KSUCCESS)) {
	com_err(progname, kpcval,
		"trying to create the V4 ticket file");
	return 0;
    }
    /* stash ticket, session key, etc. for future use */
    if ((kpcval = krb_save_credentials(v4creds.service,
				       v4creds.instance,
				       v4creds.realm, 
				       v4creds.session,
				       v4creds.lifetime,
				       v4creds.kvno,
				       &(v4creds.ticket_st), 
				       v4creds.issue_date))) {
	com_err(progname, kpcval,
		"trying to save the V4 ticket");
	return 0;
    }
    return 1;
}
#endif /* KRB5_KRB4_COMPAT */
