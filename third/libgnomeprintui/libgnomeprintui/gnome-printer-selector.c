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
 *    Raph Levien <raph@acm.org>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 1999-2003 Ximian Inc. and authors
 *
 */

#include <config.h>

#include <atk/atk.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gnome-printer-selector.h"

#include "gpaui/gpa-printer-selector.h"
#include "gpaui/gpa-settings-selector.h"
#include "gpaui/gpa-transport-selector.h"

#define GPS_PAD 4

static void gnome_printer_selector_class_init (GnomePrinterSelectorClass *klass);
static void gnome_printer_selector_init (GObject *object);
static gint gnome_printer_selector_construct (GPAWidget *gpa_widget);

static void gnome_printer_selector_finalize (GObject *object);

static GtkWidget *gpw_create_label (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *label_for);
static GtkWidget *gpw_create_label_with_mnemonic (GtkTable *table, gint l, gint r, gint t, gint b, const gchar *text, GtkWidget *mnemonic_widget, gboolean add_relation);

static void gpw_configure_clicked (GtkWidget *widget, GPAWidget *gpaw);

static GPAWidgetClass *parent_class;

GType
gnome_printer_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrinterSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_printer_selector_class_init,
			NULL, NULL,
			sizeof (GnomePrinterSelector),
			0,
			(GInstanceInitFunc) gnome_printer_selector_init
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GnomePrinterSelector", &info, 0);
	}
	return type;
}

static void
gnome_printer_selector_class_init (GnomePrinterSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;
	
	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;
	
	parent_class = gtk_type_class (GPA_TYPE_WIDGET);
	gpa_class->construct = gnome_printer_selector_construct;
	object_class->finalize  = gnome_printer_selector_finalize;
}

static void
gnome_printer_selector_init (GObject *object)
{
}

static gint
gnome_printer_selector_construct (GPAWidget *gpa_widget)
{
	GnomePrinterSelector *gpw;
	GtkWidget *f, *t, *b, *l, *h, *v;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[2];
	AtkObject *atko, *label_atko, *menu_atko, *entry_atko;

	gpw = GNOME_PRINTER_SELECTOR (gpa_widget);

	gpw->accel_group = gtk_accel_group_new ();

	f = gtk_frame_new (_("Printer"));
	v = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (v), f, 0, 0, 0);
	gtk_container_add (GTK_CONTAINER (gpw), v);
	gtk_widget_show (f);
	gtk_widget_show (v);

	t = gtk_table_new (3, 6, FALSE);
	h = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (t);
	gtk_widget_show (h);
	gtk_box_pack_start (GTK_BOX (h), t, FALSE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (f), h);

	gpw->type = gpw_create_label (GTK_TABLE (t), 1, 2, 3, 4, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 3, 4, _("State:"), gpw->type);

	gpw->location = gpw_create_label (GTK_TABLE (t), 1, 2, 4, 5, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 4, 5, _("Type:"), gpw->location);

	gpw->comment = gpw_create_label (GTK_TABLE (t), 1, 2, 5, 6, "", NULL);
	gpw_create_label (GTK_TABLE (t), 0, 1, 5, 6, _("Comment:"), gpw->comment);

	b = gtk_button_new_with_mnemonic (_("Co_nfigure"));
	gtk_widget_show (b);
	g_signal_connect (G_OBJECT (b), "clicked",
			  (GCallback) gpw_configure_clicked, gpw);
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
			  GTK_FILL, 0,
			  GPS_PAD, GPS_PAD);
	gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 0, 1,
					_("Pr_inter:"),
					((GPAPrinterSelector*) gpw->printers)->menu, TRUE);

	gpw->settings = gpa_widget_new (GPA_TYPE_SETTINGS_SELECTOR, NULL);
	gtk_widget_show (gpw->settings);
	gtk_table_attach (GTK_TABLE (t), gpw->settings, 1, 2, 1, 2,
			  GTK_FILL, 0,
			  GPS_PAD, GPS_PAD);
	l = gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 1, 2,
					    _("_Settings:"),
					    ((GPASettingsSelector*) gpw->settings)->menu, TRUE);

	gpw->transport = gpa_widget_new (GPA_TYPE_TRANSPORT_SELECTOR, NULL);
	gtk_widget_show (gpw->transport);
	gtk_table_attach (GTK_TABLE (t), gpw->transport, 1, 3, 2, 3,
			  GTK_FILL , 0,
			  GPS_PAD, GPS_PAD);
	gpw_create_label_with_mnemonic (GTK_TABLE (t), 0, 1, 2, 3,
					_("_Location:"),
					((GPATransportSelector*) gpw->transport)->menu, FALSE);
	/* Add a LABEL_FOR relation from the Location label to the option menu
	   and the entry. */
	label_atko = gtk_widget_get_accessible (l);
	menu_atko  = gtk_widget_get_accessible (((GPATransportSelector*) gpw->transport)->menu);
	entry_atko = gtk_widget_get_accessible (((GPATransportSelector*) gpw->transport)->file_entry);

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

	return TRUE;
}

static void
gnome_printer_selector_finalize (GObject *object)
{
	GnomePrinterSelector *gpw;
	
	gpw = GNOME_PRINTER_SELECTOR (object);

	if (gpw->accel_group) {
		g_object_unref (G_OBJECT (gpw->accel_group));
		gpw->accel_group = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gnome_printer_selector_new_default (void)
{
	GtkWidget *gpw;
	GnomePrintConfig *config;

	config = gnome_print_config_default ();
	gpw = gnome_printer_selector_new (config);
	gnome_print_config_unref (config);

	return gpw;
}

GtkWidget *
gnome_printer_selector_new (GnomePrintConfig *config)
{
	GnomePrinterSelector *gpw;

	g_return_val_if_fail (config != NULL, NULL);

	gpw = (GnomePrinterSelector *) gpa_widget_new (GNOME_TYPE_PRINTER_SELECTOR, config);

	gpa_widget_construct (GPA_WIDGET (gpw->printers), config);
	gpa_widget_construct (GPA_WIDGET (gpw->settings), config);
	gpa_widget_construct (GPA_WIDGET (gpw->transport), config);

	return GTK_WIDGET (gpw);
}

GnomePrintConfig *
gnome_printer_selector_get_config (GnomePrinterSelector *widget)
{
	GPAWidget *gpaw;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER_SELECTOR (widget), NULL);

	gpaw = GPA_WIDGET (widget);

	if (gpaw->config)
		gnome_print_config_ref (gpaw->config);

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
