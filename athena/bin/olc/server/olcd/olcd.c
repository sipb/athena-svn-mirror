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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/olcd.c,v $
 *      $Author: vanharen $
 */


#include <olc/olc.h>
#include <olcd.h>
#include <sys/types.h>          /* Standard type defs. */
#include <sys/socket.h>		/* IPC definitions. */
#include <sys/file.h>		/* File I/O definitions. */
#include <sys/ioctl.h>		/* For handling TTY. */
#include <netinet/in.h>		/* More IPC definitions. */
#include <sys/wait.h>		/* Wait defs. */
#include <errno.h>		/* Standard error numbers. */
#include <pwd.h>		/* Password entry defintions. */
#include <netdb.h>		/* Network database defs. */
#include <signal.h>		/* Signal definitions. */
#include <sys/dir.h>		/* Directory entry format */
#include <arpa/inet.h>		/* inet_* defs */

#ifdef SYSLOG
#include <syslog.h>             /* syslog do hickies */
#endif SYSLOG

static const char rcsid[] =
    "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/olcd.c,v 1.7 1989-12-22 16:26:52 vanharen Exp $";

/* Global variables. */

extern int errno;		      /* System error number. */
extern PROC  Proc_List[];              /* OLC Proceedure Table */
char DaemonHost[LINE_LENGTH];	      /* Name of daemon's machine. */
struct sockaddr_in sin = { AF_INET }; /* Socket address. */
static int request_count = 0;
int select_timeout = 10;

#ifdef KERBEROS
static long ticket_time = 0L;         /* Timer on kerberos ticket */
int get_kerberos_ticket(); 
extern char SERVER_REALM[];
extern char *DFLT_SERVER_REALM;
extern char *TICKET_FILE;
extern int krb_ap_req_debug;
#endif KERBEROS



/* Static variables */

static int processing_request;
static int got_signal;
static int listening_fd;
int punt();


#ifdef TEST
#define	ME	"Test OLC daemon"
#else
#define	ME	"OLC Daemon"
#endif

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
	
