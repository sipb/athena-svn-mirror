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

#include "gnet-private.h"
#include "socks-private.h"
#include "tcp.h"

/**
 *  gnet_tcp_socket_connect_async:
 *  @hostname: Name of host to connect to
 *  @port: Port to connect to
 *  @func: Callback function
 *  @data: User data passed when callback function is called.
 *
 *  A quick and easy asynchronous #GTcpSocket constructor.  This
 *  connects to the specified address and port and then calls the
 *  callback with the data.  Use this function when you're a client
 *  connecting to a server and you don't want to block or mess with
 *  #GInetAddr's.  It may call the callback before the function
 *  returns.  It will call the callback if there is a failure.
 *
 *  Returns: ID of the connection which can be used with
 *  gnet_tcp_socket_connect_async_cancel() to cancel it; NULL on
 *  failure.
 *
 **/
GTcpSocketConnectAsyncID
gnet_tcp_socket_connect_async (const gchar* hostname, gint port,
			       GTcpSocketConnectAsyncFunc func,
			       gpointer data)
{
  GTcpSocketConnectState* state;
  gpointer id;

  g_return_val_if_fail(hostname != NULL, NULL);
  g_return_val_if_fail(func != NULL, NULL);

  state = g_new0(GTcpSocketConnectState, 1);
  state->func = func;
  state->data = data;

  id = gnet_inetaddr_new_async(hostname, port,
			       gnet_tcp_socket_connect_inetaddr_cb,
			       state);

  /* Note that gnet_inetaddr_new_async can fail immediately and call
     our callback which will delete the state.  The users callback
     would be called in the process. */

  if (id == NULL)
    return NULL;

  state->inetaddr_id = id;

  return state;
}


void
gnet_tcp_socket_connect_inetaddr_cb (GInetAddr* inetaddr,
				     GInetAddrAsyncStatus status,
				     gpointer data)
{
  GTcpSocketConnectState* state = (GTcpSocketConnectState*) data;

  if (status == GINETADDR_ASYNC_STATUS_OK)
    {
      state->ia = inetaddr;

      state->inetaddr_id = NULL;
      state->tcp_id = gnet_tcp_socket_new_async(inetaddr,
						(GTcpSocketNewAsyncFunc)gnet_tcp_socket_connect_tcp_cb,
						state);
      /* Note that this call may delete the state. */
    }
  else
    {
      (*state->func)(NULL, NULL, GTCP_SOCKET_CONNECT_ASYNC_STATUS_INETADDR_ERROR,
		     state->data);
      g_free(state);
    }
}

void
gnet_tcp_socket_connect_tcp_cb(GTcpSocket* socket,
			       GTcpSocketConnectAsyncStatus status,
			       gpointer data)
{
  GTcpSocketConnectState* state = (GTcpSocketConnectState*) data;

  if (status == GTCP_SOCKET_NEW_ASYNC_STATUS_OK)
    (*state->func)(socket, state->ia, GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK, state->data);
  else
    (*state->func)(NULL, NULL, GTCP_SOCKET_CONNECT_ASYNC_STATUS_TCP_ERROR, state->data);

  g_free(state);
}

/**
 *  gnet_tcp_socket_connect_async_cancel:
 *  @id: ID of the connection.
 *
 *  Cancel an asynchronous connection that was started with
 *  gnet_tcp_socket_connect_async().
 *
 */
void
gnet_tcp_socket_connect_async_cancel(GTcpSocketConnectAsyncID id)
{
  GTcpSocketConnectState* state = (GTcpSocketConnectState*) id;

  g_return_if_fail (state != NULL);

  if (state->inetaddr_id)
    {
      gnet_inetaddr_new_async_cancel(state->inetaddr_id);
    }
  else if (state->tcp_id)
    {
      gnet_inetaddr_delete(state->ia);
      gnet_tcp_socket_new_async_cancel(state->tcp_id);
    }

  g_free (state);
}

