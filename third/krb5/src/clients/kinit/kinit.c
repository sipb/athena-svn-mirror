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
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Initialize a credentials cache.
 */

#include <krb5.h>
#ifdef KRB5_KRB4_COMPAT
#include <kerberosIV/krb.h>
#include <kerberosIV/krb4-proto.h>
#endif
#include <string.h>
#include <stdio.h>

#ifdef GETOPT_LONG
#include "getopt.h"
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#ifdef sun
/* SunOS4 unistd didn't declare these; okay to make unconditional?  */
extern int optind;
extern char *optarg;
#endif /* sun */
#else
extern int optind;
extern char *optarg;
extern int getopt();
#endif
#endif
#include "com_err.h"

#ifdef HAVE_PWD_H
#include <pwd.h>

void get_name_from_passwd_file(program_name, kcontext, me)
    char * program_name;
    krb5_context kcontext;
    krb5_principal * me;
{
    struct passwd *pw;
    krb5_error_code code;
    if (pw = getpwuid((int) getuid())) {
	if ((code = krb5_parse_name(kcontext, pw->pw_name, me))) {
	    com_err (program_name, code, "when parsing name %s", pw->pw_name);
	    exit(1);
	}
    } else {
	fprintf(stderr, "Unable to identify user from password file\n");
	exit(1);
    }
}
#else /* HAVE_PWD_H */
void get_name_from_passwd_file(kcontext, me)
    krb5_context kcontext;
    krb5_principal * me;
{
    fprintf(stderr, "Unable to identify user\n");
    exit(1);
}
#endif /* HAVE_PWD_H */

#ifdef GETOPT_LONG
/* if struct[2] == NULL, then long_getopt acts as if the short flag
   struct[3] was specified.  If struct[2] != NULL, then struct[3] is
   stored in *(struct[2]), the array index which was specified is
   stored in *index, and long_getopt() returns 0. */

struct option long_options[] = {
    { "noforwardable", 0, NULL, 'f'+0200 },
    { "noproxiable", 0, NULL, 'p'+0200 },
    { "addresses", 0, NULL, 'A'+0200},
    { "forwardable", 0, NULL, 'f' },
    { "proxiable", 0, NULL, 'p' },
    { "noaddresses", 0, NULL, 'A'},
    { "version", 0, NULL, 0x01 },
    { NULL, 0, NULL, 0 }
};
#endif

char *progname;

