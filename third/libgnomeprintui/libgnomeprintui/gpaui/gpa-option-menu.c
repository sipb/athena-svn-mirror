/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-option-menu.c:
 *
 * Libgnomeprint is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Libgnomeprint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the libgnomeprint; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors :
 *   Chema Celorio <chema@ximian.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001-2003 Ximian, Inc. 
 *
 */

#include <config.h>

#include <string.h>
#include <gtk/gtk.h>

#include "gpa-widget.h"
#include "gpa-option-menu.h"
#include "gnome-print-i18n.h"
#include "libgnomeprint/private/gnome-print-config-private.h"
#include "libgnomeprint/private/gpa-node.h"
#include "libgnomeprint/private/gpa-node-private.h"
#include "libgnomeprint/private/gpa-key.h"
#include "libgnomeprint/private/gpa-option.h"

static void     gpa_option_menu_class_init (GPAOptionMenuClass *klass);
static void     gpa_option_menu_init (GPAOptionMenu *om);
static void     gpa_option_menu_finalize (GObject *object);
static gboolean gpa_option_menu_mnemonic_activate (GtkWidget *widget,
						   gboolean group_cycling);
static GPAWidgetClass *parent_class;

GType
gpa_option_menu_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAOptionMenuClass),
			NULL, NULL,
			(GClassInitFunc) gpa_option_menu_class_init,
			NULL, NULL,
			sizeof (GPAOptionMenu),
			0,
			(GInstanceInitFunc) gpa_option_menu_init
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPAOptionMenu", &info, 0);
	}
	return type;
}

static void
gpa_option_menu_class_init (GPAOptionMenuClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *gtk_class;

	object_class = (GObjectClass *)   klass;
	gtk_class    = (GtkWidgetClass *) klass;
	parent_class = gtk_type_class (GPA_TYPE_WIDGET);
	object_class->finalize       = gpa_option_menu_finalize;
	gtk_class->mnemonic_activate = gpa_option_menu_mnemonic_activate;
}

static void
gpa_option_menu_init (GPAOptionMenu *om)
{
	om->menu = gtk_option_menu_new ();
	gtk_container_add (GTK_CONTAINER (om), om->menu);
	gtk_widget_show (GTK_WIDGET (om->menu));
}

static void
gpa_option_menu_disconnect (GPAOptionMenu *om)
{
	if (om->handler) {
		g_signal_handler_disconnect (om->node, om->handler);
		om->handler = 0;
	}

	if (om->node) {
		gpa_node_unref (om->node);
		om->node = NULL;
	}
}

static void
gpa_option_menu_finalize (GObject *object)
{
	GPAOptionMenu *om;

	om = GPA_OPTION_MENU (object);

	gpa_option_menu_disconnect (om);

	if (om->handler_config)
		g_signal_handler_disconnect (om->config, om->handler_config);
	om->handler_config = 0;
	om->config = NULL;

	g_free (om->key);
	om->key = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_option_menu_mnemonic_activate (GtkWidget *widget,
				   gboolean group_cycling)
{
	gtk_widget_grab_focus (GPA_OPTION_MENU (widget)->menu);
	return TRUE;
}

static void
gpa_option_menu_item_activate_cb (GtkMenuItem *item, GPAOptionMenu *om)
{
	GPANode *node;

	om->updating = TRUE;
	node = g_object_get_data (G_OBJECT (item), "node");
	gpa_node_set_value (om->node, gpa_node_id (node));
	om->updating = FALSE;
}

static void
gpa_option_menu_rebuild_menu (GPAOptionMenu *om)
{
	GtkWidget *menu, *item;
	GPANode *option, *child;
	gchar *def;
	gint pos = 0;
	gint sel = -1;

	menu = gtk_menu_new ();
	gtk_widget_show (menu);

	option = GPA_KEY (om->node)->option;
	def = gpa_node_get_value (om->node);

	child = gpa_node_get_child (option, NULL);
	while (child) {
		const gchar *id;
		guchar *name;

		name = gpa_option_get_name (child);
		item = gtk_menu_item_new_with_label (name);
		g_free (name);

		id = gpa_node_id (child);
		
		g_signal_connect (G_OBJECT (item), "activate", (GCallback) gpa_option_menu_item_activate_cb, om);
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
	gtk_option_menu_set_menu (GTK_OPTION_MENU (om->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (om->menu), sel);
	g_free (def);
}

/**
 * gpa_option_menu_node_modified_cb:
 * @node: 
 * @flags: 
 * @om: 
 * 
 * Callback for when the value of the node we are a view/control
 * for changes.
 **/
static void
gpa_option_menu_node_modified_cb (GPANode *node, guint flags, GPAOptionMenu *om)
{
	if (om->updating)
		return;

	gpa_option_menu_rebuild_menu (om);
}

static void
gpa_option_menu_connect (GPAOptionMenu *om)
{
	GPANode *node;

	g_assert (om->node == NULL);
	g_assert (om->handler == 0);
	g_assert (om->updating == FALSE);
	
	node = gpa_node_lookup (om->config, om->key);
	if (!node) {
		GtkWidget *menu, *item;
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (om->menu));
		menu = gtk_menu_new ();
		gtk_widget_show (menu);
		item = gtk_menu_item_new_with_label (_("No options are defined"));
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (om->menu), menu);
		return;
	}
	
	om->node = node;
	om->handler = g_signal_connect (G_OBJECT (node), "modified",
					(GCallback) gpa_option_menu_node_modified_cb, om);
	gpa_option_menu_rebuild_menu (om);
}

/**
 * gpa_option_menu_config_modified_cb:
 * @node: 
 * @flags: 
 * @om: 
 * 
 * Callback for "modified" signal of GPAConfig, called when either the active printer
 * or settings has changed.
 * 
 **/
static void
gpa_option_menu_config_modified_cb (GPANode *node, guint flags, GPAOptionMenu *om)
{
	gpa_option_menu_disconnect (om);
	gpa_option_menu_connect (om);
}

/**
 * gpa_option_menu_new:
 * @gpc: 
 * @key: 
 * 
 * Craete a new Option menu that selects the node pointed by @key.
 * 
 * Return Value: A new widget, NULL on error
 **/
GtkWidget *
gpa_option_menu_new (GnomePrintConfig *gpc, const guchar *key)
{
	GPAOptionMenu *om;
	
	g_return_val_if_fail (gpc != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	om = (GPAOptionMenu *) gpa_widget_new (GPA_TYPE_OPTION_MENU, gpc);
	om->key    = g_strdup (key);
	om->config = GNOME_PRINT_CONFIG_NODE (gpc);
	om->handler_config = g_signal_connect (G_OBJECT (om->config), "modified",
					       (GCallback) gpa_option_menu_config_modified_cb, om);
	/* FIXME: do we ref om->config? what happens if it dies? (Chema) */
	gpa_option_menu_connect (om);

	return (GtkWidget *)om;
}

