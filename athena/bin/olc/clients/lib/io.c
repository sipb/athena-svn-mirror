/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for communication between the user programs
 * and the daemon.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: io.c,v 1.26 1999-06-28 22:51:49 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: io.c,v 1.26 1999-06-28 22:51:49 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>        

#include <sys/types.h>             /* System type declarations. */
#include <sys/socket.h>            /* Network socket defs. */
#include <sys/file.h>              /* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>              /* System time definitions. */
#include <sys/param.h>
#include <netinet/in.h>
#include <errno.h>                 /* System error numbers. */
#include <netdb.h>
#include <signal.h>


/*
 * Note: All functions that deal with I/O on sockets in this file use the
 *	functions "sread()" and "swrite()", which check to ensure that the
 *	socket is, in fact, connected to something.
 */

/*
 * Function:	send_request() sends an OLC request to the daemon from a user
 *			process.
 * Arguments:	fd:		File descriptor to write to.
 *		request:	A pointer to the request structure to send.
 * Returns:	SUCCESS if the write is successful, ERROR otherwise.
 * Notes:
 *	Write the request structure to the file descriptor, checking to make
 *	sure that the write was successful.
 */

ERRCODE
send_request(fd, request)
     int fd;
     REQUEST *request;
{
  IO_REQUEST net_req;
  long i;

  int klength;

  memset(&net_req, 0, sizeof(net_req));

/* build up struct to send over */

  i = htonl((u_long) CURRENT_VERSION);
  memcpy(net_req.data, &i, sizeof(i));
  
  i = htonl((u_long) request->request_type);
  memcpy(net_req.data+4, &i, sizeof(i));

  i = htonl((u_long) request->options);
  memcpy(net_req.data+8, &i, sizeof(i));

/* options unset */

  i = htonl((u_long) request->requester.uid);
  memcpy(net_req.data+16, &i, sizeof(i));

  i = htonl((u_long) request->requester.instance);
  memcpy(net_req.data+20, &i, sizeof(i));

  strncpy(net_req.data+24, request->requester.username,  10);
  strncpy(net_req.data+34, request->requester.realname,  32);
#ifdef HAVE_KRB4
  strncpy(net_req.data+66, request->requester.realm,     40);
  strncpy(net_req.data+106, request->requester.inst,     40);
#endif
  strncpy(net_req.data+146, request->requester.nickname, 16);
  strncpy(net_req.data+162, request->requester.title,    32);
  strncpy(net_req.data+194, request->requester.machine,  32);

  i = htonl((u_long) request->target.uid);
  memcpy(net_req.data+228, &i, sizeof(i));

  i = htonl((u_long) request->target.instance);
  memcpy(net_req.data+232, &i, sizeof(i));

  strncpy(net_req.data+236, request->target.username,10);
  strncpy(net_req.data+246, request->target.realname,32);
#ifdef HAVE_KRB4
  strncpy(net_req.data+278, request->target.realm,40);
  strncpy(net_req.data+318, request->target.inst,40);
#endif
  strncpy(net_req.data+358, request->target.nickname,16);
  strncpy(net_req.data+374, request->target.title,32);
  strncpy(net_req.data+406, request->target.machine,32);

  if (swrite(fd, (char *) &net_req, sizeof(IO_REQUEST)) != sizeof(IO_REQUEST))
    return(ERROR);

#ifdef HAVE_KRB4

  klength     = htonl((u_long) request->kticket.length);
  if (swrite(fd, (char *) &klength, sizeof(int)) != sizeof(int)) 
    {
      fprintf(stderr, "Error in sending ticket length. \n");
      return(ERROR);
    }

  if (swrite(fd, (char *)request->kticket.dat,
	    sizeof(unsigned char)*request->kticket.length) 
      != sizeof(unsigned char)*request->kticket.length) 
    {
      fprintf(stderr, "Error in sending ticket. \n");
          return(ERROR);
    }
#else /* not HAVE_KRB4 */

  klength = htonl((u_long) 0);
  if (swrite(fd, &klength, sizeof(int)) != sizeof(int)) 
    {
      fprintf(stderr, "Error telling server we don't use kerberos.. \n");
      return(ERROR);
    }

#endif /* not HAVE_KRB4 */

  return(SUCCESS);
}

