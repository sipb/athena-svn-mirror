/*
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This routine changes the Kerberos encryption keys for principals,
 * i.e., users or services. 
 */

/*
 * exit returns 	 0 ==> success -1 ==> error 
 */

#include <mit-copyright.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#ifdef NEED_TIME_H
#include <time.h>
#endif
#include <sys/time.h>

#include <des.h>
#include <krb.h>
#include <krb_db.h>
/* MKEYFILE is now defined in kdc.h */
#include <kdc.h>

extern char *errmsg();
extern int errno;
extern char *optarg;
extern int optind;

void    sig_exit();

#define zaptime(foo) memset((char *)(foo), 0, sizeof(*(foo)))

char    prog[32];
char   *progname = prog;
int     nflag = 0;
int     cflag;
int     lflag;
int     uflag;
int     debug;
extern  krb_debug;
#ifndef HAVE_SYS_ERRLIST_DECL
extern char *sys_errlist[];
#endif

Key_schedule KS;
C_Block new_key;
unsigned char *input;

unsigned char *ivec;
int     i, j;
int     more;

char   *in_ptr;
char    input_name[ANAME_SZ];
char    input_instance[INST_SZ];
char    input_string[ANAME_SZ];

#define	MAX_PRINCIPAL	10
Principal principal_data[MAX_PRINCIPAL];

static Principal old_principal;
static Principal default_princ;

static C_Block master_key;
static C_Block session_key;
static Key_schedule master_key_schedule;
static char pw_str[255];
static long master_key_version;
static char *mkeyfile = NULL;

main(argc, argv)
    int     argc;
    char   *argv[];

{
    /* Local Declarations */

    int     opt;
    long    n;

    prog[sizeof prog - 1] = '\0';	/* make sure terminated */
    strncpy(prog, argv[0], sizeof prog - 1);	/* salt away invoking
						 * program */

    /* Assume a long is four bytes */
    if (sizeof(KRB_INT32) != 4) {
	fprintf(stdout, "%s: size of long is %d.\n", prog, sizeof(KRB_INT32));
	exit(-1);
    }

#if 0
    /* This code has been here for years, but I do not know why.  */
    /* Assume <=32 signals */
    if (NSIG > 32) {
	fprintf(stderr, "%s: warning: more than 32 signals defined.\n", prog);
    }
#endif

    while ((opt = getopt (argc, argv, "dk:ln")) != EOF) {
    	switch (opt) {
	case 'd':
	    /* debug flag */
	    debug = 1;
	    break;

	case 'l':
	    /* debug flag */
	    krb_debug |= 1;
	    break;

	case 'n':
	    /* read MKEYFILE for master key.  */
	    nflag = 1;
	    break;

	case 'k':
	    /* read named file for master key.  */
	    nflag = 1;
	    mkeyfile = optarg;
	    break;

	default:
	    Usage();	/* Give message and die */
	}
    }

    fprintf(stdout, "Opening database...\n");
    fflush(stdout);
    kerb_init();
    if (optind < argc) {
	if (kerb_db_set_name(argv[optind]) != 0) {
	    fprintf(stderr, "Could not open altername database name\n");
	    exit(1);
	}
    }

#ifdef	notdef
    no_core_dumps();		/* diddle signals to avoid core dumps! */

    /* ignore whatever is reasonable */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

#endif

    if (kdb_get_master_key_from ((nflag == 0), 
				 master_key, master_key_schedule, 0,
				 mkeyfile) != 0) {
      fprintf (stdout, "Couldn't read master key.\n");
      fflush (stdout);
      exit (-1);
    }

    if ((master_key_version = kdb_verify_master_key(master_key,
						    master_key_schedule,
						    stdout)) < 0)
      exit (-1);

    des_init_random_number_generator(master_key);

    /* lookup the default values */
    n = kerb_get_principal(KERB_DEFAULT_NAME, KERB_DEFAULT_INST,
			   &default_princ, 1, &more);
    if (n != 1) {
	fprintf(stderr,
	     "%s: Kerberos error on default value lookup, %d found.\n",
		progname, n);
	exit(-1);
    }
    fprintf(stdout, "Previous or default values are in [brackets] ,\n");
    fprintf(stdout, "enter return to leave the same, or new value.\n");

    while (change_principal()) {
    }

    cleanup();
    exit(0);
}

