/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpquota.c,v 1.15 1993-06-07 08:27:46 probe Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpquota.c,v $ */
/* $Author: probe $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char lpquota_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpquota.c,v 1.15 1993-06-07 08:27:46 probe Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include "logger_ncs.h"
#include "pfm.h"
#if 0
#include "rrpc.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <signal.h>
#include <pwd.h>
#include "quota_err.h"
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <lp.local.h>

extern uid_t getuid();
extern uuid_$t uuid_$nil;
extern char *getenv();
extern char *pgetstr();
extern long atol();

char *progname;

char   *
    getuser()
{
    char   *name;
    struct stat statbuf;
    struct passwd *pw;
    static char noname[] = "???";
    extern char *getlogin();

    if ((name = getlogin()) != NULL)
	if (*name != NULL)
	    return (name);
    if ((pw = getpwuid((int) getuid())) != NULL)
	return (pw->pw_name);
    if (isatty(2)) {
	if (fstat(2, &statbuf) != 0)
	    return (noname);
	if ((pw = getpwuid((int) statbuf.st_uid)) == NULL)
	    return (noname);
	return (pw->pw_name);
    }
    return (noname);
}

char *
    savestr(s)
char *s;
{
    char *ptr;
    
    ptr = malloc((unsigned) strlen(s) + 1);
    if (ptr == NULL) {
        fprintf(stderr, "malloc: out of space\n");
	exit(3);
    }
    (void) strcpy(ptr,s);
    return(ptr);
}


char *error_text(st)
status_$t st;
{
    static char buff[200];
    extern char *error_$c_text();

    return (error_$c_text(st, buff, sizeof buff));
}

main(argc, argv)
int argc;
char **argv;
{

    status_$t fst;
    pfm_$cleanup_rec crec;
    status_$t st;
    handle_t h, hl;
    socket_$addr_t addr;
    unsigned long length = sizeof(addr);
    krb_ktext auth;
    char *host = NULL, *printer = NULL, *hosttmp;
    unsigned char hostname[MAXHOSTNAMELEN];
    register char *arg;
    char *username = NULL;
    char *service = NULL;
    char *savestr(), *getname(), *krb_get_phost(), *krb_realmofhost();
    char krb_realm[REALM_SZ];
    char kname[ANAME_SZ], kinst[REALM_SZ], krealm[INST_SZ];
    char kfullname[MAX_K_NAME_SZ];
    char buf[BUFSIZ];
    struct servent *servname;
    int port;

    static char pbuf[BUFSIZ/2];
#ifdef HESIOD
char	alibuf[BUFSIZ/2];	/* buffer for printer alias */
#endif
    char *bp = pbuf;
    extern char *pgetstr();
#ifndef HESIOD
    int status;
