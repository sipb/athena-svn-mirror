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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/io.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/io.c,v 1.4 1989-08-15 13:38:07 tjcoppet Exp $";
#endif

#include <olc/olc.h>        

#include <sys/types.h>             /* System type declarations. */
#include <sys/socket.h>            /* Network socket defs. */
#include <sys/file.h>              /* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>              /* System time definitions. */
#include <netinet/in.h>
#include <errno.h>                 /* System error numbers. */
#include <netdb.h>
#include <signal.h>




/* External Variables. */

extern char DaemonHost[];			/* Name of daemon's machine. */
extern int errno;

struct hostent *gethostbyname(); /* Get host entry of a host. */

#define	MIN(a,b)	((a)>(b)?(b):(a))

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
  IO_REQUEST net_rq;
  int status;

#ifdef KERBEROS
  int klength;
#endif KERBEROS

#ifdef KERBEROS
  status =  krb_mk_req(&(request->kticket), K_SERVICE, INSTANCE, REALM, 0);  
  if(status)
    return(status);
#endif KERBEROS

#ifdef TEST
  printf("%d %d\n",request->requester.uid,CURRENT_VERSION);
#endif TEST

  net_rq.version            = (int) htonl((u_long) CURRENT_VERSION);
  net_rq.requester          = request->requester;
  net_rq.target             = request->target;
  net_rq.request_type       = (int) htonl((u_long) request->request_type);
  net_rq.options            = (int) htonl((u_long) request->options);
  net_rq.target.uid         = (int) htonl((u_long) request->target.uid);
  net_rq.requester.uid      = (int) htonl((u_long) request->requester.uid);
  net_rq.target.instance    = (int) htonl((u_long) request->target.instance);
  net_rq.requester.instance = (int) htonl((u_long) request->requester.instance);

  if (swrite(fd, &net_rq, sizeof(IO_REQUEST)) != sizeof(IO_REQUEST))
    return(ERROR);

#ifdef KERBEROS

#ifdef TEST
  printf("klength: %d\n",request->kticket.length);
#endif TEST

  klength     = htonl((u_long) request->kticket.length);
  if (write(fd, &klength, sizeof(int)) != sizeof(int)) 
    {
      fprintf(stderr, "Error in sending ticket length. \n");
      return(ERROR);
    }

  if (write(fd, request->kticket.dat,
	    sizeof(unsigned char)*request->kticket.length) 
      != sizeof(unsigned char)*request->kticket.length) 
    {
      fprintf(stderr, "Error in sending ticket. \n");
          return(ERROR);
    }
#endif KERBEROS

  return(SUCCESS);
}



read_list(fd, list)
     int fd;
     LIST *list;
{
  int size;

  size = sread(fd, list, sizeof(LIST));

  if(size == -1)        /* let's not hold up the server if we can't  */
    {                   /* handle it. */
      send_response(fd,SUCCESS);
      return(FATAL);
    }

  if(size != sizeof(LIST))
    {
      send_response(fd,ERROR);
      return(ERROR);
    }
    
  send_response(fd,SUCCESS);

  list->nseen              = ntohl((u_long) list->nseen);
  list->ustatus            = ntohl((u_long) list->ustatus);
  list->cstatus            = ntohl((u_long) list->cstatus);
  list->ukstatus           = ntohl((u_long) list->ukstatus);
  list->ckstatus           = ntohl((u_long) list->ckstatus);
  list->user.instance      = ntohl((u_long) list->user.instance);
  list->user.uid           = ntohl((u_long) list->user.uid);
  list->connected.instance = ntohl((u_long) list->connected.instance);
  list->connected.uid      = ntohl((u_long) list->connected.uid);



#ifdef TEST
  printf("%s %s %s\n",list->user.username,list->user.realname,list->user.machine);
  printf("%s %s %s\n",list->connected.username,list->connected.realname,list->connected.machine);
  printf("%d %d %d %d",list->nseen,list->ukstatus,list->user.instance,list->user.uid);
#endif TEST
  return(SUCCESS);
}



read_person(fd, person)
     int fd;
     PERSON *person;
{
  if (read(fd, person, sizeof(PERSON)) != sizeof(PERSON))
    return(ERROR);

  person->instance =  ntohl((u_long) person->instance);
  person->uid      =  ntohl((u_long) person->uid);
  return(SUCCESS);
}


/*
 * Function:	open_connection_to_daemon() opens a socket connected to the
 *			OLC daemon and provides Kerberos authentication.
 * Arguments:	None.
 * Returns:	A file descriptor bound to a socket connected to the daemon
 *		if successful.  If an error occurs, we exit.
 * Notes:
 *	First, look up the host address and service port number.  Then
 *	set up the network connection, exiting with an ERROR if no
 *	connection can be be made.  If the connection is successful, return
 *	the file descriptor attached to the socket.
 */

int
open_connection_to_daemon()
{
  int fd;
  struct hostent *hp = (struct hostent *)NULL; 
  struct servent *service = (struct servent *)NULL; 
  static struct sockaddr_in sin, *sptr = (struct sockaddr_in *) NULL;
  

  fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sptr == (struct sockaddr_in *) NULL)
    {
      hp = gethostbyname(DaemonHost);
      if (hp == (struct hostent *)NULL) 
	{
	  printf("Unable to resolve name of OLC daemon host.  ");
	  printf("Please try again later.\n");
	  exit(ERROR);
	}
      
      service = getservbyname(OLC_SERVICE, OLC_PROTOCOL);
      if (service == (struct servent *)NULL)
	{
	  printf("Unable to locate \"OLC\" service.  Please try");
	  printf(" again later.\n");
	  exit(ERROR);
	}
  
      bzero(&sin, sizeof (sin));
      bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
	
      sin.sin_family = AF_INET;
      sin.sin_port = service->s_port;
      sptr = &sin;
    }

  if (connect(fd, &sin, sizeof(sin)) < 0) 
    {
      printf("Unable to connect to OLC daemon.  Please try");
      printf(" again later.\n");
      exit(ERROR);
    }
  
  return(fd);
}




