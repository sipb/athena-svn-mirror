/*
 * Copyright 2002 Sun Microsystems Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __PANGO_ACCESSIBLE_H__
#define __PANGO_ACCESSIBLE_H__

#include <gtk/gtkaccessible.h>
#include <libgail-util/gailtextutil.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PANGO_TYPE_ACCESSIBLE                     (pango_accessible_get_type ())
#define PANGO_ACCESSIBLE(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), PANGO_TYPE_ACCESSIBLE, PangoAccessible))
#define PANGO_ACCESSIBLE_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ACCESSIBLE, PangoAccessibleClass))
#define PANGO_IS_ACCESSIBLE(obj)                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PANGO_TYPE_ACCESSIBLE))
#define PANGO_IS_ACCESSIBLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ACCESSIBLE))
#define PANGO_ACCESSIBLE_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ACCESSIBLE, PangoAccessibleClass))

typedef struct _PangoAccessible                   PangoAccessible;
typedef struct _PangoAccessibleClass              PangoAccessibleClass;

struct _PangoAccessible
{
	AtkObject	parent;
	GailTextUtil *textutil;
	PangoLayout *playout;
};

GType pango_accessible_get_type (void);

struct _PangoAccessibleClass
{
	AtkObjectClass parent_class;
};

AtkObject* pango_accessible_new (PangoLayout *gobject);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PANGO_ACCESSIBLE_H__ */
