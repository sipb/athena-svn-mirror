#define __GPA_SETTINGS_SELECTOR_C__

/*
 * GPASettingsSelector
 *
 * Simple OptonMenu for selecting settingss
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

/*
 * We have to listen:
 *
 * root - for printer disappearing
 * Printer - for printer setting list changes
 * Settings.id - for setting change
 *
 */

#include <config.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include "../gnome-print-i18n.h"
#include "settings-selector.h"

#define GPA_SS_DEBUG

static void gpa_settings_selector_class_init (GPASettingsSelectorClass *klass);
static void gpa_settings_selector_init (GPASettingsSelector *selector);
static void gpa_settings_selector_destroy (GtkObject *object);

static gint gpa_settings_selector_construct (GPAWidget *widget);

static void gpa_ss_rebuild_menu (GPASettingsSelector *ss);

static void gpa_ss_menuitem_activate (GtkWidget *widget, gint index);
static void gpa_ss_add_settings_activate (GtkWidget *widget, GPAWidget *gpaw);

static GPAWidgetClass *parent_class;

GtkType
gpa_settings_selector_get_type (void)
{
	static GtkType settings_selector_type = 0;
	if (!settings_selector_type) {
		static const GtkTypeInfo settings_selector_info = {
			"GPASettingsSelector",
			sizeof (GPASettingsSelector),
			sizeof (GPASettingsSelectorClass),
			(GtkClassInitFunc) gpa_settings_selector_class_init,
			(GtkObjectInitFunc) gpa_settings_selector_init,
			NULL, NULL, NULL
		};
		settings_selector_type = gtk_type_unique (GPA_TYPE_WIDGET, &settings_selector_info);
	}
	return settings_selector_type;
}

static void
gpa_settings_selector_class_init (GPASettingsSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GtkObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	object_class->destroy = gpa_settings_selector_destroy;

	gpa_class->construct = gpa_settings_selector_construct;
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
}

static void
gpa_settings_selector_destroy (GtkObject *object)
{
	GPASettingsSelector *ss;

	ss = (GPASettingsSelector *) object;

	while (ss->settingslist) {
		gpa_node_unref (GPA_NODE (ss->settingslist->data));
		ss->settingslist = g_slist_remove (ss->settingslist, ss->settingslist->data);
	}

	if (ss->printer) {
		gpa_node_unref (ss->printer);
		ss->printer = NULL;
	}

	if (ss->settings) {
		gpa_node_unref (ss->settings);
		ss->settings = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static gint
gpa_settings_selector_construct (GPAWidget *widget)
{
	GPASettingsSelector *ss;
	GPANode *node;

	ss = GPA_SETTINGS_SELECTOR (widget);
	node = GNOME_PRINT_CONFIG_NODE (widget->config);

	ss->printer = gpa_node_get_path_node (node, "Printer");
	ss->settings = gpa_node_get_path_node (node, "Settings");

	gpa_ss_rebuild_menu (ss);

	return TRUE;
}

static void
gpa_ss_rebuild_menu (GPASettingsSelector *ss)
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

	list = gpa_node_get_path_node (ss->printer, "Settings");
	if (!list) {
		item = gtk_menu_item_new_with_label (_("No settings available"));
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_widget_show (menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (ss->menu), menu);
		gtk_widget_set_sensitive (ss->menu, FALSE);
		return;
	}

	/* fixme: more cases */
	gtk_widget_set_sensitive (ss->menu, TRUE);

	index = 0;
	s = gpa_node_get_child (list, NULL);
	if (s != NULL) {
		while (s != NULL) {
			GPANode *next;
			gchar *name;
			name = gpa_node_get_path_value (s, "Name");
			if (name != NULL) {
				gpa_node_ref (s);
				ss->settingslist = g_slist_prepend (ss->settingslist, s);
				item = gtk_menu_item_new_with_label (name);
				gtk_object_set_data (GTK_OBJECT (item), "GPAWidget", ss);
				gtk_signal_connect (GTK_OBJECT (item), "activate",
						    GTK_SIGNAL_FUNC (gpa_ss_menuitem_activate), GINT_TO_POINTER (index));
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
	gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (gpa_ss_add_settings_activate), ss);
	/* fixme: */
	gtk_widget_set_sensitive (item, FALSE);
	gtk_widget_show (item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ss->menu), menu);
}

static void
gpa_ss_menuitem_activate (GtkWidget *widget, gint index)
{
#if 0
	GPAWidget *gpaw;
	GPANode *settings;
	gchar *value;

	gpaw = gtk_object_get_data (GTK_OBJECT (widget), "GPAWidget");
	g_return_if_fail (gpaw != NULL);
	g_return_if_fail (GPA_IS_WIDGET (gpaw));

	settings = g_slist_nth_data (GPA_SETTINGS_SELECTOR (gpaw)->settingslist, index);
	g_return_if_fail (settings != NULL);
	g_return_if_fail (GPA_IS_NODE (settings));

	value = gpa_node_get_value (settings);
	g_return_if_fail (value != NULL);

	gpa_node_set_path_value (gpaw->node, "Settings", value);

	g_free (value);
#else
	g_print ("Settings selector (%d) activated\n", index);
#endif
}

static void
gpa_ss_add_settings_activate (GtkWidget *widget, GPAWidget *gpaw)
{
#ifdef __GNUC__
#warning Implement add settings
#endif	
#if 0
	GtkWidget *apd;

	apd = gpa_add_settings_dialog_new (gpaw->node);

	gtk_widget_show (apd);
#else
	g_print ("Implement me. Add settings\n");
#endif
}