change_principal()
{
    static char temp[255];
    int     creating = 0;
    int     editpw = 0;
    int     changed = 0;
    int     temp_int;
    int     n;
    time_t  exp_date;
    struct tm 	*tp, edate, *localtime();
    long 	maketime();

    fprintf(stdout, "\nPrincipal name: ");
    fflush(stdout);
    if (!gets(input_name) || *input_name == '\0')
	return 0;
    fprintf(stdout, "Instance: ");
    fflush(stdout);
    /* instance can be null */
    gets(input_instance);
    j = kerb_get_principal(input_name, input_instance, principal_data,
			   MAX_PRINCIPAL, &more);
    if (!j) {
	fprintf(stdout, "\n\07\07<Not found>, Create [y] ? ");
	gets(temp);		/* Default case should work, it didn't */
	if (temp[0] != 'y' && temp[0] != 'Y' && temp[0] != '\0')
	    return -1;
	/* make a new principal, fill in defaults */
	j = 1;
	creating = 1;
	strcpy(principal_data[0].name, input_name);
	strcpy(principal_data[0].instance, input_instance);
	principal_data[0].old = NULL;
	principal_data[0].exp_date = default_princ.exp_date;
	principal_data[0].max_life = default_princ.max_life;
	principal_data[0].attributes = default_princ.attributes;
	principal_data[0].kdc_key_ver = (unsigned char) master_key_version;
	principal_data[0].key_version = 0; /* bumped up later */
    }
    exp_date = principal_data[0].exp_date;
    tp = localtime(&exp_date);
    (void) sprintf(principal_data[0].exp_date_txt, "%4d-%02d-%02d",
		   tp->tm_year > 1900 ? tp->tm_year : tp->tm_year + 1900,
		   tp->tm_mon + 1, tp->tm_mday); /* January is 0, not 1 */
    for (i = 0; i < j; i++) {
	for (;;) {
	    fprintf(stdout,
		    "\nPrincipal: %s, Instance: %s, kdc_key_ver: %d",
		    principal_data[i].name, principal_data[i].instance,
		    principal_data[i].kdc_key_ver);
	    editpw = 1;
	    changed = 0;
	    if (!creating) {
		/*
		 * copy the existing data so we can use the old values
		 * for the qualifier clause of the replace 
		 */
		principal_data[i].old = (char *) &old_principal;
		memcpy(&old_principal, &principal_data[i],
		       sizeof(old_principal));
		printf("\nChange password [n] ? ");
		gets(temp);
		if (strcmp("y", temp) && strcmp("Y", temp))
		    editpw = 0;
	    }
	    /* password */
	    if (editpw) {
#ifdef NOENCRYPTION
		placebo_read_pw_string(pw_str, sizeof pw_str,
		    "\nNew Password: ", TRUE);
#else
		des_read_pw_string(pw_str, sizeof pw_str,
		    "\nNew Password: ", TRUE);
#endif
		if (!strcmp(pw_str, "RANDOM")) {
		    printf("\nRandom password [y] ? ");
		    gets(temp);
		    if (!strcmp("n", temp) || !strcmp("N", temp)) {
			/* no, use literal */
#ifdef NOENCRYPTION
			memset(new_key, 0, sizeof(C_Block));
			new_key[0] = 127;
#else
			string_to_key(pw_str, new_key);
#endif
			memset(pw_str, 0, sizeof pw_str);	/* "RANDOM" */
		    } else {
#ifdef NOENCRYPTION
			memset(new_key, 0, sizeof(C_Block));
			new_key[0] = 127;
#else
			des_new_random_key(new_key);	/* yes, random */
#endif
			memset(pw_str, 0, sizeof pw_str);
		    }
		} else if (!strcmp(pw_str, "NULL")) {
		    printf("\nNull Key [y] ? ");
		    gets(temp);
		    if (!strcmp("n", temp) || !strcmp("N", temp)) {
			/* no, use literal */
#ifdef NOENCRYPTION
			memset(new_key, 0, sizeof(C_Block));
			new_key[0] = 127;
#else
			string_to_key(pw_str, new_key);
#endif
			memset(pw_str, 0, sizeof pw_str);	/* "NULL" */
		    } else {

			principal_data[i].key_low = 0;
			principal_data[i].key_high = 0;
			goto null_key;
		    }
		} else {
#ifdef NOENCRYPTION
		    memset(new_key, 0, sizeof(C_Block));
		    new_key[0] = 127;
#else
		    string_to_key(pw_str, new_key);
#endif
		    memset(pw_str, 0, sizeof pw_str);
		}

		/* seal it under the kerberos master key */
		kdb_encrypt_key (new_key, new_key, 
				 master_key, master_key_schedule,
				 ENCRYPT);
		memcpy(&principal_data[i].key_low, new_key, sizeof(KRB_INT32));
		memcpy(&principal_data[i].key_high, ((KRB_INT32 *) new_key) + 1, sizeof(KRB_INT32));
		memset(new_key, 0, sizeof(new_key));
	null_key:
		/* set master key version */
		principal_data[i].kdc_key_ver =
		    (unsigned char) master_key_version;
		/* bump key version # */
		principal_data[i].key_version++;
		fprintf(stdout,
			"\nPrincipal's new key version = %d\n",
			principal_data[i].key_version);
		fflush(stdout);
		changed = 1;
	    }
	    /* expiration date */
	    fprintf(stdout, "Expiration date (enter yyyy-mm-dd) [ %s ] ? ",
		    principal_data[i].exp_date_txt);
	    zaptime(&edate);
	    while (gets(temp) && ((n = strlen(temp)) >
				  sizeof(principal_data[0].exp_date_txt))) {
	    bad_date:
		fprintf(stdout, "\07\07Date Invalid\n");
		fprintf(stdout,
			"Expiration date (enter yyyy-mm-dd) [ %s ] ? ",
			principal_data[i].exp_date_txt);
		zaptime(&edate);
	    }

	    if (*temp) {
		if (sscanf(temp, "%d-%d-%d", &edate.tm_year,
			      &edate.tm_mon, &edate.tm_mday) != 3)
		    goto bad_date;
		(void) strcpy(principal_data[i].exp_date_txt, temp);
		edate.tm_mon--;		/* January is 0, not 1 */
		edate.tm_hour = 23;	/* nearly midnight at the end of the */
		edate.tm_min = 59;	/* specified day */
		if (!(principal_data[i].exp_date = maketime(&edate, 1)))
		    goto bad_date;
		changed = 1;
	    }

	    /* maximum lifetime */
	    fprintf(stdout, "Max ticket lifetime (*5 minutes) [ %d ] ? ",
		    principal_data[i].max_life);
	    while (gets(temp) && *temp) {
		if (sscanf(temp, "%d", &temp_int) != 1)
		    goto bad_life;
		if (temp_int > 255 || (temp_int < 0)) {
		bad_life:
		    fprintf(stdout, "\07\07Invalid, choose 0-255\n");
		    fprintf(stdout,
			    "Max ticket lifetime (*5 minutes) [ %d ] ? ",
			    principal_data[i].max_life);
		    continue;
		}
		changed = 1;
		/* dont clobber */
		principal_data[i].max_life = (unsigned short) temp_int;
		break;
	    }

	    /* attributes */
	    fprintf(stdout, "Attributes [ %d ] ? ",
		    principal_data[i].attributes);
	    while (gets(temp) && *temp) {
		if (sscanf(temp, "%d", &temp_int) != 1)
		    goto bad_att;
		if (temp_int > 65535 || (temp_int < 0)) {
		bad_att:
		    fprintf(stdout, "\07\07Invalid, choose 0-65535\n");
		    fprintf(stdout, "Attributes [ %d ] ? ",
			    principal_data[i].attributes);
		    continue;
		}
		changed = 1;
		/* dont clobber */
		principal_data[i].attributes =
		    (unsigned short) temp_int;
		break;
	    }

	    /*
	     * remaining fields -- key versions and mod info, should
	     * not be directly manipulated 
	     */
	    if (changed) {
		if (kerb_put_principal(&principal_data[i], 1)) {
		    fprintf(stdout,
			"\nError updating Kerberos database");
		} else {
		    fprintf(stdout, "Edit O.K.");
		}
	    } else {
		fprintf(stdout, "Unchanged");
	    }


	    memset(&principal_data[i].key_low, 0, 4);
	    memset(&principal_data[i].key_high, 0, 4);
	    fflush(stdout);
	    break;
	}
    }
    if (more) {
	fprintf(stdout, "\nThere were more tuples found ");
	fprintf(stdout, "than there were space for");
      }
    return 1;
}


