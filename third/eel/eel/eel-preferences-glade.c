/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-glade.c - Some functions to connect a Glade-file to gconf keys.

   Copyright (C) 2002 Jan Arne Petersen

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Jan Arne Petersen <jpetersen@uni-bonn.de>
*/

#include "eel-preferences-glade.h"

#include <glib.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtktogglebutton.h>

#include "eel-preferences.h"
#include "eel-string-list.h"

#define EEL_PREFERENCES_GLADE_DATA_KEY "eel_preferences_glade_data_key"
#define EEL_PREFERENCES_GLADE_DATA_VALUE "eel_preferences_glade_data_value"
#define EEL_PREFERENCES_GLADE_DATA_MAP "eel_preferences_glade_data_map"
#define EEL_PREFERENCES_GLADE_DATA_WIDGETS "eel_preferences_glade_data_widgets"

/* helper */

static void
eel_preferences_glade_option_menu_update (GtkOptionMenu *option_menu,
					  gpointer value,
					  GCallback change_callback)
{
	GHashTable *map;
	int history;
	gpointer key;

	map = (GHashTable *) g_object_get_data (G_OBJECT (option_menu),
						EEL_PREFERENCES_GLADE_DATA_MAP);
	history = GPOINTER_TO_INT (g_hash_table_lookup (map, value));

	if (history == -1) {
		return;
	}

	key = g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_KEY);

	g_signal_handlers_block_by_func (option_menu, change_callback, key);
	gtk_option_menu_set_history (option_menu, history);
	g_signal_handlers_unblock_by_func (option_menu, change_callback, key);
}

/* bool preference */

static void
eel_preferences_glade_bool_toggled (GtkToggleButton *toggle_button,
				    char *key)
{
	eel_preferences_set_boolean (key, gtk_toggle_button_get_active (toggle_button));
}

static void
eel_preferences_glade_bool_update (GtkToggleButton *toggle_button)
{
	gboolean value;
	gpointer key;

	key = g_object_get_data (G_OBJECT (toggle_button), EEL_PREFERENCES_GLADE_DATA_KEY);

	value = eel_preferences_get_boolean (key);
	g_signal_handlers_block_by_func (toggle_button, eel_preferences_glade_bool_toggled, key);
	gtk_toggle_button_set_active (toggle_button, value);
	g_signal_handlers_unblock_by_func (toggle_button, eel_preferences_glade_bool_toggled, key);
}

void
eel_preferences_glade_connect_bool (GladeXML *dialog,
				    const char *component,
				    const char *key)
{
	GtkToggleButton *toggle_button;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (component != NULL);
	g_return_if_fail (key != NULL);

	toggle_button = GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog, component));
	g_object_set_data_full (G_OBJECT (toggle_button), EEL_PREFERENCES_GLADE_DATA_KEY,
				g_strdup (key), (GDestroyNotify) g_free);

	eel_preferences_add_callback_while_alive (key,
				      		  (EelPreferencesCallback) eel_preferences_glade_bool_update,
				      		  toggle_button, G_OBJECT (toggle_button));
	
	g_signal_connect (G_OBJECT (toggle_button), "toggled",
			  G_CALLBACK (eel_preferences_glade_bool_toggled),
			  g_object_get_data (G_OBJECT (toggle_button),
				  		       EEL_PREFERENCES_GLADE_DATA_KEY));

	eel_preferences_glade_bool_update (toggle_button);
}

void
eel_preferences_glade_connect_bool_slave (GladeXML *dialog,
					  const char *component,
					  const char *key)
{
	GtkToggleButton *toggle_button;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (component != NULL);
	g_return_if_fail (key != NULL);

	toggle_button = GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog, component));
	
	g_signal_connect_data (G_OBJECT (toggle_button), "toggled",
			       G_CALLBACK (eel_preferences_glade_bool_toggled),
			       g_strdup (key), (GClosureNotify) g_free, 0);
}

/* string enum (OptionMenu) preference */

static void
eel_preferences_glade_string_enum_option_menu_changed (GtkOptionMenu *option_menu,
						       char *key)
{
	int history;
	char **values;
	int i;

	history = gtk_option_menu_get_history (option_menu);
	values = g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_VALUE);

	i = 0;
	while (i < history && values[i] != NULL) {
		i++;
	}

	if (values[i] == NULL) {
		return;
	}

	eel_preferences_set (key, values[i]);
}

