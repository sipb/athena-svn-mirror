/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include <atk/atk.h>

#include <gtk/gtkdialog.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderer.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktreeview.h>

#include <gtk/gtkscrolledwindow.h>

#include <libgnome/gnome-i18n.h>

#include <gconf/gconf-client.h>

#include "gnome-cd.h"
#include "preferences.h"

static GConfClient *client = NULL;

static void
do_device_changed (GnomeCDPreferences *prefs,
		   const char *device)
{
	GError *error = NULL;
	gboolean ret;
	
	if (prefs->device != NULL) {
		g_free (prefs->device);
	}

	prefs->device = g_strdup (device);
	if (prefs->gcd->cdrom != NULL &&
	    prefs->gcd->device_override == NULL) {
		ret = gnome_cdrom_set_device (prefs->gcd->cdrom, prefs->device, &error);
		if (ret == FALSE) {
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new (NULL, 0,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_OK,
							 _("%s\nThis means that the CD player will not be able to run."), error->message);
			gtk_window_set_title (GTK_WINDOW (dialog), _("Error setting device"));
			g_signal_connect (G_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_widget_show (dialog);
			g_error_free (error);
		}
		
		cd_selection_stop (prefs->gcd->cd_selection);
		prefs->gcd->cd_selection = cd_selection_start (prefs->device);
	}
}

static void
device_changed (GConfClient *_client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	GnomeCDPreferences *prefs = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	do_device_changed (prefs, gconf_value_get_string (value));
}

static void
on_start_changed (GConfClient *_client,
		  guint cnxn_id,
		  GConfEntry *entry,
		  gpointer user_data)
{
	GnomeCDPreferences *prefs = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	prefs->start = gconf_value_get_int (value);
	/* This doesn't take effect till next start,
	   so we don't need to do anything for it */
}

#ifdef HAVE_CDROMCLOSETRAY_IOCTL
static void
close_on_start_changed (GConfClient *_client,
			guint cnxn_id,
			GConfEntry *entry,
			gpointer user_data)
{
	GnomeCDPreferences *prefs = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	prefs->start_close = gconf_value_get_bool (value);
	/* This doesn't take effect till next start either */
}
#endif

static void
on_stop_changed (GConfClient *_client,
		 guint cnxn_id,
		 GConfEntry *entry,
		 gpointer user_data)
{
	GnomeCDPreferences *prefs = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	prefs->stop = gconf_value_get_int (value);
	/* This doesn't take effect till we quit... */
}

static void
do_theme_changed (GnomeCDPreferences *prefs,
		  const char *theme_name)
{
	GCDTheme *old_theme;
	
	if (prefs->theme_name != NULL) {
		g_free (prefs->theme_name);
	}

	prefs->theme_name = g_strdup (theme_name);
	old_theme = prefs->gcd->theme;
	prefs->gcd->theme = theme_load (prefs->gcd, theme_name);

	/* Revert to the old theme if something messed up */
	if (prefs->gcd->theme == NULL) {
		prefs->gcd->theme = old_theme;
	} else {
		theme_change_widgets (prefs->gcd);
		theme_free (old_theme);
	}
}

static void
theme_changed (GConfClient *_client,
	       guint cnxn_id,
	       GConfEntry *entry,
	       gpointer user_data)
{
	GnomeCDPreferences *prefs = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	do_theme_changed (prefs, gconf_value_get_string (value));
}

static void
restore_preferences (GnomeCDPreferences *prefs)
{
	GError *error = NULL;

	/* Add the dir, cos we're getting all the stuff from it anyway */
	gconf_client_add_dir (client, "/apps/gnome-cd",
			      GCONF_CLIENT_PRELOAD_NONE, &error);
	if (error != NULL) {
		g_warning ("Error: %s", error->message);
		error = NULL;
	}
	
	prefs->device = gconf_client_get_string (client,
						 "/apps/gnome-cd/device", NULL);
	if (prefs->device == NULL) {
		g_warning ("GConf schemas are not correctly installed.");
		prefs->device = g_strdup (default_cd_device);
	}
	
	prefs->device_id = gconf_client_notify_add (client,
						    "/apps/gnome-cd/device", device_changed,
						    prefs, NULL, &error);
	if (error != NULL) {
		g_warning ("Error: %s", error->message);
	}
						    
	prefs->start = gconf_client_get_int (client,
					     "/apps/gnome-cd/on-start", NULL);
	prefs->start_id = gconf_client_notify_add (client,
						   "/apps/gnome-cd/on-start",
						   on_start_changed, prefs,
						   NULL, NULL);
#ifdef HAVE_CDROMCLOSETRAY_IOCTL	
	prefs->start_close = gconf_client_get_bool (client,
						    "/apps/gnome-cd/close-on-start", NULL);
	prefs->close_id = gconf_client_notify_add (client,
						   "/apps/gnome-cd/close-on-start",
						   close_on_start_changed, prefs,
						   NULL, NULL);
#endif
	
	prefs->stop = gconf_client_get_int (client,
					    "/apps/gnome-cd/on-stop", NULL);
	prefs->stop_id = gconf_client_notify_add (client,
						  "/apps/gnome-cd/on-stop",
						  on_stop_changed, prefs,
						  NULL, NULL);

	prefs->theme_name = gconf_client_get_string (client,
						     "/apps/gnome-cd/theme-name", NULL);
	if (prefs->theme_name == NULL) {
		g_warning ("GConf schemas are not correctly installed");
		prefs->theme_name = g_strdup ("lcd");
	}
	
	prefs->theme_id = gconf_client_notify_add (client,
						   "/apps/gnome-cd/theme-name",
						   theme_changed, prefs,
						   NULL, NULL);
}

void
preferences_free (GnomeCDPreferences *prefs)
{
	g_free (prefs->device);
	g_free (prefs->theme_name);

	/* Remove the listeners */
	gconf_client_notify_remove (client, prefs->device_id);
	gconf_client_notify_remove (client, prefs->start_id);
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	gconf_client_notify_remove (client, prefs->close_id);
#endif
	gconf_client_notify_remove (client, prefs->stop_id);
	gconf_client_notify_remove (client, prefs->theme_id);

	g_free (prefs);
}

GnomeCDPreferences *
preferences_new (GnomeCD *gcd)
{
	GnomeCDPreferences *prefs;

	prefs = g_new0 (GnomeCDPreferences, 1);
	prefs->gcd = gcd;

	client = gconf_client_get_default ();
	
	restore_preferences (prefs);
	
	return prefs;
}

typedef struct _PropertyDialog {
	GnomeCD *gcd; /* The GnomeCD object this is connected to */
	GtkWidget *window;

	GtkWidget *cd_device;
	GtkWidget *apply;
	
	GtkWidget *start_nothing;
	GtkWidget *start_play;
	GtkWidget *start_stop;
	
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	GtkWidget *start_close;
#endif

	GtkWidget *stop_nothing;
	GtkWidget *stop_stop;
	GtkWidget *stop_open;
	
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	GtkWidget *stop_close;
#endif

	GtkWidget *theme_list;
	
	guint start_id;
	guint device_id;
	
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	guint close_id;
#endif
	
	guint stop_id;
	guint theme_id;
} PropertyDialog;

static void
prefs_response_cb (GtkWidget *dialog,
		   int response_id,
		   PropertyDialog *pd)
{
	GError *error = NULL;
	switch (response_id) {
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_NONE:
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_widget_destroy (dialog);
		break;

	case GTK_RESPONSE_HELP:
		gnome_help_display("gnome-cd","gtcd-prefs",&error);
		if (error) {
			GtkWidget *msg_dialog;
			msg_dialog = gtk_message_dialog_new (GTK_WINDOW(dialog),
							     GTK_DIALOG_DESTROY_WITH_PARENT,
							     GTK_MESSAGE_ERROR,
							     GTK_BUTTONS_CLOSE,
							     ("There was an error displaying help: \n%s"),
							     error->message);
			g_signal_connect (G_OBJECT (msg_dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);
	
			gtk_window_set_resizable (GTK_WINDOW (msg_dialog), FALSE);
			gtk_widget_show (msg_dialog);
			g_error_free (error);
		}
		break;


	default:
		g_print ("Response %d\n", response_id);
		g_assert_not_reached ();
		break;
	}
}

static void
prefs_destroy_cb (GtkDialog *dialog,
		  PropertyDialog *pd)
{
	gconf_client_notify_remove (client, pd->device_id);
	gconf_client_notify_remove (client, pd->start_id);
	gconf_client_notify_remove (client, pd->stop_id); 
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	gconf_client_notify_remove (client, pd->close_id);
#endif
	gconf_client_notify_remove (client, pd->theme_id);
	
	g_free (pd);
}

static void
apply_clicked_cb (GtkWidget *apply,
		  PropertyDialog *pd)
{
	const char *new_device;
	GnomeCDRom *dummy;

	new_device = gtk_entry_get_text (GTK_ENTRY (pd->cd_device));
	
	if (!new_device)
		return;

	if (pd->gcd->preferences->device &&
	    strcmp (new_device, pd->gcd->preferences->device) == 0)
		return;
	
	gconf_client_set_string (client, "/apps/gnome-cd/device", new_device, NULL);
	gtk_widget_set_sensitive (pd->apply, FALSE);

	dummy = gnome_cdrom_new (new_device, GNOME_CDROM_UPDATE_NEVER, NULL);
	if (dummy == NULL) {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (pd->window),
						   GTK_RESPONSE_CLOSE,
						   FALSE);
	} else {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (pd->window),
						   GTK_RESPONSE_CLOSE,
						   TRUE);
		g_object_unref (G_OBJECT (dummy));
	}

}

