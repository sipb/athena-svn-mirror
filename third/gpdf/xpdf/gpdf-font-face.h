/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* GnomePrint font-face wrapper
 *
 * Copyright (C) 2003 Filip Van Raemdonck <mechanix@debian.org>
 * 	              Martin Kretzschmar <m_kretzschmar@gmx.net>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_FONT_FACE_H
#define GPDF_FONT_FACE_H

#include <libgnomeprint/private/gnome-font-private.h>

G_BEGIN_DECLS

#define GPDF_FONT_FACE_TYPE            (gpdf_font_face_get_type())
#define GPDF_FONT_FACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_FONT_FACE_TYPE, GPdfFontFace))
#define GPDF_FONT_FACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_FONT_FACE_TYPE, GPdfFontFaceClass))
#define GPDF_IS_FONT_FACE(obj)	       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_FONT_FACE_TYPE))
#define GPDF_IS_FONT_FACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_FONT_FACE_TYPE))

typedef struct _GPdfFontFace GPdfFontFace;
typedef struct _GPdfFontFaceClass GPdfFontFaceClass;

struct _GPdfFontFace {
	GnomeFontFace gff;

	gboolean is_downloaded;
	guchar* font_data;
	gsize data_length;
};

#define GPDF_FONT_IS_DOWNLOADED(f) (f->face->is_downloaded)

struct _GPdfFontFaceClass {
	GnomeFontFaceClass parent_class;
};

GType          gpdf_font_face_get_type (void);
GnomeFontFace* gpdf_font_face_download (const guchar*, const guchar*, GnomeFontWeight, gboolean, const guchar*, gsize);

G_END_DECLS

#endif /* GPDF_FONT_FACE_H */
