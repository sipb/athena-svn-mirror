/* $Id: sock.c,v 1.1.1.1 2001-01-16 15:26:27 ghudson Exp $
 *
 * Handles incoming request callbacks, buffers headers, and passes
 * the request on to the upstream server.
 *
 * Copyright (C) 2000 Eazel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Authors: Robey Pointer <robey@eazel.com>
 *          Mike Fleming <mfleming@eazel.com>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/poll.h>

#include "sock.h"
#include "dnscache.h"
#include "proxy.h"
#include "log.h"
#include "sock-ssl.h"

#define IS_TUNNEL_LEFT_RIGHT(left,right)  ((left)->in_buffer == (right)->out_buffer)
#define IS_TUNNEL_RIGHT_LEFT(left,right)  ((right)->in_buffer == (left)->out_buffer)

/* sockets we have open */
static Socket *socket_chain = NULL;

/* returns 1 on success */
static SockBuf *
sockbuf_new(void)
{
        SockBuf *buf = g_new0 (SockBuf, 1);

        buf->buffer = (char *) g_malloc (SOCKBUF_SIZE);
        buf->size = SOCKBUF_SIZE;
        buf->used = 0;
        return buf;
}

static void
sockbuf_free(SockBuf *buf)
{
        g_return_if_fail (buf != NULL);
        g_return_if_fail (buf->buffer != NULL);

        g_free (buf->buffer);
        buf->size = buf->used = 0;
        buf->buffer = NULL;
        g_free (buf);
}

/* passing 0 for target_size doubles the buffer size */
static void
sockbuf_step_size (SockBuf *buf, int target_size)
{
	int new_size;

	/* find the smallest n such that buf->size * 2^n > buf->used + datalen */
	for (new_size = buf->size * 2 ; new_size < target_size ; new_size *= 2) ;

	buf->buffer = g_realloc (buf->buffer, new_size);
	buf->size = new_size;
}

/* if there's a complete (linefeed-terminated) line in the sockbuf, it's pulled off and returned --
 * the returned string must be g_free()'d later.
 */
static char *
sockbuf_getline(SockBuf *buf)
{
	int i, end;
	char *line;

	for (i = 0; i < buf->used; i++) {
		if ((buf->buffer[i] == '\n') || (buf->buffer[i] == '\r')) {
			/* found a line */
			break;
		}
	}

	if (i >= buf->used) {
		/* no complete line yet */

		if ( buf->used == buf->size ) {
			sockbuf_step_size (buf, 0);
		}
		
		return NULL;
	}

	end = i-1;
	/* push i past the linefeed */
	i++;
	if ((i < buf->used) && (buf->buffer[i-1] == '\r') && (buf->buffer[i] == '\n')) {
		i++;
	}

	line = (char *) g_malloc (end+2);
	if (!line) {
		/* out of memory? */
		return NULL;
	}
	memcpy (line, buf->buffer, end+1);
	line[end+1] = 0;
	memmove (buf->buffer, buf->buffer + i, buf->used - i);
	buf->used -= i;

	return line;
}

static int
sockbuf_get_data (SockBuf *buf, char ** pp_data, size_t *p_len)
{
	if (buf->used) {
		*p_len = buf->used;
		*pp_data = g_malloc (sizeof(char)*buf->used);
		memcpy (*pp_data,  buf->buffer, buf->used);
		buf->used = 0;
	} else {
		*p_len = 0;
		*pp_data = NULL;
	}

	return 1;
}


/* dump as much of a sockbuf as we can, into a socket */
/* returns 0, or an errno if an error occurred */
static int
sockbuf_write(SockBuf *buf, Socket *sock)
{
	int bytes;

	/* shouldn't have been polling for write if there was nothing in the buffer */
	g_assert (buf->used > 0);
	g_assert (sock != NULL);
	g_assert (buf != NULL);

#ifdef HAVE_OPENSSL
	if (sock->flags & SOCKET_FLAG_USE_SSL) {
		bytes = SSL_write (sock->ssl_handle, buf->buffer, buf->used);
	} else {
		bytes = write (sock->fd, buf->buffer, buf->used);
	}
#else
	bytes = write (sock->fd, buf->buffer, buf->used);
#endif
	if (bytes < 0) {
		return errno;
	}

#ifndef NO_DEBUG_MIRRORING
	if (sock->output_mirror) {
		fwrite (buf->buffer, buf->used, 1, sock->output_mirror);
	}
#endif 

	sock->bytes_written += bytes;

	if (bytes != buf->used) {
		memmove (buf->buffer, buf->buffer + bytes, buf->used - bytes);
	}
	buf->used -= bytes;

	return 0;
}

