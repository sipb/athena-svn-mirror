/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/message_daemon.c,v $
 * $Author: probe $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_message_daemon_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/message_daemon.c,v 1.6 1991-07-06 15:35:57 probe Exp $";
#endif lint

#include "globalmessage.h"
#ifndef GMS_SERVER_MESSAGE
#define GMS_SERVER_MESSAGE "/site/Message"
#endif /* GMS_SERVER_MESSAGE */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

/*
 * This version of the daemon is run out of inetd, with a conf line of:
 * globalmessage dgram udp wait unswitched nobody /etc/messaged messaged
 */
#include <syslog.h>
char *error_message();

#define saddr_list(sin) (unsigned)sin[0],(unsigned)sin[1],(unsigned)sin[2],(unsigned)sin[3]
main(argc,argv)
     int argc;
     char **argv;
{
  char buf[BFSZ];
  int readstat, errstat, readlen, msgfile;
  char *index();
  struct sockaddr from;
  /* these strange casts are so that I can print out the address of
   * the incoming packet.
   */
  struct sockaddr_in *cast1 = (struct sockaddr_in *)&from;
  char *cast2 = (char *)(&(cast1->sin_addr.s_addr));
  int fromlen = sizeof(from);
  time_t timestamp;
  int headerlen;

  /* initialize syslog connection for DAEMON */
  /* log_pid has the side effect of forcing log messages to be unique,
   * not listed as ``message the same, repeated 6 times'' but that
   * shouldn't be a problem.
   */
#ifdef LOG_DAEMON
  openlog(argv[0], LOG_PID, LOG_DAEMON);
#else
  openlog(argv[0], LOG_PID);
#endif
  
  /* gms is just return values; the other com_err tables used will
   * init themselves.
   */
  init_gms_err_tbl();

  /* read the packet from the socket (stdin, since we run from inetd)
   * and also record the from address so we can send a reply.
   */
  readstat = recvfrom(0, buf, BFSZ, 0, &from, &fromlen);
  if(readstat == -1) {
    syslog(LOG_INFO, "GMS daemon recvfrom failure %s", error_message(errno));
    exit(errno);
  }

  /* We got a packet successfully, so if logging is enabled record the
   * requesting address.
   */
  if(argc>1 && !strcmp(argv[1],"-log")) {
    syslog(LOG_INFO, "GMS request succeeded [%s] from %ud.%ud.%ud.%ud",
	   buf, saddr_list(cast2));
  }

  /* Check the version number, and log if it is in error. */
  if(strncmp(buf, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN)) {
    syslog(LOG_INFO, "GMS bogus version [%s] from %ud.%ud.%ud.%ud",
	   buf, saddr_list(cast2));
    exit(GMS_CLIENT_VERSION);
  } 

  /* Open the message file to read it in. Note that even were we to
   * want to cache the data for efficiency, we must stat the file to
   * make sure it hasn't changed under us. Since we're running, we
   * must open it because we have no state.
   */
  msgfile = open(GMS_SERVER_MESSAGE, O_RDONLY, 0);

  if(msgfile == -1) {
    /* no file, special case of 0 timestamp indicating to also delete
     * the remote cached copy. If the error is ENOENT, then we just
     * assume it is an expected disappearance; all other errors are
     * `interesting' and are logged. In any case, the tester can see
     * that there is no new message.
     */
    if(errno == ENOENT) {
      /* it didn't exist, we can deal with that... */
      timestamp = 0;
    } else {
      /* something went wrong, but we can't do much about it;
       * let's not trash the remote state -- just don't answer the
       * connection.
       */
      syslog(LOG_INFO, "GMS daemon open error [%s] reading message file <%s>",
	     error_message(errno), GMS_SERVER_MESSAGE);
      exit(errno);
    }
  } else {
    struct stat sbuf;
    
    fstat(msgfile, &sbuf);
    if(sbuf.st_size == 0) {
      /* an empty file means the same to the remote as a missing one. */
      timestamp = 0;
    } else {
      /* for convenient maintenance, use the real timestamp as version
       * number. */
      timestamp = sbuf.st_mtime;
    }
  }
  strcpy(buf,GMS_VERSION_STRING);
  {
    char tsbuf[GMS_TIMESTAMP_LEN];

    /* sprintf (and _doprnt in general) are slow memory hogs. However,
     * syslog is already including _doprnt, so it's already over; it
     * is cheaper just to use sprintf here instead of coding or
     * linking in the fast integer one.
     * It is almost readable, too...
     */
    sprintf(tsbuf, " %lu\n", timestamp);
    strcat(buf,tsbuf);
    headerlen = strlen(buf);
  }

  /* Now that we have the header, we have to read the rest of the file
   * into the packet after it. The length-1 specification is to
   * preserve a NUL at the end of the buffer. Use msgfile to remember
   * that we couldn't open it.
   */
  if(msgfile != -1) {
    readlen = read(msgfile, buf + headerlen, BFSZ-headerlen-1);
    if(readlen == -1) {
      /* read failed but open didn't, so record the error */
      syslog(LOG_INFO, "GMS daemon read error [%s] reading message file <%s>",
	     error_message(errno), GMS_SERVER_MESSAGE);
      exit(errno);
    }
    /* protect the fencepost... */
    buf[BFSZ-1] = '\0';
    close(msgfile);
  }
  /* send the packet back where it came from */
  if(-1 == sendto(0, buf, readlen+headerlen, 0, &from, sizeof(from))) {
    /* the send failed (probably a local error, or bad address or
     * something so log the attempt.
     */
    syslog(LOG_INFO, "GMS daemon sendto failure [%s] to %d.%d.%d.%d",
	   error_message(errno), saddr_list(cast2));
    exit(errno);
  }
  /* everything worked, die happy... */
  exit(0);
}