/**
 *  gnet_tcp_socket_new_async:
 *  @addr: Address to connect to.
 *  @func: Callback function.
 *  @data: User data passed when callback function is called.
 *
 *  Connect to a specifed address asynchronously.  When the connection
 *  is complete or there is an error, it will call the callback.  It
 *  may call the callback before the function returns.  It will call
 *  the callback if there is a failure.
 *
 *  Returns: ID of the connection which can be used with
 *  gnet_tcp_socket_connect_async_cancel() to cancel it; NULL on
 *  failure.
 *
 **/
GTcpSocketNewAsyncID
gnet_tcp_socket_new_async (const GInetAddr* addr,
			   GTcpSocketNewAsyncFunc func,
			   gpointer data)
{
  gint 			sockfd;
  gint 			flags;
  GTcpSocket* 		s;
  GInetAddr* 		socks_addr = NULL;
  const GInetAddr* 	socks_addr_save   = NULL;
  struct sockaddr	sa;
  struct sockaddr_in* 	sa_in;
  GTcpSocketAsyncState* state;

  g_return_val_if_fail(addr != NULL, NULL);
  g_return_val_if_fail(func != NULL, NULL);

  /* Create socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      (func)(NULL, GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR, data);
      return NULL;
    }

  /* Get the flags (should all be 0?) */
  flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1)
    {
      (func)(NULL, GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR, data);
      return NULL;
    }

  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
      (func)(NULL, GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR, data);
      return NULL;
    }

  /* Create our structure */
  s = g_new0(GTcpSocket, 1);
  s->ref_count = 1;
  s->sockfd = sockfd;

  /* If there's a SOCKS server, switch addresses so we connect to
     that. */
  socks_addr = gnet_socks_get_server ();
  if (socks_addr)
    {
      socks_addr_save = addr;
      addr = socks_addr;
    }

  /* Set up address and port for connection */
  memcpy(&sa, &addr->sa, sizeof(sa));
  sa_in = (struct sockaddr_in*) &sa;
  sa_in->sin_family = AF_INET;

  /* Connect (but non-blocking!) */
  if (connect(s->sockfd, &sa, sizeof(s->sa)) < 0)
    {
      if (errno != EINPROGRESS)
	{
	  (func)(NULL, GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR, data);
	  g_free(s);
	  if (socks_addr) gnet_inetaddr_delete (socks_addr);
	  return NULL;
	}
    }

  /* Swap the SOCK server back */
  if (socks_addr_save)
    addr = socks_addr_save;

  /* Save address */
  memcpy(&s->sa, &addr->sa, sizeof(s->sa));
  sa_in = (struct sockaddr_in*) &sa;
  sa_in->sin_family = AF_INET;

  /* Note that if connect returns 0, then we're already connected and
     we could call the call back immediately.  But, it would probably
     make things too complicated for the user if we could call the
     callback before we returned from this function.  */

  /* Wait for the connection */
  state = g_new0(GTcpSocketAsyncState, 1);
  state->socket = s;
  state->func = func;
  state->data = data;
  state->flags = flags;
  state->socks_addr = socks_addr;
  state->connect_watch = g_io_add_watch(GNET_SOCKET_IOCHANNEL_NEW(s->sockfd),
					GNET_ANY_IO_CONDITION,
					gnet_tcp_socket_new_async_cb,
					state);

  return state;
}


