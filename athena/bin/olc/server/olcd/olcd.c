/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the primary functions of the daemon, olcd.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Steve Dyer
 *      IBM/MIT Project Athena
 *      converted to use Hesiod in place of clustertab
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/olcd.c,v $
 *	$Id: olcd.c,v 1.47 1991-11-05 13:50:50 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/olcd.c,v 1.47 1991-11-05 13:50:50 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/types.h>          /* Standard type defs. */
#include <sys/socket.h>		/* IPC definitions. */
#include <sys/file.h>		/* File I/O definitions. */
#include <sys/ioctl.h>		/* For handling TTY. */
#if defined(_AIX) && defined(_IBMR2)
#include <sys/select.h>
#include <mon.h>
#endif
#include <netinet/in.h>		/* More IPC definitions. */
#include <sys/wait.h>		/* Wait defs. */
#include <errno.h>		/* Standard error numbers. */
#include <pwd.h>		/* Password entry defintions. */
#include <signal.h>		/* Signal definitions. */
#include <strings.h>

#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif /* ZEPHYR */

#include <netdb.h>		/* Network database defs. */
#ifndef DIRSIZ
#include <sys/dir.h>		/* Directory entry format */
#endif
#include <arpa/inet.h>		/* inet_* defs */

#include <olcd.h>

/* Global variables. */

#ifdef NEEDS_SELECT_MACROS
#define NBBY    8 /* number of bits in a byte */
#define NFDBITS (sizeof(long) * NBBY)        /* bits per mask */

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))

#endif

extern PROC  Proc_List[];	/* OLC Proceedure Table */
char DaemonHost[LINE_SIZE];	/* Name of daemon's machine. */
struct sockaddr_in sin = { AF_INET }; /* Socket address. */
int request_count = 0;
int request_counts[OLC_NUM_REQUESTS];
long start_time;
int select_timeout = 60;
char DaemonInst[LINE_SIZE];	/* "olc", "olz", "olta", etc. */

#ifdef KERBEROS
static long ticket_time = 0L;	/* Timer on kerberos ticket */
char SERVER_REALM[REALM_SZ];
char K_INSTANCEbuf[INST_SZ];
extern int krb_ap_req_debug;
#endif /* KERBEROS */


#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static void process_request P((int fd , struct sockaddr_in *from ));
static void flush_olc_userlogs P((void ));
#ifdef VOID_SIGRET
static void reap_child P((int sig ));
static void punt P((int sig ));
#else
static int reap_child P((int sig ));
static int punt P((int sig ));
#endif
#ifdef PROFILE
#ifdef VOID_SIGRET
static void dump_profile P((int sig ));
static void start_profile P((int sig ));
#else
static int dump_profile P((int sig ));
static int start_profile P((int sig ));
#endif /* VOID_SIGRET */
#endif /* PROFILE */
#undef P

/* Static variables */

static int processing_request;
static int got_signal;
static int listening_fd;

/*
 * Function:	main() is the start-up function for the olcd daemon.
 * Arguments:	argc:	Number of command line arguments.
 *		argv:	Array of words from the command line.
 * Returns:	Never returns.
 * Notes:
 *	First, fork a new daemon to separate it from the terminal,
 *	rebinding the standard error output so we can trap it.
 *	Next, we create a socket in the Internet domain and get the
 *	appropriate host address and information, using the Hesiod
 *	database.  Then we initialize the user and consultant rings,
 *	and  begin the main loop.  The daemon listens to its socket,
 *	taking connections as it receives them.  Each connection is a request
 *	from an olc or olcr process, and these are handled through the
 *	request table as they are received.
 */