static void
eel_preferences_glade_string_enum_option_menu_update (GtkOptionMenu *option_menu)
{
	char *value;

	value = eel_preferences_get (g_object_get_data (G_OBJECT (option_menu),
							EEL_PREFERENCES_GLADE_DATA_KEY));

	eel_preferences_glade_option_menu_update (option_menu, value,
						  G_CALLBACK (eel_preferences_glade_string_enum_option_menu_changed));

	g_free (value);
}

void
eel_preferences_glade_connect_string_enum_option_menu (GladeXML *dialog,
						       const char *component,
						       const char *key,
						       const char **values)
{
	GtkWidget *option_menu;
	GHashTable *map;
	int i;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (component != NULL);
	g_return_if_fail (key != NULL);
	g_return_if_fail (values != NULL);
	
	option_menu = glade_xml_get_widget (dialog, component);

	map = g_hash_table_new_full (g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

	for (i = 0; values[i] != NULL; i++) {
		g_hash_table_insert (map, g_strdup (values[i]), GINT_TO_POINTER (i));
	}

	g_object_set_data_full (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_MAP, map,
				(GDestroyNotify) g_hash_table_destroy);
	g_object_set_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_VALUE, values);
	g_object_set_data_full (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_KEY,
				g_strdup (key), (GDestroyNotify) g_free);

	eel_preferences_add_callback_while_alive (key,
				 		  (EelPreferencesCallback) eel_preferences_glade_string_enum_option_menu_update,
						  option_menu, G_OBJECT (option_menu));

	g_signal_connect (G_OBJECT (option_menu), "changed",
			  G_CALLBACK (eel_preferences_glade_string_enum_option_menu_changed),
			  g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_KEY));

	eel_preferences_glade_string_enum_option_menu_update (GTK_OPTION_MENU (option_menu));
}

void
eel_preferences_glade_connect_string_enum_option_menu_slave (GladeXML *dialog,
							     const char *component,
							     const char *key)
{
	GtkWidget *option_menu;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (component != NULL);
	g_return_if_fail (key != NULL);
	
	option_menu = glade_xml_get_widget (dialog, component);

	g_assert (g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_MAP) != NULL);

	g_signal_connect_data (G_OBJECT (option_menu), "changed",
			       G_CALLBACK (eel_preferences_glade_string_enum_option_menu_changed),
			       g_strdup (key), (GClosureNotify) g_free, 0);
}

/* int enum preference */

static void
eel_preferences_glade_int_enum_changed (GtkOptionMenu *option_menu,
					char *key)
{
	int history;
	GSList *value_list;
	int i;

	history = gtk_option_menu_get_history (option_menu);
	value_list = (GSList *) g_object_get_data (G_OBJECT (option_menu),
						   EEL_PREFERENCES_GLADE_DATA_VALUE);

	i = 0;
	while (i < history && value_list->next != NULL) {
		i++;
		value_list = value_list->next;
	}

	if (GPOINTER_TO_INT (value_list->data) == -1) {
		return;
	}

	eel_preferences_set_integer (key, GPOINTER_TO_INT (value_list->data));
}

static void
eel_preferences_glade_int_enum_update (GtkOptionMenu *option_menu)
{
	int value;

	value = eel_preferences_get_integer (g_object_get_data (G_OBJECT (option_menu),
								EEL_PREFERENCES_GLADE_DATA_KEY));

	eel_preferences_glade_option_menu_update (option_menu, GINT_TO_POINTER (value),
						  G_CALLBACK (eel_preferences_glade_int_enum_changed));
}

