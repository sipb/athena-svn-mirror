/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-printer-selector.c: A simple Optionmenu for selecting printers
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

#include <config.h>

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-printer-selector.h"
#include "libgnomeprint/private/gnome-print-config-private.h"
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-list.h>
#include <libgnomeprint/private/gpa-printer.h>
#include <libgnomeprint/private/gpa-root.h>
#include <libgnomeprint/private/gpa-config.h>

static void gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass);
static void gpa_printer_selector_init (GPAPrinterSelector *selector);
static void gpa_printer_selector_finalize (GObject *object);

static void     gpa_printer_selector_rebuild_menu (GPAPrinterSelector *ps);
static gboolean gpa_printer_selector_construct (GPAWidget *gpa);

static GPAWidgetClass *parent_class;

GtkType
gpa_printer_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAPrinterSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_printer_selector_class_init,
			NULL, NULL,
			sizeof (GPAPrinterSelector),
			0,
			(GInstanceInitFunc) gpa_printer_selector_init
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPAPrinterSelector", &info, 0);
	}
	return type;
}

static void
gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);
	gpa_class->construct = gpa_printer_selector_construct;
	object_class->finalize = gpa_printer_selector_finalize;
}

static void
gpa_printer_selector_init (GPAPrinterSelector *ps)
{
	ps->menu = gtk_option_menu_new ();
	gtk_container_add (GTK_CONTAINER (ps), ps->menu);
	gtk_widget_show (ps->menu);
}

static void
gpa_printer_selector_finalize (GObject *object)
{
	GPAPrinterSelector *ps;

	ps = (GPAPrinterSelector *) object;

	gpa_node_unref (ps->node);
	ps->node = NULL;

	if (ps->handler_config) 
		g_signal_handler_disconnect (ps->config, ps->handler_config);
	ps->handler_config = 0;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_printer_selector_item_activate_cb (GtkMenuItem *item, GPAPrinterSelector *ps)
{
	GPANode *printer;

 	printer = g_object_get_data (G_OBJECT (item), "node");

	ps->updating = TRUE;
	gpa_node_set_path_value (ps->config, "Printer", gpa_node_id (printer));
	ps->updating = FALSE;
}

static void
gpa_printer_selector_rebuild_menu (GPAPrinterSelector *ps)
{
	GtkWidget *menu, *item;
	GPANode *child;
	GPANode *def;
	gint pos = 0;
	gint sel = -1;

	menu = gtk_menu_new ();
	gtk_widget_show (menu);

	def = GPA_REFERENCE_REFERENCE (GPA_CONFIG (ps->config)->printer);

	child = gpa_node_get_child (ps->node, NULL);
	while (child) {
		const guchar *id;
		guchar *name;
		name = gpa_node_get_value (child);
		item = gtk_menu_item_new_with_label (name);
		g_free (name);

		id = gpa_node_id (child);
		
		g_signal_connect (G_OBJECT (item), "activate",
				  (GCallback) gpa_printer_selector_item_activate_cb, ps);
		gpa_node_ref (child);
		g_object_set_data (G_OBJECT (item), "node", child);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		if (child == def)
			sel = pos;
		child = gpa_node_get_child (ps->node, child);
		pos++;
	}
	if (pos < 1) {
		item = gtk_menu_item_new_with_label (_("No printers could be loaded"));
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}
	if (sel == -1) {
		g_warning ("rebuild_menu_cb, could not set value of %s to %s",
			   gpa_node_id (ps->node), gpa_node_id (def));
		sel = 0;
	}

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ps->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (ps->menu), sel);
}

static void
gpa_printer_selector_config_modified_cb (GPANode *node, guint flags, GPAPrinterSelector *ps)
{
	if (ps->updating)
		return;

	gpa_printer_selector_rebuild_menu (ps);
}

static gboolean
gpa_printer_selector_construct (GPAWidget *gpa)
{
	GPAPrinterSelector *ps;

	ps = GPA_PRINTER_SELECTOR (gpa);
	ps->config = GNOME_PRINT_CONFIG_NODE (gpa->config);
	ps->node   = GPA_NODE (gpa_get_printers ());
	ps->handler_config  = g_signal_connect (G_OBJECT (ps->config), "modified", (GCallback)
						gpa_printer_selector_config_modified_cb, ps);

	gpa_printer_selector_rebuild_menu (ps);

	return TRUE;
}

