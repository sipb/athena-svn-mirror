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

#include <string.h>

#include "htmlviewaccessible.h"
#include "htmlviewaccessiblefactory.h"
#include <libgtkhtml/layout/htmlboxinline.h>

static void       html_view_accessible_class_init          (HtmlViewAccessibleClass *klass);
static void       html_view_accessible_finalize            (GObject            *obj);
static void       html_view_accessible_initialize          (AtkObject          *obj,
                                                            gpointer           data);
static gint       html_view_accessible_get_n_children      (AtkObject          *obj);
static AtkObject* html_view_accessible_ref_child           (AtkObject          *obj,
                                                            gint               i);
static AtkStateSet* html_view_accessible_ref_state_set     (AtkObject          *obj);

static void       html_view_accessible_grab_focus_cb       (GtkWidget          *widget);
static AtkObject* html_view_accessible_get_focus_object    (GtkWidget          *widget);

static void       focus_object_destroyed                   (gpointer           data);       
static void       set_focus_object                         (GObject            *obj,
                                                            AtkObject          *focus_obj);       

static gpointer parent_class = NULL;

GType
html_view_accessible_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo tinfo = {
			0, /* class size */
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) html_view_accessible_class_init,
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			0, /* instance size */
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
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

		derived_type = g_type_parent (HTML_TYPE_VIEW);
		factory = atk_registry_get_factory (atk_get_default_registry (), derived_type);
		derived_atk_type = atk_object_factory_get_accessible_type (factory);
		g_type_query (derived_atk_type, &query);
		tinfo.class_size = query.class_size;
		tinfo.instance_size = query.instance_size;

		type = g_type_register_static (derived_atk_type, "HtmlViewAccessible", &tinfo, 0);
	}

	return type;
}

static void
html_view_accessible_class_init (HtmlViewAccessibleClass *klass)
{
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = html_view_accessible_finalize;

	class->get_n_children = html_view_accessible_get_n_children;
	class->initialize = html_view_accessible_initialize;
	class->ref_child = html_view_accessible_ref_child;
	class->ref_state_set = html_view_accessible_ref_state_set;
}

AtkObject* 
html_view_accessible_new (GtkWidget *widget)
{
	GObject *object;
	AtkObject *accessible;

	object = g_object_new (HTML_TYPE_VIEW_ACCESSIBLE, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, widget);
	accessible->role =  ATK_ROLE_HTML_CONTAINER;
	return accessible;
}

static void
html_view_accessible_initialize (AtkObject *obj,
                                 gpointer  data)
{
	GtkWidget *widget;
	AtkObject *focus_object;

	ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

	widget = GTK_WIDGET (data);
	g_signal_connect (widget, "grab_focus",
			  G_CALLBACK (html_view_accessible_grab_focus_cb),
                          NULL);
	focus_object = html_view_accessible_get_focus_object (widget);
	if (focus_object)
		set_focus_object (G_OBJECT (obj), focus_object);
}

static void
html_view_accessible_finalize (GObject *obj)
{
	gpointer focus_obj;

	focus_obj = g_object_get_data (obj, "gail-focus-object");
	if (focus_obj) {
		g_object_weak_unref (focus_obj, 
				     (GWeakNotify) focus_object_destroyed,
				     obj);
	}
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static gint 
html_view_accessible_get_n_children (AtkObject* obj)
{
	GtkAccessible *accessible;
	GtkWidget *widget;
	HtmlView *html_view;
	gint n_children = 0;

	g_return_val_if_fail (HTML_IS_VIEW_ACCESSIBLE (obj), 0);
	accessible = GTK_ACCESSIBLE (obj);
	widget = accessible->widget;
	if (widget == NULL)
		/* State is defunct */
		return 0;

	g_return_val_if_fail (HTML_IS_VIEW (widget), 0);

	html_view = HTML_VIEW (widget);

	if (html_view->root)
		n_children = 1;

	return n_children;
}

static AtkObject* 
html_view_accessible_ref_child (AtkObject *obj, gint i)
{
	GtkAccessible *accessible;
	GtkWidget *widget;
	HtmlView *html_view;
	HtmlBox *html_box;
	AtkObject *atk_child = NULL;

	if (i != 0)
		return NULL;

	g_return_val_if_fail (HTML_IS_VIEW_ACCESSIBLE (obj), NULL);
	accessible = GTK_ACCESSIBLE (obj);
	widget = accessible->widget;
	if (widget == NULL)
		/* State is defunct */
		return NULL;

	g_return_val_if_fail (HTML_IS_VIEW (widget), NULL);

	html_view = HTML_VIEW (widget);

	html_box = html_view->root;

	if (html_box) {
		atk_child = atk_gobject_accessible_for_object (G_OBJECT (html_box));
		g_object_set_data (G_OBJECT (html_box), "view", widget);
		g_object_ref (atk_child);
	}
	return atk_child;
}

static void
html_view_accessible_grab_focus_cb (GtkWidget *widget)
{
	AtkObject *focus_object;
	AtkObject *obj;

	focus_object = html_view_accessible_get_focus_object (widget);

	obj = gtk_widget_get_accessible (widget);
	set_focus_object (G_OBJECT (obj), focus_object);
	if (GTK_WIDGET_HAS_FOCUS (widget) && focus_object)
		atk_focus_tracker_notify (focus_object);
}

static AtkObject*
html_view_accessible_get_focus_object (GtkWidget *widget)
{
	HtmlView *view;
	HtmlBox *box;
	DomElement *focus_element;
	AtkObject *atk_obj;

	view = HTML_VIEW (widget);

	focus_element = view->document->focus_element;

	if (focus_element) {
		box = html_view_find_layout_box (view, DOM_NODE (focus_element), FALSE);	
		if (HTML_IS_BOX_INLINE (box))
			box = box->children;

		g_object_set_data (G_OBJECT (box), "view", widget);
		atk_obj = atk_gobject_accessible_for_object (G_OBJECT (box));
	} else {
		atk_obj = NULL;
	}
	return atk_obj;
}

static AtkStateSet*
html_view_accessible_ref_state_set (AtkObject *obj)
{
	GtkWidget *widget = GTK_ACCESSIBLE (obj)->widget;
	HtmlView *view;
 	AtkStateSet *state_set; 

	state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (obj);

	if (widget == NULL)
    		return state_set;

	view = HTML_VIEW (widget);

	if (view->document->focus_element && GTK_WIDGET_HAS_FOCUS (widget))
		atk_state_set_remove_state (state_set, ATK_STATE_FOCUSED);

	return state_set;	
}

static void focus_object_destroyed (gpointer data)
{
	g_object_set_data (G_OBJECT (data), "gail-focus-object", NULL);
}

static void
set_focus_object (GObject *obj,
                  AtkObject *focus_obj) 
{
	gpointer old_focus_obj;

	old_focus_obj = g_object_get_data (obj, "gail-focus-object");
	if (old_focus_obj) {
		g_object_weak_unref (old_focus_obj, 
				     (GWeakNotify) focus_object_destroyed,
				     obj);
	}
	if (focus_obj)
		g_object_weak_ref (G_OBJECT (focus_obj), 
				   (GWeakNotify) focus_object_destroyed,
				   obj);
	g_object_set_data (obj, "gail-focus-object", focus_obj);
}
