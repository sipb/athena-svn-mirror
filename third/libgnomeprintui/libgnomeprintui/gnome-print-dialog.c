/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-dialog.c: A system print dialog
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
 *    Michael Zucchi <notzed@helixcode.com>
 *    Chema Celorio <chema@celorio.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 */

#include <config.h>

#include <time.h>
#include <atk/atk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libgnomeprint/gnome-print-config.h>

#include "gnome-print-i18n.h"
#include "gnome-printer-selector.h"
#include "gnome-print-paper-selector.h"
#include "gnome-print-copies.h"
#include "gnome-print-dialog.h"
#include "gnome-print-dialog-private.h"

#define PAD 6

enum {
	PROP_0,
	PROP_PRINT_CONFIG
};

static void gnome_print_dialog_class_init (GnomePrintDialogClass *class);
static void gnome_print_dialog_init (GnomePrintDialog *dialog);

static GtkWidget *gpd_create_job_page (GnomePrintDialog *gpd);

static GtkDialogClass *parent_class;

GType
gnome_print_dialog_get_type (void)
{	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintDialogClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_dialog_class_init,
			NULL, NULL,
			sizeof (GnomePrintDialog),
			0,
			(GInstanceInitFunc) gnome_print_dialog_init,
			NULL,
		};
		type = g_type_register_static (GTK_TYPE_DIALOG, "GnomePrintDialog", &info, 0);
	}
	return type;
}