no_core_dumps()
{

    signal(SIGQUIT, sig_exit);
    signal(SIGILL, sig_exit);
    signal(SIGTRAP, sig_exit);
    signal(SIGIOT, sig_exit);
#ifdef SIGEMT
    signal(SIGEMT, sig_exit);
#endif
    signal(SIGFPE, sig_exit);
#ifdef SIGBUS
    signal(SIGBUS, sig_exit);
#endif
    signal(SIGSEGV, sig_exit);
#ifdef SIGSYS
    signal(SIGSYS, sig_exit);
#endif
}

void
sig_exit(sig, code, scp)
    int     sig, code;
    struct sigcontext *scp;
{
    cleanup();

#if defined(_AIX)
#if defined(i386)
    fprintf(stderr, "\nSignal caught, sig = %d code = %d\nexiting", sig, code);
#else
    fprintf(stderr,
	"\nSignal caught, sig = %d code = %d old pc = 0x%X \nexiting",
        sig, code, scp->sc_jmpbuf.jmp_context.iar);
#endif
#else /* !_AIX */
#ifdef NO_SIGCONTEXT
    fprintf(stderr,
	"\nSignal caught, sig = %d code = %d \nexiting",
        sig, code);
#else
    fprintf(stderr,
	"\nSignal caught, sig = %d code = %d old pc = 0x%X \nexiting",
        sig, code, scp->sc_pc);
#endif
#endif
    exit(-1);
}


cleanup()
{

    memset(master_key, 0, sizeof(master_key));
    memset(session_key, 0, sizeof(session_key));
    memset(master_key_schedule, 0, sizeof(master_key_schedule));
    memset(principal_data, 0, sizeof(principal_data));
    memset(new_key, 0, sizeof(new_key));
    memset(pw_str, 0, sizeof(pw_str));
}
Usage()
{
    fprintf(stderr,
	    "Usage: %s [-n] [-k mkeyfile] [database_pathname]\n",
	    progname);
    exit(1);
}