#endif

    int query_flag=0;		/* Query user */
    int create_flag=0;		/* Create user */
    int print_flag=0;		/* Allow printing status */
    int inqst_flag=0;		/* Determine status of servers */
    modify_user_type set_allow_printing;
    modify_account_type set_group_printing;
    int allow_printing = -1;
         /* The following is a one of ... */
    int inc_flag = 0;		/* Increment quota */
    int dec_flag = 0;		/* Decrement quota */
    int set_flag = 0;		/* Set quota */
    int adj_flag = 0;		/* Adjust quota usage */
    int l_flag = 0;		/* Log flag */

    int grp_flag = 0;           /* Are we accessing a group account */
    int add_admin_flag = 0;     /* Add admin to a group */
    int del_admin_flag = 0;     /* Delete admin from a group */
    int add_user_flag = 0;      /* Add user to a group */
    int del_user_flag = 0;      /* Delete user to a group */
    int no_username_flag = 0;   /* True if no username required */

    quota_value amount = -1;
    quota_account account = 0;  /* Zero means no account specified */

    int ret;
    quota_identifier qid;

    initialize_quot_error_table();

    /* Process argument
     * -P printer name to get quota info from
     * -Q quota server
     * -s service name
     */

    progname = (arg = rindex(argv[0], '/')) ? arg+1 : argv[0];

    while (--argc) {
	arg = *++argv;
	if (arg[0] == '-') {
	    switch (arg[1]) {
	    case 'Q':		/* printer name */
		if(printer) {
		    fprintf(stderr, "Can only select one of -Q or -P\n");
		    exit(1);
		}
		if (arg[2])
		    host = &arg[2];
		else if (argc > 1) {
		    argc--;
		    host = *++argv;
		} else usage();
		break;

	    case 'P':		/* printer name */
		if(host) {
		    fprintf(stderr, "Can only select one of -Q or -P\n");
		    exit(1);
		}
		if (arg[2])
		    printer = &arg[2];
		else if (argc > 1) {
		    argc--;
		    printer = *++argv;
		} else usage();
		break;

	    case 'S':
		if (arg[2])
		    service = &arg[2];
		else if (argc > 1) {
		    argc--;
		    service = *++argv;
		} else usage();
		break;

	    case 'I':
		inqst_flag++;
		break;

	    case 'c':
		create_flag++;
		break;

	    case 'q':
		query_flag++;
		break;

	    case 'l':
		l_flag++;
		break;

	    case 'f':
		if(arg[2]) 
		    allow_printing = atoi(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    allow_printing = atoi(*++argv);
		} else usage();
		if ((allow_printing != 0) && (allow_printing != 1)) {
		    fprintf(stderr,"%s: -f must be followed with either 0 or 1\n",progname);
		    exit(1);
		}
		print_flag = 1;
		break;

	    case 'i':
		if(arg[2]) 
		    amount= atoi(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    amount = atoi(*++argv);
		} else usage();
		if(amount < 0) {
		    fprintf(stderr, "%s: Invalid quota amount.\n",progname);
		    exit(1);
		}
		inc_flag=1;
		break;

	    case 'g':
	    case 'a':
		if(arg[2]) 
		    account = atol(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    account = atol(*++argv);
		} else usage();
		if(account <= (long)0) {
		    fprintf(stderr, "%s: Invalid group a/c number.\n",progname);
		    exit(1);
		}
		grp_flag=1;
		break;

	    case 'd':
		if(arg[2]) 
		    amount= atoi(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    amount = atoi(*++argv);
		} else usage();
		if(amount < 0) {
		    fprintf(stderr, "%s: Invalid quota amount.\n",progname);
		    exit(1);
		}
		dec_flag=1;
		break;

	    case 's':
		if(arg[2]) 
		    amount= atoi(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    amount = atoi(*++argv);
		} else usage();
		if(amount < 0) {
		    fprintf(stderr, "%s: Invalid quota amount.\n",progname);
		    exit(1);
		}
		set_flag=1;
		break;

	    case 'k':
		if(arg[2]) 
		    amount= atoi(&arg[2]);
		else if (argc > 1) {
		    argc--;
		    amount = atoi(*++argv);
		} else usage();
		if(amount < 0) {
		    fprintf(stderr, "%s: Invalid quota amount.\n",progname);
		    exit(1);
		}
		adj_flag=1;
		break;

	    case 'A':
		del_admin_flag++;
		break;

	    case 'U':
		del_user_flag++;
		break;

	    case 'p':
		print_flag++;
		allow_printing = 0;
		break;

	    default:
		usage();
	    }

	} else if (arg[0] == '+') {
	    switch (arg[1]) {
	    case 'A':
		add_admin_flag++;
		break;

	    case 'U':
		add_user_flag++;
		break;

	    case 'p':
		print_flag++;
		allow_printing = 1;
		break;

	    default:
		usage();
	    }
	} else {
	    /* Get persons name */
	    if(argc > 1) {
		fprintf(stderr, "Only one username allowed on command line\n");
		exit(1);
	    }
	    username = *argv;
	}
    }

    if (print_flag) {
	if (!grp_flag) {
	    if(allow_printing) set_allow_printing = U_ALLOW_PRINT;
	    else set_allow_printing = U_DISALLOW_PRINT;
	} else {
	    if(allow_printing) set_group_printing = A_ALLOW_PRINT;
	    else set_group_printing = A_DISALLOW_PRINT;
	}
    }

    if(adj_flag + inc_flag + dec_flag + set_flag > 1) {
	fprintf(stderr, "%s: May only specify one of -a, -i, -d, -s\n",progname);
	exit(1);
    }

    /* require username for the following group operations */
    if (grp_flag) {
	if (add_admin_flag || del_admin_flag || add_user_flag  || del_user_flag)
	    no_username_flag = 0;
	else
	    no_username_flag = 1;
    } else {
	if (add_admin_flag || del_admin_flag || del_user_flag || 
	    del_user_flag) {
	    fprintf(stderr, "%s: May only specify one of +A, -A, +U, -U after the -g option\n",progname);
	    exit(1);
	}
    }

    if(inqst_flag + adj_flag + inc_flag + dec_flag + set_flag + create_flag +
       print_flag + query_flag + l_flag + add_admin_flag +
       del_admin_flag + add_user_flag + del_user_flag == 0) query_flag++; 

    /* The following determines the default printer with fallback al lpr */
    if (printer == NULL && host== NULL && 
	(printer = getenv("PRINTER")) == NULL) printer = DEFLP;

    if(printer) {
#ifdef HESIOD
	if (pgetent(buf, printer) <= 0) {
	    if (pralias(alibuf, printer))
		printer = alibuf;
	    if (hpgetent(buf, printer) < 1)
		fatal("%s: unknown printer", printer);
	}
#else
	if ((status = pgetent(buf, printer)) < 0)
	    fatal("cannot open printer description file");
	else if (status == 0)
	    fatal("%s: unknown printer", printer);
#endif HESIOD
	if((host = pgetstr("rq",&bp)) == NULL) 
	    fatal("%s: no quota server registered with printer",printer);
    }

    if (host == NULL) {
	fprintf(stderr, "No host specified\n");
	exit(1);
    }

    if (service == NULL)
	service = DEFSERVICE;

    if (grp_flag) {
	if (!no_username_flag && username == NULL)
	    fatal("require a username for the group account operation");
    } else {
	if (username == NULL) {
	    if ((ret = krb_get_tf_fullname(TKT_FILE, kname, kinst, krealm)) !=
		KSUCCESS) {
		fprintf(stderr, "%s\n", krb_err_txt[ret]);
		exit(1);
	    }
	    make_kname(kname, kinst, krealm, kfullname);
	    username = (char *)kfullname;
	}
    }

    /* Contact.... */
    hosttmp = krb_get_phost(host);
    if(hosttmp == NULL) {
	fprintf(stderr, "Could not resolve name %s\n",host);
	exit(1);
    }
    (void) strcpy((char *) hostname, hosttmp);

    hosttmp = krb_realmofhost(host);
    if(hosttmp == NULL) {
	fprintf(stderr, "Could not determine kerberos realm of %s\n",host);
	exit(1);
    }
    (void) strcpy(krb_realm, hosttmp);

    ret = krb_mk_req(&auth, KLPQUOTA_SERVICE, hostname, krb_realm, (long) 0);
    if (ret != KSUCCESS) {
	fprintf(stderr, "Kerberos authentication failed - %s\n", krb_err_txt[ret]);
	fprintf(stderr, "Running unauthenticated.\n");
	auth.length=0;
	auth.dat[0]='\0';
    }

    /* Setup structures */
    if (!no_username_flag) {
	(void) strcpy((char *) qid.username, username);
    } else {
	qid.username[0] = '\0';
    }
    (void) strcpy((char *) qid.service, service);
    qid.account = (long) account;

    /* Do request */
    pfm_$init(pfm_$init_signal_handlers);
    fst = pfm_$cleanup(crec);
    (void) signal (SIGILL, SIG_DFL);
    (void) signal (SIGSEGV, SIG_DFL);
    (void) signal (SIGBUS, SIG_DFL);
    (void) signal (SIGINT, exit);
    (void) signal (SIGHUP, exit);
    (void) signal (SIGQUIT, exit);

    if (fst.all != pfm_$cleanup_set) {
	if (fst.all != status_$ok) {
	    if(fst.all == rpc_$comm_failure) {
		fprintf(stderr, "Unable to contact quota server on %s\n",hostname);
		Exit(1);
	    }

	    if(fst.all == nca_status_$string_too_long) {
		fprintf(stderr, "Server tried passing a non-terminated string\n");
		Exit(1);
	    }

	    if(fst.all == nca_status_$unk_if) {
		fprintf(stderr, "This client will not work with the remote server - version skew\n");
		Exit(1);
	    }


	    fprintf(stderr, "*** Exception raised - %s\n", error_text(fst));
	    fprintf(stderr, "Error # %d\n", fst.all);
	}
	Exit(1);
    }

    servname = getservbyname(QUOTAQUERYENT,"udp");
    if(!servname) port = QUOTAQUERYPORT;
    else port = ntohs((u_short) servname->s_port);
    
    rpc_$name_to_sockaddr (hostname, strlen(host), port, socket_$internet,
			       &addr, &length, &st);
    if (st.all != 0) {
	fprintf(stderr, "Host not found: %s \n", hostname);
	Exit(1);
    }
    h = rpc_$bind (&uuid_$nil, &addr, length, &st);
    if (st.all != 0) {
	fprintf(stderr, "Couldn't bind handle: %s\n",
		error_text(st));
	Exit(1);
    }

    if(l_flag) {
	servname = getservbyname(QUOTALOGENT,"udp");
	if(!servname) port = QUOTALOGGERPORT;
	else port = ntohs((u_short) servname->s_port);

	rpc_$name_to_sockaddr (hostname, strlen(host), port, socket_$internet,
			       &addr, &length, &st);
	if (st.all != 0) {
	    fprintf(stderr, "Host not found: %s \n", hostname);
	    Exit(1);
	}
	hl = rpc_$bind (&uuid_$nil, &addr, length, &st);
	if (st.all != 0) {
	    fprintf(stderr, "Couldn't bind handle: %s\n",
		    error_text(st));
	    Exit(1);
	}
    }
	
