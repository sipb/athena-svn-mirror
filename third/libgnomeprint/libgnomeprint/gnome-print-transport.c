/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-transport.c: Abstract base class for transport providers
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
 *    Lauris Kaplinski <lauris@helixcode.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#define GNOME_PRINT_UNSTABLE_API

#include <config.h>
#include <string.h>
#include <locale.h>

#include <gmodule.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-transport.h>
#include <stdio.h>

static void gnome_print_transport_class_init (GnomePrintTransportClass *klass);
static void gnome_print_transport_init (GnomePrintTransport *transport);
static void gnome_print_transport_finalize (GObject *object);
static gint gnome_print_transport_real_print_file (GnomePrintTransport *transport, const guchar *file_name);

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

	object_class           = (GObjectClass*) klass;

	parent_class           = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_transport_finalize;

	klass->print_file      = gnome_print_transport_real_print_file;
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
	GnomePrintReturnCode retval = GNOME_PRINT_ERROR_UNKNOWN;
	
	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (config != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->config == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	transport->config = gnome_print_config_ref (config);

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->construct)
		retval = GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->construct (transport);

	return retval;
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
		ret = GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->open (transport);

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
		ret = GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->close (transport);

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

/* Note "format" should be locale independent, so it should not use %g */
/* and friends */
gint
gnome_print_transport_printf (GnomePrintTransport *transport, const char *format, ...)
{
	va_list arguments;
	gchar *buf;
	gint ret;

	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (format != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	va_start (arguments, format);
	buf = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	ret = GNOME_PRINT_OK;

	gnome_print_transport_write (transport, buf, strlen (buf));

	g_free (buf);

	return ret;
}


gint
gnome_print_transport_print_file (GnomePrintTransport *transport, const guchar *file_name)
{
	int ret;
	
	g_return_val_if_fail (transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (file_name != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_TRANSPORT (transport), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->config != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (!transport->opened, GNOME_PRINT_ERROR_UNKNOWN);

	ret = GNOME_PRINT_OK;

	if (GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->print_file)
		ret = GNOME_PRINT_TRANSPORT_GET_CLASS (transport)->print_file (transport, file_name);

	return ret;
}


static GnomePrintTransport *
gnome_print_transport_new_from_module_name (const gchar *module_name, GnomePrintConfig *config)
{
	static GHashTable *modules;
	GnomePrintTransport *transport;
	gpointer get_type;
	GModule *module;
	gchar *path = NULL;
	gboolean insert = FALSE;

	if (!modules)
		modules = g_hash_table_new (g_str_hash, g_str_equal);
	module = g_hash_table_lookup (modules, module_name);

	if (!module) {
		insert = TRUE;
		path = g_module_build_path (GNOME_PRINT_MODULES_DIR "/transports", module_name);
		module = g_module_open (path, G_MODULE_BIND_LAZY);
	}

	if (!module) {
		insert = TRUE;
		g_free (path);
		path = g_module_build_path (GNOME_PRINT_MODULES_DIR, module_name);
		module = g_module_open (path, G_MODULE_BIND_LAZY);
	}
	
	if (!module) {
		g_warning ("Could not open %s\n", path);
		g_free (path);
		return NULL;
	}

	if (insert)
		g_hash_table_insert (modules, g_strdup (module_name), module);

	if (!g_module_symbol (module, "gnome_print__transport_get_type", &get_type)) {
		g_warning ("Missing gnome_print__transport_get_type in %s\n", path);
		g_module_close (module);
		if (path)
			g_free (path);
		return NULL;
	}

	transport = gnome_print_transport_create (get_type, config);
	if (transport == NULL) {
		g_warning ("Could not create transport in %s\n", path);
		g_module_close (module);
		if (path)
			g_free (path);
		return NULL;
	}

	if (path)
		g_free (path);
	
	return transport;
}

GnomePrintTransport *
gnome_print_transport_new (GnomePrintConfig *config)
{
	GnomePrintTransport *transport = NULL;
	guchar *module_name = NULL;
	gint print_to_file = FALSE;

	g_return_val_if_fail (config != NULL, NULL);

	gnome_print_config_get_boolean (config, "Settings.Output.Job.PrintToFile", &print_to_file);

	if (print_to_file) {
		module_name = g_strdup ("libgnomeprint-file.so");
	} else {
		module_name = gnome_print_config_get (config, "Settings.Transport.Backend.Module");
		if (!module_name) {
			g_warning ("Could not find \"Settings.Transport.Backend.Module\" using default");
			module_name = g_strdup ("libgnomeprint-lpr.so");
		}
	} 

	transport = gnome_print_transport_new_from_module_name (module_name, config);

	g_free (module_name);

	return transport;
}

static GnomePrintTransport *
gnome_print_transport_create (gpointer get_type, GnomePrintConfig *config)
{
	GnomePrintTransport *transport;
	GType (* transport_get_type) (void);
	GType type;
	GnomePrintReturnCode retval;

	transport_get_type = get_type;

	type = (* transport_get_type) ();
	g_return_val_if_fail (g_type_is_a (type, GNOME_TYPE_PRINT_TRANSPORT), NULL);

	transport = g_object_new (type, NULL);
	retval = gnome_print_transport_construct (transport, config);

	if (retval != GNOME_PRINT_OK) {
		g_warning ("Error while constructing transport inside transport_create");
		g_object_unref (G_OBJECT (transport));
		return NULL;
	}

	return transport;
}


#define BLOCK_SIZE (1024)

static gint
gnome_print_transport_real_print_file (GnomePrintTransport *transport, const guchar *file_name)
{
	FILE *input;
	char buffer[1024];
	int retval;

	input = fopen (file_name, "rb");

	if (input) {
		int count;

		gnome_print_transport_open (transport);

		while ((count = fread (buffer, 1, BLOCK_SIZE, input))) {
			retval = gnome_print_transport_write (transport, buffer, count);

			if (retval != count) {
				fclose(input);
				return retval;
			}
		}
	}

	fclose(input);
	retval = gnome_print_transport_close (transport);
	
	return retval;
}
