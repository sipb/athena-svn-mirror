#define __GPA_PRINTER_SELECTOR_C__

/*
 * GPAPrinterSelector
 *
 * Simple OptonMenu for selecting printers
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include "../gnome-print-i18n.h"
#include "printer-selector.h"

static void gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass);
static void gpa_printer_selector_init (GPAPrinterSelector *selector);
static void gpa_printer_selector_destroy (GtkObject *object);

static gint gpa_printer_selector_construct (GPAWidget *widget);

static void gpa_ps_rebuild_menu (GPAPrinterSelector *ps);

static void gpa_ps_menuitem_activate (GtkWidget *widget, gint index);
static void gpa_ps_add_printer_activate (GtkWidget *widget, GPAWidget *gpaw);
static void gpa_ps_manage_printers_activate (GtkWidget *widget, GPAWidget *gpaw);

static GPAWidgetClass *parent_class;

GtkType
gpa_printer_selector_get_type (void)
{
	static GtkType printer_selector_type = 0;
	if (!printer_selector_type) {
		static const GtkTypeInfo printer_selector_info = {
			"GPAPrinterSelector",
			sizeof (GPAPrinterSelector),
			sizeof (GPAPrinterSelectorClass),
			(GtkClassInitFunc) gpa_printer_selector_class_init,
			(GtkObjectInitFunc) gpa_printer_selector_init,
			NULL, NULL, NULL
		};
		printer_selector_type = gtk_type_unique (GPA_TYPE_WIDGET, &printer_selector_info);
	}
	return printer_selector_type;
}

static void
gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GtkObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	object_class->destroy = gpa_printer_selector_destroy;

	gpa_class->construct = gpa_printer_selector_construct;
}

static void
gpa_printer_selector_init (GPAPrinterSelector *selector)
{
	selector->menu = gtk_option_menu_new ();
	gtk_container_add (GTK_CONTAINER (selector), selector->menu);
	gtk_widget_show (selector->menu);
	selector->printers = NULL;
	selector->printerlist = NULL;
}

static void
gpa_printer_selector_destroy (GtkObject *object)
{
	GPAPrinterSelector *ps;

	ps = (GPAPrinterSelector *) object;

	while (ps->printerlist) {
		gpa_node_unref (GPA_NODE (ps->printerlist->data));
		ps->printerlist = g_slist_remove (ps->printerlist, ps->printerlist->data);
	}

	if (ps->printers) {
		gpa_node_unref (ps->printers);
		ps->printers = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static gint
gpa_printer_selector_construct (GPAWidget *widget)
{
	GPAPrinterSelector *selector;
	GPANode *node;

	selector = GPA_PRINTER_SELECTOR (widget);
	node = GNOME_PRINT_CONFIG_NODE (widget->config);

	selector->printers = gpa_node_get_path_node (node, "Globals.Printers");
	g_return_val_if_fail (selector->printers != NULL, FALSE);

	gpa_ps_rebuild_menu (selector);

	return TRUE;
}

static void
gpa_ps_rebuild_menu (GPAPrinterSelector *ps)
{
	GPANode *printer;
	GtkWidget *menu, *item;
	GSList *l;
	guchar *defid;
	gint idx, def;

	g_return_if_fail (ps->printers != NULL);

	/* selector->printerlist */
	while (ps->printerlist) {
		printer = GPA_NODE (ps->printerlist->data);
		gpa_node_unref (printer);
		ps->printerlist = g_slist_remove (ps->printerlist, printer);
	}
	/* Clear menu */
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (ps->menu));

	/* Construct local printer list */
	for (printer = gpa_node_get_child (ps->printers, NULL); printer != NULL; printer = gpa_node_get_child (ps->printers, printer)) {
		/* printer IS referenced */
		ps->printerlist = g_slist_prepend (ps->printerlist, printer);
	}

	/* Get id of default printer */
	defid = gpa_node_get_path_value (ps->printers, "Default");

	menu = gtk_menu_new ();

	ps->printerlist = g_slist_reverse (ps->printerlist);

	/* Construct menu */
	idx = 0;
	def = 0;
	for (l = ps->printerlist; l != NULL; l = l->next) {
		gchar *val;
		printer = GPA_NODE (l->data);
		val = gpa_node_get_path_value (printer, "Name");
		if (!val) {
			g_warning ("Printer does not have 'Name' attribute");
		} else {
			item = gtk_menu_item_new_with_label (val);
			gtk_object_set_data (GTK_OBJECT (item), "GPAWidget", ps);
			gtk_signal_connect (GTK_OBJECT (item), "activate",
					    GTK_SIGNAL_FUNC (gpa_ps_menuitem_activate), GINT_TO_POINTER (idx));
			gtk_widget_show (item);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			g_free (val);
			if (defid) {
				guchar *id;
				id = gpa_node_get_value (printer);
				if (!strcmp (id, defid)) def = idx;
				g_free (id);
			}
			idx += 1;
		}
	}
	if (defid) g_free (defid);

	/* Append special menuitems */
	if (idx > 0) {
		/* At least one printer */
		item = gtk_menu_item_new ();
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		item = gtk_menu_item_new_with_label (_("Manage printers"));
		gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (gpa_ps_manage_printers_activate), ps);
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	} else {
		/* No printers */
		item = gtk_menu_item_new_with_label (_("Add new printer"));
		gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC (gpa_ps_add_printer_activate), ps);
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ps->menu), menu);

	gtk_option_menu_set_history (GTK_OPTION_MENU (ps->menu), def);
}

static void
gpa_ps_menuitem_activate (GtkWidget *widget, gint index)
{
	GPAWidget *gpaw;
	GPANode *printer;
	gchar *value;

	gpaw = gtk_object_get_data (GTK_OBJECT (widget), "GPAWidget");
	g_return_if_fail (gpaw != NULL);
	g_return_if_fail (GPA_IS_WIDGET (gpaw));

	printer = g_slist_nth_data (GPA_PRINTER_SELECTOR (gpaw)->printerlist, index);
	g_return_if_fail (printer != NULL);
	g_return_if_fail (GPA_IS_NODE (printer));

	value = gpa_node_get_value (printer);
	g_return_if_fail (value != NULL);

	gnome_print_config_set (gpaw->config, "Printer", value);

	g_free (value);
}

static void
gpa_ps_add_printer_activate (GtkWidget *widget, GPAWidget *gpaw)
{
	g_print ("Add printer activated\n");
}

static void
gpa_ps_manage_printers_activate (GtkWidget *widget, GPAWidget *gpaw)
{
	g_print ("Manage printers activated\n");
}