static void
device_changed_cb (GtkWidget *entry,
		   PropertyDialog *pd)
{
	const char *new_device;

	new_device = gtk_entry_get_text (GTK_ENTRY (entry));
	if (new_device == NULL || *new_device == 0) {
		gtk_widget_set_sensitive (pd->apply, FALSE);
		return;
	}

	if (pd->gcd->preferences->device != NULL) {
		if (strcmp (new_device, pd->gcd->preferences->device) == 0) {
			gtk_widget_set_sensitive (pd->apply, FALSE);
			return;
		}
	}

	gtk_widget_set_sensitive (pd->apply, TRUE);
}

static void
change_device_widget (GConfClient *_client,
		      guint cnxn,
		      GConfEntry *entry,
		      gpointer user_data)
{
	PropertyDialog *pd = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	g_signal_handlers_block_matched (G_OBJECT (pd->cd_device), G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL, G_CALLBACK (device_changed_cb), pd);
	gtk_entry_set_text (GTK_ENTRY (pd->cd_device), gconf_value_get_string (value));
	g_signal_handlers_unblock_matched (G_OBJECT (pd->cd_device), G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL, G_CALLBACK (device_changed_cb), pd);
}

static void
set_start (PropertyDialog *pd,
	   GnomeCDPreferencesStart start)
{
	gconf_client_set_int (client,
			      "/apps/gnome-cd/on-start",
			      start, NULL);
}

