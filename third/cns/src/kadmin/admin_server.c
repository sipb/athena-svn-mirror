/*
 * admin_server.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Top-level loop of the kerberos Administration server
 * this holds the main loop and initialization and cleanup code for the server
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#ifndef sigmask
#define sigmask(m)	(1 <<((m)-1))
#endif
#include <sys/wait.h>
#include <unistd.h>		/* for F_OK */
#include <errno.h>
#include <syslog.h>
#include <kadm.h>
#include <kadm_err.h>
#include <krb_db.h>
#include "kadm_server.h"

#ifdef hpux
#define HAVE_SIGSET
#endif

#ifdef solaris20
#define HAVE_SIGSET
#endif

extern char *error_message ();

/* Almost all procs and such need this, so it is global */
admin_params prm;		/* The command line parameters struct */

char prog[32];			/* WHY IS THIS NEEDED??????? */
char *progname = prog;
char *acldir = DEFAULT_ACL_DIR;
char krbrlm[REALM_SZ];
char *gecos_file;
extern Kadm_Server server_parm;
int admin_port = 0;

/*
** Main does the logical thing, it sets up the database and RPC interface,
**  as well as handling the creation and maintenance of the syslog file...
*/
main(argc, argv)		/* admin_server main routine */
int argc;
char *argv[];
{
    int errval;
    int c;
    extern char *optarg;
    extern int fascist_cpw;
    char *kfile = 0;

    prog[sizeof(prog)-1]='\0';		/* Terminate... */
    (void) strncpy(prog, argv[0], sizeof(prog)-1);

    /* initialize the admin_params structure */
    prm.sysfile = KADM_SYSLOG;		/* default file name */
    prm.inter = 1;

    memset(krbrlm, 0, sizeof(krbrlm));

    while ((c = getopt(argc, argv, "f:hnd:a:r:FNu:k:G:")) != EOF)
	switch(c) {
	case 'f':			/* Syslog file name change */
	    prm.sysfile = optarg;
	    break;
	case 'n':
	    prm.inter = 0;
	    break;
	case 'a':			/* new acl directory */
	    acldir = optarg;
	    break;
	case 'd':
	    /* put code to deal with alt database place */
	    if (errval = kerb_db_set_name(optarg)) {
		fprintf(stderr, "opening database %s: %s",
			optarg, error_message(errval));
		exit(1);
	    }
	    break;
        case 'F':
	    fascist_cpw++;
	    break;
        case 'N':
	    fascist_cpw = 0;
	    break;
	case 'r':
	    (void) strncpy(krbrlm, optarg, sizeof(krbrlm) - 1);
	    break;
	case 'u':
	    admin_port = htons(atoi(optarg)+1);
	    break;
	case 'k':
	    kfile = optarg;
	    break;
	case 'G':
#ifdef HAVE_FGETPWENT
	    gecos_file = optarg;
	    if (access (optarg, F_OK)) {
	      perror (optarg);
	      exit (1);
	    }
#else
	    fprintf (stderr,
		     "separate GECOS file not supported in this configuration\n");
	    exit (1);
#endif
	    break;
	case 'h':			/* get help on using admin_server */
	default:
	    fprintf(stderr, "Usage:\n %s [-h] [-n] [-F] [-N]\n\t[-r realm] [-d dbname] [-f logfile] [-a acldir]\n\t[-u kdc port] [-k master key file]\n",
		    argv[0]);
	    exit(-1);			/* failure */
	}

    if (krbrlm[0] == 0)
	if (krb_get_lrealm(krbrlm, 0) != KSUCCESS) {
	    fprintf(stderr, 
		    "Unable to get local realm.  Fix krb.conf or use -r.\n");
	    exit(1);
	}

    printf("KADM Server %s initializing\n",KADM_VERSTR);
    printf("Please do not use 'kill -9' to kill this job, use a\n");
    printf("regular kill instead\n\n");

    printf("KADM Server starting in %s mode for the purposes for password changing\n\n", fascist_cpw ? "fascist" : "NON-FASCIST");
    if (admin_port)
	printf("Listening on %d instead of default\n", ntohs(admin_port));

    krb_set_logfile(prm.sysfile);
    krb_log("Admin server starting");

    (void) kerb_db_set_lockmode(KERB_DBL_NONBLOCKING);
    errval = kerb_init();		/* Open the Kerberos database */
    if (errval) {
	fprintf(stderr, "error: kerb_init() failed");
	close_syslog();
	byebye();
    }
    /* set up the server_parm struct */
    if ((errval = kadm_ser_init(prm.inter, krbrlm, kfile))==KADM_SUCCESS) {
	/* override listening port if requested */
	if (admin_port) server_parm.admin_addr.sin_port = admin_port;
	kerb_fini();			/* Close the Kerberos database--
					   will re-open later */
	errval = kadm_listen();		/* listen for calls to server from
					   clients */
    }
    if (errval != KADM_SUCCESS) {
	fprintf(stderr,"error:  %s\n",error_message(errval));
	kerb_fini();			/* Close if error */
    }
    close_syslog();			/* Close syslog file, print
					   closing note */
    byebye();				/* Say bye bye on the terminal
					   in use */
}					/* procedure main */


