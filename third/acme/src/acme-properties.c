/* ACME
 * Copyright (C) 2001, 2002 Bastien Nocera <hadess@hadess.net>
 *
 * acme-properties.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 * USA.
 */

#include <config.h>

#include <sys/file.h>
#include <sys/stat.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gdk/gdkx.h>

#include "acme.h"
#include "eggcellrendererkeys.h"

enum {
	PATH_COL,
	DESC_COL,
	KEYVAL_COL,
	PIX_COL,
	NUM_COLS
};

typedef struct {
	GtkTreeView *tree_view;
	GtkTreePath *path;
} IdleData;

GConfClient *conf_client;
GdkPixbuf *enabled, *disabled;

#define SELECTION_NAME "_ACME_SELECTION"

static gboolean
is_running (void)
{
	gboolean result = FALSE;
	Atom clipboard_atom = gdk_x11_get_xatom_by_name (SELECTION_NAME);

	XGrabServer (GDK_DISPLAY());

	if (XGetSelectionOwner (GDK_DISPLAY(), clipboard_atom) != None)
		result = TRUE;

	XUngrabServer (GDK_DISPLAY());
	gdk_flush();

	return result;
}

static void
response_cb (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_main_quit ();
}

static void
acme_error (char * msg)
{
	GtkWidget *error_dialog;

	error_dialog =
		gtk_message_dialog_new (NULL,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				"%s", msg);
	gtk_dialog_set_default_response (GTK_DIALOG (error_dialog),
			GTK_RESPONSE_OK);
	gtk_widget_show (error_dialog);
	gtk_dialog_run (GTK_DIALOG (error_dialog));
	gtk_widget_destroy (error_dialog);
}

static void
checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
	gboolean state;

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	gconf_client_set_bool (conf_client,
			"/apps/acme/use_pcm", state, NULL);
}

static void
exec_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *hbox = (GtkWidget *)data;

	if (g_spawn_command_line_async ("acme", NULL) == TRUE)
		gtk_widget_set_sensitive (hbox, FALSE);
}

static gboolean
verify_double (char *current_path, int keycode)
{
	gboolean found = FALSE;
	int i = 0;

	while (found == FALSE && i < HANDLED_KEYS)
	{
		int val;

		if (strcmp (keys[i].key_config, current_path) == 0)
		{
			i++;
			continue;
		}

		val = gconf_client_get_int (conf_client,
				keys[i].key_config, NULL);
		if (val == keycode)
		{
			found = TRUE;
			break;
		}
		i++;
	}

	return found;
}

static void
keys_edited_callback (GtkCellRendererText *cell, const char *path_string,
		guint keyval, GdkModifierType mask, guint keycode,
		gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	char *gconf_path, *value_str;
	int value;
	gboolean verified = FALSE;
	GdkPixbuf *pix;

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, PATH_COL, &gconf_path, -1);

	if (gconf_path == NULL)
		return;

	if (keyval == GDK_C || keyval == GDK_c)
	{
		gtk_tree_path_free (path);
		return;
	}

	if (keyval == GDK_BackSpace)
	{
		verified = TRUE;
		value = -1;
		value_str = g_strdup (_("Disabled"));
		pix = disabled;
	} else {
		verified = FALSE;
		value = keycode;
		value_str = g_strdup (_("Enabled"));
		pix = enabled;
	}

	if (verified == FALSE)
	{
		if (verify_double (gconf_path, value) == TRUE)
		{
			gtk_tree_path_free (path);
			g_free (value_str);
			acme_error (_("This key is already bound to an action.\nPlease select another key."));
			return;
		}
	}

	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			KEYVAL_COL, value_str,
			PIX_COL, pix,
			-1);
	gconf_client_set_int (conf_client,
			gconf_path, value, NULL);

	gtk_tree_path_free (path);
}

static gboolean
real_start_editing_cb (IdleData *idle_data)
{
	gtk_widget_grab_focus (GTK_WIDGET (idle_data->tree_view));
	gtk_tree_view_set_cursor (idle_data->tree_view,
			idle_data->path,
			gtk_tree_view_get_column (idle_data->tree_view,
				DESC_COL), TRUE);

	gtk_tree_path_free (idle_data->path);
	g_free (idle_data);
	return FALSE;
}

static gboolean
start_editing_cb (GtkTreeView *tree_view, GdkEventButton *event,
		gpointer user_data)
{
	GtkTreePath *path;

	if (event->window != gtk_tree_view_get_bin_window (tree_view))
		return FALSE;

	if (gtk_tree_view_get_path_at_pos (tree_view,
				(gint) event->x,
				(gint) event->y,
				&path, NULL,
				NULL, NULL))
	{
		IdleData *idle_data;

		idle_data = g_new (IdleData, 1);
		idle_data->tree_view = tree_view;
		idle_data->path = path;
		g_signal_stop_emission_by_name (G_OBJECT (tree_view),
				"button_press_event");
		g_idle_add ((GSourceFunc) real_start_editing_cb, idle_data);
	}

	return TRUE;
}

