/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-radiobutton.c: A radiobutton selector for boolean GPANodes
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
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc. 
 *
 */

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-radiobutton.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>

static void gpa_radiobutton_class_init (GPARadiobuttonClass *klass);
static void gpa_radiobutton_init (GPARadiobutton *selector);
static void gpa_radiobutton_finalize (GObject *object);
static gint gpa_radiobutton_construct (GPAWidget *widget);

static void gpa_radiobutton_state_modified_cb (GPANode *node, guint flags, GPARadiobutton *radiobutton);

static GPAWidgetClass *parent_class;

GType
gpa_radiobutton_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPARadiobuttonClass),
			NULL, NULL,
			(GClassInitFunc) gpa_radiobutton_class_init,
			NULL, NULL,
			sizeof (GPARadiobutton),
			0,
			(GInstanceInitFunc) gpa_radiobutton_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPARadiobutton", &info, 0);
	}
	return type;
}

static void
gpa_radiobutton_class_init (GPARadiobuttonClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_radiobutton_construct;
	object_class->finalize = gpa_radiobutton_finalize;
}

static void
gpa_radiobutton_init (GPARadiobutton *c)
{
	/* Empty */
}

static void
gpa_radiobutton_disconnect (GPARadiobutton *radiobutton)
{
	if (radiobutton->handler) {
		g_signal_handler_disconnect (radiobutton->node, radiobutton->handler);
		radiobutton->handler = 0;
	}
	
	if (radiobutton->node) {
		gpa_node_unref (radiobutton->node);
		radiobutton->node = NULL;
	}
}

static void
gpa_radiobutton_finalize (GObject *object)
{
	GPARadiobutton *radiobutton;

	radiobutton = (GPARadiobutton *) object;

	gpa_radiobutton_disconnect (radiobutton);

	if (radiobutton->handler_config)
		g_signal_handler_disconnect (radiobutton->config, radiobutton->handler_config);
	radiobutton->handler_config = 0;
	radiobutton->config = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_radiobutton_update (GPARadiobutton *c)
{
	GSList *list;
	guchar *value;

	value = gpa_node_get_value (c->node);

	list = c->group;
	while (list) {
		gchar *id;
		id = g_object_get_data (G_OBJECT (list->data), "id");
		g_assert (id);
		if (strcmp (id, value) == 0) {
			c->updating = TRUE;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (list->data), TRUE);
			c->updating = FALSE;
			break;
		}
			
		list = list->next;
	}
	g_free (value);
}

static void
gpa_radiobutton_state_modified_cb (GPANode *node, guint flags, GPARadiobutton *radiobutton)
{
	if (radiobutton->updating)
		return;
	
	gpa_radiobutton_update (radiobutton);
}

static void
gpa_radiobutton_connect (GPARadiobutton *c)
{
	c->node    = gpa_node_lookup (c->config, c->path);
	c->handler = g_signal_connect (G_OBJECT (c->node), "modified", (GCallback)
				       gpa_radiobutton_state_modified_cb, c);
}

static void
gpa_radiobutton_toggled_cb (GtkToggleButton *button, GPARadiobutton *c)
{
	gboolean state;
	const gchar *id;

	if (c->updating)
		return;

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	/* We don't care when buttons are toggled off, just on */
	if (!state)
		return;
	
	c->updating = TRUE;
	id = g_object_get_data (G_OBJECT (button), "id");
	g_assert (id);
	gpa_node_set_value (c->node, id);
	c->updating = FALSE;
}

static void
gpa_radiobutton_config_modified_cb (GPANode *node, guint flags, GPARadiobutton *radiobutton)
{
	gpa_radiobutton_disconnect (radiobutton);
	gpa_radiobutton_connect (radiobutton);
	gpa_radiobutton_update (radiobutton);
}

static gint
gpa_radiobutton_construct (GPAWidget *gpaw)
{
	GPAApplicationOption option;
	GPARadiobutton *c;
	GSList *group = NULL;
	GtkBox *box;
	gint i = 0;

	c = GPA_RADIOBUTTON (gpaw);
	c->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	c->handler_config = g_signal_connect (G_OBJECT (c->config), "modified", (GCallback)
					      gpa_radiobutton_config_modified_cb, c);

	c->box = gtk_vbox_new (FALSE, 0);
	box = GTK_BOX (c->box);

	option = c->options[i++];
	while (option.description) {
		GtkWidget *r;
		r = gtk_radio_button_new_with_mnemonic (group, _(option.description));
		g_object_set_data (G_OBJECT (r), "id", (gchar *) option.id);
		group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (r));
		gtk_box_pack_start_defaults (box, r);
		g_signal_connect (G_OBJECT (r), "toggled", (GCallback)
				  gpa_radiobutton_toggled_cb, c);
		option = c->options[i++];
	}
	c->group = group;

	gpa_radiobutton_connect (c);
	gpa_radiobutton_update (c);

	gtk_widget_show_all (c->box);
	gtk_container_add (GTK_CONTAINER (c), c->box);
	
	return TRUE;
}

GtkWidget *
gpa_radiobutton_new (GnomePrintConfig *config,
		     const guchar *path, GPAApplicationOption *options)
{
	GtkWidget *c;

	c = gpa_widget_new (GPA_TYPE_RADIOBUTTON, NULL);
	GPA_RADIOBUTTON (c)->path = g_strdup (path);
	GPA_RADIOBUTTON (c)->options = options;
	gpa_widget_construct (GPA_WIDGET (c), config);
			    
	return c;
}