/* read as much as we can into a sockbuf */
/* returns 0, or an errno if an error occurred, or -1 on EOF */
static int
sockbuf_read(SockBuf *buf, Socket *sock)
{
	int bytes;

	/* shouldn't have been polling for read if there was no buffer space */
	g_assert (buf->used < buf->size);
	g_assert (sock != NULL);
	g_assert (buf != NULL);

#ifdef HAVE_OPENSSL
	if (sock->flags & SOCKET_FLAG_USE_SSL) {
		bytes = SSL_read (sock->ssl_handle, buf->buffer + buf->used, buf->size - buf->used);
	} else {
		bytes = read (sock->fd, buf->buffer + buf->used, buf->size - buf->used);
	}
#else
	bytes = read (sock->fd, buf->buffer + buf->used, buf->size - buf->used);
#endif
	if (bytes < 0) {
                if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                        return errno;
                } else {
                        return 0;
                }
	}
        if (bytes == 0) {
                return -1;
        }

#ifndef NO_DEBUG_MIRRORING
	if (sock->input_mirror) {
		fwrite (buf->buffer + buf->used, bytes, 1, sock->input_mirror);
	}
#endif

	sock->bytes_read += bytes;

	buf->used += bytes;
	return 0;
}

static int
sockbuf_append(SockBuf *buf, char *data, int datalen)
{
	if (buf->used + datalen > buf->size) {
		sockbuf_step_size (buf, buf->used + datalen);
	}
	memcpy (buf->buffer + buf->used, data, datalen);
	buf->used += datalen;
	return 1;
}

static int
sockbuf_append_buffer(SockBuf *buf, SockBuf *xbuf)
{
        sockbuf_append (buf, xbuf->buffer, xbuf->used);
        xbuf->used = 0;
        return 1;
}



/* returns 1 on success */
static int
resolve_name(const char *name, struct in_addr *addr)
{
	return (dnscache_lookup (addr, name) == 0);
}




void
socket_destroy(Socket *sock)
{
        Socket *s;

	g_assert (IS_SOCKET (sock));

	sock->magic = 0;

	if (sock->fd >= 0) {
		close (sock->fd);
		sock->fd = -1;
	}

	if (sock->tunnel) {
		/* If tunnelling was only in one direction, then one buffer
		 * needs to be disposed of.  Otherwise, the buffers
		 * will be disposed of by the other socket's socket_destroy
		 */

		if ( ! IS_TUNNEL_LEFT_RIGHT (sock, sock->tunnel)) {
	                sockbuf_free (sock->in_buffer);
		}
		if ( ! IS_TUNNEL_RIGHT_LEFT (sock, sock->tunnel)) {
	                sockbuf_free (sock->out_buffer);
		}
		
		/* disconnect any existing tunnel */
                s = sock->tunnel;
		s->tunnel = NULL;
		sock->tunnel = NULL;

		/* signal close for other socket */

		/* FIXME !!!!  We've been experiencing a strange
		 * bug where a large PUT followed by a No Content 
		 * response resulted in the client reading only one liune
		 * of the response followed by a getting a 
		 * "Remote connection reset by peer".
		 * Placing the shutdown here in this fashion works around
		 * the problem, but is obviously broken and also does
		 * not guarentee that the socket in question
		 * will ever be closed
		 */ 
		if (0 == s->out_buffer->used) {
			shutdown (s->fd, 1);
		} else {
	                socket_close (s);
	        }
	} else {
                sockbuf_free (sock->in_buffer);
                sockbuf_free (sock->out_buffer);
        }

#ifndef NO_DEBUG_MIRRORING
	if (sock->input_mirror) {
		fclose (sock->input_mirror);
	}
	if (sock->output_mirror) {
		fclose (sock->output_mirror);
	}
#endif

        if (socket_chain == sock) {
                socket_chain = sock->next;
        } else {
                for (s = socket_chain; s->next; s = s->next) {
                        if (s->next == sock) {
                                s->next = sock->next;
                                break;
                        }
                }
        }
	g_free (sock);
}