static void
init_content_helper (GtkListStore *store, const char *path,
		const char *desc, int keyval)
{
	GtkTreeIter iter;
	char *keyval_str;
	GdkPixbuf *pix;

	if (keyval > 0)
	{
		keyval_str = g_strdup (_("Enabled"));
		pix = enabled;
	} else {
		keyval_str = g_strdup (_("Disabled"));
		pix = disabled;
	}

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			PATH_COL, path,
			DESC_COL, desc,
			KEYVAL_COL, keyval_str,
			PIX_COL, pix,
			-1);
}

static void
init_content (GtkTreeView *treeview)
{
	GtkListStore *store;
	int i, keycode;

	store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));
	for (i = 0; i < HANDLED_KEYS; i++)
	{
		keycode = gconf_client_get_int (conf_client,
				keys[i].key_config, NULL);
		init_content_helper (store, keys[i].key_config,
				_(keys[i].description), keycode);
	}
}

static void
init_columns (GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* the Status before it */
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Description"),
			renderer,
			"pixbuf", PIX_COL,
			NULL);
	gtk_tree_view_append_column (treeview, column);

	/* Labels */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer,
			"text", DESC_COL, NULL);

	/* keys */
	renderer = g_object_new (EGG_TYPE_CELL_RENDERER_KEYS,
			"editable", TRUE,
			NULL);
	g_signal_connect (G_OBJECT (renderer), "keys_edited",
			G_CALLBACK (keys_edited_callback),
			gtk_tree_view_get_model (treeview));

	column = gtk_tree_view_column_new_with_attributes (_("Status"),
			renderer,
			"text", KEYVAL_COL,
			NULL);
	gtk_tree_view_append_column (treeview, column);

	g_signal_connect (G_OBJECT (treeview), "button_press_event",
			G_CALLBACK (start_editing_cb), NULL);
}

static void
init_treeview (GtkWidget *treeview)
{
	GtkTreeModel *model;

	/* the model */
	model = GTK_TREE_MODEL (gtk_list_store_new (NUM_COLS,
				G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_STRING,
				GDK_TYPE_PIXBUF));

	/* the treeview */
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	g_object_unref (G_OBJECT (model));

	init_columns (GTK_TREE_VIEW (treeview));
	init_content (GTK_TREE_VIEW (treeview));
	gtk_widget_show (treeview);
}

static GtkWidget
*init_gui (void)
{
	GladeXML *xml;
	GtkWidget *window, *treeview, *checkbox, *exec_box, *exec;
	GdkPixbuf *pixbuf;

	xml = glade_xml_new (ACME_DATA "acme-properties.glade", NULL, NULL);

	window = glade_xml_get_widget (xml, "window");
	g_signal_connect (G_OBJECT (window), "response",
			G_CALLBACK (response_cb), NULL);

	enabled = gtk_widget_render_icon (window, GTK_STOCK_APPLY,
			GTK_ICON_SIZE_MENU, NULL);
	disabled = gtk_widget_render_icon (window, GTK_STOCK_CANCEL,
			GTK_ICON_SIZE_MENU, NULL);

	/* the treeview */
	treeview = glade_xml_get_widget (xml, "treeview");
	init_treeview (treeview);

	pixbuf = gdk_pixbuf_new_from_file (ACME_DATA "acme-48.png", NULL);
	if (pixbuf != NULL)
		gtk_window_set_icon (GTK_WINDOW (window), pixbuf);

	checkbox = glade_xml_get_widget (xml, "checkbutton1");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
			gconf_client_get_bool (conf_client,
				"/apps/acme/use_pcm", NULL));
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			G_CALLBACK (checkbox_toggled_cb), NULL);

	exec_box = glade_xml_get_widget (xml, "hbox8");
	gtk_widget_set_sensitive (exec_box, !is_running ());

	exec = glade_xml_get_widget (xml, "button1");
	g_signal_connect (GTK_BUTTON (exec), "clicked",
			G_CALLBACK (exec_clicked), exec_box);

	gtk_widget_show_all (window);

	return window;
}

int
main (int argc, char *argv[])
{
	GtkWidget *dialog;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("acme-properties", VERSION,
			LIBGNOMEUI_MODULE,
			argc, argv,
			NULL);

	glade_gnome_init ();

	conf_client = gconf_client_get_default ();
	gconf_client_add_dir (conf_client,
			"/apps/acme",
			GCONF_CLIENT_PRELOAD_ONELEVEL,
			NULL);

	dialog = init_gui ();
	gtk_main ();

	return 0;
}

