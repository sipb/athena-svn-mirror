/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-transport-selector.c: A print transport selector
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
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc. 
 *
 */

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-transport-selector.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>

static void gpa_transport_selector_class_init (GPATransportSelectorClass *klass);
static void gpa_transport_selector_init (GPATransportSelector *selector);
static void gpa_transport_selector_finalize (GObject *object);
static gint gpa_transport_selector_construct (GPAWidget *widget);

static void gpa_transport_selector_file_entry_changed_cb       (GtkEntry *entry, GPATransportSelector *ts);
static void gpa_transport_selector_custom_entry_changed_cb (GtkEntry *entry, GPATransportSelector *ts);
static void gpa_transport_selector_node_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts);

static GPAWidgetClass *parent_class;

GtkType
gpa_transport_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPATransportSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_transport_selector_class_init,
			NULL, NULL,
			sizeof (GPATransportSelector),
			0,
			(GInstanceInitFunc) gpa_transport_selector_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPATransportSelector", &info, 0);
	}
	return type;
}

static void
gpa_transport_selector_class_init (GPATransportSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_transport_selector_construct;
	object_class->finalize = gpa_transport_selector_finalize;
}

static void
gpa_transport_selector_init (GPATransportSelector *ts)
{
	GtkBox *hbox;

	hbox = (GtkBox *) gtk_hbox_new (FALSE, 4);

	ts->menu = gtk_option_menu_new ();
	ts->file_entry   = gtk_entry_new ();
	ts->custom_entry = gtk_entry_new ();

	g_signal_connect (G_OBJECT (ts->file_entry), "changed", (GCallback)
			  gpa_transport_selector_file_entry_changed_cb, ts);
	g_signal_connect (G_OBJECT (ts->custom_entry), "changed", (GCallback)
			  gpa_transport_selector_custom_entry_changed_cb, ts);

	gtk_box_pack_start (hbox, ts->menu,         FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, ts->file_entry,   FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, ts->custom_entry, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER (ts), GTK_WIDGET (hbox));
	gtk_widget_show_all (GTK_WIDGET (hbox));
}

static void
gpa_transport_selector_disconnect (GPATransportSelector *ts)
{
	if (ts->handler) {
		g_signal_handler_disconnect (ts->node, ts->handler);
		ts->handler = 0;
	}
	
	if (ts->node) {
		gpa_node_unref (ts->node);
		ts->node = NULL;
	}
}

static void
gpa_transport_selector_finalize (GObject *object)
{
	GPATransportSelector *ts;

	ts = (GPATransportSelector *) object;

	gpa_transport_selector_disconnect (ts);

	if (ts->handler_config)
		g_signal_handler_disconnect (ts->config, ts->handler_config);
	ts->handler_config = 0;
	ts->config = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_transport_selector_file_entry_changed_cb (GtkEntry *entry, GPATransportSelector *ts)
{
	const guchar *text;

	if (ts->updating)
		return;
				
	text = gtk_entry_get_text (entry);
	ts->updating = TRUE;
	gpa_node_set_path_value (ts->node, "FileName", text);
	ts->updating = FALSE;
}

static void
gpa_transport_selector_custom_entry_changed_cb (GtkEntry *entry, GPATransportSelector *ts)
{
	const guchar *text;

	if (ts->updating)
		return;

	text = gtk_entry_get_text (entry);
	ts->updating = TRUE;
	gpa_node_set_path_value (ts->node, "Command", text);
	ts->updating = FALSE;
}

static void
gpa_transport_selector_update_widgets (GPATransportSelector *ts)
{
	gchar *backend, *filename, *command;

	backend  = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend");
	filename = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend.FileName");
	command  = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend.Command");
	
	gtk_widget_hide (ts->file_entry);
	gtk_widget_hide (ts->custom_entry);

	if (backend && !strcmp (backend, "file")) {
		ts->updating = TRUE;
		gtk_entry_set_text (GTK_ENTRY (ts->file_entry), filename ? filename : "gnome-print.out");
		ts->updating = FALSE;
		gtk_widget_show (ts->file_entry);
	}
	
	if (backend && !strcmp (backend, "custom")) {
		ts->updating = TRUE;
		gtk_entry_set_text (GTK_ENTRY (ts->custom_entry), command ? command : "lpr %f");
		ts->updating = FALSE;
		gtk_widget_show (ts->custom_entry);
	}

	my_g_free (filename);
	my_g_free (command);
	my_g_free (backend);
}

static void
gpa_transport_selector_item_activate_cb (GtkWidget *item, GPATransportSelector *ts)
{
	GPANode *node;

	node =  g_object_get_data (G_OBJECT (item), "node");
	ts->updating = TRUE;
	gpa_node_set_value (ts->node, gpa_node_id (node));
	ts->updating = FALSE;

	gpa_transport_selector_update_widgets (ts);
}

static void
gpa_transport_selector_rebuild_menu (GPATransportSelector *ts)
{
	GtkWidget *menu, *item;
	GPANode *option, *child;
	guchar *def;
	gint pos = 0;
	gint sel = -1;

	menu = gtk_menu_new ();
	gtk_widget_show (menu);

	option = GPA_KEY (ts->node)->option;
	def = gpa_node_get_value (ts->node);

	child = gpa_node_get_child (option, NULL);
	while (child) {
		const guchar *id;
		guchar *name;

		name = gpa_node_get_value (child);
		item = gtk_menu_item_new_with_label (name);
		g_free (name);

		id = gpa_node_id (child);

		g_signal_connect (G_OBJECT (item), "activate", (GCallback) gpa_transport_selector_item_activate_cb, ts);
		g_object_set_data_full (G_OBJECT (item), "node", child, (GtkDestroyNotify) gpa_node_unref);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		if (GPA_NODE_ID_COMPARE (child, def))
		    sel = pos;
		
		pos++;
		child = gpa_node_get_child (option, child);
	}
	if (pos < 1) {
		item = gtk_menu_item_new_with_label (_("No options are defined"));
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}
	if (sel == -1) {
		g_warning ("rebuild_menu_cb, could not set value of %s to %s",
			   gpa_node_id (option), def);
		sel = 0;
	}

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ts->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (ts->menu), sel);
	g_free (def);

	gpa_transport_selector_update_widgets (ts);
}

static void
gpa_transport_selector_connect (GPATransportSelector *ts)
{
	ts->node = gpa_node_lookup (ts->config, "Settings.Transport.Backend");
	ts->handler = g_signal_connect (G_OBJECT (ts->node), "modified", (GCallback)
					gpa_transport_selector_node_modified_cb, ts);
}

static void
gpa_transport_selector_config_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts)
{
	gpa_transport_selector_disconnect (ts);
	gpa_transport_selector_connect (ts);
	gpa_transport_selector_rebuild_menu (ts);
}

static void
gpa_transport_selector_node_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts)
{
	if (ts->updating)
		return;
	
	gpa_transport_selector_rebuild_menu (ts);
}

static gint
gpa_transport_selector_construct (GPAWidget *gpaw)
{
	GPATransportSelector *ts;

	ts = GPA_TRANSPORT_SELECTOR (gpaw);
	ts->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	ts->handler_config = g_signal_connect (G_OBJECT (ts->config), "modified", (GCallback)
					       gpa_transport_selector_config_modified_cb, ts);

	gpa_transport_selector_connect (ts);
	gpa_transport_selector_rebuild_menu (ts);

	return TRUE;
}