ERRCODE
read_list(fd, list)
     int fd;
     LIST *list;
{
  int size;
  int len;
  IO_LIST net_req;

  memset(list, 0, sizeof(LIST));

  memset(&net_req, 0, sizeof(IO_LIST));

  len = 0;
  while (len < sizeof(IO_LIST)) {
    size = sread(fd, (char *) (&net_req+len), sizeof(IO_LIST));

    if(size == -1)        /* let's not hold up the server if we can't  */
      {                   /* handle it. */
	send_response(fd,SUCCESS);
	return(FATAL);
      }
    len += size;
  }
    
  send_response(fd,SUCCESS);


  list->ustatus	 = (ntohl(*((u_long *) net_req.data)));
  list->cstatus  = (ntohl(*((u_long *) (net_req.data+4))));
  list->ukstatus = (ntohl(*((u_long *) (net_req.data+8))));
  list->ckstatus = (ntohl(*((u_long *) (net_req.data+12))));
  list->utime    = (ntohl(*((u_long *) (net_req.data+16))));
  list->ctime    = (ntohl(*((u_long *) (net_req.data+20))));
  list->umessage = (ntohl(*((u_long *) (net_req.data+24))));
  list->cmessage = (ntohl(*((u_long *) (net_req.data+28))));
  list->nseen	 = (ntohl(*((u_long *) (net_req.data+32))));

  strncpy(list->topic,(char *)net_req.data+36, TOPIC_SIZE);
  strncpy(list->note, (char *)net_req.data+60, NOTE_SIZE);

  list->user.uid = (ntohl(*((u_long *) (net_req.data+124))));
  list->user.instance = (ntohl(*((u_long *) (net_req.data+128))));
  
  strncpy(list->user.username, (char *)net_req.data+132, LOGIN_SIZE+1);
  strncpy(list->user.realname, (char *)net_req.data+142, TITLE_SIZE);
#ifdef HAVE_KRB4
  strncpy(list->user.realm,    (char *)net_req.data+174, REALM_SZ);
  strncpy(list->user.inst,     (char *)net_req.data+214, INST_SZ);
#endif
  strncpy(list->user.nickname, (char *)net_req.data+254, STRING_SIZE);
  strncpy(list->user.title,    (char *)net_req.data+270, TITLE_SIZE);
  strncpy(list->user.machine,  (char *)net_req.data+302, TITLE_SIZE);

  list->connected.uid = (ntohl(*((u_long *) (net_req.data+336))));
  list->connected.instance = (ntohl(*((u_long *) (net_req.data+340))));
  
  strncpy(list->connected.username, (char *)net_req.data+344, LOGIN_SIZE+1);
  strncpy(list->connected.realname, (char *)net_req.data+354, TITLE_SIZE);
#ifdef HAVE_KRB4
  strncpy(list->connected.realm,    (char *)net_req.data+386, REALM_SZ);
  strncpy(list->connected.inst,     (char *)net_req.data+426, INST_SZ);
#endif
  strncpy(list->connected.nickname, (char *)net_req.data+466, STRING_SIZE);
  strncpy(list->connected.title,    (char *)net_req.data+482, TITLE_SIZE);
  strncpy(list->connected.machine,  (char *)net_req.data+514, TITLE_SIZE);

  return(SUCCESS);
}


/*
 * Function:	open_connection_to_daemon() opens a socket connected to the
 *			default OLC daemon and provides Kerberos
 *			authentication.
 *
 * Arguments:	request: a pointer to a request structure to get Kerberos
 *			tickets out of.
 *              fd: a place for the file descriptor to go.
 * Returns:	An error code
 * Notes:
 *		This is just a wrapper around
 *		open_connection_to_named_daemon
 */

