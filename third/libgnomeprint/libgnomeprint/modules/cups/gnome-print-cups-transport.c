/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-cups-transport.c: CUPS transport destination
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
 *    Dave Camp <dave@ximian.com>
 *    Chema Celorio <chema@celorio.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Raph Levien <raph@acm.org>
 *
 *  Copyright (C) 1999-2002 Ximian Inc. and authors
 *
 */

#define __GP_TRANSPORT_CUPS_C__

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>

#include <cups/cups.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-transport.h>
#include <libgnomeprint/gnome-print-config.h>

#define GP_TYPE_TRANSPORT_CUPS         (gp_transport_cups_get_type ())
#define GP_TRANSPORT_CUPS(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GP_TYPE_TRANSPORT_CUPS, GPTransportCups))
#define GP_TRANSPORT_CUPS_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), GP_TYPE_TRANSPORT_CUPS, GPTransportCupsClass))
#define GP_IS_TRANSPORT_CUPS(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GP_TYPE_TRANSPORT_CUPS))
#define GP_IS_TRANSPORT_CUPS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GP_TYPE_TRANSPORT_CUPS))
#define GP_TRANSPORT_CUPS_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GP_TYPE_TRANSPORT_CUPS, GPTransportCupsClass))

typedef struct _GPTransportCups GPTransportCups;
typedef struct _GPTransportCupsClass GPTransportCupsClass;

struct _GPTransportCups {
	GnomePrintTransport transport;
	char *temp_file;  /* Path of the temporary file */
	char *printer;    /* The name of the CUPS printer (ID) */
	FILE *file;
};

struct _GPTransportCupsClass {
	GnomePrintTransportClass parent_class;
};

GType gp_transport_cups_get_type (void);


static void gp_transport_cups_class_init (GPTransportCupsClass *klass);
static void gp_transport_cups_init (GPTransportCups *transport);

static void gp_transport_cups_finalize (GObject *object);

static gint gp_transport_cups_construct (GnomePrintTransport *transport);
static gint gp_transport_cups_open (GnomePrintTransport *transport);
static gint gp_transport_cups_close (GnomePrintTransport *transport);
static gint gp_transport_cups_write (GnomePrintTransport *transport, const guchar *buf, gint len);

static gint gp_transport_cups_print_file (GnomePrintTransport *transport, const guchar *filename);

GType gnome_print__transport_get_type (void);

static GnomePrintTransportClass *parent_class = NULL;

GType
gp_transport_cups_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPTransportCupsClass),
			NULL, NULL,
			(GClassInitFunc) gp_transport_cups_class_init,
			NULL, NULL,
			sizeof (GPTransportCups),
			0,
			(GInstanceInitFunc) gp_transport_cups_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_TRANSPORT, "GPTransportCups", &info, 0);
	}
	return type;
}

static void
gp_transport_cups_class_init (GPTransportCupsClass *klass)
{
	GnomePrintTransportClass *transport_class;
	GObjectClass *object_class;

	object_class                = (GObjectClass *) klass;
	transport_class             = (GnomePrintTransportClass *) klass;

	parent_class                = g_type_class_peek_parent (klass);

	object_class->finalize      = gp_transport_cups_finalize;

	transport_class->construct  = gp_transport_cups_construct;
	transport_class->open       = gp_transport_cups_open;
	transport_class->close      = gp_transport_cups_close;
	transport_class->write      = gp_transport_cups_write;

	transport_class->print_file = gp_transport_cups_print_file;
}

static void
gp_transport_cups_init (GPTransportCups *transport)
{
	transport->file = NULL;
	transport->printer = NULL;
	transport->temp_file = NULL;
}

