/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.
   Authors:           Radek Doulik (rodo@helixcode.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkclist.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkscrolledwindow.h>
#include <glade/glade.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include "gnome-bindings-prop.h"

enum {
	CHANGED,
	KEYMAP_SELECTED,
	LAST_SIGNAL
};

static guint gnome_bindings_properties_signals [LAST_SIGNAL] = { 0, 0 };

struct _KeymapEntry {
	gchar *name;
	gchar *bindings_name;
	gchar *signal_name;
	GtkType enum_type;
	gboolean editable;
	GList *bindings;
};
typedef struct _KeymapEntry KeymapEntry;

GnomeBindingEntry *
gnome_binding_entry_new (guint keyval, guint modifiers, gchar *command)
{
	GnomeBindingEntry *be = g_new (GnomeBindingEntry, 1);

	be->command = g_strdup (command);
	be->keyval = keyval;
	be->modifiers = modifiers;

	return be;
}

void
gnome_binding_entry_destroy (GnomeBindingEntry *be)
{
	g_free (be->command);
	g_free (be);
}

GList *
gnome_binding_entry_list_copy (GList *list)
{
	GList *new_list = NULL;
	GnomeBindingEntry *be;

	list = g_list_last (list);
	while (list) {
		be = (GnomeBindingEntry *) list->data;
		new_list = g_list_prepend (new_list, gnome_binding_entry_new (be->keyval, be->modifiers, be->command));
		list = list->prev;
	}

	return new_list;
}

void
gnome_binding_entry_list_destroy (GList *list)
{
	GList *cur = list;

	while (cur) {
		gnome_binding_entry_destroy ((GnomeBindingEntry *) cur->data);
		cur = cur->next;
	}

	g_list_free (list);
}

static KeymapEntry *
keymap_entry_new (gchar *n, gchar *bn, gchar *sn, GtkType t, gboolean editable)
{
	KeymapEntry *ke = g_new (KeymapEntry, 1);
	GtkBindingSet *bset;
	GtkBindingEntry *bentry;

	ke->name          = g_strdup (n);
	ke->bindings_name = g_strdup (bn);
	ke->signal_name   = g_strdup (sn);
	ke->enum_type     = t;
	ke->editable      = editable;
	ke->bindings      = NULL;

	bset = gtk_binding_set_find (bn);
	if (bset) {
		for (bentry = bset->entries ;bentry; bentry = bentry->set_next) {
			if (!strcmp (bentry->signals->signal_name, sn)
			    && bentry->signals->args->arg_type == GTK_TYPE_IDENTIFIER) {

				ke->bindings = g_list_prepend
					(ke->bindings,
					 gnome_binding_entry_new (bentry->keyval,
								  bentry->modifiers,
								  bentry->signals->args->d.string_data));
			}
		}
	}

	return ke;
}

static void
keymap_entry_destroy (KeymapEntry *ke)
{
	g_free (ke->name);
	g_free (ke->bindings_name);
	g_free (ke->signal_name);

	g_free (ke);
}

static gchar *
string_from_key (guint keyval, guint mods)
{
	return (keyval) ? g_strconcat ((mods & GDK_SHIFT_MASK) ? "S-" : "",
				       (mods & GDK_CONTROL_MASK) ? "C-" : "",
				       (mods & GDK_MOD1_MASK) ? "M-" : "",
				       gdk_keyval_name (keyval), NULL)
		: g_strdup (_("<None>"));
}

static KeymapEntry *
get_keymap (GnomeBindingsProperties *prop)
{
	GtkWidget *active;

	active = gtk_menu_get_active (GTK_MENU (gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap))));

	return active ? (KeymapEntry *) gtk_object_get_data (GTK_OBJECT (active), "keymap") : NULL;
}

static void
changed_option_keymap (GtkWidget *w, GnomeBindingsProperties *prop)
{
	KeymapEntry *ke = get_keymap (prop);
	GnomeBindingEntry *be;
	GList *cur;
	GtkCList *clist;

	g_return_if_fail (ke);

	clist = GTK_CLIST (prop->clist_keymap);
	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);

	for (cur = ke->bindings; cur; cur = cur->next) {
		gchar *name [2];

		be = (GnomeBindingEntry *) cur->data;
		name [0] = string_from_key (be->keyval, be->modifiers);
		name [1] = be->command;
		gtk_clist_set_row_data (clist, gtk_clist_append (clist, name), be);
		g_free (name [0]);
	}

	gtk_clist_columns_autosize (clist);
	gtk_clist_thaw (clist);
	gtk_clist_select_row (clist, 0, 0);

	/* gtk_widget_set_sensitive (prop->button_add, ke->editable);
	   gtk_widget_set_sensitive (prop->button_delete, ke->editable); */

	gtk_signal_emit (GTK_OBJECT (prop), gnome_bindings_properties_signals [KEYMAP_SELECTED], ke->name);
	gtk_signal_emit (GTK_OBJECT (prop), gnome_bindings_properties_signals [CHANGED]);
}

