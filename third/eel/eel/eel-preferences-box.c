/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-box.h - A preferences box is a widget that manages
                           prefernece panes.  Only one pane can be
                           visible at any given time.

   Copyright (C) 1999, 2000, 2001 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/


#include <config.h>
#include "eel-preferences-box.h"

#include <eel/eel-gtk-extensions.h>
#include <eel/eel-gtk-macros.h>
#include <eel/eel-string.h>
#include <eel/eel-i18n.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkscrolledwindow.h>
#include <libgnome/gnome-util.h>

#define NUM_CATEGORY_COLUMNS 1
#define CATEGORY_COLUMN 0
#define SPACING_BETWEEN_CATEGORIES_AND_PANES 8 
#define BORDER_WIDTH 5
#define STRING_LIST_DEFAULT_TOKENS_DELIMETER ","

typedef struct
{
	char *pane_name;
	EelPreferencesPane *pane_widget;
} PaneInfo;

struct EelPreferencesBoxDetails
{
	GtkTreeView  *category_view;
	GtkListStore *category_store;
	GtkWidget    *pane_notebook;
	GList        *panes;
	char         *selected_pane;
};

/* EelPreferencesBoxClass methods */
static void      eel_preferences_box_class_init         (EelPreferencesBoxClass *preferences_box_class);
static void      eel_preferences_box_init               (EelPreferencesBox      *preferences_box);

/* GObjectClass methods */
static void      eel_preferences_box_finalize           (GObject                *object);

/* Misc private stuff */
static void      preferences_box_category_list_recreate (EelPreferencesBox      *preferences_box);
static void      preferences_box_select_pane            (EelPreferencesBox      *preferences_box,
							 const char             *name);

/* PaneInfo functions */
static PaneInfo *pane_info_new                          (const char             *pane_name);
static void      pane_info_free                         (PaneInfo               *info);

/* Category list callbacks */
static void      category_list_selection_changed        (GtkTreeSelection       *selection,
							 gpointer                callback_data);

/* Convience functions */
static GtkTreeIter *preferences_box_find_row            (GtkListStore           *model,
							 const char             *pane_name);

EEL_CLASS_BOILERPLATE (EelPreferencesBox, eel_preferences_box, GTK_TYPE_HBOX)

/*
 * EelPreferencesBoxClass methods
 */
static void
eel_preferences_box_class_init (EelPreferencesBoxClass *preferences_box_class)
{
	G_OBJECT_CLASS (preferences_box_class)->finalize = eel_preferences_box_finalize;
}

static void
eel_preferences_box_init (EelPreferencesBox *preferences_box)
{
	preferences_box->details = g_new0 (EelPreferencesBoxDetails, 1);
}

/*
 * GObjectClass methods
 */