void
eel_preferences_glade_connect_int_enum (GladeXML *dialog,
					const char *component,
					const char *key,
					const int *values)
{
	GHashTable *map;
	int i;
	int value;
	GtkOptionMenu *option_menu;
	GSList *value_list;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (component != NULL);
	g_return_if_fail (key != NULL);
	g_return_if_fail (values != NULL);
	
	option_menu = GTK_OPTION_MENU (glade_xml_get_widget (dialog, component));

	map = g_hash_table_new (g_direct_hash, g_direct_equal);
	value_list = NULL;

	for (i = 0; values[i] != -1; i++) {
		value = values[i];
		value_list = g_slist_append (value_list, GINT_TO_POINTER (value));
		g_hash_table_insert (map, GINT_TO_POINTER (value), GINT_TO_POINTER (i));
	}

	g_object_set_data_full (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_MAP, map,
				(GDestroyNotify) g_hash_table_destroy);
	g_object_set_data_full (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_VALUE, value_list,
				(GDestroyNotify) g_slist_free);
	g_object_set_data_full (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_KEY,
				g_strdup (key), (GDestroyNotify) g_free);

	g_signal_connect (G_OBJECT (option_menu), "changed",
			  G_CALLBACK (eel_preferences_glade_int_enum_changed),
			  g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_KEY));

	eel_preferences_add_callback_while_alive (key,
				 		  (EelPreferencesCallback) eel_preferences_glade_int_enum_update,
						  option_menu, G_OBJECT (option_menu));

	eel_preferences_glade_int_enum_update (option_menu);
}

/* String Enum (RadioButton) preference */

static void
eel_preferences_glade_string_enum_radio_button_toggled (GtkToggleButton *toggle_button,
							char *key)
{
	if (gtk_toggle_button_get_active (toggle_button) == FALSE) {
		return;
	}

	eel_preferences_set (key,
			     g_object_get_data (G_OBJECT (toggle_button),
				     		EEL_PREFERENCES_GLADE_DATA_VALUE));
}

static void
eel_preferences_glade_string_enum_radio_button_update (GtkWidget *widget)
{
	gpointer key;
	char *value;
	GHashTable *map;
	gpointer object;

	key = g_object_get_data (G_OBJECT (widget), EEL_PREFERENCES_GLADE_DATA_KEY);
	value = eel_preferences_get (key);
	map = g_object_get_data (G_OBJECT (widget), EEL_PREFERENCES_GLADE_DATA_MAP);
	object = g_hash_table_lookup (map, value);
	g_free (value);
	if (object == NULL) {
		return;
	}

	g_signal_handlers_block_by_func (widget,
					 eel_preferences_glade_string_enum_radio_button_toggled,
					 key);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (object), TRUE);
	g_signal_handlers_unblock_by_func (widget,
					   eel_preferences_glade_string_enum_radio_button_toggled,
					   key);
}

