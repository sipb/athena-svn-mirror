/**********************************************************************
 * usage tracking library
 *
 * $Author: lwvanels $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/log.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/log.c,v 1.5 1991-04-14 20:40:43 lwvanels Exp $
 *
 * Copyright (c) 1990, Massachusetts Institute of Technology
 **********************************************************************/

#ifndef lint
#ifndef SABER
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/logger/log.c,v 1.5 1991-04-14 20:40:43 lwvanels Exp $";
#endif
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#ifdef _AUX_SOURCE
#include <time.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>

#ifdef NEEDS_SELECT_MACROS
#define NBBY    8 /* number of bits in a byte */
#define NFDBITS (sizeof(long) * NBBY)        /* bits per mask */

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))

#endif

static int session_id;
static int fd;
static int punt = 0;
static struct sockaddr_in name;

void
log_startup(type,log_host)
     char *type;
     char *log_host;
{
  char buf[BUFSIZ];
  char hostnm[MAXHOSTNAMELEN];
  char *t,*p;
  struct hostent *hp, *gethostbyname();
  time_t now;
  fd_set readfds;
  int nfound,len;
  struct timeval timeout;

  if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    punt = 1;
    return;
  }

  if ((hp = gethostbyname(log_host)) == NULL) {
    punt = 1;
    return;
  }
  
  bcopy(hp->h_addr,&name.sin_addr,hp->h_length);
  name.sin_family = AF_INET;
  name.sin_port = htons(2052);
  
  if (connect(fd, &name, sizeof(name))< 0) {
    punt = 1;
    return;
  }

  gethostname(hostnm,MAXHOSTNAMELEN);

  now = time(0);
  t = ctime(&now);
  p = index(t,'\n');
  if (p != NULL) *p = '\0';
  sprintf(buf,"S%s %s %s",t,hostnm,type);

  if (send(fd, buf, strlen(buf), 0) <0)
    return;
  
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
    
  FD_ZERO(&readfds);
  FD_SET(fd,&readfds);
  nfound = select(fd+1,&readfds,NULL,NULL,&timeout);
  if (nfound != 0) {
    len = recv(fd,buf,1024,0);
    if (len > 0)
      session_id = atoi(buf);
  }
}

void
log_view(view_id)
     char *view_id;
{
  time_t now;
  char buf[BUFSIZ];
  char *t, *p;

  if (punt || !fd)
    return;
  
  now = time(0);
  t = ctime(&now);
  p = index(t,'\n');
  if (p != NULL)
    *p = '\0';
  sprintf(buf,"M%d %s %s", session_id, t, view_id);
  send(fd, buf, strlen(buf), 0);
  return;
}


