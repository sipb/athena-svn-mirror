/*
 * kadmin/get_srvtab.c
 *
 * Copyright 1992 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Remotely generate a srvtab file for a particular host.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/file.h>
#include <pwd.h>
#include <sys/wait.h>
#include <errno.h>
#include <syslog.h>
#include <kadm.h>
#include <kadm_err.h>
#include <krb_err.h>

#include "kadm_server.h"

#define MAX_SERVICES 16
#define MAX_HOSTS 128

int	n_services, n_hosts;
char	*service_names[MAX_SERVICES], *host_names[MAX_HOSTS];
int	install_opt = 0;
int	force_opt = 0;

char myname[ANAME_SZ];
char myinst[INST_SZ];
char myrealm[REALM_SZ];

char	realm[REALM_SZ];
char	*progname;

#define TRUE 1
#define FALSE 0

#define BAD_PW 1
#define GOOD_PW 0

#define PE_NO 0
#define PE_YES 1
#define PE_UNSURE 2

void clean_up();

usage()
{
	fprintf (stderr,
		 "Usage: %s [-f] [-s service ...] [-p principal] [-r realm] [-h] host ...\n",
		 progname);
	fprintf(stderr, "   or: %s -i [-f] [-p principal] [-r realm] -s service ...\n",
		progname);
	fprintf(stderr,
		"\tIf no services are specified, defaults are rcmd and rvdsrv\n");
	exit (3);
}

PRS(argc, argv)
	int argc;
	char **argv;
{
	int reading_service, saw_service_arg, ret;
	struct passwd *pw;
	char	*user_env;
	char	tktstring[MAXPATHLEN];
	extern char *getenv();

	progname = argv[0];
	
	memset(realm, 0, sizeof(realm));
	reading_service = FALSE;
	saw_service_arg = FALSE;
 
	n_services = 2;
	n_hosts = 0;

	service_names[0] = "rcmd";
	service_names[1] = "rvdsrv";

	myname[0] = myinst[0] = myrealm[0] = 0;

	while (--argc) {
		argv++;
		if (strcmp (*argv, "-s") == 0) {
			/* this is the only way not to include rcmd */
			/* and rvdsrv or to add other services */
			if (!saw_service_arg) {
				saw_service_arg = TRUE;
				n_services = 0;
			}
			reading_service = TRUE;
			continue;
		}
		if (strcmp (*argv, "-i") == 0) {
			install_opt = TRUE;
			reading_service = FALSE;
			continue;
		}
		if (strcmp (*argv, "-f") == 0) {
			force_opt = TRUE;
			reading_service = FALSE;
			continue;
		}
		if (strcmp (*argv, "-h") == 0) {
			reading_service = FALSE;
			continue;
		}
		if (strcmp (*argv, "-p") == 0) {
			reading_service = FALSE;
			argc--; argv++;
			if (!argc) {
				fprintf (stderr,
					 "%s: -p given but no principal.\n",
					 progname);
				exit(2);
			}
			ret = kname_parse(myname, myinst, myrealm, *argv);
			if (ret) {
				fprintf(stderr,
					"%s: invalid principal format.\n",
					progname);
				exit(2);
			}
			continue;
		}
		if (strcmp (*argv, "-r") == 0) {
			reading_service = FALSE;
			argc--; argv++;
			if (!argc) {
				fprintf (stderr,
					 "%s: -r given but no realm.\n",
					 progname);
				exit (2);
			}
			strcpy (realm, *argv);
			continue;
		}
		if (reading_service) {
			if (n_services == MAX_SERVICES) {
				fprintf (stderr,
					 "%s, Max number (%i) of services exceeded, need to recompile.\n",
					 progname, MAX_SERVICES);
				exit(1);
			}
			service_names[n_services++] = *argv;
		}
		/* otherwise default to reading a host name */
		else { 
			if (*argv[0] == '-')
				usage();
			if (n_hosts == MAX_HOSTS) {
				fprintf (stderr, "%s, Max number (%i) of hosts exceeded, need to recompile.\n",
					 progname, MAX_HOSTS);
				exit(1);
			}
			host_names[n_hosts++] = *argv;
		}
	}

	if (n_services <= 0)
		usage();
	
	if (install_opt) {
		if (n_hosts > 0 || realm[0])
			usage();
		if (getuid()) {
			fprintf(stderr,
				"Sorry, you must be root to use the install option.\n");
			exit(4);
		}
	} else {
		if (n_hosts <= 0)
			usage();
	}

	if (!realm[0])
		if (krb_get_lrealm(realm, 1) != KSUCCESS) {
			fprintf(stderr, "%s: couldn't get local realm\n",
				progname);
			exit(1);
		}

	if (!myname[0]) {
		user_env = getenv("USER");
		if (user_env)
			(void) strcpy(myname, user_env);
		else {
			pw = getpwuid((int) getuid());
			if (!pw) {
				fprintf(stderr,
					"You aren't in the password file.  Who are you?\n");
				exit(1);
			}
			(void) strcpy(myname, pw->pw_name);
		}
		(void) strcpy(myinst, "root");
		(void) strcpy(myrealm, realm);
	}

	if (!myrealm[0]) {
		strcpy(myrealm, realm);
	}

	ret = kadm_init_link(PWSERV_NAME, KRB_MASTER, realm);
	if (ret != KADM_SUCCESS) {
		fprintf(stderr, "%s: Couldn't initialize kadmin link: %s\n",
			progname, error_message(ret));
	    exit(1);
	}
	
	(void) umask(077);

	/*
	 *  OK, create the temporary ticket file; and get the tickets
	 */
	(void) sprintf(tktstring, "/tmp/tkt_adm_%d", getpid());
	krb_set_tkt_string(tktstring);
	ret = get_admin_password();
	if (ret) {
		fprintf(stderr, "%s: Couldn't get admin password.\n",
			progname);
		clean_up();
		exit(1);
	}
	return 0;
}