int
main(argc, argv)
    int argc;
    char **argv;
{
    krb5_context kcontext;
    krb5_principal me = NULL;
    krb5_deltat start_time = 0;
    krb5_address **addresses = NULL;
    krb5_get_init_creds_opt opts;
    char *service_name = NULL;
    krb5_keytab keytab = NULL;
    char *cache_name = NULL;
    krb5_ccache ccache = NULL;
    enum { INIT_PW, INIT_KT, RENEW, VALIDATE} action;
    int errflg = 0, idx, i;
    krb5_creds my_creds;
    krb5_error_code code;
    int got_krb4 = 0;

    /* Ensure we can be driven from a pipe */
    if(!isatty(fileno(stdin)))
        setvbuf(stdin, 0, _IONBF, 0);
    if(!isatty(fileno(stdout)))
        setvbuf(stdout, 0, _IONBF, 0);
    if(!isatty(fileno(stderr)))
        setvbuf(stderr, 0, _IONBF, 0);

    if (code = krb5_init_context(&kcontext)) {
	com_err(argv[0], code, "while initializing kerberos library");
	exit(1);
    }

    krb5_get_init_creds_opt_init(&opts);

    action = INIT_PW;

    if (strrchr(argv[0], '/'))
	argv[0] = strrchr(argv[0], '/')+1;
    progname = argv[0];

    while (
#ifdef GETOPT_LONG
	   (i = getopt_long(argc, argv, "r:fpAl:s:c:kt:RS:v",
			    long_options, &idx)) != -1
#else
	   (i = getopt(argc, argv, "r:fpAl:s:c:kt:RS:v")) != -1
#endif
	   ) {
	switch (i) {
#ifdef GETOPT_LONG
	case 1: /* Print the version */
	    printf("%s\n", krb5_version);
	    exit(0);
#endif
        case 'l':
	    {
		krb5_deltat lifetime;
		code = krb5_string_to_deltat(optarg, &lifetime);
		if (code != 0 || lifetime == 0) {
		    fprintf(stderr, "Bad lifetime value %s\n", optarg);
		    errflg++;
		}
		krb5_get_init_creds_opt_set_tkt_life(&opts, lifetime);
	    }
	    break;
	case 'r':
	    {
		krb5_deltat rlife;

		code = krb5_string_to_deltat(optarg, &rlife);
		if (code != 0 || rlife == 0) {
		    fprintf(stderr, "Bad lifetime value %s\n", optarg);
		    errflg++;
		}
		krb5_get_init_creds_opt_set_renew_life(&opts, rlife);
	    }
	    break;
	case 'f':
	    krb5_get_init_creds_opt_set_forwardable(&opts, 1);
	    break;
#ifdef GETOPT_LONG
	case 'f'+0200:
	    krb5_get_init_creds_opt_set_forwardable(&opts, 0);
	    break;
#endif
	case 'p':
	    krb5_get_init_creds_opt_set_proxiable(&opts, 1);
	    break;
#ifdef GETOPT_LONG
	case 'p'+0200:
	    krb5_get_init_creds_opt_set_proxiable(&opts, 0);
	    break;
#endif
	case 'A':
	    krb5_get_init_creds_opt_set_address_list(&opts, NULL);
	    break;
#ifdef GETOPT_LONG
	case 'A'+0200:
	    krb5_os_localaddr(kcontext, &addresses);
	    krb5_get_init_creds_opt_set_address_list(&opts, addresses);
	    break;
#endif
       	case 's':
	    code = krb5_string_to_deltat(optarg, &start_time);
	    if (code != 0 || start_time == 0) {
		krb5_timestamp abs_starttime;
		krb5_timestamp now;

		code = krb5_string_to_timestamp(optarg, &abs_starttime);
		if (code != 0 || abs_starttime == 0) {
		    fprintf(stderr, "Bad start time value %s\n", optarg);
		    errflg++;
		} else {
		    if ((code = krb5_timeofday(kcontext, &now))) {
			com_err(argv[0], code,
				"while getting time of day");
			exit(1);
		    }

		    start_time = abs_starttime - now;
		}
	    }
	    break;
        case 'S':
	    service_name = optarg;
	    break;
        case 'k':
	    action = INIT_KT;
	    break;
        case 't':
	    if (keytab == NULL) {
		 code = krb5_kt_resolve(kcontext, optarg, &keytab);
		 if (code != 0) {
		      com_err(argv[0], code, "resolving keytab %s", optarg);
		      errflg++;
		 }
	    } else {
		 fprintf(stderr, "Only one -t option allowed.\n");
		 errflg++;
	    }
	    break;
	case 'R':
	    action = RENEW;
	    break;
	case 'v':
	    action = VALIDATE;
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
#ifdef GETOPT_LONG
	fprintf(stderr, "Usage: %s [--version] [-l lifetime] [-r renewable_life] [-f | --forwardable | --noforwardable] [-p | --proxiable | --noproxiable] [-A | --noaddresses | --addresses] [-s start_time] [-S target_service] [-k [-t keytab_file]] [-R] [-v] [-c cachename] [principal]\n", argv[0]);
#else
	fprintf(stderr, "Usage: %s [-l lifetime] [-r renewable_life] [-f] [-p] [-A] [-s start_time] [-S target_service] [-k [-t keytab_file]] [-R] [-v] [-c cachename] [principal]\n", argv[0]);
#endif
	exit(2);
    }

    if (ccache == NULL) {
	 if ((code = krb5_cc_default(kcontext, &ccache))) {
	      com_err(argv[0], code, "while getting default ccache");
	      exit(1);
	 }
    }

    if (optind == argc-1) {
	/* Use specified name */
	if ((code = krb5_parse_name (kcontext, argv[optind], &me))) {
	    com_err (argv[0], code, "when parsing name %s",argv[optind]);
	    exit(1);
	}
    } else {
	/* No principal name specified */
	if (action == INIT_KT) {
	    /* Use the default host/service name */
	    if (code = krb5_sname_to_principal(kcontext, NULL, NULL,
					       KRB5_NT_SRV_HST, &me)) {
		com_err(argv[0], code,
			"when creating default server principal name");
		exit(1);
	    }
	} else {
	    /* Get default principal from cache if one exists */
	    if (code = krb5_cc_get_principal(kcontext, ccache, &me))
		get_name_from_passwd_file(argv[0], kcontext, &me);
	}
    }
    
    switch (action) {
    case INIT_PW:
	code = krb5_get_init_creds_password(kcontext, &my_creds, me, NULL,
					    krb5_prompter_posix, NULL,
					    start_time, service_name,
					    &opts);
#ifdef KRB5_KRB4_COMPAT
	got_krb4 = try_krb4(kcontext, me, password, lifetime);
#endif
	break;
    case INIT_KT:
	code = krb5_get_init_creds_keytab(kcontext, &my_creds, me, keytab,
					    start_time, service_name,
					    &opts);
	break;
    case VALIDATE:
	code = krb5_get_validated_creds(kcontext, &my_creds, me, ccache,
					service_name);
	break;
    case RENEW:
	code = krb5_get_renewed_creds(kcontext, &my_creds, me, ccache,
				      service_name);
	break;
    }

    if (code) {
	if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
	    fprintf (stderr, "%s: Password incorrect\n", argv[0]);
	else
	    com_err (argv[0], code, "while getting initial credentials");
	exit(1);
    }

    if (code = krb5_cc_initialize(kcontext, ccache, me)) {
	com_err (argv[0], code, "when initializing cache %s",
		 cache_name?cache_name:"");
	exit(1);
    }

    if (code = krb5_cc_store_cred(kcontext, ccache, &my_creds)) {
	com_err (argv[0], code, "while storing credentials");
	exit(1);
    }

#ifdef KRB5_KRB4_COMPAT
    if (!got_krb4)
	try_convert524(kcontext, ccache);
#endif

    if (me)
	krb5_free_principal(kcontext, me);
    if (keytab)
	krb5_kt_close(kcontext, keytab);
    if (ccache)
	krb5_cc_close(kcontext, ccache);
    if (addresses)
	krb5_free_addresses(kcontext, addresses);

    krb5_free_context(kcontext);
    
    exit(0);
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
