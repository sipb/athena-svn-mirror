/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/login/login.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/login/login.c,v 1.6 1987-08-06 16:42:55 rfrench Exp $
 */

#ifndef lint
static char *rcsid_login_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/login/login.c,v 1.6 1987-08-06 16:42:55 rfrench Exp $";
#endif	lint

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)login.c	5.15 (Berkeley) 4/12/86";
#endif not lint

/*
 * login [ name ]
 * login -r hostname (for rlogind)
 * login -k hostname (for Kerberos rlogind with password access)
 * login -K hostname (for Kerberos rlogind with restricted access)
 * login -h hostname (for telnetd, etc.)
 */

#include <sys/param.h>
#ifndef VFS
#include <sys/quota.h>
#endif !VFS
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/wait.h>

#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <strings.h>
#include <krb.h>	
#include <netdb.h>
#include <sys/types.h>
/*#include <netinet/in.h>*/
#include <grp.h>
#include <zephyr/zephyr.h>
typedef struct in_addr inaddr_t;
#include <attach.h>

#define TTYGRPNAME	"tty"		/* name of group to own ttys */
#define TTYGID(gid)	tty_gid(gid)	/* gid that owns all ttys */

#define	SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))

#define NMAX	sizeof(utmp.ut_name)
#define HMAX	sizeof(utmp.ut_host)

#define	FALSE	0
#define	TRUE	-1

#ifdef VFS
#define QUOTAWARN	"/usr/ucb/quota"	/* warn user about quotas */
#endif VFS

#ifndef KRB_REALM
#define KRB_REALM	"ATHENA.MIT.EDU"
#endif

#define KRB_ENVIRON	"KRBTKFILE" /* Ticket file environment variable */
#define KRB_TK_DIR	"/tmp/tkt_" /* Where to put the ticket */
#define KRBTKLIFETIME	96	/* 8 hours */

#define PROTOTYPE_DIR	"/usr/prototype_user" /* Source for temp files */
#define TEMP_DIR_PERM	0755	/* Permission on temporary directories */

#define MAXPWSIZE   	128	/* Biggest key getlongpass will return */

#define START_UID	200	/* start assigning arbitrary UID's here */
#define MIT_GID		101	/* standard primary group "mit" */

extern char *krb_err_txt[];	/* From libkrb */

char	nolog[] =	"/etc/nologin";
char	qlog[]  =	".hushlogin";
char	maildir[30] =	"/usr/spool/mail/";
char	lastlog[] =	"/usr/adm/lastlog";
char	inhibit[] =	"/etc/nocreate";
char	noattach[] =	"/etc/noattach";
char	go_register[] =	"/etc/athena/go_register";

/* uid, gid, etc. used to be -1; guess what setreuid does with that --asp */
struct	passwd nouser = {"", "nope", -2, -2, -2, "", "", "", "" };

struct	passwd newuser = {"\0\0\0\0\0\0\0\0", "*", START_UID, MIT_GID, 0,
			  NULL, NULL, "/mit/\0\0\0\0\0\0\0\0", NULL };

struct	sgttyb ttyb;
struct	utmp utmp;
char	minusnam[16] = "-";
char	*envinit[] = { 0 };		/* now set by setenv calls */
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
int	timeout = 60;

char	term[64];

struct	passwd *pwd;
struct	passwd *hes_getpwnam();
char	*strcat(), *rindex(), *index(), *malloc(), *realloc();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getlongpass();
char	*stypeof();
extern	char **environ;
extern	int errno;

struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

struct winsize win = { 0, 0, 0, 0 };

int	rflag=0;
int	kflag=0;
int 	Kflag=0;
int	usererr = -1;
int	krbflag = FALSE;	/* True if Kerberos-authenticated login */
int	tmppwflag = FALSE;	/* True if passwd entry is temporary */
int	tmpdirflag = FALSE;	/* True if home directory is temporary */
int	inhibitflag = FALSE;	/* inhibit account creation on the fly */
int	attachable = FALSE;	/* True if /etc/noattach doesn't exist */
int	attachedflag = FALSE;	/* True if homedir attached */
int	errorprtflag = FALSE;	/* True if login error already printed */
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
char	*rhost;

AUTH_DAT *kdata = (AUTH_DAT *)NULL;

union wait waitstat;

