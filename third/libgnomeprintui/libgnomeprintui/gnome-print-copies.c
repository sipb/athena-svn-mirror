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

#define __GNOME_PRINT_COPIES_C__

#include <config.h>

#include <atk/atkobject.h>
#include <atk/atkimage.h>
#include <atk/atkrelationset.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkimage.h>

#include "gnome-print-i18n.h"
#include "gnome-print-copies.h"

enum {COPIES_SET, LAST_SIGNAL};

struct _GnomePrintCopiesSelection {
	GtkVBox vbox;
  
	guint changing : 1;

	GtkWidget *copies;
	GtkWidget *collate;
	GtkWidget *collate_image;
};

struct _GnomePrintCopiesSelectionClass {
	GtkVBoxClass parent_class;

	void (* copies_set) (GnomePrintCopiesSelection * gpc, gint copies, gboolean collate);
};

static void gnome_print_copies_selection_class_init (GnomePrintCopiesSelectionClass *class);
static void gnome_print_copies_selection_init (GnomePrintCopiesSelection *gspaper);
static void gnome_print_copies_selection_destroy (GtkObject *object);

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

GtkType
gnome_print_copies_selection_get_type (void)
{
	static GtkType copies_type = 0;
	if (!copies_type) {
		GtkTypeInfo copies_info = {
			"GnomePrintCopiesSelection",
			sizeof (GnomePrintCopiesSelection),
			sizeof (GnomePrintCopiesSelectionClass),
			(GtkClassInitFunc) gnome_print_copies_selection_class_init,
			(GtkObjectInitFunc) gnome_print_copies_selection_init,
			NULL, NULL, NULL
		};
		copies_type = gtk_type_unique (gtk_vbox_get_type (), &copies_info);
	}
	return copies_type;
}

static void
gnome_print_copies_selection_class_init (GnomePrintCopiesSelectionClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	gpc_signals[COPIES_SET] = gtk_signal_new ("copies_set",
						  GTK_RUN_FIRST,
						  G_OBJECT_CLASS_TYPE (klass),
						  GTK_SIGNAL_OFFSET (GnomePrintCopiesSelectionClass, copies_set),
						  gtk_marshal_NONE__INT_INT,
						  GTK_TYPE_NONE,
						  2, GTK_TYPE_INT, GTK_TYPE_INT);

	object_class->destroy = gnome_print_copies_selection_destroy;
}

static void
collate_toggled (GtkWidget *widget, GnomePrintCopiesSelection *gpc)
{
	gint copies;
	gboolean collate;
	GdkPixbuf *pb;

	copies = gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	pb = gdk_pixbuf_new_from_xpm_data (collate ? collate_xpm : nocollate_xpm);
	gtk_image_set_from_pixbuf (GTK_IMAGE (gpc->collate_image), pb);
	gdk_pixbuf_unref (pb);

	if (gpc->changing) return;

	gtk_signal_emit ((GtkObject *) gpc, gpc_signals[COPIES_SET], copies, collate);
}

static void
copies_changed (GtkAdjustment *adj, GnomePrintCopiesSelection *gpc)
{
	gint copies;
	gboolean collate;

	copies = gtk_adjustment_get_value (adj);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	if (gpc->changing)
		return;

	gtk_signal_emit ((GtkObject *) gpc, gpc_signals[COPIES_SET], copies, collate);
}

static void
gnome_print_copies_selection_init (GnomePrintCopiesSelection *gpc)
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
	gdk_pixbuf_unref (pb);
	gtk_widget_show (gpc->collate_image);
	gtk_table_attach_defaults ((GtkTable *)table, gpc->collate_image, 0, 1, 1, 2);
	atko = gtk_widget_get_accessible (gpc->collate_image);
	atk_image_set_image_description (ATK_IMAGE (atko), _("Image showing the collation sequence when multiple copies of the document are printed"));

	gpc->collate = gtk_check_button_new_with_mnemonic (_("_Collate"));
	gtk_widget_show(gpc->collate);
	gtk_table_attach_defaults((GtkTable *)table, gpc->collate, 1, 2, 1, 2);

	atko = gtk_widget_get_accessible (gpc->collate);
	atk_object_set_description (atko, _("If copies of the document are printed separately, one after another, rather than being interleaved"));

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
			    GTK_SIGNAL_FUNC (copies_changed), gpc);
	gtk_signal_connect (GTK_OBJECT (gpc->collate), "toggled", 
			    GTK_SIGNAL_FUNC (collate_toggled), gpc);
}

/**
 * gnome_print_copies_selection_new:
 *
 * Create a new GnomePrintCopies widget.
 * 
 * Return value: A new GnomePrintCopies widget.
 **/

GtkWidget *
gnome_print_copies_selection_new (void)
{
	return GTK_WIDGET (gtk_type_new (GNOME_TYPE_PRINT_COPIES_SELECTION));
}

static void
gnome_print_copies_selection_destroy (GtkObject *object)
{
	GnomePrintCopiesSelection *gpc;
	
	gpc = GNOME_PRINT_COPIES_SELECTION (object);

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
gnome_print_copies_selection_set_copies (GnomePrintCopiesSelection *gpc, gint copies, gint collate)
{
	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES_SELECTION (gpc));

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
gnome_print_copies_selection_get_copies (GnomePrintCopiesSelection *gpc)
{
	g_return_val_if_fail (gpc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES_SELECTION (gpc), 0);

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
gnome_print_copies_selection_get_collate (GnomePrintCopiesSelection *gpc)
{
	g_return_val_if_fail (gpc != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES_SELECTION (gpc), FALSE);

	return GTK_TOGGLE_BUTTON (gpc->collate)->active?TRUE:FALSE;
}