main(argc, argv)
     int argc;
     char *argv[];
{
  struct sockaddr_in from;           /* Socket address for input. */
  struct servent *service;           /* Network service entry. */
  struct hostent *this_host_entry;   /* Host entry for this host. */
  struct hostent *daemon_host_entry; /* Entry for daemon host.*/
  char hostname[LINE_LENGTH];        /* Name of this host. */
  int fd;			     /* Socket fd. */
  int onoff;		             /* Value variable for setsockopt. */
  int n_errs=0;		             /* Error count in accept() call */
  int arg=0;			     /* Argument counter */
  int hostset = 0;                   /* Flag if host was passed as arg */
  
#ifdef HESIOD
  char **hp;		             /* return value of Hesiod resolver */
#endif HESIOD

#ifdef KERBEROS
  strcpy(K_INSTANCEbuf,K_INSTANCE);
  strcpy(SERVER_REALM,DFLT_SERVER_REALM);
#endif KERBEROS

  /*
   * Parse any arguments
   */

  for(arg=1;arg< argc; arg++)  
    {
      if(!strcmp(argv[arg],"-h"))  
	{
	  (void) strncpy(DaemonHost,argv[++arg],LINE_LENGTH);
	  hostset = 1;
	  continue;
	}	
      fprintf(stderr, "unknown argument: %s\n",argv[arg]);
      exit(1);
    }

  /*
   * fork off
   */
	
#ifndef TEST
  switch (fork()) 
    {
    case 0:	        /* child */
      break;
    case -1:		/* error */
      perror("Can't fork");
      exit(-1);
    default:		/* parent */
      exit(0);
    }
  
  for (fd = 0; fd < 10; fd++)
    (void) close(fd);
  (void) open("/", 0);
  (void) dup2(0, 1);
  (void) dup2(0, 2);
  freopen(STDERR_LOG, "a", stderr);
  fd = open("/dev/tty", O_RDWR, 0);
  if (fd >= 0) 
    {
      ioctl(fd, TIOCNOTTY, (char *) NULL);
      (void) close(fd);
    }
#endif TEST

  setlinebuf(stdout);
  setlinebuf(stderr);
  
  /* handle setuid-ness, etc., so we can dump core */
  setreuid((uid_t) geteuid(), -1);
  setregid((gid_t) getegid(), -1);

#ifndef	TEST
  (void) umask(066);	/* sigh */
#endif not TEST

#ifdef SYSLOG

  /* 
   * open syslogs 
   */

  openlog("olc",LOG_CONS,SYSLOG_LEVEL);

#endif SYSLOG 

  /*
   * allocate lists
   */

  Knuckle_List = (KNUCKLE **) malloc(sizeof(KNUCKLE *));
  if (Knuckle_List == (KNUCKLE **) NULL)
    {
      perror("malloc");
      log_error("olcd: can't allocate Knuckle List");
      exit(ERROR);
    }
  *Knuckle_List = (KNUCKLE *) NULL;

  Topic_List = (TOPIC **) malloc(sizeof(TOPIC *));
  if (Topic_List == (TOPIC **) NULL)
    {
      perror("malloc");
      log_error("olcd: can't allocate Topic List");
      exit(ERROR);
    }
   *Topic_List = (TOPIC *) NULL;

  /*
   * find out who we are and what we are doing here
   */

  if (gethostname(hostname, LINE_LENGTH) != 0) 
    {
      perror("gethostname");
      log_error("olcd: can't get hostname");
      exit(ERROR);
    }

  if ((this_host_entry = gethostbyname(hostname)) == (struct hostent *)NULL) 
    {
      perror("gethostbyname");
      log_error("olcd: Unable to get host entry for this host.\n");
      exit(ERROR);
    }
  (void) strcpy(hostname, this_host_entry->h_name);

  if (!hostset)  
    {
#ifdef HESIOD
#ifdef OLZ
      if ((hp = hes_resolve("olz",OLC_SERV_NAME)) == NULL)
#else
      if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
#endif
	{	  
	  log_error("Unable to get name of OLC server host from nameserver.");
	  log_error("Exiting..");
	  exit(ERROR);	  
	}
      else (void) strcpy(DaemonHost, *hp);

#else HESIOD
 
#endif HESIOD
    }

  if((daemon_host_entry = gethostbyname(DaemonHost))==(struct hostent *)NULL) 
    {    
      log_error("Unable to get daemon host entry.");
      exit(ERROR);
    }

  if ((service = getservbyname(OLC_SERVICE, OLC_PROTOCOL)) == 
      (struct servent *)NULL)
    {
      log_error("olcd: olc/tcp: unknown service...exiting");
      exit(ERROR);
    }
  sin.sin_port = service->s_port;


  
  /*
   * Open up a sock and put our mouth in it
   */
  
 restart:
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
      perror("olcd");
      log_error("olcd: can't create a socket.");
      exit(ERROR);
    }
  listening_fd = fd;
                                                                               
  /* First, we try the 4.3 setsockopt() system.  If it fails, 
     try the 4.2 system. */
	
  onoff = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int))) 
    {
      onoff = 0;
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff , 0) < 0) 
	{
	  perror("setsockopt");
	  log_error("Can't set socket options.");
	  exit(ERROR);
	}
    }
	
  /*
   * s&m
   */

  if (bind(fd, &sin, sizeof(struct sockaddr_in)) < 0) 
    {
      perror("Can't bind socket");
      exit(ERROR);
    }
	
  if (chdir(LOG_DIR) == -1) 
    {
      perror(LOG_DIR);
      log_error("Can't change wdir.");
    }

  if (!string_eq(hostname, daemon_host_entry->h_name)) {
#ifdef TEST
      log_error("warning: hesiod information doesn't point here");
      strcpy (DaemonHost, hostname);
#else
      log_error("error: hesiod information doesn't point here; exiting");
      return 1;
#endif
  }

#ifdef KERBEROS
  setenv("KRBTKFILE",TICKET_FILE,TRUE);
