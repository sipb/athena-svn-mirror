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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/s_io.c,v $
 *	$Id: s_io.c,v 1.22 1991-04-08 21:13:07 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/s_io.c,v 1.22 1991-04-08 21:13:07 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/types.h>             /* System type declarations. */
#include <sys/socket.h>            /* Network socket defs. */
#include <sys/file.h>              /* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>              /* System time definitions. */
#include <netinet/in.h>
#include <errno.h>                 /* System error numbers. */
#include <netdb.h>
#include <signal.h>

#include <olcd.h>

/* External Variables. */

extern char DaemonHost[];			/* Name of daemon's machine. */
extern int errno;

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
read_request(fd, request)
     int fd;
     REQUEST *request;
{
  IO_REQUEST net_req;
  char msgbuf[BUF_SIZE];
  int len,size;

  len = 0;
  while (len < sizeof(IO_REQUEST)) {
    size = sread(fd, (char *) &net_req, sizeof(IO_REQUEST)-len);
    if (size == -1) {
      log_error("read_request: error reading io_request: %s");
      return(ERROR);
    }
    len += size;
  }

  /* build up struct from buffer sent over */

  request->version = (ntohl(*((u_long *) net_req.data)));
  request->request_type = (ntohl(*((u_long *) (net_req.data+4))));

  request->options = (ntohl(*((u_long *) (net_req.data+8))));

/* options unset */

  request->requester.uid = (ntohl(*((u_long *) (net_req.data+16))));
  request->requester.instance = (ntohl(*((u_long *) (net_req.data+20))));
  
  strncpy(request->requester.username, (char *) (net_req.data+24), 10);
  strncpy(request->requester.realname, (char *) (net_req.data+34), 32);
#ifdef KERBEROS
  strncpy(request->requester.realm, (char *) (net_req.data+66), 40);
  strncpy(request->requester.inst, (char *) (net_req.data+106), 40);
#endif
  strncpy(request->requester.nickname, (char *) (net_req.data+146), 16);
  strncpy(request->requester.title, (char *) (net_req.data+162), 32);
  strncpy(request->requester.machine, (char *) (net_req.data+194), 32);

  request->target.uid = (ntohl(*((u_long *) (net_req.data+227))));

  request->target.instance = (ntohl(*((u_long *) (net_req.data+232))));

  strncpy(request->target.username, (char *) (net_req.data+236), 10);
  strncpy(request->target.realname, (char *) (net_req.data+246), 32);
#ifdef KERBEROS
  strncpy(request->target.realm, (char *) (net_req.data+278), 40);
  strncpy(request->target.inst, (char *) (net_req.data+318), 40);
#endif
  strncpy(request->target.nickname, (char *) (net_req.data+358), 16);
  strncpy(request->target.title, (char *) (net_req.data+374), 32);
  strncpy(request->target.machine, (char *) (net_req.data+406), 32);

  if ((request->version != VERSION_5)  &&
      (request->version != VERSION_4))
    {
      sprintf(msgbuf,
	      "Error in version from %s@%s\ncurr ver = %d, ver recvd = %d",
	      request->requester.username, request->requester.machine,
	      CURRENT_VERSION, request->version);
      log_error(msgbuf);
      return(ERROR);
    }

/* Always read ticket data; may just be ignored if not using kerberos */
  
  request->kticket.length  = ntohl((u_long) request->kticket.length);
  
  if (request->kticket.length != 0) {
    if (sread(fd, (char *) request->kticket.dat,request->kticket.length) !=
	request->kticket.length) {
      log_error("error on read: kdata failure: %s");
      return(ERROR);
    }
  }
  return(SUCCESS);  
}



ERRCODE
send_list(fd, request, list)
     int fd;
     REQUEST *request;
     LIST *list;
{
  IO_LIST net_req;
  int response;
  int len,size;
  long i;

  bzero((char *) &net_req, sizeof(IO_LIST));

  i = htonl((u_long) list->ustatus);
  bcopy((char *) &i, (char *) net_req.data, sizeof(i));

  i = htonl((u_long) list->cstatus);
  bcopy((char *) &i, (char *) (net_req.data+4), sizeof(i));

  i = htonl((u_long) list->ukstatus);
  bcopy((char *) &i, (char *) (net_req.data+8), sizeof(i));

  i = htonl((u_long) list->ckstatus);
  bcopy((char *) &i, (char *) (net_req.data+12), sizeof(i));

  i = htonl((u_long) list->utime);
  bcopy((char *) &i, (char *) (net_req.data+16), sizeof(i));

  i = htonl((u_long) list->ctime);
  bcopy((char *) &i, (char *) (net_req.data+20), sizeof(i));

  i = htonl((u_long) list->umessage);
  bcopy((char *) &i, (char *) (net_req.data+24), sizeof(i));

  i = htonl((u_long) list->cmessage);
  bcopy((char *) &i, (char *) (net_req.data+28), sizeof(i));

  i = htonl((u_long) list->nseen);
  bcopy((char *) &i, (char *) (net_req.data+32), sizeof(i));

  strncpy((char *)(net_req.data+36),list->topic, TOPIC_SIZE);
  strncpy((char *)(net_req.data+60), list->note, NOTE_SIZE);

  i = htonl((u_long) list->user.uid);
  bcopy((char *) &i, (char *) (net_req.data+124), sizeof(i));

  i = htonl((u_long) list->user.instance);
  bcopy((char *) &i, (char *) (net_req.data+128), sizeof(i));
  
  strncpy((char *)(net_req.data+132), list->user.username, LOGIN_SIZE+1);
  strncpy((char *)(net_req.data+142), list->user.realname, TITLE_SIZE);
#ifdef KERBEROS
  strncpy((char *)(net_req.data+174), list->user.realm, REALM_SZ);
  strncpy((char *)(net_req.data+214), list->user.inst, INST_SZ);
#endif
  strncpy((char *)(net_req.data+254), list->user.nickname, STRING_SIZE);
  strncpy((char *)(net_req.data+270), list->user.title, TITLE_SIZE);
  strncpy((char *)(net_req.data+302), list->user.machine, TITLE_SIZE);

  i = htonl((u_long) list->connected.uid);
  bcopy((char *) &i, (char *) (net_req.data+336), sizeof(i));

  i = htonl((u_long) list->connected.instance);
  bcopy((char *) &i, (char *) (net_req.data+340), sizeof(i));
  
  strncpy((char *)(net_req.data+344), list->connected.username, LOGIN_SIZE+1);
  strncpy((char *)(net_req.data+354), list->connected.realname, TITLE_SIZE);
#ifdef KERBEROS
  strncpy((char *)(net_req.data+386), list->connected.realm, REALM_SZ);
  strncpy((char *)(net_req.data+426), list->connected.inst, INST_SZ);
#endif
  strncpy((char *)(net_req.data+466), list->connected.nickname, STRING_SIZE);
  strncpy((char *)(net_req.data+482), list->connected.title, TITLE_SIZE);
  strncpy((char *)(net_req.data+514), list->connected.machine, TITLE_SIZE);
  
  len = 0;
  while (len < sizeof(IO_LIST)) {
    size = swrite(fd, (char *) &net_req, sizeof(IO_LIST));
    if (size == -1) {
      log_error("read_list: error writing io_list: %s");
      return(ERROR);
    }
    len += size;
  }

  read_response(fd,&response);
  return(response);
}
