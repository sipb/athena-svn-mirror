/*
 * Copyright (C) 2000  Eazel, Inc.
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
 */

#ifndef _SOCK_H_
#define _SOCK_H_

/* This is stuff to disable potentially problematic crypto */
#define NO_MD2 
#define NO_MD4 
#define NO_RC4 
#define NO_RC5 
#define NO_MDC2 
#define NO_IDEA 
#define NO_RSA 

#include <time.h>
#include <glib.h>
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif
#include <stdio.h>

typedef struct _socket Socket;

typedef struct {
	char *buffer;
	int size;
	int used;
} SockBuf;

/* callback for a new incoming connection */
typedef void (*SocketAcceptFn)(Socket *sock, gpointer user_data, const char *host, int port);

/* callback when a socket dies */
typedef void (*SocketEofFn)(Socket *sock);

/* callback when new data has arrived on a socket */
typedef void (*SocketReadFn)(Socket *sock);

/* callback when a connection is complete (success or failure) */
typedef void (*SocketConnectFn)(Socket *sock, int success);

typedef enum {
	SOCKET_TUNNEL_NONE = 0,
	SOCKET_TUNNEL_LEFT_TO_RIGHT = 1,
	SOCKET_TUNNEL_RIGHT_TO_LEFT = 2,
	SOCKET_TUNNEL_BOTH = 3
} SocketTunnelMode;

struct _socket {
	guint32 magic;
	struct _socket *next;
	int fd;
        int flags;
	time_t time;			/* time socket was marked for closing */
	int	frozen_count;		/* If > 0, this socket is frozen and will not be poll'd() */
	SockBuf *in_buffer;
	SockBuf *out_buffer;
        SocketAcceptFn acceptfn;
        SocketEofFn eoffn;
        SocketReadFn readfn;
        SocketConnectFn connectfn;
	void *data;			/* user data (for storing state info) */
	struct _socket *tunnel;		/* if reads from this socket should go directly
					 * out to another socket, 'tunnel' specifies the
					 * socket to write to.
					 */	
	guint32 bytes_read;		/* bytes read/written across the socket lifetime */
	guint32 bytes_written;
#ifndef NO_DEBUG_MIRRORING
	FILE *input_mirror;
	FILE *output_mirror;
#endif
#ifdef HAVE_OPENSSL
	SSL *ssl_handle;		/* used for i/o on an ssl socket */
#endif
};


#define SOCKET_MAGIC		0x50C837FF
#define IS_SOCKET(sock)		((sock) && ((sock)->magic == SOCKET_MAGIC))

#define SOCKET_FLAG_LISTEN	(1<<0)	/* listening port */
#define SOCKET_FLAG_CONNECTING	(1<<1)	/* waiting for connect() result */
#define SOCKET_FLAG_CLOSING	(1<<2)	/* close, once the write buffer is empty */
#define SOCKET_FLAG_USE_SSL	(1<<3)	/* outbound connect only: use ssl on this socket */

/* default size for a socket buffer */
#define SOCKBUF_SIZE		4096

/* if a socket is closed, and there's still stuff buffered for writing, we'll
 * wait this long (in seconds) for the buffer to flush, before just dropping
 * the connection anyway.
 */
#define SOCKET_TIMEOUT		15


Socket *	socket_new		(const char *name, int port);
#ifndef NO_DEBUG_MIRRORING
void 		socket_set_mirrors	(Socket *sock, FILE *output_mirror, FILE *input_mirror);
#endif /* DEBUG */
void		socket_destroy		(Socket *sock);
void		socket_close		(Socket *sock);
char *		socket_getline		(Socket *sock);
int 		socket_write		(Socket *sock, char *data, int datalen);
int 		socket_read		(Socket *sock, char ** pp_data, size_t *p_len);
int 		socket_listen		(Socket *sock, SocketAcceptFn acceptfn);
void 		socket_freeze 		(Socket *sock);
void 		socket_thaw		(Socket *sock);
int		socket_connect		(Socket *sock, const char *name, int port, SocketConnectFn connectfn);
int		socket_connect_ssl	(Socket *sock, const char *name, int port, SocketConnectFn connectfn);

gint 		socket_glib_poll_func 	(GPollFD *ufds, guint nfds, gint timeout);
void		socket_event_pump 	(void);

void		socket_tunnel		(Socket *left, Socket *right, SocketTunnelMode mode);
#ifdef HAVE_OPENSSL
int		socket_tunnel_ssl 	(Socket *left, Socket *right, SocketTunnelMode mode);
int		socket_begin_ssl 	(Socket *sock);
#endif /* HAVE_OPENSSL */
void		socket_set_read_fn 	(Socket *sock, SocketReadFn readfn);
void		socket_set_eof_fn 	(Socket *sock, SocketEofFn eoffn);
void		socket_set_data 	(Socket *sock, void *data);
void *		socket_get_data		(Socket *sock);
void 		socket_get_byte_counts	(Socket *sock, guint32 *read, guint32 *written);
void 		socket_log_debug 	(void);

#endif	/* _SOCK_H_ */