#endif KERBEROS

  log_status (fmt ("%s startup....", ME));

  load_db();
  load_data();
  flush_olc_userlogs();
  
  /*
   * shhhh!
   */

  if (listen(fd, SOMAXCONN)) 
    {
      fflush(stderr);
      perror("listen");
      log_error("aborting...");
      exit(1);
    }

  processing_request = 0;
  got_signal = 0;

  signal(SIGINT, punt);	        /* ^C on control tty (for test mode) */
  signal(SIGHUP, punt);	        /* kill -1 $$ */
  signal(SIGTERM, punt);	/* kill $$ */
  signal(SIGPIPE, SIG_IGN);

  get_kerberos_ticket ();

  /*
   * Wait for requests (hum drum life of a server)
   */

  while (TRUE) 
    {
      int s;		        /* Duplicated file descriptor */
      int len = sizeof (from);  /* Length of address. */

      s = accept(fd, &from, &len);		
      if (s < 0) 
	{
	  if (errno == EINTR)
	    continue;
	  perror("accept");
	  log_error("Error accepting connection.");
	  n_errs++;
	  if (n_errs < 3)
	    continue;
	  else if (n_errs > 10)
	    abort();
	  else if (errno == ECONNABORTED) 
	    {
	      log_error("Restarting...(error reading request)");
	      close(fd);
	      goto restart;
	    }
	  else
	    abort();
	}
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

process_request(fd, from)
      int fd;                 /* File descriptor for socket. */
      struct sockaddr_in *from;
{
  REQUEST request;	/* Request structure from client. */
  int type;		/* Type of request. */
  int index;		/* Index in proc. table. */
  int auth;             /* status from authentication */
  struct hostent *hp;	/* host sending request */
  int f;

  index = 0;
  processing_request = 1;
  if(read_request(fd, &request) != SUCCESS)
    {
      log_error("Error in reading request");
      return(ERROR);
    }
  type = request.request_type;

#ifdef KERBEROS
  /*
   * make sure olc has a valid tgt before each request
   */

  get_kerberos_ticket();
#endif KERBEROS

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

  auth = authenticate(&request, from->sin_addr);
  if (auth)
    {
      send_response(fd,auth);
      return;
    }
  while  ((type != Proc_List[index].proc_code)
	  && (Proc_List[index].proc_code != UNKNOWN_REQUEST))
    index++;

  if (Proc_List[index].proc_code != UNKNOWN_REQUEST) 
    {

      ++request_count;
#if 0
      printf("%d> Got %s request from %s\n",request_count,
	     Proc_List[index].description,
	     request.requester.username);
#endif

      (*(Proc_List[index].olc_proc))(fd, &request, auth);
    }
  else 
    {
      send_response(fd, UNKNOWN_REQUEST);
#ifdef	TEST
      printf("Got unknown request from '%s' (#%d)\n",
	     request.requester.username, type);
#endif	TEST

    }
  

  /*
   * make a backup of data after request complete (if data changed)
   */

  if (needs_backup)
    backup_data();
  processing_request = 0;
}


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
  struct direct *dp;
  dirp = opendir(LOG_DIR);

  if (dirp == (DIR *)NULL) 
    {
      perror(LOG_DIR);
      log_error("Can't flush lost userlogs");
      return;
    }

  for (dp = readdir(dirp); dp != (struct direct *)NULL; dp = readdir(dirp)) 
    {
      if (!strcmp(dp->d_name+dp->d_namlen-3, ".log")) 
	{
	  char msgbuf[BUFSIZ];
	  (void) strcpy(msgbuf, "Found log file ");
	  (void) strncat(msgbuf, dp->d_name, dp->d_namlen);
	  log_status(msgbuf);
	  /* check for username among active questions */
	}
    }
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

int
punt(sig)
    int sig;
{
  olc_broadcast_message("syslog",
			fmt ("%s shutting down on signal %d.", ME, sig),
			"system");

  close(listening_fd);
  if (processing_request) 
    {
      got_signal = 1;
      log_status("Caught signal, will exit after finishing request");
    }
  else 
    {
      log_status("Caught signal, exiting...");
      exit(1);
    }
}


/*
 * Function:	authenticate()  check the kerberos tickets from the client
 *			
 * Arguments:	REQUEST *request     
 * Returns:	ERROR if cannot authenticate
 * Notes:       F, C, B flat
 *	       
 */


authenticate(request, addr)
     REQUEST *request;
     unsigned long addr;
{

#ifdef KERBEROS
  AUTH_DAT data;
#endif KERBEROS
  char mesg[BUFSIZ];

  int result;

#ifndef KERBEROS
  return(SUCCESS);
#else KERBEROS

  result = krb_rd_req(&(request->kticket),K_SERVICE,K_INSTANCEbuf,
		      addr,&data,SRVTAB_FILE);

#if 0
  printf("kerberos result: %d\n",result);
#endif TEST

  strcpy(request->requester.username,data.pname);
  strcpy(request->requester.realm,data.prealm);

  return(result);
#endif KERBEROS
}


#ifdef KERBEROS
int
get_kerberos_ticket()
{
  int ret;
  char sinstance[INST_SZ];
  char principal[ANAME_SZ];
  char *ptr;

  strcpy(principal,K_SERVICE);
  strcpy(sinstance,DaemonHost);
  ptr = index(sinstance,'.');
  if (ptr)
      *ptr = '\0';
  uncase(sinstance);

  if(ticket_time < NOW - ((96L * 5L) - 15L) * 60)
    {
	log_error (fmt ("get new tickets: %s %s ", K_SERVICE, sinstance));
	dest_tkt();
	ret = krb_get_svc_in_tkt(K_SERVICE, sinstance, SERVER_REALM, 
				 "krbtgt", SERVER_REALM,    
				 96, SRVTAB_FILE);
	if (ret != KSUCCESS) {
	    log_error (fmt ("get_tkt: %s", krb_err_txt[ret]));
	    ticket_time = 0L;
	    return(ERROR);
	}
	else
	    ticket_time = NOW;
    }
  return(SUCCESS);
}
#endif KERBEROS
