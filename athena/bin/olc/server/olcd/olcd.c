/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the primary functions of the daemon, olcd.
 *
 *      Win Treese, Dan Morgan, Bill Saphir (MIT Project Athena)
 *      Ken Raeburn (MIT Information Systems)
 *      Steve Dyer (IBM/MIT Project Athena)
 *      Tom Coppeto, Chris VanHaren, Lucien Van Elsen (MIT Project Athena)
 *
 * Copyright (C) 1990-1997 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: olcd.c,v 1.66 1999-06-28 22:52:41 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: olcd.c,v 1.66 1999-06-28 22:52:41 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <sys/types.h>          /* Standard type defs. */
#include <sys/socket.h>		/* IPC definitions. */
#include <sys/file.h>		/* File I/O definitions. */
#include <sys/stat.h>		/* stat() and friends */
#include <sys/ioctl.h>		/* For handling TTY. */
#include <termios.h>		/* For handling TTY... this time for real. */
#include <unistd.h>
#include <netinet/in.h>		/* More IPC definitions. */
#include <sys/wait.h>		/* Wait defs. */
#include <sys/param.h>
#include <errno.h>		/* Standard error numbers. */
#include <pwd.h>		/* Password entry defintions. */
#include <signal.h>		/* Signal definitions. */
#include <string.h>

#include <limits.h>

#ifdef HAVE_ZEPHYR
#include <zephyr/zephyr.h>
#endif /* HAVE_ZEPHYR */

#if HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include <netdb.h>		/* Network database defs. */
#include <arpa/inet.h>		/* inet_* defs */

#include <olcd.h>

#include <stdarg.h>

/* Global variables. */

extern PROC  Proc_List[];	/* OLC Proceedure Table */
extern PROC  Maint_Proc_List[];	/* OLC "Maintenance Mode" Proceedure Table */

char DaemonHost[LINE_SIZE];	/* Hostname of daemon's machine. */
char DaemonInst[LINE_SIZE];	/* Instance: "OLC", "OLTA", etc. */
char DaemonZClass[LINE_SIZE+6];	/* Zephyr class to use, usually DaemonInst */
struct sockaddr_in sain = { AF_INET }; /* Socket address. */
int request_count = 0;
int request_counts[OLC_NUM_REQUESTS];
long start_time;
int select_timeout = 30;	/* needed by libcommon (sigh) */
int maint_mode = 0;
PROC *proc_list;

#ifdef HAVE_KRB4
static long ticket_time = 0L;	/* Timer on kerberos ticket */
char SERVER_REALM[REALM_SZ] = DFLT_SERVER_REALM;  /* name of server's realm */
#endif /* HAVE_KRB4 */


static void process_request (int fd , struct sockaddr_in *from );
static void flush_olc_userlogs (void );

static void reap_child (int sig);
static void punt (int sig);
#ifdef PROFILE
static void dump_profile (int sig);
static void start_profile (int sig);
#endif /* PROFILE */

/* Static variables */

static int processing_request;
static int got_signal = 0;

/*
 * Function:	main() is the start-up function for the olcd daemon.
 * Arguments:	argc:	Number of command line arguments.
 *		argv:	Array of words from the command line.
 * Returns:	Returns only on error or signal.
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
int main (int argc, char **argv)
{
  struct sockaddr_in from;		/* Socket address for input. */
  struct servent *service;		/* Network service entry. */
  struct hostent *this_host_entry;	/* Host entry for this host. */
  struct hostent *daemon_host_entry;	/* Entry for daemon host.*/
  char hostname[MAXHOSTNAMELEN];	/* Name of this host. */
  char *end;				/* Pointer for string manipulation */
  char buf[BUFSIZ];			/* for constructing erorr messages */
  int listening_fd;			/* fd for listen()'ing socket */
  int fd;				/* fd for temporary use (TIOCNOTTY) */
  int on = 1;				/* Value variable for setsockopt. */
  int n_errs=0;				/* Error count in accept() call */
  int hostset = 0;			/* Flag if host was passed as arg */
  int nofork = 0;			/* Flag if you don't want to fork */
  int port_num = 0;			/* Port number explicitly requested */