static void
gnome_print_dialog_destroy (GtkObject *object)
{
	GnomePrintDialog *gpd;
	
	gpd = GNOME_PRINT_DIALOG (object);

	if (gpd->config) {
		gpd->config = gnome_print_config_unref (gpd->config);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnome_print_dialog_set_property (GObject      *object,
				 guint         prop_id,
				 GValue const *value,
				 GParamSpec   *pspec)
{
	gpointer new_config;
	GnomePrintDialog *gpd = GNOME_PRINT_DIALOG (object);

	switch (prop_id) {
	case PROP_PRINT_CONFIG:
		new_config = g_value_get_pointer (value);
		if (new_config) {
			if (gpd->config)
				gnome_print_config_unref (gpd->config);
			gpd->config = g_value_get_pointer (value);
			gnome_print_config_ref (gpd->config);
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
gnome_print_dialog_class_init (GnomePrintDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (GTK_TYPE_DIALOG);

	object_class->destroy = gnome_print_dialog_destroy;

	G_OBJECT_CLASS (class)->set_property = gnome_print_dialog_set_property;
	g_object_class_install_property (G_OBJECT_CLASS (class),
					 PROP_PRINT_CONFIG,
					 g_param_spec_pointer ("print_config",
							       "Print Config",
							       "Printing Configuration to be used",
							       G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));
}

static void
gnome_print_dialog_init (GnomePrintDialog *gpd)
{
	/* Empty */
}

static void
gpd_copies_set (GnomePrintCopiesSelector *gpc, gint copies, gboolean collate, GnomePrintDialog *gpd)
{
	if (gpd->config) {
		gnome_print_config_set_int (gpd->config, GNOME_PRINT_KEY_NUM_COPIES, copies);
		gnome_print_config_set_boolean (gpd->config, GNOME_PRINT_KEY_COLLATE, collate);
	}
}

static GtkWidget *
gpd_create_job_page (GnomePrintDialog *gpd)
{
	GtkWidget *hb, *vb, *f, *c;

	hb = gtk_hbox_new (FALSE, 0);

	vb = gtk_vbox_new (FALSE, PAD);
	gtk_widget_show (vb);

	gtk_box_pack_start (GTK_BOX (hb), vb, FALSE, FALSE, 0);

	f = gtk_frame_new (_("Print range"));
	gtk_widget_hide (f);
	gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (hb), "range", f);

	c = gnome_print_copies_selector_new ();
	if (gpd)
		g_signal_connect (G_OBJECT (c), "copies_set", (GCallback) gpd_copies_set, gpd);
	gtk_widget_hide (c);
	gtk_box_pack_start (GTK_BOX (vb), c, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (hb), "copies", c);

	return hb;
}

static GtkWidget *
gpd_create_range (gint flags, GtkWidget *range, const guchar *clabel, const guchar *rlabel)
{
	GtkWidget *t, *rb;
	GSList *group;
	gint row;

	t = gtk_table_new (4, 2, FALSE);

	group = NULL;
	row = 0;

	if (flags & GNOME_PRINT_RANGE_CURRENT) {
		rb = gtk_radio_button_new_with_mnemonic (group, clabel);
		g_object_set_data (G_OBJECT (t), "current", rb);
		gtk_widget_show (rb);
		gtk_table_attach (GTK_TABLE (t), rb, 0, 1, row, row + 1, GTK_FILL | GTK_EXPAND, 
				  GTK_FILL, 0, 0);
		group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb));
		row += 1;
	}

	if (flags & GNOME_PRINT_RANGE_ALL) {
		rb = gtk_radio_button_new_with_mnemonic(group, _("_All"));
		g_object_set_data (G_OBJECT (t), "all", rb);
		gtk_widget_show (rb);
		gtk_table_attach (GTK_TABLE (t), rb, 0, 1, row, row + 1, GTK_FILL | GTK_EXPAND, 
				  GTK_FILL, 0, 0);
		group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb));
		row += 1;
	}

	if (flags & GNOME_PRINT_RANGE_RANGE) {
		rb = gtk_radio_button_new_with_mnemonic (group, rlabel);
		g_object_set_data (G_OBJECT (t), "range", rb);
		gtk_widget_show (rb);
		gtk_table_attach (GTK_TABLE (t), rb, 0, 1, row, row + 1, GTK_FILL | GTK_EXPAND, 
				  GTK_FILL, 0, 0);
		g_object_set_data (G_OBJECT (t), "range-widget", range);
		gtk_table_attach (GTK_TABLE (t), range, 1, 2, row, row + 1, GTK_FILL, 0, 0, 0);
		group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb));
		row += 1;
	}

	if ((flags & GNOME_PRINT_RANGE_SELECTION) || (flags & GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE)) {
		rb = gtk_radio_button_new_with_mnemonic (group, _("_Selection"));
		g_object_set_data (G_OBJECT (t), "selection", rb);
		gtk_widget_show (rb);
		gtk_widget_set_sensitive (rb, !(flags & GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE));
		gtk_table_attach (GTK_TABLE (t), rb, 0, 1, row, row + 1, GTK_FILL | GTK_EXPAND, 
				  GTK_FILL, 0, 0);
		group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb));
		row += 1;
	}

	return t;
}

/**
 * gnome_print_dialog_new:
 * @gpj: GnomePrintJob
 * @title: Title of window.
 * @flags: Options for created widget.
 * 
 * Create a new gnome-print-dialog window.
 *
 * The following options flags are available:
 * GNOME_PRINT_DIALOG_RANGE: A range widget container will be created.
 * A range widget must be created separately, using one of the
 * gnome_print_dialog_construct_* functions.
 * GNOME_PRINT_DIALOG_COPIES: A copies widget will be created.
 * 
 * Return value: A newly created and initialised widget.
 **/
GtkWidget *
gnome_print_dialog_new (GnomePrintJob *gpj, const guchar *title, gint flags)
{
	GnomePrintConfig *gpc = NULL;
	GnomePrintDialog *gpd;

	gpd = GNOME_PRINT_DIALOG (g_object_new (GNOME_TYPE_PRINT_DIALOG, NULL));

	if (gpd) {
		if (gpj)
			gpc = gnome_print_job_get_config (gpj);
		if (gpc == NULL)
			gpc = gnome_print_config_default ();
		gpd->config = gpc;
		gnome_print_dialog_construct (gpd, title, flags);
	}

	return GTK_WIDGET (gpd);
}

/**
 * gnome_print_dialog_construct:
 * @gpd: A created GnomePrintDialog.
 * @title: Title of the window.
 * @flags: Initialisation options, see gnome_print_dialog_new().
 * 
 * Used for language bindings to post-initialise an object instantiation.
 *
 */