static void
init (GnomeBindingsProperties *prop)
{
	GladeXML *xml;
	GtkWidget *menu;

	prop->bindingsets = g_hash_table_new (g_str_hash, g_str_equal);

	glade_gnome_init ();
	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-capplet.glade", "vbox_ks");

	if (!xml)
		g_error (_("Could not load glade file."));

	prop->option_keymap = glade_xml_get_widget (xml, "option_keymap");
	gtk_signal_connect (GTK_OBJECT (gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap))), "selection-done",
			    changed_option_keymap, prop);
	prop->clist_keymap  = glade_xml_get_widget (xml, "clist_keymap");

	/* prop->button_add     = glade_xml_get_widget (xml, "button_shortcut_add");
	   prop->button_delete  = glade_xml_get_widget (xml, "button_shortcut_delete");
	*/

	gtk_container_add (GTK_CONTAINER (prop), glade_xml_get_widget (xml, "vbox_ks"));

	gtk_frame_set_shadow_type (GTK_FRAME (prop), GTK_SHADOW_NONE);

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap));
	gtk_menu_set_active (GTK_MENU (menu), 0);
	gtk_container_remove (GTK_CONTAINER (menu), gtk_menu_get_active (GTK_MENU (menu)));
}

static inline GList *
get_menu_items (GnomeBindingsProperties *prop)
{
	return gtk_container_children
		(GTK_CONTAINER (gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap))));
}

static void
destroy (GtkObject *prop)
{
	GList *item;

	for (item = get_menu_items (GNOME_BINDINGS_PROPERTIES (prop)); item; item = item->next)
		keymap_entry_destroy ((KeymapEntry *) gtk_object_get_data (GTK_OBJECT (item->data), "keymap"));
}

static void
class_init (GnomeBindingsPropertiesClass *klass)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);

	object_class->destroy = destroy;
	klass->changed        = NULL;

	gnome_bindings_properties_signals [CHANGED] =
		gtk_signal_new ("changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GnomeBindingsPropertiesClass, changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	gnome_bindings_properties_signals [KEYMAP_SELECTED] =
		gtk_signal_new ("keymap_selected",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GnomeBindingsPropertiesClass, changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, gnome_bindings_properties_signals, LAST_SIGNAL);
}

GtkType
gnome_bindings_properties_get_type (void)
{
	static guint bindings_properties_type = 0;

	if (!bindings_properties_type) {
		static const GtkTypeInfo bindings_properties_info = {
			"GnomeBindingsProperties",
			sizeof (GnomeBindingsProperties),
			sizeof (GnomeBindingsPropertiesClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		bindings_properties_type = gtk_type_unique (GTK_TYPE_FRAME, &bindings_properties_info);
	}

	return bindings_properties_type;
}

GtkWidget *
gnome_bindings_properties_new ()
{
	return gtk_type_new (gnome_bindings_properties_get_type ());
}

void
gnome_bindings_properties_add_keymap (GnomeBindingsProperties *prop,
				      gchar *name,
				      gchar *bindings,
				      gchar *signal_name,
				      GtkType arg_enum_type,
				      gboolean editable)
{

	KeymapEntry *ke = keymap_entry_new (name, bindings, signal_name, arg_enum_type, editable);
	GtkWidget *item;

	item = gtk_menu_item_new_with_label (name);
	gtk_object_set_data (GTK_OBJECT (item), "keymap", ke);
	gtk_menu_append (GTK_MENU (gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap))),
			 item);

	g_hash_table_insert (prop->bindingsets, name, ke);
}

void
gnome_bindings_properties_select_keymap (GnomeBindingsProperties *prop,
					 gchar *name)
{
	KeymapEntry *ke;
	GList *item;
	gint i;

	ke = g_hash_table_lookup (prop->bindingsets, name);

	for (i = 0, item = get_menu_items (prop); item; item = item->next, i ++)
		if (ke == gtk_object_get_data (GTK_OBJECT (item->data), "keymap")) {
			gtk_option_menu_set_history (GTK_OPTION_MENU (prop->option_keymap), i);
			changed_option_keymap (gtk_option_menu_get_menu (GTK_OPTION_MENU (prop->option_keymap)), prop);
			break;
		}
}

gchar *
gnome_bindings_properties_get_keymap_name (GnomeBindingsProperties *prop)
{
	KeymapEntry *ke;

	ke = get_keymap (prop);

	return ke ? ke->name : NULL;
}
