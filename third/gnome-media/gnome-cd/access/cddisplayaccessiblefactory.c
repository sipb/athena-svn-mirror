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

#include <gtk/gtkaccessible.h>
#include "cddisplayaccessiblefactory.h"
#include "cddisplayaccessible.h"
#include "../display.h"

static void cddisplay_accessible_factory_class_init(CDDisplayAccessibleFactoryClass        *klass);

static AtkObject* cddisplay_accessible_factory_create_accessible (GObject *obj);

static GType cddisplay_accessible_factory_get_accessible_type (void);

GType
cddisplay_accessible_factory_get_type (void)
{
	static GType type = 0;

	if (!type) 
	{
		static const GTypeInfo tinfo =
		{
			sizeof (CDDisplayAccessibleFactoryClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) cddisplay_accessible_factory_class_init, /* class init */
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (CDDisplayAccessibleFactory), /* instance size */
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
			NULL /* value table */
		};

		type = g_type_register_static (ATK_TYPE_OBJECT_FACTORY, 
					       "CDDisplayAccessibleFactory",
					       &tinfo, 0);
	}
	return type;
}

static void 
cddisplay_accessible_factory_class_init (CDDisplayAccessibleFactoryClass *klass)
{
	AtkObjectFactoryClass *class = ATK_OBJECT_FACTORY_CLASS (klass);
	g_return_if_fail(class != NULL);

	class->create_accessible =
			cddisplay_accessible_factory_create_accessible;
	class->get_accessible_type =
			cddisplay_accessible_factory_get_accessible_type;
}

AtkObjectFactory*
cddisplay_accessible_factory_new (void)
{
	GObject *factory;

	factory = g_object_new (CDDISPLAY_TYPE_ACCESSIBLE_FACTORY, NULL);
	g_return_val_if_fail (factory != NULL, NULL);
	return ATK_OBJECT_FACTORY (factory);
} 

static AtkObject* 
cddisplay_accessible_factory_create_accessible (GObject     *obj)
{
	GtkWidget* widget;

	g_return_val_if_fail (GTK_IS_WIDGET (obj), NULL);
	widget = GTK_WIDGET (obj);
	return cddisplay_accessible_new (widget);
}

static GType
cddisplay_accessible_factory_get_accessible_type (void)
{
	return CDDISPLAY_TYPE_ACCESSIBLE;
}
