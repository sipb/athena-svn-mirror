/* $Id: sock-ssl.c,v 1.1.1.1 2001-01-16 15:26:24 ghudson Exp $
 *
 * Handles the ugliness of using the openSSL library.
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

/* only build this file if openssl is installed */
#ifdef HAVE_OPENSSL

/* This is stuff to disable potentially problematic crypto */
#define NO_MD2 
#define NO_MD4 
#define NO_RC4 
#define NO_RC5 
#define NO_MDC2 
#define NO_IDEA 
#define NO_RSA 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <glib.h>

#include "log.h"
#include "sock-ssl.h"

/* verify server's cert?  unverified cert -> failed connect */
#define VERIFY_CERTS

/* dump debug info about the ciphers used? */
#undef DEBUG_CIPHER_LIST

/* log cipher & # bits every time a connection is made? */
#undef SOCKET_SSL_VERBOSE


/* global SSL context */
static SSL_CTX *ssl_context;


#ifdef VERIFY_CERTS
static int
verify_callback(int state, X509_STORE_CTX *x509_context)
{
	char name[256];
	int err;

	err = X509_STORE_CTX_get_error (x509_context);
	if (! state) {
		X509_NAME_oneline (X509_get_subject_name (x509_context->current_cert), name, sizeof(name));
		/* Remote site specified a certificate, but it's not correct */
		log ("ssl cert error: %s: %s", X509_verify_cert_error_string(x509_context->error), name);
		/* reject */
		return 0;
	}

	/* Accept connection */
	return 1;
}
#endif	/* VERIFY_CERTS */


/* stolen from openssl */
static void
ssl_log_error(const char *prefix)
{
	unsigned long l;
	char buf[200];
	const char *file, *data;
	int line, flags;

	l = ERR_get_error_line_data (&file, &line, &data, &flags);
	if (l) {
		log ("%s%s: %s", prefix, ERR_error_string (l, buf),
		     (flags & ERR_TXT_STRING) ? data : "");
	} else {
		log ("%s(no ssl error)", prefix);
	}
}


/* initialize the openssl library and set up some callbacks and paths.
 *     cert_directory: a directory where CA certs can be found
 *     cert_file: a single CA cert file
 * either cert_directory or cert_file may be NULL.  if both are NULL, then
 * no cert verification will succeed, so all connections will fail if we're
 * checking the server cert.  in this case, you should #undef VERIFY_CERTS
 * and as a side effect, you lose all auth checking, so don't do it.
 *
 * generally you will just pass in the path to the cert file that contains
 * the public key for the CA that signed the upstream web server's key.
 * for example, for the apache-modssl demo key set, you would use the
 * snakeoil-ca-rsa.crt file.
 */
void
eazel_init_ssl(char *cert_directory, char *cert_file)
{
	/* this is all MAGIC.  nobody knows how it works and openSSL is undocumented.
	 * touch this at your own peril!! 
	 */
	SSLeay_add_ssl_algorithms ();
	SSL_load_error_strings ();

	/* apparently this can never error */
	ssl_context = SSL_CTX_new (SSLv3_client_method ());
	if (! ssl_context) {
		ssl_log_error ("FATAL: openssl context creation failed: ");
		exit (1);
	}

	/* set verify level on certs */
	if (! SSL_CTX_set_default_verify_paths (ssl_context)) {
		ssl_log_error ("FATAL: openssl default configuration failed: ");
		exit (1);
	}

	if (! SSL_CTX_load_verify_locations (ssl_context, cert_file, cert_directory)) {
		ssl_log_error ("FATAL: openssl cert loading failed: ");
		exit(1);
	}

#ifdef VERIFY_CERTS
	SSL_CTX_set_verify (ssl_context, SSL_VERIFY_PEER, verify_callback);
#endif
}


