/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-categories.c
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

#include "e-categories.h"

#include "gal/util/e-i18n.h"
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomeui/gnome-stock.h>

#include "gal/e-table/e-table-scrolled.h"
#include "gal/e-table/e-table.h"
#include "gal/e-table/e-table-simple.h"
#include "gal/util/e-i18n.h"
#include "e-unicode.h"
#include "e-popup-menu.h"
#include "e-categories-master-list.h"
#include "e-categories-master-list-array.h"
#include "e-categories-master-list-dialog.h"

struct _ECategoriesPriv {
	/* item specific fields */
	char        *categories;
	GtkWidget   *entry;
	ETableModel *model;
	ETable      *table;

	int          list_length;
	char       **category_list;
	gboolean    *selected_list;

	GladeXML    *gui;

	ECategoriesMasterList *ecml;
	gint         ecml_changed_id;

	ECategoriesMasterListDialog *ecmld;
	int ecmld_destroy_id;
};

static void e_categories_init		(ECategories		 *card);
static void e_categories_class_init	(ECategoriesClass	 *klass);
static void e_categories_set_arg         (GtkObject *o, GtkArg *arg, guint arg_id);
static void e_categories_get_arg         (GtkObject *object, GtkArg *arg, guint arg_id);
static void e_categories_destroy         (GtkObject *object);
static int e_categories_col_count (ETableModel *etc, gpointer data);
static int e_categories_row_count (ETableModel *etc, gpointer data);
static void *e_categories_value_at (ETableModel *etc, int col, int row, gpointer data);
static void e_categories_set_value_at (ETableModel *etc, int col, int row, const void *val, gpointer data);
static gboolean e_categories_is_cell_editable (ETableModel *etc, int col, int row, gpointer data);
static gboolean e_categories_has_save_id (ETableModel *etc, gpointer data);
static char *e_categories_get_save_id (ETableModel *etc, int row, gpointer data);
static void *e_categories_duplicate_value (ETableModel *etc, int col, const void *value, gpointer data);
static void e_categories_free_value (ETableModel *etc, int col, void *value, gpointer data);
static void *e_categories_initialize_value (ETableModel *etc, int col, gpointer data);
static gboolean e_categories_value_is_empty (ETableModel *etc, int col, const void *value, gpointer data);
static char * e_categories_value_to_string (ETableModel *etc, int col, const void *value, gpointer data);
static void e_categories_toggle (ECategories *categories, int row);

static GnomeDialogClass *parent_class = NULL;

/* The arguments we take */
enum {
	ARG_0,
	ARG_CATEGORIES,
	ARG_HEADER,
	ARG_ECML,
};

GtkType
e_categories_get_type (void)
{
	static GtkType contact_editor_categories_type = 0;

	if (!contact_editor_categories_type)
		{
			static const GtkTypeInfo contact_editor_categories_info =
			{
				"ECategories",
				sizeof (ECategories),
				sizeof (ECategoriesClass),
				(GtkClassInitFunc) e_categories_class_init,
				(GtkObjectInitFunc) e_categories_init,
				/* reserved_1 */ NULL,
				/* reserved_2 */ NULL,
				(GtkClassInitFunc) NULL,
			};

			contact_editor_categories_type = gtk_type_unique (gnome_dialog_get_type (), &contact_editor_categories_info);
		}

	return contact_editor_categories_type;
}

static void
e_categories_class_init (ECategoriesClass *klass)
{
	GtkObjectClass *object_class;
	GnomeDialogClass *dialog_class;

	object_class = (GtkObjectClass*) klass;
	dialog_class = (GnomeDialogClass *) klass;

	parent_class = gtk_type_class (gnome_dialog_get_type ());

	gtk_object_add_arg_type ("ECategories::categories", GTK_TYPE_STRING,
				 GTK_ARG_READWRITE, ARG_CATEGORIES);
	gtk_object_add_arg_type ("ECategories::header", GTK_TYPE_STRING,
				 GTK_ARG_READWRITE, ARG_HEADER);
	gtk_object_add_arg_type ("ECategories::ecml", E_CATEGORIES_MASTER_LIST_TYPE,
				 GTK_ARG_READWRITE, ARG_ECML);
 
	object_class->set_arg = e_categories_set_arg;
	object_class->get_arg = e_categories_get_arg;
	object_class->destroy = e_categories_destroy;
}

