/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the Time portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/time_grp.c,v $
 *    $Author: ghudson $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *    Revision 2.0  1992/04/22 01:59:48  tom
 *    *** empty log message ***
 *
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/time_grp.c,v 2.1 1997-02-27 06:47:56 ghudson Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef TIMED

#include <protocols/timed.h>

static int timed_sock = -1;
static char *timed_req();


/*
 * Function:    lu_timed()
 * Description: Top level callback for timed. 
 */
 
int
lu_timed(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */

  switch(varnode->offset)
    {
    case N_TIMEDMASTER:
      return(make_str(&(repl->val), timed_req(TSP_MSITE)));
    default:
      syslog (LOG_ERR, "lu_timed: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}





static char *
timed_req(req)
     int req;
{
  int length;
  int cc;
  fd_set ready;
  struct sockaddr_in dest;
  struct timeval tout;
  struct sockaddr_in from;
  static struct tsp msg;
  struct servent *srvp = (struct servent *) NULL;
  struct hostent *hp = (struct hostent *) NULL;
  char hostname[MAXHOSTNAMELEN];

#ifdef ATHENA
  srvp = getservbyname("athena-timed", "udp");
  if (srvp == (struct servent *) NULL) 
    syslog(LOG_WARNING, "udp/athena-timed: unknown service.");    
#endif  ATHENA

  if(srvp == (struct servent *) NULL)
    srvp = getservbyname("timed", "udp");
  if (srvp == (struct servent *) NULL) 
    {
      syslog(LOG_ERR, "udp/timed: unknown service.");
      return((char *) NULL);
    }

  dest.sin_port = srvp->s_port;
  dest.sin_family = AF_INET;

  (void) gethostname(hostname, sizeof(hostname));
  if((hp = gethostbyname(hostname)) == (struct hostent *) NULL)
    {
      syslog(LOG_ERR, "unable to gethostname of %s", hostname);
      return((char *) NULL);
    }
  memcpy(&dest.sin_addr.s_addr, hp->h_addr, hp->h_length);

  (void)strcpy(msg.tsp_name, hostname);
  msg.tsp_type = req;
  msg.tsp_vers = TSPVERSION;
  length = sizeof(struct sockaddr_in);

  if (sendto(timed_sock, (char *)&msg, sizeof(struct tsp), 0, &dest, 
	     length) < 0) 
    {
      syslog(LOG_ERR, "unable to send timed packet");
      return((char *) NULL);
    }

  /*
   * snmpd should not do this!
   */

  tout.tv_sec = 2;
  tout.tv_usec = 0;
  FD_ZERO(&ready);
  FD_SET(timed_sock, &ready);
  
  if(select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout)) 
    {
      length = sizeof(struct sockaddr_in);
      if((cc = recvfrom(timed_sock, (char *)&msg, sizeof(struct tsp), 0, &from,
			&length)) < 0);
	
      if(msg.tsp_type == TSP_ACK)
	return(msg.tsp_name);
      else
	return((char *) NULL);
    }
  return((char *) NULL);
}





int
timed_init_socket()
{
  int port;
  struct sockaddr_in sin;

  if((timed_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      syslog(LOG_ERR, "unable to open timed socket");
      return(-1);
    }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

  for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) 
    {
      sin.sin_port = htons((u_short)port);
      if (bind(timed_sock, (struct sockaddr *) &sin, sizeof(sin)) >= 0)
	break;
      if (errno != EADDRINUSE && errno != EADDRNOTAVAIL) 
	{
	  syslog(LOG_ERR, "unable to bind timed socket");
	  (void) close(timed_sock);
	  return(-1);
	}
    }
     
  if (port == IPPORT_RESERVED / 2) 
    {
      syslog(LOG_ERR, "all reserved ports in use\n");
      (void) close(timed_sock);
      return(-1);
    }

  return(0);
}

#endif TIMED
#endif MIT