static void
gp_transport_cups_finalize (GObject *object)
{
	GPTransportCups *transport;

	transport = GP_TRANSPORT_CUPS (object);

	if (transport->file != NULL) {
		g_warning ("Destroying GPTransportCups with open file descriptor");
	}

	if (transport->temp_file) {
		g_free (transport->temp_file);
		transport->temp_file = NULL;
	}

	g_free (transport->printer);
	transport->printer = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gp_transport_cups_construct (GnomePrintTransport *gp_transport)
{
	GPTransportCups *transport;
	guchar *printer;

	transport = GP_TRANSPORT_CUPS (gp_transport);

	/* FIXME: this used to be Settings.Transport.Backend.Printer */
	printer = gnome_print_config_get (gp_transport->config, "Printer");
	if (!printer) {
		g_warning ("Could not find \"Settings.Transport.Backend.Printer\"");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	
	transport->printer = printer;
	transport->temp_file = g_build_filename(g_get_tmp_dir (), 
						"gnome-print-cups-XXXXXX", 
						NULL);

	return GNOME_PRINT_OK;
}

static gint
gp_transport_cups_open (GnomePrintTransport *gp_transport)
{
	GPTransportCups *transport;
	gint fd;

	transport = GP_TRANSPORT_CUPS (gp_transport);

	g_return_val_if_fail (transport->temp_file != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	fd = mkstemp (transport->temp_file);
	if (fd < 0) {
		g_warning ("file %s: line %d: Cannot create temporary file", __FILE__, __LINE__);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	transport->file = fdopen (fd, "r+");

	if (transport->file == NULL) {
		g_warning ("Opening file %s for output failed", transport->temp_file);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return GNOME_PRINT_OK;
}

static gint
get_job_options (GnomePrintConfig *config, cups_option_t **options)
{
	gchar *value;
	gint num = 0;

	value = gnome_print_config_get (config, GNOME_PRINT_KEY_PAPER_SOURCE);

	if (value != NULL) {
		num = cupsAddOption ("InputSlot", value, num, options);
		g_free (value);
	}
	
	value = gnome_print_config_get (config, GNOME_PRINT_KEY_HOLD);

	if (value != NULL) {
		num = cupsAddOption ("job-hold-until", value, num, options);
		g_free (value);
	}
	
	return num;
}
	

static gint
gp_transport_cups_close (GnomePrintTransport *gp_transport)
{
	GPTransportCups *transport;
	cups_option_t *options;
	gint num_options;
	char *title;

	transport = GP_TRANSPORT_CUPS (gp_transport);

	g_return_val_if_fail (transport->file != NULL,
			      GNOME_PRINT_ERROR_UNKNOWN);

	if (fclose (transport->file) < 0) {
		g_warning ("Closing output file failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	transport->file = NULL;

	title = gnome_print_config_get (gp_transport->config, 
					GNOME_PRINT_KEY_DOCUMENT_NAME);
	num_options = get_job_options (gp_transport->config, &options);
	
	cupsPrintFile (transport->printer, 
		       transport->temp_file, 
		       title, num_options, options);
	cupsFreeOptions (num_options, options);

	unlink (transport->temp_file);
	g_free (title);
	
	return GNOME_PRINT_OK;
}

static gint
gp_transport_cups_write (GnomePrintTransport *gp_transport, const guchar *buf, gint len)
{
	GPTransportCups *transport;
	gint l;

	transport = GP_TRANSPORT_CUPS (gp_transport);

	g_return_val_if_fail (transport->file != NULL,
			      GNOME_PRINT_ERROR_UNKNOWN);

	l = len;
	while (l > 0) {
		size_t written;
		written = fwrite (buf, sizeof (guchar), len, transport->file);
		if (written < 0) {
			g_warning ("Writing output file failed");
			return GNOME_PRINT_ERROR_UNKNOWN;
		}
		buf += written;
		l -= written;
	}

	return len;
}


static gint
gp_transport_cups_print_file (GnomePrintTransport *gp_transport, const guchar *filename)
{
	GPTransportCups *transport;
	cups_option_t *options;
	gint num_options;
	char *title;

	transport = GP_TRANSPORT_CUPS (gp_transport);

	title = gnome_print_config_get (gp_transport->config, 
					GNOME_PRINT_KEY_DOCUMENT_NAME);
	num_options = get_job_options (gp_transport->config, &options);
	
	cupsPrintFile (transport->printer, 
		       filename, title, 
		       num_options, options);
	cupsFreeOptions (num_options, options);

	g_free (title);
	
	return GNOME_PRINT_OK;
}

GType
gnome_print__transport_get_type (void)
{
	return GP_TYPE_TRANSPORT_CUPS;
}
