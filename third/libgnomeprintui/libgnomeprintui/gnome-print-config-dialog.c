/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-config-dialog.h: A dialog to configure specific 
 *  printer settings.
 *
 *  NOTE: This interface is considered private and should not be used by 
 *  applications directly!
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
 *      Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 *  Copyright (C) 2003  Andreas J. Guelzow
 *
 */

#define GNOME_PRINT_UNSTABLE_API

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
#include "gnome-print-config-dialog.h"
#include "gnome-print-config-dialog-private.h"
#include "gpaui/gpa-option-menu.h"

#define PAD 6

enum {
	PROP_0,
	PROP_PRINT_CONFIG
};

static void gnome_print_config_dialog_class_init (GnomePrintConfigDialogClass *class);
static void gnome_print_config_dialog_init (GnomePrintConfigDialog *dialog);

static GtkDialogClass *parent_class;

GType
gnome_print_config_dialog_get_type (void)
{	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintConfigDialogClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_config_dialog_class_init,
			NULL, NULL,
			sizeof (GnomePrintConfigDialog),
			0,
			(GInstanceInitFunc) gnome_print_config_dialog_init,
			NULL,
		};
		type = g_type_register_static (GTK_TYPE_DIALOG, "GnomePrintConfigDialog", &info, 0);
	}
	return type;
}

static void
gnome_print_config_dialog_destroy (GtkObject *object)
{
	GnomePrintConfigDialog *gpd;
	
	gpd = GNOME_PRINT_CONFIG_DIALOG (object);

	if (gpd->config) {
		gpd->config = gnome_print_config_unref (gpd->config);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnome_print_config_dialog_set_property (GObject      *object,
				 guint         prop_id,
				 GValue const *value,
				 GParamSpec   *pspec)
{
	gpointer new_config;
	GnomePrintConfigDialog *gpd = GNOME_PRINT_CONFIG_DIALOG (object);

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
gnome_print_config_dialog_class_init (GnomePrintConfigDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (GTK_TYPE_DIALOG);

	object_class->destroy = gnome_print_config_dialog_destroy;

	G_OBJECT_CLASS (class)->set_property = gnome_print_config_dialog_set_property;
	g_object_class_install_property (G_OBJECT_CLASS (class),
					 PROP_PRINT_CONFIG,
					 g_param_spec_pointer ("print_config",
							       "Print Config",
							       "Printing Configuration to be used",
							       G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));
}

static void
gnome_print_config_dialog_init (GnomePrintConfigDialog *gpd)
{
	/* Empty */
}

static void
gp_config_dialog_read_from_config (GnomePrintConfigDialog *gpd)
{
	gboolean tumble = FALSE;
	gboolean duplex = FALSE;

	if (gpd->config == NULL)
		return;

	gnome_print_config_get_boolean (gpd->config, 
					GNOME_PRINT_KEY_DUPLEX, &duplex);
	gnome_print_config_get_boolean (gpd->config, 
					GNOME_PRINT_KEY_TUMBLE, &tumble);

	gtk_toggle_button_set_active ((GtkToggleButton *) gpd->duplex, duplex);
	gtk_toggle_button_set_active ((GtkToggleButton *) gpd->tumble, tumble);	
}


static void
duplex_toggled (GtkWidget *widget, GnomePrintConfigDialog *gpd)
{
	gboolean duplex = ((GtkToggleButton *) gpd->duplex)->active;
	GdkPixbuf *pb = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		(duplex ? "stock_print-duplex" : "stock_print-non-duplex"),
		48, 0, NULL);
	if (NULL != pb) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (gpd->duplex_image), pb);
		g_object_unref (G_OBJECT (pb));
	}

	gtk_widget_set_sensitive (gpd->tumble, duplex);
	gtk_widget_set_sensitive (gpd->tumble_image, duplex);

	if (widget != NULL && gpd->config)
		gnome_print_config_set_boolean (gpd->config, 
						GNOME_PRINT_KEY_DUPLEX, 
						duplex);
}

static void
tumble_toggled (GtkWidget *widget, GnomePrintConfigDialog *gpd)
{
	gboolean tumble = ((GtkToggleButton *) gpd->tumble)->active;
	GdkPixbuf *pb = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		(tumble ? "stock_print-duplex-tumble" : "stock_print-duplex-no-tumble"),
		48, 0, NULL);
	if (NULL != pb) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (gpd->tumble_image), pb);
		g_object_unref (G_OBJECT (pb));
	}

	if (widget != NULL && gpd->config) 
		gnome_print_config_set_boolean (gpd->config, 
						GNOME_PRINT_KEY_TUMBLE, 
						tumble);
}

/**
 * gnome_print_config_dialog_new:
 *
 * Create a new gnome-print-config-dialog window.
 *
 * 
 * Return value: A newly created and initialised widget.
 **/
GtkWidget *
gnome_print_config_dialog_new (GnomePrintConfig *gpc)
{
	GnomePrintConfigDialog *gpd;

	gpd = GNOME_PRINT_CONFIG_DIALOG (g_object_new (GNOME_TYPE_PRINT_CONFIG_DIALOG, NULL));

	if (gpd) {
		if (gpc == NULL)
			gpc = gnome_print_config_default ();
		else
			gnome_print_config_ref (gpc);
		gpd->config = gpc;
		gnome_print_config_dialog_construct (gpd);
	}

	return GTK_WIDGET (gpd);
}