static void
eel_preferences_box_finalize (GObject *object)
{
	EelPreferencesBox *preferences_box;
	
	g_return_if_fail (EEL_IS_PREFERENCES_BOX (object));
	
	preferences_box = EEL_PREFERENCES_BOX (object);

	if (preferences_box->details->panes) {
		GList *panes;
		
		panes = preferences_box->details->panes;

		while (panes) {
			PaneInfo * info = panes->data;
			
			g_assert (info != NULL);
			pane_info_free (info);
			panes = panes->next;
		}
		
		g_list_free (preferences_box->details->panes);
	}

	g_object_unref (preferences_box->details->category_store);

	g_free (preferences_box->details->selected_pane);
	g_free (preferences_box->details);
	
	EEL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/*
 * Misc private stuff
 */
static void
preferences_box_select_pane (EelPreferencesBox *preferences_box,
			     const char        *pane_name)
{
	GList *pane_iterator;
	
	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (preferences_box->details != NULL);
	g_return_if_fail (preferences_box->details->panes != NULL);
	g_return_if_fail (pane_name != NULL);

	/* Show only the corresponding pane widget */
	pane_iterator = preferences_box->details->panes;

	while (pane_iterator) {
		PaneInfo *info = pane_iterator->data;

		g_assert (info != NULL);
		
		if (eel_str_is_equal (pane_name, info->pane_name)) {
 			gtk_widget_show (GTK_WIDGET (info->pane_widget));
 			gtk_notebook_set_current_page
				(GTK_NOTEBOOK (preferences_box->details->pane_notebook), 
				 g_list_position (preferences_box->details->panes, pane_iterator));

			if (pane_name != preferences_box->details->selected_pane) {
				g_free (preferences_box->details->selected_pane);
				preferences_box->details->selected_pane = g_strdup (pane_name);
			}
			return;
		}
		
		pane_iterator = pane_iterator->next;
	}

	g_warning ("Pane '%s' could not be found.", pane_name);
}

typedef struct {
	const char  *pane_name;
	GtkTreeIter *result_iter;
} foreach_find_t;

static gboolean
preferences_model_foreach_find (GtkTreeModel *model,
				GtkTreePath  *path,
				GtkTreeIter  *iter,
				gpointer      data)
{
	gboolean ret;
	char *pane_name;
	foreach_find_t *ctx;

	ctx = data;
	ret = FALSE;

	gtk_tree_model_get (model, iter, CATEGORY_COLUMN, &pane_name, -1);

	if (!strcmp (ctx->pane_name, pane_name)) {
		ctx->result_iter = gtk_tree_iter_copy (iter);
		ret = TRUE;
	}

	g_free (pane_name);

	return ret;
}

static GtkTreeIter *
preferences_box_find_row (GtkListStore *model,
			  const char   *pane_name)
{
	foreach_find_t ctx;

	ctx.pane_name = pane_name;
	ctx.result_iter = NULL;

	gtk_tree_model_foreach (GTK_TREE_MODEL (model),
				preferences_model_foreach_find,
				&ctx);

	return ctx.result_iter;
}

static void
preferences_box_category_list_recreate (EelPreferencesBox *preferences_box)
{
	GList *l;
	PaneInfo *info;
	GtkTreeIter iter, *selected_iter;

	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (GTK_IS_TREE_VIEW (preferences_box->details->category_view));

	gtk_list_store_clear (preferences_box->details->category_store);

	selected_iter = NULL;
	for (l = preferences_box->details->panes; l != NULL; l = l->next) {
		info = l->data;
	
		g_assert (EEL_IS_PREFERENCES_PANE (info->pane_widget));

		if (eel_preferences_pane_get_num_visible_groups (info->pane_widget) > 0) {

			gtk_list_store_append (preferences_box->details->category_store, &iter);
			gtk_list_store_set (preferences_box->details->category_store,
					    &iter, CATEGORY_COLUMN, info->pane_name, -1);

			if (eel_str_is_equal (info->pane_name, preferences_box->details->selected_pane)) {
				selected_iter = gtk_tree_iter_copy (&iter);
			}

			gtk_tree_model_iter_next (
				GTK_TREE_MODEL (preferences_box->details->category_store), &iter);
		}
	}
	
	gtk_widget_queue_resize (GTK_WIDGET (preferences_box->details->category_view));

	if (selected_iter == NULL) {
		if (gtk_tree_model_get_iter_root (
			GTK_TREE_MODEL (preferences_box->details->category_store), &iter)) {
			selected_iter = gtk_tree_iter_copy (&iter);
		} else {
			g_warning ("No preferenced root");
			selected_iter = NULL;
		}
	}

	if (selected_iter) {
		gtk_tree_selection_select_iter (
			gtk_tree_view_get_selection (preferences_box->details->category_view),
			selected_iter);
		gtk_tree_iter_free (selected_iter);
	}

	if (preferences_box->details->selected_pane) {
		preferences_box_select_pane (preferences_box,
					     preferences_box->details->selected_pane);
	}
}

/*
 * PaneInfo functions
 */
static PaneInfo *
pane_info_new (const char *pane_name)
{
	PaneInfo * info;

	g_assert (pane_name != NULL);

	info = g_new0 (PaneInfo, 1);

	info->pane_name = g_strdup (pane_name);

	return info;
}

static void
pane_info_free (PaneInfo *info)
{
	g_assert (info != NULL);
	
	g_free (info->pane_name);
	g_free (info);
}

/*
 * Category list callbacks
 */
static void
category_list_selection_changed (GtkTreeSelection *selection,
				 gpointer          callback_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *pane_name = NULL;
	
	g_return_if_fail (EEL_IS_PREFERENCES_BOX (callback_data));

	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}

	gtk_tree_model_get (model, &iter, CATEGORY_COLUMN, &pane_name, -1);

	g_return_if_fail (pane_name != NULL);

	preferences_box_select_pane (EEL_PREFERENCES_BOX (callback_data), pane_name);

	g_free (pane_name);
}