/* turn off SSL ciphers that have patent issues */
static int
socket_turn_off_evil_ciphers(SSL *ssl_handle)
{
	STACK_OF(SSL_CIPHER) *stack;
	char *rule = NULL, *newbuf;
	const char *cipher_name;
	int i;

	stack = SSL_get_ciphers (ssl_handle);
	for (i = 0; i < sk_SSL_CIPHER_num (stack); i++) {
		cipher_name = SSL_CIPHER_get_name (sk_SSL_CIPHER_value (stack, i));
		/* RSA: patented till around October 2000 */
		/* IDEA & RC5: patented till longer */
		/* RC4: questionable (leaked trade secret?) */
		if ((strstr (cipher_name, "RSA") != NULL) ||
		    (strstr (cipher_name, "IDEA") != NULL) ||
		    (strstr (cipher_name, "RC4") != NULL) ||
		    (strstr (cipher_name, "RC5") != NULL)) {
			if (rule) {
				newbuf = g_strconcat (rule, ":-", cipher_name, NULL);
				g_free(rule);
				rule = newbuf;
			} else {
				rule = g_strconcat ("DEFAULT:-", cipher_name, NULL);
			}
		}
	}
	/* stack is not allocated, don't free it. */

#ifdef DEBUG_CIPHER_LIST
	log ("SSL cipher list rule: '%s'", rule ? rule : "(none)");
#endif
	if (rule && (! SSL_set_cipher_list (ssl_handle, rule))) {
		ssl_log_error ("error: ssl_set_cipher_list: ");
		return SOCKET_SSL_ERROR_OPENSSL;
	}

#ifdef DEBUG_CIPHER_LIST
	/* debug list */
	stack = SSL_get_ciphers (ssl_handle);
	log ("SSL cipher list: (%d entries)", sk_SSL_CIPHER_num (stack));
	for (i = 0; i < sk_SSL_CIPHER_num (stack); i++) {
		log ("stack %d: %s", i, SSL_CIPHER_get_name (sk_SSL_CIPHER_value (stack, i)));
	}
#endif

	return 0;
}


/* given a normal socket that's just been connected somewhere, attempt to open an SSL
 * session.
 * on success, returns an SSL handle which can be used to do SSL_read/SSL_write.
 */
SSL *
ssl_begin_ssl(int fd, int *error)
{
	SSL *ssl_handle;
	int err;
	int bits, alg_bits;
	X509 *server_cert;

	*error = 0;

	/* AFAIK, these 3 calls should never fail.  -robey */

	ssl_handle = SSL_new (ssl_context);
	if (! ssl_handle) {
		ssl_log_error ("error: SSL_new: ");
		*error = SOCKET_SSL_ERROR_OPENSSL;
		goto out;
	}

	err = SSL_set_fd (ssl_handle, fd);
	if (err == 0) {
		ssl_log_error ("error: SSL_set_fd: ");
		*error = SOCKET_SSL_ERROR_OPENSSL;
		goto bad;
	}

	err = socket_turn_off_evil_ciphers (ssl_handle);
	if (err) {
		*error = err;
		goto bad;
	}

	err = SSL_connect (ssl_handle);
	if (err == 0) {
		ssl_log_error ("error: SSL_connect: ");
		*error = SOCKET_SSL_ERROR_OPENSSL;
		goto bad;
	}

	bits = SSL_get_cipher_bits (ssl_handle, &alg_bits);
	if (bits == 0) {
		log ("error: SSL not supported by server");
		*error = SOCKET_SSL_ERROR_NOT_SUPPORTED;
		goto bad;
	}

#ifdef SOCKET_SSL_VERBOSE
	log ("SSL connect: cipher %s, %d bits", SSL_get_cipher_name (ssl_handle), bits);
#endif

	server_cert = SSL_get_peer_certificate (ssl_handle);
	if (! server_cert) {
		ssl_log_error ("error: no server cert: ");
		*error = SOCKET_SSL_ERROR_NO_CERT;
		goto bad;
	}

out:
	return ssl_handle;

bad:
	SSL_free(ssl_handle);
	return NULL;
}


#endif	/* HAVE_OPENSSL */
