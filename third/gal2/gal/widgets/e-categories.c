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
#include <string.h>

#include "e-categories.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkbox.h>

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
};

static void e_categories_init		(ECategories		 *card);
static void e_categories_class_init	(ECategoriesClass	 *klass);
static void e_categories_set_property    (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void e_categories_get_property    (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void e_categories_dispose         (GObject *object);
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
static void e_categories_rebuild (ECategories *categories);
static void focus_current_etable_item (ETableGroup *group, int row);

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class = NULL;

/* The arguments we take */
enum {
	PROP_0,
	PROP_CATEGORIES,
	PROP_HEADER,
	PROP_ECML
};

E_MAKE_TYPE (e_categories,
	     "ECategories",
	     ECategories,
	     e_categories_class_init,
	     e_categories_init,
	     PARENT_TYPE)

static void
e_categories_class_init (ECategoriesClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_ref (PARENT_TYPE);
 
	object_class->set_property = e_categories_set_property;
	object_class->get_property = e_categories_get_property;
	object_class->dispose = e_categories_dispose;

	g_object_class_install_property (object_class, PROP_CATEGORIES,
					 g_param_spec_string ("categories",
							      _( "Categories" ),
							      _( "Categories" ),
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_HEADER,
					 g_param_spec_string ("header",
							      _( "Header" ),
							      _( "Header" ),
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class, PROP_ECML,
					 g_param_spec_object ("ecml",
							      "ECML",
							      "ECategoriesMasterListCombo",
							      E_CATEGORIES_MASTER_LIST_TYPE,
							      G_PARAM_READWRITE));
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
e_categories_entry_activate (GtkWidget *entry, 
			     ECategories *categories)
{
	gtk_dialog_response (GTK_DIALOG (categories), GTK_RESPONSE_OK);
}

static void
e_categories_release_ecmld (ECategories *categories)
{
	if (categories->priv->ecmld) {
		g_object_unref (categories->priv->ecmld);
		categories->priv->ecmld = NULL;
	}
}

static void
ecmld_destroyed (gpointer data, GObject *where_object_was)
{
	ECategories *categories = data;
	categories->priv->ecmld = NULL;
}

static void
e_categories_button_clicked (GtkWidget *button,
			     ECategories *categories)
{
	if (categories->priv->ecmld)
		e_categories_master_list_dialog_raise (categories->priv->ecmld);
	else {
		categories->priv->ecmld = e_categories_master_list_dialog_new (categories->priv->ecml);
		g_object_weak_ref (G_OBJECT (categories->priv->ecmld),
				   ecmld_destroyed, categories);
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
			g_signal_handler_disconnect (categories->priv->ecml,
						     categories->priv->ecml_changed_id);
		g_object_unref (categories->priv->ecml);
	}
	categories->priv->ecml = ecml;
	if (categories->priv->ecml) {
		g_object_ref (categories->priv->ecml);
		categories->priv->ecml_changed_id =
			g_signal_connect (categories->priv->ecml,
					  "changed",
					  G_CALLBACK (ecml_changed),
					  categories);
	}
	if (categories->priv->ecmld) {
		g_object_set (categories->priv->ecmld,
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


#define INITIAL_SPEC "<ETableSpecification no-headers=\"true\" draw-grid=\"true\" cursor-mode=\"line\" gettext-domain=\"" E_I18N_DOMAIN "\">\
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
	E_POPUP_ITEM (N_("Add to global category list"), GTK_SIGNAL_FUNC (add_category), 4 + 1),
	E_POPUP_ITEM (N_("Add all to global category list"), GTK_SIGNAL_FUNC (add_category), 8 + 1),
	E_POPUP_ITEM (N_("Remove from global category list"), GTK_SIGNAL_FUNC (remove_category), 4 + 2),
	E_POPUP_ITEM (N_("Remove all from global category list"), GTK_SIGNAL_FUNC (remove_category), 8 + 2),

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

static gint
table_click (ETable *table, int row, int col, GdkEvent *event, ECategories *categories)
{
	if (col == 1) {
		categories->priv->selected_list[row] = !categories->priv->selected_list[row];
		e_categories_rebuild (categories);
		focus_current_etable_item (E_TABLE_GROUP(categories->priv->table->group), row);
	}
	return TRUE;
}

static void
e_categories_init (ECategories *categories)
{
	categories->priv = g_new(ECategoriesPriv, 1);

	categories->priv->gui = NULL;
	categories->priv->ecml = NULL;

	categories->priv->list_length = 0;
	categories->priv->category_list = NULL;
	categories->priv->selected_list = NULL;
	categories->priv->categories = g_strdup ("");
	categories->priv->ecmld = NULL;
}

static void
e_categories_dispose (GObject *object)
{
	ECategories *categories = E_CATEGORIES(object);
	int i;

	if (categories->priv) {
		if (categories->priv->gui)
			g_object_unref (categories->priv->gui);

		g_free(categories->priv->categories);
		for (i = 0; i < categories->priv->list_length; i++)
			g_free(categories->priv->category_list[i]);

		if (categories->priv->ecml) {
			if (categories->priv->ecml_changed_id)
				g_signal_handler_disconnect (categories->priv->ecml,
							     categories->priv->ecml_changed_id);
			g_object_unref (categories->priv->ecml);
		}

		e_categories_release_ecmld (categories);

		g_free(categories->priv->category_list);
		g_free(categories->priv->selected_list);
		g_free(categories->priv);
		categories->priv = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->dispose)
		(* G_OBJECT_CLASS (parent_class)->dispose) (object);
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
	GladeXML *gui;
	GtkWidget *table;
	GtkWidget *e_table;
	GtkWidget *button;
	ECategoriesMasterList *ecml;

	g_return_if_fail (categories != NULL);
	g_return_if_fail (E_IS_CATEGORIES (categories));
	g_return_if_fail (initial_category_list != NULL);

	gui = glade_xml_new_with_domain (GAL_GLADEDIR "/gal-categories.glade", NULL, E_I18N_DOMAIN);

	gtk_window_set_title (GTK_WINDOW (categories), _("Edit Categories"));

	if (gui) {
		categories->priv->gui = gui;

		gtk_widget_realize (GTK_WIDGET (categories));
		gtk_dialog_set_has_separator (GTK_DIALOG (categories), FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (categories)->vbox), 0);
		gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (categories)->action_area), 12);

		gtk_dialog_add_buttons (GTK_DIALOG (categories),
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_OK,
					NULL);

		gtk_dialog_set_default_response (GTK_DIALOG (categories),
						 GTK_RESPONSE_OK);

		gtk_window_set_policy(GTK_WINDOW(categories), FALSE, TRUE, FALSE);

		table = glade_xml_get_widget(gui, "table-categories");
		gtk_widget_ref(table);
		gtk_container_remove(GTK_CONTAINER(table->parent), table);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (categories)->vbox), table, TRUE, TRUE, 6);
		gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (categories)->vbox), 6);

		gtk_widget_unref(table);

		categories->priv->entry = glade_xml_get_widget(gui, "entry-categories");
		
		g_signal_connect(categories->priv->entry, "changed",
				 G_CALLBACK(e_categories_entry_change), categories);

		if (GTK_IS_ENTRY (categories->priv->entry))
			g_signal_connect(categories->priv->entry, "activate",
					 G_CALLBACK(e_categories_entry_activate), categories);

		button = glade_xml_get_widget(gui, "button-ecmld");
		
		g_signal_connect(button, "clicked",
				 G_CALLBACK(e_categories_button_clicked), categories);

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
		g_signal_connect (categories->priv->table, "key_press",
				  G_CALLBACK(table_key_press), categories);
		g_signal_connect (categories->priv->table, "right_click",
				  G_CALLBACK(table_right_click), categories);
		g_signal_connect (categories->priv->table, "click",
				  G_CALLBACK(table_click), categories);
		
		gtk_widget_show(e_table);

		gtk_table_attach_defaults(GTK_TABLE(table),
					  e_table, 
					  0, 1,
					  3, 4);
		gtk_widget_grab_focus (categories->priv->entry);

		ecml = e_categories_master_list_array_new();
		g_object_set (categories,
			      "ecml", ecml,
			      NULL);
		g_object_unref (ecml);

		ec_set_categories (categories->priv->entry, initial_category_list);

		gtk_window_set_default_size (GTK_WINDOW (categories), 320, 400);
	}
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
	ECategories *ecat;

	ecat = E_CATEGORIES (g_object_new (E_CATEGORIES_TYPE, NULL));

	e_categories_construct (ecat, initial_category_list);

	if (ecat->priv->gui == NULL) {
		g_object_unref (ecat);
		return NULL;
	}

	return GTK_WIDGET (ecat);
}

static void
e_categories_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	ECategories *e_categories;

	e_categories = E_CATEGORIES (object);
	
	switch (prop_id){
	case PROP_CATEGORIES:
		ec_set_categories (e_categories->priv->entry, g_value_get_string (value));
		break;

	case PROP_HEADER:
		ec_set_header (e_categories, g_value_get_string (value));
		break;

	case PROP_ECML:
		ec_set_ecml (e_categories, (ECategoriesMasterList *) g_value_get_object (value));
		break;
	}
}