void
gnome_print_dialog_construct (GnomePrintDialog *gpd, const guchar *title, gint flags)
{
	GtkWidget *pp_button; 

	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (gpd));

	if (gpd->config) {
		GtkWidget *hb, *p, *l;
		gpd->notebook = gtk_notebook_new ();
		gtk_container_set_border_width (GTK_CONTAINER (gpd->notebook), 4);
		gtk_widget_show (gpd->notebook);
	
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG (gpd)->vbox), gpd->notebook);
		
		/* Add the job page, if it has any content */
		gpd->job = gpd_create_job_page (gpd);
		gtk_container_set_border_width (GTK_CONTAINER (gpd->job), 4);
		l = gtk_label_new_with_mnemonic (_("Job"));
		gtk_widget_show (l);
		gtk_notebook_append_page (GTK_NOTEBOOK (gpd->notebook), gpd->job, l);
		
		/* Add the printers page */
		hb = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (hb);
		
		gpd->printer = gnome_printer_selector_new (gpd->config);
		gtk_container_set_border_width (GTK_CONTAINER (hb), 4);
		gtk_widget_show (gpd->printer);
		
		gtk_box_pack_start (GTK_BOX (hb), gpd->printer, TRUE, TRUE, 0);
		
		l = gtk_label_new_with_mnemonic (_("Printer"));
		gtk_widget_show (l);
		
		gtk_notebook_append_page (GTK_NOTEBOOK (gpd->notebook), hb, l);
		
		/* Add a paper page */
		p = gnome_paper_selector_new (gpd->config);
		gtk_container_set_border_width (GTK_CONTAINER (p), 4);
		gtk_widget_show (p);
		
		l = gtk_label_new_with_mnemonic (_("Paper"));
		gtk_widget_show (l);
		
		gtk_notebook_append_page (GTK_NOTEBOOK (gpd->notebook), p, l);
	} 
	else {
		GtkWidget *label;
		label = gtk_label_new (_("Error while loading printer configuration"));
		gtk_widget_show (label);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (gpd)->vbox), label, TRUE, TRUE, 0);
	}

	gtk_window_set_title (GTK_WINDOW (gpd), (title) ? title : (const guchar *) _("Gnome Print Dialog"));

	gtk_dialog_add_buttons (GTK_DIALOG (gpd),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_PRINT, GNOME_PRINT_DIALOG_RESPONSE_PRINT,
				NULL);

	pp_button = gtk_dialog_add_button (GTK_DIALOG (gpd),
					   GTK_STOCK_PRINT_PREVIEW, 
					   GNOME_PRINT_DIALOG_RESPONSE_PREVIEW);

	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (GTK_DIALOG (gpd)->action_area), 
					    pp_button, 
					    TRUE);

	gtk_dialog_set_default_response (GTK_DIALOG (gpd),
					 GNOME_PRINT_DIALOG_RESPONSE_PRINT);

	/* Construct job options, if needed */
	if (flags & GNOME_PRINT_DIALOG_RANGE) {
		GtkWidget *r;
		r = g_object_get_data (G_OBJECT (gpd->job), "range");
		if (r) {
			gtk_widget_show (r);
			gtk_widget_show (gpd->job);
		}
	}

	if (flags & GNOME_PRINT_DIALOG_COPIES) {
		GtkWidget *c;
		c = g_object_get_data (G_OBJECT (gpd->job), "copies");
		if (c) {
			gtk_widget_show (c);
			gtk_widget_show (gpd->job);
		}
	}
}

/**
 * gnome_print_dialog_construct_range_custom:
 * @gpd: A GnomePrintDialog for which a range was requested.
 * @custom: A widget which will be placed in a "Range" frame in the
 * main display.
 * 
 * Install a custom range specification widget.
 **/
