/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include "gconf-editor-window.h"

#include "gconf-bookmarks.h"
#include "gconf-bookmarks-dialog.h"
#include "gconf-list-model.h"
#include "gconf-tree-model.h"
#include "gconf-cell-renderer.h"
#include "gconf-editor-application.h"
#include "gconf-key-editor.h"
#include "gconf-stock-icons.h"
#include "gconf-util.h"
#include <gconf/gconf.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkclipboard.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstatusbar.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktable.h>
#include <gtk/gtktextview.h>
#include <gtk/gtktreemodelsort.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtklabel.h>
#include <libintl.h>
#include <string.h>

#define _(x) gettext (x)
#define N_(x) (x)

static GObjectClass *parent_class;

static char *
gconf_editor_window_item_factory_translate_func (const char *path, gpointer func_data)
{
	return _(path);
}

static void
gconf_editor_window_close_window (gpointer callback_data, guint action, GtkWidget *widget)
{
	GtkWidget *window = callback_data;

	gtk_object_destroy (GTK_OBJECT (window));
}

static void
gconf_editor_window_copy_key_name (gpointer callback_data, guint action, GtkWidget *widget)
{
	GConfEditorWindow *window = GCONF_EDITOR_WINDOW (callback_data);
	char *path;
	GtkTreeIter iter;
	GtkTreeIter child_iter;
	
	gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view)), NULL, &iter);

	gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (window->sorted_tree_model), &child_iter, &iter);
	path = gconf_tree_model_get_gconf_path (GCONF_TREE_MODEL (window->tree_model), &child_iter);

	gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), path, -1);

	g_free (path);
}

static void
gconf_editor_window_add_bookmark (gpointer callback_data, guint action, GtkWidget *widget)
{
	GConfEditorWindow *window = GCONF_EDITOR_WINDOW (callback_data);
	GtkTreeIter iter;
	char *path;
	
	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view)), NULL, &iter)) {
		return;
	}
	else {
		GtkTreeIter child_iter;

		gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (window->sorted_tree_model), &child_iter, &iter);
		path = gconf_tree_model_get_gconf_path (GCONF_TREE_MODEL (window->tree_model), &child_iter);
	}

	gconf_bookmarks_add_bookmark (path);
	
	g_free (path);
}

static void
gconf_editor_window_edit_bookmarks (gpointer callback_data, guint action, GtkWidget *widget)
{
	static GtkWidget *dialog = NULL;

	if (dialog != NULL) {
		gtk_window_present (GTK_WINDOW (dialog));
		return;
	}
	
	dialog = gconf_bookmarks_dialog_new (GTK_WINDOW (callback_data));
	g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer *)&dialog);
	gtk_widget_show (dialog);
}

static void
gconf_editor_window_new_window (gpointer callback_data, guint action, GtkWidget *widget)
{
	GtkWidget *new_window;

	new_window = gconf_editor_application_create_editor_window ();
	gtk_widget_show_all (new_window);
}

