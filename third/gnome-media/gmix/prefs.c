/*
 * GMIX 3.0
 *
 * Copyright (C) 1998 Jens Ch. Restemeier <jchrr@hrz.uni-bielefeld.de>
 * Config dialog added by Matt Martin <Matt.Martin@ieee.org>, Sept 1999
 * ALSA driver by Brian J. Murrell <gnome-alsa@interlinx.bc.ca> Dec 1999
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#ifdef ALSA
#include <sys/asoundlib.h>
#else
#ifdef HAVE_LINUX_SOUNDCARD_H 
#include <linux/soundcard.h>
#else 
#include <machine/soundcard.h>
#endif
#endif

#include <gnome.h>

#include <gconf/gconf-client.h>

#include "gmix.h"
#include "prefs.h"

/******************** General Preferences Page ********************/

mixerprefs prefs={TRUE, -1, TRUE, -1, TRUE, -1};
static GtkWidget *configwin = NULL;
static GConfClient *client = NULL;

/******************** Label Name Page ********************/

static gboolean
reset_labels (GtkTreeModel *model,
	      GtkTreePath *path,
	      GtkTreeIter *iter,
	      gpointer data)
{
	char *original;

	gtk_tree_model_get (model, iter, 1, &original, -1);
	gtk_list_store_set (GTK_LIST_STORE (model), iter, 0, original, -1);
}

static void
labels_reset_page_cb(GtkWidget *button,
		     GtkTreeModel *model)
{
	gtk_tree_model_foreach (model, reset_labels, NULL);
}

/********************/

static GtkTreeModel *
make_label_model (GList *in)
{
	GList *p;
	GtkListStore *store;
	GtkTreeIter iter;

	store = gtk_list_store_new (5, G_TYPE_STRING, /* User title */
				    G_TYPE_STRING, /* Real title (Not shown) */
				    G_TYPE_STRING, /* Card name (Not shown */
				    G_TYPE_BOOLEAN, /* Is fader shown? */
				    G_TYPE_BOOLEAN); /* Editable? */
	for (p = in; p; p = p->next) {
		channel_info *channel = (channel_info *) p->data;
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    0, channel->user_title,
				    1, channel->title,
				    2, channel->device->info.name,
				    3, channel->visible,
				    4, TRUE,
				    -1);
	}

	return GTK_TREE_MODEL (store);
}

static void
toggled (GtkCellRendererToggle *cell,
	 char *path_string,
	 GtkTreeModel *model)
{
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	gboolean value;
	char *device_name, *channel_name;
	device_info *di;
	GList *p;
	
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter,
			    1, &channel_name,
			    2, &device_name,
			    3, &value, -1);

	value = !value;

	di = device_from_name (device_name);
	for (p = di->channels; p; p = p->next) {
		channel_info *ci = (channel_info *) p->data;
		if (strcmp (ci->title, channel_name) == 0) {
			gmix_change_channel (ci, value);
			break;
		}
	}
	
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 3, value, -1);

	gtk_tree_path_free (path);
}

static void
edited (GtkCellRendererText *cell,
	char *path_string,
	char *new_text,
	gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	char *device_name, *channel_name;
	device_info *di;
	GList *p;
	
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &channel_name, 2, &device_name, -1);
	di = device_from_name (device_name);

	/* Use a hash table? */
	for (p = di->channels; p; p = p->next) {
		channel_info *ci = (channel_info *) p->data;
		if (strcmp (ci->title, channel_name) == 0) {
			gtk_label_set_text (GTK_LABEL (ci->label), new_text);
			g_free (ci->user_title);
			ci->user_title = g_strdup (new_text);
			break;
		}
	}
	
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, new_text, -1);

	gtk_tree_path_free (path);
}

