/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-printer-dialog.c:
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
 *    Raph Levien (raph@acm.org)
 *    Miguel de Icaza (miguel@kernel.org)
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio (chema@celorio.com)
 *
 *  Copyright (C) 1999-2002 Ximian Inc. and authors
 *
 */

#define __GNOME_PRINTER_DIALOG_C__

#include <config.h>

#include <atk/atkobject.h>
#include <atk/atkrelationset.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>
#include "gnome-print-i18n.h"
#include "gpaui/printer-selector.h"
#include "gpaui/settings-selector.h"
#include "gpaui/transport-selector.h"
#include "gnome-printer-dialog.h"

#define GPS_PAD 4

struct _GnomePrinterSelection {
	GPAWidget gpawidget;
	GtkAccelGroup *accel_group;
	GtkWidget *printers; /* GPAPrinterSelector */
	GtkWidget *settings; /* GPASettingsSelector */
	GtkWidget *transport; /* GPATransportSelector */
	GtkWidget *state, *type, *location, *comment;
};

struct _GnomePrinterSelectionClass {
	GPAWidgetClass gpa_widget_class;
};

static void gnome_printer_selection_class_init (GnomePrinterSelectionClass *klass);
static void gnome_printer_selection_init (GtkObject *object);

static void gnome_printer_selection_destroy (GtkObject *object);

static GtkWidget *gpw_create_label (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *label_for);
static GtkWidget *gpw_create_label_with_mnemonic (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *mnemonic_widget, gboolean add_relation);

static void gpw_configure_clicked (GtkWidget *widget, GPAWidget *gpaw);

static GPAWidgetClass *gpw_parent_class;

GtkType
gnome_printer_selection_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"GnomePrinterSelection",
			sizeof (GnomePrinterSelection),
			sizeof (GnomePrinterSelectionClass),
			(GtkClassInitFunc) gnome_printer_selection_class_init,
			(GtkObjectInitFunc) gnome_printer_selection_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GPA_TYPE_WIDGET, &info);
	}
	return type;
}

static void
gnome_printer_selection_class_init (GnomePrinterSelectionClass *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) klass;
	
	gpw_parent_class = gtk_type_class (GPA_TYPE_WIDGET);
	
	object_class->destroy  = gnome_printer_selection_destroy;
}

static void
gnome_printer_selection_init (GtkObject *object)
{
	GnomePrinterSelection *gpw;
	GtkWidget *f, *t, *b, *l;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[2];
	AtkObject *atko, *label_atko, *menu_atko, *entry_atko;

	gpw = GNOME_PRINTER_SELECTION (object);

	gpw->accel_group = gtk_accel_group_new ();

	f = gtk_frame_new (_("Printer"));
	gtk_widget_show (f);
	gtk_container_add (GTK_CONTAINER (gpw), f);

	t = gtk_table_new (3, 6, FALSE);
	gtk_widget_show (t);
	gtk_container_add (GTK_CONTAINER (f), t);

	gpw->type = gpw_create_label (GTK_TABLE (t), 1, 2, 3, 4, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 3, 4, _("State:"), gpw->type);

	gpw->location = gpw_create_label (GTK_TABLE (t), 1, 2, 4, 5, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 4, 5, _("Type:"), gpw->location);

	gpw->comment = gpw_create_label (GTK_TABLE (t), 1, 2, 5, 6, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 5, 6, _("Comment:"), gpw->comment);

	b = gtk_button_new_with_mnemonic (_("Co_nfigure"));
	gtk_widget_show (b);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (gpw_configure_clicked), gpw);
	gtk_table_attach (GTK_TABLE (t), b, 2, 3, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  GPS_PAD, GPS_PAD);
	atko = gtk_widget_get_accessible (b);
	atk_object_set_description (atko, _("Adjust the settings of the selected printer"));

	/* FIXME: remove the following line when gpw_configure_clicked will be implemented */
	gtk_widget_set_sensitive (b, FALSE);

	gpw->printers = gpa_widget_new (GPA_TYPE_PRINTER_SELECTOR, NULL);
	gtk_widget_show (gpw->printers);
	gtk_table_attach (GTK_TABLE (t), gpw->printers, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  GPS_PAD, GPS_PAD);
	gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 0, 1,
					_("Pr_inter:"),
					((GPAPrinterSelector*) gpw->printers)->menu, TRUE);

	gpw->settings = gpa_widget_new (GPA_TYPE_SETTINGS_SELECTOR, NULL);
	gtk_widget_show (gpw->settings);
	gtk_table_attach (GTK_TABLE (t), gpw->settings, 1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  GPS_PAD, GPS_PAD);
	l = gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 1, 2,
					    _("_Settings:"),
					    ((GPASettingsSelector*) gpw->settings)->menu, TRUE);

	gpw->transport = gpa_widget_new (GPA_TYPE_TRANSPORT_SELECTOR, NULL);
	gtk_widget_show (gpw->transport);
	gtk_table_attach (GTK_TABLE (t), gpw->transport, 1, 2, 2, 3,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  GPS_PAD, GPS_PAD);
	gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 2, 3,
					_("_Location:"),
					((GPATransportSelector*) gpw->transport)->menu, FALSE);

	/* Add a LABEL_FOR relation from the Location label to the option menu
	   and the entry. */
	label_atko = gtk_widget_get_accessible (l);
	menu_atko = gtk_widget_get_accessible (((GPATransportSelector*) gpw->transport)->menu);
	entry_atko = gtk_widget_get_accessible (((GPATransportSelector*) gpw->transport)->fentry);

	relation_set = atk_object_ref_relation_set (label_atko);
	relation_targets[0] = menu_atko;
	relation_targets[1] = entry_atko;
	relation = atk_relation_new (relation_targets, 2,
				     ATK_RELATION_LABEL_FOR);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));

	/* Add LABELLED_BY relations the other way round. */
	relation_set = atk_object_ref_relation_set (menu_atko);
	relation_targets[0] = label_atko;
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));

	relation_set = atk_object_ref_relation_set (entry_atko);
	relation_targets[0] = label_atko;
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));
}

