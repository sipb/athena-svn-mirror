/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * e-categories-master-list-dialog.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>

#include "e-categories-master-list-dialog.h"

#include <gal/util/e-i18n.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkdialog.h>
#include <glade/glade.h>
#include <gal/util/e-util.h>
#include <gal/e-table/e-table-scrolled.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-table-model.h>

#include <gal/widgets/e-categories-master-list-dialog-model.h>

#include <stdlib.h>
#include <string.h>

struct ECategoriesMasterListDialogPriv {
	ECategoriesMasterList *ecml;

	GladeXML *gui;
};

#define PARENT_TYPE GTK_TYPE_OBJECT

static GtkObjectClass *parent_class;

/* The arguments we take */
enum {
	PROP_0,
	PROP_ECML
};

static void
ecmld_dispose (GObject *object)
{
	ECategoriesMasterListDialog *ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (object);

	if (ecmld->priv) {
		if (ecmld->priv->ecml)
			g_object_unref (ecmld->priv->ecml);
		if (ecmld->priv->gui)
			g_object_unref (ecmld->priv->gui);
		g_free (ecmld->priv);
		ecmld->priv = NULL;
	}
	
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
ecmld_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	ECategoriesMasterListDialog *ecmld;
	GtkWidget *scrolled;

	ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (object);
	
	switch (prop_id){
	case PROP_ECML:
		if (ecmld->priv->ecml) {
			g_object_unref (ecmld->priv->ecml);
		}

		ecmld->priv->ecml = (ECategoriesMasterList *) g_value_get_object (value);

		if (ecmld->priv->ecml)
			g_object_ref (ecmld->priv->ecml);

		scrolled = glade_xml_get_widget (ecmld->priv->gui, "custom-etable");
		if (scrolled && E_IS_TABLE_SCROLLED (scrolled)) {
			ETable *table;
			ETableModel *model;

			table = e_table_scrolled_get_table (E_TABLE_SCROLLED(scrolled));
			g_object_get (table,
				      "model", &model,
				      NULL);
			g_object_set (model,
				      "ecml", ecmld->priv->ecml,
				      NULL);
		}
		break;
	}
}

