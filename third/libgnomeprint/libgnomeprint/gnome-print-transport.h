/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-transport.h: Abstract base class for transport providers
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
 *    Raph Levien (raph@acm.org)
 *    Miguel de Icaza (miguel@kernel.org)
 *    Lauris Kaplinski <lauris@helixcode.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PRINT_TRANSPORT_H__
#define __GNOME_PRINT_TRANSPORT_H__

#include <glib.h>
#include <libgnomeprint/gnome-print-config.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_TRANSPORT         (gnome_print_transport_get_type ())
#define GNOME_PRINT_TRANSPORT(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransport))
#define GNOME_PRINT_TRANSPORT_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransportClass))
#define GNOME_IS_PRINT_TRANSPORT(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_TRANSPORT))
#define GNOME_IS_PRINT_TRANSPORT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GNOME_TYPE_PRINT_TRANSPORT))
#define GNOME_PRINT_TRANSPORT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransportClass))

typedef struct _GnomePrintTransport      GnomePrintTransport;
typedef struct _GnomePrintTransportClass GnomePrintTransportClass;

struct _GnomePrintTransport {
	   GObject object;
	   GnomePrintConfig *config;
	   guint opened : 1;	/* TRUE if it has not been _closed */
	   GError	*err;	/* Contains the first error that can not be handled */
};

struct _GnomePrintTransportClass {
	   GObjectClass parent_class;
	   
	   gint (* construct)  (GnomePrintTransport *transport);
	   gint (* open)       (GnomePrintTransport *transport);
	   gint (* close)      (GnomePrintTransport *transport);
	   gint (* write)      (GnomePrintTransport *transport, const guchar *buf, gint len);
	   gint (* print_file) (GnomePrintTransport *transport, const guchar *file_name);

	/* Run time check to see if the transport makes sense (eg lpr on win32) */
	gboolean (* exists) (GnomePrintTransport const *transport);

	/* Padding for future expansion */
	void (*_gp_reserved0) (void);
	void (*_gp_reserved1) (void);
	void (*_gp_reserved2) (void);
	void (*_gp_reserved3) (void);
};

GType                 gnome_print_transport_get_type (void);
GnomePrintTransport * gnome_print_transport_new (GnomePrintConfig *config);
gboolean	      gnome_print_transport_exists_by_name (const char *name);

gint gnome_print_transport_open      (GnomePrintTransport *transport);
gint gnome_print_transport_close     (GnomePrintTransport *transport);
gint gnome_print_transport_write     (GnomePrintTransport *transport, const guchar *buf, gint len);
gint gnome_print_transport_printf    (GnomePrintTransport *transport, const char *fmt, ...);
gint gnome_print_transport_set_error (GnomePrintTransport *transport, GError *err);

#ifdef GNOME_PRINT_UNSTABLE_API
gint gnome_print_transport_print_file (GnomePrintTransport *transport, const guchar *file_name);
#endif

G_END_DECLS

#endif /*  __GNOME_PRINT_TRANSPORT_H__ */
