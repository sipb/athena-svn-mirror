/* $Id: digest.c,v 1.1.1.1 2001-01-16 15:26:04 ghudson Exp $
 * 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:  Michael Fleming <mfleming@eazel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "digest.h"
#include "http-connection.h"
#include "utils.h"
#include "log.h"


#include <unistd.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/md5.h>
#include <openssl/rand.h>

#define MD5_DIGEST_LENGTH 16
#define SIZE_RANDOM_SEED 256

struct DigestState {
	int nonce_count;
	char *token;
	char *nonce;
	char *realm;
	char *username;
} ;

static char * digest_gen_token (const char * username, const char *realm, const char *password );
static void digest_random_seed ();
static char * digest_gen_cnonce();

/* Note: this function may block! */

static void
digest_random_seed ()
{
	FILE *random_file;
	unsigned char * random_bytes [SIZE_RANDOM_SEED];
	gboolean success = FALSE;
	static gboolean has_inited = FALSE;

	if ( has_inited ) {
		return;
	}

	if ( NULL != (random_file = fopen ("/dev/urandom", "ro") ) ) {
		if ( 1 == fread (random_bytes, SIZE_RANDOM_SEED, 1, random_file ) ) {
			success = TRUE;
		}
		fclose (random_file);
	}

	if ( !success && NULL != (random_file = fopen ("/dev/random", "ro" ) ) ) {
		if ( 1 == fread (random_bytes, SIZE_RANDOM_SEED, 1, random_file ) ) {
			success = TRUE;
		}
		fclose (random_file);
	}

	if ( !success ) {
		/* This oughtta get me in Schneier's doghouse for sure */
		int i;
		
		srandom (time(NULL) + getpid());

		for ( i=0; i < SIZE_RANDOM_SEED/sizeof(long int); i++ ) {
			*((long int *)(random_bytes+(i*sizeof(long int)))) = random();
		}  
	}

	RAND_seed (random_bytes, 256);

	has_inited = TRUE;
}

static char *
digest_gen_cnonce()
{
	guchar random_byte;
	int cnonce_size;
	guchar * random_bytes;
	
	digest_random_seed();

	RAND_bytes (&random_byte, 1);

	cnonce_size = (random_byte % 27) + 5;	/* cnonce length is between 5 and 32 */

	random_bytes = g_new (guchar, cnonce_size);
	RAND_bytes (random_bytes, cnonce_size);

	return to_hex_string (random_bytes, cnonce_size);
}



/**
 * 
 * digest_init
 * 
 * Initialize a digest authentication session.
 * 
 * username  -- username that should be authenticated
 * passwd    -- passwd that should be authenticated
 * authn_header -- a "WWW-Authenticate: Digest" header line.  This argument is
 *                 destroyed during the function call
 * 
 * Returns a DigestState * or NULL if error
 *  
 */

DigestState * 
digest_init ( const char *username, const char *passwd , char *authn_header )
{
	char *current;
	char *key;
	char *value;
	char *realm = NULL;
	char *nonce = NULL;
	gboolean success = TRUE;
	gboolean have_algorithm = FALSE;
	DigestState *p_digest = NULL;;

	g_return_val_if_fail ( NULL != username, NULL);
	g_return_val_if_fail ( NULL != passwd, NULL);
	g_return_val_if_fail ( NULL != authn_header, NULL);

	p_digest = g_new0 (DigestState, 1);

	current = authn_header;		
	current += strlen (HTTP_AUTHENTICATE_HEADER);

	log ("Got header: %s", authn_header);

	for ( ; *current && isspace (*current); current++ ) ;

	if ( ! STRING_STARTS_WITH (current, "Digest")) {
		log ("Received WWW-Authenticate, but isn't 'Digest'");
		success = FALSE;
		goto error;;
	}

	current += strlen ("Digest");

	if ( ! isspace (*current) ) {
		log ("Received WWW-Authenticate, but isn't 'Digest'");
		success = FALSE;
		goto error;
	}

	for ( ; *current && isspace (*current); current++ ) ;

	while ( ! http_parse_authn_header ( &current, &key, &value ) ) {
		if ( STRING_CASE_EQUALS (key, "realm") && value ) {
			realm = g_strdup (value);
		} else if ( STRING_CASE_EQUALS (key, "nonce") && value ) {
			nonce = g_strdup (value);
		} else if ( STRING_CASE_EQUALS (key, "algorithm") && value && ( STRING_CASE_EQUALS (value, "MD5") /* || STRING_CASE_EQUALS (value, "md5-sess") */ ) ) {
			/* Is this supposed to be "MD5" or "md5-sess"? */
			have_algorithm = TRUE;
		} else if ( STRING_CASE_EQUALS (key, "qop" ) && value ) {
			char *tmp;
			if ( ! ( (tmp = strstr (value, "auth") ) && ( '\0' == tmp[4] || isspace (tmp[4]) ) ) ) {
				log ("Incompatible WWW-Authenticate header");
				success = FALSE;
				goto error;
			}
		}
	}

	/* Looks like apache 1.3.x doesn't send the algorithm */ 
	if ( ! (realm && nonce /*&& have_algorithm*/ ) ) {
		log ("Incompatible WWW-Authenticate header");
		success = FALSE;
		goto error;
	}

	p_digest->realm = realm;
	p_digest->nonce = nonce;
	p_digest->username = g_strdup (username);

	p_digest->token = digest_gen_token ( username, realm, passwd );

	log ("Token: '%s' nonce: '%s' realm: '%s'", p_digest->token, p_digest->nonce, p_digest->realm);

error:
	if ( ! success ) {
		g_free (nonce);
		g_free (realm);
		g_free (p_digest);
		p_digest = NULL;
	}
	
	return p_digest;
}