main(argc, argv)
    char *argv[];
{
    register char *namep;
    int pflag = 0, hflag = 0, t, f, c;
    int invalid, quietlog, forkval;
    FILE *nlfd;
    char *ttyn, *tty, saltc[2];
    long salt;
    int ldisc = 0, zero = 0, found = 0, i;
    char **envnew;

    signal(SIGALRM, timedout);
    alarm(timeout);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    setpriority(PRIO_PROCESS, 0, 0);
#ifndef VFS
    quota(Q_SETUID, 0, 0, 0);
#endif !VFS
    /*
     * -p is used by getty to tell login not to destroy the environment
     * -r is used by rlogind to cause the autologin protocol;
     * -k is used by klogind to cause the Kerberos autologin protocol;
     * -K is used by klogind to cause the Kerberos autologin protocol with
     *    restricted access.;
     * -h is used by other servers to pass the name of the
     * remote host to login so that it may be placed in utmp and wtmp
     */
    while (argc > 1) {
	if (strcmp(argv[1], "-r") == 0) {
	    if (rflag || kflag || Kflag || hflag) {
		printf("Only one of -r -k -K or -h allowed\n");
		exit(1);
	    }
	    rflag = 1;
	    usererr = doremotelogin(argv[2]);
	    SCPYN(utmp.ut_host, argv[2]);
	    argc -= 2;
	    argv += 2;
	    continue;
	}
		if (strcmp(argv[1], "-k") == 0) {
			if (rflag || kflag || Kflag || hflag) {
				printf("Only one of -r -k -K or -h allowed\n");
				exit(1);
			}
			kflag = 1;
			usererr = doKerberosLogin(argv[2]);
			SCPYN(utmp.ut_host, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-K") == 0) {
			if (rflag || kflag || Kflag || hflag) {
				printf("Only one of -r -k -K or -h allowed\n");
				exit(1);
			}
			Kflag = 1;
			usererr = doKerberosLogin(argv[2]);
			SCPYN(utmp.ut_host, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
	if (strcmp(argv[1], "-h") == 0 && getuid() == 0) {
	    if (rflag || kflag || Kflag || hflag) {
		printf("Only one of -r -k -K or -h allowed\n");
		exit(1);
	    }
	    hflag = 1;
	    SCPYN(utmp.ut_host, argv[2]);
	    argc -= 2;
	    argv += 2;
	    continue;
	}
	if (strcmp(argv[1], "-p") == 0) {
	    argc--;
	    argv++;
	    pflag = 1;
	    continue;
	}
	break;
    }
    ioctl(0, TIOCLSET, &zero);
    ioctl(0, TIOCNXCL, 0);
    ioctl(0, FIONBIO, &zero);
    ioctl(0, FIOASYNC, &zero);
    ioctl(0, TIOCGETP, &ttyb);
    /*
     * If talking to an rlogin process,
     * propagate the terminal type and
     * baud rate across the network.
     */
    if (rflag || kflag || Kflag)
	doremoteterm(term, &ttyb);
    ttyb.sg_erase = CERASE;
    ttyb.sg_kill = CKILL;
    ioctl(0, TIOCSLTC, &ltc);
    ioctl(0, TIOCSETC, &tc);
    ioctl(0, TIOCSETP, &ttyb);
    for (t = getdtablesize(); t > 2; t--)
	close(t);
    ttyn = ttyname(0);
    if (ttyn == (char *)0 || *ttyn == '\0')
	ttyn = "/dev/tty??";
    tty = rindex(ttyn, '/');
    if (tty == NULL)
	tty = ttyn;
    else
	tty++;
    openlog("login", LOG_ODELAY, LOG_AUTH);

    /* destroy environment unless user has asked to preserve it */
    /* (Moved before passwd stuff by asp) */
    if (!pflag)
	environ = envinit;

    /* set up environment, this time without destruction */
    /* copy the environment before setenving */
    i = 0;
    while (environ[i] != NULL)
	i++;
    envnew = (char **) malloc(sizeof (char *) * (i + 1));
    for (; i >= 0; i--)
	envnew[i] = environ[i];
    environ = envnew;

    t = 0;
    invalid = FALSE;
    inhibitflag = !access(inhibit,F_OK);
    attachable = access(noattach, F_OK);
    do {
	    errorprtflag = 0;
	    ldisc = 0;
	found = 0;
	ioctl(0, TIOCSETD, &ldisc);
	SCPYN(utmp.ut_name, "");
	/*
	 * Name specified, take it.
	 */
	if (argc > 1) {
	    SCPYN(utmp.ut_name, argv[1]);
	    argc = 0;
	}
	/*
	 * If remote login take given name,
	 * otherwise prompt user for something.
	 */
	if ((rflag || kflag || Kflag) && !invalid) {
	    SCPYN(utmp.ut_name, lusername);
	    if((pwd = getpwnam(lusername)) == NULL) {
		    pwd = &nouser;
		    found = 0;
	    } else found = 1;
	} else
	  found = getloginname(&utmp);
	invalid = FALSE;
	if (!strcmp(pwd->pw_shell, "/bin/csh")) {
	    ldisc = NTTYDISC;
	    ioctl(0, TIOCSETD, &ldisc);
	}
	/*
	 * If no remote login authentication and
	 * a password exists for this user, prompt
	 * for one and verify it.
	 */
	if (usererr == -1 && *pwd->pw_passwd != '\0') {
		/* we need to be careful to overwrite the password once it has
		 * been checked, so that it can't be recovered from a core image.
		 */

	    char *pp, pp2[MAXPWSIZE+1];
	    int krbval;
	    char tkfile[32];
	    char realm[REALM_SZ];
	    
	    /* Set up the ticket file environment variable */
	    SCPYN(tkfile, KRB_TK_DIR);
	    strncat(tkfile, rindex(ttyn, '/')+1,
		    sizeof(tkfile) - strlen(tkfile));
	    (void) unlink (tkfile);
	    setenv(KRB_ENVIRON, tkfile);
	    
	    setpriority(PRIO_PROCESS, 0, -4);
	    pp = getlongpass("Password:");
	    
	    if (!found) /* check if we can create an entry */
		    if (inhibitflag)
			invalid = TRUE;
		    else /* we are allowed to create an entry */
			pwd = &newuser;

	    /* Modifications for Kerberos authentication -- asp */
	    SCPYN(pp2, pp);
	    pp[8]='\0';
	    if (found)
		    namep = crypt(pp, pwd->pw_passwd);
	    else {
		    salt = 9 * getpid();
		    saltc[0] = salt & 077;
		    saltc[1] = (salt>>6) & 077;
		    for (i=0;i<2;i++) {
			    c = saltc[i] + '.';
			    if (c > '9')
				    c += 7;
			    if (c > 'Z')
				    c += 6;
			    saltc[i] = c;
		    }
		    pwd->pw_passwd = namep = crypt(pp, saltc);
	    } 
			    
	    bzero(pp, 8);		/* No, Senator, I don't recall
					   anything of that nature ... */
	    setpriority(PRIO_PROCESS, 0, 0);

	    if (!invalid && (pwd->pw_uid != 0)) { 
		    /* if not root, get Kerberos tickets */
		if(get_krbrlm(realm, 1) != KSUCCESS) {
		    SCPYN(realm, KRB_REALM);
		}
		strncpy(lusername, utmp.ut_name, NMAX);
		lusername[NMAX] = '\0';
		krbval = get_in_tkt(lusername, "", realm,
				    "krbtgt", realm, KRBTKLIFETIME, pp2);
		bzero(pp2, MAXPWSIZE+1); /* Yes, he's senile.  He doesn't know
					    what his administration is doing */
		switch (krbval) {
			case INTK_OK:
			invalid = FALSE;
			krbflag = TRUE;
			if (!found) {
				/* create a password entry: first ask the nameserver */
				/* to get us finger and shell info */
				struct passwd *nspwd;
				if ((nspwd = hes_getpwnam(lusername)) != NULL) {
					pwd->pw_uid = nspwd->pw_uid;
					pwd->pw_gid = nspwd->pw_gid;
					pwd->pw_gecos = nspwd->pw_gecos;
					pwd->pw_shell = nspwd->pw_shell;
				} else {
					pwd->pw_uid = 200;
					pwd->pw_gid = MIT_GID;
					pwd->pw_gecos = "";
					pwd->pw_shell = "/bin/csh";
				}
				strncpy(pwd->pw_name, utmp.ut_name, NMAX);
				strncat(pwd->pw_dir, utmp.ut_name, NMAX);
				(void) insert_pwent(pwd);
				tmppwflag = TRUE;
			}
			/* If we already have a homedir, use it.
			 * Otherwise, try to attach.  If that fails,
			 * try to create.
			 */
			tmpdirflag = FALSE;
			if (!goodhomedir()) {
				if (attach_homedir()) {
					if (make_homedir() >= 0) {
						puts("\nWARNING -- Your home directory is temporary.");
						puts("It will be deleted when this workstation deactivates.\n");
						tmpdirflag = TRUE;
					}
					else if (chdir("/") < 0) {
						printf("No directory!\n");
						invalid = TRUE;
					} else {
						puts("Can't find or build home directory! Logging in with home=/");
						pwd->pw_dir = "/";
						tmpdirflag = FALSE;
					}
				}
				else {
					attachedflag = TRUE;
				} 
			}
			break;
		    
		  case KDC_NULL_KEY:
			/* tell the luser to go register with kerberos */

			alarm(0);	/* If we are changing password,
					   he won't be logging in in this
					   process anyway, so we can reset */
			if (!found)
				(void) insert_pwent(pwd);
			
			if (forkval = fork()) { /* parent */
			    if (forkval < 0) {
				perror("forking for registration program");
				sleep(3);
				exit(1);
			    }
			    while(wait(0) != forkval);
			    if (!found)
				    remove_pwent(pwd);
			    exit(0);
			}
			/* run the passwd program as the user */
			setuid(pwd->pw_uid);
			
			execl(go_register, go_register, lusername, 0);
			perror("executing registration program");
			sleep(2);
			exit(1);
			/* These errors should be printed and are fatal */
		case KDC_PR_UNKNOWN:
		case INTK_BADPW:
		case KDC_PR_N_UNIQUE:
			invalid = TRUE;
			errorprtflag = TRUE;
			fprintf(stderr, "%s\n",
				krb_err_txt[krbval]);
			goto leavethis;
		    /* These should be printed but are not fatal */
		  case INTK_W_NOTALL:
		    invalid = FALSE;
		    krbflag = TRUE;
		    fprintf(stderr, "Kerberos error: %s\n",
			    krb_err_txt[krbval]);
			goto leavethis;
		  default:
		    fprintf(stderr, "Kerberos error: %s\n",
			    krb_err_txt[krbval]);
		    invalid = TRUE;
			errorprtflag = TRUE;
			goto leavethis;
		}
	} else { /* root logging in or inhibited; check password */
		bzero(pp2, MAXPWSIZE+1); /* Yes, he's senile.  He doesn't know
					  * what his administration is doing */
		invalid = TRUE;
	} 
	    /* if password is good, user is good */
	    invalid = invalid && strcmp(namep, pwd->pw_passwd);
    } 

leavethis:
	
	/*
	 * If our uid < 0, we must be a bogus user.
	 */
	if(pwd->pw_uid < 0) invalid = TRUE;

	/*
	 * If user not super-user, check for logins disabled.
	 */
	if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
	    while ((c = getc(nlfd)) != EOF)
		putchar(c);
	    fflush(stdout);
	    sleep(5);
	    if (krbflag)
		    (void) dest_tkt();
	    exit(0);
	}
	/*
	 * If valid so far and root is logging in,
	 * see if root logins on this terminal are permitted.
	 */
	if (!invalid && pwd->pw_uid == 0 && !rootterm(tty)) {
	    if (utmp.ut_host[0])
		syslog(LOG_CRIT,
		       "ROOT LOGIN REFUSED ON %s FROM %.*s",
		       tty, HMAX, utmp.ut_host);
	    else
		syslog(LOG_CRIT,
		       "ROOT LOGIN REFUSED ON %s", tty);
	    invalid = TRUE;
	}
	if (invalid) {
		if (!errorprtflag)
			printf("Login incorrect\n");
	    if (++t >= 5) {
		if (utmp.ut_host[0])
		    syslog(LOG_CRIT,
			   "REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
			   tty, HMAX, utmp.ut_host,
			   NMAX, utmp.ut_name);
		else
		    syslog(LOG_CRIT,
			   "REPEATED LOGIN FAILURES ON %s, %.*s",
			   tty, NMAX, utmp.ut_name);
		ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
		close(0), close(1), close(2);
		sleep(10);
		exit(1);
	    }
	}
	if (*pwd->pw_shell == '\0')
	    pwd->pw_shell = "/bin/sh";
	if (chdir(pwd->pw_dir) < 0 && !invalid ) {
	    if (chdir("/") < 0) {
		printf("No directory!\n");
		invalid = TRUE;
	    } else {
		puts("No directory! Logging in with home=/\n");
		pwd->pw_dir = "/";
	    }
	}
	/*
	 * Remote login invalid must have been because
	 * of a restriction of some sort, no extra chances.
	 */
	if (!usererr && invalid)
	    exit(1);

    } while (invalid);
    /* committed to login turn off timeout */
    alarm(0);

    if (tmppwflag) {
	    remove_pwent(pwd);
	    insert_pwent(pwd);
    } 

    if (!krbflag) puts("Warning: no Kerberos tickets obtained.");
    get_groups();
#ifndef VFS
    if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
	if (errno == EUSERS)
	    printf("%s.\n%s.\n",
		   "Too many users logged on already",
		   "Try again later");
	else if (errno == EPROCLIM)
	    printf("You have too many processes running.\n");
	else
	    perror("quota (Q_SETUID)");
	sleep(5);
	if (krbflag)
		(void) dest_tkt();
	exit(0);
    }
#endif VFS
    time(&utmp.ut_time);
    t = ttyslot();
    if (t > 0 && (f = open("/etc/utmp", O_WRONLY)) >= 0) {
	lseek(f, (long)(t*sizeof(utmp)), 0);
	SCPYN(utmp.ut_line, tty);
	write(f, (char *)&utmp, sizeof(utmp));
	close(f);
    }
    if ((f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND)) >= 0) {
	write(f, (char *)&utmp, sizeof(utmp));
	close(f);
    }
    quietlog = access(qlog, F_OK) == 0;
    if ((f = open(lastlog, O_RDWR)) >= 0) {
	struct lastlog ll;

	lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
	if (read(f, (char *) &ll, sizeof ll) == sizeof ll &&
	    ll.ll_time != 0 && !quietlog) {
	    printf("Last login: %.*s ",
		   24-5, (char *)ctime(&ll.ll_time));
	    if (*ll.ll_host != '\0')
	    printf("from %.*s\n",
		   sizeof (ll.ll_host), ll.ll_host);
	    else
	    printf("on %.*s\n",
		   sizeof (ll.ll_line), ll.ll_line);
	}
	lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
	time(&ll.ll_time);
	SCPYN(ll.ll_line, tty);
	SCPYN(ll.ll_host, utmp.ut_host);
	write(f, (char *) &ll, sizeof ll);
	close(f);
    }
    chown(ttyn, pwd->pw_uid, TTYGID(pwd->pw_gid));
    if(krbflag) chown(getenv(KRB_ENVIRON), pwd->pw_uid, pwd->pw_gid);

    if (!hflag && !rflag && !pflag && !kflag && !Kflag)		/* XXX */
	ioctl(0, TIOCSWINSZ, &win);
    chmod(ttyn, 0620);

    init_wgfile();
    
    /* Fork so that we can call kdestroy, notification server */
    dofork();
	
    setgid(pwd->pw_gid);
    strncpy(name, utmp.ut_name, NMAX);
    name[NMAX] = '\0';
    initgroups(name, pwd->pw_gid);
#ifndef VFS
    quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
#endif !VFS

    /* This call MUST succeed */
    if(setuid(pwd->pw_uid) < 0) {
	perror("setuid");
	if (krbflag)
		(void) dest_tkt();
	exit(1);
    }

    setenv("HOME", pwd->pw_dir, 1);
    setenv("SHELL", pwd->pw_shell, 1);
    if (term[0] == '\0')
	strncpy(term, stypeof(tty), sizeof(term));
    setenv("TERM", term, 0);
    setenv("USER", pwd->pw_name, 1);
    setenv("PATH", ":/usr/athena:/bin/athena:/usr/new:/usr/new/mh/bin:\
/usr/local:/usr/ucb:/bin:/usr/bin", 0);

    if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
	namep = pwd->pw_shell;
    else
	namep++;
    strcat(minusnam, namep);
    if (tty[sizeof("tty")-1] == 'd')
	syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
    if (pwd->pw_uid == 0)
	if (utmp.ut_host[0])
			if (kdata) {
				syslog(LOG_NOTICE, "ROOT LOGIN via Kerberos from %.*s",
					HMAX, utmp.ut_host);
				syslog(LOG_NOTICE, "     (name=%s, instance=%s, realm=%s).",
					kdata->pname, kdata->pinst, kdata->prealm );
			} else {
				syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
					tty, HMAX, utmp.ut_host);
			}
		else
			if (kdata) {
				syslog(LOG_NOTICE, "ROOT LOGIN via Kerberos %s ", tty);
				syslog(LOG_NOTICE, "     (name=%s, instance=%s, realm=%s).",
					kdata->pname,kdata->pinst,kdata->prealm);
			} else {
				syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);
			}
    if (!quietlog) {
	struct stat st;

	showmotd();
	strcat(maildir, pwd->pw_name);
	if (stat(maildir, &st) == 0 && st.st_size != 0)
	    printf("You have %smail.\n",
		   (st.st_mtime > st.st_atime) ? "new " : "");
    }
