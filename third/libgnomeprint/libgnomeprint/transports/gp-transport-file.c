/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-transport-file.c: FILE transport destination
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Raph Levien <raph@acm.org>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright (C) 1999-2001 Ximian Inc. and authors
 *
 */

#define __GP_TRANSPORT_FILE_C__

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgnomeprint/gnome-print.h>

#include "gp-transport-file.h"

static void gp_transport_file_class_init (GPTransportFileClass *klass);
static void gp_transport_file_init (GPTransportFile *tf);

static void gp_transport_file_finalize (GObject *object);

static gint gp_transport_file_construct (GnomePrintTransport *transport);
static gint gp_transport_file_open (GnomePrintTransport *transport);
static gint gp_transport_file_close (GnomePrintTransport *transport);
static gint gp_transport_file_write (GnomePrintTransport *transport, const guchar *buf, gint len);

GType gnome_print__transport_get_type (void);

static GnomePrintTransportClass *parent_class = NULL;

GType
gp_transport_file_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPTransportFileClass),
			NULL, NULL,
			(GClassInitFunc) gp_transport_file_class_init,
			NULL, NULL,
			sizeof (GPTransportFile),
			0,
			(GInstanceInitFunc) gp_transport_file_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_TRANSPORT, "GPTransportFile", &info, 0);
	}
	return type;
}

static void
gp_transport_file_class_init (GPTransportFileClass *klass)
{
	GObjectClass *object_class;
	GnomePrintTransportClass *transport_class;

	object_class = (GObjectClass *) klass;
	transport_class = (GnomePrintTransportClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gp_transport_file_finalize;

	transport_class->construct = gp_transport_file_construct;
	transport_class->open = gp_transport_file_open;
	transport_class->close = gp_transport_file_close;
	transport_class->write = gp_transport_file_write;
}

static void
gp_transport_file_init (GPTransportFile *tf)
{
	tf->name = NULL;
	tf->fd = -1;
}

static void
gp_transport_file_finalize (GObject *object)
{
	GPTransportFile *tf;

	tf = GP_TRANSPORT_FILE (object);

	if (tf->fd != -1) {
		g_warning ("Destroying GPTransportFile with open file descriptor");
	}

	if (tf->name) {
		g_free (tf->name);
		tf->name = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gp_transport_file_construct (GnomePrintTransport *transport)
{
	GPTransportFile *tf;
	guchar *value;

	tf = GP_TRANSPORT_FILE (transport);

	value = gnome_print_config_get (transport->config, "Settings.Transport.Backend.FileName");

	if (!value) {
		g_warning ("Configuration does not specify filename");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
		
	tf->name = value;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_file_open (GnomePrintTransport *transport)
{
	GPTransportFile *tf;

	tf = GP_TRANSPORT_FILE (transport);

	g_return_val_if_fail (tf->name != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	tf->fd = open (tf->name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if (tf->fd < 0) {
		g_warning ("Opening file %s for output failed", tf->name);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return GNOME_PRINT_OK;
}

static gint
gp_transport_file_close (GnomePrintTransport *transport)
{
	GPTransportFile *tf;

	tf = GP_TRANSPORT_FILE (transport);

	g_return_val_if_fail (tf->fd >= 0, GNOME_PRINT_ERROR_UNKNOWN);

	if (close (tf->fd) < 0) {
		g_warning ("Closing output file failed [%s]", tf->name);
		tf->fd = -1;
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	tf->fd = -1;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_file_write (GnomePrintTransport *transport, const guchar *buf, gint len)
{
	GPTransportFile *tf;
	gint l;

	tf = GP_TRANSPORT_FILE (transport);

	g_return_val_if_fail (tf->fd >= 0, GNOME_PRINT_ERROR_UNKNOWN);

	l = len;
	while (l > 0) {
		size_t written;
		written = write (tf->fd, buf, len);
		if (written < 0) {
			g_warning ("Writing output file failed");
			return GNOME_PRINT_ERROR_UNKNOWN;
		}
		buf += written;
		l -= written;
	}

	return len;
}

GType
gnome_print__transport_get_type (void)
{
	return GP_TYPE_TRANSPORT_FILE;
}
