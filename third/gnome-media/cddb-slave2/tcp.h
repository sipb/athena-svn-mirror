/* GNet - Networking library
 * Copyright (C) 2000  David Helder
 * Copyright (C) 2000  Andrew Lanoix
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */


#ifndef _GNET_TCP_H
#define _GNET_TCP_H

#include "inetaddr.h"

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*

  All fields in GTcpSocket are private and should be accessed only by
  using the functions below.

 */
typedef struct _GTcpSocket GTcpSocket;


/* **************************************** */
/* Asyncronous stuff 	 */


/**
 *   GTcpSocketConnectAsyncID:
 * 
 *   ID of an asynchronous connection started with
 *   gnet_tcp_socket_connect_async().  The connection can be canceled
 *   by calling gnet_tcp_socket_connect_async_cancel() with the ID.
 *
 **/
typedef gpointer GTcpSocketConnectAsyncID;



/**
 *   GTcpSocketConnectAsyncStatus:
 * 
 *   Status for connecting via gnet_tcp_socket_connect_async(), passed
 *   by GTcpSocketConnectAsyncFunc.  More errors may be added in the
 *   future, so it's best to compare against
 *   %GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK.
 *
 **/
typedef enum {
  GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK,
  GTCP_SOCKET_CONNECT_ASYNC_STATUS_INETADDR_ERROR,
  GTCP_SOCKET_CONNECT_ASYNC_STATUS_TCP_ERROR
} GTcpSocketConnectAsyncStatus;



/**
 *   GTcpSocketConnectAsyncFunc:
 *   @socket: TcpSocket that was connecting
 *   @ia: InetAddr of the TcpSocket
 *   @status: Status of the connection
 *   @data: User data
 *   
 *   Callback for gnet_tcp_socket_connect_async().
 *
 **/
typedef void (*GTcpSocketConnectAsyncFunc)(GTcpSocket* socket, 
					   GInetAddr* ia,
					   GTcpSocketConnectAsyncStatus status, 
					   gpointer data);


/**
 *   GTcpSocketNewAsyncID:
 * 
 *   ID of an asynchronous tcp socket creation started with
 *   gnet_tcp_socket_new_async().  The creation can be canceled by
 *   calling gnet_tcp_socket_new_async_cancel() with the ID.
 *
 **/
typedef gpointer GTcpSocketNewAsyncID;



/**
 *   GTcpSocketAsyncStatus:
 * 
 *   Status for connecting via gnet_tcp_socket_new_async(), passed by
 *   GTcpSocketNewAsyncFunc.  More errors may be added in the future,
 *   so it's best to compare against %GTCP_SOCKET_NEW_ASYNC_STATUS_OK.
 *
 **/
typedef enum {
  GTCP_SOCKET_NEW_ASYNC_STATUS_OK,
  GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR
} GTcpSocketNewAsyncStatus;



/**
 *   GTcpSocketNewAsyncFunc:
 *   @socket: Socket that was connecting
 *   @status: Status of the connection
 *   @data: User data
 *   
 *   Callback for gnet_tcp_socket_new_async().
 *
 **/
typedef void (*GTcpSocketNewAsyncFunc)(GTcpSocket* socket, 
				       GTcpSocketNewAsyncStatus status, 
				       gpointer data);




/* ********** */


/* Quick and easy blocking constructor */
GTcpSocket* gnet_tcp_socket_connect (const gchar* hostname, gint port);

/* Quick and easy asynchronous constructor */
GTcpSocketConnectAsyncID
gnet_tcp_socket_connect_async (const gchar* hostname, gint port, 
			       GTcpSocketConnectAsyncFunc func, 
			       gpointer data);

/* Cancel quick and easy asynchronous constructor */
void gnet_tcp_socket_connect_async_cancel (GTcpSocketConnectAsyncID id);

/* Blocking constructor */
GTcpSocket* gnet_tcp_socket_new (const GInetAddr* addr);

/* Asynchronous constructor */
GTcpSocketNewAsyncID 
gnet_tcp_socket_new_async (const GInetAddr* addr, 
			   GTcpSocketNewAsyncFunc func,
			   gpointer data);

/* Cancel asynchronous constructor */
void gnet_tcp_socket_new_async_cancel (GTcpSocketNewAsyncID id);

void gnet_tcp_socket_delete (GTcpSocket* s);

void gnet_tcp_socket_ref (GTcpSocket* s);
void gnet_tcp_socket_unref (GTcpSocket* s);


/* ********** */

GIOChannel* gnet_tcp_socket_get_iochannel (GTcpSocket* socket);
GInetAddr*  gnet_tcp_socket_get_inetaddr (const GTcpSocket* socket);
gint        gnet_tcp_socket_get_port (const GTcpSocket* socket);


/* ********** */

typedef enum
{
  GNET_TOS_NONE,
  GNET_TOS_LOWDELAY,
  GNET_TOS_THROUGHPUT,
  GNET_TOS_RELIABILITY,
  GNET_TOS_LOWCOST

} GNetTOS;

void gnet_tcp_socket_set_tos (GTcpSocket* socket, GNetTOS tos);


/* ********** */

GTcpSocket* gnet_tcp_socket_server_new (gint port);
GTcpSocket* gnet_tcp_socket_server_new_interface (const GInetAddr* iface);

GTcpSocket* gnet_tcp_socket_server_accept (const GTcpSocket* socket);
GTcpSocket* gnet_tcp_socket_server_accept_nonblock (const GTcpSocket* socket);


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /* _GNET_TCP_H */
