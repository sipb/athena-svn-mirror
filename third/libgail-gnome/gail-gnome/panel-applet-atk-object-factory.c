/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2001 Sun Microsystems Inc.
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

#include <gtk/gtkwidget.h>
#include <atk/atkobjectfactory.h>
#include <libspi/Accessibility.h>
#include "panel-applet-atk-object.h"
#include "panel-applet-atk-object-factory.h"


static void panel_applet_atk_object_factory_class_init (
                                PanelAppletAtkObjectFactoryClass        *klass);

static AtkObject* panel_applet_atk_object_factory_create_object (
                                GObject                       *obj);

static GType panel_applet_atk_object_factory_get_object_type (void);

GType
panel_applet_atk_object_factory_get_type ()
{
  static GType type = 0;

  if (!type) 
  {
    static const GTypeInfo tinfo =
    {
      sizeof (PanelAppletAtkObjectFactoryClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) panel_applet_atk_object_factory_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (PanelAppletAtkObjectFactory), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) NULL, /* instance init */
      NULL /* value table */
    };
    type = g_type_register_static (ATK_TYPE_OBJECT_FACTORY, 
                           "PanelAppletAtkObjectFactory" , &tinfo, 0);
  }

  return type;
}

static void 
panel_applet_atk_object_factory_class_init (PanelAppletAtkObjectFactoryClass *klass)
{
  AtkObjectFactoryClass *class = ATK_OBJECT_FACTORY_CLASS (klass);

  class->create_accessible   = panel_applet_atk_object_factory_create_object;
  class->get_accessible_type = panel_applet_atk_object_factory_get_object_type;
}

AtkObjectFactory*
panel_applet_atk_object_factory_new ()
{
  GObject *factory;

  factory = g_object_new (PANEL_APPLET_TYPE_ATK_OBJECT_FACTORY, NULL);

  return ATK_OBJECT_FACTORY (factory);
} 

static AtkObject* 
panel_applet_atk_object_factory_create_object (GObject *obj)
{
  g_return_val_if_fail (PANEL_IS_APPLET (obj), NULL);

  return panel_applet_atk_object_new (PANEL_APPLET (obj));
}

static GType
panel_applet_atk_object_factory_get_object_type (void)
{
  return PANEL_APPLET_TYPE_ATK_OBJECT;
}