static void
gconf_editor_window_about_window (gpointer callback_data, guint action, GtkWidget *widget)
{
	GtkWidget *about_window;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *image;
	GdkPixbuf *icon;
	
	gchar *markup;
	
	about_window = gtk_dialog_new ();
	gtk_dialog_add_button (GTK_DIALOG (about_window),
			       GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (about_window),
					 GTK_RESPONSE_OK);
	
	gtk_window_set_title (GTK_WINDOW (about_window), _("About GConf-Editor"));
	icon = gdk_pixbuf_new_from_file (IMAGEDIR"/gconf-editor.png", NULL);
	if (icon != NULL) {
		gtk_window_set_icon (GTK_WINDOW (about_window), icon);
		g_object_unref (icon);
	}
	
	gtk_window_set_resizable (GTK_WINDOW (about_window), FALSE);
	gtk_window_set_position (GTK_WINDOW (about_window), 
				 GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_type_hint (GTK_WINDOW (about_window), 
				  GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_transient_for (GTK_WINDOW (about_window), 
				      GTK_WINDOW (callback_data));

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about_window)->vbox), vbox, FALSE, FALSE, 0);

	image = gtk_image_new_from_file (IMAGEDIR"/gconf-editor.png");
	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);

	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	markup = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">%s "VERSION"</span>\n\n"
				  "%s\n\n"
				  "<span size=\"small\">%s</span>\n",
				  _("GConf-Editor"),
				  _("An editor for the GConf configuration system."),
				  _("Copyright (C) Anders Carlsson 2001, 2002"));
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	
	gtk_widget_show_all (about_window);
	gtk_dialog_run (GTK_DIALOG (about_window));
	gtk_widget_destroy (about_window);
}
	
				
static GtkItemFactoryEntry menu_items[] =
{
	{ N_("/_File"),                        NULL,         0,                                   0, "<Branch>" },
	{ N_("/File/tearoff1"),                NULL,         NULL,                                0, "<Tearoff>" },
	{ N_("/File/_New window"),             "<control>N", gconf_editor_window_new_window,      0, "<StockItem>", GTK_STOCK_NEW },
	{ N_("/File/_Close window"),           "<control>W", gconf_editor_window_close_window,    0, "<StockItem>", GTK_STOCK_CLOSE },
	{ N_("/File/sep1"),                    NULL,         0,                                   0, "<Separator>" },
	{ N_("/File/_Quit"),                   "<control>Q", gtk_main_quit,                       0, "<StockItem>", GTK_STOCK_QUIT },
	{ N_("/_Edit"),                        NULL,         0,                                   0, "<Branch>" },
	{ N_("/Edit/tearoff2"),                NULL,         NULL,                                0, "<Tearoff>" },
	{ N_("/Edit/_Copy key name"),          NULL,         gconf_editor_window_copy_key_name,   0, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/_Bookmarks"),                   NULL,         0,                                   0, "<Branch>", },
	{ N_("/Bookmarks/tearoff3"),           NULL,         NULL,                                0, "<Tearoff>" },
	{ N_("/Bookmarks/_Add bookmark"),      NULL,         gconf_editor_window_add_bookmark,    0, "<Item>", },
	{ N_("/Bookmarks/_Edit bookmarks..."), NULL,         gconf_editor_window_edit_bookmarks,  0, "<Item>", },
	{ N_("/_Help"),                        NULL,         0,                                   0, "<Branch>" },
	{ N_("/Help/tearoff4"),                NULL,         NULL,                                0, "<Tearoff>" },
	{ N_("/Help/_About..."),               NULL,         gconf_editor_window_about_window,    0, "<StockItem>", GCONF_STOCK_ABOUT },
};

const char *tearoff_paths [] = {
	N_("/File/tearoff1"),
	N_("/Edit/tearoff2"),
	N_("/Bookmarks/tearoff3"),
	N_("/Help/tearoff4")
};

const char *image_menu_items_paths [] = {
	N_("/File/New window"),
	N_("/File/Close window"),
	N_("/File/Quit"),
	N_("/Edit/Copy key name"),
	N_("/Help/About...")
};

static void
gconf_editor_window_popup_error_dialog (GtkWindow   *parent,
					const gchar *message,
					GError      *error)
{
	GtkWidget *dialog;

	g_return_if_fail (error != NULL);

	dialog = gtk_message_dialog_new (parent,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 message,
					 error->message);
	g_error_free (error);

	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_widget_show (dialog);
}

static void
gconf_editor_popup_window_unset_key (gpointer callback_data, guint action, GtkWidget *widget)
{
	GConfEditorWindow *window = GCONF_EDITOR_WINDOW (callback_data);
	GtkTreeIter iter;
	gchar *key;
	GError *error = NULL;
	
	gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->list_view)),
					 NULL, &iter);
	gtk_tree_model_get (window->sorted_list_model, &iter,
			    GCONF_LIST_MODEL_KEY_PATH_COLUMN, &key,
			    -1);

	gconf_client_unset (gconf_client_get_default (), key, &error);
	g_free (key);

	if (error != NULL) {
		gconf_editor_window_popup_error_dialog (GTK_WINDOW (window),
							_("Couldn't unset key. Error was:\n%s"), error);
	}
}

