/* $Id: sock-ssl.h,v 1.1.1.1 2001-01-16 15:26:24 ghudson Exp $
 *
 * Prototypes and definitions for the proxy server's SSL wrappers.
 *
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

#ifndef _SOCK_SSL_H_
#define _SOCK_SSL_H_

#ifdef HAVE_OPENSSL

void eazel_init_ssl(char *cert_directory, char *cert_file);
SSL *ssl_begin_ssl(int fd, int *error);

/* errors that might be returned by ssl_begin_ssl() */
#define SOCKET_SSL_ERROR_OPENSSL	1	/* some internal openssl library error */
#define SOCKET_SSL_ERROR_NOT_SUPPORTED	2	/* remote server doesn't support encryption (zero bits) */
#define SOCKET_SSL_ERROR_NO_CERT	3	/* server doesn't have any certificate */

#endif	/* HAVE_OPENSSL */

#endif	/* _SOCK_SSL_H_ */
