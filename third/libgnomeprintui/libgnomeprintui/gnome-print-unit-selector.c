/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-unit-selector.c: A unit selector for gnome-print
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
 *    James Henstridge <james@daa.com.au>
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>
 *
 */

#include <config.h>

#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

#include "gnome-print-unit-selector.h"

#define noGPP_VERBOSE

struct _GnomePrintUnitSelector {
	GtkHBox box;

	GtkWidget *menu;

	guint bases;
	GList *units;
	const GnomePrintUnit *unit;
	gdouble ctmscale, devicescale;
	guint plural : 1;
	guint abbr : 1;

	GList *adjustments;
};

struct _GnomePrintUnitSelectorClass {
	GtkHBoxClass parent_class;

	void (* modified) (GnomePrintUnitSelector *selector);
};

static void gnome_print_unit_selector_class_init (GnomePrintUnitSelectorClass *klass);
static void gnome_print_unit_selector_init (GnomePrintUnitSelector *selector);
static void gnome_print_unit_selector_finalize (GObject *object);

static GtkHBoxClass *unit_selector_parent_class;

enum {
	GNOME_PRINT_UNIT_SELECTOR_MODIFIED,
	GNOME_PRINT_UNIT_SELECTOR_LAST_SIGNAL
};

static guint gnome_print_unit_selector_signals [GNOME_PRINT_UNIT_SELECTOR_LAST_SIGNAL] = { 0 };

GType
gnome_print_unit_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintUnitSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_unit_selector_class_init,
			NULL, NULL,
			sizeof (GnomePrintUnitSelector),
			0,
			(GInstanceInitFunc) gnome_print_unit_selector_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePrintUnitSelector", &info, 0);
	}
	return type;
}

static void
gnome_print_unit_selector_class_init (GnomePrintUnitSelectorClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	unit_selector_parent_class = g_type_class_peek_parent (klass);

	gnome_print_unit_selector_signals[GNOME_PRINT_UNIT_SELECTOR_MODIFIED] =
		g_signal_new ("modified",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GnomePrintUnitSelectorClass,
					       modified),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	
	object_class->finalize = gnome_print_unit_selector_finalize;
}

static void
cb_gpus_opt_menu_changed (GtkOptionMenu *menu, GnomePrintUnitSelector *us)
{
	g_signal_emit (G_OBJECT (us),
		       gnome_print_unit_selector_signals[GNOME_PRINT_UNIT_SELECTOR_MODIFIED],
		       0);
}

static void
gnome_print_unit_selector_init (GnomePrintUnitSelector *us)
{
	us->ctmscale = 1.0;
	us->devicescale = 1.0;
	us->abbr = FALSE;
	us->plural = TRUE;

	us->menu = gtk_option_menu_new ();
	g_signal_connect (G_OBJECT (us->menu),
			  "changed", G_CALLBACK (cb_gpus_opt_menu_changed), us);
	gtk_widget_show (us->menu);
	gtk_box_pack_start (GTK_BOX (us), us->menu, TRUE, TRUE, 0);
}

static void
gnome_print_unit_selector_finalize (GObject *object)
{
	GnomePrintUnitSelector *selector;
	
	selector = GNOME_PRINT_UNIT_SELECTOR (object);

	if (selector->menu) {
		selector->menu = NULL;
	}

	while (selector->adjustments) {
		g_object_unref (G_OBJECT (selector->adjustments->data));
		selector->adjustments = g_list_remove (selector->adjustments, selector->adjustments->data);
	}

	if (selector->units) {
		gnome_print_unit_free_list (selector->units);
	}

	selector->unit = NULL;

	G_OBJECT_CLASS (unit_selector_parent_class)->finalize (object);
}

GtkWidget *
gnome_print_unit_selector_new (guint bases)
{
	GnomePrintUnitSelector *us;

	us = g_object_new (GNOME_TYPE_PRINT_UNIT_SELECTOR, NULL);

	gnome_print_unit_selector_set_bases (us, bases);

	return (GtkWidget *) us;
}

