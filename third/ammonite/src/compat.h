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

#ifndef _EAZEL_PROXY_COMPAT_H_
#define _EAZEL_PROXY_COMPAT_H_

#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

gboolean uid_from_local_sockaddr_in (const struct sockaddr_in *peer_name, 
	const struct sockaddr_in *our_name, 
	uid_t *p_uid
);

#endif /* _EAZEL_PROXY_COMPAT_H_ */