main (argc, argv)
    int argc;
    char **argv;
{
    struct sockaddr_in from;		/* Socket address for input. */
    struct servent *service;		/* Network service entry. */
    struct hostent *this_host_entry;	/* Host entry for this host. */
    struct hostent *daemon_host_entry;	/* Entry for daemon host.*/
    char hostname[MAXHOSTNAMELEN];	/* Name of this host. */
    char buf[BUFSIZ];			/* for constructing erorr messages */
    int fd;				/* Socket fd. */
    int onoff;				/* Value variable for setsockopt. */
    int n_errs=0;			/* Error count in accept() call */
    int arg=0;				/* Argument counter */
    int hostset = 0;			/* Flag if host was passed as arg */
    int nofork = 0;			/* Flag if you don't want to fork */
    int port_num = 0;			/* Port number explicitly requested */
    int i;
#ifdef ZEPHYR
    int ret;				/* return value from ZInitialize. */
#endif
#ifdef HESIOD
    char **hp;				/* return value of Hesiod resolver */
#endif
#ifdef _POSIX_SOURCE
    struct sigaction action;
#endif

#if defined(PROFILE)
    /* Turn off profiling on startup; that way, we collect "steady state" */
    /* statistics, not the initial flurry of startup activity */
    moncontrol(0);
#endif
#ifdef KERBEROS
    strcpy(K_INSTANCEbuf,K_INSTANCE);
    strcpy(SERVER_REALM,DFLT_SERVER_REALM);
#endif

    strcpy(DaemonInst, OLC_SERVICE); /* set default 'instance' -- "olc" */
    upcase_string(DaemonInst);

    /*
     * Parse any arguments
     */

    for (arg=1;arg< argc; arg++) {
	/*
	 * XXX: Check for parameter missing, values already specified,
	 * invalid values.
	 */
	if (!strcmp (argv[arg],"-h")
	    || !strcmp (argv[arg], "-host")) {
	    (void) strncpy(DaemonHost,argv[++arg],LINE_SIZE);
	    hostset = 1;
	    continue;
	}
	else if (!strcmp (argv[arg], "-nofork")
	    || !strcmp (argv[arg], "-no_fork")) {
	    nofork = 1;
	}
	else if (!strcmp (argv[arg], "-port")) {
	    if (!argv[++arg]) {
		fprintf (stderr, "-port requires port number\n");
		return 1;
	    }
	    port_num = htons (atoi (argv[arg]));
	}
	else if (!strcmp (argv[arg], "-inst")) {
	  if (!argv[++arg])
	    fprintf (stderr, "-inst requires an instance name\n");
	  else
	    strcpy(DaemonInst, argv[arg]);
	  upcase_string(DaemonInst);
	}
	else {
	    fprintf (stderr, "unknown argument: %s\n",argv[arg]);
	    return 1;
	}
    }

#ifdef KERBEROS
    setenv("KRBTKFILE",TICKET_FILE,TRUE);
#endif /* KERBEROS */

#ifdef ZEPHYR
	/** We must ZInitialize now, *before* we use "log_error" **/
	/** for  the first time, as errors are broadcast via zephyr. **/
    if ((ret = ZInitialize()) != ZERR_NONE) {
      char buf[BUFSIZ];

      sprintf(buf,"Error in ZInitialize: %d",ret);
      log_zephyr_error(buf);
    }
#endif /* ZEPHYR */  
    
    /*
     * fork off
     */

    if (!nofork) {
	int max_fd = getdtablesize ();

	switch (fork()) {
	case 0:				/* child */
	    break;
	case -1:			/* error */
	    log_error ("Can't fork: %m");
	    exit(-1);
	default:			/* parent */
	    exit(0);
	}

	for (fd = 0; fd < max_fd; fd++)
	    (void) close (fd);
	fd = open ("/", O_RDONLY);
	if (fd < 0) {
	    log_error("Can't open /");
	    return 1;
	}
	if (fd != 0)
	    dup2 (fd, 0);
	if (fd != 1)
	    dup2 (fd, 1);
	if (fd != 2)
	    dup2 (fd, 2);
	if (fd > 2)
	    close (fd);

	freopen(STDERR_LOG, "a", stderr);
	fd = open("/dev/tty", O_RDWR, 0);
	if (fd >= 0) {
	    ioctl(fd, TIOCNOTTY, (char *) NULL);
	    (void) close (fd);
	}
    }

#ifdef _AUX_SOURCE
    setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
    setvbuf(stderr,NULL,_IOLBF,BUFSIZ);
#else
    setlinebuf(stdout);
    setlinebuf(stderr);
#endif

    /* handle setuid-ness, etc., so we can dump core */
    setreuid((uid_t) geteuid(), -1);
    setregid((gid_t) getegid(), -1);

#ifndef	TEST
    (void) umask(066);		/* sigh */
#endif

    /*
     * allocate lists
     */

    if ((Knuckle_free = (KNUCKLE *) calloc(KNUC_ALLOC_SZ,sizeof(KNUCKLE)))
	== NULL) {
      log_error("olcd: can't allocate initial Knucles");
      exit(ERROR);
    }
    /* Link up allocated knuckles into a free list */
    for(i=0;i<KNUC_ALLOC_SZ-1;i++)
      Knuckle_free[i].next = &Knuckle_free[i+1];
    Knuckle_free[KNUC_ALLOC_SZ-1].next = NULL;

    if ((User_free = (USER *) calloc(USER_ALLOC_SZ,sizeof(USER)))
	== NULL) {
      log_error("olcd: can't allocate initial User list");
      exit(ERROR);
    }
    for(i=0;i<USER_ALLOC_SZ-1;i++)
      User_free[i].next = &User_free[i+1];
    User_free[KNUC_ALLOC_SZ-1].next = NULL;

    if ((Question_free = (QUESTION *) calloc(QUES_ALLOC_SZ,sizeof(QUESTION)))
	== NULL) {
      log_error("olcd: can't allocate initial Knucles");
      exit(ERROR);
    }
    for(i=0;i<QUES_ALLOC_SZ-1;i++)
      Question_free[i].next = &Question_free[i+1];
    Question_free[KNUC_ALLOC_SZ-1].next = NULL;


    Knuckle_List = (KNUCKLE **) malloc(sizeof(KNUCKLE *));
    if (Knuckle_List == (KNUCKLE **) NULL)
    {
	log_error("olcd: can't allocate Knuckle List");
	exit(ERROR);
    }
    *Knuckle_List = (KNUCKLE *) NULL;

    Topic_List = (TOPIC **) malloc(sizeof(TOPIC *));
    if (Topic_List == (TOPIC **) NULL)
    {
	log_error("olcd: can't allocate Topic List");
	exit(ERROR);
    }
    *Topic_List = (TOPIC *) NULL;

    /*
     * find out who we are and what we are doing here
     */

    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
	log_error("olcd: can't get hostname: %m");
	exit(ERROR);
    }

    if ((this_host_entry = gethostbyname(hostname)) == (struct hostent *)NULL)
    {
	log_error("olcd: Unable to get host entry for this host: %m");
	exit(ERROR);
    }
    (void) strcpy(hostname, this_host_entry->h_name);

