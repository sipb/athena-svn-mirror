#ifndef __GP_TRANSPORT_FILE_H__
#define __GP_TRANSPORT_FILE_H__

/*
 * FILE transport destination
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

#define GP_TYPE_TRANSPORT_FILE (gp_transport_file_get_type ())
#define GP_TRANSPORT_FILE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GP_TYPE_TRANSPORT_FILE, GPTransportFile))
#define GP_TRANSPORT_FILE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GP_TYPE_TRANSPORT_FILE, GPTransportFileClass))
#define GP_IS_TRANSPORT_FILE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GP_TYPE_TRANSPORT_FILE))
#define GP_IS_TRANSPORT_FILE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GP_TYPE_TRANSPORT_FILE))
#define GP_TRANSPORT_FILE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GP_TYPE_TRANSPORT_FILE, GPTransportFileClass))

typedef struct _GPTransportFile GPTransportFile;
typedef struct _GPTransportFileClass GPTransportFileClass;

#include "../gnome-print-transport.h"

struct _GPTransportFile {
	GnomePrintTransport transport;
	guchar *name;
	gint fd;
};

struct _GPTransportFileClass {
	GnomePrintTransportClass parent_class;
};

GType gp_transport_file_get_type (void);

G_END_DECLS

#endif