void
gnome_print_dialog_construct_range_custom (GnomePrintDialog *gpd, GtkWidget *custom)
{
	GtkWidget *f, *r;

	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (gpd));
	g_return_if_fail (custom != NULL);
	g_return_if_fail (GTK_IS_WIDGET (custom));

	f = g_object_get_data (G_OBJECT (gpd->job), "range");
	g_return_if_fail (f != NULL);
	r = g_object_get_data (G_OBJECT (f), "range");
	if (r)
		gtk_container_remove (GTK_CONTAINER (f), r);

	gtk_widget_show (custom);
	gtk_widget_show (gpd->job);
	gtk_container_add (GTK_CONTAINER (f), custom);
	g_object_set_data (G_OBJECT (f), "range", custom);
}

/**
 * gnome_print_dialog_construct_range_any:
 * @gpd: An initialise GnomePrintDialog, which can contain a range.
 * @flags: Options flags, which ranges are displayed.
 * @range_widget: Widget to display for the range option.
 * @currentlabel: Label to display next to the 'current page' button.
 * @rangelabel: Label to display next to the 'range' button.
 * 
 * Create a generic range area within the print range dialogue.  The flags
 * field contains a mask of which options you wish displayed:
 *
 * GNOME_PRINT_RANGE_CURRENT: A label @currentlabel will be displayed.
 * GNOME_PRINT_RANGE_ALL: A label "All" will be displayed.
 * GNOME_PRINT_RANGE_RANGE: A label @rangelabel will be displayed, next
 * to the range specification widget @range_widget.
 * GNOME_PRINT_RANGE_SELECTION: A label "Selection" will be displayed.
 * 
 **/
void
gnome_print_dialog_construct_range_any (GnomePrintDialog *gpd, gint flags, GtkWidget *range_widget,
					const guchar *currentlabel, const guchar *rangelabel)
{
	GtkWidget *f, *r;

	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (gpd));
	g_return_if_fail (!range_widget || GTK_IS_WIDGET (range_widget));
	g_return_if_fail (!(range_widget && !(flags & GNOME_PRINT_RANGE_RANGE)));
	g_return_if_fail (!(!range_widget && (flags & GNOME_PRINT_RANGE_RANGE)));
	g_return_if_fail (!((flags & GNOME_PRINT_RANGE_SELECTION) && (flags & GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE)));

	f = g_object_get_data (G_OBJECT (gpd->job), "range");
	g_return_if_fail (f != NULL);
	r = g_object_get_data (G_OBJECT (f), "range");
	if (r)
		gtk_container_remove (GTK_CONTAINER (f), r);

	r = gpd_create_range (flags, range_widget, currentlabel, rangelabel);

	if (r) {
		gtk_widget_show (r);
		gtk_widget_show (gpd->job);
		gtk_container_add (GTK_CONTAINER (f), r);
	}

	g_object_set_data (G_OBJECT (f), "range", r);
}

/**
 * gnome_print_dialog_construct_range_page:
 * @gpd: An initialise GnomePrintDialog, which can contain a range.
 * @flags: Option flags.  See gnome_print_dialog_construct_any().
 * @start: First page which may be printed.
 * @end: Last page which may be printed.
 * @currentlabel: Label text for current option.
 * @rangelabel: Label text for range option.
 * 
 * Construct a generic page/sheet range area.
 **/
