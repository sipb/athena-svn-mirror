/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-spinbutton.c:
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
 *  Authors:
 *    Lutz Müller <lutz@users.sourceforge.net>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2004 Lutz Müller
 *  Copyright (C) 2003 Ximian, Inc. 
 *
 */

#include "config.h"

#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-spinbutton.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>
#include <libgnomeprint/gnome-print-unit.h>

#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static void gpa_spinbutton_class_init (GPASpinbuttonClass *klass);
static void gpa_spinbutton_init (GPASpinbutton *selector);
static void gpa_spinbutton_finalize (GObject *object);
static gint gpa_spinbutton_construct (GPAWidget *widget);

static void gpa_spinbutton_state_modified_cb (GPANode *node, guint flags, GPASpinbutton *spinbutton);

static GPAWidgetClass *parent_class;

GType
gpa_spinbutton_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPASpinbuttonClass),
			NULL, NULL,
			(GClassInitFunc) gpa_spinbutton_class_init,
			NULL, NULL,
			sizeof (GPASpinbutton),
			0,
			(GInstanceInitFunc) gpa_spinbutton_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPASpinbutton", &info, 0);
	}
	return type;
}

static void
gpa_spinbutton_class_init (GPASpinbuttonClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_spinbutton_construct;
	object_class->finalize = gpa_spinbutton_finalize;
}

static void
gpa_spinbutton_init (GPASpinbutton *c)
{
	c->factor = 1.;
}

static void
gpa_spinbutton_connect (GPASpinbutton *s)
{
	s->node    = gpa_node_lookup (s->config, s->path);
	s->handler = g_signal_connect (G_OBJECT (s->node), "modified",
			G_CALLBACK (gpa_spinbutton_state_modified_cb), s);
}

static gboolean
gpa_spinbutton_is_connected (GPASpinbutton *s)
{
	g_return_val_if_fail (GPA_IS_SPINBUTTON (s), FALSE);

	return (s->node != NULL);
}

static void
gpa_spinbutton_disconnect (GPASpinbutton *spinbutton)
{
	if (spinbutton->handler) {
		g_signal_handler_disconnect (spinbutton->node,
					     spinbutton->handler);
		spinbutton->handler = 0;
	}
	
	if (spinbutton->node) {
		gpa_node_unref (spinbutton->node);
		spinbutton->node = NULL;
	}
}

