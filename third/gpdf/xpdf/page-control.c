/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Page number entry
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <aconf.h>
#include <stdlib.h>
#include <libgnome/gnome-macros.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gpdf-util.h"
#include "gpdf-marshal.h"
#include "page-control.h"

#define PARENT_TYPE GTK_TYPE_HBOX
GPDF_CLASS_BOILERPLATE (GPdfPageControl, gpdf_page_control,
			GtkHBox, PARENT_TYPE)

struct _GPdfPageControlPrivate
{
        GtkWidget *entry;
	GtkWidget *label;
	GtkTooltips *tooltips;
};

enum {
        SET_PAGE_SIGNAL,
        LAST_SIGNAL
};

static guint gpdf_page_control_signals [LAST_SIGNAL];

static gboolean
gpdf_page_control_return_pressed (GPdfPageControl *control)
{
	const char *text;
	char *endptr;
	long page_number;
	
	text = gtk_entry_get_text (GTK_ENTRY (control->priv->entry));
	page_number = strtol (text, &endptr, 10);
	page_number = MAX (page_number, 1);

	g_signal_emit (G_OBJECT (control),
		       gpdf_page_control_signals [SET_PAGE_SIGNAL],
		       0, page_number);

	return TRUE;
}

static gboolean
gpdf_page_control_key_press_event_cb (GtkWidget *entry, GdkEventKey *event,
				      GPdfPageControl *control)
{
        switch (event->keyval) {
        case GDK_Return:
        case GDK_KP_Enter:
		return gpdf_page_control_return_pressed (control);
        default:
		return FALSE;
        }
}

void
gpdf_page_control_set_page (GPdfPageControl *control, int page)
{
        char *page_string;

        page_string = g_strdup_printf ("%d", page);
        gtk_entry_set_text (GTK_ENTRY (control->priv->entry), page_string);
        g_free (page_string);
}

void
gpdf_page_control_set_total_pages (GPdfPageControl *control, int n_pages)
{
	char *of_pages;

	if (n_pages < 0)
		n_pages = 0;

	of_pages = g_strdup_printf (_(" of %d"), n_pages);
	gtk_label_set_text (GTK_LABEL (control->priv->label), of_pages);
	g_free (of_pages);
}

static gint
gpdf_page_control_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GPdfPageControlPrivate *priv;
	
	if (GTK_WIDGET_DRAWABLE (widget))
		gtk_paint_box (widget->style,
			       widget->window,
			       GTK_WIDGET_STATE (widget),
			       GTK_SHADOW_NONE,
			       &event->area, widget, "toolbar",
			       widget->allocation.x - 1,
			       widget->allocation.y - 1,
			       widget->allocation.width + 2,
			       widget->allocation.height + 2);

	priv = GPDF_PAGE_CONTROL (widget)->priv;

	gtk_container_propagate_expose (GTK_CONTAINER (widget),
					priv->entry,
					event);

	gtk_container_propagate_expose (GTK_CONTAINER (widget),
					priv->label,
					event);

	return FALSE;
}


static void
gpdf_page_control_destroy (GtkObject *object)
{
	GPdfPageControlPrivate *priv = GPDF_PAGE_CONTROL (object)->priv;

	if (priv->tooltips) {
		g_object_unref (priv->tooltips);
		priv->tooltips = NULL;
	}

	GNOME_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
gpdf_page_control_finalize (GObject *object)
{
        GPdfPageControl *control = GPDF_PAGE_CONTROL (object);

        if (control->priv) {
                g_free (control->priv);
                control->priv = NULL;
        }

        GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

void
gpdf_page_control_class_init (GPdfPageControlClass *klass)
{
        GObjectClass *object_class;
	GtkObjectClass *gtk_object_class;
	GtkWidgetClass *gtk_widget_class;

        object_class = G_OBJECT_CLASS (klass);
	gtk_object_class = GTK_OBJECT_CLASS (klass);
	gtk_widget_class = GTK_WIDGET_CLASS (klass);

        object_class->finalize = gpdf_page_control_finalize;
	gtk_object_class->destroy = gpdf_page_control_destroy;
	gtk_widget_class->expose_event = gpdf_page_control_expose;

        gpdf_page_control_signals [SET_PAGE_SIGNAL] = g_signal_new (
                "set_page",
                G_TYPE_FROM_CLASS (object_class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GPdfPageControlClass, set_page),
                NULL, NULL,
                gpdf_marshal_VOID__INT,
                G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
gpdf_page_control_setup_widgets (GPdfPageControl *control)
{
	GPdfPageControlPrivate *priv;

	priv = control->priv;

	priv->entry = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (priv->entry), 5);
	gtk_widget_show (priv->entry);
	gtk_box_pack_start (GTK_BOX (control), priv->entry, TRUE, TRUE, 0);

	priv->label = gtk_label_new ("");
	gpdf_page_control_set_total_pages (control, -1);
	gtk_widget_show (priv->label);
	gtk_box_pack_start (GTK_BOX (control), priv->label, FALSE, TRUE, 0);

	g_signal_connect (G_OBJECT (priv->entry), "key-press-event",
			  G_CALLBACK (gpdf_page_control_key_press_event_cb),
			  control);
}

static void
gpdf_page_control_setup_tooltips (GPdfPageControl *control)
{
	GPdfPageControlPrivate *priv;

	priv = control->priv;
	priv->tooltips = gtk_tooltips_new ();
	g_object_ref (G_OBJECT (priv->tooltips));
	gtk_object_sink (GTK_OBJECT (priv->tooltips));
	gtk_tooltips_set_tip (GTK_TOOLTIPS (priv->tooltips), priv->entry,
			      _("Current page"),
			      _("This shows the current page number. "
				"To jump to another page, enter its number."));
}

static void
gpdf_page_control_setup_at (GPdfPageControl *control)
{
	AtkObject *accessible;

	accessible = gtk_widget_get_accessible (GTK_WIDGET (control));
	atk_object_set_name (accessible, _("Page"));

	accessible = gtk_widget_get_accessible (control->priv->entry);
	atk_object_set_name (accessible, _("Current page"));
}

void
gpdf_page_control_instance_init (GPdfPageControl *control)
{
        GPdfPageControlPrivate *priv = g_new0 (GPdfPageControlPrivate, 1);
        control->priv = priv;

	gpdf_page_control_setup_widgets (control);
	gpdf_page_control_setup_tooltips (control);
	gpdf_page_control_setup_at (control);
}
