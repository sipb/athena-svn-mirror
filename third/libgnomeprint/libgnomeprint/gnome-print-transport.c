/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-transport.c: abstract base class for transport providers
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
 *  Copyright (C) 1999-2001 Ximian, Inc. and authors
 *
 */

#define __GNOME_PRINT_TRANSPORT_C__

#include <string.h>
#include <locale.h>
#include <gmodule.h>
#include "gnome-print.h"
#include "gnome-print-transport.h"

static void gnome_print_transport_class_init (GnomePrintTransportClass *klass);
static void gnome_print_transport_init (GnomePrintTransport *transport);

static void gnome_print_transport_finalize (GObject *object);

static GnomePrintTransport *gnome_print_transport_create (gpointer get_type, GnomePrintConfig *config);

static GObjectClass *parent_class = NULL;

GType
gnome_print_transport_get_type (void)
{
	static GType transport_type = 0;
	if (!transport_type) {
		static const GTypeInfo transport_info = {
			sizeof (GnomePrintTransportClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_transport_class_init,
			NULL, NULL,
			sizeof (GnomePrintTransport),
			0,
			(GInstanceInitFunc) gnome_print_transport_init
		};
		transport_type = g_type_register_static (G_TYPE_OBJECT, "GnomePrintTransport", &transport_info, 0);
	}

	return transport_type;
}

static void
gnome_print_transport_class_init (GnomePrintTransportClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_transport_finalize;
}

static void
gnome_print_transport_init (GnomePrintTransport *transport)
{
	transport->config = NULL;
	transport->opened = FALSE;
}

static void
gnome_print_transport_finalize (GObject *object)
{
	GnomePrintTransport *transport;

	transport = GNOME_PRINT_TRANSPORT (object);

	if (transport->opened) {
		g_warning ("Destroying open transport provider");
	}

	if (transport->config) {
		transport->config = gnome_print_config_unref (transport->config);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

gint
gnome_print_transport_construct (GnomePrintTransport *transport, GnomePrintConfig *config)
{
	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (config != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	g_return_val_if_fail (transport->config == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	transport->config = gnome_print_config_ref (config);

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->construct)
		GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->construct (transport);

	return TRUE;
}

gint
gnome_print_transport_open (GnomePrintTransport *transport)
{
	gint ret;

	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->config != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (!transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->open)
		GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->open (transport);

	if (ret == GNOME_PRINT_OK) {
		transport->opened = TRUE;
	}

	return ret;
}

gint
gnome_print_transport_close (GnomePrintTransport *transport)
{
	gint ret;

	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->close)
		GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->close (transport);

	if (ret == GNOME_PRINT_OK) {
		transport->opened = FALSE;
	}

	return ret;
}

gint
gnome_print_transport_write (GnomePrintTransport *transport, const guchar *buf, gint len)
{
	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (buf != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (len >= 0, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->write)
		return GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->write (transport, buf, len);

	return 0;
}

gint
gnome_print_transport_printf (GnomePrintTransport *transport, const char *format, ...)
{
	va_list arguments;
	const char *loc;
	gchar *buf;
	gint ret;

	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (format != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	loc = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");

	va_start (arguments, format);
	buf = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	ret = GNOME_PRINT_OK;

	gnome_print_transport_write (transport, buf, strlen (buf));

	g_free (buf);

	setlocale (LC_NUMERIC, loc);
	g_free (loc);

	return ret;
}

GnomePrintTransport *
gnome_print_transport_new (GnomePrintConfig *config)
{
	GnomePrintTransport *transport;
	guchar *drivername;
	guchar *modulename;

	g_return_val_if_fail (config != NULL, NULL);

	drivername = gnome_print_config_get (config, "Settings.Transport.Backend");
	if (!drivername) {
		g_warning ("Settings do not specify transport driver");
		return NULL;
	}

	transport = NULL;

	modulename = gnome_print_config_get (config, "Settings.Transport.Backend.Module");
	if (modulename) {
		static GHashTable *modules = NULL;
		GModule *module;
		if (!modules) modules = g_hash_table_new (g_str_hash, g_str_equal);
		module = g_hash_table_lookup (modules, modulename);
		if (!module) {
			gchar *path;
			path = g_module_build_path (GNOME_PRINT_LIBDIR "/transports", modulename);
			module = g_module_open (path, G_MODULE_BIND_LAZY);
			if (module)
				g_hash_table_insert (modules, g_strdup (modulename), module);
			else
				g_warning ("Could not find %s\n", path);
			g_free (path);
		}
		if (module) {
			gpointer get_type;
			if (g_module_symbol (module, "gnome_print__transport_get_type", &get_type)) {
				transport = gnome_print_transport_create (get_type, config);
			} else {
				g_warning ("Missing gnome_print__transport_get_type in %s\n", modulename);
				g_module_close (module);
			}
		} else {
			g_warning ("Cannot open module: %s\n", modulename);
		}
		g_free (modulename);
	} else {
		g_warning ("Unknown transport driver: %s", modulename);
	}

	g_free (drivername);

	return transport;
}

static GnomePrintTransport *
gnome_print_transport_create (gpointer get_type, GnomePrintConfig *config)
{
	GnomePrintTransport *transport;
	GType (* transport_get_type) (void);
	GType type;

	transport_get_type = get_type;

	type = (* transport_get_type) ();
	g_return_val_if_fail (g_type_is_a (type, GNOME_TYPE_PRINT_TRANSPORT), NULL);

	transport = g_object_new (type, NULL);
	gnome_print_transport_construct (transport, config);

	return transport;
}