#if 0
    rpc_$set_short_timeout(h, 1, &st);
    if (st.all !=0) {
	/* Oh well, long timeout... */
    }

    (rrpc_$client_epv.rrpc_$are_you_there) (h, &st);
    if (st.all != 0) {
	/* This should not happen as the error handler should trap this. */
	fprintf(stderr, "Server not responding: %s\n",
		error_text(st));
	Exit(1);
    }
    rpc_$set_short_timeout(h, 0, &st);
    if (st.all !=0) {
	/* Oh well, nothing too bad... The docs say it should be long anyhow */
    }
#endif

    if (inqst_flag) qstatus(h,&auth);

    if (!grp_flag) {
	if(create_flag) {
	    if(qmodify_user(h,&qid,&auth,U_NEW,(quota_value) 0)) Exit(3);
	}	
	if(inc_flag) {
	    if(qmodify_user(h,&qid,&auth,U_ADD,amount)) Exit(3);
	}	
	if(dec_flag) {
	    if(qmodify_user(h,&qid,&auth,U_SUBTRACT,amount)) Exit(3);
	}	
	if(set_flag) {
	    if(qmodify_user(h,&qid,&auth,U_SET,amount)) Exit(3);
	}	
	if(adj_flag) {
	    if(qmodify_user(h,&qid,&auth,U_ADJUST,amount)) Exit(3);
	}	
	if(print_flag) {
	    if(qmodify_user(h,&qid,&auth, set_allow_printing, 
			    (quota_value) 0)) Exit(3);
	}
	if(query_flag) {
	    if (query_quota(h,&qid,&auth)) Exit(3);
	}
	if(l_flag) {
	    if (query_logs(hl,&qid,&auth)) Exit(3);
	}
    } else {
	if(create_flag) {
	    if(qmodify_account(h,&auth,account,A_NEW,&qid,(quota_value) 0)) 
		Exit(4);
	}
	if(inc_flag) {
	    if(qmodify_account(h,&auth,account,A_ADD,&qid,amount)) 
		Exit(4);
	}	
	if(dec_flag) {
	    if(qmodify_account(h,&auth,account,A_SUBTRACT,&qid,amount)) 
		Exit(4);
	}	
	if(set_flag) {
	    if(qmodify_account(h,&auth,account,A_SET,&qid,amount)) 
		Exit(4);
	}	
	if(adj_flag) {
	    if(qmodify_account(h,&auth,account,A_ADJUST,&qid,amount)) 
		Exit(4);
	}	
	if(print_flag) {
	    if(qmodify_account(h,&auth,account,set_group_printing,
			       &qid,(quota_value) 0)) 
		Exit(4);
	}
	if (add_admin_flag) {
	    if(qmodify_account(h,&auth,account,A_ADD_ADMIN,
			       &qid,(quota_value) 0)) 
		Exit(4);
	}
	if (del_admin_flag) {
	    if(qmodify_account(h,&auth,account,A_DELETE_ADMIN,
			       &qid,(quota_value) 0)) 
		Exit(4);
	}
	if (add_user_flag) {
	    if(qmodify_account(h,&auth,account,A_ADD_USER,
			       &qid,(quota_value) 0)) 
		Exit(4);
	}
	if (del_user_flag) {
	    if(qmodify_account(h,&auth,account,A_DELETE_USER,
			       &qid,(quota_value) 0)) 
		Exit(4);
	}
	if(query_flag) {
	    if (query_group_quota(h,account,&qid,&auth)) Exit(4);
	}
	if(l_flag) {
	    if (query_group_logs(hl,account,&qid,&auth)) Exit(4);
	}
    }
}