static void
add_list_unique(ECategories *categories, char *string)
{
	int k;
	char *temp = e_strdup_strip(string);
	char **list = categories->priv->category_list;

	if (!*temp) {
		g_free(temp);
		return;
	}
	for (k = 0; k < categories->priv->list_length; k++) {
		if (!strcmp(list[k], temp)) {
			categories->priv->selected_list[k] = TRUE;
			break;
		}
	}
	if (k == categories->priv->list_length) {
		categories->priv->selected_list[categories->priv->list_length] = TRUE;
		list[categories->priv->list_length++] = temp;
	} else {
		g_free(temp);
	}
}

static void
do_parse_categories(ECategories *categories)
{
	char *str = categories->priv->categories;
	int length = strlen(str);
	char *copy = g_new(char, length + 1);
	int i, j;
	char **list;
	int count = 1;
	int master_count;

	e_table_model_pre_change(categories->priv->model);

	for (i = 0; i < categories->priv->list_length; i++)
		g_free(categories->priv->category_list[i]);
	g_free(categories->priv->category_list);
	g_free(categories->priv->selected_list);

	for (i = 0; str[i]; i++) {
		switch (str[i]) {
		case '\\':
			i++;
			if (!str[i])
				i--;
			break;
		case ',':
			count ++;
			break;
		}
	}
	if (categories->priv->ecml)
		master_count = e_categories_master_list_count (categories->priv->ecml);
	else
		master_count = 0;
	list = g_new(char *, count + master_count + 1);
	categories->priv->category_list = list;

	categories->priv->selected_list = g_new(gboolean, count + 1 + master_count);

	for (count = 0; count < master_count; count++) {
		list[count] = g_strdup (e_categories_master_list_nth (categories->priv->ecml, count));
		categories->priv->selected_list[count] = 0;
	}

	categories->priv->list_length = count;

	for (i = 0, j = 0; str[i]; i++, j++) {
		switch (str[i]) {
		case '\\':
			i++;
			if (str[i]) {
				copy[j] = str[i];
			} else
				i--;
			break;
		case ',':
			copy[j] = 0;
			add_list_unique(categories, copy);
			j = -1;
			break;
		default:
			copy[j] = str[i];
			break;
		}
	}
	copy[j] = 0;
	add_list_unique(categories, copy);
	g_free(copy);
	e_table_model_changed(categories->priv->model);
}

static void
e_categories_entry_change (GtkWidget *entry, 
			   ECategories *categories)
{
	g_free(categories->priv->categories);
	categories->priv->categories = e_utf8_gtk_entry_get_text(GTK_ENTRY(entry));
	do_parse_categories(categories);
}

static void
e_categories_release_ecmld (ECategories *categories)
{
	if (categories->priv->ecmld) {
		if (categories->priv->ecmld_destroy_id)
			gtk_signal_disconnect (GTK_OBJECT(categories->priv->ecmld),
					       categories->priv->ecmld_destroy_id);
		categories->priv->ecmld_destroy_id = 0;
		gtk_object_unref (GTK_OBJECT (categories->priv->ecmld));
		categories->priv->ecmld = NULL;
	}
}

static void
ecmld_destroyed (GtkObject *object, ECategories *categories)
{
	e_categories_release_ecmld (categories);
}

static void
e_categories_button_clicked (GtkWidget *button,
			     ECategories *categories)
{
	if (categories->priv->ecmld)
		e_categories_master_list_dialog_raise (categories->priv->ecmld);
	else {
		categories->priv->ecmld = e_categories_master_list_dialog_new (categories->priv->ecml);
		categories->priv->ecmld_destroy_id = gtk_signal_connect (GTK_OBJECT (categories->priv->ecmld), "destroy",
									 GTK_SIGNAL_FUNC (ecmld_destroyed), categories);
		gtk_object_ref (GTK_OBJECT (categories->priv->ecmld));
	}
}

