/**********************************************************************
 * usage tracking module
 *
 * $Author: lwvanels $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/log.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/log.c,v 1.1 1991-03-01 12:20:38 lwvanels Exp $
 *
 * Copyright (c) 1990, Massachusetts Institute of Technology
 **********************************************************************/

#ifndef lint
#ifndef SABER
static char rcsid_viewer_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/log.c,v 1.1 1991-03-01 12:20:38 lwvanels Exp $";
#endif
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>

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
  char *t,*p;
  char *ws_type;
  struct hostent *hp, *gethostbyname();
  time_t now;
  fd_set readfds;
  int nfound,len;
  struct timeval timeout;

  if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    punt = 1;
    return;
  }

  if ((hp = gethostbyname("brennin.mit.edu")) == NULL) {
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

  if (getuid() == 1)
    ws_type = "pre-login";
  else {
    if (strncmp(hostnm,"e40-008",7) == 0)
      ws_type = "dialup";
    else
      ws_type = "workstation";
  }
  now = time(0);
  t = ctime(&now);
  p = index(t,'\n');
  if (p != NULL) *p = '\0';
  sprintf(buf,"STARTUP %s %s %s %s",t,ws_type,hostnm,type);

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
log_view(node_id)
     char *node_id;
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
  sprintf(buf,"MODULE %d %s %s", session_id, t, node_id);
  send(fd, buf, strlen(buf), 0);
  return;
}