static void
gnome_print_unit_selector_recalculate_adjustments (GnomePrintUnitSelector *us,
						   const GnomePrintUnit *unit)
{
	GList *l;
	const GnomePrintUnit *old;

	old = us->unit;
	us->unit = unit;
	for (l = us->adjustments; l != NULL; l = l->next) {
		GtkAdjustment *adj;
		adj = GTK_ADJUSTMENT (l->data);
#ifdef GPP_VERBOSE
		g_print ("Old val %g ... ", adj->value);
#endif
		gnome_print_convert_distance_full (&adj->value, old, unit,
						   us->ctmscale, us->devicescale);
		gnome_print_convert_distance_full (&adj->lower, old, unit,
						   us->ctmscale, us->devicescale);
		gnome_print_convert_distance_full (&adj->upper, old, unit,
						   us->ctmscale, us->devicescale);
#ifdef GPP_VERBOSE
		g_print ("new val %g\n", adj->value);
#endif
		gtk_adjustment_changed (adj);
		gtk_adjustment_value_changed (adj);

	}

}

const GnomePrintUnit *
gnome_print_unit_selector_get_unit (GnomePrintUnitSelector *us)
{
	g_return_val_if_fail (us != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us), NULL);

	return us->unit;
}

static void
gpus_unit_activate (GtkWidget *widget, GnomePrintUnitSelector *us)
{
	const GnomePrintUnit *unit;

	unit = g_object_get_data (G_OBJECT (widget), "unit");
	g_return_if_fail (unit != NULL);

#ifdef GPP_VERBOSE
	g_print ("Old unit %s new unit %s\n", us->unit->name, unit->name);
#endif
	if (us->unit == unit)
		return;

	gnome_print_unit_selector_recalculate_adjustments (us, unit);
}

static void
gpus_rebuild_menu (GnomePrintUnitSelector *us)
{
	GtkWidget *m, *i;
	GList *l;
	gint pos, p;

	m = gtk_menu_new ();
	gtk_widget_show (m);

	pos = p = 0;
	for (l = us->units; l != NULL; l = l->next) {
		const GnomePrintUnit *u;
		u = l->data;
		i = gtk_menu_item_new_with_label ((us->abbr) ? (us->plural) ? u->abbr_plural : u->abbr : (us->plural) ? u->plural : u->name);
		g_object_set_data (G_OBJECT (i), "unit", (gpointer) u);
		g_signal_connect (G_OBJECT (i), "activate", (GCallback) gpus_unit_activate, us);
		gtk_widget_show (i);
		gtk_menu_shell_append (GTK_MENU_SHELL (m), i);
		if (u == us->unit)
			pos = p;
		p += 1;
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (us->menu), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (us->menu), pos);
}

void
gnome_print_unit_selector_set_bases (GnomePrintUnitSelector *us, guint bases)
{
	GList *units;

	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));

	if (bases == us->bases)
		return;

	units = gnome_print_unit_get_list (bases);
	g_return_if_fail (units != NULL);
	gnome_print_unit_free_list (us->units);
	us->units = units;
	us->unit = units->data;

	gpus_rebuild_menu (us);
}

void
gnome_print_unit_selector_set_unit (GnomePrintUnitSelector *us, const GnomePrintUnit *unit)
{
	gint pos;

	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (unit != NULL);

	if (unit == us->unit)
		return;

	pos = g_list_index (us->units, unit);
	g_return_if_fail (pos >= 0);

	gnome_print_unit_selector_recalculate_adjustments (us,  unit);
	gtk_option_menu_set_history (GTK_OPTION_MENU (us->menu), pos);
}

void
gnome_print_unit_selector_add_adjustment (GnomePrintUnitSelector *us, GtkAdjustment *adj)
{
	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (adj != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	g_return_if_fail (!g_list_find (us->adjustments, adj));

	g_object_ref (G_OBJECT (adj));
	us->adjustments = g_list_prepend (us->adjustments, adj);
}

void
gnome_print_unit_selector_remove_adjustment (GnomePrintUnitSelector *us, GtkAdjustment *adj)
{
	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (adj != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	g_return_if_fail (g_list_find (us->adjustments, adj));

	us->adjustments = g_list_remove (us->adjustments, adj);
	g_object_unref (G_OBJECT (adj));
}
