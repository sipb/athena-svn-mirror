/**********************************************************************
 * usage tracking daemon
 *
 * $Author: lwvanels $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v 1.4 1991-04-10 15:32:13 lwvanels Exp $
 *
 * Copyright (c) 1990, Massachusetts Institute of Technology
 **********************************************************************/

#ifndef lint
#ifndef SABER
static char rcsid_[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/bbd.c,v 1.4 1991-04-10 15:32:13 lwvanels Exp $";
#endif
#endif

#include <syslog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int log_fd;

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
    if ((fd = open(cfile,O_RDONLY,0644)) < 0) {
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
  if ((fd = open(cfile,O_WRONLY|O_CREAT,0644)) < 0) {
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

  write(log_fd,msg,len);
  write(log_fd," ",1);
  write(log_fd,buf,strlen(buf));
}

main(argc, argv)
     int argc;
     char *argv[];
{
  char *logfile=NULL;
  int nofork = 0;
  int fd;
  struct sockaddr_in name,from;
  char buf[1024];
  int onoff;
  int len,rlen,i;

  for (i=1;i<argc;i++) {
    if (!strcmp(argv[i], "-nofork")) {
      nofork = 1;
      continue;
    }
    logfile = argv[i];
  }

  if (logfile == NULL) {
    fprintf(stderr,"usage: bbd [-nofork] logfile\n");
    exit(1);
  }

#ifdef SABER
  nofork = 1;
#endif

  if (!nofork) {
    int max_fd = getdtablesize ();
    
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

  if ((log_fd = open(logfile,O_WRONLY|O_CREAT|O_APPEND,0600)) < 0) {
    syslog(LOG_ERR,"opening %s: %m");
    exit(1);
  }

#ifdef  mips
  openlog("bbd",LOG_PID);
#else
  openlog("bbd",LOG_CONS|LOG_PID,LOG_LOCAL2);
#endif

  /* Create socket */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    syslog(LOG_ERR,"opening datagram socket: %m");
    exit(1);
  }

  /* Create name with wildcards */

  name.sin_family = AF_INET;
  name.sin_port = htons(2052);

  onoff = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int))) {
    syslog(LOG_ERR,"setsockopt: %m");
    exit(1);
  }

  if (bind(fd, (struct sockaddr *) &name, sizeof(struct sockaddr_in)) < 0) {
    syslog(LOG_ERR,"Can't bind socket: %m");
    exit(1);
  }
  
  while (1) {
    len = sizeof(struct sockaddr_in);
    if ((rlen = recvfrom(fd,buf,1024,0,&from,&len)) < 0) {
      syslog(LOG_ERR,"recvfrom: %m");
      continue;
    }
    if (buf[0] == 'S')
      handle_startup(fd,&buf[1],rlen,from,logfile);
    else {
      write(log_fd,&buf[1],rlen);
      write(log_fd,"\n",1);
    }
  }
}

