/* -*- mode: c; style: linux -*- */

/* prefs-widget.c
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Written by Bradford Hovinen <hovinen@helixcode.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

#include "prefs-widget.h"

#define WID(str) (glade_xml_get_widget (prefs_widget->dialog_data, str))

enum {
	ARG_0,
	ARG_PREFERENCES
};

static CappletWidgetClass *parent_class;

static void prefs_widget_init             (PrefsWidget *prefs_widget);
static void prefs_widget_class_init       (PrefsWidgetClass *class);

static void prefs_widget_set_arg          (GtkObject *object, 
					   GtkArg *arg, 
					   guint arg_id);
static void prefs_widget_get_arg          (GtkObject *object, 
					   GtkArg *arg, 
					   guint arg_id);

static void read_preferences              (PrefsWidget *prefs_widget,
					   Preferences *prefs);

static void volume_changed_cb             (GtkAdjustment *adjustment,
					   PrefsWidget *prefs_widget);
static void pitch_changed_cb              (GtkAdjustment *adjustment,
					   PrefsWidget *prefs_widget);
static void duration_changed_cb           (GtkAdjustment *adjustment,
					   PrefsWidget *prefs_widget);

static void test_button_clicked_cb        (GtkButton *button,
					   PrefsWidget *prefs_widget);

guint
prefs_widget_get_type (void)
{
	static guint prefs_widget_type = 0;

	if (!prefs_widget_type) {
		GtkTypeInfo prefs_widget_info = {
			"PrefsWidget",
			sizeof (PrefsWidget),
			sizeof (PrefsWidgetClass),
			(GtkClassInitFunc) prefs_widget_class_init,
			(GtkObjectInitFunc) prefs_widget_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		prefs_widget_type = 
			gtk_type_unique (capplet_widget_get_type (), 
					 &prefs_widget_info);
	}

	return prefs_widget_type;
}

static void
prefs_widget_init (PrefsWidget *prefs_widget)
{
	GtkWidget *widget;
	GtkAdjustment *adjustment;

	prefs_widget->dialog_data = 
		glade_xml_new (GLADE_DATADIR "/bell-properties.glade",
			       "prefs_widget");

	widget = glade_xml_get_widget (prefs_widget->dialog_data, 
				       "prefs_widget");
	gtk_container_add (GTK_CONTAINER (prefs_widget), widget);

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("volume_entry")));
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
			    volume_changed_cb, prefs_widget);

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("pitch_entry")));
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
			    pitch_changed_cb, prefs_widget);

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("duration_entry")));
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
			    duration_changed_cb, prefs_widget);

	glade_xml_signal_connect_data
		(prefs_widget->dialog_data, "test_button_clicked_cb",
		 test_button_clicked_cb, prefs_widget);
}

static void
prefs_widget_class_init (PrefsWidgetClass *class) 
{
	GtkObjectClass *object_class;

	gtk_object_add_arg_type ("PrefsWidget::preferences",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_PREFERENCES);

	object_class = GTK_OBJECT_CLASS (class);
	object_class->set_arg = prefs_widget_set_arg;
	object_class->get_arg = prefs_widget_get_arg;

	parent_class = CAPPLET_WIDGET_CLASS
		(gtk_type_class (capplet_widget_get_type ()));
}

static void
prefs_widget_set_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	PrefsWidget *prefs_widget;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (object));

	prefs_widget = PREFS_WIDGET (object);

	switch (arg_id) {
	case ARG_PREFERENCES:
		if (prefs_widget->prefs)
			gtk_object_unref (GTK_OBJECT (prefs_widget->prefs));

		prefs_widget->prefs = GTK_VALUE_POINTER (*arg);

		if (prefs_widget->prefs) {
			gtk_object_ref (GTK_OBJECT (prefs_widget->prefs));
			read_preferences (prefs_widget, prefs_widget->prefs);
		}

		break;

	default:
		g_warning ("Bad argument set");
		break;
	}
}

static void
prefs_widget_get_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	PrefsWidget *prefs_widget;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (object));

	prefs_widget = PREFS_WIDGET (object);

	switch (arg_id) {
	case ARG_PREFERENCES:
		GTK_VALUE_POINTER (*arg) = prefs_widget->prefs;
		break;

	default:
		g_warning ("Bad argument get");
		break;
	}
}

GtkWidget *
prefs_widget_new (Preferences *prefs) 
{
	g_return_val_if_fail (prefs == NULL || IS_PREFERENCES (prefs), NULL);

	return gtk_widget_new (prefs_widget_get_type (),
			       "preferences", prefs,
			       NULL);
}

void
prefs_widget_set_preferences (PrefsWidget *prefs_widget, Preferences *prefs)
{
	g_return_if_fail (prefs_widget != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (prefs_widget));
	g_return_if_fail (prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs));

	gtk_object_set (GTK_OBJECT (prefs_widget), "preferences", prefs, NULL);
}

static void
read_preferences (PrefsWidget *prefs_widget, Preferences *prefs)
{
	GtkAdjustment *adjustment;

	g_return_if_fail (prefs_widget != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (prefs_widget));
	g_return_if_fail (prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs));

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("volume_entry")));
	gtk_adjustment_set_value (adjustment, prefs->percent);

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("pitch_entry")));
	gtk_adjustment_set_value (adjustment, prefs->pitch);

	adjustment = gtk_range_get_adjustment 
		(GTK_RANGE (WID ("duration_entry")));
	gtk_adjustment_set_value (adjustment, prefs->duration);
}

static void
volume_changed_cb (GtkAdjustment *adjustment, PrefsWidget *prefs_widget) 
{
	g_return_if_fail (prefs_widget != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (prefs_widget));
	g_return_if_fail (prefs_widget->prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs_widget->prefs));
	g_return_if_fail (adjustment != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

	prefs_widget->prefs->percent = adjustment->value;

	preferences_changed (prefs_widget->prefs);
	capplet_widget_state_changed (CAPPLET_WIDGET (prefs_widget), TRUE);
}

static void
pitch_changed_cb (GtkAdjustment *adjustment, PrefsWidget *prefs_widget) 
{
	g_return_if_fail (prefs_widget != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (prefs_widget));
	g_return_if_fail (prefs_widget->prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs_widget->prefs));
	g_return_if_fail (adjustment != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

	prefs_widget->prefs->pitch = adjustment->value;

	preferences_changed (prefs_widget->prefs);
	capplet_widget_state_changed (CAPPLET_WIDGET (prefs_widget), TRUE);
}

static void
duration_changed_cb (GtkAdjustment *adjustment, PrefsWidget *prefs_widget) 
{
	g_return_if_fail (prefs_widget != NULL);
	g_return_if_fail (IS_PREFS_WIDGET (prefs_widget));
	g_return_if_fail (prefs_widget->prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs_widget->prefs));
	g_return_if_fail (adjustment != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

	prefs_widget->prefs->duration = adjustment->value;

	preferences_changed (prefs_widget->prefs);
	capplet_widget_state_changed (CAPPLET_WIDGET (prefs_widget), TRUE);
}

static void
test_button_clicked_cb (GtkButton *button, PrefsWidget *prefs_widget) 
{
        XBell (GDK_DISPLAY (), 0);
}
