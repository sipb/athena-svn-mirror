#ifndef __GNOME_PRINT_PREVIEW_PRIVATE_H__
#define __GNOME_PRINT_PREVIEW_PRIVATE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GnomePrintPreviewPrivate GnomePrintPreviewPrivate;

#include <libgnomecanvas/libgnomecanvas.h>
#include <libgnomeprint/private/gnome-print-private.h>
#include "gnome-print-preview.h"

struct _GnomePrintPreview {
	GnomePrintContext pc;

	GnomePrintPreviewPrivate *priv;

	GnomeCanvas *canvas;
};

struct _GnomePrintPreviewClass {
	GnomePrintContextClass parent_class;
};

G_END_DECLS

#endif

