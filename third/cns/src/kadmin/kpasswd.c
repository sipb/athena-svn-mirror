/*
 * kpasswd.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * change your password with kerberos
 */

#include <mit-copyright.h>
/*
 * kpasswd
 * change your password with kerberos
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pwd.h>
#include <kadm_err.h>
#include "kadm.h"

extern void krb_set_tkt_string();
static int using_preauth = 0;

static int oldhist[256], newhist[256];

main(argc,argv)
  int argc;
  char *argv[];
{
    char name[ANAME_SZ];	/* name of user */
    char inst[INST_SZ];		/* instance of user */
    char realm[REALM_SZ];	/* realm of user */
    char default_name[ANAME_SZ];
    char default_inst[INST_SZ];
    char default_realm[REALM_SZ];
    int realm_given = 0;	/* True if realm was give on cmdline */
    int use_default = 1;	/* True if we should use default name */
    int skip_old = 0;		/* True if we should skip getting old pw */
    struct passwd *pw;
    int status;			/* return code */
    des_cblock new_key;
    char pword[MAX_KPW_LEN];	/* storage for the password */
    int c;
    extern char *optarg;
    extern int optind;
    char tktstring[MAXPATHLEN];
    char *ret_st;
    
    void get_pw_new_key();
    
#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else
#define read_long_pw_string des_read_pw_string
#endif
    int read_long_pw_string();
    
    memset(name, 0, sizeof(name));
    memset(inst, 0, sizeof(inst));
    memset(realm, 0, sizeof(realm));
    
    if (krb_get_tf_fullname(TKT_FILE, default_name, default_inst, 
			    default_realm) != KSUCCESS) {
	pw = getpwuid((int) getuid());
	if (pw)
	    (void) strcpy(default_name, pw->pw_name);
	else
	    /* seems like a null name is kinda silly */
	    (void) strcpy(default_name, ""); 
	strcpy(default_inst, "");
	if (krb_get_lrealm(default_realm, 1) != KSUCCESS)
	    strcpy(default_realm, KRB_REALM);
    }

    while ((c = getopt(argc, argv, "u:n:i:r:hp")) != EOF) {
	switch (c) {
	  case 'u':
	    if (status = kname_parse(name, inst, realm, optarg)) {
		fprintf(stderr, "Kerberos error: %s\n",
			krb_get_err_text(status));
		exit(2);
	    }
	    if (realm[0])
		realm_given++;
	    else
		if (krb_get_lrealm(realm, 1) != KSUCCESS)
		    strcpy(realm, KRB_REALM);
	    break;
	  case 'n':
	    if (k_isname(optarg))
		(void) strncpy(name, optarg, sizeof(name) - 1);
	    else {
		fprintf(stderr, "Bad name: %s\n", optarg);
		usage(1);
	    }
	    break;
	  case 'i':
	    if (k_isinst(optarg))
		(void) strncpy(inst, optarg, sizeof(inst) - 1);
	    else {
		fprintf(stderr, "Bad instance: %s\n", optarg);
		usage(1);
	    }
	    (void) strcpy(inst, optarg);
	    break;
	  case 'r':
	    if (k_isrealm(optarg)) {
		(void) strncpy(realm, optarg, sizeof(realm) - 1);
		realm_given++; 
	    }
	    else {
		fprintf(stderr, "Bad realm: %s\n", optarg);
		usage(1);
	    }
	    break;
	  case 'h':
	    usage(0);
	    break;
	  case 'p':
	    using_preauth++;
	    break;
	  default:
	    usage(1);
	    break;
	}
	use_default = 0;
    }
    if (optind < argc)
	usage(1);

    if (use_default) {
	strcpy(name, default_name);
	strcpy(inst, default_inst);
	strcpy(realm, default_realm);
    }
    else {
	if (!name[0])
	    strcpy(name, default_name);
	if (!realm[0])
	    strcpy(realm, default_realm);
    }

    (void) sprintf(tktstring, "/tmp/tkt_cpw_%d",getpid());
    krb_set_tkt_string(tktstring);
    
try_again:
    get_pw_new_key(new_key, pword, name, inst, realm, realm_given, skip_old);
    skip_old++;
    
    if ((status = kadm_init_link(PWSERV_NAME, KRB_MASTER, realm)) 
	!= KADM_SUCCESS) {
	com_err(argv[0], status, "while initializing");
	memset(pword, 0, sizeof(pword));
    } else {
#ifdef CHECK_ONLY
	    status = kadm_check_pw(new_key, pword, (u_char **)&ret_st);
#else
	    status = kadm_change_pw2(new_key, pword, (u_char **)&ret_st);
#endif
	    memset(pword, 0, sizeof(pword));
	    if (ret_st) {
		    printf("\n%s\n", ret_st);
		    free(ret_st);
	    }
	    if (status != KADM_SUCCESS)
		    com_err(argv[0], status,
			    "while attempting to change password.");
	    if (status == KADM_DB_INUSE)
		    com_err(argv[0], 0, "Please try again later.");
	    if (status == KADM_INSECURE_PW) {
		    printf("Please choose another password.\n\n");
		    goto try_again;
	    }
    }
    