static void
gconf_editor_new_key_response (GtkDialog *editor,
			       int        response)
{
	if (response == GTK_RESPONSE_OK) {
		GConfValue *value;
		char       *full_path;
		char       *why_not_valid = NULL;
		
		full_path = gconf_key_editor_get_full_key_path (GCONF_KEY_EDITOR (editor));
		
		if (!gconf_valid_key (full_path, &why_not_valid)) {
			GtkWidget *message_dialog;

			g_assert (why_not_valid != NULL);

			message_dialog = gtk_message_dialog_new (GTK_WINDOW (editor),
								 GTK_DIALOG_MODAL,
								 GTK_MESSAGE_ERROR,
								 GTK_BUTTONS_OK,
								 _("Could not create key. The error is:\n%s"),
								 why_not_valid);
			gtk_dialog_set_default_response (GTK_DIALOG (message_dialog), GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (message_dialog));
			gtk_widget_destroy (message_dialog);

			g_free (full_path);
			g_free (why_not_valid);

			/* leave the key editor in place */
			return;
		}
			
		/* Create the key */
		value = gconf_key_editor_get_value (GCONF_KEY_EDITOR (editor));

		gconf_client_set (gconf_client_get_default (), full_path, value, NULL);

		gconf_value_free (value);
		g_free (full_path);
	}

	gtk_widget_destroy (GTK_WIDGET (editor));
}

static void
gconf_editor_popup_window_new_key (gpointer callback_data, guint action, GtkWidget *widget)
{
	GConfEditorWindow *window = GCONF_EDITOR_WINDOW (callback_data);
	GtkTreeIter iter;
	GtkWidget *editor;
	char *path;
	
	editor = gconf_key_editor_new (GCONF_KEY_EDITOR_NEW_KEY);

	if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view)), NULL, &iter)) {
		GtkTreeIter child_iter;

		gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (window->sorted_tree_model), &child_iter, &iter);
		path = gconf_tree_model_get_gconf_path (GCONF_TREE_MODEL (window->tree_model), &child_iter);
	}
	else {
		path = g_strdup ("/");
	}
	
	gconf_key_editor_set_key_path (GCONF_KEY_EDITOR (editor), path);
	g_free (path);

	g_signal_connect (editor, "response",
			  G_CALLBACK (gconf_editor_new_key_response), NULL);

	gtk_widget_show (editor);
}

static void
gconf_editor_edit_key_response (GtkDialog *editor,
				int        response)
{
	if (response == GTK_RESPONSE_OK) {
		GConfValue *value;
		GError     *error = NULL;
		const char *path;

		value = gconf_key_editor_get_value (GCONF_KEY_EDITOR (editor));

		path = gconf_key_editor_get_key_name (GCONF_KEY_EDITOR (editor));
		g_assert (gconf_valid_key (path, NULL));

		gconf_client_set (gconf_client_get_default (),
				  path, value, &error);

		gconf_value_free (value);

		if (error != NULL) {
			gconf_editor_window_popup_error_dialog (GTK_WINDOW (editor),
								_("Could not change key value. Error message:\n%s"),
								error);
			g_error_free (error);
			return;
		}
	}

	gtk_widget_destroy (GTK_WIDGET (editor));
}

static void
gconf_editor_popup_window_edit_key (gpointer callback_data, guint action, GtkWidget *widget)
{
	GConfEditorWindow *window = GCONF_EDITOR_WINDOW (callback_data);
	GtkTreeIter iter;
	GtkWidget *editor, *dialog;
	GConfValue *value; 
	char *path = NULL;
	
	gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->list_view)),
					 NULL, &iter);
	gtk_tree_model_get (window->sorted_list_model, &iter,
			    GCONF_LIST_MODEL_KEY_PATH_COLUMN, &path,
			    GCONF_LIST_MODEL_VALUE_COLUMN, &value,
			    -1);

	if (value && (value->type == GCONF_VALUE_SCHEMA ||
		      value->type == GCONF_VALUE_PAIR)) {
		dialog = gtk_message_dialog_new (GTK_WINDOW (window), 0,
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_OK,
						 _("Currently pairs and schemas can't "
						   "be edited. This will be change in a later "
						   "version."));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		gconf_value_free (value);
		g_free (path);
		return;
	}
	
	editor = gconf_key_editor_new (GCONF_KEY_EDITOR_EDIT_KEY);

	gconf_key_editor_set_value (GCONF_KEY_EDITOR (editor), value);
	gconf_value_free (value);

	gconf_key_editor_set_key_name (GCONF_KEY_EDITOR (editor), path);
	g_free (path);

	g_signal_connect (editor, "response",
			  G_CALLBACK (gconf_editor_edit_key_response), NULL);

	gtk_widget_show (editor);
}

