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

#include <config.h>
#include <glib-object.h>
#include <atk/atkcomponent.h>
#include <atk/atkstateset.h>
#include <libgnome/gnome-i18n.h>

#include "html.h"
#include "object.h"
#include "utils.h"

static void html_a11y_class_init (HTMLA11YClass *klass);
static void html_a11y_init       (HTMLA11Y *a11y_paragraph);

static void atk_component_interface_init (AtkComponentIface *iface);
static AtkObject*  html_a11y_get_parent (AtkObject *accessible);
static gint html_a11y_get_index_in_parent (AtkObject *accessible);
static AtkStateSet * html_a11y_ref_state_set (AtkObject *accessible);
static gint html_a11y_get_n_children (AtkObject *accessible);
static AtkObject * html_a11y_ref_child (AtkObject *accessible, gint index);

void html_a11y_get_size (AtkComponent *component, gint *width, gint *height);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_class_init,                     /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11Y),                                         /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_init,                        /* instance init */
			NULL                                                       /* value table */
		};

		static const GInterfaceInfo atk_component_info = {
			(GInterfaceInitFunc) atk_component_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (ATK_TYPE_OBJECT, "HTMLA11Y", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_COMPONENT, &atk_component_info);
	}

	return type;
}

static void 
atk_component_interface_init (AtkComponentIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_extents = html_a11y_get_extents;
	iface->get_size = html_a11y_get_size;

	/* FIX2
	   iface->add_focus_handler = gail_widget_add_focus_handler;
	   iface->get_extents = gail_widget_get_extents;
	   iface->get_layer = gail_widget_get_layer;
	   iface->grab_focus = gail_widget_grab_focus;
	   iface->remove_focus_handler = gail_widget_remove_focus_handler;
	   iface->set_extents = gail_widget_set_extents;
	   iface->set_position = gail_widget_set_position;
	   iface->set_size = gail_widget_set_size;
	*/
}

static void
html_a11y_finalize (GObject *obj)
{
}

static void
html_a11y_initialize (AtkObject *obj, gpointer data)
{
	/* printf ("html_a11y_initialize\n"); */

	g_object_set_data (G_OBJECT (obj), HTML_ID, data);

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
}

static void
html_a11y_class_init (HTMLA11YClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = html_a11y_initialize;
	atk_class->get_parent = html_a11y_get_parent;
	atk_class->get_index_in_parent = html_a11y_get_index_in_parent;
	atk_class->ref_state_set = html_a11y_ref_state_set;
	atk_class->get_n_children = html_a11y_get_n_children;
	atk_class->ref_child = html_a11y_ref_child;

	gobject_class->finalize = html_a11y_finalize;
}

static void
html_a11y_init (HTMLA11Y *a11y_paragraph)
{
}

static HTMLObject *
get_parent_html (AtkObject *accessible)
{
	HTMLObject *obj;

	obj = HTML_A11Y_HTML (accessible);

	return obj ? obj->parent : NULL;
}

static AtkObject* 
html_a11y_get_parent (AtkObject *accessible)
{
	AtkObject *parent;

	parent = accessible->accessible_parent;

	if (parent != NULL)
		g_return_val_if_fail (ATK_IS_OBJECT (parent), NULL);
	else {
		HTMLObject *parent_obj;

		parent_obj = get_parent_html (accessible);
		if (parent_obj) {
			parent = HTML_OBJECT_ACCESSIBLE (parent_obj);
		}
	}

	/* printf ("html_a11y_get_parent resolve to %p\n", parent); */

	return parent;
}

static gint
html_a11y_get_index_in_parent (AtkObject *accessible)
{
	HTMLObject *obj;
	gint index = -1;

	obj = HTML_A11Y_HTML (accessible);
	if (obj && obj->parent) {
		index = html_object_get_child_index (obj->parent, obj);
	}

	/* printf ("html_a11y_get_index_in_parent resolve to %d\n", index); */

	return index;  
}

static AtkStateSet *
html_a11y_ref_state_set (AtkObject *accessible)
{
	AtkStateSet *state_set;

	if (ATK_OBJECT_CLASS (parent_class)->ref_state_set)
		state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);
	if (!state_set)
		state_set = atk_state_set_new ();

	atk_state_set_add_state (state_set, ATK_STATE_VISIBLE);
	atk_state_set_add_state (state_set, ATK_STATE_ENABLED);

	/* printf ("html_a11y_ref_state_set resolves to %p\n", state_set); */

	return state_set;
}

static gint
html_a11y_get_n_children (AtkObject *accessible)
{
	HTMLObject *parent;
	gint n_children = 0;

	parent = HTML_A11Y_HTML (accessible);
	if (parent) {
		n_children = html_object_get_n_children (parent);
	}

	/* printf ("html_a11y_get_n_children resolves to %d\n", n_children); */

	return n_children;
}

static AtkObject *
html_a11y_ref_child (AtkObject *accessible, gint index)
{
	HTMLObject *parent, *child;
	AtkObject *accessible_child = NULL;

	parent = HTML_A11Y_HTML (accessible);
	if (parent) {
		child = html_object_get_child (parent, index);
		if (child) {
			accessible_child = html_utils_get_accessible (child, accessible);
			if (accessible_child)
				g_object_ref (accessible_child);
		}
	}		

	/* printf ("html_a11y_ref_child %d resolves to %p\n", index, accessible_child); */

	return accessible_child;
}

GtkHTMLA11Y *
html_a11y_get_gtkhtml_parent (HTMLA11Y *a11y)
{
	GtkHTMLA11Y *gtkhtml_a11y = NULL;
	AtkObject *obj = ATK_OBJECT (a11y);

	while (obj) {
		obj = atk_object_get_parent (obj);
		if (G_IS_GTK_HTML_A11Y (obj)) {
			gtkhtml_a11y = GTK_HTML_A11Y (obj);
			break;
		}
	}

	return gtkhtml_a11y;
}

void
html_a11y_get_extents (AtkComponent *component, gint *x, gint *y, gint *width, gint *height, AtkCoordType coord_type)
{
	HTMLObject *obj = HTML_A11Y_HTML (component);
	GtkHTMLA11Y *a11y = NULL;
	gint ax, ay;

	g_return_if_fail (obj);

	a11y = html_a11y_get_gtkhtml_parent (HTML_A11Y (component));
	g_return_if_fail (a11y);

	atk_component_get_extents (ATK_COMPONENT (a11y), x, y, width, height, coord_type);
	html_object_calc_abs_position (obj, &ax, &ay);
	*x += ax;
	*y += ay - obj->ascent;
	*width = obj->width;
	*height = obj->ascent + obj->descent;
}

void
html_a11y_get_size (AtkComponent *component, gint *width, gint *height)
{
	HTMLObject *obj = HTML_A11Y_HTML (component);

	g_return_if_fail (obj);

	*width = obj->width;
	*height = obj->ascent + obj->descent;
}

AtkObject *
html_a11y_new (HTMLObject *html_obj, AtkRole role)
{
	GObject *object;
	AtkObject *accessible;

	object = g_object_new (G_TYPE_HTML_A11Y, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, html_obj);

	accessible->role = role;

	return accessible;
}