static void
start_nothing_toggled_cb (GtkToggleButton *tb,
			  PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_start (pd, GNOME_CD_PREFERENCES_START_NOTHING);
}

static void
start_play_toggled_cb (GtkToggleButton *tb,
		       PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_start (pd, GNOME_CD_PREFERENCES_START_START);
}

static void
start_stop_toggled_cb (GtkToggleButton *tb,
		       PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_start (pd, GNOME_CD_PREFERENCES_START_STOP);
}

static void
set_stop (PropertyDialog *pd,
	  GnomeCDPreferencesStop stop)
{
	gconf_client_set_int (client,
			      "/apps/gnome-cd/on-stop",
			      stop, NULL);
}

static void
stop_nothing_toggled_cb (GtkToggleButton *tb,
			 PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_stop (pd, GNOME_CD_PREFERENCES_STOP_NOTHING);
}

static void
stop_stop_toggled_cb (GtkToggleButton *tb,
		      PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_stop (pd, GNOME_CD_PREFERENCES_STOP_STOP);
}

static void
stop_open_toggled_cb (GtkToggleButton *tb,
		      PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_stop (pd, GNOME_CD_PREFERENCES_STOP_OPEN);
}

#ifdef HAVE_CDROMCLOSETRAY_IOCTL
static void
stop_close_toggled_cb (GtkToggleButton *tb,
		       PropertyDialog *pd)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	set_stop (pd, GNOME_CD_PREFERENCES_STOP_CLOSE);
}
#endif