static void 
ec_set_categories (GtkWidget *entry, const char *categories)
{
	e_utf8_gtk_entry_set_text(GTK_ENTRY(entry), categories);
}

static void
ecml_changed (ECategoriesMasterList *ecml, ECategories *categories)
{
	do_parse_categories(categories);
}

static void 
ec_set_ecml (ECategories *categories, ECategoriesMasterList *ecml)
{
	if (categories->priv->ecml) {
		if (categories->priv->ecml_changed_id)
			gtk_signal_disconnect (GTK_OBJECT(categories->priv->ecml),
					       categories->priv->ecml_changed_id);
		gtk_object_unref (GTK_OBJECT (categories->priv->ecml));
	}
	categories->priv->ecml = ecml;
	if (categories->priv->ecml) {
		gtk_object_ref (GTK_OBJECT (categories->priv->ecml));
		categories->priv->ecml_changed_id =
			gtk_signal_connect (GTK_OBJECT (categories->priv->ecml),
					    "changed",
					    GTK_SIGNAL_FUNC (ecml_changed),
					    categories);
	}
	if (categories->priv->ecmld) {
		gtk_object_set (GTK_OBJECT (categories->priv->ecmld),
				"ecml", ecml,
				NULL);
	}
	do_parse_categories(categories);
}

static void 
ec_set_header (ECategories *categories, const char *header)
{
	GtkWidget *widget;
	widget = glade_xml_get_widget(categories->priv->gui, "label-header");
	if (widget && GTK_IS_LABEL(widget))
		gtk_label_set_text(GTK_LABEL(widget), header);
}


#define INITIAL_SPEC "<ETableSpecification no-headers=\"true\" draw-grid=\"true\" cursor-mode=\"line\">\
  <ETableColumn model_col=\"0\" _title=\"Active\" expansion=\"0.0\" minimum_width=\"20\" resizable=\"false\" cell=\"checkbox\"       compare=\"integer\"/> \
  <ETableColumn model_col=\"1\" _title=\"Category\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/> \
        <ETableState>                                                           \
		<column source=\"0\"/>						\
		<column source=\"1\"/>						\
        	<grouping> <leaf column=\"1\" ascending=\"true\"/> </grouping>	\
        </ETableState> \
</ETableSpecification>"

static gint
table_key_press (ETable *table, int row, int col, GdkEvent *event, ECategories *categories)
{
	switch (event->key.keyval) {
	case GDK_space:
		e_categories_toggle(categories, row);
		return TRUE;
		break;
	}
	return FALSE;
}

static void
add_single_category (int model_row, gpointer closure)
{
	ECategories *categories = closure;
	if (model_row >= e_categories_master_list_count (categories->priv->ecml)) {
		e_categories_master_list_append (categories->priv->ecml,
						 categories->priv->category_list[model_row],
						 NULL,  /* FIXME: color for this category */
						 NULL); /* FIXME: icon for this category */
	}
}

static void
remove_single_category (int model_row, gpointer closure)
{
	ECategories *categories = closure;
	if (model_row < e_categories_master_list_count (categories->priv->ecml)) {
		e_categories_master_list_delete (categories->priv->ecml,
						 model_row);
	}
}

static void
add_category (GtkWidget *widget, gpointer user_data)
{
	ECategories *categories = user_data;

	e_table_selected_row_foreach (categories->priv->table,
				      add_single_category,
				      categories);

	e_categories_master_list_commit (categories->priv->ecml);
}

static void
remove_category (GtkWidget *widget, gpointer user_data)
{
	ECategories *categories = user_data;

	e_table_selected_row_foreach (categories->priv->table,
				      remove_single_category,
				      categories);

	e_categories_master_list_commit (categories->priv->ecml);
}