static void
gconf_editor_window_list_view_row_activated (GtkTreeView       *tree_view,
					     GtkTreePath       *path,
					     GtkTreeViewColumn *column,
					     GConfEditorWindow *window)
{
	gconf_editor_popup_window_edit_key (window, 0, NULL);
}

static GtkItemFactoryEntry popup_menu_items[] = {
	{ N_("/_New key..."), NULL, gconf_editor_popup_window_new_key,   0, "<StockItem>", GTK_STOCK_NEW },
	{ N_("/Edit key..."), NULL, gconf_editor_popup_window_edit_key,  1, "<Item>", NULL }, 
	{ N_("/sep1"),        NULL, 0,                                   0, "<Separator>" },
	{ N_("/_Unset key"),  NULL, gconf_editor_popup_window_unset_key, 2, "<StockItem>", GTK_STOCK_DELETE },
};

const char *image_popup_items_paths [] = {
	N_("/New key..."),
	N_("/Unset key"),
};

static void
gconf_editor_window_selection_changed (GtkTreeSelection *selection, GConfEditorWindow *window)
{
	GtkTreeIter iter;
	
	if (selection == NULL)
		gtk_window_set_title (GTK_WINDOW (window), _("GConf editor"));
	else {
		gchar *name, *title, *path;
		GtkTreeIter child_iter;

		if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
			gtk_window_set_title (GTK_WINDOW (window),
					      _("GConf editor"));
			return;
		}

		gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (window->sorted_tree_model), &child_iter, &iter);

		name = gconf_tree_model_get_gconf_name (GCONF_TREE_MODEL (window->tree_model), &child_iter);

		title = g_strdup_printf (_("GConf editor - %s"), name);
		
		gtk_window_set_title (GTK_WINDOW (window), title);
		g_free (title);
		g_free (name);

		path = gconf_tree_model_get_gconf_path (GCONF_TREE_MODEL (window->tree_model), &child_iter);
		gtk_statusbar_pop (GTK_STATUSBAR (window->statusbar), 0);
		gtk_statusbar_push (GTK_STATUSBAR (window->statusbar), 0, path);

		gconf_list_model_set_root_path (GCONF_LIST_MODEL (window->list_model), path);

		g_free (path);

	}
}