static void
change_start_widget (GConfClient *client,
		     guint cnxn,
		     GConfEntry *entry,
		     gpointer user_data)
{
	PropertyDialog *pd = user_data;
	GConfValue *value = gconf_entry_get_value (entry);
	GCallback func;
	GtkToggleButton *tb;
	
	switch (gconf_value_get_int (value)) {
	case GNOME_CD_PREFERENCES_START_NOTHING:
		tb = GTK_TOGGLE_BUTTON (pd->start_nothing);
		func = G_CALLBACK (start_nothing_toggled_cb);
		break;

	case GNOME_CD_PREFERENCES_START_START:
		tb = GTK_TOGGLE_BUTTON (pd->start_play);
		func = G_CALLBACK (start_play_toggled_cb);
		break;

	case GNOME_CD_PREFERENCES_START_STOP:
		tb = GTK_TOGGLE_BUTTON (pd->start_stop);
		func = G_CALLBACK (start_stop_toggled_cb);
		break;

	default:
		g_warning ("Unknown value: %d", gconf_value_get_int (value));
		return;
	}
	
	g_signal_handlers_block_matched (G_OBJECT (tb), G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL, func, pd);
	gtk_toggle_button_set_active (tb, TRUE);
	g_signal_handlers_unblock_matched (G_OBJECT (tb), G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL, func, pd);
}

static void
change_stop_widget (GConfClient *client,
		    guint cnxn,
		    GConfEntry *entry,
		    gpointer user_data)
{
	PropertyDialog *pd = user_data;
	GConfValue *value = gconf_entry_get_value (entry);
	GCallback func;
	GtkToggleButton *tb;

	switch (gconf_value_get_int (value)) {
	case GNOME_CD_PREFERENCES_STOP_NOTHING:
		tb = GTK_TOGGLE_BUTTON (pd->stop_nothing);
		func = G_CALLBACK (stop_nothing_toggled_cb);
		break;

	case GNOME_CD_PREFERENCES_STOP_STOP:
		tb = GTK_TOGGLE_BUTTON (pd->stop_stop);
		func = G_CALLBACK (stop_stop_toggled_cb);
		break;

	case GNOME_CD_PREFERENCES_STOP_OPEN:
		tb = GTK_TOGGLE_BUTTON (pd->stop_open);
		func = G_CALLBACK (stop_open_toggled_cb);
		break;
		
#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	case GNOME_CD_PREFERENCES_STOP_CLOSE:
		tb = GTK_TOGGLE_BUTTON (pd->stop_close);
		func = G_CALLBACK (stop_close_toggled_cb);
		break;
#endif
		
	default:
		g_warning ("Unknown stop value: %d", gconf_value_get_int (value));
		return;
	}

	g_signal_handlers_block_matched (G_OBJECT (tb), G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL, func, pd);
	gtk_toggle_button_set_active (tb, TRUE);
	g_signal_handlers_unblock_matched (G_OBJECT (tb), G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL, func, pd);
}

#ifdef HAVE_CDROMCLOSETRAY_IOCTL
static void
start_close_toggled_cb (GtkToggleButton *tb,
			PropertyDialog *pd)
{
	gboolean on;

	on = gtk_toggle_button_get_active (tb);
	gconf_client_set_bool (client,
			       "/apps/gnome-cd/close-on-start", on, NULL);
}

static void
change_start_close_widget (GConfClient *client,
			   guint cnxn,
			   GConfEntry *entry,
			   gpointer user_data)
{
	PropertyDialog *pd = user_data;
	GConfValue *value = gconf_entry_get_value (entry);

	g_signal_handlers_block_matched (G_OBJECT (pd->start_close), G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL, G_CALLBACK (start_close_toggled_cb), pd);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->start_close),
				      gconf_value_get_bool (value));
	g_signal_handlers_unblock_matched (G_OBJECT (pd->start_close), G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL, G_CALLBACK (start_close_toggled_cb), pd);
}
#endif