static void
gnome_printer_selection_destroy (GtkObject *object)
{
	GnomePrinterSelection *gpw;
	
	gpw = GNOME_PRINTER_SELECTION (object);

	if (gpw->accel_group) {
		gtk_accel_group_unref (gpw->accel_group);
		gpw->accel_group = NULL;
	}

	if (((GtkObjectClass *) gpw_parent_class)->destroy)
		(* ((GtkObjectClass *) gpw_parent_class)->destroy) (object);
}

GtkWidget *
gnome_printer_selection_new_default (void)
{
	GtkWidget *gpw;
	GnomePrintConfig *config;

	config = gnome_print_config_default ();
	gpw = gnome_printer_selection_new (config);
	gnome_print_config_unref (config);

	return gpw;
}

GtkWidget *
gnome_printer_selection_new (GnomePrintConfig *config)
{
	GnomePrinterSelection *gpw;

	g_return_val_if_fail (config != NULL, NULL);

	gpw = gtk_type_new (GNOME_TYPE_PRINTER_SELECTION);
	gpa_widget_construct (GPA_WIDGET (gpw), config);

	gpa_widget_construct (GPA_WIDGET (gpw->printers), config);
	gpa_widget_construct (GPA_WIDGET (gpw->settings), config);
	gpa_widget_construct (GPA_WIDGET (gpw->transport), config);

	return GTK_WIDGET (gpw);
}

GnomePrintConfig *
gnome_printer_selection_get_config (GnomePrinterSelection *widget)
{
	GPAWidget *gpaw;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER_SELECTION (widget), NULL);

	gpaw = GPA_WIDGET (widget);

	if (gpaw->config) gnome_print_config_ref (gpaw->config);

	return gpaw->config;
}

static GtkWidget *
gpw_create_label (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *label_for)
{
	GtkWidget *w;

	w = gtk_label_new (text);
	gtk_widget_show (w);
	gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
	gtk_table_attach (table, w, l, r, t, b, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, GPS_PAD, GPS_PAD);

	if (label_for) {
		AtkRelationSet *relation_set;
		AtkRelation *relation;
		AtkObject *relation_targets[1];
		AtkObject *atko;

		/* Add a LABEL_FOR relation from the label to the label_for
		   widget. */
		atko = gtk_widget_get_accessible (w);
		relation_set = atk_object_ref_relation_set (atko);
		relation_targets[0] = gtk_widget_get_accessible (label_for);
		relation = atk_relation_new (relation_targets, 1,
					     ATK_RELATION_LABEL_FOR);
		atk_relation_set_add (relation_set, relation);
		g_object_unref (G_OBJECT (relation));
		g_object_unref (G_OBJECT (relation_set));

		/* Add a LABELLED_BY relation from the mnemonic widget to the
		   label. */
		atko = gtk_widget_get_accessible (label_for);
		relation_set = atk_object_ref_relation_set (atko);
		relation_targets[0] = gtk_widget_get_accessible (w);
		relation = atk_relation_new (relation_targets, 1,
					     ATK_RELATION_LABELLED_BY);
		atk_relation_set_add (relation_set, relation);
		g_object_unref (G_OBJECT (relation));
		g_object_unref (G_OBJECT (relation_set));
	}

	return w;
}