static GtkWidget *
label_one_device_config_widget (gpointer device)
{
	GtkWidget *ubervbox, *treeview, *bbox, *button, *scrlwin;
	GtkWidget *topvbox;
	GtkTreeModel *model;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;
	
	device_info *info = (device_info *) device;

	/* The page: A scrolled viewport */
	scrlwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlwin),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlwin),
					    GTK_SHADOW_IN);  

	topvbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (topvbox), scrlwin, TRUE, TRUE, 0);
	
	model = make_label_model (info->channels);
	treeview = gtk_tree_view_new_with_model (model);

	cell = gtk_cell_renderer_toggle_new ();
	g_signal_connect (G_OBJECT (cell), "toggled",
			  G_CALLBACK (toggled), model);
	col = gtk_tree_view_column_new_with_attributes (_("Shown"), cell,
							"active", 3, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Shown"), cell,
						     "active", 3,
						     NULL);
	
						     
	cell = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (cell), "edited",
			  G_CALLBACK (edited), model);
	col = gtk_tree_view_column_new_with_attributes (_("Mixer label"), cell,
							"text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Mixer label"),
						     cell,
						     "text", 0,
						     "editable", 4,
						     NULL);

	gtk_container_add (GTK_CONTAINER (scrlwin), treeview);
	
	/* Add the label reset button */
	button = gtk_button_new_with_mnemonic (_("_Reset labels to their defaults"));
	gtk_box_pack_start (GTK_BOX(topvbox), button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT(button), "clicked",
			  G_CALLBACK (labels_reset_page_cb), model);

	return topvbox;
}

/*
 * Whip up a page showing all the labels for a single sound device.
 * This page is presented as a vertically scrolling pane containing a
 * table of channels and their labels, and button to reset all the
 * labels to their default values.  Each row in the table is a single
 * sound channel giving its "name" (gtk label) and its current value
 * (gtk entry).  All of the gtk entries are strung onto a list for
 * easy access later in the callback routines.  A clist would have
 * been convenient for this task except that it doesn't support
 * editable items.
 *
 * @param device A pointer to the device_info data structure for this
 * sound device.
 *
 * @param parent A pointer to the parent notebook that contains a
 * single page for each sound device.
 */
static void
label_one_device_config (gpointer device,
			 gpointer parent)
{
	GtkWidget *topvbox;
	GtkWidget *label;

	device_info *info = (device_info *) device;
	
	topvbox = label_one_device_config_widget (device);
	label = gtk_label_new(info->info.name);

	gtk_notebook_append_page(GTK_NOTEBOOK(parent), topvbox, label);
}

/*
 * Whip up a notebook page for the preferences panel containing all
 * the current labels.  This is arranged as a sub-notebook (within the
 * preferences notebook) containing one page per device.  This routine
 * calls a helper routine once for each sound device, and that creates
 * the individual pages in the notebook.
 *
 * @return GtkWidget * A pointer to the newly created notebook of
 * editable labels.  This notebook should be installed within the
 * preferences notebook.
 */
static GtkWidget *labels_create_page(void)
{
	GtkWidget *page;

	if (devices->next == NULL) {
		/* devices->next will only be NULL if there's only one device. */
		GtkWidget *widget;

		widget = label_one_device_config_widget (devices->data);
		page = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (page), widget, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (page), 5);
	} else {
		page = gtk_notebook_new();
		gtk_container_set_border_width(GTK_CONTAINER(page), 5);

		g_list_foreach(devices, label_one_device_config, page);
	}

	return(page);
}

static void
prefs_response_cb (GtkDialog *dialog,
		   guint response_id,
		   gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
prefs_destroy_cb (GtkWidget *widget,
		  gpointer data)
{
	/* nothing - yet :)*/
}

void
prefs_make_window (GtkWidget *toplevel)
{
	GtkWidget *page;

	if (configwin != NULL) {
		gdk_window_show (configwin->window);
		gdk_window_raise (configwin->window);
	} else {
		configwin = gtk_dialog_new_with_buttons (_("Gnome Volume Control Preferences"),
							 GTK_WINDOW (toplevel),
							 GTK_DIALOG_DESTROY_WITH_PARENT,
							 GTK_STOCK_CLOSE,
							 GTK_RESPONSE_CLOSE, NULL);
		gtk_window_set_default_size (GTK_WINDOW (configwin), 380, 256);
		g_signal_connect (G_OBJECT (configwin), "response",
				  G_CALLBACK (prefs_response_cb), NULL);
		g_signal_connect (G_OBJECT (configwin), "destroy",
				  G_CALLBACK (prefs_destroy_cb), NULL);
		g_signal_connect (G_OBJECT (configwin), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &configwin);

		page = labels_create_page ();
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (configwin)->vbox),
				    page, TRUE, TRUE, 0);
		
		gtk_widget_show_all (configwin);
	}
}

/* Not sure if this is needed... */
void get_gui_config(void)
{
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	gconf_client_add_dir (client, "/apps/gnome-volume-control",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
}