static void
theme_selection_changed_cb (GtkTreeSelection *selection,
			    PropertyDialog *pd)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter) == TRUE) {
		char *theme_name;

		gtk_tree_model_get (model, &iter, 0, &theme_name, -1);
		gconf_client_set_string (client, "/apps/gnome-cd/theme-name", theme_name, NULL);
	}
}

static void
change_theme_selection_widget (GConfClient *client,
			       guint cnxn,
			       GConfEntry *entry,
			       gpointer user_data)
{
	PropertyDialog *pd = user_data;
	GConfValue *value = gconf_entry_get_value (entry);
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pd->theme_list));
	gtk_tree_model_get_iter_root (model, &iter);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pd->theme_list));
	
	do {
		char *name;
		
		gtk_tree_model_get (model, &iter, 0, &name, -1);
		g_print ("Name: %s\n", name);
		
		if (strcmp (name, gconf_value_get_string (value)) == 0) {
			g_signal_handlers_block_matched (G_OBJECT (selection), G_SIGNAL_MATCH_FUNC,
							 0, 0, NULL,
							 G_CALLBACK (theme_selection_changed_cb), pd);
			gtk_tree_selection_select_iter (selection, &iter);
			g_signal_handlers_unblock_matched (G_OBJECT (selection), G_SIGNAL_MATCH_FUNC,
							   0, 0, NULL,
							   G_CALLBACK (theme_selection_changed_cb), pd);
		}
	} while (gtk_tree_model_iter_next (model, &iter));
}

static GtkTreeModel *
create_theme_model (PropertyDialog *pd,
		    GtkTreeView *view,
		    GtkTreeSelection *selection)
{
	GtkListStore *store;
	GtkTreeIter iter;
	DIR *dir;
	struct dirent *d;

	dir = opendir (THEME_DIR);
	if (dir == NULL) {
		g_warning ("No theme dir; %s", THEME_DIR);
		return NULL;
	}
	
	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model (view, GTK_TREE_MODEL (store));
	
	while ((d = readdir (dir))) {

		/* Can't have a theme with a . as the first char */
		if (d->d_name[0] == '.') {
			continue;
		}

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, g_strdup (d->d_name), -1);

		/*
		if (strcmp (d->d_name, pd->gcd->preferences->theme_name) == 0) {
			g_print ("Match\n");
			gtk_tree_selection_select_iter (selection, &iter);
		}
		*/
	}

	closedir (dir);
	
	return GTK_TREE_MODEL (store);
}

static void
add_relation (AtkRelationSet *set,
	      AtkRelationType type,
	      AtkObject *target)
{
	AtkRelation *relation;

	relation = atk_relation_set_get_relation_by_type (set, type);

	if (relation != NULL) {
		GPtrArray *array = atk_relation_get_target (relation);

		g_ptr_array_remove (array, target);
		g_ptr_array_add (array, target);
	} else {
		/* Relation hasn't been created yet */
		relation = atk_relation_new (&target, 1, type);

		atk_relation_set_add (set, relation);
		g_object_unref (relation);
	}
}

static void
add_paired_relations (GtkWidget *target1,
		      AtkRelationType target1_type,
		      GtkWidget *target2,
		      AtkRelationType target2_type)
{
	AtkObject *atk_target1;
	AtkObject *atk_target2;
	AtkRelationSet *set1;
	AtkRelationSet *set2;

	atk_target1 = gtk_widget_get_accessible (target1);
	atk_target2 = gtk_widget_get_accessible (target2);

	set1 = atk_object_ref_relation_set (atk_target1);
	add_relation (set1, target1_type, atk_target2);

	set2 = atk_object_ref_relation_set (atk_target2);
	add_relation (set2, target2_type, atk_target1);
}

