/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-checkbutton.c: A checkbutton selector for boolean GPANodes
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

#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-checkbutton.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>

static void gpa_checkbutton_class_init (GPACheckbuttonClass *klass);
static void gpa_checkbutton_init (GPACheckbutton *selector);
static void gpa_checkbutton_finalize (GObject *object);
static gint gpa_checkbutton_construct (GPAWidget *widget);

static void gpa_checkbutton_state_modified_cb (GPANode *node, guint flags, GPACheckbutton *checkbutton);

static GPAWidgetClass *parent_class;

GType
gpa_checkbutton_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPACheckbuttonClass),
			NULL, NULL,
			(GClassInitFunc) gpa_checkbutton_class_init,
			NULL, NULL,
			sizeof (GPACheckbutton),
			0,
			(GInstanceInitFunc) gpa_checkbutton_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPACheckbutton", &info, 0);
	}
	return type;
}

static void
gpa_checkbutton_class_init (GPACheckbuttonClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_checkbutton_construct;
	object_class->finalize = gpa_checkbutton_finalize;
}

static void
gpa_checkbutton_init (GPACheckbutton *c)
{
	/* Empty */
}

static void
gpa_checkbutton_disconnect (GPACheckbutton *checkbutton)
{
	if (checkbutton->handler) {
		g_signal_handler_disconnect (checkbutton->node, checkbutton->handler);
		checkbutton->handler = 0;
	}
	
	if (checkbutton->node) {
		gpa_node_unref (checkbutton->node);
		checkbutton->node = NULL;
	}
}

static void
gpa_checkbutton_finalize (GObject *object)
{
	GPACheckbutton *checkbutton;

	checkbutton = (GPACheckbutton *) object;

	gpa_checkbutton_disconnect (checkbutton);

	if (checkbutton->handler_config)
		g_signal_handler_disconnect (checkbutton->config, checkbutton->handler_config);
	checkbutton->handler_config = 0;
	checkbutton->config = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_checkbutton_update (GPACheckbutton *c)
{
	guchar *v;
	gboolean state = FALSE;
	
	v = gpa_node_get_value (c->node);

	/* FIXME use gpa_node_get_boolean */
	if (v &&
	    (!g_ascii_strcasecmp (v, "true") ||
	     !g_ascii_strcasecmp (v, "yes") ||
	     !g_ascii_strcasecmp (v, "y") ||
	     !g_ascii_strcasecmp (v, "yes") ||
	     (atoi (v) != 0))) {
		state = TRUE;
	}
	g_free (v);
	/* End */

	c->updating = TRUE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (c->checkbox),
				      state);
	c->updating = FALSE;

	if ((GPA_NODE_FLAGS (c->node) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED) {
		gtk_widget_set_sensitive (c->checkbox, FALSE);
	} else {
		gtk_widget_set_sensitive (c->checkbox, TRUE);
	}
}

static void
gpa_checkbutton_state_modified_cb (GPANode *node, guint flags, GPACheckbutton *checkbutton)
{
	if (checkbutton->updating)
		return;
	
	gpa_checkbutton_update (checkbutton);
}

static void
gpa_checkbutton_connect (GPACheckbutton *c)
{
	c->node    = gpa_node_lookup (c->config, c->path);
	c->handler = g_signal_connect (G_OBJECT (c->node), "modified", (GCallback)
				       gpa_checkbutton_state_modified_cb, c);
}

static void
gpa_checkbutton_toggled_cb (GtkToggleButton *button, GPACheckbutton *c)
{
	gboolean state;

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (c->checkbox));
	
	if (c->updating)
		return;

	c->updating = TRUE;
	gpa_node_set_value (c->node, state ? "true" : "false");
	c->updating = FALSE;
}

static void
gpa_checkbutton_config_modified_cb (GPANode *node, guint flags, GPACheckbutton *checkbutton)
{
	gpa_checkbutton_disconnect (checkbutton);
	gpa_checkbutton_connect (checkbutton);
	gpa_checkbutton_update (checkbutton);
}

static gint
gpa_checkbutton_construct (GPAWidget *gpaw)
{
	GPACheckbutton *c;

	c = GPA_CHECKBUTTON (gpaw);
	c->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	c->handler_config = g_signal_connect (G_OBJECT (c->config), "modified", (GCallback)
					      gpa_checkbutton_config_modified_cb, c);
	c->checkbox = gtk_check_button_new ();

	g_signal_connect (G_OBJECT (c->checkbox), "toggled", (GCallback)
			  gpa_checkbutton_toggled_cb, c);

	gpa_checkbutton_connect (c);
	gpa_checkbutton_update (c);

	gtk_widget_show (c->checkbox);
	gtk_container_add (GTK_CONTAINER (c), c->checkbox);
	
	return TRUE;
}

GtkWidget *
gpa_checkbutton_new (GnomePrintConfig *config,
		     const guchar *path, const guchar *label)
{
	GtkWidget *c;
	GtkButton *b;

	c = gpa_widget_new (GPA_TYPE_CHECKBUTTON, NULL);
	GPA_CHECKBUTTON (c)->path = g_strdup (path);
	gpa_widget_construct (GPA_WIDGET (c), config);
	b = GTK_BUTTON (GPA_CHECKBUTTON (c)->checkbox);
	gtk_button_set_use_underline (b, TRUE);
	gtk_button_set_label (b, label);
			    
	return c;
}