Exit(n)
int n;
{
    exit(n);
}

usage()
{
    fprintf(stderr, "Usage: %s options... [username]\n", progname);
    fprintf(stderr, "   where options are :\n");
    fprintf(stderr, "\t [-Q quotaserver] - Name of quotaserver\n");
    fprintf(stderr, "\t [-P printer]     - Name of printer\n");
    fprintf(stderr, "\t [-S service]     - Name of service\n");
    fprintf(stderr, "\t [-a account_no]  - Group account number\n");
    fprintf(stderr, "\t [-c]             - Create new user or group account\n");
    fprintf(stderr, "\t [-q]             - Display user or group usage\n");
    fprintf(stderr, "\t [-l]             - Display user or group logs\n");
    fprintf(stderr, "\t [-f allowed]     - Enable/Disable printing\n");
    fprintf(stderr, "\t [-i amount]      - Increase quota by amount\n");
    fprintf(stderr, "\t [-d amount]      - Decrease quota by amount\n");
    fprintf(stderr, "\t [-s amount]      - Set quota to amount\n");
    fprintf(stderr, "\t [-k amount]      - Credit usage by amount (adjustment)\n");
    fprintf(stderr, "\t [+A]             - Add user to group admin list\n");
    fprintf(stderr, "\t [-A]             - Remove user from group admin list\n");
    fprintf(stderr, "\t [+U]             - Add user from group user list\n");
    fprintf(stderr, "\t [-U]             - Remove user from group user list\n");
    exit(1);
}