static void
add_description (GtkWidget *widget, 
		 const gchar *desc)
{
	AtkObject *atk_widget;

	atk_widget = gtk_widget_get_accessible (widget);
	atk_object_set_description (atk_widget, desc);
}


GtkWidget *
preferences_dialog_show (GnomeCD *gcd,
			 gboolean only_device)
{
	PropertyDialog *pd;
	GtkWindow *windy;
	GtkWidget *hbox, *vbox, *label, *frame;
	GtkWidget *sw;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	
	pd = g_new0 (PropertyDialog, 1);
	
	pd->gcd = gcd;

	if (gcd->window != NULL) {
		windy = GTK_WINDOW (gcd->window);
	} else {
		windy = NULL;
	}
	
	pd->window = gtk_dialog_new_with_buttons (_("CD Player Preferences"),
						  windy,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	gtk_window_set_default_size (GTK_WINDOW (pd->window), 390, 375);
       
	if (only_device == FALSE) {
		g_signal_connect (G_OBJECT (pd->window), "response",
				  G_CALLBACK (prefs_response_cb), pd);
	} else {
		/* A bit of a cheat:
		   but if we're only setting the device,
		   that means the device is wrong... */
		gtk_dialog_set_response_sensitive (GTK_DIALOG (pd->window),
						   GTK_RESPONSE_CLOSE,
						   FALSE);
	}

	g_signal_connect (G_OBJECT (pd->window), "destroy",
			  G_CALLBACK (prefs_destroy_cb), pd);

	/* General */
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (pd->window)->vbox), 2);
	
	/* Top stuff */
	hbox = gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pd->window)->vbox), hbox, FALSE, FALSE, 4);

	label = gtk_label_new_with_mnemonic (_("CD player de_vice:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	pd->cd_device = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (pd->cd_device), gcd->preferences->device);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), pd->cd_device);
	add_paired_relations (label, ATK_RELATION_LABEL_FOR,
			      pd->cd_device, ATK_RELATION_LABELLED_BY);
	
	g_signal_connect (G_OBJECT (pd->cd_device), "changed",
			  G_CALLBACK (device_changed_cb), pd);
	g_signal_connect (G_OBJECT (pd->cd_device), "activate",
			  G_CALLBACK (apply_clicked_cb), pd);
	gtk_box_pack_start (GTK_BOX (hbox), pd->cd_device, TRUE, TRUE, 0);
	
	pd->apply = gtk_button_new_with_mnemonic (_("_Apply change"));
	gtk_widget_set_sensitive (pd->apply, FALSE);
	g_signal_connect (G_OBJECT (pd->apply), "clicked",
			  G_CALLBACK (apply_clicked_cb), pd);
	gtk_box_pack_start (GTK_BOX (hbox), pd->apply, FALSE, FALSE, 0);

	pd->device_id = gconf_client_notify_add (client,
						 "/apps/gnome-cd/device",
						 change_device_widget, pd, NULL, NULL);
	hbox = gtk_hbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pd->window)->vbox), hbox, FALSE, FALSE, 4);

	/* left side */
	frame = gtk_frame_new (_("When CD player starts"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
	
	vbox = gtk_vbox_new (TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	
	pd->start_nothing = gtk_radio_button_new_with_mnemonic (NULL, _("Do _nothing"));
	add_description (pd->start_nothing, _("Do nothing when CD Player starts"));

	if (gcd->preferences->start == GNOME_CD_PREFERENCES_START_NOTHING) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->start_nothing), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->start_nothing), "toggled",
			  G_CALLBACK (start_nothing_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->start_nothing, FALSE, FALSE, 0);

	pd->start_play = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->start_nothing),
									 _("Start _playing CD"));
	add_description (pd->start_play, _("Start playing CD when CD Player starts"));
	if (gcd->preferences->start == GNOME_CD_PREFERENCES_START_START) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->start_play), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->start_play), "toggled",
			  G_CALLBACK (start_play_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->start_play, FALSE, FALSE, 0);
	
	pd->start_stop = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->start_play),
									 _("_Stop playing CD"));
	add_description (pd->start_stop, _("Stop playing CD when CD Player starts"));

	if (gcd->preferences->start == GNOME_CD_PREFERENCES_START_STOP) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->start_stop), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->start_stop), "toggled",
			  G_CALLBACK (start_stop_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->start_stop, FALSE, FALSE, 0);

	pd->start_id = gconf_client_notify_add (client,
						"/apps/gnome-cd/on-start",
						change_start_widget, pd, NULL, NULL);