static gboolean
list_view_button_press_event (GtkTreeView *tree_view, GdkEventButton *event, GConfEditorWindow *window)
{
	GtkTreePath *path;

	if (event->button == 3) {
		gtk_widget_grab_focus (GTK_WIDGET (tree_view));

		/* Select our row */
		if (gtk_tree_view_get_path_at_pos (tree_view, event->x, event->y, &path, NULL, NULL, NULL)) {
			gtk_tree_selection_select_path (gtk_tree_view_get_selection (tree_view), path);

			gtk_widget_set_sensitive (gtk_item_factory_get_widget_by_action (window->popup_factory, 1), TRUE);
			gtk_widget_set_sensitive (gtk_item_factory_get_widget_by_action (window->popup_factory, 2), TRUE);
			
			gtk_tree_path_free (path);
		}
		else {
			gtk_widget_set_sensitive (gtk_item_factory_get_widget_by_action (window->popup_factory, 1), FALSE);
			gtk_widget_set_sensitive (gtk_item_factory_get_widget_by_action (window->popup_factory, 2), FALSE);
		}
		
		gtk_item_factory_popup (window->popup_factory, event->x_root, event->y_root,
					event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static void
gconf_editor_gconf_value_changed (GConfCellRenderer *cell, const gchar *path_str, GConfValue *new_value, GConfEditorWindow *window)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gchar *key;
	
	path = gtk_tree_path_new_from_string (path_str);

	gtk_tree_model_get_iter (window->sorted_list_model, &iter, path);

	gtk_tree_model_get (window->sorted_list_model, &iter,
			    GCONF_LIST_MODEL_KEY_PATH_COLUMN, &key,
			    -1);

	gconf_client_set (gconf_client_get_default (), key, new_value, NULL);

	g_free (key);
}

static void
gconf_editor_window_list_view_popup_menu (GtkWidget *widget, GConfEditorWindow *window)
{
	int x, y;

	gdk_window_get_origin (widget->window, &x, &y);
	
	gtk_item_factory_popup (window->popup_factory, x, y,
				0, gtk_get_current_event_time ());
			
}

static char *
strip_whitespace (const char *text)
{
	const char *p;
	const char *end;
	GString *str;

	p = text;
	end = text + strlen (text);

	/* First skip the leading whitespace */
	while (p != end && g_unichar_isspace (g_utf8_get_char (p))) {
	  p = g_utf8_next_char (p);
	}
	  
	str = g_string_new (NULL);
	
	while (p != end) {
		gunichar ch;

		ch = g_utf8_get_char (p);

		if (g_unichar_isspace (ch)) {
			while (p != end && g_unichar_isspace (ch)) {
				p = g_utf8_next_char (p);
				ch = g_utf8_get_char (p);
			}

			p = g_utf8_prev_char (p);
			g_string_append_unichar (str, ' ');
		}
		else {
			g_string_append_unichar (str, ch);
		}

		p = g_utf8_next_char (p);
	}

	return str->str;
}

static void
set_label_and_strip_whitespace (GtkLabel *label, const char *text)
{
	char *stripped_text;

	stripped_text = strip_whitespace (text);
	gtk_label_set_text (GTK_LABEL (label), stripped_text);
	g_free (stripped_text);
}

static void
gconf_editor_window_update_list_selection (GtkTreeSelection *selection, GConfEditorWindow *window)
{
	GtkTreeIter iter;
	GConfSchema *schema;
	char *path;
	
	if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_label_set_text (GTK_LABEL (window->key_name_label), _("(None)"));
		gtk_label_set_text (GTK_LABEL (window->owner_label), _("(None)"));
		gtk_label_set_text (GTK_LABEL (window->short_desc_label), _("(None)"));
		gtk_text_buffer_set_text (window->long_desc_buffer, _("(None)"), -1);
		
		return;
	}
	
	gtk_tree_model_get (window->sorted_list_model, &iter,
			    GCONF_LIST_MODEL_KEY_PATH_COLUMN, &path,
			    -1);
	
	gtk_label_set_text (GTK_LABEL (window->key_name_label), path);

	schema = gconf_client_get_schema_for_key (gconf_client_get_default (), path);

	if (schema != NULL && gconf_schema_get_long_desc (schema) != NULL) {
		char *long_desc;

		long_desc = strip_whitespace (gconf_schema_get_long_desc (schema));
		
		gtk_text_buffer_set_text (window->long_desc_buffer, long_desc, -1);
		g_free (long_desc);
	}
	else {
		gtk_text_buffer_set_text (window->long_desc_buffer, _("(None)"), -1);
	}

	if (schema != NULL && gconf_schema_get_short_desc (schema) != NULL) {
		set_label_and_strip_whitespace (GTK_LABEL (window->short_desc_label),
						gconf_schema_get_short_desc (schema));
	}
	else {
		gtk_label_set_text (GTK_LABEL (window->short_desc_label), _("(None)"));

	}

	if (schema != NULL && gconf_schema_get_owner (schema) != NULL) {
		set_label_and_strip_whitespace (GTK_LABEL (window->owner_label),
						gconf_schema_get_owner (schema));
	}
	else {
		gtk_label_set_text (GTK_LABEL (window->owner_label), _("(None)"));

	}
		
	g_free (path);
}

static void
gconf_editor_window_set_have_tearoffs (GConfEditorWindow *window,
				       gboolean           have_tearoffs)
{
	int i;

	for (i = 0; i < G_N_ELEMENTS (tearoff_paths); i++) {
		GtkWidget *item;

		item = gtk_item_factory_get_item (window->item_factory, tearoff_paths [i]);

		if (have_tearoffs)
			gtk_widget_show (item);
		else
			gtk_widget_hide (item);
	}
}

static void
gconf_editor_window_have_tearoffs_notify (GConfClient       *client,
					  guint              cnxn_id,
					  GConfEntry        *entry,
					  GConfEditorWindow *window)
{
	gboolean have_tearoffs;

	if (entry->value->type != GCONF_VALUE_BOOL)
		return;

	have_tearoffs = gconf_value_get_bool (entry->value);

	gconf_editor_window_set_have_tearoffs (window, have_tearoffs);
}

static void
gconf_editor_window_set_item_has_icon (GtkItemFactory *item_factory,
				       const char     *path,
				       gboolean        have_icons)
{
	GtkWidget *item;
	GtkWidget *image;

	item = gtk_item_factory_get_item (item_factory, path);

	image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (item));
	if (image && !g_object_get_data (G_OBJECT (item), "gconf-editor-icon"))
		g_object_set_data_full (G_OBJECT (item), "gconf-editor-icon",
					g_object_ref (image), g_object_unref);

	if (!image)
		image = g_object_get_data (G_OBJECT (item), "gconf-editor-icon");

	if (!image)
		return;

	if (have_icons)
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	else
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), NULL);
}