#ifdef HAVE_ZEPHYR
  int ret;				/* return value from ZInitialize. */
#endif
#ifdef HAVE_HESIOD
  char **hp;				/* return value of Hesiod resolver */
#endif
  struct sigaction action;		/* POSIX signal structure */

  /* Before doing *anything* that might cause a coredump, move any old
   * coredumps out of the way.
   */
  stash_olc_corefile();

#if defined(PROFILE)
  /* Turn off profiling on startup; that way, we collect "steady state"
   * statistics, not the initial flurry of startup activity
   */
  moncontrol(0);
#endif

  /*
   * Things needed before parsing command line
   */

  /* get the hostname of current host */
  if (gethostname(hostname, sizeof(hostname)) != 0)
    {
      /* Failed.  Logging is not up yet: just print to stderr */
      fprintf(stderr, "olcd: can't get hostname??? [errno=%d]\n", errno);
      exit(1);
    }
  hostname[sizeof(hostname)-1] = '\0';  /* ensure NUL-termination */

  /* find the canonical address for this host */
  this_host_entry = gethostbyname(hostname);
  if (this_host_entry == NULL)
    {
      /* Failed.  Logging is not up yet: just print to stderr */
      fprintf(stderr, "olcd: Unable to get host entry for '%s', ignored.\n",
	      hostname);
    } else {
      /* we have the canonical name; use it. */
      strncpy(hostname, this_host_entry->h_name, sizeof(hostname)-1);
      hostname[sizeof(hostname)-1] = '\0';  /* ensure NUL-termination */
    }
  downcase_string(hostname);

  /* copy the the host name to DaemonHost, without a domain name */
  end = strchr(hostname, '.');		/* end points to a dot... */
  if (! end)
    end = hostname + strlen(hostname);	/* ...or the final NUL. */
  strncpy(DaemonHost, hostname, end-hostname);
  DaemonHost[end-hostname] = '\0';  /* ensure NUL-termination */

  /* set default server 'instance' -- "OLC" */
  strcpy(DaemonInst, OLXX_SERVICE);
  upcase_string(DaemonInst);

  strcpy(DaemonZClass, OLXX_SERVICE);

  /*
   * Parse any arguments
   */

  argv++;
  argc--;
  while (argc > 0)
    {
      /* Options that take no arguments. */
      if (!strcmp (argv[0], "-nofork") || !strcmp (argv[0], "-no_fork"))
	{
	  nofork = 1;
	  argv++;
	  argc--;
	}
      else if (!strcmp(argv[0], "-maint"))
	{
	  maint_mode = 1;
	  argv++;
	  argc--;
	}
      else if (argc > 1)
	{
	  /* Options taking 1 arg go here, where they know they'll get one */
	  if (!strcmp(argv[0], "-h") || !strcmp (argv[0], "-host"))
	    {
	      strncpy(DaemonHost, argv[1], sizeof(DaemonHost)-1);
	      DaemonHost[sizeof(DaemonHost)-1] = '\0';
	      argv += 2;
	      argc -= 2;
	    }
	  else if (!strcmp (argv[0], "-port"))
	    {
	      port_num = htons(atoi(argv[1]));
	      argv += 2;
	      argc -= 2;
	    }
	  else if (!strcmp (argv[0], "-inst"))
	    {
	      strcpy(DaemonInst, argv[1]);
	      upcase_string(DaemonInst);
#ifdef HAVE_ZEPHYR
	      /* also reset DaemonZClass! */
	      strcpy(DaemonZClass, argv[1]);
	      downcase_string(DaemonZClass);
#endif /* HAVE_ZEPHYR */
	      argv += 2;
	      argc -= 2;
	    }
#ifdef HAVE_ZEPHYR
	  else if (!strcmp (argv[0], "-zclass"))
	    {
	      strcpy(DaemonZClass, argv[1]);
	      argv += 2;
	      argc -= 2;
	    }
#endif /* HAVE_ZEPHYR */
	  else
	    {
	      /* We have enough args, but no arg names matched... oops. */
	      fprintf(stderr, "olcd: unknown argument: '%s'\n",argv[0]);
	      exit(1);
	    }
	}
      else
	{
	  /* Flags without args didn't match, and this was the last argument */
	  if (!strcmp(argv[0], "-h") || !strcmp (argv[0], "-host")
	       || !strcmp (argv[0], "-port") || !strcmp (argv[0], "-inst"))
	    fprintf(stderr, "olcd: flag '%s' requires an argument.\n",
		    argv[0]);
	  else
	    fprintf(stderr, "olcd: unknown argument: '%s'\n",argv[0]);

	  exit(1);
	}
    }

  if (maint_mode)
    proc_list = Maint_Proc_List;
  else
    proc_list = Proc_List;