static void
ecmld_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	ECategoriesMasterListDialog *ecmld;

	ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (object);

	switch (prop_id) {
	case PROP_ECML:
		g_value_set_object (value, ecmld->priv->ecml);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ecmld_class_init (GObjectClass *object_class)
{
	parent_class   = g_type_class_ref (PARENT_TYPE);

	object_class->dispose = ecmld_dispose;
	object_class->set_property = ecmld_set_property;
	object_class->get_property = ecmld_get_property;

	g_object_class_install_property (object_class, PROP_ECML,
					 g_param_spec_object ("ecml",
							      _( "ECML" ),
							      _( "ECategoriesMasterListCombo" ),
							      E_CATEGORIES_MASTER_LIST_TYPE,
							      G_PARAM_READWRITE));

	glade_init ();
}

static void
dialog_destroyed (gpointer data, GObject *where_object_was)
{
	ECategoriesMasterListDialog *ecmld = data;
	gtk_object_destroy (ecmld);
}

static void
dialog_response (GtkObject *dialog, int id, ECategoriesMasterListDialog *ecmld)
{
	GtkWidget *scrolled;
	switch (id) {
	case 0:    /* Remove */
		scrolled = glade_xml_get_widget (ecmld->priv->gui, "custom-etable");
		if (scrolled && E_IS_TABLE_SCROLLED (scrolled)) {
			ETable *table;
			int row;

			table = e_table_scrolled_get_table (E_TABLE_SCROLLED(scrolled));

			row = e_table_get_cursor_row(table);
			if (row != -1) {
				e_categories_master_list_delete (ecmld->priv->ecml, row);
				e_categories_master_list_commit (ecmld->priv->ecml);
			}
		}
		break;
	case GTK_RESPONSE_CLOSE:
		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	}
}

#if 0
static void
connect_button (ECategoriesMasterListDialog *ecmld, GladeXML *gui, const char *widget_name, void *cback)
{
	GtkWidget *button = glade_xml_get_widget (gui, widget_name);

	if (button && GTK_IS_BUTTON (button))
		g_signal_connect(button, "clicked",
				 G_CALLBACK (cback), ecmld);
}
#endif

#ifdef JUST_FOR_TRANSLATORS
static char *list [] = {
	N_("* Click here to add a category *"),
	N_("Category"),
};
#endif

#define SPEC "<ETableSpecification cursor-mode=\"line\" draw-grid=\"false\" draw-focus=\"true\" selection-mode=\"browse\" no-headers=\"true\""		\
             " gettext-domain=\"" E_I18N_DOMAIN "\" click-to-add=\"true\" _click-to-add-message=\"* Click here to add a category *\">"			\
		"<ETableColumn model_col= \"0\" _title=\"Category\" expansion=\"1.0\" resizable=\"true\" minimum_width=\"24\" cell=\"string\" compare=\"string\"/>" \
			"<ETableState> <column source=\"0\"/>"												\
				"<grouping> <leaf column=\"0\" ascending=\"true\"/> </grouping>"							\
			"</ETableState>"														\
	     "</ETableSpecification>"

GtkWidget *create_ecmld_etable(gchar *name,
			       gchar *string1, gchar *string2,
			       gint int1, gint int2);

GtkWidget *
create_ecmld_etable(gchar *name,
		    gchar *string1, gchar *string2,
		    gint int1, gint int2)
{
	ETableModel *model;
	GtkWidget *table;

	model = e_categories_master_list_dialog_model_new();
	table = e_table_scrolled_new(model, NULL, SPEC, NULL);

	/* We show the table here, since the libglade 'show' property
	 * doesn't work for custom widgets. */
	gtk_widget_show (table);

	return table;
}

static void
setup_gui (ECategoriesMasterListDialog *ecmld)
{
	GladeXML *gui = glade_xml_new (
		GAL_GLADEDIR "/e-categories-master-list-dialog.glade", NULL, PACKAGE);
	GtkWidget *dialog;

	ecmld->priv->gui = gui;

	dialog = glade_xml_get_widget (gui, "dialog-ecmld");

	g_object_weak_ref (G_OBJECT (dialog),
			   dialog_destroyed, ecmld);
	if (dialog && GTK_IS_DIALOG (dialog))
		g_signal_connect (dialog, "response",
				  G_CALLBACK (dialog_response), ecmld);
}

static void
ecmld_init (ECategoriesMasterListDialog *ecmld)
{
	ecmld->priv       = g_new (ECategoriesMasterListDialogPriv, 1);

	ecmld->priv->ecml = NULL;
	ecmld->priv->gui  = NULL;

	setup_gui (ecmld);
}

ECategoriesMasterListDialog *
e_categories_master_list_dialog_construct (ECategoriesMasterListDialog *ecmld,
					   ECategoriesMasterList       *ecml)
{
	ETableModel *model;
	GtkWidget *scrolled;

	g_return_val_if_fail (ecmld != NULL, NULL);
	g_return_val_if_fail (ecml != NULL, NULL);

	ecmld->priv->ecml = ecml;
	g_object_ref (ecmld->priv->ecml);

	scrolled = glade_xml_get_widget (ecmld->priv->gui, "custom-etable");
	if (scrolled && E_IS_TABLE_SCROLLED (scrolled)) {
		ETable *table = e_table_scrolled_get_table (E_TABLE_SCROLLED(scrolled));
		g_object_get (table,
			      "model", &model,
			      NULL);
		g_object_set (model,
			      "ecml", ecml,
			      NULL);
	}

	return E_CATEGORIES_MASTER_LIST_DIALOG (ecmld);
}

/**
 * e_categories_master_list_dialog_new:
 *
 * Creates a new ECategoriesMasterListDialog object.
 *
 * Returns: The ECategoriesMasterListDialog object.
 */
ECategoriesMasterListDialog *
e_categories_master_list_dialog_new (ECategoriesMasterList *ecml)
{
	ECategoriesMasterListDialog *ecmld = g_object_new (E_CATEGORIES_MASTER_LIST_DIALOG_TYPE, NULL);
	GtkWidget *dialog;

	if (e_categories_master_list_dialog_construct (ecmld, ecml) == NULL){
		gtk_object_destroy (GTK_OBJECT (ecmld));
		return NULL;
	}

	dialog = glade_xml_get_widget (ecmld->priv->gui, "dialog-ecmld");

	if (dialog && GTK_IS_WIDGET (dialog))
		gtk_widget_show (dialog);
	return E_CATEGORIES_MASTER_LIST_DIALOG (ecmld);
}

/**
 * e_categories_master_list_dialog_raise:
 * @ecmld: The ECategoriesMasterListDialog object.
 *
 * Raises the dialog associated with this ECategoriesMasterListDialog object.
 */
void
e_categories_master_list_dialog_raise (ECategoriesMasterListDialog *ecmld)
{
	GtkWidget *dialog;

	dialog = glade_xml_get_widget (ecmld->priv->gui, "dialog-ecmld");

	if (dialog && GTK_IS_WIDGET (dialog))
		gdk_window_raise (dialog->window);
}

E_MAKE_TYPE(e_categories_master_list_dialog, "ECategoriesMasterListDialog", ECategoriesMasterListDialog, ecmld_class_init, ecmld_init, PARENT_TYPE)
