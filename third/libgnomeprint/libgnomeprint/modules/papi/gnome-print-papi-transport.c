/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-papi-transport.c: PAPI transport destination
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
 *    Danek Duvall <danek.duvall@sun.com>
 *
 *  Copyright (C) 1999-2002 Ximian Inc. and authors
 *  Copyright 2004 Sun Microsystems, Inc.
 *
 */

#define __GP_TRANSPORT_PAPI_C__

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <papi.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-transport.h>

#define GP_TYPE_TRANSPORT_PAPI         (gp_transport_papi_get_type ())
#define GP_TRANSPORT_PAPI(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GP_TYPE_TRANSPORT_PAPI, GPTransportPAPI))
#define GP_TRANSPORT_PAPI_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), GP_TYPE_TRANSPORT_PAPI, GPTransportPAPIClass))
#define GP_IS_TRANSPORT_PAPI(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GP_TYPE_TRANSPORT_PAPI))
#define GP_IS_TRANSPORT_PAPI_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GP_TYPE_TRANSPORT_PAPI))
#define GP_TRANSPORT_PAPI_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GP_TYPE_TRANSPORT_PAPI, GPTransportPAPIClass))

typedef struct _GPTransportPAPI GPTransportPAPI;
typedef struct _GPTransportPAPIClass GPTransportPAPIClass;

struct _GPTransportPAPI {
	GnomePrintTransport transport;
	char *printer;
	papi_service_t service;
	papi_stream_t stream;
	papi_attribute_t **attributes;
};

struct _GPTransportPAPIClass {
	GnomePrintTransportClass parent_class;
};

GType gp_transport_papi_get_type (void);


static void gp_transport_papi_class_init (GPTransportPAPIClass *klass);
static void gp_transport_papi_init (GPTransportPAPI *transport);

static void gp_transport_papi_finalize (GObject *object);

static gint gp_transport_papi_construct (GnomePrintTransport *transport);
static gint gp_transport_papi_open (GnomePrintTransport *transport);
static gint gp_transport_papi_close (GnomePrintTransport *transport);
static gint gp_transport_papi_write (GnomePrintTransport *transport, const guchar *buf, gint len);

static gint gp_transport_papi_print_file (GnomePrintTransport *transport, const guchar *filename);

GType gnome_print__transport_get_type (void);

static GnomePrintTransportClass *parent_class = NULL;

GType
gp_transport_papi_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPTransportPAPIClass),
			NULL, NULL,
			(GClassInitFunc) gp_transport_papi_class_init,
			NULL, NULL,
			sizeof (GPTransportPAPI),
			0,
			(GInstanceInitFunc) gp_transport_papi_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_TRANSPORT,
			"GPTransportPAPI", &info, 0);
	}
	return type;
}

static void
gp_transport_papi_class_init (GPTransportPAPIClass *klass)
{
	GnomePrintTransportClass *transport_class;
	GObjectClass *object_class;

	object_class                = (GObjectClass *) klass;
	transport_class             = (GnomePrintTransportClass *) klass;

	parent_class                = g_type_class_peek_parent (klass);

	object_class->finalize      = gp_transport_papi_finalize;

	transport_class->construct  = gp_transport_papi_construct;
	transport_class->open       = gp_transport_papi_open;
	transport_class->close      = gp_transport_papi_close;
	transport_class->write      = gp_transport_papi_write;

	transport_class->print_file = gp_transport_papi_print_file;
}

static void
gp_transport_papi_init (GPTransportPAPI *transport)
{
	transport->printer = NULL;
	transport->service = NULL;
	transport->stream = NULL;
	transport->attributes = NULL;
}

