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

#ifndef _GNOME_BINDINGS_PROPERTIES_
#define _GNOME_BINDINGS_PROPERTIES_

typedef struct _GnomeBindingEntry GnomeBindingEntry;
typedef struct _GnomeBindingsProperties GnomeBindingsProperties;
typedef struct _GnomeBindingsPropertiesClass GnomeBindingsPropertiesClass;

#include <gtk/gtkwidget.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtktypeutils.h>

#define GNOME_TYPE_BINDINGS_PROPERTIES            (gnome_bindings_properties_get_type ())
#define GNOME_BINDINGS_PROPERTIES(obj)            (GTK_CHECK_CAST ((obj), \
								   GNOME_TYPE_BINDINGS_PROPERTIES, \
								   GnomeBindingsProperties))
#define GNOME_BINDINGS_PROPERTIES_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), \
									 GNOME_TYPE_BINDINGS_PROPERTIES, \
									 GnomeBindingsPropertiesClass))
#define GNOME_IS_BINDINGS_PROPERTIES(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_BINDINGS_PROPERTIES))
#define GNOME_IS_BINDINGS_PROPERTIES_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_BINDINGS_PROPERTIES))

struct _GnomeBindingsProperties {
	GtkFrame base;

	GtkWidget *option_keymap;
	GtkWidget *clist_keymap;

	/* GtkWidget *button_add;
	   GtkWidget *button_delete;
	*/

	GHashTable *bindingsets;
};

struct _GnomeBindingsPropertiesClass {
	GtkFrameClass parent_class;

	void   (*changed)           (GnomeBindingsProperties *prop);
	void   (*keymap_selected)   (GnomeBindingsProperties *prop, gchar *keymap);
};

struct _GnomeBindingEntry {
	guint keyval;
	guint modifiers;

	gchar *command;
};

GtkType            gnome_bindings_properties_get_type         (void);
GtkWidget         *gnome_bindings_properties_new              (void);
void               gnome_bindings_properties_add_keymap       (GnomeBindingsProperties *prop,
							       gchar                   *name,
							       gchar                   *bindings,
							       gchar                   *signal_name,
							       GtkType                  arg_enum_type,
							       gboolean                 editable);
void               gnome_bindings_properties_select_keymap    (GnomeBindingsProperties *prop,
							       gchar                   *name);
gchar             *gnome_bindings_properties_get_keymap_name  (GnomeBindingsProperties *prop);

/* binding entry */
GnomeBindingEntry *gnome_binding_entry_new                    (guint                    keyval,
							       guint                    modifiers,
							       gchar                   *command);
void               gnome_binding_entry_destroy                (GnomeBindingEntry       *be);
GList             *gnome_binding_entry_list_copy              (GList                   *list);
void               gnome_binding_entry_list_destroy           (GList                   *list);

#endif
