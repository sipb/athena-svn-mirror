/* zvt_access-- accessibility support for libzvt
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "zvt-accessible-factory.h"
#include "zvt-accessible.h"

static void
zvt_accessible_factory_class_init ( ZvtAccessibleFactoryClass *klass);

static AtkObject* 
zvt_accessible_factory_create_accessible (GObject     *obj);

static AtkObjectFactoryClass    *parent_class = NULL;

GType
zvt_accessible_factory_get_type ()
{
  static GType type = 0;

  if (!type) 
  {
    static const GTypeInfo tinfo =
    {
      sizeof (ZvtAccessibleFactoryClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) zvt_accessible_factory_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (ZvtAccessibleFactory), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) NULL, /* instance init */
      NULL /* value table */
    };
    type = g_type_register_static (ATK_TYPE_OBJECT_FACTORY, 
                           "ZvtAccessibleFactory" , &tinfo, 0);
  }

  return type;
}

static void 
zvt_accessible_factory_class_init (ZvtAccessibleFactoryClass *klass)
{
  AtkObjectFactoryClass *class = ATK_OBJECT_FACTORY_CLASS (klass);

  parent_class = g_type_class_ref (ATK_TYPE_OBJECT_FACTORY);

  class->create_accessible = zvt_accessible_factory_create_accessible;
}


AtkObjectFactory *
zvt_accessible_factory_new (void)
{
  GObject *factory;

  factory = g_object_new (ZVT_TYPE_ACCESSIBLE_FACTORY, NULL);

  g_return_val_if_fail (factory != NULL, NULL);
  return ATK_OBJECT_FACTORY (factory);
} 

static AtkObject* 
zvt_accessible_factory_create_accessible (GObject     *obj)
{
  GtkAccessible *accessible;
  GtkWidget* widget;

  g_return_val_if_fail (GTK_IS_WIDGET(obj), NULL);

  widget = GTK_WIDGET (obj);

  accessible = GTK_ACCESSIBLE (zvt_accessible_new (widget));

  g_return_val_if_fail (accessible != NULL, NULL);

  return ATK_OBJECT (accessible);
}
