/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: Michael Fleming <mfleming@eazel.com>
 *
 */

#include "compat.h"
 
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "regexp.h"
#include "log.h"

/*
 * uid_from_local_sockaddr_in
 * 
 * Discover the UID of a local AF_INET socket's peer
 * Currently uses /proc/inet/tcp, which is Linux-specific 
 */

/*
 * From net/ipv4/proc.c :
 * 
 *	sprintf(tmpbuf, "%4d: %08lX:%04X %08lX:%04X"
 *		" %02X %08X:%08X %02X:%08lX %08X %5d %8d %ld",
 *	Note that buffer is 127 chars
 */
 
/* Prepend this pattern with "%08X:%04X %08X:%04X" for local/remote address/port */
#define PROC_TCP_PATTERN_NMATCH 2
#define PROC_TCP_PATTERN_MATCH_UID 1
#define PROC_TCP_PATTERN " [0-9A-F]{2} [0-9A-F]{8}:[0-9A-F]{8} [0-9A-F]{2}:[0-9A-F]{8} [0-9A-F]{8} +([0-9]{1,5})"
#define PROC_TCP_PATH "/proc/net/tcp"
#define PROC_BUFFER_SIZE (128+1)

gboolean
uid_from_local_sockaddr_in (
	const struct sockaddr_in *peer_name, 
	const struct sockaddr_in *our_name, 
	uid_t *p_uid
) {
	char *ip_string 		= NULL;
	char *search_string 		= NULL;
	FILE *proc_fp 			= NULL;
	gboolean result			= FALSE;
	char buffer[PROC_BUFFER_SIZE];
	int err;

	gboolean	preg_inited 	= FALSE;
	regex_t 	preg;
	regmatch_t 	pmatch[PROC_TCP_PATTERN_NMATCH];

	g_return_val_if_fail (AF_INET == peer_name->sin_family, FALSE);
	g_return_val_if_fail (AF_INET == our_name->sin_family, FALSE);
	g_return_val_if_fail (NULL != p_uid, FALSE);

	ip_string = g_strdup_printf("%08X:%04X %08X:%04X",
				peer_name->sin_addr.s_addr,
				ntohs(peer_name->sin_port),
				our_name->sin_addr.s_addr,
				ntohs(our_name->sin_port)
			);

	search_string = g_strconcat(ip_string, PROC_TCP_PATTERN, NULL);

	proc_fp = fopen(PROC_TCP_PATH, "r");

	if (NULL == proc_fp) {
		log ("Couldn't open %s", PROC_TCP_PATH);
		result = FALSE; 
		goto error;
	}

	err = regcomp (&preg, search_string, REG_EXTENDED | REG_ICASE );
	if ( 0 != err ) {
		log ("Couldn't compile search string");
		result = FALSE;
		goto error;
	}
	preg_inited = TRUE;

	result = FALSE;
	while (NULL != fgets (buffer, PROC_BUFFER_SIZE, proc_fp)) {
		err = regexec (&preg, buffer, PROC_TCP_PATTERN_NMATCH, pmatch, 0);

		if ( 0 == err 
		     && -1 != pmatch[PROC_TCP_PATTERN_MATCH_UID].rm_so
		) {
			buffer[pmatch[PROC_TCP_PATTERN_MATCH_UID].rm_eo] = 0;
			*p_uid = atoi(buffer + pmatch[PROC_TCP_PATTERN_MATCH_UID].rm_so);
			result = TRUE;
			break;
		}
	}

error:
	if (preg_inited) {
		regfree( &preg );
	}

	if (proc_fp) {
		fclose (proc_fp);
	}

	g_free (search_string);
	g_free (ip_string);

	return result;
}


