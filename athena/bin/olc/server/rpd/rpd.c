/*
 * Log Replayer Daemon
 *
 * This replays question logs
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Id: rpd.c,v 1.21 1999-06-28 22:52:52 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "rpd.h"

int sock;	/* the listening socket */
int fd;		/* the accepting socket */

#ifdef PROFILE
static int dump_profile (int sig );
static int start_profile (int sig );
#endif /* PROFILE */

static void clean_up (int sig);

main(argc, argv)
     int argc;
     char **argv;
{
  int len,max_fd;
#ifdef RLIMIT_NOFILE
  struct rlimit rl;
#endif
  struct sockaddr_in sin,from;
  struct servent *sent;
  int onoff;
  int nofork=0;
  int arg;
  struct sigaction action;

#ifdef PROFILE
    /* Turn off profiling on startup; that way, we collect "steady state" */
    /* statistics, not the initial flurry of startup activity */
    moncontrol(0);
#endif

  for (arg=1;arg< argc; arg++) {
    if (!strcmp(argv[arg],"-nofork")) {
      nofork = 1;
      continue;
    }
    else {
      fprintf (stderr,"rpd: Unknown argument: %s\n",argv[arg]);
      exit(1);
    }
  }


  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = clean_up;

  sigaction(SIGHUP, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);
#ifdef PROFILE
  action.sa_handler = dump_profile;      /* Dump profiling information and */
  sigaction(SIGUSR1, &action, NULL);     /* stop profiling */

  action.sa_handler = start_profile;     /* Start profiling */
  sigaction(SIGUSR2, &action, NULL);
#endif /* PROFILE */

#ifdef HAVE_SYSLOG
  openlog("rpd", LOG_CONS | LOG_PID, SYSLOG_FACILITY);
#endif /* HAVE_SYSLOG */

#ifndef SABER
  if (!nofork) {
    /* Fork off */
    switch (fork()) {
    case 0:		/* Child */
      break;
    case 1:
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"Can't fork: %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    default:
      exit(0);		/* Parent */
    }
#endif
#ifdef RLIMIT_NOFILE
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
      max_fd = OPEN_MAX; /* either that or abort()... --bert 29jan1996 */
    else
      max_fd = (int)rl.rlim_cur;
#else
    max_fd = getdtablesize();
#endif
    
    for(fd = 0;fd<max_fd;fd++)
      close(fd);
    fd = open("/",O_RDONLY,0);
    if (fd < 0) {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"Can't open /: %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    }
    if (fd != 0)
      dup2 (fd, 0);
    if (fd != 1)
      dup2 (fd, 1);
    if (fd != 2)
      dup2 (fd, 2);
    if (fd > 2)
      close (fd);
    
    fd = open("/dev/tty",O_RDWR,0);
    if (fd>=0) {
      ioctl(fd, TIOCNOTTY, (char *) NULL);
      close(fd);
    }
  }

  init_cache();

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"socket: %m");
#endif /* HAVE_SYSLOG */
    exit(1);
  }

  sent = (struct servent *) NULL;
#ifdef HAVE_HESIOD
  sent = hes_getservbyname(RPD_SERVICE_NAME, OLC_PROTOCOL);
#endif
  if (sent == NULL) {
    sent = getservbyname(RPD_SERVICE_NAME, OLC_PROTOCOL);
    if (sent == NULL) {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,
	     "getservbyname(" RPD_SERVICE_NAME "/" OLC_PROTOCOL "): %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    }
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = sent->s_port;

  onoff = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&onoff,
		 sizeof(int)) < 0)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"setsockopt: %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"bind: %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    }

  if (listen(sock,SOMAXCONN) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"bind: %m");
#endif /* HAVE_SYSLOG */
      exit(1);
    }

  /* chdir so cores get dumped in the right directory */
  if (chdir(CORE_DIR) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR, "chdir: %m");
#endif /* HAVE_SYSLOG */
    }

#ifdef HAVE_SYSLOG
  syslog(LOG_INFO,"Ready to accept connections..");
#endif /* HAVE_SYSLOG */

  /* Main loop... */

  while (1)
    {
      len = sizeof(from);
      fd = accept(sock,(struct sockaddr *) &from, &len);
      if (fd == -1)
	{
	  if (errno == EINTR)
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_WARNING,
		     "Interrupted by signal in accept; continuing");
#endif /* HAVE_SYSLOG */
	      continue;
	    }
#ifdef HAVE_SYSLOG
	  syslog(LOG_ERR,"accept: %m");
#endif /* HAVE_SYSLOG */
	  exit(1);
	}
      handle_request(fd,from);
      close(fd);
    }
}

#ifdef PROFILE
static int
dump_profile(sig)
     int sig;
{
#ifdef HAVE_SYSLOG
  syslog(LOG_INFO,"Dumping profile..");
#endif /* HAVE_SYSLOG */
  monitor(0);
  moncontrol(0);
  return 0;
}

static int
start_profile(sig)
     int sig;
{
#ifdef HAVE_SYSLOG
  syslog(LOG_INFO,"Starting profile..");
#endif /* HAVE_SYSLOG */
  moncontrol(1);
  return 0;
}
#endif /* PROFILE */

static void
clean_up(int signal)
{
  close(fd);
  close(sock);
#ifdef HAVE_SYSLOG
  syslog(LOG_NOTICE,"rpd shutting down on signal %d",signal);
#endif /* HAVE_SYSLOG */
  exit(0);
  return;
}
