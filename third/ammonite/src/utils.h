/* $Id: utils.h,v 1.1.1.1 2001-01-16 15:26:17 ghudson Exp $
 *
 * See 'utils.h' for a detailed description.
 *
 * Copyright (C) 1998  Steven Young
 * Copyright (C) 1999  Robert James Kaes (rjkaes@flarenet.com)
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

#ifndef __UTILS_H_
#define __UTILS_H_	1

#include <unistd.h>
#include <stdlib.h>
#include "sock.h"

/* Cheesy macros */
#define STRING_STARTS_WITH(string, with) (0 == strncmp( (string), (with), strlen(with)))
#define STRING_EQUALS(string, with) (0 == memcmp( (string), (with), strlen(with) + 1))
#define STRING_CASE_EQUALS(string, with) (0 == g_strcasecmp( (string), (with) ))

/* Watch out--these macros eval their args multiple times */
#define u_replace_string(p_to_replace, with) \
	do {							\
		gchar *orig = *p_to_replace;			\
		*(p_to_replace) = (with);			\
		g_free (orig);					\
	} while (0);

#define u_concat_replace_string(p_to_replace, with) \
	do {									\
		if ( NULL == *(p_to_replace) ) {				\
			*(p_to_replace) = g_strdup (with);			\
		} else {							\
			u_replace_string (p_to_replace, 			\
				g_strconcat(*(p_to_replace), with, NULL ));	\
		}								\
	} while (0);


#define NO_EXCEPTION(p_ev) (CORBA_NO_EXCEPTION == (p_ev)->_major)

#define ARRAY_LENGTH(x)  (sizeof(x)/sizeof(x[0]))

int show_stats(Socket *sock);
int http_error(Socket *sock, int err, char *msg);
void make_daemon(void);
int eazel_check_run(void);
int eazel_check_connection (int fd);

char * to_hex_string (guchar *binary, size_t length);
char * util_url_encode (const char *to_escape);

GList * piece_response_add (GList *list_response_pieces, char *piece, size_t piece_len);
char * piece_response_combine (GList *list_response_pieces, size_t *p_length);
void piece_response_free (GList *list_response_pieces);

GList * g_list_remove_all_custom (GList * list, gpointer data, GCompareFunc func);

pid_t util_fork_exec (const char *path, char *const argv[]);

gboolean util_validate_url (const char * url);

gint /* GCompareFunc */ util_glist_string_starts_with (gconstpointer a, gconstpointer b);

#endif