static void
gpa_spinbutton_finalize (GObject *object)
{
	GPASpinbutton *spinbutton;

	spinbutton = (GPASpinbutton *) object;

	gpa_spinbutton_disconnect (spinbutton);

	if (spinbutton->handler_config) {
		g_signal_handler_disconnect (spinbutton->config,
					     spinbutton->handler_config);
		spinbutton->handler_config = 0;
	}

	spinbutton->config = NULL;
	if (spinbutton->unit) {
		g_free (spinbutton->unit);
		spinbutton->unit = NULL;
	}

	if (spinbutton->path) {
		g_free (spinbutton->path);
		spinbutton = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_spinbutton_load (GPASpinbutton *s)
{
	guchar *v, *e = NULL;
	const GnomePrintUnit *u = NULL;

	g_return_if_fail (GPA_IS_SPINBUTTON (s));
	g_return_if_fail (gpa_spinbutton_is_connected (s));

	if (s->loading || s->saving || s->updating) return;

	s->loading = TRUE;

	/* The unit stuff is borrowed from gnome-print-config. */
	v = gpa_node_get_value (s->node);
	if (v) {
		gchar *stupid_gcc = NULL; /* kludge around silly strict-aliasing warning */
		s->value = g_ascii_strtod (v, &stupid_gcc);
		e = (guchar *)stupid_gcc;

		if ((errno == 0) && (e != v)) {
			while (*e && ! g_ascii_isalnum (*e)) e++;
			if (*e != '\0') {
				u = gnome_print_unit_get_by_abbreviation (e);
				if (!u) u = gnome_print_unit_get_by_name (e);
				if (u) gnome_print_convert_distance (
					&s->value, u, GNOME_PRINT_PS_UNIT);
				if (u && !s->unit)
					gpa_spinbutton_set_unit (s, u->abbr);
			}
		}
		g_free (v);
	} else
		s->value = 0;

	if ((GPA_NODE_FLAGS (s->node) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED)
		gtk_widget_set_sensitive (s->spinbutton, FALSE);
	else
		gtk_widget_set_sensitive (s->spinbutton, TRUE);

	s->loading = FALSE;

	gpa_spinbutton_update (s);
}

static void
gpa_spinbutton_state_modified_cb (GPANode *node, guint flags, GPASpinbutton *spinbutton)
{
	gpa_spinbutton_load (spinbutton);
}

static void
gpa_spinbutton_save (GPASpinbutton *s)
{
	gchar *v;

	g_return_if_fail (GPA_IS_SPINBUTTON (s));
	g_return_if_fail (gpa_spinbutton_is_connected (s));

	if (s->loading || s->saving) return;

	s->saving = TRUE;

	if (!s->unit || !strcmp (s->unit, "%"))
		v = g_strdup_printf ("%f Pt", s->value);
	else
		v = g_strdup_printf ("%f %s", s->value * s->factor, s->unit);
	gpa_node_set_value (s->node, v);
	g_free (v);

	s->saving = FALSE;
}

#undef EPSILON
#define EPSILON 0.0000000001

static void
gpa_spinbutton_value_changed_cb (GtkAdjustment *a, GPASpinbutton *s)
{
	g_return_if_fail (GPA_IS_SPINBUTTON (s));
	g_return_if_fail (GTK_IS_ADJUSTMENT (a));

	if (s->updating) return;

	if (fabs (a->value / s->factor -s->value) < EPSILON) return;

	s->value = a->value / s->factor;
	gpa_spinbutton_save (s);
}

static void
gpa_spinbutton_config_modified_cb (GPANode *node, guint flags, GPASpinbutton *spinbutton)
{
	gpa_spinbutton_disconnect (spinbutton);
	gpa_spinbutton_connect (spinbutton);
	gpa_spinbutton_load (spinbutton);
}

static gint
gpa_spinbutton_construct (GPAWidget *gpaw)
{
	GPASpinbutton *s;
	GtkAdjustment *a;

	s = GPA_SPINBUTTON (gpaw);
	s->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	s->handler_config = g_signal_connect (G_OBJECT (s->config),
		"modified", (GCallback) gpa_spinbutton_config_modified_cb, s);
	gpa_spinbutton_connect (s);

	/* Create the spinbutton */
	a = GTK_ADJUSTMENT (gtk_adjustment_new (s->lower, s->lower,
		s->upper, s->step_increment, s->page_increment, s->page_size));
	s->spinbutton = gtk_spin_button_new (a, s->climb_rate, s->digits);
	gtk_widget_show (s->spinbutton);
	gtk_container_add (GTK_CONTAINER (s), s->spinbutton);
	g_signal_connect (a, "value_changed",
			  G_CALLBACK (gpa_spinbutton_value_changed_cb), s);

	/* Load the current settings */
	gpa_spinbutton_load (s);

	return TRUE;
}

GtkWidget *
gpa_spinbutton_new (GnomePrintConfig *config,
	const guchar *path, gdouble lower, gdouble upper,
	gdouble step_increment, gdouble page_increment,
	gdouble page_size, gdouble climb_rate, guint digits)
{
	GtkWidget *c;

	/* If the node does not exist, create it. */
	if (!gpa_node_lookup (GNOME_PRINT_CONFIG_NODE (config), path))
		gpa_key_insert (GNOME_PRINT_CONFIG_NODE (config), path, "");

	c = gpa_widget_new (GPA_TYPE_SPINBUTTON, NULL);
	GPA_SPINBUTTON (c)->lower = lower;
	GPA_SPINBUTTON (c)->upper = upper;
	GPA_SPINBUTTON (c)->step_increment = step_increment;
	GPA_SPINBUTTON (c)->page_increment = page_increment;
	GPA_SPINBUTTON (c)->page_size = page_size;
	GPA_SPINBUTTON (c)->climb_rate = climb_rate;
	GPA_SPINBUTTON (c)->digits = digits;
	GPA_SPINBUTTON (c)->path = g_strdup (path);
	gpa_widget_construct (GPA_WIDGET (c), config);

	return c;
}

typedef struct {
	guchar *abbr;
	gint digits;
	gfloat step_increment;
} GPASpinbuttonProps;
                                                                                
static const GPASpinbuttonProps props[] = {
	{N_("%"), 0, 1.0},         /* Percent must be first */
	{N_("Pt"), 1, 1.0},
	{N_("mm"), 1, 1.0},
	{N_("cm"), 2, 0.5},
	{N_("m"),  3, 0.01},
	{N_("in"), 2, 0.25},
	{NULL,     2, 1.0}          /* Default must be last */
};

void
gpa_spinbutton_set_unit (GPASpinbutton *s, const gchar *unit)
{
	const GnomePrintUnit *u = NULL;

	g_return_if_fail (GPA_IS_SPINBUTTON (s));
	g_return_if_fail (unit != NULL);

	if (s->unit && !strcmp (unit, s->unit)) return;

	if (!strcmp (unit, "%")) {
		 g_free (s->unit);
                 s->unit = g_strdup ("%");
		 s->factor = 100.;
	} else {
		u = gnome_print_unit_get_by_abbreviation (unit);
		if (!u) u = gnome_print_unit_get_by_name (unit);
		if (u) {
			g_free (s->unit);
			s->unit = g_strdup (u->abbr);
			s->factor = 1. / u->unittobase;
		}
	}
	gpa_spinbutton_update (s);
}

void
gpa_spinbutton_update (GPASpinbutton *s)
{
	guint i;
	GtkAdjustment *a;

	g_return_if_fail (GPA_IS_SPINBUTTON (s));

	if (s->updating) return;

	if (s->unit && !strcmp (s->unit, "%")) i = 0;
	else for (i = 1; props[i].abbr != NULL; i++)
		if (s->unit && !strcmp (s->unit, props[i].abbr)) break;

	/* Update the adjustment */
	a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (s->spinbutton));
	a->step_increment = props[i].step_increment;
	a->page_increment = props[i].step_increment * 10;
	a->upper = s->upper * s->factor;
	a->lower = s->lower * s->factor;
	s->updating = TRUE;
	gtk_adjustment_changed (a);
	s->updating = FALSE;

	/* Update the spinbutton */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON (s->spinbutton),
				    props[i].digits);
	s->updating = TRUE;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (s->spinbutton),
				   s->value * s->factor);
	s->updating = FALSE;
}
