/**********************************************************************
 * usage tracking daemon
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v 1.15 1997-04-30 18:18:34 ghudson Exp $
 *
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 **********************************************************************/

#ifndef lint
#ifndef SABER
static char rcsid_[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v 1.15 1997-04-30 18:18:34 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <limits.h>
#include <netinet/in.h>
#include <netdb.h>
#if defined(__STDC__)
#include <stdlib.h>
#endif
#ifdef POSIX
#include <termios.h>
#endif

#define SERVICE_NAME "ols"

char *lf;
int log_fd;
int tick=0;

#ifdef NEEDS_ERRNO_DEFS
extern int      errno;
extern char     *sys_errlist[];
extern int      sys_nerr;
#endif

void
handle_startup(s,msg,len,from,logfile)
     int s;
     char *msg;
     int len;
     struct sockaddr_in from;
     char *logfile;
{
  static char cfile[MAXPATHLEN];
  static int counter;
  int fd;
  char buf[BUFSIZ];
  int size;
  
  if (cfile[0] == '\0') {
    /* initialize filename and counter */
    sprintf(cfile,"%s.cnt",logfile);
    fd = open(cfile,O_RDONLY,0644);
    if (fd < 0) {
      if (errno != ENOENT) {
	syslog(LOG_ERR,"Could not open counter file %s: %m", cfile);
	exit(1);
      }
      else
	counter = 0;
    }
    else {
      size = read(fd,buf,BUFSIZ);
      close(fd);
      buf[size] = '\0';
      counter = atoi(buf);
    }
  }

  counter++;

  /* Save it to the counter file */
  sprintf(buf,"%d\n",counter);
  fd = open(cfile,O_WRONLY|O_CREAT,0644);
  if (fd < 0) {
    syslog(LOG_ERR,"Could not open counter file %s for writing: %m", cfile);
    exit(1);
  }
  write(fd,buf,strlen(buf));
  close(fd);

  if (sendto(s, buf, strlen(buf), 0, &from, sizeof(from)) <0) {
    syslog(LOG_ERR,"Error sending datagram to %s/%d: %m",
	   inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    close(fd);
    exit(1);
  }
    
  close(fd);

  write(log_fd,"START ",6);
  write(log_fd,msg,len);
  write(log_fd," ",1);
  write(log_fd,buf,strlen(buf));
}

#ifdef VOID_SIGRET
void
#else
int
#endif
do_tick(sig)
     int sig;
{
  long now;

#ifdef POSIX
  struct sigaction action;

  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = do_tick;
  sigaction(SIGALRM, &action, NULL);
#else /* not POSIX */
  signal(SIGALRM,do_tick);
#endif /* not POSIX */

  alarm(60 * tick);
  now = time(0);
  write(log_fd,"TICK ",5);
  write(log_fd,ctime(&now),25);
#ifdef VOID_SIGRET
  return;
#else
  return(0);
#endif
}

#ifdef VOID_SIGRET
void
#else
int
#endif
handle_hup(sig)
     int sig;
{
#ifndef POSIX
  signal(SIGHUP,handle_hup);    /* Reset the handler if needed (non-POSIX) */
#endif
  close(log_fd);
  log_fd = open(lf,O_WRONLY|O_CREAT|O_APPEND,0600);
  if (log_fd < 0) {
    syslog(LOG_ERR,"opening %s: %m");
    exit(1);
  }
  do_tick(0);
#ifdef VOID_SIGRET
  return;
#else
  return(0);
#endif
}

main(argc, argv)
     int argc;
     char *argv[];
{
  int nofork = 0;
  int fd;
  struct sockaddr_in name,from;
  struct servent *service;
  char buf[1024];
  int onoff;
  int len,rlen,i;
  int port=0;
#ifdef POSIX
  sigset_t oldmask,alarmmask;
  struct sigaction action;
#else
  int oldmask,alarmmask;
#endif
  char *pidfile = "/usr/local/bin/bbd.pid";

  for (i=1;i<argc;i++) {
    if (!strcmp(argv[i], "-nofork")) {
      nofork = 1;
      continue;
    }
    if (!strcmp(argv[i], "-port")) {
      if (i+1 == argc) {
	fprintf(stderr,"Must specify port number with -port\n");
	break;
      }
      port = atoi(argv[++i]);
      continue;
    }
    if (!strcmp(argv[i], "-tick")) {
      if (i+1 == argc) {
	fprintf(stderr,"Must specify number of minutes with -tick\n");
	break;
      }
      tick = atoi(argv[++i]);
      continue;
    }
    if (!strcmp(argv[i], "-pidfile")) {
      if (i+1 == argc) {
	fprintf(stderr,"Must specify pid filename with -pidfile\n");
	break;
      }
      pidfile = argv[++i];
      continue;
    }
    lf = argv[i];
  }

  if (lf == NULL) {
    fprintf(stderr,"usage: bbd\n");
    fprintf(stderr,"       [-nofork]\n");
    fprintf(stderr,"       [-port portno]\n");
    fprintf(stderr,"       [-tick interval]\n");
    fprintf(stderr,"       [-pidfile filename]\n");
    fprintf(stderr,"       logfile\n");
    exit(1);
  }

#ifdef SABER
  nofork = 1;
#endif

  if (!nofork) {
    int max_fd;
#ifdef RLIMIT_NOFILE
    struct rlimit rl;

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
      max_fd = OPEN_MAX; /* either that or abort()... --bert 29jan1996 */
    else
      max_fd = (int)rl.rlim_cur;
#else
    max_fd = getdtablesize();
#endif
    
    switch (fork()) {
    case 0:                             /* child */
      break;
    case -1:                    /* error */
      perror ("Can't fork");
      exit(-1);
    default:                    /* parent */
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
    
    fd = open("/dev/tty", O_RDWR, 0);
    if (fd >= 0) {
      ioctl(fd, TIOCNOTTY, (char *) NULL);
      (void) close (fd);
    }
  }

  log_fd = open(lf,O_WRONLY|O_CREAT|O_APPEND,0600);
  if (log_fd < 0) {
    syslog(LOG_ERR,"opening %s: %m",lf);
    exit(1);
  }

#ifdef ultrix
  openlog("bbd",LOG_PID);
#else
  openlog("bbd",LOG_CONS|LOG_PID,LOG_LOCAL2);
#endif

  unlink(pidfile);
  fd = open(pidfile,O_WRONLY|O_CREAT|O_TRUNC,0400);
  if (fd < 0) {
    syslog(LOG_ERR,"opening %s: %m",pidfile);
    exit(1);
  }

  sprintf(buf,"%d\n",getpid());
  write(fd,buf,strlen(buf));
  close(fd);

  /* Create socket */
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    syslog(LOG_ERR,"opening datagram socket: %m");
    exit(1);
  }


  /* Find port number if not already defined */
  if (port == 0) {
    service = getservbyname(SERVICE_NAME,"tcp");
    if (service == NULL) {
      syslog(LOG_ERR,"error getting service %s/udp: %m",SERVICE_NAME);
      exit(1);
    }
    port = service->s_port;
  }
  else 
    port = htons(port);

  /* Create name with wildcards */
  name.sin_family = AF_INET;
  name.sin_port = port;

  onoff = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int))) {
    syslog(LOG_ERR,"setsockopt: %m");
    exit(1);
  }

  if (bind(fd, (struct sockaddr *) &name, sizeof(struct sockaddr_in)) < 0) {
    syslog(LOG_ERR,"Can't bind socket: %m");
    exit(1);
  }
  
