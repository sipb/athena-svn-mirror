/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-copies.c: A system print copies widget
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
 *    Michael Zucchi <notzed@ximian.com>
 *
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 */

/* FIXME: Hook this up to GnomePrintConfig and no need to have signals (Chema) */

#include <config.h>

#include <atk/atk.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gnome-print-copies.h"
#include "gnome-printui-marshal.h"

enum {COPIES_SET, LAST_SIGNAL};

struct _GnomePrintCopiesSelector {
	GtkVBox vbox;
  
	guint changing : 1;

	GtkWidget *copies;
	GtkWidget *collate;
	GtkWidget *collate_image;
};

struct _GnomePrintCopiesSelectorClass {
	GtkVBoxClass parent_class;

	void (* copies_set) (GnomePrintCopiesSelector * gpc, gint copies, gboolean collate);
};

static void gnome_print_copies_selector_class_init (GnomePrintCopiesSelectorClass *class);
static void gnome_print_copies_selector_init       (GnomePrintCopiesSelector *gspaper);
static void gnome_print_copies_selector_destroy    (GtkObject *object);

/* again, these images may be here temporarily */

/* XPM */
static const char *collate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #020202",
"@	c #FFFFFF",
"#	c #010101",
"$	c #070707",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"..+++++++++##++++++$@@@@@@@@@..   ..+++++++++##++++++$@@@@@@@@@..",
"..+++++++++##+++++#+@@@@@@@@@..   ..+++++++++##+++++#+@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@..@@@@..   ..@@@@@@@@@@@@@@@@++@@@..@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@.@@@..   ..@@@@@@@@@@@@@@@@++@@.@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@.@@@..   ..@@@@@@@@@@@@@@@@++@@@@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@.@@@@@..   ..@@@@@@@@@@@@@@@@++@@@.@@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@@@@@..   ..@@@@@@@@@@@@@@@@++@@.@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@....@@@..   ..@@@@@@@@@@@@@@@@++@@....@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@.@@@@.............   ..@@@@@@@@@@@.@@@@.............",
"..@@@@@@@@@@..@@@@.............   ..@@@@@@@@@@..@@@@.............",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@...@@@..              ..@@@@@@@@@@...@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"....................              ....................           ",
"....................              ....................           "};

/* XPM */
static const char *nocollate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #020202",
"#	c #010101",
"$	c #070707",
"           ....................              ....................",
"           ....................              ....................",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"..@@@@@@@@@##@@@@@@$+++++++++..   ..@@@@@@@@@##@@@@@@$+++++++++..",
"..@@@@@@@@@##@@@@@#@+++++++++..   ..@@@@@@@@@##@@@@@#@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++..++++..",
"..++++++++++++++++@@+++..++++..   ..++++++++++++++++@@++.++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++.+++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++.++++++..",
"..++++++++++++++++@@+++...+++..   ..++++++++++++++++@@++....+++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..+++++++++++.++++.............   ..++++++++++..++++.............",
"..++++++++++..++++.............   ..+++++++++.++.+++.............",
"..+++++++++++.++++..              ..++++++++++++.+++..           ",
"..+++++++++++.++++..              ..+++++++++++.++++..           ",
"..+++++++++++.++++..              ..++++++++++.+++++..           ",
"..+++++++++++.++++..              ..+++++++++.++++++..           ",
"..++++++++++...+++..              ..+++++++++....+++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"....................              ....................           ",
"....................              ....................           "};


static GtkVBoxClass *parent_class;
static guint gpc_signals[LAST_SIGNAL] = {0};

GType
gnome_print_copies_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintCopiesSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_copies_selector_class_init,
			NULL, NULL,
			sizeof (GnomePrintCopiesSelector),
			0,
			(GInstanceInitFunc) gnome_print_copies_selector_init,
			NULL,
		};
		type = g_type_register_static (GTK_TYPE_VBOX, "GnomePrintCopiesSelector", &info, 0);
	}
	return type;
}

