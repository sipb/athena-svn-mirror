/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-categories-master-list-dialog-model.c
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

#include "e-categories-master-list-dialog-model.h"

#include <gnome.h>

#define PARENT_TYPE e_table_model_get_type()
static ETableModelClass *parent_class;

/*
 * ECategoriesMasterListDialogModel callbacks
 * These are the callbacks that define the behavior of our custom model.
 */
static void ecmldm_set_arg (GtkObject *o, GtkArg *arg, guint arg_id);
static void ecmldm_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

struct ECategoriesMasterListDialogModelPriv {
	ECategoriesMasterList *ecml;

	int ecml_changed_id;
};


enum {
	ARG_0,
	ARG_ECML
};

#define COLS (3)

static void
ecmldm_destroy(GtkObject *object)
{
	ECategoriesMasterListDialogModel *model = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(object);

	if (model->priv->ecml) {
		if (model->priv->ecml_changed_id)
			gtk_signal_disconnect (GTK_OBJECT (model->priv->ecml),
					       model->priv->ecml_changed_id);
		gtk_object_unref (GTK_OBJECT (model->priv->ecml));
	}
	g_free (model->priv);
	model->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* This function returns the number of columns in our ETableModel. */
static int
ecmldm_col_count (ETableModel *etm)
{
	return COLS;
}

/* This function returns the number of rows in our ETableModel. */
static int
ecmldm_row_count (ETableModel *etm)
{
	ECategoriesMasterListDialogModel *ecmldm = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(etm);
	if (ecmldm->priv->ecml)
		return e_categories_master_list_count (ecmldm->priv->ecml);
	else
		return 0;
}

/* This function returns the value at a particular point in our ETableModel. */
static void *
ecmldm_value_at (ETableModel *etm, int col, int row)
{
	ECategoriesMasterListDialogModel *ecmldm = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(etm);
	const char *value;
	if (ecmldm->priv->ecml == NULL)
		return NULL;
	if (col < 0 || row < 0 || col >= COLS || row >= e_categories_master_list_count (ecmldm->priv->ecml))
		return NULL;

	value = e_categories_master_list_nth (ecmldm->priv->ecml, row);
	return (void *)(value ? value : "");
}

/* This function sets the value at a particular point in our ETableModel. */
static void
ecmldm_set_value_at (ETableModel *etm, int col, int row, const void *val)
{
	ECategoriesMasterListDialogModel *ecmldm = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(etm);

	if (ecmldm->priv->ecml == NULL)
		return;
	if (col < 0 || row < 0 || col >= COLS || row >= e_categories_master_list_count (ecmldm->priv->ecml))
		return;
	e_categories_master_list_delete (ecmldm->priv->ecml, row);
	if (val && *(char *)val)
		e_categories_master_list_append (ecmldm->priv->ecml, (const char *) val, NULL, NULL);
	e_categories_master_list_commit (ecmldm->priv->ecml);
}

/* This function returns whether a particular cell is editable. */
static gboolean
ecmldm_is_cell_editable (ETableModel *etm, int col, int row)
{
	return TRUE;
	/* return E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(etm)->priv->editable; */
}

static void
ecmldm_append_row (ETableModel *etm, ETableModel *source, gint row)
{
	ECategoriesMasterListDialogModel *ecmldm = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(etm);
	char *val;

	if (ecmldm->priv->ecml == NULL)
		return;

	val = e_table_model_value_at (source, 0, row);

	if (val && *val)
		e_categories_master_list_append (ecmldm->priv->ecml, (const char *) val, NULL, NULL);
	e_categories_master_list_commit (ecmldm->priv->ecml);
}

/* This function duplicates the value passed to it. */
static void *
ecmldm_duplicate_value (ETableModel *etm, int col, const void *value)
{
	return g_strdup(value);
}

/* This function frees the value passed to it. */
static void
ecmldm_free_value (ETableModel *etm, int col, void *value)
{
	g_free(value);
}

static void *
ecmldm_initialize_value (ETableModel *etm, int col)
{
	return g_strdup("");
}

static gboolean
ecmldm_value_is_empty (ETableModel *etm, int col, const void *value)
{
	return !(value && *(char *)value);
}

static char *
ecmldm_value_to_string (ETableModel *etm, int col, const void *value)
{
	return g_strdup(value);
}

static void
ecmldm_class_init (GtkObjectClass *object_class)
{
	ETableModelClass *model_class = (ETableModelClass *) object_class;

	parent_class                  = gtk_type_class (PARENT_TYPE);

	object_class->destroy         = ecmldm_destroy;
	object_class->set_arg         = ecmldm_set_arg;
	object_class->get_arg         = ecmldm_get_arg;

	gtk_object_add_arg_type ("ECategoriesMasterListDialogModel::ecml", E_CATEGORIES_MASTER_LIST_TYPE, 
				 GTK_ARG_READWRITE, ARG_ECML);

	model_class->column_count     = ecmldm_col_count;
	model_class->row_count        = ecmldm_row_count;
	model_class->value_at         = ecmldm_value_at;
	model_class->set_value_at     = ecmldm_set_value_at;
	model_class->is_cell_editable = ecmldm_is_cell_editable;
	model_class->append_row       = ecmldm_append_row;
	model_class->duplicate_value  = ecmldm_duplicate_value;
	model_class->free_value       = ecmldm_free_value;
	model_class->initialize_value = ecmldm_initialize_value;
	model_class->value_is_empty   = ecmldm_value_is_empty;
	model_class->value_to_string  = ecmldm_value_to_string;
}

static void
ecmldm_init (GtkObject *object)
{
	ECategoriesMasterListDialogModel *model = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL(object);
	model->priv       = g_new (ECategoriesMasterListDialogModelPriv, 1);
	model->priv->ecml = NULL;
}

static void
ecml_changed (ECategoriesMasterList *ecml, ECategoriesMasterListDialogModel *ecmldm)
{
	e_table_model_changed (E_TABLE_MODEL (ecmldm));
}

static void
ecmldm_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListDialogModel *model;

	model = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL (o);
	
	switch (arg_id){
	case ARG_ECML:
		if (model->priv->ecml) {
			if (model->priv->ecml_changed_id)
				gtk_signal_disconnect (GTK_OBJECT (model->priv->ecml),
						       model->priv->ecml_changed_id);
			gtk_object_unref (GTK_OBJECT (model->priv->ecml));
		}
		model->priv->ecml = (ECategoriesMasterList *) GTK_VALUE_OBJECT (*arg);
		if (model->priv->ecml) {
			gtk_object_ref (GTK_OBJECT (model->priv->ecml));
			model->priv->ecml_changed_id =
				gtk_signal_connect (GTK_OBJECT (model->priv->ecml), "changed",
						    GTK_SIGNAL_FUNC (ecml_changed), model);
		}
		e_table_model_changed (E_TABLE_MODEL(model));
		break;
	}
}

static void
ecmldm_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListDialogModel *ecmldm;

	ecmldm = E_CATEGORIES_MASTER_LIST_DIALOG_MODEL (object);

	switch (arg_id) {
	case ARG_ECML:
		GTK_VALUE_OBJECT (*arg) = (GtkObject *)ecmldm->priv->ecml;
		break;
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

GtkType
e_categories_master_list_dialog_model_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"ECategoriesMasterListDialogModel",
			sizeof (ECategoriesMasterListDialogModel),
			sizeof (ECategoriesMasterListDialogModelClass),
			(GtkClassInitFunc) ecmldm_class_init,
			(GtkObjectInitFunc) ecmldm_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

ETableModel *
e_categories_master_list_dialog_model_new (void)
{
	ECategoriesMasterListDialogModel *et;

	et = gtk_type_new (e_categories_master_list_dialog_model_get_type ());
	
	return E_TABLE_MODEL(et);
}