/* close the system log file */
close_syslog()
{
   krb_log("Shutting down admin server");
}

byebye()			/* say goodnight gracie */
{
   printf("Admin Server (kadm server) has completed operation.\n");
}

static clear_secrets()
{
    memset((char *)server_parm.master_key, 0, sizeof(server_parm.master_key));
    memset((char *)server_parm.master_key_schedule, 0,
	  sizeof(server_parm.master_key_schedule));
    server_parm.master_key_version = 0L;
    return;
}

static exit_now = 0;

sigtype
doexit(xxx)
     int xxx;
{
    exit_now = 1;
}
   
unsigned pidarraysize = 0;
int *pidarray = (int *)0;
int unknown_child = 0;

/*
kadm_listen
listen on the admin servers port for a request
*/
kadm_listen()
{
    extern int errno;
    int found;
    int admin_fd;
    int peer_fd;
    fd_set mask, readfds;
    struct sockaddr_in peer;
    int addrlen;
    void process_client(), kill_children();
    int pid;
    sigtype do_child();
    extern char *malloc();
    extern char *realloc();

    (void) signal(SIGINT, doexit);
    (void) signal(SIGTERM, doexit);
    (void) signal(SIGHUP, doexit);
    (void) signal(SIGQUIT, doexit);
    (void) signal(SIGPIPE, SIG_IGN); /* get errors on write() */
    (void) signal(SIGALRM, doexit);
#ifdef HAVE_SIGSET
    {
      struct sigaction new_act;
      new_act.sa_handler = do_child;
      sigemptyset(&new_act.sa_mask);
      sigaction(SIGCHLD, &new_act, 0);
    }
#else
    (void) signal(SIGCHLD, do_child);
#endif

    if ((admin_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return KADM_NO_SOCK;
    if (bind(admin_fd, (struct sockaddr *)&server_parm.admin_addr,
	     sizeof(struct sockaddr_in)) < 0)
	return KADM_NO_BIND;
    (void) listen(admin_fd, 1);
    FD_ZERO(&mask);
    FD_SET(admin_fd, &mask);

    for (;;) {				/* loop nearly forever */
	if (exit_now) {
	    clear_secrets();
	    kill_children();
	    return(0);
	}
	readfds = mask;
	if ((found = select(admin_fd+1,&readfds,(fd_set *)0,
			    (fd_set *)0, (struct timeval *)0)) == 0)
	    continue;			/* no things read */
	if (found < 0) {
	    if (errno != EINTR)
		krb_log("select: %s",error_message(errno));
	    continue;
	}      
	if (FD_ISSET(admin_fd, &readfds)) {
	    /* accept the conn */
	    addrlen = sizeof(peer);
	    if ((peer_fd = accept(admin_fd, (struct sockaddr *)&peer,
				  &addrlen)) < 0) {
		krb_log("accept: %s",error_message(errno));
		continue;
	    }
#ifndef DEBUG
	    /* if you want a sep daemon for each server */
	    if (pid = fork()) {
		/* parent */
		if (pid < 0) {
		    krb_log("fork: %s",error_message(errno));
		    (void) close(peer_fd);
		    continue;
		}
		/* fork succeeded: keep tabs on child */
		(void) close(peer_fd);
	  	if (unknown_child != pid) {
		    if (pidarray) {
			pidarray = (int *)realloc((char *)pidarray,
					(++pidarraysize) * sizeof(int));
			pidarray[pidarraysize-1] = pid;
		    } else {
			pidarray = (int *)malloc((pidarraysize = 1) * sizeof(int));
			pidarray[0] = pid;
		    }
		}
	    } else {
		/* child */
		(void) close(admin_fd);
#endif /* DEBUG */
		/* do stuff */
		process_client (peer_fd, &peer);
#ifndef DEBUG
	    }
#endif
	} else {
	    krb_log("something else woke me up!");
	    return(0);
	}
    }
    /*NOTREACHED*/
}

#ifdef DEBUG
#define cleanexit(code) {kerb_fini(); return;}
#endif

void
process_client(fd, who)
int fd;
struct sockaddr_in *who;
{
    u_char *dat;
    int dat_len;
    u_short dlen;
    int retval;
    int on = 1;
    Principal service;
    des_cblock skey;
    int more;
    int status;
    extern char *malloc();

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on)) < 0)
	krb_log("setsockopt keepalive: %d",errno);

    server_parm.recv_addr = *who;

    if (kerb_init()) {			/* Open as client */
	krb_log("can't open krb db");
	cleanexit(1);
    }
    /* need to set service key to changepw.KRB_MASTER */

    status = kerb_get_principal(server_parm.sname, server_parm.sinst, &service,
			    1, &more);
    if (status == -1) {
      /* db locked */
      unsigned KRB_INT32 retcode = KADM_DB_INUSE;
      char *pdat;
      
      dat_len = KADM_VERSIZE + sizeof(retcode);
      dat = (u_char *) malloc((unsigned)dat_len);
      pdat = (char *) dat;
      retcode = htonl((u_long) KADM_DB_INUSE);
      (void) strncpy(pdat, KADM_ULOSE, KADM_VERSIZE);
      memcpy(&pdat[KADM_VERSIZE], (char *)&retcode, sizeof(retcode));
      goto out;
    } else if (!status) {
      krb_log("no service %s.%s",server_parm.sname, server_parm.sinst);
      cleanexit(2);
    }

    memcpy((char *)skey, (char *)&service.key_low, sizeof(KRB_INT32));
    memcpy((char *)(((KRB_INT32 *) skey) + 1), (char *)&service.key_high, 
    	   sizeof(KRB_INT32));
    memset((char *)&service, 0, sizeof(service));
    kdb_encrypt_key (skey, skey, server_parm.master_key,
		     server_parm.master_key_schedule, DECRYPT);
    (void) krb_set_key((char *)skey, 0); /* if error, will show up when
					    rd_req fails */
    memset((char *)skey, 0, sizeof(skey));

    while (1) {
	if ((retval = krb_net_read(fd, (char *)&dlen, sizeof(u_short))) !=
	    sizeof(u_short)) {
	    if (retval < 0)
		krb_log("dlen read: %s",error_message(errno));
	    else if (retval)
		krb_log("short dlen read: %d",retval);
	    (void) close(fd);
	    cleanexit(retval ? 3 : 0);
	}
	if (exit_now) {
	    cleanexit(0);
	}
	dat_len = (int) ntohs(dlen);
	dat = (u_char *) malloc((unsigned)dat_len);
	if (!dat) {
	    krb_log("malloc: No memory");
	    (void) close(fd);
	    cleanexit(4);
	}
	if ((retval = krb_net_read(fd, (char *)dat, dat_len)) != dat_len) {
	    if (retval < 0)
		krb_log("data read: %s",error_message(errno));
	    else
		krb_log("short read: %d vs. %d", dat_len, retval);
	    (void) close(fd);
	    cleanexit(5);
	}
    	if (exit_now) {
	    cleanexit(0);
	}
	if ((retval = kadm_ser_in(&dat,&dat_len)) != KADM_SUCCESS)
	    krb_log("processing request: %s", error_message(retval));
    
	/* kadm_ser_in did the processing and returned stuff in
	   dat & dat_len , return the appropriate data */
    
    out:
	dlen = (u_short) dat_len;

	if (dat_len != (int)dlen) {
	    clear_secrets();
	    abort();			/* XXX */
	}
	dlen = htons(dlen);
    
	if (krb_net_write(fd, (char *)&dlen, sizeof(u_short)) < 0) {
	    krb_log("writing dlen to client: %s",error_message(errno));
	    (void) close(fd);
	    cleanexit(6);
	}
    
	if (krb_net_write(fd, (char *)dat, dat_len) < 0) {
	    /* this used to use LOG_ERR -- but this isn't syslog! */
	    krb_log("writing to client: %s",error_message(errno));
	    (void) close(fd);
	    cleanexit(7);
	}
	free((char *)dat);
    }
    /*NOTREACHED*/
}