#ifdef HAVE_KRB4
  set_env_var("KRBTKFILE", TICKET_FILE);
  get_kerberos_ticket();  /* get a ticket now, so Zephyr likes us */
#endif /* HAVE_KRB4 */

#ifdef HAVE_ZEPHYR
  /* We must ZInitialize now, *before* we use "log_error" for the first
   * time, as we want to broadcast errors via zephyr.  ZInitialize will
   * initialize krb and zeph error tables for com_err.
   */
  ret = ZInitialize();
  if (ret != ZERR_NONE)
    log_zephyr_error("Error in ZInitialize: %d (%m)",ret);
#else /* don't HAVE_ZEPHYR */
#ifdef HAVE_KRB4
  /* initialize Kerberos error table for com_err. */
  initialize_krb_error_table();
#endif /* HAVE_KRB4 */
#endif /* don't HAVE_ZEPHYR */  

  /* make libcommon.a use syslog (and Zephyr, if used) for error logging */
  set_olc_perror(log_error_string);

  /*
   * fork off
   */

  if (!nofork) {

    switch (fork()) {
    case 0:				/* child */
      break;
    case -1:				/* error */
      log_error ("Can't fork: %m");
      exit(2);
    default:				/* parent */
      exit(0);
    }

    /* make stdin open to /dev/null for reading */
    fd = open ("/dev/null", O_RDONLY);
    if (fd < 0) {
      log_error("Can't open /dev/null for reading");
      exit(2);
    }
    if (fd != 0) {
      close(0);
      dup2 (fd, 0);
      close (fd);
    }
    /* make stdout and stderr open to /dev/null for writing */
    fd = open ("/dev/null", O_WRONLY);
    if (fd < 0) {
      log_error("Can't open /dev/null for writing");
      exit(2);
    }
    if (fd != 1) {
      close(1);
      dup2 (fd, 1);
    }
    if (fd != 2) {
      close(2);
      dup2 (fd, 2);
    }
    if ((fd != 1) && (fd != 2))
      close(fd);

    fd = open("/dev/tty", O_RDWR, 0);
    if (fd >= 0) {
      ioctl(fd, TIOCNOTTY, NULL);
      close (fd);
    }
  }

#ifdef HAVE_SETVBUF
    setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
    setvbuf(stderr,NULL,_IOLBF,BUFSIZ);
#else
    setlinebuf(stdout);
    setlinebuf(stderr);