/* create a new socket.  if name is non-NULL, bind to that address */
Socket *
socket_new(const char *name, int port)
{
	Socket *sock;
	int flags, i;
	struct in_addr addr;
	struct sockaddr_in sockaddr;

        sock = g_new0 (Socket, 1);
	sock->magic = SOCKET_MAGIC;
        sock->next = socket_chain;
        socket_chain = sock;

        sock->in_buffer = sockbuf_new ();
        sock->out_buffer = sockbuf_new ();
        if (!sock->in_buffer || !sock->out_buffer) {
		socket_destroy (sock);
		return NULL;
	}

	sock->fd = socket (AF_INET, SOCK_STREAM, 0);
	if (sock->fd < 0) {
		socket_destroy (sock);
		return NULL;
	}

        /* reuse address */

        i = 1;
        if (setsockopt (sock->fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i)) != 0) {
                socket_destroy (sock);
                return NULL;
        }
                        
	/* set it nonblocking */
	flags = fcntl (sock->fd, F_GETFL, 0);
	fcntl (sock->fd, F_SETFL, flags | O_NONBLOCK);

	/* bind, if an address was given */
	memset (&sockaddr, 0, sizeof(struct sockaddr_in));
	memset (&addr, 0, sizeof(struct in_addr));
	sockaddr.sin_family = AF_INET;
	if (name) {
		resolve_name (name, &addr);
		memcpy (&sockaddr.sin_addr.s_addr, &addr, sizeof(struct in_addr));
	}
	sockaddr.sin_port = htons (port);

	if (bind (sock->fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) != 0) {
		/* failed to bind. :( */
		socket_destroy (sock);
		return NULL;
	}

	return sock;
}

#ifndef NO_DEBUG_MIRRORING
void
socket_set_mirrors(Socket *sock, FILE *output_mirror, FILE *input_mirror)
{
	if (sock->output_mirror) {
		fclose (sock->output_mirror);
	}

	sock->output_mirror = output_mirror;

	if (sock->input_mirror) {
		fclose (sock->input_mirror);
	}

	sock->input_mirror = input_mirror;
}
#endif /* DEBUG */

void
socket_close(Socket *sock)
{
        if (sock->flags & SOCKET_FLAG_CLOSING) {
                return;
        }

        sock->flags |= SOCKET_FLAG_CLOSING;

	if (sock->out_buffer->used) {
		/* still waiting for socket to write -- wait before closing */
		sock->time = time (NULL);
		return;
	}

	if (sock->eoffn) {
		(*sock->eoffn) (sock);
	}
	socket_destroy (sock);
}

/* return the next line from a socket (if there is one).
 * you must g_free() it when done.
 */
char *
socket_getline(Socket *sock)
{
	return sockbuf_getline (sock->in_buffer);
}

/* write data to a socket (might be buffered for later if the socket is busy) */
int
socket_write(Socket *sock, char *data, int datalen)
{
	int bytes;

	g_return_val_if_fail(sock != NULL, 0);
	g_return_val_if_fail(!(sock->flags & (SOCKET_FLAG_LISTEN | SOCKET_FLAG_CONNECTING |
					      SOCKET_FLAG_CLOSING)), 0);

	if (sock->out_buffer->used > 0) {
		/* already buffering output to this socket -- just append */
		return sockbuf_append (sock->out_buffer, data, datalen);
	}

	/* not buffering, so try a direct write first */
#ifdef HAVE_OPENSSL
	if (sock->flags & SOCKET_FLAG_USE_SSL) {
		bytes = SSL_write (sock->ssl_handle, data, datalen);
	} else {
		bytes = write (sock->fd, data, datalen);
	}
#else
	bytes = write (sock->fd, data, datalen);
#endif

	if (bytes < 0) {
		return 0;	/* ouch */
	}

#ifndef NO_DEBUG_MIRRORING
	if (sock->output_mirror && bytes > 0) {
		fwrite (data, bytes, 1, sock->output_mirror); 
	}
#endif
	sock->bytes_written += bytes;

	if (bytes == datalen) {
		return 1;
	}

	/* wrote less than we had -- buffer the rest */
	return sockbuf_append (sock->out_buffer, data + bytes, datalen - bytes);
}