#ifdef CHECK_ONLY
    fprintf(stderr, "Passwrd NOT changed --- this is a test version of kpasswd.\n");
#else
    if (status != KADM_SUCCESS)
	fprintf(stderr,"Password NOT changed.\n");
    else
	printf("Password changed.\n");
#endif

    (void) dest_tkt();
    if (status)
	exit(2);
    else 
	exit(0);
}

void get_pw_new_key(new_key, pword, name, inst, realm, print_realm, skip_old)
  des_cblock new_key;
  char *pword;
  char *name;
  char *inst;
  char *realm;
  int print_realm;		/* True if realm was give on cmdline */
  int skip_old;
{
    char ppromp[40+ANAME_SZ+INST_SZ+REALM_SZ]; /* for the password prompt */
    char npromp[40+ANAME_SZ+INST_SZ+REALM_SZ]; /* for the password prompt */
    
    char local_realm[REALM_SZ];
    int status, diff, i, d;
    char *s;
    
    /*
     * We don't care about failure; this is to determine whether or
     * not to print the realm in the prompt for a new password. 
     */
    (void) krb_get_lrealm(local_realm, 1);
    
    if (strcmp(local_realm, realm))
	print_realm++;
    
    if (!skip_old) {
	    (void) sprintf(ppromp,"Old password for %s%s%s%s%s:",
			   name, *inst ? "." : "", inst,
			   print_realm ? "@" : "", print_realm ? realm : "");
	    if (read_long_pw_string(pword, MAX_KPW_LEN-1, ppromp, 0)) {
		    fprintf(stderr, "Error reading old password.\n");
		    exit(1);
	    }

	    if (using_preauth) 
		status = krb_get_pw_in_tkt_preauth(name, inst, realm, PWSERV_NAME,
					  	   KADM_SINST, 1, pword);
	    else
                status = krb_get_pw_in_tkt(name, inst, realm, PWSERV_NAME,
                                           KADM_SINST, 1, pword);

	    if (status != KSUCCESS) {
		    if (status == INTK_BADPW) {
			    printf("Incorrect old password.\n");
			    exit(0);
		    }
		    else {
			    fprintf(stderr, "Kerberos error: %s\n",
				    krb_get_err_text(status));
			    exit(1);
		    }
	    }
	    for (i = 0; i < 256; i++)
	      oldhist[i] = 0;
	    for (s = pword; *s; s++)
	      oldhist[*s & 0xff]++;
    }
    do {
	(void) sprintf(npromp,"New Password for %s%s%s%s%s:",
		       name, *inst ? "." : "", inst,
		       print_realm ? "@" : "", print_realm ? realm : "");
	if (read_long_pw_string(pword, MAX_KPW_LEN-1, npromp, 1))
	    go_home("Error reading new password, password unchanged.\n",0);
	if (strlen(pword) == 0)
	    printf("Null passwords are not allowed; try again.\n");
	for (i = 0; i < 256; i++)
	  newhist[i] = 0;
	for (s = pword; *s; s++)
	  newhist[*s & 0xff]++;
	for (i = 0, diff = 0; i < 256; i++) {
	  d = newhist[i] - oldhist[i];
	  if (d > 0)
	    diff += d;
	  else
	    diff -= d;
	}
	if (diff < 3) {
	  printf ("Password too similar to previous; try again.\n");
	  pword[0] = 0;
	}
    } while (strlen(pword) == 0);
    
#ifdef NOENCRYPTION
    memset((char *) new_key, 0, sizeof(des_cblock));
    new_key[0] = (unsigned char) 1;
#else
    (void) des_string_to_key(pword, new_key);
#endif
}

usage(value)
  int value;
{
    fprintf(stderr, "Usage: ");
    fprintf(stderr, "kpasswd [-h ] [-n user] [-i instance] [-r realm] [-p] ");
    fprintf(stderr, "[-u fullname]\n");
    exit(value);
}

go_home(str,x)
  char *str;
  int x;
{
    fprintf(stderr, str, x);
    (void) dest_tkt();
    exit(1);
}

/* ksrvutil can fail due to talking to the wrong KDC. kpasswd will only 
   confuse the user, as the password will still get tickets, and that will
   suffice for the change. However, the user may find it confusing if they
   change passwords quickly and it doesn't work, so for now we have them
   talk to the master for everything. In the long run, we would want to 
   change the protocol.
 */
krb_get_krbhst(h,r,n)
    char *h;
    char *r;
    int n;
{
  return krb_get_admhst(h, r, n);
}