static EPopupMenu menu[] = {
	{ N_("Add to global category list"),                         NULL,
	  GTK_SIGNAL_FUNC (add_category),         NULL,  4 + 1 },
	{ N_("Add all to global category list"),                         NULL,
	  GTK_SIGNAL_FUNC (add_category),         NULL,  8 + 1 },
	{ N_("Remove from global category list"),                         NULL,
	  GTK_SIGNAL_FUNC (remove_category),         NULL,  4 + 2 },
	{ N_("Remove all from global category list"),                         NULL,
	  GTK_SIGNAL_FUNC (remove_category),         NULL,  8 + 2 },

	E_POPUP_TERMINATOR
};

typedef struct {
	guint any_in : 1;
	guint any_missing : 1;
	guint one_selected : 1;
	guint more_than_one_selected : 1;

	ECategories *categories;
} Selection;

static void
check_selection (int model_row, gpointer closure)
{
	Selection *selection = closure;
	if (selection->one_selected)
		selection->more_than_one_selected = TRUE;
	selection->one_selected = TRUE;
	if (model_row < e_categories_master_list_count (selection->categories->priv->ecml))
		selection->any_in = TRUE;
	else
		selection->any_missing = TRUE;
}

static gint
table_right_click (ETable *table, int row, int col, GdkEvent *event, ECategories *categories)
{
	Selection selection;
	int hide_mask;

	selection.any_in = FALSE;
	selection.any_missing = FALSE;
	selection.one_selected = FALSE;
	selection.more_than_one_selected = FALSE;
	selection.categories = categories;

	if (categories->priv->ecml == NULL)
		return FALSE;

	e_table_selected_row_foreach (categories->priv->table,
				      check_selection,
				      &selection);

	if (! selection.one_selected)
		return FALSE;

	if (selection.more_than_one_selected)
		hide_mask = 4;
	else
		hide_mask = 8;

	if (! selection.any_in) 
		hide_mask |= 2;
	if (! selection.any_missing) 
		hide_mask |= 1;
	e_popup_menu_run (menu, event, 0, hide_mask, categories);

	return TRUE;
}

static void
e_categories_init (ECategories *categories)
{
	GladeXML *gui;
	GtkWidget *table;
	GtkWidget *e_table;
	GtkWidget *button;
	ECategoriesMasterList *ecml;

	categories->priv = g_new(ECategoriesPriv, 1);

	categories->priv->ecml = NULL;

	categories->priv->list_length = 0;
	categories->priv->category_list = NULL;
	categories->priv->selected_list = NULL;
	categories->priv->categories = g_strdup ("");
	categories->priv->ecmld = NULL;
	categories->priv->ecmld_destroy_id = 0;

	gnome_dialog_append_button ( GNOME_DIALOG(categories),
				     GNOME_STOCK_BUTTON_OK);
	
	gnome_dialog_append_button ( GNOME_DIALOG(categories),
				     GNOME_STOCK_BUTTON_CANCEL);

	gnome_dialog_set_default (GNOME_DIALOG (categories), 0);

	gtk_window_set_policy(GTK_WINDOW(categories), FALSE, TRUE, FALSE);

	gui = glade_xml_new_with_domain (GAL_GLADEDIR "/gal-categories.glade", NULL, PACKAGE);
	categories->priv->gui = gui;

	table = glade_xml_get_widget(gui, "table-categories");
	gtk_widget_ref(table);
	gtk_container_remove(GTK_CONTAINER(table->parent), table);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (categories)->vbox), table, TRUE, TRUE, 0);
	gtk_widget_unref(table);

	categories->priv->entry = glade_xml_get_widget(gui, "entry-categories");
	
	gtk_signal_connect(GTK_OBJECT(categories->priv->entry), "changed",
			   GTK_SIGNAL_FUNC(e_categories_entry_change), categories);

	button = glade_xml_get_widget(gui, "button-ecmld");
	
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(e_categories_button_clicked), categories);

	categories->priv->model = e_table_simple_new(e_categories_col_count, 
						     e_categories_row_count,
						     NULL,

						     e_categories_value_at,
						     e_categories_set_value_at,
						     e_categories_is_cell_editable,

						     e_categories_has_save_id,
						     e_categories_get_save_id,

						     e_categories_duplicate_value,
						     e_categories_free_value,
						     e_categories_initialize_value,
						     e_categories_value_is_empty,
						     e_categories_value_to_string,
						     categories);

	e_table = e_table_scrolled_new (categories->priv->model, NULL, INITIAL_SPEC, NULL);

	categories->priv->table = e_table_scrolled_get_table(E_TABLE_SCROLLED(e_table));
	gtk_signal_connect(GTK_OBJECT(categories->priv->table), "key_press",
			   GTK_SIGNAL_FUNC(table_key_press), categories);
	gtk_signal_connect(GTK_OBJECT(categories->priv->table), "right_click",
			   GTK_SIGNAL_FUNC(table_right_click), categories);

	gtk_object_sink(GTK_OBJECT(categories->priv->model));
	
	gtk_widget_show(e_table);

	gtk_table_attach_defaults(GTK_TABLE(table),
				  e_table, 
				  0, 1,
				  3, 4);

	gtk_widget_grab_focus (categories->priv->entry);
	gnome_dialog_editable_enters (GNOME_DIALOG (categories), GTK_EDITABLE (categories->priv->entry));

	ecml = e_categories_master_list_array_new();
	gtk_object_set (GTK_OBJECT (categories),
			"ecml", ecml,
			NULL);
	gtk_object_unref(GTK_OBJECT (ecml));
}