sigtype
do_child()
{
    /* SIGCHLD brings us here */
    int pid;
    register int i, j;

#ifdef WAIT_USES_INT
    int status;
#else      
    union wait status;
#endif

    pid = wait(&status);

    for (i = 0; i < pidarraysize; i++)
	if (pidarray[i] == pid) {
	    /* found it */
	    for (j = i; j < pidarraysize-1; j++)
		/* copy others down */
		pidarray[j] = pidarray[j+1];
	    pidarraysize--;
#ifdef WAIT_USES_INT
	    if (WIFEXITED(status) || WIFSIGNALED(status))
		krb_log("child %d: termsig %d, retcode %d", pid,
		    WTERMSIG(status), WEXITSTATUS(status));
#else
	    if (status.w_retcode || status.w_coredump || status.w_termsig)
		krb_log("child %d: termsig %d, coredump %d, retcode %d", pid,
		    status.w_termsig, status.w_coredump, status.w_retcode);
#endif
	    goto re_turn;
	}
    unknown_child = pid;
#ifdef WAIT_USES_INT
    krb_log("child %d not in list: termsig %d, retcode %d", pid,
	WTERMSIG(status), WEXITSTATUS(status));
#else
    krb_log("child %d not in list: termsig %d, coredump %d, retcode %d", pid,
	status.w_termsig, status.w_coredump, status.w_retcode);
#endif
re_turn:
    ;
}

#ifndef DEBUG
cleanexit(val)
{
    kerb_fini();
    clear_secrets();
    exit(val);
}
#endif

void
kill_children()
{
    register int i;
    sigmasktype mask;

    SIGBLOCK(mask, SIGCHLD);

    for (i = 0; i < pidarraysize; i++) {
	kill(pidarray[i], SIGINT);
	krb_log("killing child %d", pidarray[i]);
    }

    SIGSETMASK(mask);
    return;
}