static void
gconf_editor_window_set_have_icons (GConfEditorWindow *window,
				    gboolean           have_icons)
{
	int i;

	for (i = 0; i < G_N_ELEMENTS (image_menu_items_paths); i++)
		gconf_editor_window_set_item_has_icon (window->item_factory, image_menu_items_paths [i], have_icons);

	for (i = 0; i < G_N_ELEMENTS (image_popup_items_paths); i++)
		gconf_editor_window_set_item_has_icon (window->popup_factory, image_popup_items_paths [i], have_icons);
}

static void
gconf_editor_window_have_icons_notify (GConfClient       *client,
				       guint              cnxn_id,
				       GConfEntry        *entry,
				       GConfEditorWindow *window)
{
	gboolean have_icons;

	if (entry->value->type != GCONF_VALUE_BOOL)
		return;

	have_icons = gconf_value_get_bool (entry->value);

	gconf_editor_window_set_have_icons (window, have_icons);
}

static void
gconf_editor_window_setup_ui_prefs_handler (GConfEditorWindow *window)
{
	GConfClient *client;

	client = gconf_client_get_default ();

	gconf_editor_window_set_have_tearoffs (
		window, gconf_client_get_bool (client, "/desktop/gnome/interface/menus_have_tearoff", NULL));

	window->tearoffs_notify_id = gconf_client_notify_add (
						client,
						"/desktop/gnome/interface/menus_have_tearoff",
						(GConfClientNotifyFunc) gconf_editor_window_have_tearoffs_notify,
						window, NULL, NULL);

	gconf_editor_window_set_have_icons (
		window, gconf_client_get_bool (client, "/desktop/gnome/interface/menus_have_icons", NULL));

	window->icons_notify_id = gconf_client_notify_add (
						client,
						"/desktop/gnome/interface/menus_have_icons",
						(GConfClientNotifyFunc) gconf_editor_window_have_icons_notify,
						window, NULL, NULL);

	g_object_unref (client);
}

static void
gconf_editor_window_finalize (GObject *object)
{
        GConfEditorWindow *window = (GConfEditorWindow *) object;
	GConfClient       *client;

	if (window->item_factory)
		g_object_unref (window->item_factory);

	client = gconf_client_get_default ();

	if (window->tearoffs_notify_id)
		gconf_client_notify_remove (client, window->tearoffs_notify_id);

	if (window->icons_notify_id)
		gconf_client_notify_remove (client, window->icons_notify_id);

	g_object_unref (client);

        parent_class->finalize (object);
}