void
digest_change_password ( DigestState *p_digest_state, const char *passwd)
{
	p_digest_state->token = digest_gen_token ( p_digest_state->username, p_digest_state->realm, passwd );

	p_digest_state->nonce_count = 0;
	
	log ("New Digest Username: '%s' Token: '%s' username nonce: '%s' realm: '%s'", 
		p_digest_state->username, 
		p_digest_state->token, 
		p_digest_state->nonce, 
		p_digest_state->realm
	);

}


char * 
digest_gen_response_header (DigestState *p_digest, const char *uri, const char *http_method)
{
	MD5_CTX context;
	
	guchar A2[MD5_DIGEST_LENGTH];			/* A2 as defined by RFC 2617 */
	guchar digest[MD5_DIGEST_LENGTH];

	const char * const colon = ":";

	char *ret = NULL;
	
	char *cnonce = NULL;
	char *nonce_count_string = NULL;
	char *A2_string = NULL;
	char *digest_string = NULL;

	cnonce = digest_gen_cnonce();
	nonce_count_string = g_strdup_printf ("%08x", ++p_digest->nonce_count);

	/* First, create A2 */
	MD5_Init (&context);
	/*
	 * There appears to be a real problem with implementations for A2 here.
	 * RFC 2831 (SASL) states that A2 should be "AUTHENTICATE:<uri>" for
	 * qop="auth", but RFC 2617 states that it should be "<method>:<uri>" 
	 * 
	 * What's *REALLY* bad is that this mechanism is busted with Apache 1.3.12
	 * for methods such as HEAD.  For some reason, mod_auth_digest seems
	 * to get called multiple times for a single request.  If the request's
	 * method is HEAD, it'll get called once with a method set to HEAD
	 * and again with method set to GET.  When the method is set to GET,
	 * ha2 will be calculated incorrectly and authentication will be denied.
	 * 
	 * To get mod_auth_digest to work with the RFC2831 flag, replace the
	 * following line in "mod_auth_digest.c:new_digest()"
	 * 	a2 = ap_pstrcat(r->pool, r->method, ":", resp->uri, NULL);
	 * with:
	 * 	a2 = ap_pstrcat(r->pool, "AUTHENTICATE", ":", resp->uri, NULL);
	 */
#undef RFC2831 
#ifdef RFC2831
	MD5_Update (&context, "AUTHENTICATE", strlen ("AUTHENTICATE"));
#else 
	MD5_Update (&context, http_method, strlen (http_method));
#endif
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, uri, strlen (uri));
	MD5_Final (A2, &context);

#ifdef RFC2831
	log ("Generating ha2 from '%s:%s'", "AUTHENTICATE", uri);
#else 
	log ("Generating ha2 from '%s:%s'", http_method, uri);
#endif
	A2_string = to_hex_string(A2, MD5_DIGEST_LENGTH);

	MD5_Init (&context);
	MD5_Update (&context, p_digest->token, strlen (p_digest->token));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, p_digest->nonce, strlen (p_digest->nonce));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, nonce_count_string, strlen (nonce_count_string));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, cnonce, strlen (cnonce));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, "auth", strlen ("auth"));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, A2_string, 2*MD5_DIGEST_LENGTH );
	MD5_Final (digest, &context);

	log ("Generating digest from '%s:%s:%s:%s:%s:%s'", p_digest->token, p_digest->nonce, 
		nonce_count_string, cnonce, "auth", A2_string);

	digest_string = to_hex_string (digest, MD5_DIGEST_LENGTH);

	ret = g_strdup_printf ("Authorization: Digest username=\"%s\", "
				"realm=\"%s\", nonce=\"%s\", uri=\"%s\", "
				"qop=auth, nc=%s, cnonce=\"%s\", response=\"%s\"",
				p_digest->username,
				p_digest->realm,
				p_digest->nonce,
				uri,
				nonce_count_string,
				cnonce,
				digest_string
	      );

	log ("%s", ret);

	g_free (cnonce);
	g_free (nonce_count_string);
	g_free (A2_string);
	g_free (digest_string);

	return ret;
}

void
digest_free (DigestState * digest)
{
	if (NULL != digest) {
		g_free (digest->token);	
		g_free (digest->nonce);	
		g_free (digest->realm);	
		g_free (digest->username);	
		g_free (digest);
	}	
}


/*
 * digest_gen_token
 * 
 * This function will generate a user's long-term token.  Assuming the
 * qop is auth, and the algorithm is MD5-sess, this token will be
 * valid for the lifetime of the user's account.
 * 
 * username - The user's name
 * realm_value - The realm-value
 * passphrase - The user's pass phrase
 *
 * returns hex-encoded generated token, which must be g_free()'d.
 */

static char *
digest_gen_token (const char * username, const char *realm, const char *password )
{
	MD5_CTX context; /* the context for the MD5 object */
	const char * const colon = ":";
	guchar digest[MD5_DIGEST_LENGTH];

	/* First, calculate
	 * H(username + ":" + realm_value + ":" + password)
	 * I use the variable name 'passphrase' because
	 * I think it makes more sense
	 */

	MD5_Init (&context);
	MD5_Update (&context, username, strlen (username));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, realm, strlen (realm));
	MD5_Update (&context, colon, 1);
	MD5_Update (&context, password, strlen (password));

 	MD5_Final (digest, &context);

	return to_hex_string (digest, MD5_DIGEST_LENGTH);
}