void
gnome_print_dialog_construct_range_page (GnomePrintDialog *gpd, gint flags, gint start, gint end,
					 const guchar *currentlabel, const guchar *rangelabel)
{
	GtkWidget *hbox;

	hbox = NULL;

	if (flags & GNOME_PRINT_RANGE_RANGE) {
		GtkWidget *l, *sb;
		GtkObject *a;
		AtkRelationSet *relation_set;
		AtkRelation *relation;
		AtkObject *relation_targets[1];
		AtkObject *atko;

		hbox = gtk_hbox_new (FALSE, 3);
		gtk_widget_show (hbox);

		l = gtk_label_new_with_mnemonic (_("_From:"));
		gtk_widget_show (l);
		gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 0);

		a = gtk_adjustment_new (start, start, end, 1, 10, 10);
		g_object_set_data (G_OBJECT (hbox), "from", a);
		sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1, 0.0);
		gtk_widget_show (sb);
		gtk_box_pack_start (GTK_BOX (hbox), sb, FALSE, FALSE, 0);
		gtk_label_set_mnemonic_widget ((GtkLabel *) l, sb);

		/* Add a LABELLED_BY relation from the spinbutton to the label.
		 */
		atko = gtk_widget_get_accessible (sb);
		atk_object_set_description (atko, _("Sets the start of the range of pages to be printed"));

		relation_set = atk_object_ref_relation_set (atko);
		relation_targets[0] = gtk_widget_get_accessible (l);
		relation = atk_relation_new (relation_targets, 1,
					     ATK_RELATION_LABELLED_BY);
		atk_relation_set_add (relation_set, relation);
		g_object_unref (G_OBJECT (relation));
		g_object_unref (G_OBJECT (relation_set));

		l = gtk_label_new_with_mnemonic (_("_To:"));
		gtk_widget_show (l);
		gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 0);

		a = gtk_adjustment_new (end, start, end, 1, 10, 10);
		g_object_set_data (G_OBJECT (hbox), "to", a);
		sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1, 0.0);
		gtk_widget_show (sb);
		gtk_box_pack_start (GTK_BOX (hbox), sb, FALSE, FALSE, 0);
		gtk_label_set_mnemonic_widget ((GtkLabel *) l, sb);

		/* Add a LABELLED_BY relation from the spinbutton to the label.
		 */
		atko = gtk_widget_get_accessible (sb);
		atk_object_set_description (atko, _("Sets the end of the range of pages to be printed"));

		relation_set = atk_object_ref_relation_set (atko);
		relation_targets[0] = gtk_widget_get_accessible (l);
		relation = atk_relation_new (relation_targets, 1,
					     ATK_RELATION_LABELLED_BY);
		atk_relation_set_add (relation_set, relation);
		g_object_unref (G_OBJECT (relation));
		g_object_unref (G_OBJECT (relation_set));
	}

	gnome_print_dialog_construct_range_any (gpd, flags, hbox, currentlabel, rangelabel);
}

/**
 * gnome_print_dialog_get_range:
 * @gpd: A GnomePrintDialog with a range display.
 * 
 * Return the range option selected by the user.  This is a bitmask
 * with only 1 bit set, out of:
 *
 * GNOME_PRINT_RANGE_CURRENT: The current option selected.
 * GNOME_PRINT_RANGE_ALL: The all option selected.
 * GNOME_PRINT_RANGE_RANGE The range option selected.
 * GNOME_PRINT_RANGE_SELECTION: The selection option selected.
 * 
 * Return value: A bitmask with one option set.
 **/
GnomePrintRangeType
gnome_print_dialog_get_range (GnomePrintDialog *gpd)
{
	GtkWidget *f, *r, *b;

	g_return_val_if_fail (gpd != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_DIALOG (gpd), 0);

	f = g_object_get_data (G_OBJECT (gpd->job), "range");
	g_return_val_if_fail (f != NULL, 0);
	r = g_object_get_data (G_OBJECT (f), "range");
	g_return_val_if_fail (r != NULL, 0);

	b = g_object_get_data (G_OBJECT (r), "current");
	if (b && GTK_IS_TOGGLE_BUTTON (b) && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) 
		return GNOME_PRINT_RANGE_CURRENT;
	
	b = g_object_get_data (G_OBJECT (r), "all");
	if (b && GTK_IS_TOGGLE_BUTTON (b) && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) 
		return GNOME_PRINT_RANGE_ALL;

	b = g_object_get_data (G_OBJECT (r), "range");
	if (b && GTK_IS_TOGGLE_BUTTON (b) && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) 
		return GNOME_PRINT_RANGE_RANGE;
	
	b = g_object_get_data (G_OBJECT (r), "selection");
	if (b && GTK_IS_TOGGLE_BUTTON (b) && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) 
		return GNOME_PRINT_RANGE_SELECTION;

	return 0;
}