ERRCODE
open_connection_to_daemon(request, fd)
     REQUEST *request;
     int *fd;
{
  return(open_connection_to_named_daemon(request, fd, DaemonHost));
}

/*
 * Function:	open_connection_to_named_daemon() opens a socket connected
 *			to the specified OLC daemon and provides Kerberos
 *			authentication.
 *
 * Arguments:	request: a pointer to a request structure to get Kerberos
 *                      tickets out of.
 *              fd: place for the file descriptor to go.
 *              hostname: the host where the daemon lives.
 * Returns:	An error code.
 * Notes:
 *	First, look up the host address and service port number.  Then
 *	set up the network connection, exiting with an ERROR if no
 *	connection can be be made.  If the connection is successful, return
 *	the file descriptor attached to the socket.
 */

ERRCODE
open_connection_to_named_daemon(request, fd, hostname)
     REQUEST *request;
     int *fd;
     char *hostname;
{
  struct hostent *hp = NULL; 
  struct servent *service = NULL; 
  static struct sockaddr_in sin, *sptr = NULL;
  static char cached_hostname[MAXHOSTNAMELEN];
  ERRCODE status;

#ifdef HAVE_KRB4
  status =  krb_mk_req(&(request->kticket), K_SERVICE, INSTANCE, REALM, 0);  
  if(status != SUCCESS)
    return(status);
#endif /* HAVE_KRB4 */

  *fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if ((sptr == (struct sockaddr_in *) NULL) ||
      (strcmp(cached_hostname,hostname) != 0)) {
    char *port_env;
    hp = gethostbyname(DaemonHost);
    if (hp == (struct hostent *)NULL) {
      close(*fd);
      return(ERROR_NAME_RESOLVE);
    }
      
    memset(&sin, 0, sizeof (sin));
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);

    sin.sin_family = AF_INET;

    port_env = (char *) getenv ("OLCD_PORT");
    if (port_env != NULL)
      sin.sin_port = htons (atoi (port_env));
    else {
#ifdef HAVE_HESIOD
      service = hes_getservbyname(client_service_name(), OLC_PROTOCOL);
#endif
      /* Fall back to /etc/services if no hesiod information avail. */
      if (service == NULL) {
	service = getservbyname(client_service_name(), OLC_PROTOCOL);
	if (service == NULL) {
	  close(*fd);
	  return(ERROR_SLOC);
	}
      }
      sin.sin_port = service->s_port;
    }
    sptr = &sin;
    strcpy(cached_hostname,hostname);
  }

  if (connect(*fd, (struct sockaddr *)(&sin), sizeof(sin)) < 0) 
    {
      close(*fd);
      return(ERROR_CONNECT);
    }
  
  return(SUCCESS);
}

ERRCODE
open_connection_to_nl_daemon(fd)
     int *fd;
{
  struct hostent *hp = (struct hostent *)NULL;
  struct servent *service = (struct servent *)NULL;
  static struct sockaddr_in sin;
  static int init = 0;

  if (init == 0) {
    hp = gethostbyname(DaemonHost);
    if (hp == (struct hostent *)NULL) {
      fprintf(stderr,"Couldn't resolve address of %s\n",DaemonHost);
      return(ERROR);
    }

#ifdef HAVE_HESIOD
    service = hes_getservbyname(client_nl_service_name(), OLC_PROTOCOL);
#endif    
    /* Fall back to /etc/services if no hesiod- */
    if (service == NULL) {
      service = getservbyname(client_nl_service_name(), OLC_PROTOCOL);
      if (service == NULL) {
	fprintf(stderr,"ols/tcp unknown service\n");
	return(ERROR);
      }
    }

    memset(&sin, 0, sizeof(sin));
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_family = AF_INET;
    sin.sin_port = service->s_port;
    init = 1;
  }

  *fd = socket(AF_INET, SOCK_STREAM, 0);
  if (*fd == -1) {
    olc_perror("socket");
    return(ERROR);
  }
  
  if (connect(*fd,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    olc_perror("connect");
    close(*fd);
    return(ERROR);
  }
  return(SUCCESS);
}