#ifdef HESIOD
    if (!hostset)
    {
	if ((hp = hes_resolve(DaemonInst, OLC_SERV_NAME)) == NULL)
	{
	    log_error("Unable to get name of OLC server host from nameserver.");
	    log_error("Exiting..");
	    exit(ERROR);
	}
	else (void) strcpy(DaemonHost, *hp);
    }
#endif /* HESIOD */

    if((daemon_host_entry = gethostbyname(DaemonHost))==(struct hostent *)NULL)
    {
	log_error("Unable to get daemon host entry.");
	exit(ERROR);
    }

    if (port_num == 0) {
      service = (struct servent *) NULL;
#ifdef HESIOD
      service = hes_getservbyname(OLC_SERVICE, OLC_PROTOCOL);
#endif
      if (service == (struct servent *) NULL) {
	if ((service = getservbyname(OLC_SERVICE, OLC_PROTOCOL)) ==
	    (struct servent *)NULL)
	  {
	    log_error("olcd: olc/tcp: unknown service...exiting");
	    exit(ERROR);
	  }
      }
      port_num = service->s_port;
    }
    sin.sin_port = port_num;


    /*
     * Open up a socket
     */

restart:
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	log_error("olcd: can't create a socket: %m");
	exit(ERROR);
    }
    listening_fd = fd;

    /* First, we try the 4.3 setsockopt() system.  If it fails,
     * try the 4.2 system. */

    onoff = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int)))
    {
	onoff = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff , 0) < 0)
	{
	    log_error("Can't set socket options: %m");
	    exit(ERROR);
	}
    }

    /*
     * bind the socket to the port
     */

    if (bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) < 0)
    {
	log_error("bind: %m");
	exit(ERROR);
    }

    /*
     * chdir so cores get dumped in the right directory
     */

    if (chdir(LOG_DIR) == -1)
    {
	log_error("Can't change wdir: %m");
    }

    uncase(hostname);
    uncase(daemon_host_entry->h_name);

    if (!string_eq(hostname, daemon_host_entry->h_name)) {
      /* format message first, because h_name is static buffer */
      sprintf(buf,"%s != %s", hostname, daemon_host_entry->h_name);
      log_error(buf);
      log_error("error: host information doesn't point here; exiting");
      return 1;
    }

    sprintf(buf,"%s Daemon startup....", DaemonInst);
    log_status(buf);

    load_db();
    load_data();
    flush_olc_userlogs();

    /*
     * Listen for connections..
     */

    if (listen(fd, SOMAXCONN))
    {
	fflush(stderr);
	log_error("aborting from listen: %m");
	exit(1);
    }

    processing_request = 0;
    got_signal = 0;

