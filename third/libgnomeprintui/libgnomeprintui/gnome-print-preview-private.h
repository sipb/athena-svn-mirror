/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GNOME_PRINT_PREVIEW_PRIVATE_H__
#define __GNOME_PRINT_PREVIEW_PRIVATE_H__

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-preview.c: print preview driver
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
 *    Miguel de Icaza <miguel@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 1999-2002 Ximian Inc. and authors
 *
 */

#include <glib.h>

G_BEGIN_DECLS


#include <libgnomecanvas/libgnomecanvas.h>
#include <libgnomeprint/private/gnome-print-private.h>

#include "gnome-print-preview.h"

typedef struct _GnomePrintPreviewPrivate GnomePrintPreviewPrivate;

struct _GnomePrintPreview {
	GnomePrintContext pc;

	GnomePrintPreviewPrivate *priv;

	GnomeCanvas *canvas;
};

struct _GnomePrintPreviewClass {
	GnomePrintContextClass parent_class;
};

#define GPP_COLOR_RGBA(color, ALPHA)              \
                ((guint32) (ALPHA               | \
                (((color).red   / 256) << 24)   | \
                (((color).green / 256) << 16)   | \
                (((color).blue  / 256) << 8)))

void gnome_print_preview_theme_compliance (GnomePrintPreview *pp,
					   gboolean strict);

void gnome_print_preview_use_theme (gboolean theme);

G_END_DECLS

#endif /* __GNOME_PRINT_PREVIEW_PRIVATE_H__ */