int
socket_read (Socket *sock, char ** pp_data, size_t *p_len)
{
	g_return_val_if_fail (NULL != sock, 0);
	g_return_val_if_fail (IS_SOCKET(sock), 0);
	g_return_val_if_fail (NULL != pp_data, 0);
	g_return_val_if_fail (NULL != p_len, 0);


	return sockbuf_get_data (sock->in_buffer, pp_data, p_len) ;
}



static void
socket_finish_connect(Socket *sock)
{
        int err;
        int len;
	int success;

        sock->flags &= ~SOCKET_FLAG_CONNECTING;

	len = sizeof(int);
        if ((getsockopt (sock->fd, SOL_SOCKET, SO_ERROR, &err, &len) != 0) ||
            (err != 0)) {
		success = 0;
		goto out;
        }
#ifdef HAVE_OPENSSL
	if (sock->flags & SOCKET_FLAG_USE_SSL) {
		int flags;

		/* NOTE: ssl negotiation has to be done over a nonblocking
		 * socket.  that's kinda lame.
		 */
		flags = fcntl(sock->fd, F_GETFL, 0);
		fcntl(sock->fd, F_SETFL, flags &~ O_NONBLOCK);

		/* negotiate ssl over this connection */
		sock->ssl_handle = ssl_begin_ssl (sock->fd, &err);
		if (! sock->ssl_handle) {
			success = 0;
			goto out;
		}
	}
#endif
	success = 1;

out:
	if (sock->connectfn) {
		(*sock->connectfn) (sock, success);
		sock->connectfn = NULL;
	}
	return;
}

/* connect to a remote host & port.
 * returns 0 if the DNS failed, 1 if the connection is being attempted
 */
int
socket_connect(Socket *sock, const char *name, int port, SocketConnectFn connectfn)
{
        struct sockaddr_in sockaddr;
        struct in_addr addr;
        int err;

	g_return_val_if_fail (sock != NULL, 0);

	memset (&sockaddr, 0, sizeof(struct sockaddr_in));
	memset (&addr, 0, sizeof(struct in_addr));
	sockaddr.sin_family = AF_INET;
        if (! resolve_name (name, &addr)) {
                return 0;
        }
        memcpy (&sockaddr.sin_addr.s_addr, &addr, sizeof(struct in_addr));
        sockaddr.sin_port = htons (port);

        sock->connectfn = connectfn;

        err = connect (sock->fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));
        if (err == 0) {
                /* instant connect! */
		socket_finish_connect (sock);
                return 1;
        }
        if (errno == EINPROGRESS) {
                sock->flags |= SOCKET_FLAG_CONNECTING;
                return 1;
        }
        (*sock->connectfn) (sock, 0);
        return 1;
}

int
socket_connect_ssl(Socket *sock, const char *name, int port, SocketConnectFn connectfn)
{
	g_return_val_if_fail (sock != NULL, 0);

	sock->flags |= SOCKET_FLAG_USE_SSL;
	return socket_connect (sock, name, port, connectfn);
}

static void
socket_accept(Socket *sock)
{
        int newfd;
        int addrlen;
        struct sockaddr_in sockaddr;
        Socket *newsock;
	char host_ip[20];
	int port;

        addrlen = sizeof(struct sockaddr_in);
        newfd = accept (sock->fd, (struct sockaddr *)&sockaddr, &addrlen);
        if (newfd < 0) {
                return;
        }

	newsock = g_new0 (Socket, 1);
	newsock->magic = SOCKET_MAGIC;

        newsock->fd = newfd;
        newsock->next = socket_chain;
        socket_chain = newsock;
        newsock->in_buffer = sockbuf_new ();
        newsock->out_buffer = sockbuf_new ();
        if (!newsock->in_buffer || !newsock->out_buffer) {
		socket_destroy (newsock);
		return;
	}

        if (sock->acceptfn) {
		strcpy (host_ip, inet_ntoa (*(struct in_addr *)&sockaddr.sin_addr.s_addr));
		port = ntohs (sockaddr.sin_port);
                (*sock->acceptfn) (newsock, sock->data, host_ip, port);
        }
}

static void
socket_eof(Socket *sock)
{
        if (sock->flags & SOCKET_FLAG_CLOSING) {
                return;
        }
        sock->flags |= SOCKET_FLAG_CLOSING;

        if (sock->eoffn) {
                (*sock->eoffn) (sock);
        }
        socket_destroy (sock);
}