/*
 * EelPreferencesBox public methods
 */
GtkWidget*
eel_preferences_box_new (void)
{
	GtkWidget         *tree_view;
	GtkWidget         *scrolled_window;
	EelPreferencesBox *preferences_box;
	GtkWidget         *categories_vbox;
	GtkWidget         *categories_label;

	preferences_box = EEL_PREFERENCES_BOX
		(gtk_widget_new (eel_preferences_box_get_type (), NULL));

	/* Configure ourselves */
 	gtk_box_set_homogeneous (GTK_BOX (preferences_box), FALSE);
 	gtk_box_set_spacing (GTK_BOX (preferences_box), SPACING_BETWEEN_CATEGORIES_AND_PANES);
	gtk_container_set_border_width (GTK_CONTAINER (preferences_box), BORDER_WIDTH);

	/* The category list */
	categories_vbox = gtk_vbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (preferences_box), 
			    categories_vbox, FALSE, TRUE, 0);

	categories_label = gtk_label_new_with_mnemonic (_("Cat_egories:"));
	gtk_misc_set_alignment (GTK_MISC (categories_label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (categories_vbox), 
			    categories_label, FALSE, FALSE, 0);

	preferences_box->details->category_store = gtk_list_store_new (
		NUM_CATEGORY_COLUMNS, G_TYPE_STRING);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
					     GTK_SHADOW_IN);
	tree_view = gtk_tree_view_new_with_model (
		GTK_TREE_MODEL (preferences_box->details->category_store));
	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);

	preferences_box->details->category_view = GTK_TREE_VIEW (tree_view);

	gtk_label_set_mnemonic_widget (GTK_LABEL (categories_label),
				       tree_view);

	gtk_tree_view_insert_column_with_attributes
		(preferences_box->details->category_view,
		 0, "", gtk_cell_renderer_text_new (),
		 "text", CATEGORY_COLUMN,
		 NULL);
	gtk_tree_view_set_headers_visible (preferences_box->details->category_view, FALSE);
	gtk_tree_view_columns_autosize (preferences_box->details->category_view);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (preferences_box->details->category_view),
				     GTK_SELECTION_BROWSE);

	g_signal_connect (
		gtk_tree_view_get_selection (
			preferences_box->details->category_view),
		"changed",
		G_CALLBACK (category_list_selection_changed),
		preferences_box);
	
	gtk_box_pack_start (GTK_BOX (categories_vbox), 
			    scrolled_window, TRUE, TRUE, 0);

	/* The gtk notebook that the panes go into. */
	preferences_box->details->pane_notebook = gtk_notebook_new ();

	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (preferences_box->details->pane_notebook), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (preferences_box->details->pane_notebook), FALSE);
	
	gtk_box_pack_start (GTK_BOX (preferences_box),
			    preferences_box->details->pane_notebook,
			    TRUE,
			    TRUE,
			    0);

	gtk_widget_show_all (categories_vbox);
	gtk_widget_show (preferences_box->details->pane_notebook);

	return GTK_WIDGET (preferences_box);
}

static EelPreferencesPane *
preferences_box_add_pane (EelPreferencesBox *preferences_box,
			  const char *pane_title)
{
	PaneInfo *info;

	g_return_val_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box), NULL);
	g_return_val_if_fail (pane_title != NULL, NULL);

	info = pane_info_new (pane_title);
	
	preferences_box->details->panes = g_list_append (preferences_box->details->panes, info);
	
	info->pane_widget = EEL_PREFERENCES_PANE (eel_preferences_pane_new ());
	
	gtk_notebook_append_page (GTK_NOTEBOOK (preferences_box->details->pane_notebook),
				  GTK_WIDGET (info->pane_widget),
				  NULL);

	return info->pane_widget;
}

void
eel_preferences_box_update (EelPreferencesBox	*preferences_box)
{
	GList *iterator;

	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));

	for (iterator = preferences_box->details->panes; iterator != NULL; iterator = iterator->next) {
		PaneInfo *info = iterator->data;
		
		g_assert (EEL_IS_PREFERENCES_PANE (info->pane_widget));

		eel_preferences_pane_update (info->pane_widget);
	}

	preferences_box_category_list_recreate (preferences_box);
}