void
eel_preferences_glade_connect_string_enum_radio_button (GladeXML *dialog,
							const char **components,
							const char *key,
							const char **values)
{
	GHashTable *map;
	int i;
	GtkWidget *widget;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (components != NULL);
	g_return_if_fail (key != NULL);
	g_return_if_fail (values != NULL);

	map = g_hash_table_new_full (g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

	widget = NULL;
	for (i = 0; components[i] != NULL && values[i] != NULL; i++) {
		widget = glade_xml_get_widget (dialog, components[i]);
		g_hash_table_insert (map, g_strdup (values[i]), widget);
		if (i == 0) {
			g_object_set_data_full (G_OBJECT (widget),
						EEL_PREFERENCES_GLADE_DATA_MAP, map,
					        (GDestroyNotify) g_hash_table_destroy);
		} else {
			g_object_set_data (G_OBJECT (widget),
					   EEL_PREFERENCES_GLADE_DATA_MAP, map);
		}
		g_object_set_data_full (G_OBJECT (widget),
					EEL_PREFERENCES_GLADE_DATA_VALUE, g_strdup (values[i]),
					(GDestroyNotify) g_free);
		g_object_set_data_full (G_OBJECT (widget),
					EEL_PREFERENCES_GLADE_DATA_KEY, g_strdup (key),
					(GDestroyNotify) g_free);

		g_signal_connect (G_OBJECT (widget), "toggled",
				  G_CALLBACK (eel_preferences_glade_string_enum_radio_button_toggled),
				  g_object_get_data (G_OBJECT (widget),
					  	     EEL_PREFERENCES_GLADE_DATA_KEY));
	}

	eel_preferences_add_callback_while_alive (key,
						  (EelPreferencesCallback) eel_preferences_glade_string_enum_radio_button_update,
						  widget, G_OBJECT (widget));

	eel_preferences_glade_string_enum_radio_button_update (widget);
}

/* list enum preference */

static void
eel_preferences_glade_list_enum_changed (GtkOptionMenu *option_menu,
					 char *key)
{
	GSList *widgets;
	int history;
	char **values;
	int i;
	EelStringList *list;

	widgets = g_object_get_data (G_OBJECT (option_menu), EEL_PREFERENCES_GLADE_DATA_WIDGETS);

	list = eel_string_list_new (TRUE);
	for (; widgets != NULL; widgets = widgets->next) {
		history = gtk_option_menu_get_history (GTK_OPTION_MENU (widgets->data));
		values = g_object_get_data (G_OBJECT (option_menu),
					    EEL_PREFERENCES_GLADE_DATA_VALUE);

		i = 0;
		while (i < history && values[i] != NULL) {
			i++;
		}

		if (values[i] != NULL) {
			eel_string_list_insert (list, g_strdup (values[i]));
		}
	}

	eel_preferences_set_string_list (key, list);
	eel_string_list_free (list);
}

static void
eel_preferences_glade_list_enum_update (GtkWidget *widget)
{
	EelStringList *value_list;
	int values;
	GSList *components;
	int i;
	char *item;

	value_list = eel_preferences_get_string_list (g_object_get_data (G_OBJECT (widget),
									 EEL_PREFERENCES_GLADE_DATA_KEY));
	values = eel_string_list_get_length (value_list);
	components = g_object_get_data (G_OBJECT (widget), EEL_PREFERENCES_GLADE_DATA_WIDGETS);
	for (i = 0; i < values && components != NULL; i++, components = components->next) {
		item = eel_string_list_nth (value_list, i);
		eel_preferences_glade_option_menu_update (GTK_OPTION_MENU (components->data), 
							  item,
							  G_CALLBACK (eel_preferences_glade_list_enum_changed));
		g_free (item);
	}

	eel_string_list_free (value_list);
}

void 
eel_preferences_glade_connect_list_enum (GladeXML *dialog,
					 const char **components,
					 const char *key,
					 const char **values)
{
	GtkWidget *option_menu;
	GHashTable *map;
	int i;
	GSList *widgets;

 	g_return_if_fail (dialog != NULL);
	g_return_if_fail (components != NULL);
	g_return_if_fail (key != NULL);
	g_return_if_fail (values != NULL);
	
	map = g_hash_table_new_full (g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

	for (i = 0; values[i] != NULL; i++) {
		g_hash_table_insert (map, g_strdup (values[i]), GINT_TO_POINTER (i));
	}

	option_menu = NULL;
	widgets = NULL;
	for (i = 0; components[i] != NULL; i++) {
		option_menu = glade_xml_get_widget (dialog, components[i]);
		widgets = g_slist_append (widgets, option_menu);
		if (i == 0) {
			g_object_set_data_full (G_OBJECT (option_menu),
						EEL_PREFERENCES_GLADE_DATA_MAP, map,
						(GDestroyNotify) g_hash_table_destroy);
			g_object_set_data_full (G_OBJECT (option_menu),
						EEL_PREFERENCES_GLADE_DATA_WIDGETS,
						widgets, (GDestroyNotify) g_slist_free);
		} else {
			g_object_set_data (G_OBJECT (option_menu),
					   EEL_PREFERENCES_GLADE_DATA_MAP, map);
			g_object_set_data (G_OBJECT (option_menu),
					   EEL_PREFERENCES_GLADE_DATA_WIDGETS, widgets);
		}
		g_object_set_data (G_OBJECT (option_menu),
				   EEL_PREFERENCES_GLADE_DATA_VALUE, values);
		g_object_set_data_full (G_OBJECT (option_menu),
				        EEL_PREFERENCES_GLADE_DATA_KEY, g_strdup (key),
					(GDestroyNotify) g_free);

		g_signal_connect (G_OBJECT (option_menu), "changed",
			  	  G_CALLBACK (eel_preferences_glade_list_enum_changed),
				  g_object_get_data (G_OBJECT (option_menu),
					  	     EEL_PREFERENCES_GLADE_DATA_KEY));
	}

	eel_preferences_add_callback_while_alive (key,
						  (EelPreferencesCallback) eel_preferences_glade_list_enum_update,
						  option_menu, G_OBJECT (option_menu));
	
	eel_preferences_glade_list_enum_update (option_menu);
}
