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

#ifndef PANEL_APPLET_ATK_OBJECT_FACTORY_H_
#define PANEL_APPLET_ATK_OBJECT_FACTORY_H_

#include <atk/atk.h>

G_BEGIN_DECLS

#define PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY                    (panel_applet_atk_object_factory_get_type ())
#define PANEL_APPLET_ATK_OBJECT_FACTORY(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY, PanelAppletAtkObjectFactory))
#define PANEL_APPLET_ATK_OBJECT_FACTORY_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY, PanelAppletAtkObjectFactoryClass))
#define PANEL_APPLET_IS_ATK_OBJECT_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY))
#define PANEL_APPLET_IS_ATK_OBJECT_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY))
#define PANEL_APPLET_ATK_OBJECT_FACTORY_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY, PanelAppletAtkObjectFactoryClass))

typedef struct _PanelAppletAtkObjectFactory                   PanelAppletAtkObjectFactory;
typedef struct _PanelAppletAtkObjectFactoryClass              PanelAppletAtkObjectFactoryClass;

struct _PanelAppletAtkObjectFactory
{
  AtkObjectFactory parent;
};

struct _PanelAppletAtkObjectFactoryClass
{
  AtkObjectFactoryClass parent_class;
};

GType panel_applet_atk_object_factory_get_type();

AtkObjectFactory *panel_applet_atk_object_factory_new();

G_END_DECLS

#endif /* PANEL_APPLET_ATK_OBJECT_FACTORY_H_ */
