/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "gtkhtml.h"
#include "htmlengine.h"
#include "htmlobject.h"

#include "object.h"
#include "paragraph.h"
#include "utils.h"

static void gtk_html_a11y_class_init (GtkHTMLA11YClass *klass);
static void gtk_html_a11y_init       (GtkHTMLA11Y *a11y);

static GtkAccessibleClass *parent_class = NULL;

GType
gtk_html_a11y_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo tinfo = {
			sizeof (GtkHTMLA11YClass),
			NULL,                                            /* base init */
			NULL,                                            /* base finalize */
			(GClassInitFunc) gtk_html_a11y_class_init,       /* class init */
			NULL,                                            /* class finalize */
			NULL,                                            /* class data */
			sizeof (GtkHTMLA11Y),                            /* instance size */
			0,                                               /* nb preallocs */
			(GInstanceInitFunc) gtk_html_a11y_init,          /* instance init */
			NULL                                             /* value table */
		};

		/*
		 * Figure out the size of the class and instance 
		 * we are deriving from
		 */
		AtkObjectFactory *factory;
		GType derived_type;
		GTypeQuery query;
		GType derived_atk_type;

		derived_type = g_type_parent (GTK_TYPE_HTML);
		factory = atk_registry_get_factory (atk_get_default_registry (), derived_type);
		derived_atk_type = atk_object_factory_get_accessible_type (factory);
		g_type_query (derived_atk_type, &query);
		tinfo.class_size = query.class_size;
		tinfo.instance_size = query.instance_size;

		type = g_type_register_static (derived_atk_type, "GtkHTMLA11Y", &tinfo, 0);
	}

	return type;
}

static void
gtk_html_a11y_finalize (GObject *obj)
{
}

static void
gtk_html_a11y_initialize (AtkObject *obj, gpointer data)
{
	/* printf ("gtk_html_a11y_initialize\n"); */

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

	g_object_set_data (G_OBJECT (obj), GTK_HTML_ID, data);
}

static gint
gtk_html_a11y_get_n_children (AtkObject *accessible)
{
	HTMLObject *clue;
	gint n_children = 0;

	clue = GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue;
	if (clue)
		n_children = html_object_get_n_children (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue);

	/* printf ("gtk_html_a11y_get_n_children resolves to %d\n", n_children); */

	return n_children;
}

static AtkObject *
gtk_html_a11y_ref_child (AtkObject *accessible, gint index)
{
	HTMLObject *child;
	AtkObject *accessible_child = NULL;
	
	if (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue) {
		child = html_object_get_child (GTK_HTML_A11Y_GTKHTML (accessible)->engine->clue, index);
		if (child) {
			accessible_child = html_utils_get_accessible (child, accessible);
			if (accessible_child)
				g_object_ref (accessible_child);
		}
	}

	/* printf ("gtk_html_a11y_ref_child %d resolves to %p\n", index, accessible_child); */

	return accessible_child;
}

static void
gtk_html_a11y_class_init (GtkHTMLA11YClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = gtk_html_a11y_initialize;
	atk_class->get_n_children = gtk_html_a11y_get_n_children;
	atk_class->ref_child = gtk_html_a11y_ref_child;

	gobject_class->finalize = gtk_html_a11y_finalize;
}

static void
gtk_html_a11y_init (GtkHTMLA11Y *a11y)
{
}

AtkObject* 
gtk_html_a11y_new (GtkWidget *widget)
{
	GObject *object;
	AtkObject *accessible;

	g_return_val_if_fail (GTK_IS_HTML (widget), NULL);

	object = g_object_new (G_TYPE_GTK_HTML_A11Y, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, widget);

	accessible->role = ATK_ROLE_HTML_CONTAINER;

	/* printf ("created new gtkhtml accessible object\n"); */

	return accessible;
}