#ifdef _POSIX_SOURCE
    /* Set up sigaction structure */
    /* This is all done because the RS/6000 emulation of signal sets the */
    /* signal action back to the default action when the signal handler is */
    /* called, instead of leaving well enough alone.. */
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = punt;

    sigaction(SIGINT, &action, NULL);/* ^C on control tty (for test mode) */
    sigaction(SIGHUP, &action, NULL);	/* kill -1 $$ */
    sigaction(SIGTERM, &action, NULL);	/* kill $$ */

    action.sa_handler = reap_child;
    sigaction(SIGCHLD, &action, NULL); /* When a child dies, reap it. */

    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, NULL);

#ifdef PROFILE
    action.sa_handler = dump_profile;
    sigaction(SIGUSR1, &action, NULL); /* Dump profiling information */
    					  /* and stop profiling */
    action.sa_handler = start_profile;
    sigaction(SIGUSR2, &action, NULL); /* Start profiling */
#endif /* PROFILE */
#else
    signal(SIGINT, punt);	/* ^C on control tty (for test mode) */
    signal(SIGHUP, punt);	/* kill -1 $$ */
    signal(SIGTERM, punt);	/* kill $$ */
    signal(SIGCHLD, reap_child); /* When a child dies, reap it. */
    signal(SIGPIPE, SIG_IGN);
#ifdef PROFILE
    signal(SIGUSR1, dump_profile); /* Dump profiling information and stop */
				   /* profiling */
    signal(SIGUSR2, start_profile); /* Start profiling */
#endif /* PROFILE */
#endif
#ifdef KERBEROS
    get_kerberos_ticket ();
#endif
    sprintf(buf,"%s Daemon successfully started.  Waiting for requests.",
		    DaemonInst),
    olc_broadcast_message("syslog", buf, "system");
    start_time = time(0);

     /*
     * Wait for requests
     */

    while (TRUE)
    {
	int s;		        /* Duplicated file descriptor */
	int len = sizeof (from);  /* Length of address. */
#ifdef KERBEROS
	struct timeval tval;
	int nfound;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(fd,&readfds);
	tval.tv_sec = 600;   /* 10 minutes */
	tval.tv_usec = 0;

	/* Try to get kerberos ticket if no activity in 10 minutes */
	/* This guarantees we _always_ have one, even if no requests have */
	/* come in for > 15 minutes */

	nfound = select(fd+1,&readfds,NULL,NULL,&tval);
	if (nfound < 0) {
	  if (errno == EINTR)
	    continue;

	  n_errs++;
	  if (n_errs < 3)
	    continue;
	  else if (n_errs > 10) { 
	    log_error("Too many errors accepting connections; exit");
	    abort();
	  }
	  else if (errno == ECONNABORTED)
	    {
	      log_error("Restarting...(error reading request)");
	      close(fd);
	      goto restart;
	    }
	  else {
	    log_error("Unexpected error from accept; exiting");
	    abort();
	  }
	}

	if (nfound == 0) {
	  get_kerberos_ticket();
	  continue;
	}

#endif
	s = accept(fd, (struct sockaddr *) &from, &len);
	if (s < 0) {
	    if (errno == EINTR)
		continue;
	    log_error("Error accepting connection: %m");
	    n_errs++;
	    if (n_errs < 3)
		continue;
	    else if (n_errs > 10) { 
	      log_error("Too many errors accepting connections; exit");
	      abort();
	    }
	    else if (errno == ECONNABORTED)
	    {
		log_error("Restarting...(error reading request)");
		close(fd);
		goto restart;
	    }
	    else {
	      log_error("Unexpected error from accept; exiting");
	      abort();
	    }
	}
#if 0
	{
	    struct sockaddr addr[20];
	    int len = sizeof (addr);
	    struct sockaddr_in *in = (struct sockaddr_in *) addr;
	    if (getpeername (s, addr, &len) == 0) {
	      sprintf(buf,"connect from %s/%d\n", inet_ntoa (in->sin_addr),
		      in->sin_port);
	      log_debug(buf);
	    }
	  }
#endif
	process_request(s, &from);
	close(s);
	if (got_signal)
	    punt(-1);
	n_errs = 0;
    }
}