static void
e_categories_get_property    (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	ECategories *e_categories;
	GtkWidget *widget;

	e_categories = E_CATEGORIES (object);

	switch (prop_id) {
	case PROP_CATEGORIES:
		g_value_set_string (value, g_strdup(e_categories->priv->categories));
		break;

	case PROP_HEADER:
		widget = glade_xml_get_widget(e_categories->priv->gui, "label-header");
		if (widget && GTK_IS_LABEL(widget)) {
			g_object_get_property (G_OBJECT (widget), "label", value);
		} else
			g_value_set_string (value, NULL);
		break;

	case PROP_ECML:
		g_value_set_object (value, e_categories->priv->ecml);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
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

/* Finds the current descendant of the group that is an ETableItem and focuses it */
static void
focus_current_etable_item (ETableGroup *group, int row)
{
	GnomeCanvasGroup *cgroup;
	GList *l;

	cgroup = GNOME_CANVAS_GROUP (group);

	for (l = cgroup->item_list; l; l = l->next) {
		GnomeCanvasItem *i;

		i = GNOME_CANVAS_ITEM (l->data);

		if (E_IS_TABLE_GROUP (i))
			focus_current_etable_item (E_TABLE_GROUP (i), row);
		else if (E_IS_TABLE_ITEM (i)) {
			e_table_item_set_cursor (E_TABLE_ITEM (i), 0, row);
			gnome_canvas_item_grab_focus (i);
		}
	}
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
	focus_current_etable_item (E_TABLE_GROUP(categories->priv->table->group), row);
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
		focus_current_etable_item (E_TABLE_GROUP(categories->priv->table->group), row);
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
	return FALSE;
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
