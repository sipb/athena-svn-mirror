/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-settings-selector.c: A simple Optionmenu for selecting settings
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors :
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-settings-selector.h"
#include "libgnomeprint/private/gnome-print-config-private.h"

static void gpa_settings_selector_class_init (GPASettingsSelectorClass *klass);
static void gpa_settings_selector_init (GPASettingsSelector *selector);
static void gpa_settings_selector_finalize (GObject *object);
static gint gpa_settings_selector_construct (GPAWidget *widget);

static void gpa_settings_selector_rebuild_menu (GPASettingsSelector *ss);
static void gpa_settings_selector_menuitem_activate (GtkWidget *widget, gint index);
static void gpa_settings_selector_add_settings_activate (GtkWidget *widget, GPAWidget *gpaw);

static GPAWidgetClass *parent_class;

GType
gpa_settings_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPASettingsSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_settings_selector_class_init,
			NULL, NULL,
			sizeof (GPASettingsSelector),
			0,
			(GInstanceInitFunc) gpa_settings_selector_init
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPASettingsSelector", &info, 0);
	}
	return type;
}

static void
gpa_settings_selector_class_init (GPASettingsSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	gpa_class    = (GPAWidgetClass *) klass;
	object_class = (GObjectClass *) klass;
	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	object_class->finalize = gpa_settings_selector_finalize;
	
	gpa_class->construct  = gpa_settings_selector_construct;
}

static void
gpa_settings_selector_init (GPASettingsSelector *selector)
{
	selector->menu = gtk_option_menu_new ();
	gtk_container_add (GTK_CONTAINER (selector), selector->menu);
	gtk_widget_show (selector->menu);

	selector->printer = NULL;
	selector->settings = NULL;
	selector->settingslist = NULL;
	selector->handler = 0;
}

static void
gpa_settings_selector_finalize (GObject *object)
{
	GPASettingsSelector *ss;

	ss = (GPASettingsSelector *) object;

	while (ss->settingslist) {
		gpa_node_unref (GPA_NODE (ss->settingslist->data));
		ss->settingslist = g_slist_remove (ss->settingslist, ss->settingslist->data);
	}

	if (ss->handler) {
		g_signal_handler_disconnect (G_OBJECT (ss->printer), ss->handler);
		ss->handler = 0;
	}
	
	if (ss->printer) {
		gpa_node_unref (ss->printer);
		ss->printer = NULL;
	}

	if (ss->settings) {
		gpa_node_unref (ss->settings);
		ss->settings = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_settings_selector_printer_changed_cb (GPANode *node, guint flags, gpointer ss_ptr)
{
	GPASettingsSelector *ss = ss_ptr;

	g_return_if_fail (GPA_IS_SETTINGS_SELECTOR (ss));

	node = GNOME_PRINT_CONFIG_NODE (GPA_WIDGET (ss)->config);

	gpa_node_unref (ss->printer);
	gpa_node_unref (ss->settings);
	
	ss->printer  = gpa_node_get_child_from_path (node, "Printer");
	ss->settings = gpa_node_get_child_from_path (node, "Settings");

	gpa_settings_selector_rebuild_menu (ss);
}

static gint
gpa_settings_selector_construct (GPAWidget *widget)
{
	GPASettingsSelector *ss;
	GPANode *node;

	ss = GPA_SETTINGS_SELECTOR (widget);
	node = GNOME_PRINT_CONFIG_NODE (widget->config);

	ss->printer  = gpa_node_get_child_from_path (node, "Printer");
	ss->settings = gpa_node_get_child_from_path (node, "Settings");

	g_assert (ss->printer);
	g_assert (ss->settings);
	
	ss->handler = g_signal_connect (G_OBJECT (ss->printer), "modified",
					(GCallback) gpa_settings_selector_printer_changed_cb, ss);
	
	gpa_settings_selector_rebuild_menu (ss);

	return TRUE;
}

static void
gpa_settings_selector_rebuild_menu (GPASettingsSelector *ss)
{
	GPANode *list, *s;
	GtkWidget *menu, *item;
	gint index;

	/* Clear menu and settingslist */
	while (ss->settingslist) {
		gpa_node_unref (GPA_NODE (ss->settingslist->data));
		ss->settingslist = g_slist_remove (ss->settingslist, ss->settingslist->data);
	}
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (ss->menu));

	menu = gtk_menu_new ();

	if (!ss->printer) {
		item = gtk_menu_item_new_with_label (_("No printer selected"));
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_widget_show (menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (ss->menu), menu);
		gtk_widget_set_sensitive (ss->menu, FALSE);
		return;
	}

	list = gpa_node_get_child_from_path (ss->printer, "Settings");
	if (!list) {
		item = gtk_menu_item_new_with_label (_("No settings available"));
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_widget_show (menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (ss->menu), menu);
		gtk_widget_set_sensitive (ss->menu, FALSE);
		return;
	}

	/* FIXME: more cases (Lauris) */
	gtk_widget_set_sensitive (ss->menu, TRUE);

	index = 0;
	s = gpa_node_get_child (list, NULL);
	if (s != NULL) {
		while (s != NULL) {
			GPANode *next;
			gchar *name;
			name = gpa_node_get_value (s);
			if (name != NULL) {
				gpa_node_ref (s);
				ss->settingslist = g_slist_prepend (ss->settingslist, s);
				item = gtk_menu_item_new_with_label (name);
				g_object_set_data (G_OBJECT (item), "GPAWidget", ss);
				g_signal_connect (G_OBJECT (item), "activate",
						  (GCallback) gpa_settings_selector_menuitem_activate, GINT_TO_POINTER (index));
				gtk_widget_show (item);
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
				g_free (name);
				index += 1;
			}
			next = gpa_node_get_child (list, s);
			gpa_node_unref (s);
			s = next;
		}
		ss->settingslist = g_slist_reverse (ss->settingslist);
		item = gtk_menu_item_new ();
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}

	gpa_node_unref (list);

	item = gtk_menu_item_new_with_label (_("Add new settings"));
	g_signal_connect (G_OBJECT (item), "activate",
			  (GCallback) gpa_settings_selector_add_settings_activate, ss);
	/* FIXME: (Lauris) */
	gtk_widget_set_sensitive (item, FALSE);
	gtk_widget_show (item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ss->menu), menu);
}

static void
gpa_settings_selector_menuitem_activate (GtkWidget *widget, gint index)
{
#if 0
	GPAWidget *gpaw;
	GPANode *settings;
	gchar *value;

	gpaw = gtk_object_get_data (G_OBJECT (widget), "GPAWidget");
	g_return_if_fail (gpaw != NULL);
	g_return_if_fail (GPA_IS_WIDGET (gpaw));

	settings = g_slist_nth_data (GPA_SETTINGS_SELECTOR (gpaw)->settingslist, index);
	g_return_if_fail (settings != NULL);
	g_return_if_fail (GPA_IS_NODE (settings));

	value = gpa_node_get_value (settings);
	g_return_if_fail (value != NULL);

	gpa_node_set_path_value (gpaw->node, "Settings", value);

	g_free (value);
#endif
}

static void
gpa_settings_selector_add_settings_activate (GtkWidget *widget, GPAWidget *gpaw)
{
/* FIXME: Implement add settings (Chema) */
#if 0
	GtkWidget *apd;

	apd = gpa_add_settings_dialog_new (gpaw->node);

	gtk_widget_show (apd);
#endif
}