gboolean
gnet_tcp_socket_new_async_cb (GIOChannel* iochannel,
			      GIOCondition condition,
			      gpointer data)
{
  GTcpSocketAsyncState* state = (GTcpSocketAsyncState*) data;
  gint error, len;

  /* Remove the watch now in case we don't return immediately */
  g_source_remove (state->connect_watch);

  errno = 0;
  if (!((condition & G_IO_IN) || (condition & G_IO_OUT)))
    goto error;

  len = sizeof(error);

  /* Get the error option */
  if (getsockopt(state->socket->sockfd, SOL_SOCKET, SO_ERROR, (void*) &error, &len) < 0)
    goto error;

  /* Check if there is an error */
  if (error)
    goto error;

  /* Reset the flags */
  if (fcntl(state->socket->sockfd, F_SETFL, state->flags) != 0)
    goto error;

  /* Negotiate with SOCKS? */
  if (state->socks_addr)
    {
      GInetAddr* addr;
      int rv;

      addr = gnet_private_inetaddr_sockaddr_new(state->socket->sa);
      rv = gnet_private_negotiate_socks_server (state->socket, addr);
      gnet_inetaddr_delete (addr);
      if (rv < 0)
	goto error;
    }

  /* Success */
  (*state->func)(state->socket, GTCP_SOCKET_NEW_ASYNC_STATUS_OK, state->data);
  gnet_inetaddr_delete (state->socks_addr);
  g_free(state);
  return FALSE;

  /* Error */
 error:
  (*state->func)(NULL, GTCP_SOCKET_NEW_ASYNC_STATUS_ERROR, state->data);
  gnet_tcp_socket_delete (state->socket);
  gnet_inetaddr_delete (state->socks_addr);
  g_free(state);

  return FALSE;
}


/**
 *  gnet_tcp_socket_new_async_cancel:
 *  @id: ID of the connection.
 *
 *  Cancel an asynchronous connection that was started with
 *  gnet_tcp_socket_new_async().
 *
 **/
void
gnet_tcp_socket_new_async_cancel(GTcpSocketNewAsyncID id)
{
  GTcpSocketAsyncState* state = (GTcpSocketAsyncState*) id;

  g_source_remove(state->connect_watch);
  gnet_tcp_socket_delete(state->socket);
  gnet_inetaddr_delete (state->socks_addr);
  g_free (state);
}

/**
 *  gnet_tcp_socket_delete:
 *  @s: TcpSocket to delete.
 *
 *  Close and delete a #GTcpSocket.
 *
 **/
void
gnet_tcp_socket_delete(GTcpSocket* s)
{
  if (s != NULL)
    gnet_tcp_socket_unref(s);
}



/**
 *  gnet_tcp_socket_ref
 *  @s: GTcpSocket to reference
 *
 *  Increment the reference counter of the GTcpSocket.
 *
 **/
void
gnet_tcp_socket_ref(GTcpSocket* s)
{
  g_return_if_fail(s != NULL);

  ++s->ref_count;
}


/**
 *  gnet_tcp_socket_unref
 *  @s: #GTcpSocket to unreference
 *
 *  Remove a reference from the #GTcpSocket.  When reference count
 *  reaches 0, the socket is deleted.
 *
 **/
void
gnet_tcp_socket_unref(GTcpSocket* s)
{
  g_return_if_fail(s != NULL);

  --s->ref_count;

  if (s->ref_count == 0)
    {
      GNET_CLOSE_SOCKET(s->sockfd);	/* Don't care if this fails... */

      if (s->iochannel)
	g_io_channel_unref(s->iochannel);

      g_free(s);
    }
}



/**
 *  gnet_tcp_socket_get_iochannel:
 *  @socket: GTcpSocket to get GIOChannel from.
 *
 *  Get the #GIOChannel for the #GTcpSocket.
 *
 *  For a client socket, the #GIOChannel represents the data stream.
 *  Use it like you would any other #GIOChannel.
 *
 *  For a server socket however, the #GIOChannel represents incoming
 *  connections.  If you can read from it, there's a connection
 *  waiting.
 *
 *  There is one channel for every socket.  This function refs the
 *  channel before returning it.  You should unref the channel when
 *  you are done with it.  However, you should not close the channel -
 *  this is done when you delete the socket.
 *
 *  Returns: A #GIOChannel; NULL on failure.
 *
 **/
GIOChannel*
gnet_tcp_socket_get_iochannel(GTcpSocket* socket)
{
  g_return_val_if_fail (socket != NULL, NULL);

  if (socket->iochannel == NULL)
    socket->iochannel = GNET_SOCKET_IOCHANNEL_NEW(socket->sockfd);

  g_io_channel_ref (socket->iochannel);

  return socket->iochannel;
}
