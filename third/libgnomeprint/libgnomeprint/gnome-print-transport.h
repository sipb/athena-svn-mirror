#ifndef _GNOME_PRINT_TRANSPORT_H_
#define _GNOME_PRINT_TRANSPORT_H_

/*
 * Abstract base class for transport providers
 *
 * Authors:
 *   Raph Levien (raph@acm.org)
 *   Miguel de Icaza (miguel@kernel.org)
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Chema Celorio (chema@celorio.com)
 *
 * Copyright (C) 1999-2001 Ximian, Inc. and authors
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_TRANSPORT (gnome_print_transport_get_type ())
#define GNOME_PRINT_TRANSPORT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransport))
#define GNOME_PRINT_TRANSPORT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransportClass))
#define GNOME_IS_PRINT_TRANSPORT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_TRANSPORT))
#define GNOME_IS_PRINT_TRANSPORT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_TYPE_PRINT_TRANSPORT))
#define GNOME_PRINT_TRANSPORT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GNOME_TYPE_PRINT_TRANSPORT, GnomePrintTransportClass))

/* GnomePrintTransport is defined in gnome-print-private.h */
typedef struct _GnomePrintTransportClass GnomePrintTransportClass;

#include <libgnomeprint/gnome-print-private.h>

struct _GnomePrintTransport {
	GObject object;
	GnomePrintConfig *config;
	guint opened : 1;
};

struct _GnomePrintTransportClass {
	GObjectClass parent_class;

	gint (* construct) (GnomePrintTransport *transport);

	gint (* open) (GnomePrintTransport *transport);
	gint (* close) (GnomePrintTransport *transport);

	gint (* write) (GnomePrintTransport *transport, const guchar *buf, gint len);
};

GType gnome_print_transport_get_type (void);

gint gnome_print_transport_construct (GnomePrintTransport *transport, GnomePrintConfig *config);
gint gnome_print_transport_open (GnomePrintTransport *transport);
gint gnome_print_transport_close (GnomePrintTransport *transport);

gint gnome_print_transport_write (GnomePrintTransport *transport, const guchar *buf, gint len);

/* Convenience methods */

gint gnome_print_transport_printf (GnomePrintTransport *pc, const char *fmt, ...);

GnomePrintTransport *gnome_print_transport_new (GnomePrintConfig *config);

G_END_DECLS

#endif


