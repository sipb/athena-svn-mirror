/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-transport-lpr.c:
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

#define __GP_TRANSPORT_LPR_C__

#include "config.h"
#include <libgnomeprint/gnome-print.h>
#include <gmodule.h>

#include "gp-transport-lpr.h"

#if !defined(HAVE_POPEN)
#ifdef G_OS_WIN32
#include <stdio.h>
#define popen(f,m) _popen(f,m)
#endif
#endif

#if !defined(HAVE_CLOSE)
#ifdef G_OS_WIN32
#include <stdio.h>
#define pclose(f) _pclose(f)
#endif
#endif

static void gp_transport_lpr_class_init (GPTransportLPRClass *klass);
static void gp_transport_lpr_init (GPTransportLPR *tlpr);

static void gp_transport_lpr_finalize (GObject *object);

static gint gp_transport_lpr_construct (GnomePrintTransport *transport);
static gint gp_transport_lpr_open (GnomePrintTransport *transport);
static gint gp_transport_lpr_close (GnomePrintTransport *transport);
static gint gp_transport_lpr_write (GnomePrintTransport *transport, const guchar *buf, gint len);
static gboolean gp_transport_lpr_exists (GnomePrintTransport const *transport);

G_MODULE_EXPORT GType gnome_print__transport_get_type (void);

static GnomePrintTransportClass *parent_class = NULL;

GType
gp_transport_lpr_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPTransportLPRClass),
			NULL, NULL,
			(GClassInitFunc) gp_transport_lpr_class_init,
			NULL, NULL,
			sizeof (GPTransportLPR),
			0,
			(GInstanceInitFunc) gp_transport_lpr_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_TRANSPORT, "GPTransportLPR", &info, 0);
	}
	return type;
}

static void
gp_transport_lpr_class_init (GPTransportLPRClass *klass)
{
	GObjectClass *object_class;
	GnomePrintTransportClass *transport_class;

	object_class = (GObjectClass *) klass;
	transport_class = (GnomePrintTransportClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gp_transport_lpr_finalize;

	transport_class->construct = gp_transport_lpr_construct;
	transport_class->open = gp_transport_lpr_open;
	transport_class->close = gp_transport_lpr_close;
	transport_class->write = gp_transport_lpr_write;
	transport_class->exists = gp_transport_lpr_exists;
}

static void
gp_transport_lpr_init (GPTransportLPR *tlpr)
{
	tlpr->printer = NULL;
	tlpr->pipe = NULL;
}

static void
gp_transport_lpr_finalize (GObject *object)
{
	GPTransportLPR *tlpr;

	tlpr = GP_TRANSPORT_LPR (object);

	if (tlpr->pipe) {
		g_warning ("Destroying GnomePrintTransportLPR with open pipe");
		pclose (tlpr->pipe);
		tlpr->pipe = NULL;
	}

	if (tlpr->printer) {
		g_free (tlpr->printer);
		tlpr->printer = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gp_transport_lpr_construct (GnomePrintTransport *transport)
{
	GPTransportLPR *tlpr;
	guchar *value;

	tlpr = GP_TRANSPORT_LPR (transport);

	value = gnome_print_config_get (transport->config, "Settings.Transport.Backend.Printer");

	if (value && *value) {
		tlpr->printer = value;
	}

	return GNOME_PRINT_OK;
}

static char *
find_lpr (void)
{
	gchar *lpr_path = g_find_program_in_path ("lpr");
	const gchar *candidate;

	/* This is the Solaris location, possibly not in PATH.  */
	candidate = "/usr/ucb/lpr";
	if (lpr_path == NULL && g_file_test (candidate, G_FILE_TEST_IS_EXECUTABLE))
		return g_strdup (candidate);
	return lpr_path;
}

static gint
gp_transport_lpr_open (GnomePrintTransport *transport)
{
	GPTransportLPR *tlpr = GP_TRANSPORT_LPR (transport);
	gchar *command;
	char *lpr_path = find_lpr ();

	if (lpr_path == NULL)
		return GNOME_PRINT_ERROR_UNKNOWN;

	if (tlpr->printer) {
		/* FIXME: verify printer name?  */
		command = g_strdup_printf ("%s '-P%s'", lpr_path, tlpr->printer);
		g_free (lpr_path);
	} else {
		command = lpr_path;
		lpr_path = NULL;
	}

	tlpr->pipe = popen (command, "w");

	if (tlpr->pipe == NULL) {
		g_warning ("Opening '%s' for output failed", command);
		g_free (command);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	
	g_free (command);
	
	return GNOME_PRINT_OK;
}

static gint
gp_transport_lpr_close (GnomePrintTransport *transport)
{
	GPTransportLPR *tlpr;

	tlpr = GP_TRANSPORT_LPR (transport);

	g_return_val_if_fail (tlpr->pipe != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	if (pclose (tlpr->pipe) < 0) {
		g_warning ("Closing output pipe failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	tlpr->pipe = NULL;

	return GNOME_PRINT_OK;
}

static gint
gp_transport_lpr_write (GnomePrintTransport *transport, const guchar *buf, gint len)
{
	GPTransportLPR *tlpr;
	size_t written;

	tlpr = GP_TRANSPORT_LPR (transport);

	g_return_val_if_fail (tlpr->pipe != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	written = fwrite (buf, sizeof (guchar), len, tlpr->pipe);

	if (written < len) {
		g_warning ("Writing output pipe failed");
		return GNOME_PRINT_ERROR_UNKNOWN;
	}

	return len;
}

static gboolean
gp_transport_lpr_exists (GnomePrintTransport const *transport)
{
	char *lpr_path = find_lpr ();
	if (lpr_path == NULL)
		return FALSE;
	g_free (lpr_path);
	return TRUE;
}

G_MODULE_EXPORT GType
gnome_print__transport_get_type (void)
{
	return GP_TYPE_TRANSPORT_LPR;
}

