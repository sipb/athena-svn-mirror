/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Process all sorts of incoming URIs
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <aconf.h>
#include "gpdf-uri-input.h"
#include "eel-vfs-extensions.h"

#include <libgnome/gnome-macros.h>
#include <libgnomevfs/gnome-vfs-uri.h>

#define PARENT_TYPE G_TYPE_OBJECT
GNOME_CLASS_BOILERPLATE (GPdfURIInput, gpdf_uri_input,
			 GObject, PARENT_TYPE)

struct _GPdfURIInputPrivate {
	GPdfRecentFacade *recent_facade;
};

enum {
	OPEN_REQUEST_SIGNAL,
	LAST_SIGNAL
};

static guint gpdf_uri_input_signals [LAST_SIGNAL];

void
gpdf_uri_input_open_uri (GPdfURIInput *uri_in, const char *uri)
{
	if (uri_in->priv->recent_facade)
		gpdf_recent_facade_add_uri (uri_in->priv->recent_facade, uri);
	g_signal_emit (G_OBJECT (uri_in),
		       gpdf_uri_input_signals [OPEN_REQUEST_SIGNAL],
		       0, uri);
}

static void
open_vfs_uri_with_uri_in (GnomeVFSURI *vfs_uri, GPdfURIInput *uri_in)
{
	gpdf_uri_input_open_uri (
		uri_in,
		gnome_vfs_uri_to_string (vfs_uri, GNOME_VFS_URI_HIDE_NONE));
}

void
gpdf_uri_input_open_uri_glist (GPdfURIInput *uri_in, GList *uri_glist)
{
	g_list_foreach (uri_glist, (GFunc)open_vfs_uri_with_uri_in, uri_in);
}

void
gpdf_uri_input_open_uri_list (GPdfURIInput *uri_in, const char *uri_list)
{
	GList *uri_glist = NULL;

	uri_glist = gnome_vfs_uri_list_parse (uri_list);
	gpdf_uri_input_open_uri_glist (uri_in, uri_glist);
	gnome_vfs_uri_list_unref (uri_glist);
	g_list_free (uri_glist);
}

void
gpdf_uri_input_open_shell_arg    (GPdfURIInput *uri_in, const char *location)
{
	char *uri;

	uri = eel_make_uri_from_shell_arg (location);
	g_assert (uri != NULL);
	gpdf_uri_input_open_uri (uri_in, uri);
	g_free (uri);
}

void
gpdf_uri_input_set_recent_facade (GPdfURIInput *uri_in,
				  GPdfRecentFacade *recent)
{
	if (uri_in->priv->recent_facade)
		g_object_unref (G_OBJECT (uri_in->priv->recent_facade));
	g_object_ref (G_OBJECT (recent));
	uri_in->priv->recent_facade = recent;
}

static void
gpdf_uri_input_instance_init (GPdfURIInput *uri_in)
{
	uri_in->priv = g_new0 (GPdfURIInputPrivate, 1);
}

static void
gpdf_uri_input_class_init (GPdfURIInputClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gpdf_uri_input_signals [OPEN_REQUEST_SIGNAL] = g_signal_new (
		"open_request",
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfURIInputClass, open_request),
		NULL, NULL,
		g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);
}