#ifdef VFS
    system(QUOTAWARN);
#endif VFS
    signal(SIGALRM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_IGN);
    execlp(pwd->pw_shell, minusnam, 0);
    perror(pwd->pw_shell);
    printf("No shell\n");
    if (krbflag)
	    (void) dest_tkt();
    exit(0);
}

getloginname(up)
	register struct utmp *up;
{
	register char *namep;
	int c;

	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
	}
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	if((pwd = getpwnam(lusername)) == NULL) {
	    pwd = &nouser;
	    return(0);			/* NOT FOUND */
	}
	return(1);			/* FOUND */
}

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
	exit(0);
}

int	stopmotd;
catch()
{

	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

rootterm(tty)
	char *tty;
{
	register struct ttyent *t;

	if ((t = getttynam(tty)) != NULL) {
		if (t->ty_status & TTY_SECURE)
			return (1);
	}
	return (0);
}

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if ((mf = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}

#undef	UNKNOWN
#define UNKNOWN "su"

char *
stypeof(ttyid)
	char *ttyid;
{
	register struct ttyent *t;

	if (ttyid == NULL || (t = getttynam(ttyid)) == NULL)
		return (UNKNOWN);
	return (t->ty_type);
}

doremotelogin(host)
	char *host;
{
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	if((pwd = getpwnam(lusername)) == NULL) {
	    pwd = &nouser;
	    return(-1);
	}
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername));
}