static GtkWidget *
gpw_create_label_with_mnemonic (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *mnemonic_widget, gboolean add_relation)
{
	GtkWidget *w;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];
	AtkObject *atko;

	w = gtk_label_new_with_mnemonic (text);
	gtk_widget_show (w);
	gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
	gtk_table_attach (table, w, l, r, t, b, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, GPS_PAD, GPS_PAD);
	gtk_label_set_mnemonic_widget ((GtkLabel *) w, mnemonic_widget);

	if (add_relation) {
		/* Add a LABELLED_BY relation from the mnemonic widget to the
		   label. */
		atko = gtk_widget_get_accessible (mnemonic_widget);
		relation_set = atk_object_ref_relation_set (atko);
		relation_targets[0] = gtk_widget_get_accessible (w);
		relation = atk_relation_new (relation_targets, 1,
					     ATK_RELATION_LABELLED_BY);
		atk_relation_set_add (relation_set, relation);
		g_object_unref (G_OBJECT (relation));
		g_object_unref (G_OBJECT (relation_set));
	}

	return w;
}

static void
gpw_configure_clicked (GtkWidget *widget, GPAWidget *gpaw)
{
}

struct _GnomePrinterDialog {
	GtkDialog dialog;
	GnomePrinterSelection *gnome_printer_selection;
};

struct _GnomePrinterDialogClass {
	GtkDialogClass parent_class;
};

static GtkDialogClass *dialog_parent_class = NULL;

static void
gnome_printer_dialog_destroy (GtkObject *object)
{
	GnomePrinterDialog *printer_dialog;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINTER_DIALOG (object));
	
	printer_dialog = GNOME_PRINTER_DIALOG (object);
	
	(* GTK_OBJECT_CLASS (dialog_parent_class)->destroy) (object);
}

static void
gnome_printer_dialog_class_init (GnomePrinterDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) class;
	
	dialog_parent_class = gtk_type_class (GTK_TYPE_DIALOG);
	
	object_class->destroy = gnome_printer_dialog_destroy;
}

GtkType
gnome_printer_dialog_get_type (void)
{
	static GtkType printer_dialog_type = 0;
	
	if (!printer_dialog_type)
	{
		GtkTypeInfo printer_dialog_info =
		{
			"GnomePrinterDialog",
			sizeof (GnomePrinterDialog),
			sizeof (GnomePrinterDialogClass),
			(GtkClassInitFunc) gnome_printer_dialog_class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		printer_dialog_type = gtk_type_unique (GTK_TYPE_DIALOG, &printer_dialog_info);
	}
	
	return printer_dialog_type;
}

GtkWidget *
gnome_printer_dialog_new_default (void)
{
	GtkWidget *gpw;
	GnomePrintConfig *config;

	config = gnome_print_config_default ();
	gpw = gnome_printer_dialog_new (config);
	gnome_print_config_unref (config);

	return gpw;
}

/**
 * gnome_printer_dialog_new:
 *
 * Creates a dialog box for selecting a printer.
 * This returns a GnomePrinterDialog object, the programmer
 * is resposible for querying the gnome_printer_dialog
 * to fetch the selected GnomePrinter object
 *
 * Returns: the GnomeDialog, ready to be ran.
 */
GtkWidget *
gnome_printer_dialog_new (GnomePrintConfig *config)
{
	GtkWidget *printer_dialog;
	GnomePrinterDialog *pd;
	GnomePrinterSelection *gpw;
	
	pd = gtk_type_new (gnome_printer_dialog_get_type ());
	printer_dialog = GTK_WIDGET (pd);
	
	gtk_window_set_title (GTK_WINDOW (printer_dialog), _("Select Printer"));
	
	gtk_dialog_add_button (GTK_DIALOG (printer_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
	
	gtk_dialog_add_button (GTK_DIALOG(printer_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_OK);
	
	gtk_dialog_set_default_response (GTK_DIALOG (printer_dialog), GTK_RESPONSE_OK);
	/* We have a frame around the widgets, so we don't need a separator. */
	gtk_dialog_set_has_separator (GTK_DIALOG (printer_dialog), FALSE);
	
	pd->gnome_printer_selection = GNOME_PRINTER_SELECTION (gnome_printer_selection_new (config));
	if (pd->gnome_printer_selection == NULL) return NULL;
	gtk_widget_show (GTK_WIDGET (pd->gnome_printer_selection));

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (printer_dialog)->vbox),
			    GTK_WIDGET (pd->gnome_printer_selection), TRUE, TRUE, 0);
	gpw = pd->gnome_printer_selection;

	gtk_window_add_accel_group (GTK_WINDOW (pd), gpw->accel_group);
	
	return printer_dialog;
}

/**
 * gnome_printer_dialog_get_printer:
 * @dialog: a GnomePrinterDialog
 *
 * Returns: the GnomePrinter associated with the @dialog GnomePrinterDialog
 */
GnomePrintConfig *
gnome_printer_dialog_get_config (GnomePrinterDialog *dialog)
{
	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER_DIALOG (dialog), NULL);

	return gnome_printer_selection_get_config (GNOME_PRINTER_SELECTION (dialog->gnome_printer_selection));
}