static void
gconf_editor_window_class_init (GConfEditorWindowClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	object_class->finalize = gconf_editor_window_finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
gconf_editor_window_init (GConfEditorWindow *window)
{
	GtkWidget *hpaned, *vpaned, *scrolled_window, *vbox;
	GdkPixbuf *icon;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkAccelGroup *accel_group;      
	GtkItemFactory *item_factory;
	GtkTreePath *path;
	GtkWidget *details_frame, *table, *label, *text_view;
	
	/* Create popup menu */
	window->popup_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", NULL);
	gtk_item_factory_set_translate_func (window->popup_factory,
					     gconf_editor_window_item_factory_translate_func,
					     NULL, NULL);
	
	gtk_item_factory_create_items (window->popup_factory, G_N_ELEMENTS (popup_menu_items),
				       popup_menu_items, window);

	gtk_window_set_title (GTK_WINDOW (window), _("GConf editor"));
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 400);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	/* Create menu bar */
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	g_object_unref (accel_group);
	
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", accel_group);
	gtk_item_factory_set_translate_func (item_factory, gconf_editor_window_item_factory_translate_func, NULL, NULL);

	/* Set up item factory to go away with the window */
	window->item_factory = g_object_ref (item_factory);
	gtk_object_sink (GTK_OBJECT (item_factory));
	
	/* Create menu items */
	gtk_item_factory_create_items (item_factory, G_N_ELEMENTS (menu_items),
				       menu_items, window);
	gtk_box_pack_start (GTK_BOX (vbox), gtk_item_factory_get_widget (item_factory, "<main>"), FALSE, FALSE, 0);

	/* Hook up bookmarks */
	gconf_bookmarks_hook_up_menu (window, gtk_item_factory_get_widget (item_factory, "/Bookmarks"));

	/* Create content area */
	hpaned = gtk_hpaned_new ();
	gtk_paned_set_position (GTK_PANED (hpaned), 200);
	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);

	/* Create status bar */
	window->statusbar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (vbox), window->statusbar, FALSE, FALSE, 0);
	
	/* Create tree model and tree view */
	window->tree_model = gconf_tree_model_new ();
	window->sorted_tree_model = gtk_tree_model_sort_new_with_model (window->tree_model);
	window->tree_view = gtk_tree_view_new_with_model (window->sorted_tree_model);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (window->tree_view), FALSE);
	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view))), "changed",
			  G_CALLBACK (gconf_editor_window_selection_changed), window);
	g_object_unref (G_OBJECT (window->tree_model));

	path = gtk_tree_path_new_first ();
	gtk_tree_view_expand_row (GTK_TREE_VIEW (window->tree_view), path, FALSE);
	gtk_tree_path_free (path);

	column = gtk_tree_view_column_new ();

	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "pixbuf", GCONF_TREE_MODEL_CLOSED_ICON_COLUMN,
					     "pixbuf_expander_closed", GCONF_TREE_MODEL_CLOSED_ICON_COLUMN,
					     "pixbuf_expander_open", GCONF_TREE_MODEL_OPEN_ICON_COLUMN,
					     NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", GCONF_TREE_MODEL_NAME_COLUMN,
					     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (window->tree_view), column);

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (window->sorted_tree_model), 0, GTK_SORT_ASCENDING);
	
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
					     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), window->tree_view);
	gtk_paned_add1 (GTK_PANED (hpaned), scrolled_window);

	/* Create list model and list view */
	window->list_model = gconf_list_model_new ();
	window->sorted_list_model = gtk_tree_model_sort_new_with_model (window->list_model);
	window->list_view = gtk_tree_view_new_with_model (window->sorted_list_model);
	g_object_unref (G_OBJECT (window->list_model));
	g_object_unref (G_OBJECT (window->sorted_list_model));

	g_signal_connect (window->list_view, "popup_menu",
			  G_CALLBACK (gconf_editor_window_list_view_popup_menu), window);
	g_signal_connect (window->list_view, "row_activated",
			  G_CALLBACK (gconf_editor_window_list_view_row_activated), window);

	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->list_view)), "changed",
			  G_CALLBACK (gconf_editor_window_update_list_selection), window);
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_title (column, _("Name"));

	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "pixbuf", GCONF_LIST_MODEL_ICON_COLUMN,
					     NULL);

	gtk_tree_view_column_set_resizable (column, TRUE);
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", GCONF_LIST_MODEL_KEY_NAME_COLUMN,
					     NULL);
	gtk_tree_view_column_set_sort_column_id (column, GCONF_LIST_MODEL_KEY_NAME_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW (window->list_view), column);

	cell = gconf_cell_renderer_new ();
	g_signal_connect (cell, "changed",
			  G_CALLBACK (gconf_editor_gconf_value_changed), window);
	
	window->value_column = column = gtk_tree_view_column_new_with_attributes (_("Value"),
										  cell,
										  "value", GCONF_LIST_MODEL_VALUE_COLUMN,
										  NULL);
	g_signal_connect (window->list_view, "button_press_event",
			  G_CALLBACK (list_view_button_press_event), window);

	gtk_tree_view_column_set_reorderable (column, TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);

	gtk_tree_view_append_column (GTK_TREE_VIEW (window->list_view), column);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
					     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), window->list_view);

	vpaned = gtk_vpaned_new ();
	gtk_paned_add2 (GTK_PANED (hpaned), vpaned);

	gtk_paned_pack1 (GTK_PANED (vpaned), scrolled_window, TRUE, FALSE);

	/* Create details area */
	details_frame = gtk_frame_new (_("Key Documentation"));
	gtk_paned_pack2 (GTK_PANED (vpaned), details_frame, FALSE, FALSE);

	table = gtk_table_new (2, 4, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_container_add (GTK_CONTAINER (details_frame), table);
	
	label = gtk_label_new (_("Key Name:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 0, 1,
			  GTK_FILL, 0, 0, 0);
	window->key_name_label = gtk_label_new (_("(None)"));
	gtk_label_set_selectable (GTK_LABEL (window->key_name_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (window->key_name_label), 0.0, 0.5);	
	gtk_table_attach (GTK_TABLE (table), window->key_name_label,
			  1, 2, 0, 1,
			  GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Key Owner:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);	
	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 1, 2,
			  GTK_FILL, 0, 0, 0);
	window->owner_label= gtk_label_new (_("(None)"));
	gtk_label_set_selectable (GTK_LABEL (window->owner_label), TRUE);	
	gtk_misc_set_alignment (GTK_MISC (window->owner_label), 0.0, 0.5);		
	gtk_table_attach (GTK_TABLE (table), window->owner_label,
			  1, 2, 1, 2,
			  GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Short Description:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);	
	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 2, 3,
			  GTK_FILL, 0, 0, 0);
	window->short_desc_label= gtk_label_new (_("(None)"));
	gtk_label_set_line_wrap (GTK_LABEL (window->short_desc_label), TRUE);
	gtk_label_set_selectable (GTK_LABEL (window->short_desc_label), TRUE);	
	gtk_misc_set_alignment (GTK_MISC (window->short_desc_label), 0.0, 0.5);		
	gtk_table_attach (GTK_TABLE (table), window->short_desc_label,
			  1, 2, 2, 3,
			  GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Long Description:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.0);	
	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 3, 4,
			  GTK_FILL, GTK_FILL, 0, 0);

	window->long_desc_buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (window->long_desc_buffer, _("(None)"), -1);	
	text_view = gtk_text_view_new_with_buffer (window->long_desc_buffer);
	gtk_widget_modify_base (text_view, GTK_STATE_NORMAL,
				&GTK_WIDGET (window)->style->bg[GTK_STATE_NORMAL]);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
	
	gtk_table_attach (GTK_TABLE (table), scrolled_window, 
			  1, 2, 3, 4,
			  GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	gtk_widget_show_all (vbox);

	/* Set window icon */
	icon = gdk_pixbuf_new_from_file (IMAGEDIR"/gconf-editor.png", NULL);
	if (icon != NULL) {
		gtk_window_set_icon (GTK_WINDOW (window), icon);
		g_object_unref (icon);
	}

	gconf_editor_window_setup_ui_prefs_handler (window);
}

GType
gconf_editor_window_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (GConfEditorWindowClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gconf_editor_window_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GConfEditorWindow),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gconf_editor_window_init
		};

		object_type = g_type_register_static (GTK_TYPE_WINDOW, "GConfEditorWindow", &object_info, 0);
	}

	return object_type;
}

