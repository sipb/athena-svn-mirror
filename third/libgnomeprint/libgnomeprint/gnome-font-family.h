#ifndef __GNOME_FONT_FAMILY_H__
#define __GNOME_FONT_FAMILY_H__

/*
 * GnomeFontFamily
 *
 * Authors:
 *   Jody Goldberg <jody@helixcode.com>
 *   Miguel de Icaza <miguel@helixcode.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *   Christopher James Lahey <clahey@helixcode.com>
 *   Michael Meeks <michael@helixcode.com>
 *   Morten Welinder <terra@diku.dk>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc. and authors
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_FONT_FAMILY (gnome_font_family_get_type ())
#define GNOME_FONT_FAMILY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_FONT_FAMILY, GnomeFontFamily))
#define GNOME_IS_FONT_FAMILY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_FONT_FAMILY))

typedef struct _GnomeFontFamily GnomeFontFamily;

#include <libgnomeprint/gnome-font-face.h>

GType gnome_font_family_get_type (void);

#define gnome_font_family_ref(f) g_object_ref (G_OBJECT (f))
#define gnome_font_family_unref(f) g_object_unref (G_OBJECT (f))

/*
 * Methods
 *
 */

GnomeFontFamily * gnome_font_family_new (const gchar * name);

GList * gnome_font_family_style_list (GnomeFontFamily * family);
void gnome_font_family_style_list_free (GList * list);

GnomeFontFace * gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style);

G_END_DECLS

#endif /* __GNOME_FONT_FAMILY_H__ */