/*
 * Function:	process_request() processes requests from the olc and olcr
 *			processes.
 * Arguments:	fd:	The file descriptor for the socket.
 * Returns:	Nothing.
 * Notes:
 *	First, we read the request off of the socket, then look up the
 *	request in the request table.
 */

static void
process_request (fd, from)
    int fd;
    struct sockaddr_in *from;
{
    REQUEST request;	/* Request structure from client. */
    int type;		/* Type of request. */
    int ind;		/* Index in proc. table. */
    int auth;             /* status from authentication */
    struct hostent *hp;	/* host sending request */

    ind = 0;
    processing_request = 1;
    if(read_request(fd, &request) != SUCCESS) {
	log_error ("Error in reading request");
	return;
    }
    type = request.request_type;

#ifdef KERBEROS
    /*
     * make sure olc has a valid tgt before each request
     */

    get_kerberos_ticket();
#endif /* KERBEROS */

    /* get the right hostname */

    hp = gethostbyaddr(&from->sin_addr, sizeof(struct in_addr),
		       from->sin_family);
    if (!hp)
    {
	char msgbuf[BUFSIZ];
	sprintf(msgbuf, "Can't resolve name of host %s\n",
		inet_ntoa(from->sin_addr));
	log_error(msgbuf);
    }
    (void) strcpy(&request.requester.machine[0], hp ? hp->h_name :
		  inet_ntoa(from->sin_addr));

    /* authenticate requestor using kerberos if available */

    auth = authenticate(&request, from->sin_addr.s_addr);
    if (auth)
    {
	send_response(fd,auth);
	return;
    }
    while  ((type != Proc_List[ind].proc_code)
	    && (Proc_List[ind].proc_code != UNKNOWN_REQUEST))
	ind++;

    if (Proc_List[ind].proc_code != UNKNOWN_REQUEST)
    {

	++request_count;
	++request_counts[ind];
#if 0
	printf("%d> Got %s request from %s\n",request_count,
	       Proc_List[ind].description,
	       request.requester.username);
#endif

	(*(Proc_List[ind].olc_proc))(fd, &request);
    }
    else
    {
      send_response(fd, UNKNOWN_REQUEST);
    }


    /*
     * make a backup of data after request complete (if data changed)
     */

    if (needs_backup) {
	backup_data();
	dump_list();
      }
    processing_request = 0;
}


static void
flush_olc_userlogs()
{
    /* placeholder until this is really written */
    /*
     * algorithm: scan through spool directory entries; skip
     * "backup.dat", ".", "..", and any "%s.log" corresponding to
     * usernames.  any others "*.log" files should be forwarded to
     * crash log
     */

    DIR *dirp;
#ifdef _POSIX_SOURCE
    struct dirent *dp;
#else
    struct direct *dp;
#endif
    dirp = opendir(LOG_DIR);

    if (dirp == (DIR *)NULL)
    {
	log_error("Can't flush lost userlogs");
	return;
    }

#ifdef _POSIX_SOURCE
    for (dp = readdir(dirp); dp != (struct dirent *)NULL; dp = readdir(dirp))
#else
    for (dp = readdir(dirp); dp != (struct direct *)NULL; dp = readdir(dirp))
#endif
    {
	if (!strcmp(dp->d_name+dp->d_namlen-4, ".log"))
	{
	    char msgbuf[BUFSIZ];
	    (void) strcpy(msgbuf, "Found log file ");
	    (void) strncat(msgbuf, dp->d_name, dp->d_namlen);
	    log_status(msgbuf);
	    /* check for username among active questions */
	    /* (yet to be implemented) */
	}
    }
    closedir(dirp);
}

/*
 * Function:	punt() stops the olcd daemon.
 * Arguments:	none
 * Returns:	nothing
 * Notes:
 *	If the daemon is not handling a request, exit.  Otherwise,
 *	set a flag that will cause it to exit after it finishes
 *	processing the current request.
 */