main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i, ret;
	char	my_hostname[MAXHOSTNAMELEN];
	
	PRS(argc, argv);
	
	if (install_opt) {
		ret = gethostname(my_hostname, sizeof(my_hostname));
		if (ret) {
			perror("gethostname");
			clean_up();
			exit(1);
		}
		strcpy(my_hostname, krb_get_phost(my_hostname));
		make_srvtab(service_names, n_services, my_hostname, 1);
	} else {
		for (i = 0; i < n_hosts; i++) {
			make_srvtab (service_names, n_services,
				     host_names[i], 0);
		}
	}
	clean_up();
	exit(0);
}

make_srvtab (services, n_services, host, install_it)
     char *services[];
     int n_services;
     char *host;
     int install_it;
{
	char	out_fn[MAXPATHLEN], buf[MAXPATHLEN];
	FILE	*fout;
	int	i, ret;
	int	failed = 0;
	int	warned = 0;
	Kadm_vals	values;
	struct hostent	*hp;
	char	hostbuf[MAXHOSTNAMELEN];

	hp = gethostbyname(host);
	if (!hp && !force_opt) {
		printf("\n\007The hostname '%s' does not exist in the DNS namespace!\n",
		       host);
		printf("Do you wish to proceed anyway(Y/N? ");
		fgets(buf, sizeof(buf), stdin);
		if ((buf[0] != 'Y') && (buf[0] != 'y')) {
			printf("Aborting generation of srvtab for %s.\n\n",
			       host);
			return -1;
		}
		printf("\n");
	}

	strncpy(hostbuf, krb_get_phost(host), sizeof(hostbuf));

	if (strcmp(hostbuf, host) && !force_opt) {
		printf("\n\007Warning: hostname has been '%s' cannonicalized to '%s'.\n", host,
		       hostbuf);
		printf("Do you wish to U)se '%s', K)eep old name, or A)bort? ",
		       hostbuf);
		fgets(buf, sizeof(buf), stdin);
		if ((buf[0] == 'U') || (buf[0] == 'u')) {
			host = hostbuf;
			printf("Using hostname %s...\n", host);
		} else if ((buf[0] != 'K') && (buf[0] != 'k')) {
			printf("Aborting generation of srvtab for %s.\n\n",
			       host);
			return -1;
		} else
			printf("Using original hostname of %s....\n", host);
	}
	
	if (install_it)
		sprintf(out_fn, "%s#new", KEYFILE);
	else
		sprintf(out_fn, "%s-new-srvtab", host);
	(void) unlink(out_fn);
	if ((fout = fopen(out_fn, "w")) == NULL) {
		fprintf(stderr, "Couldn't create file '%s'.\n", out_fn);
		return -1;
	}
	
	printf("\n\tMaking srvtab for '%s'....\n", host);

	for (i = 0; i < n_services; i++) {
		if (!princ_exists(services[i], host, realm)) {
			if (!warned)
				printf("\n\007Warning: the following service(s) do not yet exist:\n");
			printf("\t%s.%s\n", services[i], host);
			warned++;
		}
	}
	if (warned && !force_opt) {
		printf("If you proceed, they will be created.  Do you wish to proceed(Y/N)? ");
		fgets(buf, sizeof(buf), stdin);
		if ((buf[0] != 'Y') && (buf[0] != 'y')) {
			fclose(fout);
			unlink(out_fn);
			return -1;
		}
	}
	
	for (i = 0; i < n_services; i++) {
		ret = kadm_change_srvtab(services[i], host, &values);
		if (ret) {
			fprintf(stderr, "Couldn't get srvtab entry for %s.%s: %s\n",
				services[i], host, error_message(ret));
			failed++;
			continue;
		}

		/* convert to host order */
		values.key_low = ntohl(values.key_low);
		values.key_high = ntohl(values.key_high);
		
		fwrite(values.name, strlen(values.name) + 1, 1, fout);
		fwrite(values.instance, strlen(values.instance) + 1, 1, fout);
		fwrite(realm, strlen(realm) + 1, 1, fout);
		/* Note: really max_lifetime is really key_version */
		fwrite(&values.max_life, sizeof(values.max_life), 1, fout);
		fwrite(&values.key_low, sizeof(KRB_INT32), 1, fout);
		fwrite(&values.key_high, sizeof(KRB_INT32), 1, fout);
		values.key_low = values.key_high = 0;
		fprintf (stderr, "Written %s.%s\n", services[i], host);
	}
	fclose(fout);
	(void) chmod(out_fn, 0400);
	if (install_it) {
		if (failed) {
			fprintf(stderr, "\nThere were problems creating the srvtab file;\n");
			fprintf(stderr, "The new srvtab was left in file '%s'.\n",
				out_fn);
			return(0);
		}
		sprintf(buf, "%s.old", KEYFILE);
		ret = rename(KEYFILE, buf);
		if (ret) {
			perror("rename");
			fprintf(stderr, "Couldn't rename %s to %s.\n",
				KEYFILE, buf);
		}
		ret = rename(out_fn, KEYFILE);
		if (ret) {
			perror("rename");
			fprintf(stderr, "Couldn't rename %s to %s.\n",
				out_fn, KEYFILE);
		}
		printf("\nSuccessfully created srvtab file %s.\n\n",
		       KEYFILE);
		return(0);
	}
	if (failed) {
		printf("\nErrors encountered while creating srvtab file %s.\n",
		       out_fn);
		printf("The srvtab file may be incomplete.\n\n");
	} else {
		printf("\nSuccessfully created srvtab file %s.\n\n", out_fn);
	}
	return(0);
}

