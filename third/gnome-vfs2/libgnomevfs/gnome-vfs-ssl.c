/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-ssl.h
 *
 * Copyright (C) 2001 Ian McKellar
 * Copyright (C) 2002 Andrew McDonald
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */
/*
 * Authors: Ian McKellar <yakk@yakk.net>
 *   My knowledge of SSL programming is due to reading Jeffrey Stedfast's
 *   excellent SSL implementation in Evolution.
 *
 *          Andrew McDonald <andrew@mcdonald.org.uk>
 *   Basic SSL/TLS support using the LGPL'ed GNUTLS Library
 */

#include <config.h>
#include "gnome-vfs-ssl.h"

#include "gnome-vfs-ssl-private.h"
#include <glib/gmem.h>
#include <string.h>

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#elif defined HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif
#if defined(HAVE_OPENSSL) || defined(HAVE_GNUTLS)
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

typedef struct {
#ifdef HAVE_OPENSSL
	int sockfd;
	SSL *ssl;
#elif defined HAVE_GNUTLS
	int sockfd;
	GNUTLS_STATE tlsstate;
	GNUTLS_CERTIFICATE_CLIENT_CREDENTIALS xcred;
#elif defined HAVE_NSS
	PRFileDesc *sockfd;
#else
	char	dummy;
#endif
} GnomeVFSSSLPrivate;

struct GnomeVFSSSL {
	GnomeVFSSSLPrivate *private;
};

void 
_gnome_vfs_ssl_init () {
#ifdef HAVE_OPENSSL
	SSL_library_init ();
#elif defined HAVE_GNUTLS
	gnutls_global_init();
#endif
}

/**
 * gnome_vfs_ssl_enabled:
 *
 * Checks whether GnomeVFS was compiled with SSL support.
 *
 * Return value: %TRUE if GnomeVFS was compiled with SSL support,
 * otherwise %FALSE.
 **/
gboolean
gnome_vfs_ssl_enabled ()
{
#if defined HAVE_OPENSSL || defined HAVE_GNUTLS
	return TRUE;
#else
	return FALSE;
#endif
}

/**
 * gnome_vfs_ssl_create:
 * @handle_return: pointer to a GnmoeVFSSSL struct, which will
 * contain an allocated GnomeVFSSSL object on return.
 * @host: string indicating the host to establish an SSL connection with
 * @port: the port number to connect to
 *
 * Creates an SSL socket connection at @handle_return to @host using
 * port @port.
 *
 * Return value: GnomeVFSResult indicating the success of the operation
 **/
