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
#include "text.h"

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
		GTypeQuery query;
		GType derived_atk_type;

		factory = atk_registry_get_factory (atk_get_default_registry (), GTK_TYPE_WIDGET);
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

static AtkObject *
gtk_html_a11y_get_focus_object (GtkWidget * widget)
{
	GtkHTML * html;
	HTMLObject * htmlobj = NULL;
        AtkObject *obj = NULL;

	html = GTK_HTML(widget);
	if (html->engine && html->engine->cursor)
		htmlobj = html->engine->cursor->object;
	if (htmlobj)
		obj = html_utils_get_accessible (htmlobj, NULL);

	return obj;
}

static void
gtk_html_a11y_grab_focus_cb(GtkWidget * widget)
{
        AtkObject *focus_object, *obj, *clue;


	 focus_object = gtk_html_a11y_get_focus_object (widget);
        obj = gtk_widget_get_accessible (widget);
        g_object_set_data (G_OBJECT(obj), "gail-focus-object", focus_object);

	clue = html_utils_get_accessible(GTK_HTML(widget)->engine->clue, obj);
	atk_object_set_parent(clue, obj);

        atk_focus_tracker_notify (focus_object);

}

static void
gtk_html_a11y_cursor_move_cb(GtkWidget *widget,  GtkDirectionType dir_type, GtkHTMLCursorSkipType skip)
{
        AtkObject *focus_object, *obj;
	static AtkObject * prev_object = NULL;

	focus_object = gtk_html_a11y_get_focus_object (widget);
       obj = gtk_widget_get_accessible (widget);
	
	if (prev_object != focus_object) {
		prev_object = focus_object;
        	g_object_set_data (G_OBJECT(obj), "gail-focus-object", focus_object);
        	atk_focus_tracker_notify (focus_object);
	} else {
		if (G_IS_HTML_A11Y_TEXT(focus_object)) {
			gint offset;

			offset = (GTK_HTML(widget))->engine->cursor->offset;
			g_signal_emit_by_name(focus_object, "text_caret_moved",offset);
                }
        }
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
	g_signal_connect_after(widget, "grab_focus", 
			G_CALLBACK (gtk_html_a11y_grab_focus_cb),
			NULL);
	g_signal_connect_after(widget, "cursor_move",
			G_CALLBACK(gtk_html_a11y_cursor_move_cb),
			NULL);
	html_utils_get_accessible(GTK_HTML(widget)->engine->clue, accessible);

	/* printf ("created new gtkhtml accessible object\n"); */

	return accessible;
}