static void dispatch_socket_events (
	struct pollfd *poll_fds, 
	Socket **socket_array, 
	unsigned int count_my_fds
) {
	unsigned int i;
	Socket *sock;
	int err;

	for (i = 0; i < count_my_fds; i++) {
		sock = socket_array[i];
		if ( POLLOUT & poll_fds[i].revents ) {					
			if (sock->flags & SOCKET_FLAG_CONNECTING) {
				socket_finish_connect (sock);
				goto out;
			} else {
				sockbuf_write (sock->out_buffer, sock);
				if (!(sock->out_buffer->used) && (sock->flags & SOCKET_FLAG_CLOSING)) {
					/* can finally close this socket */
					sock->flags &= ~SOCKET_FLAG_CLOSING;
					socket_close (sock);
					goto out;
				}
			}
		}

		if ( POLLIN & poll_fds[i].revents ) {
			if (sock->flags & SOCKET_FLAG_LISTEN) {
				socket_accept (sock);
				goto out;
			} else {
				err = sockbuf_read (sock->in_buffer, sock);
				if (err != 0) {	
					socket_eof (sock);
					goto out;
				} else if (sock->readfn) {
					(*sock->readfn) (sock);
					goto out;
	                        }
			}
		}

		if ( POLLHUP & poll_fds[i].revents ) {
			if (sock->flags & SOCKET_FLAG_CONNECTING) {
				socket_finish_connect (sock);
				goto out;
			} else {
				socket_eof (sock);
				goto out;
			}
		}
	}

out:
}

/* Shared variables between socket_glib_poll_func and socket_event_pump */

static struct pollfd *s_poll_fds = NULL;
static unsigned int s_count_my_fds;
/* socket_array maps pollfd items (based on index) to Socket*'s, for */
/* resolving when poll() returns */
static Socket **s_socket_array = NULL;

/**
 * socket_event_pump
 * 
 * Dispatch socket events that occurred during the last call to
 * socket_glib_poll_func
 * 
 * This obnoxious hack is necessary so that functions called by socket
 * event dispatches can call g_main_iterate -- if they called g_main_iterate
 * while the glib main loop was still socket_glib_poll_func, glib would
 * not be that happy.
 * 
 * It is intended to be called like this:
 * 
 * 	while (g_main_is_running(l_main_loop)) {
 *		g_main_iteration (TRUE);
 *		socket_event_pump ();
 *	}
 *
 * 
 * For those who ask why a poll function wrapper is used rather than IoChannels,
 * the answer is essentially that the proxy needs to turn on and off 
 * the select bits for each FD rather dynamically, depending upon whether
 * the buffer is full or a socket is currently frozen.  There's no convienient
 * way to do this in glib.
 *  
 */
void 
socket_event_pump ()
{
	if ( NULL != s_poll_fds && NULL != s_socket_array && s_count_my_fds > 0 ) {
		dispatch_socket_events (s_poll_fds, s_socket_array, s_count_my_fds);
	}
}


/* Note that this function may return before "timeout" has elapsed.
 * So, in fact, it has different semantics than poll()
 */
