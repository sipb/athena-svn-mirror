/* $Id: dnscache.h,v 1.1.1.1 2001-01-16 15:25:51 ghudson Exp $
 *
 * See 'dnscache.c' for a detailed description.
 *
 * Copyright (C) 1999  Robert James Kaes (rjkaes@flarenet.com
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

#ifndef _DNSCACHE_H_
#define _DNSCACHE_H_	1

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int dnscache_lookup(struct in_addr *addr, const char *domain);
extern void dnscache_cleanup(void);

#endif