quota_error(err)
quota_error_code err;
{
    com_err(progname, err, "");
    return(err);
}

query_quota(h,qid,auth)
handle_t h;
quota_identifier *qid;
krb_ktext *auth;
{
    quota_error_code qerr;
    quota_return qret;

    if((qerr=QuotaQuery(h,auth,qid,&qret)))
	return quota_error(qerr);

    printf("Username\t: %s\n", qid->username);
    printf("Service\t\t: %s\n", qid->service);
    if(strcmp((char *) qret.currency, "cents")) {
	printf("Usage:  %d %s	limit:	%d %s\n", 
	       qret.usage, qret.currency, qret.limit, qret.currency);
	if(qret.last_bill) 
	    printf("Last bill date: %s", ctime(&(qret.last_bill)));
	printf("Last bill: %d %s\t",qret.last_charge,qret.currency);
	printf("Ytd billed: %d %s\n",qret.ytd_billed,qret.currency);
    } else {
	printf("Usage: $%.2f\t\tLimit: $%.2f\n", 
	       (float) qret.usage/100.0, (float) qret.limit/100.0);
	if(qret.last_bill) 
	    printf("Last bill date: %s", ctime(&(qret.last_bill)));
	printf("Last bill: $%.2f\t",(float) qret.last_charge/100.0);
	printf("Ytd billed: $%.2f\n",(float) qret.ytd_billed/100.0);
    }
    printf("%s\n", qret.message);

    return 0;
}


