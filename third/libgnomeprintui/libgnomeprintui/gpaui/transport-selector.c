#define __GPA_TRANSPORT_SELECTOR_C__

/*
 * GPATransportSelector
 *
 * Transport selector for gnome-print
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
#include <gtk/gtkhbox.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkradiobutton.h>
#include "../gnome-print-i18n.h"
#include "transport-selector.h"

static void gpa_transport_selector_class_init (GPATransportSelectorClass *klass);
static void gpa_transport_selector_init (GPATransportSelector *selector);
static void gpa_transport_selector_destroy (GtkObject *object);

static gint gpa_transport_selector_construct (GPAWidget *widget);

static void gpa_ts_rebuild_widget (GPATransportSelector *ts);

static void gpa_ts_fentry_changed (GtkEntry *entry, GPAWidget *gpw);
static void gpa_ts_rbentry_changed (GtkEntry *entry, GPAWidget *gpw);
static void gpa_ts_centry_changed (GtkEntry *entry, GPAWidget *gpw);
static void gpa_ts_rbdefault_toggled (GtkRadioButton *radiobutton, GPATransportSelector *ts);
static void gpa_ts_rbother_toggled (GtkRadioButton *radiobutton, GPATransportSelector *ts);
static void gpa_ts_menuitem_activate (GtkWidget *widget, gint index);

static void gpa_ts_select_transport (GPATransportSelector *ts, const gchar *id);

static GPAWidgetClass *parent_class;
static gchar *deflabel = N_("Default printer");

GtkType
gpa_transport_selector_get_type (void)
{
	static GtkType transport_selector_type = 0;
	if (!transport_selector_type) {
		static const GtkTypeInfo transport_selector_info = {
			"GPATransportSelector",
			sizeof (GPATransportSelector),
			sizeof (GPATransportSelectorClass),
			(GtkClassInitFunc) gpa_transport_selector_class_init,
			(GtkObjectInitFunc) gpa_transport_selector_init,
			NULL, NULL, NULL
		};
		transport_selector_type = gtk_type_unique (GPA_TYPE_WIDGET, &transport_selector_info);
	}
	return transport_selector_type;
}

static void
gpa_transport_selector_class_init (GPATransportSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GtkObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	object_class->destroy = gpa_transport_selector_destroy;

	gpa_class->construct = gpa_transport_selector_construct;
}

static void
gpa_ts_fentry_changed (GtkEntry *entry, GPAWidget *gpw)
{
	const guchar *text;

	text = gtk_entry_get_text (entry);

	gnome_print_config_set (gpw->config, "Settings.Transport.Backend.FileName", text);
}

static void
gpa_ts_rbentry_changed (GtkEntry *entry, GPAWidget *gpw)
{
	const guchar *text;

	text = gtk_entry_get_text (entry);

	gnome_print_config_set (gpw->config, "Settings.Transport.Backend.Printer", text);
}

static void
gpa_ts_centry_changed (GtkEntry *entry, GPAWidget *gpw)
{
	const guchar *text;

	text = gtk_entry_get_text (entry);

	gnome_print_config_set (gpw->config, "Settings.Transport.Backend.Command", text);
}

static void
gpa_ts_rbdefault_toggled (GtkRadioButton *radiobutton, GPATransportSelector *ts)
{

	gtk_widget_set_sensitive (ts->rbentry, FALSE);

}

static void
gpa_ts_rbother_toggled (GtkRadioButton *radiobutton, GPATransportSelector *ts)
{
	gtk_widget_set_sensitive (ts->rbentry, TRUE);
}

static void
gpa_transport_selector_init (GPATransportSelector *ts)
{
	ts->hbox = gtk_hbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (ts), ts->hbox);
	gtk_widget_show (ts->hbox);

	/* Create transports option menu */
	ts->menu = gtk_option_menu_new ();
	gtk_box_pack_start (GTK_BOX (ts->hbox), ts->menu, FALSE, FALSE, 0);

	/* Create filename entry */
	ts->fentry = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (ts->fentry), "changed", GTK_SIGNAL_FUNC (gpa_ts_fentry_changed), ts);
	gtk_box_pack_start (GTK_BOX (ts->hbox), ts->fentry, TRUE, TRUE, 0);

	/* Create printname selector */
	ts->rbhbox = gtk_hbox_new (FALSE, 4);
	ts->rbdefault = gtk_radio_button_new_with_label (NULL, deflabel);
	gtk_signal_connect (GTK_OBJECT (ts->rbdefault), "toggled", GTK_SIGNAL_FUNC (gpa_ts_rbdefault_toggled), ts);
	gtk_box_pack_start (GTK_BOX (ts->rbhbox), ts->rbdefault, TRUE, TRUE, 0);
	gtk_widget_show (ts->rbdefault);
	ts->rbother = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON (ts->rbdefault));
	gtk_signal_connect (GTK_OBJECT (ts->rbother), "toggled", GTK_SIGNAL_FUNC (gpa_ts_rbother_toggled), ts);
	gtk_box_pack_start (GTK_BOX (ts->rbhbox), ts->rbother, TRUE, TRUE, 0);
	gtk_widget_show (ts->rbother);
	
	ts->rbentry = gtk_entry_new ();
	gtk_widget_set_sensitive (ts->rbentry, FALSE);
	gtk_signal_connect (GTK_OBJECT (ts->rbentry), "changed", GTK_SIGNAL_FUNC (gpa_ts_rbentry_changed), ts);
	gtk_box_pack_end (GTK_BOX (ts->rbhbox), ts->rbentry, TRUE, TRUE, 0);
	gtk_widget_show (ts->rbentry);

	gtk_box_pack_start (GTK_BOX (ts->hbox), ts->rbhbox, TRUE, TRUE, 0);

	/* Create custom command entry */
	ts->centry = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (ts->centry), "changed", GTK_SIGNAL_FUNC (gpa_ts_centry_changed), ts);
	gtk_box_pack_end (GTK_BOX (ts->hbox), ts->centry, TRUE, TRUE, 0);
	

	ts->transportlist = NULL;
}

