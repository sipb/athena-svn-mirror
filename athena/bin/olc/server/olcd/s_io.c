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
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/s_io.c,v $
 *	$Id: s_io.c,v 1.16 1990-07-16 10:16:47 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/s_io.c,v 1.16 1990-07-16 10:16:47 vanharen Exp $";
#endif

#include <mit-copyright.h>

#include <olc/lang.h>

#if is_cplusplus
extern "C" {
#endif
#include <sys/types.h>             /* System type declarations. */
#include <sys/socket.h>            /* Network socket defs. */
#include <sys/file.h>              /* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>              /* System time definitions. */
#include <netinet/in.h>
#include <errno.h>                 /* System error numbers. */
#include <netdb.h>
#include <signal.h>
#if is_cplusplus
};
#endif

#include <olc/olc.h>
#include <olcd.h>

/* External Variables. */

extern char DaemonHost[];			/* Name of daemon's machine. */
extern int errno;

static struct hostent *hp = (struct hostent *)NULL; /* daemon host */
static struct servent *service = (struct servent *)NULL; /* service entry */

#define	MIN(a,b)	((a)>(b)?(b):(a))
#define MAX(a,b)        ((a)<(b)?(b):(a))

/*
 * Note: All functions that deal with I/O on sockets in this file use the
 *	functions "sread()" and "swrite()", which check to ensure that the
 *	socket is, in fact, connected to something.
 */



/*
 * Function:	read_request() reads a request from a user program.
 * Arguments:	fd:		File descriptor to read from.
 *		request:	Pointer to request structure to hold data.
 * Returns:	SUCCESS if the read is successful, ERROR otherwise.
 * Notes:
 *	Read the appropriate number of bytes from the file descriptor,
 *	returning SUCCESS if the read succeeds, and ERROR if it does not.
 */

ERRCODE
#ifdef __STDC__
read_request(int fd, REQUEST *request)
#else
read_request(fd, request)
     int fd;
     REQUEST *request;
#endif /* STDC */
{
  static IO_REQUEST io_req;
  char msgbuf[BUF_SIZE];

  if (sread(fd, (char *) &io_req, sizeof(IO_REQUEST)) != sizeof(IO_REQUEST))
    return(ERROR);
  request->requester          = io_req.requester;
  request->target             = io_req.target;
  request->request_type       = ntohl((u_long) io_req.request_type);
  request->options            = ntohl((u_long) io_req.options);
  request->target.uid         = ntohl((u_long) io_req.target.uid);
  request->requester.uid      = ntohl((u_long) io_req.requester.uid);
  request->target.instance    = ntohl((u_long) io_req.target.instance);
  request->requester.instance = ntohl((u_long) io_req.requester.instance);
  request->version            = ntohl((u_long) io_req.version);

#if 0
  printf("%d %d\n",request->requester.uid,request->version);
#endif TEST

  if ((request->version != VERSION_5)  &&
      (request->version != VERSION_4)  &&
      (request->version != VERSION_3))
    {
      sprintf(msgbuf,
	      "Error in version from %s@%s\ncurr ver = %d, ver recvd = %d",
	      io_req.requester.username, io_req.requester.machine,
	      CURRENT_VERSION, request->version);
      log_error(msgbuf);
      return(ERROR);
    }

#ifdef KERBEROS
  if (read(fd, (char *) &(request->kticket.length), sizeof(int)) != 
      sizeof(int)) 
    {
      log_error("error on read: klength failure");
      return(ERROR);
    }
  
  request->kticket.length  = ntohl((u_long) request->kticket.length);

#if 0
  printf("klength: %d\n",request->kticket.length);
#endif TEST

  if (read(fd, (char *) request->kticket.dat,
	   MIN(sizeof(unsigned char)*request->kticket.length,
	   sizeof(request->kticket.dat))) != 
      sizeof(unsigned char)*request->kticket.length) 
    {
      log_error("error on read: kdata failure");
      return(ERROR);
    }

#endif KERBEROS

return(SUCCESS);  
}



int
#ifdef __STDC__
send_list(int fd, REQUEST *request, LIST *list)
#else
send_list(fd, request, list)
     int fd;
     REQUEST *request;
     LIST *list;
#endif /* STDC */
{
  LIST list_rq;
  OLDLIST frep;
  int response;

  if(request->version >= VERSION_4)
    {
      list_rq = *list;
      list_rq.nseen              = htonl((u_long) list->nseen);
      list_rq.umessage           = htonl((u_long) list->umessage);
      list_rq.cmessage           = htonl((u_long) list->cmessage);
      list_rq.utime              = htonl((u_long) list->utime);
      list_rq.ctime              = htonl((u_long) list->ctime);
      list_rq.ustatus            = htonl((u_long) list->ustatus);
      list_rq.cstatus            = htonl((u_long) list->cstatus);
      list_rq.ukstatus           = htonl((u_long) list->ukstatus);
      list_rq.ckstatus           = htonl((u_long) list->ckstatus);
      list_rq.user.instance      = htonl((u_long) list->user.instance);
      list_rq.user.uid           = htonl((u_long) list->user.uid);
      list_rq.connected.instance = htonl((u_long) list->connected.instance);
      list_rq.connected.uid      = htonl((u_long) list->connected.uid);
      
      if (swrite(fd, (char *) &list_rq, sizeof(LIST)) != sizeof(LIST))
	{
	  perror("error in size");
	}
    }
  else
    {
      frep.user = list->user;
      frep.connected = list->connected;
      strncpy(frep.topic, list->topic, TOPIC_SIZE);
      list->topic[TOPIC_SIZE-1] = 0;
      strncpy(frep.note, list->note, NOTE_SIZE);
      list->note[NOTE_SIZE-1] = 0;
      frep.nseen              = htonl((u_long) list->nseen);
      frep.ustatus            = htonl((u_long) list->ustatus);
      frep.cstatus            = htonl((u_long) list->cstatus);
      frep.ukstatus           = htonl((u_long) list->ukstatus);
      frep.ckstatus           = htonl((u_long) list->ckstatus);
      frep.user.instance      = htonl((u_long) list->user.instance);
      frep.user.uid           = htonl((u_long) list->user.uid);
      frep.connected.instance = htonl((u_long) list->connected.instance);
      frep.connected.uid      = htonl((u_long) list->connected.uid);
      
      if (swrite(fd, (char *) &frep, sizeof(OLDLIST)) != sizeof(OLDLIST))
	{
	  perror("error in size");
	}
    }

  read_response(fd,&response);
  return(response);
}
  

ERRCODE
#ifdef __STDC__
send_person(int fd, PERSON *person)
#else
send_person(fd, person)
     int fd;
     PERSON *person;
#endif /* STDC */
{
  PERSON person_rq;

  person_rq = *person;
  person_rq.instance =  htonl((u_long) person->instance);
  person_rq.uid      =  htonl((u_long) person->uid);

  if (swrite(fd, (char *) &person_rq, sizeof(PERSON)) != sizeof(PERSON))
    return(ERROR);

  else
    return(SUCCESS);
}