#ifdef HAVE_CDROMCLOSETRAY_IOCTL	
	pd->start_close = gtk_check_button_new_with_mnemonic (_("Attempt to _close CD tray"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->start_close),
				      gcd->preferences->start_close);
	g_signal_connect (G_OBJECT (pd->start_close), "toggled",
			  G_CALLBACK (start_close_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->start_close, FALSE, FALSE, 0);

	pd->close_id = gconf_client_notify_add (client,
						"/apps/gnome-cd/close-on-start",
						change_start_close_widget, pd, NULL, NULL);
#endif
	/* Right side */
	frame = gtk_frame_new (_("When CD player quits"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

	vbox = gtk_vbox_new (TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	pd->stop_nothing = gtk_radio_button_new_with_mnemonic (NULL, _("Do not_hing"));
	add_description (pd->stop_nothing, _("Do nothing when CD Player exits"));
	if (gcd->preferences->stop == GNOME_CD_PREFERENCES_STOP_NOTHING) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->stop_nothing), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->stop_nothing), "toggled",
			  G_CALLBACK (stop_nothing_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->stop_nothing, FALSE, FALSE, 0);

	pd->stop_stop = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->stop_nothing),
									_("S_top playing CD"));
	add_description (pd->stop_stop, _("Stop playing CD when CD player quits"));

	if (gcd->preferences->stop == GNOME_CD_PREFERENCES_STOP_STOP) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->stop_stop), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->stop_stop), "toggled",
			  G_CALLBACK (stop_stop_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->stop_stop, FALSE, FALSE, 0);

	pd->stop_open = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->stop_stop),
									_("Attempt to _open CD tray"));
	add_description (pd->stop_open, _("Attempt to open CD tray when CD Player exits"));
	if (gcd->preferences->stop == GNOME_CD_PREFERENCES_STOP_OPEN) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->stop_open), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->stop_open), "toggled",
			  G_CALLBACK (stop_open_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->stop_open, FALSE, FALSE, 0);

#ifdef HAVE_CDROMCLOSETRAY_IOCTL
	pd->stop_close = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pd->stop_open),
									 _("Attempt to c_lose CD tray"));
	if (gcd->preferences->stop == GNOME_CD_PREFERENCES_STOP_CLOSE) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->stop_close), TRUE);
	}
	g_signal_connect (G_OBJECT (pd->stop_close), "toggled",
			  G_CALLBACK (stop_close_toggled_cb), pd);
	gtk_box_pack_start (GTK_BOX (vbox), pd->stop_close, FALSE, FALSE, 0);
#endif
	
	pd->stop_id = gconf_client_notify_add (client,
					       "/apps/gnome-cd/on-stop",
					       change_stop_widget, pd, NULL, NULL);
	
	if (only_device == TRUE) {
		gtk_widget_set_sensitive (hbox, FALSE);
	}
	
	/* Theme selector */
	pd->theme_list = gtk_tree_view_new ();
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pd->theme_list));

	model = create_theme_model (pd, GTK_TREE_VIEW (pd->theme_list), selection);
	if (model == NULL) {
		/* Should free stuff here */
		return NULL;
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW (pd->theme_list), model);
	g_object_unref (model);

	cell = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Theme name"), cell,
							"text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pd->theme_list), col);
	
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (theme_selection_changed_cb), pd);
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw), pd->theme_list);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pd->window)->vbox), sw, TRUE, TRUE, 4);

	pd->theme_id = gconf_client_notify_add (client,
						"/apps/gnome-cd/theme-name",
						change_theme_selection_widget, pd, NULL, NULL);
	gtk_widget_show_all (pd->window);

	if (only_device == TRUE) {
		gtk_widget_set_sensitive (sw, FALSE);
	}
	
	return pd->window;
}