GnomeVFSResult
gnome_vfs_ssl_create (GnomeVFSSSL **handle_return, 
		      const char *host, 
		      unsigned int port)
{
/* FIXME: add *some* kind of cert verification! */
#if defined(HAVE_OPENSSL) || defined(HAVE_GNUTLS)
	int fd;
	int ret;
	struct hostent *h;
	struct sockaddr_in sin;

        sin.sin_port = htons (port);
	h = gethostbyname (host);

	if (h == NULL) {
		/* host lookup failed */
		return gnome_vfs_result_from_h_errno ();
	}

        sin.sin_family = h->h_addrtype;
        memcpy (&sin.sin_addr, h->h_addr, sizeof (sin.sin_addr));

        fd = socket (h->h_addrtype, SOCK_STREAM, 0);
	if (fd < 0) {
		return gnome_vfs_result_from_errno ();
	}

	ret = connect (fd, (struct sockaddr *)&sin, sizeof (sin));
	if (ret == -1) {
		/* connect failed */
		return gnome_vfs_result_from_errno ();
	}

	return gnome_vfs_ssl_create_from_fd (handle_return, fd);
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

#ifdef HAVE_GNUTLS
static const int protocol_priority[] = {GNUTLS_TLS1, GNUTLS_SSL3, 0};
static const int cipher_priority[] = 
	{GNUTLS_CIPHER_RIJNDAEL_128_CBC, GNUTLS_CIPHER_3DES_CBC,
	 GNUTLS_CIPHER_RIJNDAEL_256_CBC, GNUTLS_CIPHER_TWOFISH_128_CBC,
	 GNUTLS_CIPHER_ARCFOUR, 0};
static const int comp_priority[] =
	{GNUTLS_COMP_ZLIB, GNUTLS_COMP_NULL, 0};
static const int kx_priority[] =
	{GNUTLS_KX_DHE_RSA, GNUTLS_KX_RSA, GNUTLS_KX_DHE_DSS, 0};
static const int mac_priority[] =
	{GNUTLS_MAC_SHA, GNUTLS_MAC_MD5, 0};
#endif

/**
 * gnome_vfs_ssl_create_from_fd:
 * @handle_return: pointer to a GnmoeVFSSSL struct, which will
 * contain an allocated GnomeVFSSSL object on return.
 * @fd: file descriptior to try and establish an SSL connection over
 *
 * Try to establish an SSL connection over the file descriptor @fd.
 *
 * Return value: a GnomeVFSResult indicating the success of the operation
 **/
GnomeVFSResult
gnome_vfs_ssl_create_from_fd (GnomeVFSSSL **handle_return, 
		              gint fd)
{
#ifdef HAVE_OPENSSL
	GnomeVFSSSL *ssl;
	SSL_CTX *ssl_ctx = NULL;
	int ret;

	ssl = g_new0 (GnomeVFSSSL, 1);
	ssl->private = g_new0 (GnomeVFSSSLPrivate, 1);
	ssl->private->sockfd = fd;

        /* SSLv23_client_method will negotiate with SSL v2, v3, or TLS v1 */
        ssl_ctx = SSL_CTX_new (SSLv23_client_method ());

	if (ssl_ctx == NULL) {
		return GNOME_VFS_ERROR_INTERNAL;
	}

        /* FIXME: SSL_CTX_set_verify (ssl_ctx, SSL_VERIFY_PEER, &ssl_verify);*/
        ssl->private->ssl = SSL_new (ssl_ctx);

	if (ssl->private->ssl == NULL) {
		return GNOME_VFS_ERROR_IO;
	}

        SSL_set_fd (ssl->private->ssl, fd);

	ret = SSL_connect (ssl->private->ssl);
	if (ret != 1) {
                SSL_shutdown (ssl->private->ssl);

                if (ssl->private->ssl->ctx)
                        SSL_CTX_free (ssl->private->ssl->ctx);

                SSL_free (ssl->private->ssl);
		g_free (ssl->private);
		g_free (ssl);
		return GNOME_VFS_ERROR_IO;
	}

	*handle_return = ssl;

	return GNOME_VFS_OK;

#elif defined HAVE_GNUTLS
	GnomeVFSSSL *ssl;
	int err;

	ssl = g_new0 (GnomeVFSSSL, 1);
	ssl->private = g_new0 (GnomeVFSSSLPrivate, 1);
	ssl->private->sockfd = fd;

	err = gnutls_certificate_allocate_sc (&ssl->private->xcred);
	if (err < 0) {
		g_free (ssl->private);
		g_free (ssl);
		return GNOME_VFS_ERROR_INTERNAL;
	}

	gnutls_init (&ssl->private->tlsstate, GNUTLS_CLIENT);

	/* set socket */
	gnutls_transport_set_ptr (ssl->private->tlsstate, fd);

	gnutls_protocol_set_priority (ssl->private->tlsstate, protocol_priority);
	gnutls_cipher_set_priority (ssl->private->tlsstate, cipher_priority);
	gnutls_compression_set_priority (ssl->private->tlsstate, comp_priority);
	gnutls_kx_set_priority (ssl->private->tlsstate, kx_priority);
	gnutls_mac_set_priority (ssl->private->tlsstate, mac_priority);

	gnutls_cred_set (ssl->private->tlsstate, GNUTLS_CRD_CERTIFICATE,
			 ssl->private->xcred);

	err = gnutls_handshake (ssl->private->tlsstate);

	while (err == GNUTLS_E_AGAIN || err == GNUTLS_E_INTERRUPTED) {
		err = gnutls_handshake (ssl->private->tlsstate);
	}

	if (err < 0) {
		gnutls_certificate_free_sc (ssl->private->xcred);
		gnutls_deinit (ssl->private->tlsstate);
		g_free (ssl->private);
		g_free (ssl);
		return GNOME_VFS_ERROR_IO;
	}

	*handle_return = ssl;

	return GNOME_VFS_OK;
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

/**
 * gnome_vfs_ssl_read:
 * @ssl: SSL socket to read data from
 * @buffer: allocated buffer of at least @bytes bytes to be read into
 * @bytes: number of bytes to read from @ssl into @buffer
 * @bytes_read: pointer to a GnomeVFSFileSize, will contain
 * the number of bytes actually read from the socket on return.
 *
 * Read @bytes bytes of data from the SSL socket @ssl into @buffer.
 *
 * Return value: GnomeVFSResult indicating the success of the operation
 **/
GnomeVFSResult 
gnome_vfs_ssl_read (GnomeVFSSSL *ssl,
		    gpointer buffer,
		    GnomeVFSFileSize bytes,
		    GnomeVFSFileSize *bytes_read)
{
#if HAVE_OPENSSL
	if (bytes == 0) {
		*bytes_read = 0;
		return GNOME_VFS_OK;
	}

	*bytes_read = SSL_read (ssl->private->ssl, buffer, bytes);

	if (*bytes_read <= 0) {
		*bytes_read = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#elif defined HAVE_GNUTLS
	if (bytes == 0) {
		*bytes_read = 0;
		return GNOME_VFS_OK;
	}

	*bytes_read = gnutls_record_recv (ssl->private->tlsstate, buffer, bytes);

	if (*bytes_read <= 0) {
		*bytes_read = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

/**
 * gnome_vfs_ssl_write:
 * @ssl: SSL socket to write data to
 * @buffer: data to write to the socket
 * @bytes: number of bytes from @buffer to write to @ssl
 * @bytes_written: pointer to a GnomeVFSFileSize, will contain
 * the number of bytes actually written to the socket on return.
 *
 * Write @bytes bytes of data from @buffer to @ssl.
 *
 * Return value: GnomeVFSResult indicating the success of the operation
 **/
GnomeVFSResult 
gnome_vfs_ssl_write (GnomeVFSSSL *ssl,
		     gconstpointer buffer,
		     GnomeVFSFileSize bytes,
		     GnomeVFSFileSize *bytes_written)
{
#if HAVE_OPENSSL
	if (bytes == 0) {
		*bytes_written = 0;
		return GNOME_VFS_OK;
	}

	*bytes_written = SSL_write (ssl->private->ssl, buffer, bytes);

	if (*bytes_written <= 0) {
		*bytes_written = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#elif defined HAVE_GNUTLS
	if (bytes == 0) {
		*bytes_written = 0;
		return GNOME_VFS_OK;
	}

	*bytes_written = gnutls_record_send (ssl->private->tlsstate, buffer, bytes);

	if (*bytes_written <= 0) {
		*bytes_written = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

/**
 * gnome_vfs_ssl_destroy:
 * @ssl: SSL socket to be closed and destroyed
 *
 * Free resources used by @ssl and close the connection.
 */
void
gnome_vfs_ssl_destroy (GnomeVFSSSL *ssl) 
{
#if HAVE_OPENSSL
	SSL_shutdown (ssl->private->ssl);
	SSL_CTX_free (ssl->private->ssl->ctx);
	SSL_free (ssl->private->ssl);
	close (ssl->private->sockfd);
#elif defined HAVE_GNUTLS
	gnutls_bye (ssl->private->tlsstate, GNUTLS_SHUT_RDWR);
	gnutls_certificate_free_sc (ssl->private->xcred);
	gnutls_deinit (ssl->private->tlsstate);
	close (ssl->private->sockfd);
#else
#endif
	g_free (ssl->private);
	g_free (ssl);
}

static GnomeVFSSocketImpl ssl_socket_impl = {
	(GnomeVFSSocketReadFunc)gnome_vfs_ssl_read,
	(GnomeVFSSocketWriteFunc)gnome_vfs_ssl_write,
	(GnomeVFSSocketCloseFunc)gnome_vfs_ssl_destroy
};

/**
 * gnome_vfs_ssl_to_socket:
 * @ssl: SSL socket to convert into a standard socket
 *
 * Wrapper an SSL socket inside a standard GnomeVFSSocket.
 *
 * Return value: a newly allocated GnomeVFSSocket corresponding to @ssl.
 **/
GnomeVFSSocket *
gnome_vfs_ssl_to_socket (GnomeVFSSSL *ssl)
{
	return gnome_vfs_socket_new (&ssl_socket_impl, ssl);
}