#ifdef VOID_SIGRET
static void
#else
static int
#endif
punt(sig)
    int sig;
{
  char buf[BUFSIZ];

  sprintf(buf,"%s Daemon shutting down on signal %d.", DaemonInst, sig),
  olc_broadcast_message("syslog", buf, "system");
  
  close(listening_fd);
  if (processing_request) {
    got_signal = 1;
    log_status("Caught signal, will exit after finishing request");
  }
  else {
    log_status("Caught signal, exiting...");
    dump_request_stats(REQ_STATS_LOG);
    dump_question_stats(QUES_STATS_LOG);
    exit(1);
  }
}


/*
 * Function:	reap_child() gets rid of forked children.
 * Arguments:	none
 * Returns:	nothing
 * Notes:
 *		loop while there are any children waiting to report status.
 */

#ifdef VOID_SIGRET
static void
#else
static int
#endif
reap_child(sig)
     int sig;
{
#ifdef _POSIX_SOURCE
  int status;
#else
  union wait status;
#endif
  int pid;

#ifdef SABER
  sig = sig;
#endif
  signal(SIGCHLD, reap_child); /* When a child dies, reap it. */
  pid = wait3(&status,WNOHANG,0);
  while(pid > 0)
    pid = wait3(&status,WNOHANG,0);
#ifdef VOID_SIGRET
  return;
#else
  return(0);
#endif
}

#ifdef PROFILE
#ifdef VOID_SIGRET
static void
#else
static int
#endif
dump_profile(sig)
     int sig;
{
  char buf[BUFSIZ];

  sprintf(buf,"%s Daemon dumping profiling stats.", DaemonInst);
  olc_broadcast_message("syslog", buf ,"system");
  log_status("Dumping profile..");
  monitor(0);
  moncontrol(0);
}

#ifdef VOID_SIGRET
static void
#else
static int
#endif
start_profile(sig)
     int sig;
{
  char buf[BUFSIZ];

  sprintf(buf,"%s Daemon starting profiling.", DaemonInst);
  olc_broadcast_message("syslog", buf, "system");
  log_status("Starting profile..");
  moncontrol(1);
}
#endif /* PROFILE */



/*
 * Function:	authenticate()  check the kerberos tickets from the client
 *
 * Arguments:	REQUEST *request
 * Returns:	ERROR if cannot authenticate
 *
 */

int
authenticate(request, addr)
    REQUEST *request;
    unsigned long addr;
{

#ifdef KERBEROS
    AUTH_DAT data;
#endif /* KERBEROS */

    int result;

#ifndef KERBEROS
    return(SUCCESS);
#else /* KERBEROS */

    result = krb_rd_req(&(request->kticket),K_SERVICE,K_INSTANCEbuf,
			addr,&data,SRVTAB_FILE);

    strcpy(request->requester.username,data.pname);
    strcpy(request->requester.realm,data.prealm);

    return(result);
#endif /* KERBEROS */
}


#ifdef KERBEROS
int
get_kerberos_ticket()
{
  int ret;
  char sinstance[INST_SZ];
  char principal[ANAME_SZ];
  char buf[BUFSIZ];
  char *ptr;
  
  /* If the ticket time is going to expire in 15 minutes or less, get a */
  /* new one */
  if(ticket_time < NOW - ((96L * 5L) - 15L) * 60)
    {
      strcpy(principal,K_SERVICE);
      strcpy(sinstance,DaemonHost);
      ptr = index(sinstance,'.');
      if (ptr)
	*ptr = '\0';
      uncase(sinstance);
      sprintf(buf,"get new tickets: %s %s ", K_SERVICE, sinstance);

      log_status(buf);
      dest_tkt();
      ret = krb_get_svc_in_tkt(K_SERVICE, sinstance, SERVER_REALM,
			       "krbtgt", SERVER_REALM,
			       96, SRVTAB_FILE);
      if (ret != KSUCCESS) {
	sprintf(buf,"get_tkt: %s", krb_err_txt[ret]);
	log_error(buf);
	ticket_time = 0L;
	return(ERROR);
      }
      else
	ticket_time = NOW;
    }
  return(SUCCESS);
}
#endif /* KERBEROS */