/* fixme: */
static void
gpa_transport_selector_destroy (GtkObject *object)
{
	GPATransportSelector *ts;

	ts = (GPATransportSelector *) object;

	if (ts->printer) {
		gpa_node_unref (ts->printer);
		ts->printer = NULL;
	}

	while (ts->transportlist) {
		gpa_node_unref (GPA_NODE (ts->transportlist->data));
		ts->transportlist = g_slist_remove (ts->transportlist, ts->transportlist->data);
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static gint
gpa_transport_selector_construct (GPAWidget *gpaw)
{
	GPATransportSelector *ts;

	ts = GPA_TRANSPORT_SELECTOR (gpaw);

	ts->printer = gpa_node_get_path_node (GNOME_PRINT_CONFIG_NODE (gpaw->config), "Printer");

	gpa_ts_rebuild_widget (ts);
}

static void
gpa_ts_rebuild_widget (GPATransportSelector *ts)
{
	GPANode *node;
	GPANode *option, *child;
	GtkWidget *menu, *item;
	gint idx, def;
	guchar *defid;
	GSList *l;

	node = GNOME_PRINT_CONFIG_NODE (GPA_WIDGET (ts)->config);

	/* Cleanup old state */
	while (ts->transportlist) {
		gpa_node_unref (GPA_NODE (ts->transportlist->data));
		ts->transportlist = g_slist_remove (ts->transportlist, ts->transportlist->data);
	}

	gtk_option_menu_remove_menu (GTK_OPTION_MENU (ts->menu));

	/* Construct new list */
	option = gpa_node_get_path_node (node, "Settings.Transport.Option.Backend");
	if (!option) {
		/* No transport node */
		gtk_widget_hide (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_activate (ts->rbdefault);
		gtk_widget_hide (ts->centry);
		gtk_widget_show (ts->rbhbox);
		return;
	}
	for (child = gpa_node_get_child (option, NULL); child != NULL; child = gpa_node_get_child (option, child)) {
		ts->transportlist = g_slist_prepend (ts->transportlist, child);
	}
	if (!ts->transportlist) {
		/* Empty transport list */
		gtk_widget_hide (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_activate (ts->rbdefault);
		gtk_widget_hide (ts->centry);
		gtk_widget_show (ts->rbhbox);
		return;
	}

	menu = gtk_menu_new ();
	ts->transportlist = g_slist_reverse (ts->transportlist);

	idx = 0;
	def = 0;
	defid = gpa_node_get_path_value (node, "Settings.Transport.Backend");
	/* Construct transport list */
	for (l = ts->transportlist; l != NULL; l = l->next) {
		gchar *val;
		val = gpa_node_get_path_value (GPA_NODE (l->data), "Name");
		if (!val) {
			g_warning ("Transport does not have 'Name' attribute");
		} else {
			item = gtk_menu_item_new_with_label (val);
			gtk_object_set_data (GTK_OBJECT (item), "GPAWidget", ts);
			gtk_signal_connect (GTK_OBJECT (item), "activate",
					    GTK_SIGNAL_FUNC (gpa_ts_menuitem_activate), GINT_TO_POINTER (idx));
			gtk_widget_show (item);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			g_free (val);
			if (defid) {
				guchar *id;
				id = gpa_node_get_value (GPA_NODE (l->data));
				if (id && !strcmp (id, defid))
					def = idx;
				g_free (id);
			}
			idx += 1;
		}
	}
	if (idx == 0) {
		/* No valid transports */
		gtk_widget_destroy (menu);
		gtk_widget_hide (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_activate (ts->rbdefault);
		gtk_widget_hide (ts->centry);
		gtk_widget_show (ts->rbhbox);
		return;
	}

	gtk_widget_show (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ts->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (ts->menu), def);

	if (defid) {
		gpa_ts_select_transport (ts, defid);
		g_free (defid);
	}

	gtk_widget_set_sensitive (ts->menu, TRUE);
}

static void
gpa_ts_menuitem_activate (GtkWidget *widget, gint index)
{
	GPAWidget *gpaw;
	GPANode *transport;
	gchar *value;

	gpaw = gtk_object_get_data (GTK_OBJECT (widget), "GPAWidget");
	g_return_if_fail (gpaw != NULL);
	g_return_if_fail (GPA_IS_WIDGET (gpaw));

	transport = g_slist_nth_data (GPA_TRANSPORT_SELECTOR (gpaw)->transportlist, index);
	g_return_if_fail (transport != NULL);
	g_return_if_fail (GPA_IS_NODE (transport));

	value = gpa_node_get_value (transport);
	g_return_if_fail (value != NULL);

	gpa_ts_select_transport (GPA_TRANSPORT_SELECTOR (gpaw), value);

	g_free (value);
}

static void
gpa_ts_select_transport (GPATransportSelector *ts, const gchar *id)
{
	GPAWidget *gpaw;

	gpaw = GPA_WIDGET (ts);

	gnome_print_config_set (gpaw->config, "Settings.Transport.Backend", id);

	if (!strcmp (id, "file")) {
		guchar *filename;
		/* Current transport is file */
		filename = gnome_print_config_get (gpaw->config, "Settings.Transport.Backend.FileName");
		if (filename) {
			gtk_entry_set_text (GTK_ENTRY (ts->fentry), filename);
			g_free (filename);
		} else {
			gtk_entry_set_text (GTK_ENTRY (ts->fentry), "gnome-print.out");
		}
		gtk_widget_show (ts->menu);
		gtk_widget_show (ts->fentry);
		gtk_widget_hide (ts->rbhbox);
		gtk_widget_hide (ts->centry);
	} else if (!strcmp (id, "lpr")) {
		guchar *lp;
		/* Current transport is lpr */
		lp = gnome_print_config_get (gpaw->config, "Settings.Transport.Backend.Printer");
		if (lp && *lp) {
			gtk_widget_activate (ts->rbother);
			gtk_entry_set_text (GTK_ENTRY (ts->rbentry), lp);
		} else {
			gtk_widget_activate (ts->rbdefault);
		}
		if (lp) g_free (lp);
		gtk_widget_show (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_show (ts->rbhbox);
		gtk_widget_hide (ts->centry);
	} else if (!strcmp (id, "custom")) {
		guchar *command;
		/* Current transport is a custom command */
		command = gnome_print_config_get (gpaw->config, "Settings.Transport.Backend.Command");
		if (command) {
			gtk_entry_set_text (GTK_ENTRY (ts->centry), command);
			g_free (command);
		} else {
			gtk_entry_set_text (GTK_ENTRY (ts->centry), "lpr");
		}
		gtk_widget_show (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_hide (ts->rbhbox);
		gtk_widget_show (ts->centry);
	} else {
		gtk_widget_show (ts->menu);
		gtk_widget_hide (ts->fentry);
		gtk_widget_hide (ts->rbhbox);
		gtk_widget_hide (ts->centry);
	}
}