static void
gp_transport_papi_finalize (GObject *object)
{
	GPTransportPAPI *transport;

	transport = GP_TRANSPORT_PAPI (object);

	if (transport->stream != NULL) {
		/*
		 * What does g_warning do?  Do we want it everywhere we have a
		 * failure?
		 */
		g_warning ("Destroying GPTransportPAPI with open PAPI stream");
	}

	g_assert (transport->printer);
	g_free (transport->printer);
	transport->printer = NULL;

	papiAttributeListFree (transport->attributes);
	transport->attributes = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gp_transport_papi_construct (GnomePrintTransport *gp_transport)
{
	GPTransportPAPI *transport;
	char *value;
	long valint;
	papi_status_t status;
	papi_service_t service = NULL;
	papi_attribute_t **attributes = NULL;

	transport = GP_TRANSPORT_PAPI (gp_transport);

	/* gnome_print_config_dump (gp_transport->config); */

	/* FIXME: this used to be Settings.Transport.Backend.Printer */
	value = (char *)gnome_print_config_get (gp_transport->config,
		(unsigned char *)"Printer");
	if (!value) {
		g_warning ("Could not find \"Settings.Transport.Backend.Printer\"");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	transport->printer = value;

	status = papiServiceCreate (&service, transport->printer, NULL, NULL,
		NULL, PAPI_ENCRYPT_NEVER, NULL);
	if (status != PAPI_OK) {
		g_warning ("Could not create PAPI service");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	transport->service = service;

	value = (char *)gnome_print_config_get (gp_transport->config,
		(unsigned char *)GNOME_PRINT_KEY_NUM_COPIES);
	errno = 0;
	valint = strtol ((char *)value, NULL, 10);
	if (errno == 0)
		papiAttributeListAddInteger (&attributes, PAPI_ATTR_EXCL,
			"copies", valint);
	else
		papiAttributeListAddInteger (&attributes, PAPI_ATTR_EXCL,
			"copies", 1);
	g_free (value);

	/*
	 * We really ought to pull this value from somewhere.  It probably will
	 * be "application/postscript" most of the time, but at least with
	 * "application/octet-stream" (raw data), the queue should do the right
	 * thing.
	 */
	papiAttributeListAddString (&attributes, PAPI_ATTR_EXCL,
		"document-format", "application/octet-stream");
	/*
	 * Always print burst pages; GNOME doesn't have an attribute to query
	 * for it.
	 */
	papiAttributeListAddString (&attributes, PAPI_ATTR_EXCL,
		"job-sheets", "standard");

	/*
	 * How does PaperSource translate into an IPP attribute?  I assume that
	 * this is the input tray, but it's not in gnome-print-config.h.  This
	 * is a holdover from the CUPS module, which uses InputSlot.
	 */
	/*
	value = gnome_print_config_get (gp_transport->config,
		"Settings.Output.PaperSource");
	papiAttributeListAddString (&attributes, PAPI_ATTR_EXCL, "???", value);
	g_free (value);
	*/

	/*
	 * lpsched can't deal with bad media values; printing to a remote
	 * printer via lpd will simply fail.
	 */
	/*
	value = (char *)gnome_print_config_get (gp_transport->config,
		(unsigned char *)GNOME_PRINT_KEY_PAPER_SIZE);
	papiAttributeListAddString (&attributes, PAPI_ATTR_EXCL,
		"media", value);
	g_free (value);
	*/

	value = (char *)gnome_print_config_get (gp_transport->config,
		(unsigned char *)GNOME_PRINT_KEY_DOCUMENT_NAME);
	if (value != NULL && value[0] != '\0')
		papiAttributeListAddString (&attributes, PAPI_ATTR_EXCL,
			"job-name", value);
	g_free (value);

	transport->attributes = attributes;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_papi_open (GnomePrintTransport *gp_transport)
{
	GPTransportPAPI *transport;
	papi_status_t status;
	papi_stream_t stream = NULL;

	transport = GP_TRANSPORT_PAPI (gp_transport);

	status = papiJobStreamOpen (transport->service, transport->printer,
		(const papi_attribute_t **)transport->attributes,
		NULL, &stream);

	if (status != PAPI_OK)
		return GNOME_PRINT_ERROR_UNKNOWN;

	transport->stream = stream;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_papi_close (GnomePrintTransport *gp_transport)
{
	GPTransportPAPI *transport;
	papi_job_t job = NULL;
	papi_status_t status;

	transport = GP_TRANSPORT_PAPI (gp_transport);

	g_return_val_if_fail (transport->service != NULL,
		GNOME_PRINT_ERROR_UNKNOWN);

	status = papiJobStreamClose (transport->service,
		transport->stream, &job);
	/* Check to see if the job matches what we sent (in terms of attributes) */

	papiJobFree(job);

	if (status != PAPI_OK) {
		g_warning ("Closing PAPI stream failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	transport->stream = NULL;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_papi_write (GnomePrintTransport *gp_transport, const guchar *buf, gint len)
{
	GPTransportPAPI *transport;
	papi_status_t status;

	transport = GP_TRANSPORT_PAPI (gp_transport);

	g_return_val_if_fail (transport->service != NULL,
		GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (transport->stream != NULL,
		GNOME_PRINT_ERROR_UNKNOWN);

	status = papiJobStreamWrite (transport->service, transport->stream,
		buf, len);
	if (status != PAPI_OK) {
		g_warning ("Writing output stream failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return len;
}


static gint
gp_transport_papi_print_file (GnomePrintTransport *gp_transport, const guchar *filename)
{
	GPTransportPAPI *transport;
	papi_status_t status;
	papi_job_t job = NULL;

	transport = GP_TRANSPORT_PAPI (gp_transport);

	status = papiJobSubmit (transport->service, transport->printer,
		(const papi_attribute_t **)transport->attributes, NULL,
		(const char **)&filename, &job);

	papiJobFree(job);

	if (status != PAPI_OK)
		return GNOME_PRINT_ERROR_UNKNOWN;

	return GNOME_PRINT_OK;
}

GType
gnome_print__transport_get_type (void)
{
	return GP_TYPE_TRANSPORT_PAPI;
}
