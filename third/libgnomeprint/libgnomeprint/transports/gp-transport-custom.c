/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-transport-custom.c:
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
 *    Carlos Perello Marin <carlos@gnome-db.org>
 *
 *  Copyright (C) 1999-2001 Ximian Inc. and authors
 *
 */

#define __GP_TRANSPORT_CUSTOM_C__

#include "config.h"
#include <libgnomeprint/gnome-print.h>
#include "gp-transport-custom.h"

static void gp_transport_custom_class_init (GPTransportCustomClass *klass);
static void gp_transport_custom_init (GPTransportCustom *tcustom);

static void gp_transport_custom_finalize (GObject *object);

static gint gp_transport_custom_construct (GnomePrintTransport *transport);
static gint gp_transport_custom_open (GnomePrintTransport *transport);
static gint gp_transport_custom_close (GnomePrintTransport *transport);
static gint gp_transport_custom_write (GnomePrintTransport *transport, const guchar *buf, gint len);

GType gnome_print__transport_get_type (void);

static GnomePrintTransportClass *parent_class = NULL;

GType
gp_transport_custom_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPTransportCustomClass),
			NULL, NULL,
			(GClassInitFunc) gp_transport_custom_class_init,
			NULL, NULL,
			sizeof (GPTransportCustom),
			0,
			(GInstanceInitFunc) gp_transport_custom_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_TRANSPORT, "GPTransportCustom", &info, 0);
	}
	return type;
}

static void
gp_transport_custom_class_init (GPTransportCustomClass *klass)
{
	GObjectClass *object_class;
	GnomePrintTransportClass *transport_class;

	object_class = (GObjectClass *) klass;
	transport_class = (GnomePrintTransportClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gp_transport_custom_finalize;

	transport_class->construct = gp_transport_custom_construct;
	transport_class->open = gp_transport_custom_open;
	transport_class->close = gp_transport_custom_close;
	transport_class->write = gp_transport_custom_write;
}

static void
gp_transport_custom_init (GPTransportCustom *tcustom)
{
	tcustom->command = NULL;
	tcustom->pipe = NULL;
}

static void
gp_transport_custom_finalize (GObject *object)
{
	GPTransportCustom *tcustom;

	tcustom = GP_TRANSPORT_CUSTOM (object);

	if (tcustom->pipe) {
		g_warning ("Destroying GnomePrintTransportCustom with open pipe");
		pclose (tcustom->pipe);
		tcustom->pipe = NULL;
	}

	if (tcustom->command) {
		g_free (tcustom->command);
		tcustom->command = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gp_transport_custom_construct (GnomePrintTransport *transport)
{
	GPTransportCustom *tcustom;
	guchar *value;

	tcustom = GP_TRANSPORT_CUSTOM (transport);

	value = gnome_print_config_get (transport->config, "Settings.Transport.Backend.Command");

	if (value && *value) {
		tcustom->command = value;
	}

	return GNOME_PRINT_OK;
}

static gint
gp_transport_custom_open (GnomePrintTransport *transport)
{
	GPTransportCustom *tcustom;
	guchar *command;

	tcustom = GP_TRANSPORT_CUSTOM (transport);

	if (tcustom->command) {
		command = g_strdup (tcustom->command);
	} else {
		/* FIXME: Should we use other default? */
		command = g_strdup ("lpr");
	}

	tcustom->pipe = popen (command, "w");

	if (tcustom->pipe == NULL) {
		g_warning ("Opening '%s' for output failed", command);
		g_free (command);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	g_free (command);
	return GNOME_PRINT_OK;
}

static gint
gp_transport_custom_close (GnomePrintTransport *transport)
{
	GPTransportCustom *tcustom;

	tcustom = GP_TRANSPORT_CUSTOM (transport);

	g_return_val_if_fail (tcustom->pipe != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	if (pclose (tcustom->pipe) < 0) {
		g_warning ("Closing output pipe failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	tcustom->pipe = NULL;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_custom_write (GnomePrintTransport *transport, const guchar *buf, gint len)
{
	GPTransportCustom *tcustom;
	size_t written;

	tcustom = GP_TRANSPORT_CUSTOM (transport);

	g_return_val_if_fail (tcustom->pipe != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	written = fwrite (buf, sizeof (guchar), len, tcustom->pipe);

	if (written < 0) {
		g_warning ("Writing output pipe failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return len;
}

GType
gnome_print__transport_get_type (void)
{
	return GP_TYPE_TRANSPORT_CUSTOM;
}

