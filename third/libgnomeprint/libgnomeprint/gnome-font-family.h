/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-font-family.h:
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
 *    Jody Goldberg <jody@helixcode.com>
 *    Miguel de Icaza <miguel@helixcode.com>
 *    Lauris Kaplinski <lauris@helixcode.com>
 *    Christopher James Lahey <clahey@helixcode.com>
 *    Michael Meeks <michael@helixcode.com>
 *    Morten Welinder <terra@diku.dk>
 *
 *  Copyright (C) 2000-2003 Ximian Inc. and authors
 *
 */

#ifndef __GNOME_FONT_FAMILY_H__
#define __GNOME_FONT_FAMILY_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_FONT_FAMILY  (gnome_font_family_get_type ())
#define GNOME_FONT_FAMILY(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_FONT_FAMILY, GnomeFontFamily))
#define GNOME_IS_FONT_FAMILY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_FONT_FAMILY))

#define gnome_font_family_ref(f)   g_object_ref   (G_OBJECT (f))
#define gnome_font_family_unref(f) g_object_unref (G_OBJECT (f))

typedef struct _GnomeFontFamily GnomeFontFamily;

#include <libgnomeprint/gnome-font-face.h>

GType             gnome_font_family_get_type (void);
GnomeFontFamily * gnome_font_family_new (const gchar * name);

GnomeFontFace *   gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style);

GList *           gnome_font_family_style_list (GnomeFontFamily * family);
void              gnome_font_family_style_list_free (GList * list);

G_END_DECLS

#endif /* __GNOME_FONT_FAMILY_H__ */






