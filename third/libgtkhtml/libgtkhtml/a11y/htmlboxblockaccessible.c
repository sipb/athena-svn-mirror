/*
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

#include <libgtkhtml/gtkhtml.h>
#include "htmlboxblockaccessible.h"

static void       html_box_block_accessible_class_init      (HtmlBoxBlockAccessibleClass *klass);

GType
html_box_block_accessible_get_type (void)
{
	static GType type = 0;

	if (!type) {
		 static const GTypeInfo tinfo = {
			sizeof (HtmlBoxBlockAccessibleClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) html_box_block_accessible_class_init,
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (HtmlBoxBlockAccessible),
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
			NULL /* value table */
		};

		type = g_type_register_static (HTML_TYPE_BOX_ACCESSIBLE, "HtmlBoxBlockAccessible", &tinfo, 0);
	}

	return type;
}

AtkObject*
html_box_block_accessible_new (GObject *obj)
{
	GObject *object;
	AtkObject *atk_object;

	g_return_val_if_fail (HTML_IS_BOX_BLOCK (obj), NULL);
	object = g_object_new (HTML_TYPE_BOX_BLOCK_ACCESSIBLE, NULL);
	atk_object = ATK_OBJECT (object);
	atk_object_initialize (atk_object, obj);
	atk_object->role = ATK_ROLE_PANEL;
	return atk_object;
}

static void
html_box_block_accessible_class_init (HtmlBoxBlockAccessibleClass *klass)
{
}