#endif

    umask(077);
    log_debug("started...");

    /*
     * allocate lists
     */

    Knuckle_List = (KNUCKLE **) malloc(sizeof(KNUCKLE *));
    if (Knuckle_List == NULL)
    {
	log_error("olcd: can't allocate Knuckle List: %m");
	exit(8);
    }
    *Knuckle_List = NULL;

    Topic_List = (TOPIC **) malloc(sizeof(TOPIC *));
    if (Topic_List == NULL)
    {
	log_error("olcd: can't allocate Topic List: %m");
	exit(8);
    }
    *Topic_List = NULL;

    /*
     * find out who we are and what we are doing here
     */

    daemon_host_entry = gethostbyname(DaemonHost);
    if (daemon_host_entry == NULL)
    {
	log_error("Unable to get daemon host entry: %m");
	exit(2);
    }

    if (port_num == 0) {
      service =  NULL;
#ifdef HAVE_HESIOD
      service = hes_getservbyname(OLCD_SERVICE_NAME, OLC_PROTOCOL);
#endif
      if (service == NULL) {
	service = getservbyname(OLCD_SERVICE_NAME, OLC_PROTOCOL);
	if (service == NULL)
	  {
	    log_error("olcd: %s/%s: %m... exiting",
		      OLCD_SERVICE_NAME, OLC_PROTOCOL);
	    exit(2);
	  }
      }
      port_num = service->s_port;
    }
    sain.sin_port = port_num;


    /*
     * Open up a socket
     */

restart:
    log_debug("creating socket.");
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
	log_error("olcd: can't create a socket: %m");
	exit(2);
    }
    listening_fd = fd;

    /* Enable reusing local addrs on the socket.  Don't panic the machine. */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)))
    {
      log_error("Can't set SO_REUSEADDR: %m");
      exit(2);
    }

    /*
     * bind the socket to the port
     */

    if (bind(fd, (struct sockaddr *) &sain, sizeof(struct sockaddr_in)) < 0)
    {
	log_zephyr_error("bind: %m");
	  /* no, this isn't really a zephyr error, but we don't want to
	     broadcast this out over zephyr because we use cron to try
	     and restart every 20 min., and of course it fails here, and
	     broadcasting that every 20 min gets annoying... */
	/* TODO: I'm not sure this is a sane policy.  --bert 29jan1997 */
	exit(2);
    }

    /*
     * chdir so cores get dumped in the right directory
     */

    if (chdir(CORE_DIR) == -1)
    {
	log_error("Can't change wdir: %m");
    }

    uncase(hostname);
    uncase(daemon_host_entry->h_name);

    if (!string_eq(hostname, daemon_host_entry->h_name)) {
      /* warning: h_name is a static buffer */
      log_error("%s != %s", hostname, daemon_host_entry->h_name);
      log_error("error: host information doesn't point here; exiting");
      exit(2);
    }

    log_status("%s Daemon startup....", DaemonInst);

    load_db();
    load_data();
    dump_list();
    if (needs_backup)
      backup_data();

    flush_olc_userlogs();

    /*
     * Listen for connections..
     */

    if (listen(fd, SOMAXCONN))
    {
      log_error("listen: %m");
      exit(2);
    }

    processing_request = 0;
    got_signal = 0;

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

    sprintf(buf,"%s Daemon successfully started.  Waiting for requests.",
	    DaemonInst);
    olc_broadcast_message("syslog", buf, "system");
    start_time = time(NULL);

    /*
     * Wait for requests
     */

    while (! got_signal)
    {
	int s;		        /* Duplicated file descriptor */
	int len = sizeof (from);  /* Length of address. */
	KNUCKLE **k_ptr;

	struct timeval tval;
	int nfound;
	fd_set readfds;
	
	FD_ZERO(&readfds);
	FD_SET(fd,&readfds);
	tval.tv_sec = TICKET_FREQ*60;   /* 1 minute increments */
	tval.tv_usec = 0;

	/* select() returns after at most TICKET_FREQ minutes.  That
	 * guarantees we _always_ have tickets, even if no requests have
	 * come in for > 15 minutes, or if we lost them to reactivate.
	 */
	nfound = select(fd+1,&readfds,NULL,NULL,&tval);
	if (nfound < 0) {
	  if (errno == EINTR)
	    continue;

	  n_errs++;
	  if (n_errs < 3)
	    continue;
	  else if (n_errs > 10)
	    { 
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
#ifdef HAVE_KRB4
	/* We get Kerberos tickets whenever we get a request, but this
	 * makes sure we have valid tickets even when there are no requests.
	 */
	if (nfound == 0) {
	  get_kerberos_ticket();
	  continue;
	}
#endif /* HAVE_KRB4 */

	/* If select() on a listening socket retuns "readable" data,
	 * that means we have a connection to accept.
	 */
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
	      log_debug("connect from %s/%d\n",
			inet_ntoa(in->sin_addr), in->sin_port);
	    }
	  }
#endif

	process_request(s, &from);

	close(s);

	n_errs = 0;
    }

    sprintf(buf,"%s Daemon shutting down on signal %d.",
	    DaemonInst, got_signal);
    olc_broadcast_message("syslog", buf, "system");

    close(listening_fd);
    exit(0);
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
#ifdef LOG_REQUEST
    log_debug("got request type = %d", type);