#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else
#define read_long_pw_string des_read_pw_string
#endif
extern int read_long_pw_string();

int
get_admin_password()
{
    int status;
    char admin_passwd[MAX_KPW_LEN];	/* Admin's password */
    char buf[1024];
    int ticket_life = 1;	/* minimum ticket lifetime */

    sprintf(buf, "Password for %s.%s@%s:", myname, myinst, myrealm);
    if (princ_exists(myname, myinst, myrealm) != PE_NO) {
	if (read_long_pw_string(admin_passwd, sizeof(admin_passwd)-1,
				buf, 0)) {
	    fprintf(stderr, "Error reading admin password.\n");
	    goto bad;
	}
	status = krb_get_pw_in_tkt(myname, myinst, myrealm, PWSERV_NAME, 
				   KADM_SINST, ticket_life, admin_passwd);
	memset(admin_passwd, 0, sizeof(admin_passwd));
    }
    else
	status = KDC_PR_UNKNOWN;

    switch(status) {
    case GT_PW_OK:
	return(GOOD_PW);
    case KDC_PR_UNKNOWN:
	printf("Principal %s%s%s@%s does not exist.\n", myname, 
	       myinst[0]?".":"", myinst, myrealm);
	goto bad;
    case GT_PW_BADPW:
	printf("Incorrect admin password.\n");
	goto bad;
    default:
	com_err(progname, status+krb_err_base,
		"while getting password tickets");
	goto bad;
    }
    
 bad:
    memset(admin_passwd, 0, sizeof(admin_passwd));
    (void) dest_tkt();
    return(BAD_PW);
}

void
clean_up()
{
    (void) dest_tkt();
    return;
}

void 
quit() 
{
    printf("Cleaning up and exiting.\n");
    clean_up();
    exit(0);
}

int
princ_exists(name, instance, realm)
  char *name;
  char *instance;
  char *realm;
{
    int status;

    status = krb_get_pw_in_tkt(name, instance, realm, "krbtgt", realm, 1, "");

    if ((status == KSUCCESS) || (status == INTK_BADPW))
	return(PE_YES);
    else if (status == KDC_PR_UNKNOWN)
	return(PE_NO);
    else
	return(PE_UNSURE);
}

