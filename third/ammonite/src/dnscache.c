/* $Id: dnscache.c,v 1.1.1.1 2001-01-16 15:25:51 ghudson Exp $
 *
 * This is a caching DNS system. When a host name is needed we look it up here
 * and see if there is already an answer for it. The domains are placed in a
 * hashed linked list. If the name is not here, then we need to look it up and
 * add it to the system. This really speeds up the connection to servers since
 * the DNS name does not need to be looked up each time. It's kind of cool. :)
 *
 * Copyright (C) 1999  Robert James Kaes (rjkaes@flarenet.com)
 * Copyright (C) 2000  Chris Lightfoot (chris@ex-parrot.com)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <glib.h>

#include "dnscache.h"


#define HASH_BOX 25
#define DNSEXPIRE (5 * 60)

struct dnscache_s {
	struct in_addr ipaddr;
	char *domain;
	time_t expire;
	struct dnscache_s *next;
};

struct dnscache_s *cache[HASH_BOX];

static unsigned int hash(const unsigned char *keystr, unsigned int box)
{
	unsigned long hashc = 0;
	unsigned char ch;

	g_assert (keystr != NULL);
	g_assert (box > 0);

	while ((ch = *keystr++))
		hashc += tolower(ch);

	return hashc % box;
}

static int lookup(struct in_addr *addr, const char *domain)
{
	unsigned int box = hash(domain, HASH_BOX);
	struct dnscache_s **rptr = &cache[box];
	struct dnscache_s *ptr = cache[box];

	assert(domain);

	while (ptr && (g_strcasecmp (ptr->domain, domain) != 0)) {
		rptr = &ptr->next;
		ptr = ptr->next;
	}

	if (ptr && (g_strcasecmp (ptr->domain, domain) == 0)) {
		/* Woohoo... found it. Make sure it hasn't expired */
		if (difftime(time(NULL), ptr->expire) > DNSEXPIRE) {
			/* Oops... expired */
			*rptr = ptr->next;
			g_free (ptr->domain);
			g_free (ptr);
			return -1;
		}

		/* chris - added this so that the routine can be used to just
		 * look stuff up.
		*/
		if (addr)
			*addr = ptr->ipaddr;
		return 0;
	}

	return -1;
}

static int insert(struct in_addr *addr, const char *domain)
{
	unsigned int box = hash(domain, HASH_BOX);
	struct dnscache_s **rptr = &cache[box];
	struct dnscache_s *ptr = cache[box];
	struct dnscache_s *newptr;

	g_assert (addr != NULL);
	g_assert (domain != NULL);

	while (ptr) {
		rptr = &ptr->next;
		ptr = ptr->next;
	}

	newptr = g_new0 (struct dnscache_s, 1);

	if (!(newptr->domain = g_strdup (domain))) {
		g_free (newptr);
		return -1;
	}

	newptr->ipaddr = *addr;

	newptr->expire = time(NULL);

	*rptr = newptr;
	newptr->next = ptr;

	return 0;
}

int dnscache_lookup(struct in_addr *addr, const char *domain)
{
	struct hostent *resolv;

	g_return_val_if_fail (addr != NULL, -1);
	g_return_val_if_fail (domain != NULL, -1);

	if (inet_aton(domain, (struct in_addr *) addr) != 0)
		return 0;

	/* Well, we're not dotted-decimal so we need to look it up */
	if (lookup(addr, domain) == 0)
		return 0;

	/* Okay, so not in the list... need to actually look it up. */
	if (!(resolv = gethostbyname(domain)))
		return -1;

	memcpy(addr, resolv->h_addr_list[0], resolv->h_length);
	insert(addr, domain);

	return 0;
}

static void dnsdelete(unsigned int c, struct dnscache_s *del)
{
	struct dnscache_s **rptr;
	struct dnscache_s *ptr;

	assert(c > 0);
	assert(del);

	rptr = &cache[c];
	ptr = cache[c];

	while (ptr && (ptr != del)) {
		rptr = &ptr->next;
		ptr = ptr->next;
	}

	if (ptr == del) {
		*rptr = ptr->next;
		g_free (ptr->domain);
		g_free (ptr);
	}
}

void dnscache_cleanup(void)
{
	unsigned int c;
	struct dnscache_s *ptr, *tmp;

	for (c = 0; c < HASH_BOX; c++) {
		ptr = cache[c];

		while (ptr) {
			tmp = ptr->next;

			if (difftime(time(NULL), ptr->expire) > DNSEXPIRE)
				dnsdelete(c, ptr);

			ptr = tmp;
		}
	}
}