#endif /* LOG_REQUEST */

#ifdef HAVE_KRB4
    /*
     * make sure olc has a valid tgt before each request
     */

    get_kerberos_ticket();
#endif /* HAVE_KRB4 */

    /* get the right hostname */

    hp = gethostbyaddr((void *)&from->sin_addr, sizeof(struct in_addr),
		       from->sin_family);
    if (!hp)
      {
	log_error("No hostname found for IP address %s\n",
		  inet_ntoa(from->sin_addr));
      }

    if (hp && (strlen(hp->h_name) < sizeof(request.requester.machine)))
      {
	strcpy(request.requester.machine, hp->h_name);
      }
    else
      {
	strncpy(request.requester.machine, inet_ntoa(from->sin_addr),
		sizeof(request.requester.machine));
	request.requester.machine[sizeof(request.requester.machine)-1] = '\0';
      }

    /* authenticate requestor using kerberos if available */

    auth = authenticate(&request, from->sin_addr.s_addr);
    if (auth)
    {
        log_debug("auth failed for '%s'", request.requester.machine);
	send_response(fd,auth);
	return;
    }
    /* check that client hasn't sent bad request packet by 
       overflowing username */
    if (request.target.username[LOGIN_SIZE-1] != 0 ||
	request.requester.username[LOGIN_SIZE-1] != 0)
      {
	log_error("bad request: username overflowed");
	send_response(fd,TARGET_NOT_FOUND); /* should be a better error code
					       but client can't be changed now
					       to understand ST */
	return;
      }
    
    while  ((type != proc_list[ind].proc_code)
	    && (proc_list[ind].proc_code != UNKNOWN_REQUEST))
	ind++;

    if (proc_list[ind].proc_code != UNKNOWN_REQUEST)
    {

	char msgbuf[BUFSIZ];

	++request_count;
	++request_counts[ind];
#ifdef DEBUG_REQUEST
	log_debug("serial #%d> \"%s\" (# %d) request from %s\n",
		  request_count,
		  proc_list[ind].description,
		  request_counts[ind],
		  request.requester.username);
#endif

	(*(proc_list[ind].olc_proc))(fd, &request);
    }
    else
    {
      send_response(fd,
		    maint_mode ? PERMISSION_DENIED : UNKNOWN_REQUEST);
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
    struct dirent *dp;
    dirp = opendir(LOG_DIR);

    if (dirp == (DIR *)NULL)
    {
	log_error("Can't flush lost userlogs");
	return;
    }

    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if (!strcmp(dp->d_name+strlen(dp->d_name)-4,".log"))
	{
	    char msgbuf[BUFSIZ];
	    (void) strcpy(msgbuf, "Found log file ");
	    (void) strncat(msgbuf, dp->d_name, strlen(dp->d_name));
	    log_status(msgbuf);
	    /* check for username among active questions */
	    /* (yet to be implemented) */
	}
    }
    closedir(dirp);
}

/* Signal handler for olcd.
 * Note: Just sets a flag, to avoid reentrancy issues.
 */

static void
punt(int sig)
{
  got_signal = sig;
  return;
}


/*
 * Function:	reap_child() gets rid of forked children.
 * Arguments:	none
 * Returns:	nothing
 * Notes:
 *		loop while there are any children waiting to report status.
 */

