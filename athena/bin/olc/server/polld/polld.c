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
 *	$Id: polld.c,v 1.16 1999-06-10 18:41:34 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: polld.c,v 1.16 1999-06-10 18:41:34 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olcd.h>
#include <polld.h>

#include <com_err.h>

#include <signal.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <limits.h>

#ifdef   HAVE_SYSLOG_H
#include   <syslog.h>
#ifndef    LOG_CONS
#define      LOG_CONS 0  /* if LOG_CONS isn't defined, just ignore it */
#endif     /* LOG_CONS */
#endif /* HAVE_SYSLOG_H */

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

static RETSIGTYPE clean_up P((int sig));

#undef P

/* Static vars */

static int listening_fd;

static RETSIGTYPE
clean_up(int sig)
{
  close(listening_fd);
#ifdef HAVE_SYSLOG
  syslog(LOG_INFO,"Exiting on signal %d",sig);
#endif /* HAVE_SYSLOG */
  exit(1);
  return;
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

int main(int argc, char **argv)
{
  int nofork = 0;
  int i;
  int cycle=0;
  long last_cycle = 0L;
  struct timeval tp;
  PTF *users;
  int n_people, max_people;
  int fd;
  int retval;
  char *dhost = NULL;
  struct sigaction action;

#ifdef PROFILE
  /* Turn off profiling on startup; that way, we collect "steady state" */
  /* statistics, not the initial flurry of startup activity */
  moncontrol(0);
#endif
  
  strcpy(DaemonInst, OLXX_SERVICE);
  upcase_string(DaemonInst);

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
    else if (!strcmp (argv[i], "-cycle")) {
      cycle = 1;
    }
    else {
      fprintf (stderr, "unknown argument: %s\n",argv[i]);
      exit(1);
    }
  }
  
  if (dhost == NULL) {
    if (gethostname(DaemonHost, sizeof(DaemonHost)) < 0) {
      fprintf (stderr, "Can't find server host name!\n");
      exit (1);
    }
  } else {
    strcpy(DaemonHost, dhost);
  }

#ifdef HAVE_KRB4
  set_env_var("KRBTKFILE", TICKET_FILE);    /* piggyback on olcd's tickets */
#endif /* HAVE_KRB4 */

  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = clean_up;

  sigaction(SIGHUP, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);
  
#ifdef HAVE_SYSLOG
  openlog ("polld", LOG_CONS | LOG_PID, SYSLOG_FACILITY);
#endif /* HAVE_SYSLOG */

#ifdef SABER
  nofork = 1;
#endif
  
  /*
   * fork off
   */
  
  if (!nofork) {
    int max_fd;
#ifdef RLIMIT_NOFILE
    struct rlimit rl;

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
      max_fd = OPEN_MAX; /* either that or abort()... --bert 29jan1996 */
    else
      max_fd = (int)rl.rlim_cur;
#else
    max_fd = getdtablesize ();
#endif
    
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
  
#ifdef HAVE_SETVBUF
  setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
  setvbuf(stderr,NULL,_IOLBF,BUFSIZ);
#else
  setlinebuf(stdout);
  setlinebuf(stderr);
#endif

  init_cache();

#ifdef HAVE_SYSLOG
  syslog(LOG_INFO,"Ready to start polling...");
#endif /* HAVE_SYSLOG */
  
#ifdef HAVE_ZEPHYR
  initialize_zeph_error_table();
  retval = ZInitialize();
  if (retval != ZERR_NONE)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"Error in ZInitialize: %s",error_message(retval));
#endif /* HAVE_SYSLOG */
    }
#endif /* HAVE_ZEPHYR */
    
  /* Incarnate polld with minimum data, so we won't need a config file */
  if (incarnate_hardcoded("polld", OLXX_SERVICE, DaemonHost) != SUCCESS) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR, "Program incarnation failed???");
#endif /* HAVE_SYSLOG */
  }

  n_people = 0;
  max_people = 100;
  users = (PTF *)calloc(max_people,sizeof(PTF));
  if (users == NULL) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"Cannot calloc for user list");
#endif /* HAVE_SYSLOG */
    exit(1);
  }

  do {
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

#ifdef HAVE_ZEPHYR
    check_zephyr();
#endif

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
  while(cycle);

#ifdef HAVE_SYSLOG
  syslog(LOG_INFO, "Polling completed, exiting.");
#endif /* HAVE_SYSLOG */
  exit(0);
}
