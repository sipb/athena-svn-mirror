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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <sys/types.h>
#include <bonobo.h>

#include "../cdrom.h"
#include "cddisplayaccessible.h"
#include "../display.h"
#include "../gnome-cd.h"
#include "pangoaccessiblefactory.h"

static void cddisplay_accessible_class_init(CDDisplayAccessibleClass *klass);
static gint cddisplay_accessible_get_n_children   (AtkObject       *obj);
static AtkObject* cddisplay_accessible_ref_child  (AtkObject *obj, gint i);

static void cddisplay_accessible_init_pango_layout(AtkObject *aobj, gint j);
static void atk_object_set_name_description (AtkObject *aobj,
					     const gchar *name,
					     const gchar *desc);

/*
 * Array for holding the AtkObject structures for the children
 */
AtkObject *pango_accessible[CD_DISPLAY_END];

GType
cddisplay_accessible_get_type (void)
{
	static GType type = 0;

	if (!type)
	{
		static GTypeInfo tinfo =
		{
			sizeof (CDDisplayAccessibleClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) cddisplay_accessible_class_init, /* class init */
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (CDDisplayAccessible), /* instance size */
			0, /* nb preallocs */
			NULL, /* instance init */
			NULL /* value table */
		};

		/*
		 * Figure out the size of the class and instance
		 * we are deriving from
		 */
		AtkObjectFactory *factory;
		GType derived_type;
		GTypeQuery query;
		GType derived_atk_type;

		derived_type = g_type_parent (CD_DISPLAY_TYPE);
		factory = atk_registry_get_factory (atk_get_default_registry(),
						    derived_type);
		derived_atk_type = atk_object_factory_get_accessible_type (factory);
		g_type_query (derived_atk_type, &query);
		tinfo.class_size = query.class_size;
		tinfo.instance_size = query.instance_size;

		type = g_type_register_static(derived_atk_type,
					      "CDDisplayAccessible", &tinfo, 0);
	}

	return type;
}

static void
cddisplay_accessible_class_init (CDDisplayAccessibleClass *klass)
{
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	g_return_if_fail(class != NULL);
	class->ref_child = cddisplay_accessible_ref_child;
	class->get_n_children = cddisplay_accessible_get_n_children;
}

AtkObject *
cddisplay_accessible_new (GtkWidget *widget)
{
	GObject *object;
	GtkAccessible *gtk_accessible;
	AtkObject *accessible;

	object = g_object_new (CDDISPLAY_TYPE_ACCESSIBLE, NULL);
	g_return_val_if_fail(object != NULL, NULL);

	accessible = ATK_OBJECT (object);
	gtk_accessible = GTK_ACCESSIBLE (accessible);
	gtk_accessible->widget = widget; 
	atk_object_initialize (accessible, widget);

	accessible->name = g_strdup (_("CD Display"));
	accessible->role = ATK_ROLE_DRAWING_AREA;
	accessible->description = g_strdup (_("Displays information about the currently playing album, artist and time elapsed"));

	return accessible;
}

/*
 * We report the number of lines contained in the CD Display
 */

static gint
cddisplay_accessible_get_n_children (AtkObject* obj)
{
	return CD_DISPLAY_END;
}

/*
 * Return the children (pango layout) depending on the index i
 */
static AtkObject*
cddisplay_accessible_ref_child (AtkObject *obj,
                       		gint      i)
{
	CDDisplay *display;
	GtkWidget *widget;
	gint j;
	AtkRegistry *default_registry;
	AtkObjectFactory *factory;
	PangoLayout *playout;

	static gint first_time = 1;

	widget = GTK_ACCESSIBLE (obj)->widget;
	g_return_val_if_fail(widget != NULL, NULL);

	display = CD_DISPLAY (widget);

	/*
	 * Create the factory for the children if it's the first time.
	 */
	if (first_time) {
		default_registry = atk_get_default_registry();
		factory = atk_registry_get_factory(default_registry,
						   PANGO_TYPE_LAYOUT);
		for (j = 0; j < CD_DISPLAY_END; j++) {
			playout = (PangoLayout *)cd_display_get_pango_layout(
							display, j);
			g_return_val_if_fail(playout != NULL, NULL);

			pango_accessible[j] =
				atk_object_factory_create_accessible (factory,
							G_OBJECT(playout));
			cddisplay_accessible_init_pango_layout(
						pango_accessible[j], j);
			atk_object_set_parent(pango_accessible[j], obj);
		}
		first_time--;
	}

	g_return_val_if_fail(i < CD_DISPLAY_END, NULL);

	g_object_ref (pango_accessible[i]);
	return pango_accessible[i];
}

static void
cddisplay_accessible_init_pango_layout(AtkObject *aobj, gint j)
{
	switch (j) {
		case 0: 
			atk_object_set_name_description (aobj, _("Time Line"),
				_("Line for displaying the time elapsed for the current track"));
			break;
		case 1: 
			atk_object_set_name_description (aobj, _("Info Line"),
				_("Line for displaying information"));
			break;
		case 2: 
			atk_object_set_name_description (aobj, _("Artist Line"),
				_("Line for displaying the name of the artist"));
			break;
		case 3: 
			atk_object_set_name_description (aobj, _("Album Line"),
				_("Line for displaying the name of the album"));	
			break;
		default:
			return;
	}
}

static void
atk_object_set_name_description (AtkObject *aobj,
				 const gchar *name,
				 const gchar *desc)
{
	g_return_if_fail(aobj != NULL);

	aobj->name = g_strdup (name);
	aobj->description = g_strdup (desc);
}
