/*

  
  					W3C Sample Code Library libwww Socket Open and Close


!
  Socket Open and Close
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

The Socket Open and Close methods knows how to establish a BSD socket TCP
connection and is part of the Transport interface.

This module is implemented by HTTCP.c, and it is a
part of the W3C Sample Code
Library.
*/

#ifndef HTTCP_H
#define HTTCP_H
#include "HTReq.h"
#include "HTNet.h"

/*
.
  Active Connection Establishment
.

This makes an active connect to the specified host. An
HTNet Object is parsed in order to handle errors
and other stuff. The default port might be overwritten by any port indication
in the URL specified
as <host>:<port> If it is a multihomed host (a
host having multiple IP-addresses with the same host name) &nbsp;then
HTDoConnect() measures the time it takes to connect and updates
the calculated weights in the DNS object.
*/

extern int HTDoConnect (HTNet * net, char * url, u_short default_port);

/*
.
  Passive Connection Establishment
.

This function makes an accept on a port. The net must contain
a valid socket to accept on. If accept is OK then we duplicate the
net object and assign the accepted socket to the newly created net object.
The original Net object will keep accepting connections
on the original socket, for example port 80 in the case of
HTTP. The newly created Net object will be freed
when the protocol module has finished. If the
accepted net object pointer points to the net object itself, that
is - the same object all along - then we reuse the same Net obejct; closes
the original socket and replaces it wik the accepted one. This is a quick
way of accepting a single connection.
*/
extern int HTDoAccept (HTNet * net, HTNet ** accepted);

/*
.
  Listen on a Socket
.

Listens on the specified port. Passing port 0 means that we
don't care and a temporary one will be assigned. If
master==INVSOC then we listen on all local interfaces (using
a wildcard). If INVSOC is not NULL then
use this as the local interface. backlog is the number of
connections that can be queued on the socket - you can use
HT_BACKLOG for a platform dependent value (typically 5 on BSD
and 32 on SVR4). Returns HT_ERROR or HT_OK.
*/
extern int HTDoListen (HTNet * net, u_short port, SOCKET master, int backlog);

/*
.
  Closing a socket
.

Closes a socket
*/

extern int HTDoClose (HTNet * net);

/*
*/

#endif   /* HTTCP_H */

/*

  

  @(#) $Id: HTTCP.h,v 1.1.1.1 2000-03-10 17:53:01 ghudson Exp $

*/