gint
socket_glib_poll_func (GPollFD *ufds, guint nfds, gint timeout)
{
	Socket *sock;
	int poll_return;

	unsigned int count_poll_fds;
	guint i;

	/* Clean up from last time */
	g_free (s_poll_fds);
	g_free (s_socket_array);
	s_count_my_fds = 0;

	/* Close timed-out sockets, count active FD's */

	for (sock = socket_chain; sock; ) {
		if ((sock->flags & SOCKET_FLAG_CLOSING) &&
		    (time(NULL) - sock->time > SOCKET_TIMEOUT)) {
			Socket *old_sock;
		    
			/* trying to close this socket.  was going to wait for
			 * the output buffer to flush, but it's taking too long.
			 * ditch this sucker.
			 */
			sock->out_buffer->used = 0;
			sock->flags &= ~SOCKET_FLAG_CLOSING;

			/* socket_close removes the socket from the 
			 * socket list, so we need to be careful when iterating
			 */
			old_sock = sock;
			sock = sock->next;
			socket_close (old_sock);
			/* Skip normal iteration below */
			continue;
		} else if (0 == sock->frozen_count) {
			/* skip frozen sockets in our count */
			s_count_my_fds++;
		}

		/* Normal iteration */
		sock = sock->next;
	}

	count_poll_fds = s_count_my_fds + nfds;
	s_poll_fds = g_new0 (struct pollfd, count_poll_fds);
	s_socket_array = g_new0 (Socket *, s_count_my_fds);

	/* Add our sockets to the pool_fds array */
	
	for (sock = socket_chain, i = 0; sock; sock = sock->next) {

		/* skip frozen sockets; they were skipped in s_count_my_fds above
		 * as well
		 */
		if ( 0 == sock->frozen_count ) {
			s_poll_fds[i].fd = sock->fd;

	                if ((sock->flags & SOCKET_FLAG_LISTEN) ||
	                    (sock->in_buffer->used < sock->in_buffer->size)) {
				s_poll_fds[i].events |= POLLIN;
			}		
			if ((sock->flags & SOCKET_FLAG_CONNECTING) ||
	                    (sock->out_buffer->used > 0)) {
				s_poll_fds[i].events |= POLLOUT;
			}
		
			/* keep track of which poll_fd element goes to which Socket* */
			s_socket_array[i] = sock;

			i++;
		}
	}

	/* Fill out pollfd's for the fd's that glib handed us */
	/* This copy sucks, esp since the structs are probably the same */
	for (i = 0; i < nfds; i++ ) {
		s_poll_fds[i + s_count_my_fds].fd = ufds[i].fd;
		s_poll_fds[i + s_count_my_fds].events = ufds[i].events;
	}

	poll_return = poll (s_poll_fds, count_poll_fds, timeout);
	if (poll_return <= 0) {
		goto error;
	}

	/* Subtract our activated sockets from the poll return value*/
	for (i = 0; i < s_count_my_fds; i++) {
		if ( 0 != s_poll_fds[i].revents ) {
			poll_return--;	 
		}
	}

	/* Copy revents for glib's poll fd's */
	if (poll_return > 0) {
		for (i = 0; i < nfds; i++ ) {
			ufds[i].revents = s_poll_fds[i + s_count_my_fds].revents;
		}
	}
	
error:
	return poll_return;
}

int
socket_listen(Socket *sock, SocketAcceptFn acceptfn)
{
        if (listen (sock->fd, 64) != 0) {
                return 0;
        }

        sock->flags |= SOCKET_FLAG_LISTEN;
        sock->acceptfn = acceptfn;
        return 1;
}

/**
 * socket_freeze
 * 
 * Temporarally prevent socket events on this particular from being processed,
 * and thus generating callbacks.  Useful for modal event loops.
 */

void
socket_freeze (Socket *sock)
{
	sock->frozen_count++;
}

/**
 * socket_thaw
 * 
 * undo a socket_freeze
 */

void
socket_thaw (Socket *sock)
{
	sock->frozen_count--;
	g_assert ( sock->frozen_count >= 0 );
}

/**
 * socket_tunnel
 * 
 * Connect two sockets directly together, directing the output of one into another
 * 
 * socket_tunnel can set up tunnelling in one direction or both directions.  Once
 * tunnelling has been set up one direction, socket_tunnel can be called again
 * to establish it in both directions but it can never be disabled in a direction
 * once it has been established.
 * 
 * Once tunnelling has been established between two sockets, neither of 
 * the sockets may be tunneled to any other socket.
 */

