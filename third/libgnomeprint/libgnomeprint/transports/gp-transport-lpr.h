#ifndef __GP_TRANSPORT_LPR_H__
#define __GP_TRANSPORT_LPR_H__

/*
 * LPR transport destination
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

#define GP_TYPE_TRANSPORT_LPR (gp_transport_lpr_get_type ())
#define GP_TRANSPORT_LPR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GP_TYPE_TRANSPORT_LPR, GPTransportLPR))
#define GP_TRANSPORT_LPR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GP_TYPE_TRANSPORT_LPR, GPTransportLPRClass))
#define GP_IS_TRANSPORT_LPR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GP_TYPE_TRANSPORT_LPR))
#define GP_IS_TRANSPORT_LPR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GP_TYPE_TRANSPORT_LPR))
#define GP_TRANSPORT_LPR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GP_TYPE_TRANSPORT_LPR, GPTransportLPRClass))

typedef struct _GPTransportLPR GPTransportLPR;
typedef struct _GPTransportLPRClass GPTransportLPRClass;

#include <stdio.h>
#include "../gnome-print-transport.h"

struct _GPTransportLPR {
	GnomePrintTransport transport;
	guchar *printer;
	FILE *pipe;
};

struct _GPTransportLPRClass {
	GnomePrintTransportClass parent_class;
};

GType gp_transport_lpr_get_type (void);

G_END_DECLS

#endif