static void
gnome_print_copies_selector_class_init (GnomePrintCopiesSelectorClass *klass)
{
	GtkObjectClass *gtk_object_class;
	GObjectClass *object_class;
  
	object_class = (GObjectClass *) klass;
	gtk_object_class = (GtkObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	gpc_signals[COPIES_SET] = g_signal_new ("copies_set",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (GnomePrintCopiesSelectorClass, copies_set),
						NULL, NULL,
						libgnomeprintui_marshal_VOID__INT_BOOLEAN,
						G_TYPE_NONE,
						2,
						G_TYPE_INT, G_TYPE_BOOLEAN);

	gtk_object_class->destroy = gnome_print_copies_selector_destroy;
}

static void
collate_toggled (GtkWidget *widget, GnomePrintCopiesSelector *gpc)
{
	gint copies;
	gboolean collate;
	GdkPixbuf *pb;

	copies = gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	pb = gdk_pixbuf_new_from_xpm_data (collate ? collate_xpm : nocollate_xpm);
	gtk_image_set_from_pixbuf (GTK_IMAGE (gpc->collate_image), pb);
	g_object_unref (G_OBJECT (pb));

	if (gpc->changing)
		return;

	g_signal_emit (G_OBJECT (gpc), gpc_signals[COPIES_SET], 0, copies, collate);
}

static void
copies_changed (GtkAdjustment *adj, GnomePrintCopiesSelector *gpc)
{
	gint copies;
	gboolean collate;

	copies = gtk_adjustment_get_value (adj);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	if (gpc->changing)
		return;

	g_signal_emit (G_OBJECT (gpc), gpc_signals[COPIES_SET], 0, copies, collate);
}

static void
gnome_print_copies_selector_init (GnomePrintCopiesSelector *gpc)
{
	GtkWidget *table, *label, *frame;
	GtkAdjustment *adj;
	GdkPixbuf *pb;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];
	AtkObject *atko;

	frame = gtk_frame_new (_("Copies"));
	gtk_container_add (GTK_CONTAINER (gpc), frame);
	gtk_widget_show (frame);

	table = gtk_table_new(2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER(table), 4);
	gtk_container_add (GTK_CONTAINER(frame), GTK_WIDGET (table));
	gtk_widget_show (table);

	label = gtk_label_new_with_mnemonic (_("N_umber of copies:"));
	gtk_widget_show (label);
	gtk_table_attach_defaults ((GtkTable *)table, label, 0, 1, 0, 1);

	adj = (GtkAdjustment *) gtk_adjustment_new(1, 1, 1000, 1.0, 10.0, 10.0);
	gpc->copies = gtk_spin_button_new (adj, 1.0, 0);
	gtk_widget_show (gpc->copies);
	gtk_table_attach_defaults ((GtkTable *)table, gpc->copies, 1, 2, 0, 1);

	gtk_label_set_mnemonic_widget ((GtkLabel *) label, gpc->copies);

	/* Add a LABELLED_BY relation from the entry to the label. */
	atko = gtk_widget_get_accessible (gpc->copies);
	relation_set = atk_object_ref_relation_set (atko);
	relation_targets[0] = gtk_widget_get_accessible (label);
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));

	pb = gdk_pixbuf_new_from_xpm_data (nocollate_xpm);
	gpc->collate_image = gtk_image_new_from_pixbuf (pb);
	g_object_unref (G_OBJECT (pb));
	gtk_widget_show (gpc->collate_image);
	gtk_table_attach_defaults ((GtkTable *)table, gpc->collate_image, 0, 1, 1, 2);
	atko = gtk_widget_get_accessible (gpc->collate_image);
	atk_image_set_image_description (ATK_IMAGE (atko), _("Image showing the collation sequence when multiple copies of the document are printed"));

	gpc->collate = gtk_check_button_new_with_mnemonic (_("_Collate"));
	gtk_widget_show(gpc->collate);
	gtk_table_attach_defaults((GtkTable *)table, gpc->collate, 1, 2, 1, 2);

	atko = gtk_widget_get_accessible (gpc->collate);
	atk_object_set_description (atko, _("If copies of the document are printed separately, one after another, rather than being interleaved"));

	g_signal_connect (G_OBJECT (adj), "value_changed",
			  (GCallback) copies_changed, gpc);
	g_signal_connect (G_OBJECT (gpc->collate), "toggled",
			  (GCallback) collate_toggled, gpc);
}

/**
 * gnome_print_copies_selector_new:
 *
 * Create a new GnomePrintCopies widget.
 * 
 * Return value: A new GnomePrintCopies widget.
 **/

GtkWidget *
gnome_print_copies_selector_new (void)
{
	return GTK_WIDGET (g_object_new (GNOME_TYPE_PRINT_COPIES_SELECTOR, NULL));
}

static void
gnome_print_copies_selector_destroy (GtkObject *object)
{
	GnomePrintCopiesSelector *gpc;
	
	gpc = GNOME_PRINT_COPIES_SELECTOR (object);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * gnome_print_copies_set_copies:
 * @gpc: An initialised GnomePrintCopies widget.
 * @copies: New number of copies.
 * @collate: New collation status.
 * 
 * Set the number of copies and collation sequence to be displayed.
 **/

void
gnome_print_copies_selector_set_copies (GnomePrintCopiesSelector *gpc, gint copies, gint collate)
{
	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES_SELECTOR (gpc));

	gpc->changing = TRUE;

	gtk_toggle_button_set_active ((GtkToggleButton *) gpc->collate, collate);

	gpc->changing = FALSE;

	gtk_spin_button_set_value ((GtkSpinButton *) gpc->copies, copies);
}

/**
 * gnome_print_copies_get_copies:
 * @gpc: An initialised GnomePrintCopies widget.
 * 
 * Retrieve the number of copies set
 *
 * Return value: Number of copies set
 **/

gint
gnome_print_copies_selector_get_copies (GnomePrintCopiesSelector *gpc)
{
	g_return_val_if_fail (gpc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES_SELECTOR (gpc), 0);

	return gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
}

/**
 * gnome_print_copies_get_collate:
 * @gpc: An initialised GnomePrintCopies widget.
 * 
 * Retrieve the collation status
 *
 * Return value: Collation status
 **/

gboolean
gnome_print_copies_selector_get_collate (GnomePrintCopiesSelector *gpc)
{
	g_return_val_if_fail (gpc != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES_SELECTOR (gpc), FALSE);

	return GTK_TOGGLE_BUTTON (gpc->collate)->active?TRUE:FALSE;
}