static void
e_categories_destroy (GtkObject *object)
{
	ECategories *categories = E_CATEGORIES(object);
	int i;

	if (categories->priv->gui)
		gtk_object_unref(GTK_OBJECT(categories->priv->gui));

	g_free(categories->priv->categories);
	for (i = 0; i < categories->priv->list_length; i++)
		g_free(categories->priv->category_list[i]);

	if (categories->priv->ecml) {
		if (categories->priv->ecml_changed_id)
			gtk_signal_disconnect (GTK_OBJECT(categories->priv->ecml),
					       categories->priv->ecml_changed_id);
		gtk_object_unref (GTK_OBJECT (categories->priv->ecml));
	}

	e_categories_release_ecmld (categories);

	g_free(categories->priv->category_list);
	g_free(categories->priv->selected_list);
	g_free(categories->priv);
	categories->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * e_categories_construct:
 * @categories: An uninitialized %ECategories widget.
 * @initial_category_list: Comma-separated list of initial categories.
 * 
 * Construct the @categories object.
 **/
void
e_categories_construct (ECategories *categories,
			const char *initial_category_list)
{
	g_return_if_fail (categories != NULL);
	g_return_if_fail (E_IS_CATEGORIES (categories));
	g_return_if_fail (initial_category_list != NULL);

	ec_set_categories (categories->priv->entry, initial_category_list);

	gtk_window_set_default_size (GTK_WINDOW (categories), 200, 400);
}

/**
 * e_categories_new:
 * @initial_category_list: Comma-separated list of initial categories.
 * 
 * Create a new %ECategories widget.
 * 
 * Return value: A pointer to the newly created %ECategories widget.
 **/
GtkWidget*
e_categories_new (const char *initial_category_list)
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gtk_type_new (e_categories_get_type ()));

	e_categories_construct (E_CATEGORIES (widget), initial_category_list);

	return widget;
}

static void
e_categories_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategories *e_categories;

	e_categories = E_CATEGORIES (o);
	
	switch (arg_id){
	case ARG_CATEGORIES:
		ec_set_categories (e_categories->priv->entry, GTK_VALUE_STRING (*arg));
		break;

	case ARG_HEADER:
		ec_set_header (e_categories, GTK_VALUE_STRING (*arg));
		break;

	case ARG_ECML:
		ec_set_ecml (e_categories, (ECategoriesMasterList *) GTK_VALUE_OBJECT (*arg));
		break;
	}
}

