/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2004 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef PANEL_APPLET_ATK_OBJECT_H_
#define PANEL_APPLET_ATK_OBJECT_H_

#include <gtk/gtkaccessible.h>
#include <panel-applet.h>

G_BEGIN_DECLS

#define PANEL_APPLET_TYPE_ATK_OBJECT            (panel_applet_atk_object_get_type ())
#define PANEL_APPLET_ATK_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PANEL_APPLET_TYPE_ATK_OBJECT, PanelAppletAtkObject))
#define PANEL_APPLET_ATK_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANEL_APPLET_TYPE_ATK_OBJECT, PanelAppletAtkObjectClass))
#define PANEL_APPLET_IS_ATK_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PANEL_APPLET_TYPE_ATK_OBJECT))
#define PANEL_APPLET_IS_ATK_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANEL_APPLET_TYPE_ATK_OBJECT))
#define PANEL_APPLET_ATK_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANEL_APPLET_TYPE_ATK_OBJECT, PanelAppletAtkObjectClass))

typedef struct _PanelAppletAtkObject      PanelAppletAtkObject;
typedef struct _PanelAppletAtkObjectClass PanelAppletAtkObjectClass;

struct _PanelAppletAtkObject
{
  GtkAccessible parent;
};

struct _PanelAppletAtkObjectClass
{
  GtkAccessibleClass parent_class;
};

GType      panel_applet_atk_object_get_type ();

AtkObject *panel_applet_atk_object_new (PanelApplet *applet);

G_END_DECLS

#endif /* PANEL_APPLET_ATK_OBJECT_H_ */
