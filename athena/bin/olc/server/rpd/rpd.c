/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/rpd.c,v 1.7 1990-12-31 20:49:36 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

int sock;	/* the listening socket */
int fd;		/* the accepting socket */

main()
{
  int len,max_fd;
  struct sockaddr_in sin,from;
  struct servent *sent;
  int onoff;

  signal(SIGHUP,clean_up);
  signal(SIGINT,clean_up);
  signal(SIGTERM,clean_up);
  signal(SIGPIPE,SIG_IGN);

#ifdef mips
  openlog("rpd",LOG_PID);  /* Broken ultrix syslog.. */
#else
  openlog("rpd",LOG_CONS | LOG_PID,SYSLOG_FACILITY);
#endif

#ifndef SABER
  /* Fork off */
  switch (fork()) {
  case 0:		/* Child */
    break;
  case 1:
    syslog(LOG_ERR,"Can't fork: %m");
    exit(1);
  default:
    exit(0);		/* Parent */
  }
#endif
  max_fd = getdtablesize();

  for(fd = 0;fd<max_fd;fd++)
    close(fd);
  fd = open("/",O_RDONLY,S_IREAD);
  if (fd < 0) {
    syslog(LOG_ERR,"Can't open /: %m");
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

  init_cache();

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    syslog(LOG_ERR,"socket: %m");
    exit(1);
  }

  if ((sent = getservbyname("ols","tcp")) == NULL) {
    syslog(LOG_ERR,"ols/tcp unknown service");
    exit(1);
  }

  bzero(&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = sent->s_port;

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int)) < 0) {
    syslog(LOG_ERR,"setsockopt: %m");
    exit(1);
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    syslog(LOG_ERR,"bind: %m");
    exit(1);
  }

  if (listen(sock,SOMAXCONN) == -1) {
    syslog(LOG_ERR,"bind: %m");
    exit(1);
  }

  syslog(LOG_INFO,"Ready to accept connections..");

  /* Main loop... */

  while (1) {
    len = sizeof(from);
    if ((fd = accept(sock,(struct sockaddr *) &from, &len)) == -1) {
      if (errno == EINTR) {
	syslog(LOG_WARNING,"Interrupted by signal in accept; continuing");
	continue;
      }
      syslog(LOG_ERR,"accept: %m");
      exit(1);
    }
    handle_request(fd,from);
    close(fd);
  }
}

int
clean_up(signal)
     int signal;
{
  close(fd);
  shutdown(sock,2);
  close(sock);
  syslog(LOG_NOTICE,"rpd shutting down on signal %d",signal);
  exit(0);
}