void
socket_tunnel(Socket *left, Socket *right, SocketTunnelMode mode)
{
        g_return_if_fail (left != NULL);
        g_return_if_fail (right != NULL);
	g_return_if_fail (left != right);
        /* Can only tunnel to a socket you've already tunneled to */
        g_return_if_fail (left->tunnel == NULL || left->tunnel == right);
        g_return_if_fail (right->tunnel == NULL || right->tunnel == left);
	/* Can only upgrade modes--can't downgrade */
	g_return_if_fail ( ! (IS_TUNNEL_LEFT_RIGHT(left,right) && !(mode & SOCKET_TUNNEL_LEFT_TO_RIGHT) ) );
	g_return_if_fail ( ! (IS_TUNNEL_RIGHT_LEFT(left,right) && !(mode & SOCKET_TUNNEL_RIGHT_TO_LEFT) ) );

        left->tunnel = right;
        right->tunnel = left;

	/*
	 * Tunnelling:
	 * 1. Anything existing in the tunneler's output buffer will be appended
	 *    to the tunnellee's input buffer
	 * 2. The tunneler's output buffer is disposed of, and connected directly
	 *    to the tunnellee's input buffer
	 * 3. See socket_destroy for special-case socket closing code
	 */

	if ((mode & SOCKET_TUNNEL_LEFT_TO_RIGHT) && !IS_TUNNEL_LEFT_RIGHT (left,right) ) {
	        sockbuf_append_buffer (right->out_buffer, left->in_buffer);
	        sockbuf_free (left->in_buffer);
	        left->in_buffer = right->out_buffer;
	}

	if ((mode & SOCKET_TUNNEL_RIGHT_TO_LEFT) && !IS_TUNNEL_RIGHT_LEFT (left,right) ) {
	        sockbuf_append_buffer (left->out_buffer, right->in_buffer);
	        sockbuf_free (right->in_buffer);
	        right->in_buffer = left->out_buffer;
	}
}

#ifdef HAVE_OPENSSL

/* Begins SSL on an established Socket */
/* This call will block */
int
socket_begin_ssl (Socket *sock)
{
	int fcntl_flags;
	int err;
	
	sock->flags |= SOCKET_FLAG_USE_SSL;

	/* NOTE: ssl negotiation has to be done over a nonblocking
	 * socket.  that's kinda lame.
	 */
	fcntl_flags = fcntl(sock->fd, F_GETFL, 0);
	fcntl(sock->fd, F_SETFL, fcntl_flags &~ O_NONBLOCK);

	sock->ssl_handle = ssl_begin_ssl (sock->fd, &err);

	return (NULL != sock->ssl_handle);
}
#endif /* HAVE_OPENSSL */

void
socket_set_read_fn(Socket *sock, SocketReadFn readfn)
{
        g_return_if_fail (sock != NULL);

	sock->readfn = readfn;
}

void
socket_set_eof_fn(Socket *sock, SocketEofFn eoffn)
{
        g_return_if_fail (sock != NULL);

	sock->eoffn = eoffn;
}

void
socket_set_data(Socket *sock, void *data)
{
        g_return_if_fail (sock != NULL);

	sock->data = data;
}

void *
socket_get_data(Socket *sock)
{
        g_return_val_if_fail (sock != NULL, NULL);

	return sock->data;
}

void
socket_get_byte_counts(Socket *sock, guint32 *read, guint32 *written)
{
	g_assert (IS_SOCKET (sock));

	if (read) {
		*read = sock->bytes_read;
	}
	if (written) {
		*written = sock->bytes_written;
	}
}

void
socket_log_debug(void)
{
	Socket *sock;
	char *state, statebuf[10];
	char calls[20], *callptr;

	log ("::: socket debug dump :::");

	for (sock = socket_chain; sock; sock = sock->next) {
		if (sock->flags & SOCKET_FLAG_LISTEN) {
			state = "lstn";
		} else if (sock->flags & SOCKET_FLAG_CONNECTING) {
			state = "conn";
                } else if (sock->tunnel) {
                        state = statebuf;
                        sprintf (statebuf, "->%2d", sock->tunnel->fd);
		} else {
			state = " -- ";
		}

		callptr = calls;
		*callptr++ = (sock->acceptfn ? 'A' : ' ');
		*callptr++ = (sock->eoffn ? 'E' : ' ');
		*callptr++ = (sock->readfn ? 'R': ' ');
		*callptr++ = (sock->connectfn ? 'C' : ' ');
#ifdef HAVE_OPENSSL
		if (sock->flags & SOCKET_FLAG_USE_SSL) {
			strcpy (callptr, "(ssl)");
		} else {
			strcpy (callptr, "     ");
		}
		callptr += 5;
#endif
		*callptr = 0;

		log ("%4d %s (read:%5d) (writ:%5d)  %s %08X", sock->fd, state, sock->bytes_read,
		     sock->bytes_written, calls, sock->data);
		if (sock->in_buffer->used || sock->out_buffer->used) {
			log ("        (inbuf:%5d) (outbuf:%5d)", sock->in_buffer->used,
			     sock->out_buffer->used);
		}
	}

	log ("::: end of socket debug dump :::");
}
