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

#ifndef __CDDISPLAY_ACCESSIBLE_H__
#define __CDDISPLAY_ACCESSIBLE_H__

#include <gtk/gtkaccessible.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CDDISPLAY_TYPE_ACCESSIBLE                     (cddisplay_accessible_get_type ())
#define CDDISPLAY_ACCESSIBLE(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), CDDISPLAY_TYPE_ACCESSIBLE, CDDisplayAccessible))
#define CDDISPLAY_ACCESSIBLE_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), CDDISPLAY_TYPE_ACCESSIBLE, CDDisplayAccessibleClass))
#define CDDISPLAY_IS_ACCESSIBLE(obj)                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CDDISPLAY_TYPE_ACCESSIBLE))
#define CDDISPLAY_IS_ACCESSIBLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE ((klass), CDDISPLAY_TYPE_ACCESSIBLE))
#define CDDISPLAY_ACCESSIBLE_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), CDDISPLAY_TYPE_ACCESSIBLE, CDDisplayAccessibleClass))

typedef struct _CDDisplayAccessible                   CDDisplayAccessible;
typedef struct _CDDisplayAccessibleClass              CDDisplayAccessibleClass;

struct _CDDisplayAccessible
{
	GtkAccessible	parent;
};

GType cddisplay_accessible_get_type (void);

struct _CDDisplayAccessibleClass
{
	GtkAccessibleClass parent_class;
};

AtkObject* cddisplay_accessible_new (GtkWidget *widget);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CDDISPLAY_ACCESSIBLE_H__ */
