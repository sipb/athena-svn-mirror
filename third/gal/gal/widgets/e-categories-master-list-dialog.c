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
#include <libgnomeui/gnome-dialog.h>
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

#define PARENT_TYPE (gtk_object_get_type())

static GtkObjectClass *parent_class;

/* The arguments we take */
enum {
	ARG_0,
	ARG_ECML
};

static void
ecmld_destroy (GtkObject *object)
{
	ECategoriesMasterListDialog *ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (object);

	if (ecmld->priv->ecml)
		gtk_object_unref (GTK_OBJECT (ecmld->priv->ecml));
	if (ecmld->priv->gui)
		gtk_object_unref (GTK_OBJECT (ecmld->priv->gui));
	g_free (ecmld->priv);
	ecmld->priv = NULL;
	
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
ecmld_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListDialog *ecmld;
	GtkWidget *scrolled;

	ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (o);
	
	switch (arg_id){
	case ARG_ECML:
		if (ecmld->priv->ecml) {
			gtk_object_unref (GTK_OBJECT (ecmld->priv->ecml));
		}

		ecmld->priv->ecml = (ECategoriesMasterList *) GTK_VALUE_OBJECT (*arg);

		if (ecmld->priv->ecml)
			gtk_object_ref (GTK_OBJECT(ecmld->priv->ecml));

		scrolled = glade_xml_get_widget (ecmld->priv->gui, "custom-etable");
		if (scrolled && E_IS_TABLE_SCROLLED (scrolled)) {
			ETable *table;
			ETableModel *model;

			table = e_table_scrolled_get_table (E_TABLE_SCROLLED(scrolled));
			gtk_object_get (GTK_OBJECT(table),
					"model", &model,
					NULL);
			gtk_object_set (GTK_OBJECT(model),
					"ecml", ecmld->priv->ecml,
					NULL);
		}
		break;
	}
}

static void
ecmld_get_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListDialog *ecmld;

	ecmld = E_CATEGORIES_MASTER_LIST_DIALOG (o);

	switch (arg_id) {
	case ARG_ECML:
		GTK_VALUE_OBJECT (*arg) = (GtkObject *) ecmld->priv->ecml;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
ecmld_class_init (GtkObjectClass *object_class)
{
	parent_class   = gtk_type_class (PARENT_TYPE);

	object_class->destroy = ecmld_destroy;
	object_class->set_arg = ecmld_set_arg;
	object_class->get_arg = ecmld_get_arg;

	gtk_object_add_arg_type ("ECategoriesMasterListDialog::ecml", E_CATEGORIES_MASTER_LIST_TYPE,
				 GTK_ARG_READWRITE, ARG_ECML);

	glade_gnome_init ();
}

static void
dialog_destroyed (GtkObject *dialog, ECategoriesMasterListDialog *ecmld)
{
	gtk_object_destroy (GTK_OBJECT (ecmld));
}

static void
dialog_clicked (GtkObject *dialog, int button, ECategoriesMasterListDialog *ecmld)
{
	GtkWidget *scrolled;
	switch (button) {
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
	case 1:    /* Close */
		gnome_dialog_close (GNOME_DIALOG (dialog));
		break;
	}
}

#if 0
static void
connect_button (ECategoriesMasterListDialog *ecmld, GladeXML *gui, const char *widget_name, void *cback)
{
	GtkWidget *button = glade_xml_get_widget (gui, widget_name);

	if (button && GTK_IS_BUTTON (button))
		gtk_signal_connect(GTK_OBJECT (button), "clicked",
				   GTK_SIGNAL_FUNC (cback), ecmld);
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

	return table;
}

static void
setup_gui (ECategoriesMasterListDialog *ecmld)
{
	GladeXML *gui = glade_xml_new_with_domain
		(GAL_GLADEDIR "/e-categories-master-list-dialog.glade", NULL, PACKAGE);
	GtkWidget *dialog;

	ecmld->priv->gui = gui;

	dialog = glade_xml_get_widget (gui, "dialog-ecmld");

	gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
			    GTK_SIGNAL_FUNC (dialog_destroyed), ecmld);
	if (dialog && GNOME_IS_DIALOG (dialog))
		gtk_signal_connect (GTK_OBJECT (dialog), "clicked",
				    GTK_SIGNAL_FUNC (dialog_clicked), ecmld);
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
	gtk_object_ref (GTK_OBJECT(ecmld->priv->ecml));

	scrolled = glade_xml_get_widget (ecmld->priv->gui, "custom-etable");
	if (scrolled && E_IS_TABLE_SCROLLED (scrolled)) {
		ETable *table = e_table_scrolled_get_table (E_TABLE_SCROLLED(scrolled));
		gtk_object_get (GTK_OBJECT(table),
				"model", &model,
				NULL);
		gtk_object_set (GTK_OBJECT(model),
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
	ECategoriesMasterListDialog *ecmld = gtk_type_new (E_CATEGORIES_MASTER_LIST_DIALOG_TYPE);
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