static void
reap_child(int sig)
{
  int status;
  int pid;

#ifdef SABER
  sig = sig;
#endif
  pid = waitpid(-1, &status, WNOHANG);
  while(pid > 0)
    pid = waitpid(-1, &status, WNOHANG);
  return;
}

#ifdef PROFILE
static void
dump_profile(int sig)
{
  char buf[BUFSIZ];

  sprintf(buf,"%s Daemon dumping profiling stats.", DaemonInst);
  olc_broadcast_message("syslog", buf ,"system");
  log_status("Dumping profile..");
  monitor(0);
  moncontrol(0);
}

static void
start_profile(int sig)
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
    char instance[INST_SZ];
    char buf[BUFSIZ];
#ifdef HAVE_KRB4
    AUTH_DAT data;
#endif /* HAVE_KRB4 */

    int result;

#ifndef HAVE_KRB4
    return(SUCCESS);   /* we have no way of checking. */
#else /* HAVE_KRB4 */

    strcpy(instance, K_INSTANCE);  /* otherwise, things break for "*". */
    result = krb_rd_req(&(request->kticket), K_SERVICE, instance,
			addr, &data, OLC_SRVTAB);

    if (result != 0)
      log_status("krb_rd_req returned %d: [%s] [%s] ",
		 result, K_SERVICE, instance);

    if (!strcasecmp(data.pname, K_SERVICE))
      {
	/* We have a request from olc.*; it better be coming from us. */
	if (strcasecmp(data.pinst, DaemonHost))
	  {
	    log_error("Request by srvtab %s.%s@%s [from %s] refused.",
		      data.pname, data.pinst, data.prealm,
		      request->requester.machine);
	    return PERMISSION_DENIED;
	  }
      }
    else
      {
	/* User request.  Only allow null, root and dbadmin instances. */
	if (data.pinst[0]
	    && strcasecmp(data.pinst, "root")
	    && strcasecmp(data.pinst, "dbadmin"))
	  {
	    log_admin("Request by %s.%s@%s [from %s] refused due to instance.",
		      data.pname, data.pinst, data.prealm,
		      request->requester.machine);
	    return PERMISSION_DENIED;
	  }
      }

    strcpy(request->requester.username, data.pname);
    strcpy(request->requester.realm,    data.prealm);

    return(result);
#endif /* HAVE_KRB4 */
}


#ifdef HAVE_KRB4
int
get_kerberos_ticket()
{
  int ret;
  char sinstance[INST_SZ];
  char principal[ANAME_SZ];
  char buf[BUFSIZ];
  char *ptr;
  char primary_name[100];

  struct stat dummy;
  time_t now = time(NULL);
  int exists;

  exists = stat(TICKET_FILE, &dummy);
  while ((exists < 0) && (errno == EINTR))
    exists = stat(TICKET_FILE, &dummy);

  /* Get new tickets if the ticket file is missing or if we got the tickets
   * more than TICKET_WHEN ago.  [TICKET_WHEN is in 5min chunks]
   */
  if ((exists < 0) || (ticket_time < (now - 5*60L * TICKET_WHEN)))
    {
      strcpy(principal,K_SERVICE);
      strcpy(sinstance,DaemonHost);
      ptr = strchr(sinstance,'.');
      if (ptr)
	*ptr = '\0';
      uncase(sinstance);

      /* dest_tkt(); wade 3/10/94  try not destroying tickets */
      strcpy(primary_name,K_SERVICE);

      ret = krb_get_svc_in_tkt(primary_name, sinstance, SERVER_REALM,
			       "krbtgt", SERVER_REALM,
			       TICKET_LIFE, OLC_SRVTAB);

      if (ret != KSUCCESS)
	{
	  /* since we may have no authentication, don't send zephyr msg */
	  log_status("get_tkt: %m");
	  ticket_time = 0L;
	  return(ERROR);
	}
      else
	{
	  ticket_time = now;
#ifdef HAVE_ZEPHYR
	  ZResetAuthentication();
#endif
	}
    }

  return(SUCCESS);
}
#endif /* HAVE_KRB4 */
