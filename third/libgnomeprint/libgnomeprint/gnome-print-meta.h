/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-meta.h: Metafile implementation for gnome-print
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
 *    Miguel de Icaza (miguel@gnu.org)
 *    Michael Zucchi <notzed@helixcode.com>
 *    Morten Welinder (terra@diku.dk)
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PRINT_META_H__
#define __GNOME_PRINT_META_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_META         (gnome_print_meta_get_type ())
#define GNOME_PRINT_META(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_META, GnomePrintMeta))
#define GNOME_IS_PRINT_META(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_META))

typedef struct _GnomePrintMeta GnomePrintMeta;

#include <libgnomeprint/gnome-print.h>

GType gnome_print_meta_get_type (void);

GnomePrintContext * gnome_print_meta_new (void);

gint           gnome_print_meta_get_length (const GnomePrintMeta *meta);
const guchar * gnome_print_meta_get_buffer (const GnomePrintMeta *meta);
int            gnome_print_meta_get_pages  (const GnomePrintMeta *meta);

gint           gnome_print_meta_render_data      (GnomePrintContext *ctx, const guchar *data, gint length);
gint           gnome_print_meta_render_data_page (GnomePrintContext *ctx, const guchar *data, gint length, gint page, gboolean pageops);
gint           gnome_print_meta_render_file      (GnomePrintContext *ctx, const guchar *filename);
gint           gnome_print_meta_render_file_page (GnomePrintContext *ctx, const guchar *filename, gint page, gboolean pageops);

G_END_DECLS

#endif /* __GNOME_PRINT_META_H__ */