/**
 * gnome_print_dialog_get_range_page:
 * @gpd: A GnomePrintDialog with a page range display.
 * @start: Return for the user-specified start page.
 * @end: Return for the user-specified end page.
 * 
 * Retrieves the user choice for range type and range, if the user
 * has requested a range of pages to print.
 * 
 * Return value: A bitmask with the user-selection set.  See
 * gnome_print_dialog_get_range().
 **/
gint
gnome_print_dialog_get_range_page (GnomePrintDialog *gpd, gint *start, gint *end)
{
	gint mask;

	g_return_val_if_fail (gpd != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_DIALOG(gpd), 0);

	mask = gnome_print_dialog_get_range (gpd);

	if (mask & GNOME_PRINT_RANGE_RANGE) {
		GtkObject *f, *r, *w, *a;
		f = g_object_get_data (G_OBJECT (gpd->job), "range");
		g_return_val_if_fail (f != NULL, 0);
		r = g_object_get_data (G_OBJECT (f), "range");
		g_return_val_if_fail (r != NULL, 0);
		w = g_object_get_data (G_OBJECT (r), "range-widget");
		g_return_val_if_fail (w != NULL, 0);
		a = g_object_get_data (G_OBJECT (w), "from");
		g_return_val_if_fail (a && GTK_IS_ADJUSTMENT (a), 0);
		if (start)
			*start = (gint) gtk_adjustment_get_value (GTK_ADJUSTMENT (a));
		a = g_object_get_data (G_OBJECT (w), "to");
		g_return_val_if_fail (a && GTK_IS_ADJUSTMENT (a), 0);
		if (end)
			*end = (gint) gtk_adjustment_get_value (GTK_ADJUSTMENT (a));
	}

	return mask;
}

/**
 * gnome_print_dialog_get_copies:
 * @gpd: A GnomePrintDialog with a copies display.
 * @copies: Return for the number of copies.
 * @collate: Return for collation flag.
 * 
 * Retrieves the number of copies and collation indicator from
 * the print dialogue.  If the print dialogue does not have a
 * copies indicator, then a default of 1 copy is returned.
 **/
void
gnome_print_dialog_get_copies (GnomePrintDialog *gpd, gint *copies, gint *collate)
{
	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (gpd));

	if (copies) *copies = 1;
	if (collate) *collate = FALSE;

	if (gpd->job) {
		GnomePrintCopiesSelector *c;

		c = g_object_get_data (G_OBJECT (gpd->job), "copies");
		if (c && GNOME_IS_PRINT_COPIES_SELECTOR (c)) {
			if (copies)
				*copies = gnome_print_copies_selector_get_copies (c);
			if (collate)
				*collate = gnome_print_copies_selector_get_collate (c);
		}
	}
}

/**
 * gnome_print_dialog_set_copies:
 * @gpd: A GnomePrintDialog with a copies display.
 * @copies: New number of copies.
 * @collate: New collation status.
 * 
 * Sets the print copies and collation status in the print dialogue.
 **/
void
gnome_print_dialog_set_copies (GnomePrintDialog *gpd, gint copies, gint collate)
{
	GnomePrintCopiesSelector *c;

	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (gpd));
	g_return_if_fail (gpd->job != NULL);
	c = g_object_get_data (G_OBJECT (gpd->job), "copies");
	g_return_if_fail (c && GNOME_IS_PRINT_COPIES_SELECTOR (c));

	gnome_print_copies_selector_set_copies (c, copies, collate);
}

/**
 * gnome_print_dialog_get_printer:
 * @gpd: An initialised GnomePrintDialog.
 * 
 * Retrieve the user-requested printer from the printer area of
 * the print dialogue.
 * 
 * Return value: The user-selected printer.
 **/
GnomePrintConfig *
gnome_print_dialog_get_config (GnomePrintDialog *gpd)
{
	g_return_val_if_fail (gpd != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_DIALOG (gpd), NULL);

	return gnome_print_config_ref (gpd->config);
}



