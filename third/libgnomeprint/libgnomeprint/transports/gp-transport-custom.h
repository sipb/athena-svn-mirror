#ifndef __GP_TRANSPORT_CUSTOM_H__
#define __GP_TRANSPORT_CUSTOM_H__

/*
 * Custom transport destination
 *
 * Authors:
 *   Raph Levien (raph@acm.org)
 *   Miguel de Icaza (miguel@kernel.org)
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Chema Celorio (chema@celorio.com)
 *   Carlos Perelló Marín <carlos@gnome-db.org>
 *
 * Copyright (C) 1999-2001 Ximian, Inc. and authors
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GP_TYPE_TRANSPORT_CUSTOM (gp_transport_custom_get_type ())
#define GP_TRANSPORT_CUSTOM(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GP_TYPE_TRANSPORT_CUSTOM, GPTransportCustom))
#define GP_TRANSPORT_CUSTOM_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GP_TYPE_TRANSPORT_CUSTOM, GPTransportCustomClass))
#define GP_IS_TRANSPORT_CUSTOM(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GP_TYPE_TRANSPORT_CUSTOM))
#define GP_IS_TRANSPORT_CUSTOM_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GP_TYPE_TRANSPORT_CUSTOM))
#define GP_TRANSPORT_CUSTOM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GP_TYPE_TRANSPORT_CUSTOM, GPTransportCustomClass))

typedef struct _GPTransportCustom GPTransportCustom;
typedef struct _GPTransportCustomClass GPTransportCustomClass;

#include <stdio.h>
#include "../gnome-print-transport.h"

struct _GPTransportCustom {
	GnomePrintTransport transport;
	guchar *command;
	FILE *pipe;
};

struct _GPTransportCustomClass {
	GnomePrintTransportClass parent_class;
};

GType gp_transport_custom_get_type (void);

G_END_DECLS

#endif


