/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the primary functions of the polling daemon, polld.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/polld.c,v $
 *	$Id: polld.c,v 1.6 1991-04-08 21:17:09 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/polld.c,v 1.6 1991-04-08 21:17:09 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olcd.h>
#include <polld.h>

#include <signal.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/ioctl.h>

/* Global variables. */

int select_timeout = 600;
char DaemonHost[NAME_SIZE];
char DaemonInst[20];

/* Static procedure definitions */

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int clean_up P(( int sig ));

#undef P

/* Static vars */

static int listening_fd;

static int
clean_up(sig)
     int sig;
{
  close(listening_fd);
  syslog(LOG_INFO,"Exiting on signal %d",sig);
  exit(1);
}

/*
 * Function:	main() is the start-up function for the polling daemon.
 * Arguments:	argc:	Number of command line arguments.
 *		argv:	Array of words from the command line.
 * Returns:	Never returns.
 * Notes:
 *	Fork a new daemon to separate it from the terminal,
 *	rebinding the standard error output so we can trap it.
 *	Then, go into a loop to poll the list of users regularly, and send
 *	results back to the main daemon.
 */

main(argc, argv)
     int argc;
     char *argv[];
{
  int nofork = 0;
  int i;
  long last_cycle = 0L;
  struct timeval tp;
  PTF *users;
  int n_people, max_people;
  int fd;
  int retval;
  char *dhost = NULL;

#ifdef PROFILE
  /* Turn off profiling on startup; that way, we collect "steady state" */
  /* statistics, not the initial flurry of startup activity */
  moncontrol(0);
#endif
  
  strcpy(DaemonInst,"OLC");

  for (i=1; i< argc; i++) {
    if (!strcmp (argv[i], "-nofork") || !strcmp (argv[i], "-no_fork")) {
      nofork = 1;
    }
    else if (!strcmp(argv[i],"-server") || !strcmp (argv[i],"-host")) {
      if (!argv[++i])
	fprintf (stderr, "-host requires a host name \n");
      else
        dhost = argv[i];
    }
    else if (!strcmp (argv[i], "-inst")) {
      if (!argv[++i])
	fprintf (stderr, "-inst requires an instance name\n");
      else
	strcpy(DaemonInst, argv[i]);
      upcase_string(DaemonInst);
    }
    else {
      fprintf (stderr, "unknown argument: %s\n",argv[i]);
      exit(1);
    }
  }
  
#ifdef HESIOD
  if (dhost == NULL) {
    char **hp;
    if ((hp = hes_resolve(DaemonInst,"sloc")) == NULL) {	
      syslog(LOG_ERR,"Unable to find %s service location in hesiod",
	     OLC_SERVICE);
      exit(1);
    }
    else
      dhost = *hp;
  }
#endif /* HESIOD */

  if (dhost == NULL) {
    fprintf (stderr, "Can't find OLC server host!\n");
    exit (1);
  }

  strcpy(DaemonHost,dhost);

  signal(SIGHUP,clean_up);
  signal(SIGINT,clean_up);
  signal(SIGTERM,clean_up);
  signal(SIGPIPE,SIG_IGN);
#ifdef PROFILE
  signal(SIGUSR1, dump_profile); /* Dump profiling information and stop */
  /* profiling */
  signal(SIGUSR2, start_profile); /* Start profiling */
#endif /* PROFILE */
  
#if defined(ultrix)
#ifdef LOG_CONS
  openlog ("polld", LOG_CONS | LOG_PID);
#else
  openlog ("polld", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
  openlog ("polld", LOG_CONS | LOG_PID,SYSLOG_LEVEL);
#else
  openlog ("polld", LOG_PID, SYSLOG_LEVEL);
#endif /* LOG_CONS */
#endif /* ultrix */

#ifdef SABER
  nofork = 1;
#endif
  
  /*
   * fork off
   */
  
  if (!nofork) {
    int max_fd = getdtablesize ();
    
    switch (fork()) {
    case 0:				/* child */
      break;
    case -1:			/* error */
      perror ("Can't fork");
      exit(-1);
    default:			/* parent */
      exit(0);
    }
    
    for (fd = 0; fd < max_fd; fd++)
      (void) close (fd);
    fd = open ("/", O_RDONLY);
    if (fd < 0) {
      perror ("Can't open /");
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
  
  setlinebuf(stdout);
  setlinebuf(stderr);
  
  /* handle setuid-ness, etc., so we can dump core */
  setreuid((uid_t) geteuid(), -1);
  setregid((gid_t) getegid(), -1);
  
  init_cache();

  syslog(LOG_INFO,"Ready to start polling...");
  
#ifdef ZEPHYR
  if ((retval = ZInitialize()) != ZERR_NONE)
    syslog(LOG_ERR,"Error in ZInitialize: %s",error_message(retval));
#endif /* ZEPHYR */
    
  n_people = 0;
  max_people = 100;
  users = (PTF *)calloc(max_people,sizeof(PTF));
  if (users == NULL) {
    syslog(LOG_ERR,"Cannot calloc for user list");
    exit(1);
  }

  while(1) {
    gettimeofday(&tp,0);
    if ((tp.tv_sec -last_cycle) < CYCLE_TIME*60)
      /* If it's been less than CYCLE_TIME minutes, sleep until it's time to */
      /* do another cycle..*/
      sleep((unsigned int) ((last_cycle + CYCLE_TIME*60) - tp.tv_sec));

    last_cycle = tp.tv_sec;
    n_people = get_user_list(users,&max_people);
    if (n_people < 0) {
       /* Error reading list */
       /* Give up on this cycle- try again later */
       continue;
     }

    for(i=0;i<n_people;i++) {
      /* get incoming datagrams for who's (later) */
      switch (locate_person(&users[i])) {
      case LOC_ERROR: /* Error */
	break;
      case LOC_NO_CHANGE: /* No change in status */
	break;
      case LOC_CHANGED: /* Changed status */
	tell_main_daemon(users[i]);
	break;
      }
    }
  }
}
