/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-print-to-file.c: A print to file selector
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
 *  Copyright (C) 2003 Ximian, Inc. 
 */

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-print-to-file.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>

static void gpa_p2f_class_init (GPAPrintToFileClass *klass);
static void gpa_p2f_init (GPAPrintToFile *selector);
static void gpa_p2f_finalize (GObject *object);
static gint gpa_p2f_construct (GPAWidget *widget);

static void gpa_p2f_state_modified_cb (GPANode *node, guint flags, GPAPrintToFile *p2f);

static GPAWidgetClass *parent_class;

GType
gpa_p2f_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAPrintToFileClass),
			NULL, NULL,
			(GClassInitFunc) gpa_p2f_class_init,
			NULL, NULL,
			sizeof (GPAPrintToFile),
			0,
			(GInstanceInitFunc) gpa_p2f_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPAPrintToFile", &info, 0);
	}
	return type;
}

static void
gpa_p2f_class_init (GPAPrintToFileClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_p2f_construct;
	object_class->finalize = gpa_p2f_finalize;
}

static void
gpa_p2f_init (GPAPrintToFile *c)
{
	/* Empty */
}

static void
gpa_p2f_disconnect (GPAPrintToFile *p2f)
{
	if (p2f->handler) {
		g_signal_handler_disconnect (p2f->node, p2f->handler);
		p2f->handler = 0;
	}
	
	if (p2f->handler_output) {
		g_signal_handler_disconnect (p2f->node_output, p2f->handler_output);
		p2f->handler_output = 0;
	}
	
	if (p2f->node) {
		gpa_node_unref (p2f->node);
		p2f->node = NULL;
	}
	if (p2f->node_output) {
		gpa_node_unref (p2f->node_output);
		p2f->node_output = NULL;
	}
}

static void
gpa_p2f_finalize (GObject *object)
{
	GPAPrintToFile *p2f;

	p2f = (GPAPrintToFile *) object;

	gpa_p2f_disconnect (p2f);

	if (p2f->handler_config)
		g_signal_handler_disconnect (p2f->config, p2f->handler_config);
	p2f->handler_config = 0;
	p2f->config = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_p2f_checkbox_update (GPAPrintToFile *p2f)
{
	guchar *v;
	gboolean state = FALSE;
	
	v = gpa_node_get_value (p2f->node);

	/* FIXME use gpa_node_get_boolean (Chema) */
	if (v &&
	    (!g_ascii_strcasecmp (v, "true") ||
	     !g_ascii_strcasecmp (v, "yes") ||
	     !g_ascii_strcasecmp (v, "y") ||
	     !g_ascii_strcasecmp (v, "yes") ||
	     (atoi (v) != 0))) {
		state = TRUE;
	}
	g_free (v);
	/* FIXME: End */

	p2f->updating = TRUE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p2f->checkbox),
				      state);
	gtk_widget_set_sensitive (p2f->entry, state);
	p2f->updating = FALSE;
}

static void
gpa_p2f_entry_update (GPAPrintToFile *w)
{
	guchar *v;

	v = gpa_node_get_value (w->node_output);

	w->updating = TRUE;
	gtk_entry_set_text (GTK_ENTRY (w->entry), v);
	w->updating = FALSE;

	g_free (v);
}

static void
gpa_p2f_file_modified_cb (GPANode *node, guint flags, GPAPrintToFile *p2f)
{
	if (p2f->updating)
		return;

	gpa_p2f_entry_update (p2f);
}

static void
gpa_p2f_state_modified_cb (GPANode *node, guint flags, GPAPrintToFile *p2f)
{
	if (p2f->updating)
		return;
	
	gpa_p2f_checkbox_update (p2f);
}

static void
gpa_p2f_connect (GPAPrintToFile *c)
{
	c->node    = gpa_node_lookup (c->config, "Settings.Output.Job.PrintToFile");
	c->handler = g_signal_connect (G_OBJECT (c->node), "modified", (GCallback)
				       gpa_p2f_state_modified_cb, c);
	c->node_output    = gpa_node_lookup (c->config, "Settings.Output.Job.Filename");
	c->handler_output = g_signal_connect (G_OBJECT (c->node_output), "modified", (GCallback)
					      gpa_p2f_file_modified_cb, c);
}

static void
gpa_p2f_checkbox_toggled_cb (GtkToggleButton *button, GPAPrintToFile *c)
{
	gboolean state;

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (c->checkbox));
	gtk_widget_set_sensitive (c->entry, state);
	
	if (c->updating)
		return;

	c->updating = TRUE;
	gpa_node_set_value (c->node, state ? "True" : "False");
	c->updating = FALSE;
}

static void
gpa_p2f_entry_changed_cb (GtkEntry *entry, GPAPrintToFile *c)
{
	const gchar *entry_txt;
	
	if (c->updating)
		return;

	entry_txt = gtk_entry_get_text (GTK_ENTRY (c->entry));

	c->updating = TRUE;
	gpa_node_set_value (c->node_output, entry_txt);
	c->updating = FALSE;
}

static void
gpa_p2f_update (GPAPrintToFile *c)
{
	gpa_p2f_checkbox_update (c);
	gpa_p2f_entry_update (c);

	if ((GPA_NODE_FLAGS (c->node) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED) {
		gtk_widget_set_sensitive (c->checkbox, FALSE);
	} else {
		gtk_widget_set_sensitive (c->checkbox, TRUE);
	}
}

static void
gpa_p2f_config_modified_cb (GPANode *node, guint flags, GPAPrintToFile *p2f)
{
	gpa_p2f_disconnect (p2f);
	gpa_p2f_connect (p2f);
	gpa_p2f_update (p2f);
}

static gint
gpa_p2f_construct (GPAWidget *gpaw)
{
	GtkWidget *hbox;
	GPAPrintToFile *c;
	
	c = GPA_P2F (gpaw);
	c->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	c->handler_config = g_signal_connect (G_OBJECT (c->config), "modified", (GCallback)
					      gpa_p2f_config_modified_cb, c);
	c->entry    = gtk_entry_new ();
	c->checkbox = gtk_check_button_new_with_mnemonic (_("Print to _file"));

	g_signal_connect (G_OBJECT (c->checkbox), "toggled", (GCallback)
			  gpa_p2f_checkbox_toggled_cb, c);
	g_signal_connect (G_OBJECT (c->entry), "changed", (GCallback)
			  gpa_p2f_entry_changed_cb, c);

	gpa_p2f_connect (c);
	gpa_p2f_update (c);

	hbox = gtk_vbox_new (FALSE, 4);

	gtk_box_pack_end (GTK_BOX (hbox), GTK_WIDGET (c->entry),
			  FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (hbox), GTK_WIDGET (c->checkbox),
			  FALSE, FALSE, 0);

	gtk_widget_show_all (hbox);
	gtk_container_add (GTK_CONTAINER (c), hbox);
	
	return TRUE;
}

void
gpa_p2f_enable_filename_entry (GPAPrintToFile *c, gboolean enable)
{
	g_return_if_fail (GPA_IS_P2F (c));

	if (enable)
		gtk_widget_show (c->entry);
	else
		gtk_widget_hide (c->entry);
}
