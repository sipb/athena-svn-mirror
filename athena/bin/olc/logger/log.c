/**********************************************************************
 * usage tracking library
 *
 * $Id: log.c,v 1.14 1999-06-10 18:41:27 ghudson Exp $
 *
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 **********************************************************************/

#ifndef lint
#ifndef SABER
static char rcsid[] = "$Id: log.c,v 1.14 1999-06-10 18:41:27 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <hesiod.h>

#define HESIOD_LOG_TYPE	"tloc"

static int session_id;
static int fd;
static int punt = 0;
static struct sockaddr_in name;

void
log_startup(type)
     char *type;
{
  char buf[BUFSIZ];
  char hostnm[MAXHOSTNAMELEN];
  char log_host[MAXHOSTNAMELEN];
  char *t,*p;
  struct hostent *hp, *gethostbyname();
  time_t now;
  fd_set readfds;
  int nfound,len;
  struct timeval timeout;
  int port;
#ifdef HAVE_HESIOD  
  char **hesinfo;

/*
   Get location of usage logger from hesiod
   If the hesiod info doesn't exist, no logging to be done
   should be of the form "loggerhost port_num"
*/

  hesinfo = hes_resolve(type,HESIOD_LOG_TYPE);
  if (hesinfo == NULL) {
    punt = 1;
    return;
  }

  p = strchr(*hesinfo,' ');
  if (p) {
    *p++ = '\0';
    (void) strcpy(log_host,*hesinfo);
    port = htons(atoi(p));
  } else {
    punt = 1;
    return;
  }
#else
  /* No hesiod, have to hardcode it in */
  /* insert whatever's appropriate for you */
  strcpy(log_host,"whatever.foo.bar");
  port = htons(2051);
#endif
  fd = socket(AF_INET,SOCK_DGRAM,0);
  if (fd < 0) {
    punt = 1;
    return;
  }

  hp = gethostbyname(log_host);
  if (hp == NULL) {
    punt = 1;
    return;
  }
  
  memcpy(&name.sin_addr, hp->h_addr, hp->h_length);
  name.sin_family = AF_INET;
  name.sin_port = port;
  
  if (connect(fd, &name, sizeof(name))< 0) {
    punt = 1;
    return;
  }

  gethostname(hostnm,MAXHOSTNAMELEN);

  now = time(0);
  t = ctime(&now);
  p = strchr(t,'\n');
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
  p = strchr(t,'\n');
  if (p != NULL)
    *p = '\0';
  sprintf(buf,"M%d %s %s", session_id, t, view_id);
  send(fd, buf, strlen(buf), 0);
  return;
}