static PaneInfo *
preferences_box_find_pane (const EelPreferencesBox *preferences_box,
			   const char *pane_name)
{
	GList *node;
	PaneInfo *info;

	g_return_val_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box), NULL);

	for (node = preferences_box->details->panes; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		info = node->data;
		if (eel_str_is_equal (info->pane_name, pane_name)) {
			return info;
		}
	}

	return NULL;
}

static EelPreferencesPane *
preferences_box_find_pane_widget (const EelPreferencesBox *preferences_box,
				  const char *pane_name)
{
	PaneInfo *info;

	g_return_val_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box), NULL);

	info = preferences_box_find_pane (preferences_box, pane_name);
	if (info == NULL) {
		return NULL;
	}

	return info->pane_widget;
}

static void
preferences_box_populate_pane (EelPreferencesBox *preferences_box,
			       const char *pane_name,
			       const EelPreferencesItemDescription *items)
{
	EelPreferencesPane *pane;
	EelPreferencesGroup *group;
	EelPreferencesItem *item;
	EelStringList *group_names;
	const char *translated_group_name;
	guint i;

	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (pane_name != NULL);
	g_return_if_fail (items != NULL);

	/* Create the pane if needed */
	pane = preferences_box_find_pane_widget (preferences_box, pane_name);
	if (pane == NULL) {
		pane = EEL_PREFERENCES_PANE (preferences_box_add_pane (preferences_box, pane_name));
	}

	group_names = eel_string_list_new (TRUE);

	for (i = 0; items[i].group_name != NULL; i++) {
		translated_group_name = gettext (items[i].group_name);
		if (!eel_string_list_contains (group_names, translated_group_name)) {
			eel_string_list_insert (group_names, translated_group_name);
			eel_preferences_pane_add_group (pane,
							translated_group_name);
		}
	}

	for (i = 0; items[i].group_name != NULL; i++) {
		group = EEL_PREFERENCES_GROUP (eel_preferences_pane_find_group (pane,
										gettext (items[i].group_name)));
		g_return_if_fail (EEL_IS_PREFERENCES_GROUP (group));

		if (items[i].preference_name != NULL) {
			if (items[i].preference_description != NULL) {
				eel_preferences_set_description (items[i].preference_name,
								 gettext (items[i].preference_description));
			}
		
			item = EEL_PREFERENCES_ITEM (eel_preferences_group_add_item (group,
										     items[i].preference_name,
										     items[i].item_type,
										     items[i].column));
			
			/* Install a control preference if needed */
			if (items[i].control_preference_name != NULL) {
				eel_preferences_item_set_control_preference (item,
									     items[i].control_preference_name);
				eel_preferences_item_set_control_action (item,
									 items[i].control_action);				
				eel_preferences_pane_add_control_preference (pane,
									     items[i].control_preference_name);
			}
			
			/* Install exceptions to enum lists uniqueness rule */
			if (items[i].enumeration_list_unique_exceptions != NULL) {
				g_assert (items[i].item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL
					  || items[i].item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL);
				eel_preferences_item_enumeration_list_set_unique_exceptions (item,
											     items[i].enumeration_list_unique_exceptions,
											     STRING_LIST_DEFAULT_TOKENS_DELIMETER);
			}
		}

		if (items[i].populate_function != NULL) {
			(* items[i].populate_function) (group);
		}
	}

	eel_string_list_free (group_names);
}

void
eel_preferences_box_populate (EelPreferencesBox *preferences_box,
			      const EelPreferencesPaneDescription *panes)
{
	guint i;

	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (panes != NULL);

	for (i = 0; panes[i].pane_name != NULL; i++) {
		preferences_box_populate_pane (preferences_box,
					       gettext (panes[i].pane_name),
					       panes[i].items);
	}

	eel_preferences_box_update (preferences_box);
}