query_group_quota(h,acct,qid,auth)
handle_t h;
long acct;
quota_identifier *qid;
krb_ktext *auth;
{
    quota_error_code qerr;
    qstartingpoint startadmin, startuser;
    qmaxtotransfer maxadmin, maxuser;
    quota_return qret;
    long numadmin, numuser, flag;
    int count_user, count_admin;
    Principal admin[GQUOTA_MAX_ADMIN+1], user[GQUOTA_MAX_USER+1];
    int first_run = 1;
    int i;

    /* Initialize */
    startadmin = startuser = 0;
    maxadmin = G_ADMINMAXRETURN;
    maxuser = G_USERMAXRETURN;
    count_user = count_admin = 1;

    while (1) {
	if (qerr=QuotaQueryAccount(h,auth,acct,qid,startadmin,maxadmin,
				   startuser,maxuser,&qret,&numadmin,
				   admin, &numuser, 
				   user, &flag))
	    return quota_error(qerr);

	count_admin += numadmin;
	count_user  += numuser;

	/* Display the quota usage the first time around */
	if (first_run) {
	    printf("Account\t\t: %d\n", acct);
	    printf("Service\t\t: %s\n", qid->service);
	    if(strcmp((char *) qret.currency, "cents")) {
		printf("Usage:  %d %s	limit:	%d %s\n", 
		       qret.usage, qret.currency, qret.limit, qret.currency);
		if(qret.last_bill) 
		    printf("Last bill date: %s", ctime(&(qret.last_bill)));
		printf("Last bill: %d %s\t",qret.last_charge,qret.currency);
		printf("Ytd billed: %d %s\n",qret.ytd_billed,qret.currency);
	    } else {
		printf("Usage: $%.2f\t\tLimit: $%.2f\n", 
		       (float) qret.usage/100.0, (float) qret.limit/100.0);
		if(qret.last_bill) 
		    printf("Last bill date: %s", ctime(&(qret.last_bill)));
		printf("Last bill: $%.2f\t",(float) qret.last_charge/100.0);
		printf("Ytd billed: $%.2f\n",(float) qret.ytd_billed/100.0);
	    }
	    printf("%s\n", qret.message);
	    first_run = 0;
	}
	
	/* If we have no more admin and users then display the ones
	 * we have accumulated 
	 */
	if (flag == GQUOTA_NONE) break;
	
	/* Hmm, there are more user and admins, we need to get them */
	if (startuser == 0)
	    startuser += numuser + 1;
	else
	    startuser += numuser;

	if (startadmin == 0)
	    startadmin += numadmin + 1;
	else
	    startadmin += numadmin;

	if (flag != GQUOTA_BOTH) {
	    if (flag == GQUOTA_MORE_USER)
		startadmin = -1;  /* Ignore admin list when doing server query */
	    else 
		startuser = -1;
	}
    }
    /* Now display the admin and users */
    if (count_admin > 1) {
	printf("List of group account administrators :\n");
	for(i = 0; i < count_admin; i++)
	    printf("\t%s\n", admin[i]);
    }
    if (count_user > 1) {
	printf("List of group account users :\n");
	for(i = 0; i < count_user; i++)
	    printf("\t%s\n", user[i]);
    }
    return 0;
}


qmodify_user(h,qid,auth,qtype,qamount)
handle_t h;
quota_identifier *qid;
krb_ktext *auth;
modify_user_type qtype;
quota_value qamount;
{
    quota_error_code qerr;

    if((qerr=QuotaModifyUser(h,auth,qid,qtype,qamount)))
	return quota_error(qerr);

    return 0;

}

qmodify_account(h,auth,qaccount,qtype,qid,qamount)
handle_t h;
quota_account qaccount;
quota_identifier *qid;
krb_ktext *auth;
modify_account_type qtype;
quota_value qamount;
{
    quota_error_code qerr;

    if((qerr=QuotaModifyAccount(h,auth,qaccount,qtype,qid,qamount)))
	return quota_error(qerr);

    return 0;

}