static void
e_categories_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	ECategories *e_categories;
	GtkWidget *widget;
	char *header;

	e_categories = E_CATEGORIES (object);

	switch (arg_id) {
	case ARG_CATEGORIES:
		GTK_VALUE_STRING (*arg) = g_strdup(e_categories->priv->categories);
		break;

	case ARG_HEADER:
		widget = glade_xml_get_widget(e_categories->priv->gui, "label-header");
		if (widget && GTK_IS_LABEL(widget)) {
			gtk_object_get(GTK_OBJECT(widget),
				       "label", &header,
				       NULL);
			GTK_VALUE_STRING (*arg) = header;
		} else
			GTK_VALUE_STRING (*arg) = NULL;
		break;

	case ARG_ECML:
		GTK_VALUE_OBJECT (*arg) = (GtkObject *) e_categories->priv->ecml;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

/* This function returns the number of columns in our ETableModel. */
static int
e_categories_col_count (ETableModel *etc, gpointer data)
{
	return 2;
}

/* This function returns the number of rows in our ETableModel. */
static int
e_categories_row_count (ETableModel *etc, gpointer data)
{
	ECategories *categories = E_CATEGORIES(data);
	return categories->priv->list_length;
}

/* This function returns the value at a particular point in our ETableModel. */
static void *
e_categories_value_at (ETableModel *etc, int col, int row, gpointer data)
{
	ECategories *categories = E_CATEGORIES(data);
	if (col == 0)
		return GINT_TO_POINTER (categories->priv->selected_list[row]);
	else
		return categories->priv->category_list[row];
}

static void
e_categories_rebuild (ECategories *categories)
{
	char **strs;
	int i, j;
	char *string;

	strs = g_new(char *, categories->priv->list_length + 1);
	for (i = 0, j = 0; i < categories->priv->list_length; i++) {
		if (categories->priv->selected_list[i])
			strs[j++] = categories->priv->category_list[i];
	}
	strs[j] = 0;
	string = g_strjoinv(", ", strs);
	e_utf8_gtk_entry_set_text(GTK_ENTRY(categories->priv->entry), string);
	g_free(string);
	g_free(strs);
}

static void
e_categories_toggle (ECategories *categories, int row)
{
	categories->priv->selected_list[row] = !categories->priv->selected_list[row];
	e_categories_rebuild (categories);
}

/* This function sets the value at a particular point in our ETableModel. */
static void
e_categories_set_value_at (ETableModel *etc, int col, int row, const void *val, gpointer data)
{
	ECategories *categories = E_CATEGORIES(data);
	if ( col == 0 ) {
		categories->priv->selected_list[row] =
			GPOINTER_TO_INT (val);
		e_categories_rebuild (categories);
	}
	if ( col == 1 )
		return;
}

/* This function returns whether a particular cell is editable. */
static gboolean
e_categories_is_cell_editable (ETableModel *etc, int col, int row, gpointer data)
{
	return col == 0;
}

static gboolean
e_categories_has_save_id (ETableModel *etc, gpointer data)
{
	return TRUE;
}

static char *
e_categories_get_save_id (ETableModel *etc, int row, gpointer data)
{
	ECategories *categories = E_CATEGORIES(data);

	return strdup (categories->priv->category_list[row]);
}

/* This function duplicates the value passed to it. */
static void *
e_categories_duplicate_value (ETableModel *etc, int col, const void *value, gpointer data)
{
	if (col == 0)
		return (void *)value;
	else
		return g_strdup(value);
}

/* This function frees the value passed to it. */
static void
e_categories_free_value (ETableModel *etc, int col, void *value, gpointer data)
{
	if (col == 0)
		return;
	else
		g_free(value);
}

static void *
e_categories_initialize_value (ETableModel *etc, int col, gpointer data)
{
	if (col == 0)
		return NULL;
	else
		return g_strdup("");
}

static gboolean
e_categories_value_is_empty (ETableModel *etc, int col, const void *value, gpointer data)
{
	if (col == 0)
		return value == NULL;
	else
		return !(value && *(char *)value);
}

static char *
e_categories_value_to_string (ETableModel *etc, int col, const void *value, gpointer data)
{
	if (col == 0)
		return g_strdup_printf("%d", GPOINTER_TO_INT (value));
	else
		return g_strdup(value);
}