GtkWidget *
eel_preferences_dialog_new (const char *title,
			    const EelPreferencesPaneDescription *panes)
{
	GtkWidget *dialog;
	GtkWidget *preference_box;
	GtkWidget *vbox;

	g_return_val_if_fail (title != NULL, NULL);
	
	dialog = gtk_dialog_new_with_buttons (title, NULL, 0,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					      NULL);

	/* Setup the dialog */
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

  	gtk_container_set_border_width (GTK_CONTAINER (dialog), 0);
	
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	preference_box = eel_preferences_box_new ();

	vbox = GTK_DIALOG (dialog)->vbox;
	
	gtk_box_set_spacing (GTK_BOX (vbox), 10);
	
	gtk_box_pack_start (GTK_BOX (vbox),
			    preference_box,
			    TRUE,	/* expand */
			    TRUE,	/* fill */
			    0);		/* padding */

	if (panes != NULL) {
		eel_preferences_dialog_populate (GTK_WINDOW (dialog), panes);
	}
	
	gtk_widget_show (preference_box);

	return dialog;
}

EelPreferencesBox *
eel_preferences_dialog_get_box (const GtkWindow *dialog)
{
	GtkWidget *vbox;
	const GList *last_node;
	const GtkBoxChild *box_child;

	g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);

	vbox = GTK_DIALOG (dialog)->vbox;

	g_return_val_if_fail (GTK_IS_VBOX (vbox), NULL);
	last_node = g_list_last (GTK_BOX (vbox)->children);
	g_return_val_if_fail (last_node != NULL, NULL);
	g_return_val_if_fail (last_node->data != NULL, NULL);
	box_child = last_node->data;
	g_return_val_if_fail (EEL_IS_PREFERENCES_BOX (box_child->widget), NULL);
	return EEL_PREFERENCES_BOX (box_child->widget);
}

void
eel_preferences_dialog_populate (GtkWindow *dialog,
				 const EelPreferencesPaneDescription *panes)
{
	EelPreferencesBox *preferences_box;

	g_return_if_fail (GTK_IS_WINDOW (dialog));
	g_return_if_fail (panes != NULL);
	preferences_box = eel_preferences_dialog_get_box (dialog);
	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	eel_preferences_box_populate (preferences_box, panes);
}

void
eel_preferences_box_for_each_pane (const EelPreferencesBox *preferences_box,
				   EelPreferencesBoxForEachCallback callback,
				   gpointer callback_data)
{
	GList *node;
	PaneInfo *info;
	
	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (callback != NULL);
	
	for (node = preferences_box->details->panes; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		info = node->data;
		(* callback) (info->pane_name, info->pane_widget, callback_data);
	}
}

void
eel_preferences_box_rename_pane (EelPreferencesBox *preferences_box,
				 const char        *pane_name,
				 const char        *new_pane_name)
{
	GtkTreeIter *iter;
	PaneInfo    *pane_info;

	g_return_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box));
	g_return_if_fail (eel_strlen (pane_name) > 0);
	g_return_if_fail (eel_strlen (new_pane_name) > 0);

	if (eel_str_is_equal (pane_name, new_pane_name)) {
		return;
	}

	pane_info = preferences_box_find_pane (preferences_box, pane_name);
	if (pane_info == NULL) {
		g_warning ("The box does not have a pane called '%s'", pane_name);
		return;
	}

	iter = preferences_box_find_row (preferences_box->details->category_store,
					 pane_name);
	if (iter == NULL) {
		g_warning ("No pane called '%s'", pane_info->pane_name);
		return;
	}

	g_free (pane_info->pane_name);
	pane_info->pane_name = g_strdup (new_pane_name);

	gtk_list_store_set (preferences_box->details->category_store,
			    iter, 0, pane_info->pane_name, -1);

	gtk_tree_iter_free (iter);
}

char *
eel_preferences_box_get_pane_name (const EelPreferencesBox *preferences_box,
				   const EelPreferencesPane *pane)
{
	GList *node;
	PaneInfo *info;
	
	g_return_val_if_fail (EEL_IS_PREFERENCES_BOX (preferences_box), NULL);
	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), NULL);
	
	for (node = preferences_box->details->panes; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		info = node->data;
		if (info->pane_widget == pane) {
			return g_strdup (info->pane_name);
		}
	}

	return NULL;
}

char *
eel_preferences_box_get_active_pane (EelPreferencesBox *preferences_box)
{
	return g_strdup (preferences_box->details->selected_pane);
}
