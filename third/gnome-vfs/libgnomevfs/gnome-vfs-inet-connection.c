/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-inet-connection.c - Functions for creating and destroying Internet
   connections.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
extern int h_errno;

#include <glib.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"


struct GnomeVFSInetConnection {
	struct sockaddr_in addr;
	guint sock;
};


GnomeVFSResult
gnome_vfs_inet_connection_create (GnomeVFSInetConnection **connection_return,
				  const gchar *host_name,
				  guint host_port,
				  GnomeVFSCancellation *cancellation)
{
	GnomeVFSInetConnection *new;
	struct hostent *host_info;
	struct sockaddr_in addr;
	gint sock;

	g_return_val_if_fail (connection_return != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (host_name != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (host_port != 0, GNOME_VFS_ERROR_BAD_PARAMETERS);

	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return gnome_vfs_result_from_errno ();

	host_info = gethostbyname (host_name);
	if (gnome_vfs_cancellation_check (cancellation))
		return GNOME_VFS_ERROR_CANCELLED;

	if (host_info == NULL)
		return gnome_vfs_result_from_h_errno ();

	addr.sin_family = host_info->h_addrtype;
	addr.sin_addr = * (struct in_addr *) host_info->h_addr;
	addr.sin_port = htons (host_port);

	if (connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
		return gnome_vfs_result_from_errno ();

	new = g_new (GnomeVFSInetConnection, 1);
	memcpy (&new->addr, &addr, sizeof (addr));
	new->sock = sock;

	*connection_return = new;

	return GNOME_VFS_OK;
}


void
gnome_vfs_inet_connection_destroy (GnomeVFSInetConnection *connection,
				   GnomeVFSCancellation *cancellation)
{
	g_return_if_fail (connection != NULL);

	close (connection->sock);
	g_free (connection);
}

GnomeVFSIOBuf *
gnome_vfs_inet_connection_get_iobuf (GnomeVFSInetConnection *connection)
{
	g_return_val_if_fail (connection != NULL, NULL);

	return gnome_vfs_iobuf_new (connection->sock);
}