query_logs(h,qid,auth)
handle_t h;
quota_identifier *qid;
krb_ktext *auth;

    {
	quota_error_code qerr;
	startingpoint start;
	long numtrans;
	int i;
	LogEnt LogEnts[LOGMAXRETURN], *lent;
	int done = 0, any = 0;
	char *chargestr="", who[MAX_K_NAME_SZ], who1[MAX_K_NAME_SZ];
	quota_currency currency;

	start = 0;

	while(done == 0) {
	    if(qerr = LoggerJournal(h, auth, qid, start, 20,
			    (loggerflags) 0, &numtrans, LogEnts, currency))
		return quota_error(qerr);
	    if(numtrans == 0) {
		if(any == 0) {
		    if (qid->account == 0)
			printf("No printer logs available for user.\n");
		    else
			printf("No printer logs available for group account.\n");
		}
		printf("\n");
		return 0;
	    }
	    any ++;
	    for(i = 1, lent = LogEnts; i <= numtrans; i++, lent++) {
	    switch((int) lent->func) {
	    case LO_ADD:
		chargestr = "increased by";
		goto rest;
	    case LO_SUBTRACT:
		chargestr = "decreased by";
		goto rest;
	    case LO_SET:
		chargestr = "set to";
		goto rest;
	    case LO_ADJUST:
		chargestr = "credited by";
		goto rest;		
	    case LO_DELETEUSER:
		goto rest;		
	    case LO_ALLOW:
		chargestr = "allowed";
		goto rest;		
	    case LO_DISALLOW:
		chargestr = "disallowed";
	    rest:
		(void) strcpy(who, (char *) lent->offset.wname);
		
		if (lent->func == LO_DELETEUSER) {
		    if (qid->account == 0)
			printf("%.24s user quota was deleted by %s\n",
			   ctime(&(lent->time)), who);
		    else
			printf("%.24s group account quota was deleted by %s\n",
			   ctime(&(lent->time)), who);			
		} else if (lent->func == LO_ALLOW || 
			   lent->func == LO_DISALLOW) {
		    printf("%.24s printing was %s by %s\n",
			   ctime(&(lent->time)), chargestr, who);	    
		} else {
		    if(strcmp((char *) currency, "cents")) 
			printf("%.24s quota was %s %d %s by %s\n",
			       ctime(&(lent->time)), chargestr, 
			       lent->offset.amount, 
			       currency, who);
		    else 
			printf("%.24s quota was %s $%.2f by %s\n",
			       ctime(&(lent->time)), chargestr, 
			       (float) (lent->offset.amount)/100.0, 
			       who);
		}
		break;
	    case LO_CHARGE:
		/* We know who it was, need to print sub time, service, where*/
		if (qid->account == 0) {
		    printf("%.24s %d page%c @ %d %s/page on %s\n",
			   ctime(&(lent->charge.ptime)),
			   lent->charge.npages,
			   (lent->charge.npages == 1) ? '\0' : 's', 
			   lent->charge.pcost,
			   currency, 
			   lent->charge.where);
		} else {
		    (void) strcpy(who, (char *) lent->charge.wname);
		    printf("%.24s %d page%c @ %d %s/page on %s by %s\n",
			   ctime(&(lent->charge.ptime)),
			   lent->charge.npages,
			   (lent->charge.npages == 1) ? '\0' : 's', 
			   lent->charge.pcost,
			   currency, 
			   lent->charge.where,
			   who);
		}
		break;
	    case LO_ADD_ADMIN:
		chargestr = "added to admin";
		goto rest1;
	    case LO_ADD_USER:
		chargestr = "added to user";
		goto rest1;
	    case LO_DELETE_ADMIN:
		chargestr = "deleted from admin";
		goto rest1;
	    case LO_DELETE_USER:
		chargestr = "deleted from user";
		goto rest1;

	    rest1:
		(void) strcpy(who, (char *) lent->group.uname);
		(void) strcpy(who1, (char *) lent->group.aname);

		printf("%.24s %s was %s list by %s\n",
		       ctime(&(lent->time)),
		       who, chargestr, who1);
		break;
	    }
	    if(lent->next == 0) done++;
	    start = lent->next;
	}

    }

    /* All done !! */
    printf("\n");
    return 0;
    }


query_group_logs(h,acct,qid,auth)
handle_t h;
long acct;
quota_identifier *qid;
krb_ktext *auth;

{
	qid->account = acct;
	return(query_logs(h, qid, auth));
}

qstatus(h,auth)
handle_t h;
krb_ktext *auth;
    {
	quota_error_code qerr;
	quota_message msg;
	if(qerr = QuotaServerStatus(h, auth, msg)) 
	    return(quota_error(qerr));
	else
	    printf("%s\n", msg);
	return 0;
}

/*VARARGS1*/
fatal(msg, a1, a2, a3)
	char *msg;
{
	printf("%s: ", progname);
	printf(msg, a1, a2, a3);
	(void) putchar('\n');
	exit(1);
}


/* Form a complete string name consisting of principal,
 * instance and realm
 */
#ifdef __STDC__
void make_kname(char *principal, char *instance, char *realm, char *out_name)
#else
void make_kname(principal, instance, realm, out_name)
char *principal, *instance, *realm, *out_name;
#endif
{
    if ((instance[0] == '\0') && (realm[0] == '\0'))
        (void) strcpy(out_name, principal);
    else {
        if (realm[0] == '\0')
            (void) sprintf(out_name, "%s.%s", principal, instance);
        else {
            if (instance[0] == '\0')
                (void) sprintf(out_name, "%s@%s", principal, realm);
            else
                (void) sprintf(out_name, "%s.%s@%s", principal,
                        instance, realm);
        }
    }
}