/**
 * gnome_print_config_dialog_construct:
 * @gpd: A created GnomePrintConfigDialog.
 * 
 * Used for language bindings to post-initialise an object instantiation.
 *
 */
void
gnome_print_config_dialog_construct (GnomePrintConfigDialog *gpd)
{
	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONFIG_DIALOG (gpd));

	gtk_window_set_title (GTK_WINDOW (gpd), 
			      (const guchar *) _("Default Settings"));

	if (gpd->config) {
		GtkWidget *table;
		AtkObject *atko;
		guchar  *title;

		title = gnome_print_config_get (gpd->config, "Printer");
		if (title) {
			gtk_window_set_title (GTK_WINDOW (gpd), 
					      (const gchar *) title);
			g_free (title);
		}
		table = gtk_table_new(2, 2, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (table), PAD);
		gtk_table_set_col_spacings (GTK_TABLE (table), PAD);
		gtk_container_set_border_width  (GTK_CONTAINER (table), PAD);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (gpd)->vbox), 
				    table, TRUE, TRUE, 0);

		gpd->duplex_image = gtk_image_new ();
		gtk_widget_show (gpd->duplex_image);
		gtk_table_attach_defaults ((GtkTable *)table, 
					   gpd->duplex_image, 0, 1, 0, 1);
		atko = gtk_widget_get_accessible (gpd->duplex_image);
		atk_image_set_image_description (ATK_IMAGE (atko), 
						 _("Image showing pages "
						   "being printed in "
						   "duplex."));
		
		gpd->duplex = gtk_check_button_new_with_mnemonic (_("_Duplex"));
		gtk_widget_show(gpd->duplex);
		gtk_table_attach_defaults((GtkTable *)table, gpd->duplex, 1, 2, 0, 1);
		
		atko = gtk_widget_get_accessible (gpd->duplex);
		atk_object_set_description (atko, 
					    _("Pages are printed in duplex."));
		
		gpd->tumble_image = gtk_image_new ();
		gtk_widget_show (gpd->tumble_image);
		gtk_table_attach_defaults ((GtkTable *)table, 
					   gpd->tumble_image, 0, 1, 1, 2);
		atko = gtk_widget_get_accessible (gpd->tumble_image);
		atk_image_set_image_description (ATK_IMAGE (atko), 
						 _("Image showing the second "
						   "page of a duplex printed "
						   "sequence to be printed "
						   "upside down."));
		
		gpd->tumble = gtk_check_button_new_with_mnemonic (_("_Tumble"));
		gtk_widget_show(gpd->tumble);
		gtk_table_attach_defaults((GtkTable *)table, gpd->tumble, 1, 2, 1, 2);
		
		atko = gtk_widget_get_accessible (gpd->tumble);
		atk_object_set_description (atko, 
					    _("If copies of the document are "
					      "printed in duplex, the second "
					      "page is flipped upside down,"));
		g_signal_connect (G_OBJECT (gpd->duplex), "toggled",
				  (GCallback) duplex_toggled, gpd);
		g_signal_connect (G_OBJECT (gpd->tumble), "toggled",
				  (GCallback) tumble_toggled, gpd);

		gp_config_dialog_read_from_config (gpd);

		tumble_toggled (NULL, gpd);
		duplex_toggled (NULL, gpd);

		/* Print Time */
		{
			GtkWidget *l, *menu;
			AtkRelationSet *relation_set;
			AtkRelation *relation;
			AtkObject *relation_targets[1];

			l = gtk_label_new_with_mnemonic (_("_Printing Time:"));
			gtk_widget_show(l);
			gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
			gtk_table_attach_defaults (GTK_TABLE (table), l, 
						   0, 1, 2, 3);
			
			menu = gpa_option_menu_new (gpd->config, 
						    GNOME_PRINT_KEY_HOLD);
			gtk_widget_show(menu);
			gtk_table_attach_defaults (GTK_TABLE (table), menu, 
						   1, 2, 2, 3);
			gtk_label_set_mnemonic_widget ((GtkLabel *) l, menu);

			atko = gtk_widget_get_accessible (menu);
			relation_set = atk_object_ref_relation_set (atko);
			relation_targets[0] = gtk_widget_get_accessible (l);
			relation = atk_relation_new (relation_targets, 1,
						     ATK_RELATION_LABELLED_BY);
			atk_relation_set_add (relation_set, relation);
			g_object_unref (G_OBJECT (relation));
			g_object_unref (G_OBJECT (relation_set));
		}

		gtk_widget_show(table);
	} else {
		GtkWidget *label;
		label = gtk_label_new (_("Error while loading printer configuration"));
		gtk_widget_show (label);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (gpd)->vbox), label, TRUE, TRUE, 0);
	}


	gtk_dialog_add_buttons (GTK_DIALOG (gpd),
				GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				NULL);


	gtk_dialog_set_default_response (GTK_DIALOG (gpd), GTK_RESPONSE_CLOSE);
}