doKerberosLogin(host)
	char *host;
{
	int rc;

	kdata = (AUTH_DAT *)malloc( sizeof(AUTH_DAT) );
	if (rc=GetKerberosData( host, "rcmd", kdata )) {
		printf("Kerberos rlogin failed: %s\r\n",krb_err_txt[rc]);
		if (Kflag) {
			/*
			 * Paranoid hosts, such as a Kerberos server, specify the Klogind
			 * daemon to disallow even password access here.
			 */
			printf("Sorry, you must have Kerberos authentication to access this host.\r\n");
			exit(1);
		}
	}
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}

	/*
	 * if Kerberos login failed because of an error in GetKerberosData,
	 * return the indication of a bad attempt.  User will be prompted
	 * for a password.  We CAN'T check the .rhost file, because we need 
	 * the remote username to do that, and the remote username is in the 
	 * Kerberos ticket.  This affects ONLY the case where there is Kerberos 
	 * on both ends, but Kerberos fails on the server end. 
	 */
	if (rc) {
		return(-1);
	}

	if (rc=kuserok(kdata,lusername)) {
		printf("login: %s has not given you permission to login without a password.\r\n",lusername);
		if (Kflag) {
		  exit(1);
		}
		return(-1);
	}
	return(0);

}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	register char *cp = index(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

/* BEGIN TRASH
 *
 * This is here only long enough to get us by to the revised rlogin
 */
compatsiz(cp)
	char *cp;
{
	struct winsize ws;

	ws.ws_row = ws.ws_col = -1;
	ws.ws_xpixel = ws.ws_ypixel = -1;
	if (cp) {
		ws.ws_row = atoi(cp);
		cp = index(cp, ',');
		if (cp == 0)
			goto done;
		ws.ws_col = atoi(++cp);
		cp = index(cp, ',');
		if (cp == 0)
			goto done;
		ws.ws_xpixel = atoi(++cp);
		cp = index(cp, ',');
		if (cp == 0)
			goto done;
		ws.ws_ypixel = atoi(++cp);
	}
done:
	if (ws.ws_row != -1 && ws.ws_col != -1 &&
	    ws.ws_xpixel != -1 && ws.ws_ypixel != -1)
		ioctl(0, TIOCSWINSZ, &ws);
}
/* END TRASH */

/*
 * Set the value of var to be arg in the Unix 4.2 BSD environment env.
 * Var should NOT end in '='; setenv inserts it. 
 * (bindings are of the form "var=value")
 * This procedure assumes the memory for the first level of environ
 * was allocated using malloc.
 */
setenv(var, value, clobber)
	char *var, *value;
{
	extern char **environ;
	int index = 0;
	int varlen = strlen(var);
	int vallen = strlen(value);

	for (index = 0; environ[index] != NULL; index++) {
		if (strncmp(environ[index], var, varlen) == 0) {
			/* found it */
			if (!clobber)
				return;
			environ[index] = malloc(varlen + vallen + 2);
			strcpy(environ[index], var);
			strcat(environ[index], "=");
			strcat(environ[index], value);
			return;
		}
	}
	environ = (char **) realloc(environ, sizeof (char *) * (index + 2));
	if (environ == NULL) {
		fprintf(stderr, "login: malloc out of memory\n");
		if (krbflag)
			(void) dest_tkt();
		exit(1);
	}
	environ[index] = malloc(varlen + vallen + 2);
	strcpy(environ[index], var);
	strcat(environ[index], "=");
	strcat(environ[index], value);
	environ[++index] = NULL;
}


/*
 * This routine handles cleanup stuff, notification service, and the like.
 * It exits only in the child process.
 */
dofork()
{
    int child,retval,zephyrable,i,wgcpid;

    if((child=fork()) == 0) return; /* Child process */

    /* Setup stuff?  This would be things we could do in parallel with login */
    chdir("/");	/* Let's not keep the fs busy... */
    
    zephyrable = 1;
    if ((retval = ZInitialize()) != ZERR_NONE) {
	    com_err("login",retval,"initializing");
	    zephyrable = 0;
    }
    if (zephyrable && (retval = ZOpenPort((int *)0) != ZERR_NONE)) {
	    com_err("login",retval,"opening port");
	    zephyrable = 0;
    } 

    if (zephyrable && krbflag)
	    if (!fork()) {
		    setuid(pwd->pw_uid);
		    (void) ZSetLocation();
		    exit(0);
	    }

    wgcpid = 0;
    
    if (krbflag && !(wgcpid = fork())) {
	    setuid(pwd->pw_uid);
	    execl("/usr/etc/zwgc","zwgc",0);
	    exit (1);
    }

    if (krbflag && !fork()) {
	    setuid(pwd->pw_uid);
	    execl("/usr/athena/zinit","zinit",0);
	    exit (1);
    }
    
    /* If we're the parent, watch the child until it dies */
    while(wait(0) != child)
	    ;

    /* Cleanup stuff */

    /* Run dest_tkt to destroy tickets */
    (void) dest_tkt();		/* If this fails, we lose quietly */

    if (wgcpid > 0)
	    kill(wgcpid, SIGTERM);
    
    /* Detach home directory if previously attached */
    if (attachedflag)
	    (void) detach_homedir();

    if (!fork()) {
	    setuid(pwd->pw_uid);
	    (void) ZUnsetLocation();
	    exit(0);
    } 

    if (tmppwflag)
	    if (remove_pwent(pwd))
		    puts("Couldn't remove password entry");

    /* Leave */
    exit(0);
}


tty_gid(default_gid)
int default_gid;
{
    struct group *getgrnam(), *gr;
    int gid = default_gid;
    
    gr = getgrnam(TTYGRPNAME);
    if (gr != (struct group *) 0)
      gid = gr->gr_gid;
    
    endgrent();
    
    return (gid);
}

char *
getlongpass(prompt)
char *prompt;
{
	struct sgttyb ttyb;
	int flags;
	register char *p;
	register c;
	FILE *fi;
	static char pbuf[MAXPWSIZE+1];
	int (*signal())();
	int (*sig)();

	if ((fi = fdopen(open("/dev/tty", 2), "r")) == NULL)
		fi = stdin;
	else
		setbuf(fi, (char *)NULL);
	sig = signal(SIGINT, SIG_IGN);
	ioctl(fileno(fi), TIOCGETP, &ttyb);
	flags = ttyb.sg_flags;
	ttyb.sg_flags &= ~ECHO;
	ioctl(fileno(fi), TIOCSETP, &ttyb);
	fprintf(stderr, "%s", prompt); fflush(stderr);
	for (p=pbuf; (c = getc(fi))!='\n' && c!=EOF;) {
		if (p < &pbuf[MAXPWSIZE])
			*p++ = c;
	}
	*p = '\0';
	fprintf(stderr, "\n"); fflush(stderr);
	ttyb.sg_flags = flags;
	ioctl(fileno(fi), TIOCSETP, &ttyb);
	signal(SIGINT, sig);
	if (fi != stdin)
		fclose(fi);
	pbuf[MAXPWSIZE]='\0';
	return(pbuf);
}

/* Attach the user's home directory if "attachable" is set.
 */
attach_homedir()
{
	union wait status;
	int attachpid;
	
	if (!attachable)
		return (1);
	
	if (!(attachpid = fork())) {
		setuid(pwd->pw_uid);
		freopen("/dev/null","w",stdout);
		freopen("/dev/null","w",stderr);
		execl("/bin/athena/attach","attach",lusername,0);
		exit (-1);
	} 
	while (wait(&status) != attachpid)
		;
	if (status.w_status == ATTACH_OK ||
	    status.w_status == ATTACH_ERR_ATTACHED) {
		chown(pwd->pw_dir, pwd->pw_uid, pwd->pw_gid);
		chdir(pwd->pw_dir);
		return (0);
	}
	return (1);
} 

/* Detach the user's home directory */
detach_homedir()
{
	union wait status;
	char *level;
	int pid,i;

#ifdef notdef
	for (i=0;i<3;i++) {
#endif
		if (!(pid = fork())) {
			setuid(pwd->pw_uid);
			freopen("/dev/null","w",stdout);
			freopen("/dev/null","w",stderr);
			execl("/bin/athena/detach","detach",lusername,0);
			exit (-1);
		} 
		while (wait(&status) != pid)
			;
#ifdef notdef
		if (status.w_status == DETACH_OK)
			return;
		level = "1";
		if (i == 1)
			level = "9";
		if (i == 2)
			level = "9";
		printf("Killing processes using %s with signal %s\n",
		       pwd->pw_dir,level);
		if (!(pid = fork())) {
			freopen("/dev/null","w",stdout);
			freopen("/dev/null","w",stderr);
			execl("/etc/athena/ofiles","ofiles","-k",
			      level,pwd->pw_dir,0);
			exit (-1);
		}
		while (wait(0) != pid)
			;
	}
#endif notdef
	return;
#ifdef notdef
	printf("Couldn't detach home directory!\n");
#endif notdef
}

goodhomedir()
{
	DIR *dp;
	
	if (access(pwd->pw_dir,F_OK))
		return (0);
	dp = opendir(pwd->pw_dir);
	if (!dp)
		return (0);
	readdir(dp);
	readdir(dp);
	if (readdir(dp)) {
		closedir(dp);
		return (1);
	}
	closedir(dp);
	return (0);
}
	
/*
 * Make a home directory, copying over files from PROTOTYPE_DIR.
 * Ownership and group will be set to the user's uid and gid.  Default
 * permission is TEMP_DIR_PERM.  Returns 0 on success, -1 on failure.
 */
make_homedir()
{
    DIR *proto;
    struct direct *dp;
    char tempname[MAXPATHLEN+1];
    char buf[MAXBSIZE];
    struct stat statbuf;
    int fold, fnew;
    int n;
    extern int errno;

    if (inhibitflag)
	    return (-1);
    
    strcpy(pwd->pw_dir,"/tmp/");
    strcat(pwd->pw_dir,lusername);
    
    /* Make the home dir and chdir to it */
    unlink(pwd->pw_dir);
    if(mkdir(pwd->pw_dir) < 0) {
	    if (errno == EEXIST)
		    return (0);
	    else
		    return(-1);
    } 
    chown(pwd->pw_dir, pwd->pw_uid, pwd->pw_gid);
    chmod(pwd->pw_dir, TEMP_DIR_PERM);
    chdir(pwd->pw_dir);
    
    /* Copy over the proto files */
    if((proto = opendir(PROTOTYPE_DIR)) == NULL) {
	puts("Can't open prototype directory!");
	unlink(pwd->pw_dir);
	return(-1);
    }

    for(dp = readdir(proto); dp != NULL; dp = readdir(proto)) {
	/* Don't try to copy . or .. */
	if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

	/* Copy the file */
	SCPYN(tempname, PROTOTYPE_DIR);
	strcat(tempname, "/");
	strncat(tempname, dp->d_name, sizeof(tempname) - strlen(tempname) - 1);
	if(stat(tempname, &statbuf) < 0) {
	    perror(tempname);
	    continue;
	}
	/* Only copy plain files */
	if(!(statbuf.st_mode & S_IFREG)) continue;

	/* Try to open the source file */
	if((fold = open(tempname, O_RDONLY, 0)) < 0) {
	    perror(tempname);
	    continue;
	}

	/* Open the destination file */
	if((fnew = open(dp->d_name, O_WRONLY|O_CREAT|O_EXCL,
			statbuf.st_mode)) < 0) {
			    perror(dp->d_name);
			    continue;
			}

	/* Change the ownership */
	fchown(fnew, pwd->pw_uid, pwd->pw_gid);

	/* Do the copy */
	for (;;) {
	    n = read(fold, buf, sizeof buf);
	    if(n==0) break;
	    if(n<0) {
		perror(tempname);
		break;
	    }
	    if (write(fnew, buf, n) != n) {
		perror(dp->d_name);
		break;
	    }
	}
	close(fnew);
	close(fold);
    }
    return(0);
}

insert_pwent(pwd)
struct passwd *pwd;
{
    FILE *pfile;
    int cnt;

    while (getpwuid(pwd->pw_uid))
      (pwd->pw_uid)++;

    cnt = 10;
    while (!access("/etc/ptmp",0) && --cnt)
	    sleep(1);
    unlink("/etc/ptmp");
    
    if((pfile=fopen("/etc/passwd", "a")) != NULL) {
	fprintf(pfile, "%s:%s:%d:%d:%s:%s:%s\n",
		pwd->pw_name,
		pwd->pw_passwd,
		pwd->pw_uid,
		pwd->pw_gid,
		pwd->pw_gecos,
		pwd->pw_dir,
		pwd->pw_shell);
	fclose(pfile);
    }
}

remove_pwent(pwd)
struct passwd *pwd;
{
    FILE *newfile;
    struct passwd *copypw;
    int cnt;

    cnt = 10;
    while (!access("/etc/ptmp",0) && --cnt)
	    sleep(1);
    unlink("/etc/ptmp");
    
    if ((newfile = fopen("/etc/ptmp", "w")) != NULL) {
	setpwent();
	while ((copypw = getpwent()) != 0)
	    if (copypw->pw_uid != pwd->pw_uid)
		    fprintf(newfile, "%s:%s:%d:%d:%s:%s:%s\n",
			    copypw->pw_name,
			    copypw->pw_passwd,
			    copypw->pw_uid,
			    copypw->pw_gid,
			    copypw->pw_gecos,
			    copypw->pw_dir,
			    copypw->pw_shell);
	endpwent();
	fclose(newfile);
	rename("/etc/ptmp", "/etc/passwd");
	return(0);
    } else return(1);
}

get_groups()
{
	FILE *grin,*grout;
	char **cp,grbuf[4096],*ptr,*pwptr,*numptr,*lstptr,**grname,**grnum;
	char grlst[4096],grtmp[4096],*tmpptr;
	int ngroups,i,cnt;
	
	if (inhibitflag)
		return;
	
	cp = (char **)hes_resolve(pwd->pw_name,"grplist");
	if (!cp || !*cp)
		return;

	cnt = 10;
	while (!access("/etc/gtmp",0) && --cnt)
		sleep(1);
	unlink("/etc/gtmp");
	
	grin = fopen("/etc/group","r");
	if (!grin) {
		fprintf(stderr,"Can't open /etc/group!\n");
		return;
	}
	grout = fopen("/etc/gtmp","w");
	if (!grout) {
		fprintf(stderr,"Can't open /etc/gtmp!\n");
		fclose(grin);
		return;
	}

	ngroups = 0;
	for (ptr=cp[0];*ptr;ptr++)
		if (*ptr == ':')
			ngroups++;

	ngroups = (ngroups+1)/2;

	if (ngroups > NGROUPS-1)
		ngroups = NGROUPS-1;

	grname = (char **)malloc(ngroups * sizeof(char *));
	if (!grname) {
		fprintf(stderr,"Out of memory!\n");
		fclose(grin);
		fclose(grout);
		unlink("/etc/gtmp");
		return;
	}

	grnum = (char **)malloc(ngroups * sizeof(char *));
	if (!grnum) {
		fprintf(stderr,"Out of memory!\n");
		fclose(grin);
		fclose(grout);
		unlink("/etc/gtmp");
		return;
	}

	for (i=0,ptr=cp[0];i<ngroups;i++) {
		grname[i] = ptr;
		ptr = (char *)index(ptr,':');
		if (!ptr) {
			fprintf(stderr,"Internal failure while initializing groups\n");
			fclose(grin);
			fclose(grout);
			free(grname);
			free(grnum);
			unlink("/etc/gtmp");
			return;
		}
		*ptr++ = '\0';
		grnum[i] = ptr;
		ptr = (char *)index(ptr,':');
		if (!ptr)
			ptr = grnum[i]+strlen(grnum[i]);
		*ptr++ = '\0';
	}

	while (fgets(grbuf,sizeof grbuf,grin) > 0) {
		if (!*grbuf)
			break;
		grbuf[strlen(grbuf)-1] = '\0';
		pwptr = (char *)index(grbuf,':');
		if (!pwptr)
			continue;
		*pwptr++ = '\0';
		numptr = (char *)index(pwptr,':');
		if (!numptr)
			continue;
		*numptr++ = '\0';
		lstptr = (char *)index(numptr,':');
		if (!lstptr)
			continue;
		*lstptr++ = '\0';
		strcpy(grlst,lstptr);
		for (i=0;i<ngroups;i++) {
			if (strcmp(grname[i],grbuf))
				continue;
			lstptr = grlst;
			while (lstptr) {
				strcpy(grtmp,lstptr);
				tmpptr = (char *)index(grtmp,',');
				if (tmpptr)
					*tmpptr = '\0';
				if (!strcmp(grtmp,pwd->pw_name))
					break;
				lstptr = (char *)index(lstptr,',');
				if (lstptr)
					lstptr++;
			}
			if (lstptr)
				break;
			strcat(grlst,",");
			strcat(grlst,pwd->pw_name);
			grname[i] = "*";
			break;
		}
		fprintf(grout,"%s:%s:%s:%s\n",grbuf,pwptr,numptr,grlst);
	}

	for (i=0;i<ngroups;i++)
		if (strcmp(grname[i],"*"))
			fprintf(grout,"%s:%s:%s:%s\n",grname[i],"*",
				grnum[i],pwd->pw_name);

	fclose(grin);
	fclose(grout);
	rename("/etc/gtmp","/etc/group");
	unlink("/etc/gtmp");
	free(grname);
	free(grnum);
}

init_wgfile()
{
	char *wgfile;

	wgfile = "/tmp/wg.XXXXXX";

	mktemp(wgfile);

	setenv("WGFILE",wgfile,1);
}