#ifdef POSIX
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = handle_hup;
  sigaction(SIGHUP, &action, NULL);
#else /* not POSIX */
  signal(SIGHUP,handle_hup);
#endif /* not POSIX */

  if (tick != 0) {
    do_tick(0);
  }

#ifdef POSIX
  sigemptyset(&alarmmask);
  sigaddset(&alarmmask, SIGALRM);
#else
  alarmmask = sigmask(SIGALRM);
#endif
  while (1) {
    len = sizeof(struct sockaddr_in);
    rlen = recvfrom(fd,buf,1024,0,&from,&len);
    if (rlen < 0) {
      if (errno != EINTR)
	syslog(LOG_ERR,"recvfrom: %m");
      continue;
    }
#ifdef POSIX
    sigprocmask(SIG_BLOCK, &alarmmask, &oldmask);
#else
    oldmask = sigblock(alarmmask);
#endif
    if (buf[0] == 'S')
      handle_startup(fd,&buf[1],(rlen-1),from,lf);
    else {
      write(log_fd,"VIEW ",5);
      write(log_fd,&buf[1],(rlen-1));
      write(log_fd,"\n",1);
    }
#ifdef POSIX
    sigprocmask(SIG_BLOCK, &oldmask, NULL);
#else
    (void) sigblock(oldmask);
#endif
  }
}

