#ifndef __GNOME_PRINT_META_H__
#define __GNOME_PRINT_META_H__

/*
 *  Copyright (C) 1999-2001 Ximian Inc. and authors
 *
 *  Authors:
 *    Miguel de Icaza (miguel@gnu.org)
 *    Michael Zucchi <notzed@helixcode.com>
 *    Morten Welinder (terra@diku.dk)
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Metafile implementation for gnome-print
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
 */

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_META (gnome_print_meta_get_type ())
#define GNOME_PRINT_META(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_META, GnomePrintMeta))
#define GNOME_PRINT_META_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GNOME_TYPE_PRINT_META, GnomePrintMetaClass))
#define GNOME_IS_PRINT_META(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_META))
#define GNOME_IS_PRINT_META_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_TYPE_PRINT_META))
#define GNOME_PRINT_META_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GNOME_TYPE_PRINT_META, GnomePrintMetaClass))

typedef struct _GnomePrintMeta GnomePrintMeta;
typedef struct _GnomePrintMetaClass GnomePrintMetaClass;

/*
 * Metafile is architecure idependent representation of print stream.
 *
 * It is used, for example, by bonobo print to transfer job from
 * component to container, and print master to store stream generated
 * by application.
 *
 * You are free to use it for your own purposes, but beware that it
 * is somewhat slow and memory hungry. I'll replace it with something
 * more compact for internal storage (GnomePrintMaster), although it
 * probably remains there as bonobo transport format.
 *
 * Still, the API remains in-place, so it is safe to use it.
 *
 * Notice - it is safe to use unclosed streams, but if you render those,
 * final showpage will be omitted as well (although that can be exactly
 * what you want).
 *
 */

#include <libgnomeprint/gnome-print.h>

GType gnome_print_meta_get_type (void);

/* Create new metafile context */
GnomePrintContext *gnome_print_meta_new (void);
/* Create new metafile context for local use - resulting stream may not be architecture neutral */
GnomePrintContext *gnome_print_meta_new_local (void);

/* Get generated stream data */
const guchar *gnome_print_meta_get_buffer (const GnomePrintMeta *meta);
gint gnome_print_meta_get_length (const GnomePrintMeta *meta);

/* Get number of pages in stream */
int gnome_print_meta_get_pages (const GnomePrintMeta *meta);

/* Render stream to specified output context */
/* pageops specifies, whether you want to send beginpage/showpage to output or not */
gint gnome_print_meta_render_data (GnomePrintContext *ctx, const guchar *data, gint length);
gint gnome_print_meta_render_data_page (GnomePrintContext *ctx, const guchar *data, gint length, gint page, gboolean pageops);
gint gnome_print_meta_render_file (GnomePrintContext *ctx, const guchar *filename);
gint gnome_print_meta_render_file_page (GnomePrintContext *ctx, const guchar *filename, gint page, gboolean pageops);

G_END_DECLS

#endif /* __GNOME_PRINT_META_H__ */
