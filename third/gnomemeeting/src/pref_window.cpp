
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         pref_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es> 
 */


#include "../config.h"

#include "pref_window.h"
#include "gnomemeeting.h"
#include "ils.h"
#include "sound_handling.h"
#include "misc.h"
#include "urlhandler.h"
#include "codec_info.h"
#include "callbacks.h"
#include "lid.h"

#include "dialog.h"
#include "gnome_prefs_window.h"
#include "gconf_widgets_extensions.h"


/* Declarations */

extern GtkWidget *gm;

static void refresh_devices_list_button_clicked (GtkWidget *,
						 gpointer);

static void personal_data_update_button_clicked (GtkWidget *,
						 gpointer);

static void gatekeeper_update_button_clicked (GtkWidget *,
					      gpointer);

static void codecs_list_button_clicked_callback (GtkWidget *,
						 gpointer);

static void codecs_list_info_button_clicked_callback (GtkWidget *,
						      gpointer);

static void gnomemeeting_codecs_list_add (GtkTreeIter,
					  GtkListStore *, 
					  const gchar *,
					  bool,
					  bool,
					  gchar *);

static GtkWidget *gnomemeeting_pref_window_add_update_button (GtkWidget *,
							      const char *,
							      const char *,
							      GtkSignalFunc,
							      gchar *,
							      gfloat);
static void sound_event_changed_cb (GtkEntry *,
				    gpointer);

static void sound_event_clicked_cb (GtkTreeSelection *,
				    gpointer);

static void sound_event_toggled_cb (GtkCellRendererToggle *,
				    gchar *, 
				    gpointer);

static void codecs_list_fixed_toggled (GtkCellRendererToggle *,
				       gchar *, 
				       gpointer);

static void browse_button_clicked_cb (GtkWidget *,
				      gpointer);

static void file_selector_clicked (GtkFileSelection *,
				   gpointer);

static void gnomemeeting_init_pref_window_general (GtkWidget *,
						   GtkWidget *);

static void gnomemeeting_init_pref_window_interface (GtkWidget *,
						     GtkWidget *);

static void gnomemeeting_init_pref_window_directories (GtkWidget *,
						       GtkWidget *);

static void gnomemeeting_init_pref_window_sound_events (GtkWidget *,
							GtkWidget *);

static void gnomemeeting_init_pref_window_call_forwarding (GtkWidget *,
							   GtkWidget *);

static void gnomemeeting_init_pref_window_call_options (GtkWidget *,
							GtkWidget *);

static void gnomemeeting_init_pref_window_h323_advanced (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_gatekeeper (GtkWidget *,
						      GtkWidget *);

static void gnomemeeting_init_pref_window_gateway (GtkWidget *,
						   GtkWidget *);

static void gnomemeeting_init_pref_window_nat (GtkWidget *,
					       GtkWidget *);

static void gnomemeeting_init_pref_window_video_devices (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_audio_devices (GtkWidget *,
							 GtkWidget *);

static void gnomemeeting_init_pref_window_audio_codecs (GtkWidget *,
							GtkWidget *);

static void gnomemeeting_init_pref_window_video_codecs (GtkWidget *,
							GtkWidget *);


enum {
  
  COLUMN_CODEC_ACTIVE,
  COLUMN_CODEC_NAME,
  COLUMN_CODEC_INFO,
  COLUMN_CODEC_BANDWIDTH,
  COLUMN_CODEC_SELECTABLE,
  COLUMN_CODEC_COLOR,
  COLUMN_CODEC_NUMBER
};


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the refresh devices list button in the prefs.
 * BEHAVIOR     :  Redetects the devices and refreshes the menu.
 * PRE          :  /
 */
static void
refresh_devices_list_button_clicked (GtkWidget *w,
				     gpointer data)
{
  GnomeMeeting::Process ()->DetectDevices ();

  gnomemeeting_pref_window_update_devices_list ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the Personal data Settings.
 * BEHAVIOR     :  Updates the values.
 * PRE          :  /
 */
static void personal_data_update_button_clicked (GtkWidget *widget, 
						  gpointer data)
{
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  /* Prevent crossed-mutex deadlock */
  gdk_threads_leave ();
  
  /* Both are able to not register if the option is not active */
  endpoint->ILSRegister ();
  endpoint->GatekeeperRegister ();

  gdk_threads_enter ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the gatekeeper Settings.
 * BEHAVIOR     :  Updates the values, and try to register to the gatekeeper.
 * PRE          :  /
 */
static void 
gatekeeper_update_button_clicked (GtkWidget *widget, 
                                  gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  /* Prevent GDK deadlock */
  gdk_threads_leave ();
  
  /* Register the current Endpoint to the Gatekeeper */
  ep->GatekeeperRegister ();

  gdk_threads_enter ();
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button in the Audio Codecs Settings 
 *                 (Up, Down)
 * BEHAVIOR     :  It updates the list order.
 * PRE          :  /
 */
static void codecs_list_button_clicked_callback (GtkWidget *widget, 
						 gpointer data)
{ 	
  GtkTreeIter iter;
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;
  GSList *codecs_data = NULL;
  GSList *codecs_data_element = NULL;
  GSList *codecs_data_iter = NULL;
  gchar *selected_codec_name = NULL;
  gchar **couple;
  int codec_pos = 0;
  int operation = 0;


  /* Get the current selected codec name, there is always one */
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (data), "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL,
				   &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (data), &iter,
		      COLUMN_CODEC_NAME, &selected_codec_name, -1);


  /* We set the selected codec name as data of the list store, to select 
     it again once the codecs list has been rebuilt */
  g_object_set_data (G_OBJECT (data), "selected_codec", 
		     (gpointer) selected_codec_name); 

  /* the gchar * must not be freed,
     it points to the internal
     element of the list_store */
			  
  /* Read all codecs, build the gconf data for the key, after having 
     set the selected codec one row above its current plance */
  codecs_data = gconf_get_string_list (AUDIO_CODECS_KEY "list"); 

  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {
    
    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0]) {

      if (!strcmp (couple [0], selected_codec_name)) {

	g_strfreev (couple);

	break;
      }

      codec_pos++;
    }

    codecs_data_iter = codecs_data_iter->next;
  }

  
  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), 
	       "up"))
    operation = 1;


  /* The selected codec is at pos codec_pos, we will build the gconf key data,
     and set that codec one pos up or one pos down */
  if (((codec_pos == 0)&&(operation == 1))||
      ((codec_pos == GM_AUDIO_CODECS_NUMBER - 1)&&(operation == 0))) {

    g_slist_free (codecs_data);

    return;
  }

  
  if (operation == 1) {

    
    codecs_data_element = g_slist_nth (codecs_data, codec_pos);
    codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
    codecs_data = 
      g_slist_insert (codecs_data, (gchar *) codecs_data_element->data, 
		      codec_pos - 1);
    g_slist_free (codecs_data_element);
  }
  else {
    
    codecs_data_element = g_slist_nth (codecs_data, codec_pos);
    codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
    codecs_data = 
      g_slist_insert (codecs_data, (gchar *) codecs_data_element->data, 
		      codec_pos + 1);
    g_slist_free (codecs_data_element);    
  }


  gconf_set_string_list (AUDIO_CODECS_KEY "list", codecs_data);
  
  g_slist_free (codecs_data);
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the info button in the Audio Codecs Settings.
 * BEHAVIOR     :  Displays an information popup about the codec.
 * PRE          :  /
 */
static void codecs_list_info_button_clicked_callback (GtkWidget *widget, 
						      gpointer data)
{ 	
  PString info;
  GMH323CodecInfo codec;

  gchar *selected_codec_name = NULL;

  GmWindow *gw = NULL;

  GtkTreeIter iter;
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;


  gw = GnomeMeeting::Process ()->GetMainWindow ();

  /* Get the current selected codec name, there is always one */
  tree_view = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (data), "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL,
				   &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (data), &iter,
		      COLUMN_CODEC_NAME, &selected_codec_name, -1);

  if (selected_codec_name) {

    codec = GMH323CodecInfo (selected_codec_name);
    gnomemeeting_message_dialog (GTK_WINDOW (gw->pref_window), 
				 _("Codec Information"), 
				 codec.GetCodecInfo ());

    g_free (selected_codec_name);
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button of the file selector.
 * BEHAVIOR     :  It sets the selected filename in the good entry (given
 *                 as data of the object because of the bad API). Emits the
 *                 focus-out-event to simulate it.
 * PRE          :  data = the file selector.
 */
static void  
file_selector_clicked (GtkFileSelection *b, gpointer data) 
{
  gchar *filename = NULL;
  
  filename =
    (gchar *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (data));

  gtk_entry_set_text (GTK_ENTRY (g_object_get_data (G_OBJECT (data), "entry")),
		      filename);

  g_signal_emit_by_name (G_OBJECT (g_object_get_data (G_OBJECT (data), "entry")), "activate");
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the browse button (in the video devices or sound events).
 * BEHAVIOR     :  It displays the file selector widget.
 * PRE          :  /
 */
static void
browse_button_clicked_cb (GtkWidget *b, gpointer data)
{
  GtkWidget *selector = NULL;

  selector = gtk_file_selection_new (_("Choose a Picture"));

  gtk_widget_show (selector);

  /* FIX ME: Ugly hack cause the file selector API is not good and I don't
     want to use global variables */
  g_object_set_data (G_OBJECT (selector), "entry", (gpointer) data);
    
  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
		    "clicked",
		    G_CALLBACK (file_selector_clicked),
		    (gpointer) selector);
     
  /* Ensure that the dialog box is destroyed when the user clicks a button. */
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);
}


static void 
gnomemeeting_codecs_list_add (GtkTreeIter iter, GtkListStore *store, 
			      const gchar *codec_name, bool enabled,
			      bool possible, gchar *color)
{
  GMH323CodecInfo codec;

  PString codec_quality;
  PString codec_bitrate;

  if (!codec_name || !store || !color)
    return;

  codec = GMH323CodecInfo (codec_name);
  codec_quality = codec.GetCodecQuality ();
  codec_bitrate = codec.GetCodecBitRate ();
    
  if (!codec_quality.IsEmpty () && !codec_bitrate.IsEmpty ()) {

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			COLUMN_CODEC_ACTIVE, enabled,
			COLUMN_CODEC_NAME, codec_name,
			COLUMN_CODEC_INFO, (const char *) codec_quality,
			COLUMN_CODEC_BANDWIDTH, (const char *) codec_bitrate,
			COLUMN_CODEC_SELECTABLE, possible,
			COLUMN_CODEC_COLOR, color,
			-1);
  }
}


/* DESCRIPTION  :  This callback is called when the user changes
 *                 the sound file in the GtkEntry widget.
 * BEHAVIOR     :  It udpates the GConf key corresponding the currently
 *                 selected sound event and updates it to the new value
 *                 if required.
 * PRE          :  /
 */
static void
sound_event_changed_cb (GtkEntry *entry,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  const char *entry_text = NULL;
  gchar *gconf_key = NULL;
  gchar *sound_event = NULL;
  
  GmPrefWindow *pw = NULL;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			2, &gconf_key, -1);
    
    if (gconf_key) { 

      entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
      sound_event = gconf_get_string (gconf_key);
      
      if (!sound_event || strcmp (entry_text, sound_event))
	gconf_set_string (gconf_key, (gchar *) entry_text);

      g_free (gconf_key);
      g_free (sound_event);
    }
  } 
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list.
 * BEHAVIOR     :  It udpates the GtkEntry to the GConf value for the key
 *                 corresponding to the currently selected sound event.
 *                 The sound_event_changed_cb is blocked to prevent it to
 *                 be triggered when the GtkEntry is udpated with the new
 *                 value.
 * PRE          :  /
 */
static void
sound_event_clicked_cb (GtkTreeSelection *selection,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *gconf_key = NULL;
  gchar *sound_event = NULL;
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			2, &gconf_key, -1);
    
    if (gconf_key) { 

      sound_event = gconf_get_string (gconf_key);
      g_signal_handlers_block_matched (G_OBJECT (data),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) sound_event_changed_cb,
				       NULL);
      if (sound_event)
	gtk_entry_set_text (GTK_ENTRY (data), sound_event);
      g_signal_handlers_unblock_matched (G_OBJECT (data),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) sound_event_changed_cb,
					 NULL);
      
      g_free (gconf_key);
      g_free (sound_event);
    }
  }
}


static void
sound_event_play_clicked_cb (GtkWidget *b,
			     gpointer data)
{
  GMSoundEvent ((const char *) gtk_entry_get_text (GTK_ENTRY (data)));
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list and change the toggle.
 * BEHAVIOR     :  It udpates the GConf key associated with the currently
 *                 selected sound event so that it reflects the state of the
 *                 sound event (enabled or disabled).
 * PRE          :  /
 */
static void
sound_event_toggled_cb (GtkCellRendererToggle *cell,
			gchar *path_str,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gchar *gconf_key = NULL;
  
  BOOL fixed = FALSE;

  
  model = (GtkTreeModel *) data;
  path = gtk_tree_path_new_from_string (path_str);

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &fixed, 3, &gconf_key, -1);

  fixed ^= 1;

  gconf_set_bool (gconf_key, fixed);
  
  g_free (gconf_key);
  gtk_tree_path_free (path);
}


static void
codecs_list_fixed_toggled (GtkCellRendererToggle *cell,
			   gchar *path_str,
			   gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *) data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gchar *codec_new = NULL, **couple;
  GSList *codecs_data = NULL, *codecs_data_iter = NULL;
  GSList *codecs_data_element = NULL;
  gboolean fixed;
  gchar *selected_codec_name = NULL;
  int current_row = 0;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_ACTIVE, &fixed, -1);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_NAME, &selected_codec_name, -1);
  fixed ^= 1;
  gtk_tree_path_free (path);

  /* We set the selected codec name as data of the list store, 
     to select it again once the codecs list has been rebuilt */
  g_object_set_data (G_OBJECT (data), "selected_codec", 
		     (gpointer) selected_codec_name); 
  /* Stores a copy of the pointer,
     the gchar * must not be freed,
     as it points to the list_store
     element */

  /* Read all codecs, build the gconf data for the key, 
     after having set the selected codec
     one row above its current plance */
  codecs_data = gconf_get_string_list (AUDIO_CODECS_KEY "list");

  /* We are reading the codecs */
  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {
    
    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0]) {

      if (!strcmp (couple [0], selected_codec_name)) {

	gchar *v = g_strdup_printf ("%d", (int) fixed);
	codec_new = g_strconcat (couple [0], "=", v,  NULL);
	g_free (v);
	g_strfreev (couple);

	break;
      }

      current_row++;
    }

    g_strfreev (couple);

    codecs_data_iter = codecs_data_iter->next;
  }  


  /* Rebuilt the gconf_key with the update values */
  codecs_data_element = g_slist_nth (codecs_data, current_row); 
  codecs_data = g_slist_remove_link (codecs_data, codecs_data_element);
  codecs_data = g_slist_insert (codecs_data, codec_new, current_row);
  
  g_slist_free (codecs_data_element);
  
  gconf_set_string_list (AUDIO_CODECS_KEY "list", codecs_data);

  g_slist_free (codecs_data);
  g_free (codec_new);
}


/* Misc functions */
void
gnomemeeting_prefs_window_sound_events_list_build (GtkTreeView *tree_view)
{
  GtkTreeSelection *selection = NULL;
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter, selected_iter;

  BOOL enabled = FALSE;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter))
    path = gtk_tree_model_get_path (model, &selected_iter);

  gtk_list_store_clear (GTK_LIST_STORE (model));
  
  /* Sound on incoming calls */
  enabled = gconf_get_bool (SOUND_EVENTS_KEY "enable_incoming_call_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play sound on incoming calls"),
		      2, SOUND_EVENTS_KEY "incoming_call_sound",
		      3, SOUND_EVENTS_KEY "enable_incoming_call_sound",
		      -1);

  enabled = gconf_get_bool (SOUND_EVENTS_KEY "enable_ring_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play ring tone"),
		      2, SOUND_EVENTS_KEY "ring_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_ring_tone_sound",
		      -1);

  enabled = gconf_get_bool (SOUND_EVENTS_KEY "enable_busy_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play busy tone"),
		      2, SOUND_EVENTS_KEY "busy_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_busy_tone_sound",
		      -1);

  if (!path)
    path = gtk_tree_path_new_from_string ("0");

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (tree_view),
			    path, NULL, false);
  gtk_tree_path_free (path);
}


void
gnomemeeting_codecs_list_build (GtkListStore *codecs_list_store,
				BOOL is_quicknet,
				BOOL software_supported) 
{
  GtkTreeView *tree_view = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;
  GtkTreeIter list_iter;

  GdkEvent *event = NULL;

  int selected_row = 0;
  int current_row = 0;

  gchar *cselect_row = NULL;
  gchar *selected_codec = NULL;
    
  GMH323CodecInfo cdec;

  PString codec;
    
  GSList *codecs_data = NULL;

  codecs_data = gconf_get_string_list (AUDIO_CODECS_KEY "list");

  selected_codec =
    (gchar *) g_object_get_data (G_OBJECT (codecs_list_store), 
				 "selected_codec");

  gtk_list_store_clear (GTK_LIST_STORE (codecs_list_store));

  /* We are adding the codecs */
  while (codecs_data) {

    gchar **couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple [0] && couple [1]) {

      codec = PString (couple [0]);

      if ((!is_quicknet && codec.Find ("G.723.1") != P_MAX_INDEX)
	  || (is_quicknet && !software_supported
	      && codec.Find ("G.723.1") != P_MAX_INDEX
	      && codec.Find ("G.711") != P_MAX_INDEX))
	gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				      couple [0], 0, false, "darkgray");
      else
	gnomemeeting_codecs_list_add (list_iter, codecs_list_store, 
				      couple [0], atoi (couple [1]),
				      true, "black");
    }

    
    if ((selected_codec) && (!strcmp (selected_codec, couple [0]))) 
      selected_row = current_row;

    g_strfreev (couple);
    codecs_data = codecs_data->next;
    current_row++;
  }


  /* Select the right row, and disable if needed the properties button */
  cselect_row = g_strdup_printf("%d", selected_row);
  tree_path = gtk_tree_path_new_from_string (cselect_row);
  tree_view =
    GTK_TREE_VIEW (g_object_get_data (G_OBJECT (codecs_list_store), 
				      "tree_view"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));  
  gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection),
				  tree_path);

  g_free (cselect_row);
  g_slist_free (codecs_data);

  gtk_tree_path_free (tree_path);

  event = gdk_event_new (GDK_BUTTON_PRESS);
  g_signal_emit_by_name (G_OBJECT (tree_view), "event-after", event);
  gdk_event_free (event);
}
                                                                  
                                                                               
static GtkWidget *
gnomemeeting_pref_window_add_update_button (GtkWidget *box,
					    const char *stock_id,
					    const char *label,
					    GtkSignalFunc func,
					    gchar *tooltip,
					    gfloat valign)  
{
  GtkWidget *alignment = NULL;
  GtkWidget *image = NULL;
  GtkWidget *button = NULL;                                                    
  GmPrefWindow *pw = NULL;                                           

  
  pw = GnomeMeeting::Process ()->GetPrefWindow ();                                      

  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  button = gnomemeeting_button_new (label, image);

  alignment = gtk_alignment_new (1, valign, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment), button);
  gtk_container_set_border_width (GTK_CONTAINER (button), 6);

  gtk_box_pack_start (GTK_BOX (box), alignment, TRUE, TRUE, 0);
                                                                               
  g_signal_connect (G_OBJECT (button), "clicked",                          
		    G_CALLBACK (func), (gpointer) pw);

  
  return button;                                                               
}                                                                              
                                                                               
                                                                               
/* BEHAVIOR     :  It builds the container for general settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_general (GtkWidget *window,
				       GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GtkWidget *entry = NULL;

  subsection = gnome_prefs_subsection_new (window, container,
					   _("Personal Information"), 4, 2);
  
  /* Add all the fields */
  entry =
    gnome_prefs_entry_new (subsection, _("_First name:"),
			   PERSONAL_DATA_KEY "firstname",
			   _("Enter your first name"), 0, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("Sur_name:"),
			   PERSONAL_DATA_KEY "lastname",
			   _("Enter your surname"), 1, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("E-_mail address:"),
			   PERSONAL_DATA_KEY "mail",
			   _("Enter your e-mail address"), 2, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("_Comment:"),
			   PERSONAL_DATA_KEY "comment",
			   _("Enter a comment about yourself"), 3, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("_Location:"),
			   PERSONAL_DATA_KEY "location",
			   _("Enter your country or city"), 4, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  
  /* Add the update button */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (personal_data_update_button_clicked), _("Click here to update the user directory you are registered to with the new First Name, Last Name, E-Mail, Comment and Location or to update your alias on the Gatekeeper"), 0);
}                                                                              
                                                                               

/* BEHAVIOR     :  It builds the container for interface settings
 *                 add returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_interface (GtkWidget *window,
					 GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  
  /* GnomeMeeting GUI */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("GnomeMeeting GUI"), 2, 2);

  gnome_prefs_toggle_new (subsection, _("_Show splash screen"), USER_INTERFACE_KEY "show_splash_screen", _("If enabled, the splash screen will be displayed when GnomeMeeting starts"), 0);

  gnome_prefs_toggle_new (subsection, _("Start _hidden"), USER_INTERFACE_KEY "start_hidden", _("If enabled, GnomeMeeting will start hidden provided that the notification area is present in the GNOME panel"), 1);

  
  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (window, container, _("Video Display"), 2, 1);

#ifdef HAS_SDL
  /* Translators: the full sentence is Use a fullscreen size 
     of X by Y pixels */
  gnome_prefs_range_new (subsection, _("Use a fullscreen size of"), NULL, _("by"), NULL, _("pixels"), VIDEO_DISPLAY_KEY "fullscreen_width", VIDEO_DISPLAY_KEY "fullscreen_height", _("The image width for fullscreen."), _("The image height for fullscreen."), 10.0, 10.0, 640.0, 480.0, 10.0, 0);
#endif
  
  gnome_prefs_toggle_new (subsection, _("Place windows displaying video _above other windows"), VIDEO_DISPLAY_KEY "stay_on_top", _("Place windows displaying video above other windows during calls"), 2);

  /* Text Chat */
  subsection =
    gnome_prefs_subsection_new (window, container, _("Text Chat"), 1, 1);
  
  gnome_prefs_toggle_new (subsection, _("Automatically clear the text chat at the end of calls"), USER_INTERFACE_KEY "auto_clear_text_chat", _("If enabled, the text chat will automatically be cleared at the end of calls"), 0);
}


/* BEHAVIOR     :  It builds the container for XDAP directories,
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_directories (GtkWidget *window,
					   GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* Packing widgets for the XDAP directory */
  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("User Directory"), 3, 2);


  /* Add all the fields */                                                     
  gnome_prefs_entry_new (subsection, _("User directory:"), LDAP_KEY "server", _("The user directory server to register with"), 0, true);

  gnome_prefs_toggle_new (subsection, _("Enable _registering"), LDAP_KEY "enable_registering", _("If enabled, register with the selected user directory"), 1);

  gnome_prefs_toggle_new (subsection, _("_Publish my details in the users directory when registering"), LDAP_KEY "show_details", _("If enabled, your details are shown to people browsing the user directory. If disabled, you are not visible to users browsing the user directory, but they can still use the callto URL to call you."), 2);
}


/* BEHAVIOR     :  It builds the container for call forwarding,
 *                 and returns it.
 * PRE          :  /                                             
 */                                                                            
static void
gnomemeeting_init_pref_window_call_forwarding (GtkWidget *window,
					       GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Call Forwarding"), 4, 2);


  /* Add all the fields */                                                     
  entry =
    gnome_prefs_entry_new (subsection, _("Forward calls to _host:"), CALL_FORWARDING_KEY "forward_host", _("The host where calls should be forwarded to in the cases selected above"), 0, true);
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), GMURL ().GetDefaultURL ());
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);  
  
  gnome_prefs_toggle_new (subsection, _("_Always forward calls to the given host"), CALL_FORWARDING_KEY "always_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above"), 1);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _no answer"), CALL_FORWARDING_KEY "forward_on_no_answer", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you do not answer the call"), 2);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _busy"), CALL_FORWARDING_KEY "forward_on_busy", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you already are in a call or if you are in Do Not Disturb mode"), 3);
}


/* BEHAVIOR     :  It builds the container for call control,
 *                 and returns it.
 * PRE          :  /                                             
 */                                                                            
static void
gnomemeeting_init_pref_window_call_options (GtkWidget *window,
					    GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  GmPrefWindow *pw = NULL;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Call Options"), 2, 3);


  /* Add all the fields */
  gnome_prefs_toggle_new (subsection, _("Automatically _clear calls after 30 seconds of inactivity"), CALL_OPTIONS_KEY "clear_inactive_calls", _("If enabled, calls for which no audio and video has been received in the last 30 seconds are automatically cleared"), 0);  

  /* Translators: the full sentence is Reject or forward
     unanswered incoming calls after X s (seconds) */
  gnome_prefs_spin_new (subsection, _("Reject or forward unanswered incoming calls after "), CALL_OPTIONS_KEY "no_answer_timeout", _("Automatically reject or forward incoming calls if no answer is given after the specified amount of time (in seconds)"), 10.0, 299.0, 1.0, 1, _("seconds"), true);
}


/* BEHAVIOR     :  It builds the container for gnomemeeting sound events
 *                 and returns it.
 * PRE          :  /                                             
 */                                                                            
static void
gnomemeeting_init_pref_window_sound_events (GtkWidget *window,
					    GtkWidget *container)
{
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *button = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkCellRenderer *renderer = NULL;

  GmPrefWindow *pw = NULL;


  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  
  subsection = gnome_prefs_subsection_new (window, container,
					   _("GnomeMeeting Sound Events"), 
					   1, 1);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (subsection), vbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
                    0, 0);
  
  /* The 3rd column will be invisible and contain the GConf key containing
     the file to play. The 4th one contains the key determining if the
     sound event is enabled or not. */
  list_store =
    gtk_list_store_new (4,
			G_TYPE_BOOLEAN,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING);

  pw->sound_events_list =
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pw->sound_events_list), TRUE);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), pw->sound_events_list);
  gtk_container_set_border_width (GTK_CONTAINER (pw->sound_events_list), 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     0,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (sound_event_toggled_cb), 
		    GTK_TREE_MODEL (list_store));
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     1,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 325);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     2,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (column), FALSE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     3,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (column), FALSE);

  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  
  label = gtk_label_new (_("Sound to play:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  
  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 2);
  
  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (browse_button_clicked_cb),
		    (gpointer) entry);

  button = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (sound_event_clicked_cb),
		    (gpointer) entry);
  
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (sound_event_play_clicked_cb),
		    (gpointer) entry);

  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (sound_event_changed_cb),
		    (gpointer) entry);

  
  /* Place it after the signals so that we can make sure they are run if
     required */
  gnomemeeting_prefs_window_sound_events_list_build (GTK_TREE_VIEW (pw->sound_events_list));
}


/* BEHAVIOR     :  It builds the container for the H.323 advanced settings
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_h323_advanced (GtkWidget *window,
					     GtkWidget *container)
{
  GmPrefWindow *pw = NULL;
  
  GtkWidget *subsection = NULL;

  gchar *capabilities [] = {_("All"),
			    _("None"),
			    _("rfc2833"),
			    _("Signal"),
			    _("String"),
			    NULL};

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  
  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("H.323 Version 2 Settings"), 3, 1);

  /* The toggles */
  gnome_prefs_toggle_new (subsection, _("Enable H.245 _tunneling"), H323_ADVANCED_KEY "enable_h245_tunneling", _("This enables H.245 Tunneling mode. In H.245 Tunneling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunneling was introduced in H.323v2 and Netmeeting does not support it. Using both Fast Start and H.245 Tunneling can crash some versions of Netmeeting."), 0);

   gnome_prefs_toggle_new (subsection, _("Enable _early H.245"), H323_ADVANCED_KEY "enable_early_h245", _("This enables H.245 early in the setup"), 1);

  gnome_prefs_toggle_new (subsection, _("Enable fast _start procedure"), H323_ADVANCED_KEY "enable_fast_start", _("Connection will be established in Fast Start mode. Fast Start is a new way to start calls faster that was introduced in H.323v2. It is not supported by Netmeeting and using both Fast Start and H.245 Tunneling can crash some versions of Netmeeting."), 2);

  
  /* Packing widget */                                                         
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("DTMF Sending"), 1, 1);

  gnome_prefs_int_option_menu_new (subsection, _("_Send DTMF as:"), capabilities, H323_ADVANCED_KEY "dtmf_sending", _("This permits to set the mode for DTMFs sending. The values can be \"All\", \"None\", \"rfc2833\", \"Signal\" or \"String\" (default is \"All\"). Choosing other values than \"All\", \"String\" or \"rfc2833\" disables the Text Chat."), 0);
}                               


/* BEHAVIOR     :  It builds the container for the gatekeeper settings
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_gatekeeper (GtkWidget *window,
					  GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  gchar *options [] = {_("Do not register"), 
		       _("Gatekeeper host"), 
		       _("Gatekeeper ID"), 
		       _("Automatically discover"), 
		       NULL};

  
  /* Add fields for the gatekeeper */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Gatekeeper"), 4, 3);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _ID:"), H323_GATEKEEPER_KEY "id", _("The Gatekeeper identifier to register with"), 1, false);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _host:"), H323_GATEKEEPER_KEY "host", _("The Gatekeeper host to register with"), 2, false);

  gnome_prefs_entry_new (subsection, _("Gatekeeper _alias:"), H323_GATEKEEPER_KEY "alias", _("The Gatekeeper alias to use when registering (string, or E164 ID if only 0123456789#)"), 3, false);

  entry =
    gnome_prefs_entry_new (subsection, _("Gatekeeper _password:"), H323_GATEKEEPER_KEY "password", _("The Gatekeeper password to use for H.235 authentication to the Gatekeeper"), 4, false);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);

  gnome_prefs_toggle_new (subsection, _("Register this alias as the primary alias with the gatekeeper"), H323_GATEKEEPER_KEY "register_alias_as_primary", _("Use this option to ensure the above alias is used as the primary alias when registering with a gatekeeper. This may be required if your gatekeeper can only perform authentication using the first alias in the list."), 5);
  
  gnome_prefs_int_option_menu_new (subsection, _("Registering method:"), options, H323_GATEKEEPER_KEY "registering_method", _("The registering method to use"), 0);

  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (gatekeeper_update_button_clicked), _("Click here to update your Gatekeeper settings"), 0);
}


/* BEHAVIOR     :  It builds the container for the gateway/proxy settings
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_gateway (GtkWidget *window,
				       GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* Add fields for the gatekeeper */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Gateway/Proxy"), 2, 2);

  gnome_prefs_entry_new (subsection, _("Gateway / Proxy host:"), H323_GATEWAY_KEY "host", _("The Gateway host is the host to use to do H.323 calls through a gateway that will relay calls"), 1, false);

  gnome_prefs_toggle_new (subsection, _("Use gateway or proxy"), H323_GATEWAY_KEY "use_gateway", _("Use the specified gateway to do calls"), 2);
}


/* BEHAVIOR     :  It builds the container for NAT support
 *                 and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_nat (GtkWidget *window,
				   GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* IP translation */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("NAT/PAT Router Support"), 3, 1);

  gnome_prefs_toggle_new (subsection, _("Enable IP _translation"), NAT_KEY "enable_ip_translation", _("This enables IP translation. IP translation is useful if GnomeMeeting is running behind a NAT/PAT router. You have to put the public IP of the router in the field below. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service. If your router natively supports H.323, you can disable this."), 1);

  gnome_prefs_toggle_new (subsection, _("Enable _automatic IP checking"), NAT_KEY "enable_ip_checking", _("This enables IP checking from seconix.com and fills the IP in the public IP of the NAT/PAT gateway field of GnomeMeeting. The returned IP is only used when IP Translation is enabled. If you disable IP checking, you will have to manually enter the IP of your gateway in the GnomeMeeting preferences."), 2);
  
  gnome_prefs_entry_new (subsection, _("Public _IP of the NAT/PAT router:"), NAT_KEY "public_ip", _("Enter the public IP of your NAT/PAT router if you want to use IP translation. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service."), 3, false);
}


/* BEHAVIOR     :  It builds the container for the audio devices 
 *                 settings and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_audio_devices (GtkWidget *window,
					     GtkWidget *container)
{
  GmWindow *gw = NULL;
  GmPrefWindow *pw = NULL;
  
  GtkWidget *entry = NULL;  
  GtkWidget *subsection = NULL;

  gchar **array = NULL;
  gchar *aec [] = {_("Off"),
		   _("Low"),
		   _("Medium"),
		   _("High"),
		   _("AGC"),
		   NULL};

  gchar *types_array [] = {_("POTS"),
			   _("Headset"),
			   NULL};


  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  

  subsection = gnome_prefs_subsection_new (window, container,
					   _("Audio Plugin"), 1, 2);
                                                                               
  /* Add all the fields for the audio manager */
  array = gw->audio_managers.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Audio plugin:"), array, AUDIO_DEVICES_KEY "plugin", _("The audio plugin that will be used to detect the devices and manage them."), 0);
  free (array);


  /* Add all the fields */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Audio Devices"), 4, 2);
                                                                               

  /* The player */
  array = gw->audio_player_devices.ToCharArray ();
  pw->audio_player =
    gnome_prefs_string_option_menu_new (subsection, _("Output device:"), array, AUDIO_DEVICES_KEY "output_device", _("Select the audio output device to use"), 0);
  free (array);
  
  /* The recorder */
  array = gw->audio_recorder_devices.ToCharArray ();
  pw->audio_recorder =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), array, AUDIO_DEVICES_KEY "input_device", _("Select the audio input device to use"), 2);
  free (array);

#ifdef HAS_IXJ
  /* The Quicknet devices related options */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Quicknet Hardware"), 3, 2);
  
  gnome_prefs_int_option_menu_new (subsection, _("Echo _cancellation:"), aec, AUDIO_DEVICES_KEY "lid_echo_cancellation_level", _("The Automatic Echo Cancellation level: Off, Low, Medium, High, Automatic Gain Compensation. Choosing Automatic Gain Compensation modulates the volume for best quality."), 0);

  gnome_prefs_int_option_menu_new (subsection, _("Output device type:"), types_array, AUDIO_DEVICES_KEY "lid_output_device_type", _("The output device type is the type of device connected to your Quicknet card. It can be either a POTS (Plain Old Telephone System) or a headset."), 1);
  
  entry =
    gnome_prefs_entry_new (subsection, _("Country _code:"), AUDIO_DEVICES_KEY "lid_country_code", _("The two-letter country code of your country (e.g.: BE, UK, FR, DE, ...)."), 2, false);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 2);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 100, -1);
#endif

  
  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices_list_button_clicked), _("Click here to refresh the devices list"), 1);
}


/* BEHAVIOR     :  It builds the container for the video devices 
 *                 settings and returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_video_devices (GtkWidget *window,
					     GtkWidget *container)
{
  GmWindow *gw = NULL;
  GmPrefWindow *pw = NULL;
  
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  GtkWidget *button = NULL;

  gchar **array = NULL;
  gchar *video_size [] = {_("Small"),
			  _("Large"), 
			  NULL};
  gchar *video_format [] = {_("PAL (Europe)"), 
			    _("NTSC (America)"), 
			    _("SECAM (France)"), 
			    _("Auto"), 
			    NULL};


  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  

  /* The video manager */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Video Plugin"), 1, 2);

  array = gw->video_managers.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Video plugin:"), array, VIDEO_DEVICES_KEY "plugin", _("The video plugin that will be used to detect the devices and manage them"), 0);
  free (array);


  /* The video devices related options */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Video Devices"), 5, 3);

  /* The video device */
  array = gw->video_devices.ToCharArray ();
  pw->video_device =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), array, VIDEO_DEVICES_KEY "input_device", _("Select the video input device to use. If an error occurs when using this device a test picture will be transmitted."), 0);
  free (array);
  
  /* Video Channel */
  gnome_prefs_spin_new (subsection, _("Channel:"), VIDEO_DEVICES_KEY "channel", _("The video channel number to use (to select camera, tv or other sources)"), 0.0, 10.0, 1.0, 3, NULL, false);
  
  gnome_prefs_int_option_menu_new (subsection, _("Size:"), video_size, VIDEO_DEVICES_KEY "size", _("Select the transmitted video size: Small (QCIF 176x144) or Large (CIF 352x288)"), 1);

  gnome_prefs_int_option_menu_new (subsection, _("Format:"), video_format, VIDEO_DEVICES_KEY "format", _("Select the format for video cameras (does not apply to most USB cameras)"), 2);

  entry =
    gnome_prefs_entry_new (subsection, _("Image:"), VIDEO_DEVICES_KEY "image", _("The image to transmit if \"Picture\" is selected as video plugin or if the opening of the device fails. Leave blank to use the default GnomeMeeting logo."), 4, false);

  /* The file selector button */
  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_table_attach (GTK_TABLE (subsection), button, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (browse_button_clicked_cb),
		    (gpointer) entry);

  /* That button will refresh the devices list */
  gnomemeeting_pref_window_add_update_button (container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices_list_button_clicked), _("Click here to refresh the devices list."), 1);
}


/* BEHAVIOR     :  It builds the container for audio codecs settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_audio_codecs (GtkWidget *window,
					    GtkWidget *container)
{
  GMH323EndPoint *ep = NULL;
  
  GtkWidget *subsection = NULL;
  
  GtkWidget *alignment = NULL;
  GtkWidget *buttons_vbox = NULL;
  GtkWidget *hbox = NULL;
    
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;

  GtkWidget *tree_view = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;                        
                                                       
  BOOL use_quicknet = FALSE;
  BOOL soft_codecs_supported = FALSE;
  
  GmPrefWindow *pw = NULL;

  
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  
  /* Packing widgets */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("Available Audio Codecs"), 1, 1);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (subsection), hbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
                    0, 0);

  pw->codecs_list_store = gtk_list_store_new (COLUMN_CODEC_NUMBER,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING);

  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (pw->codecs_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),0);
  
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_CODEC_ACTIVE,
						     NULL);
  gtk_tree_view_column_add_attribute (column, renderer, "activatable", 
				      COLUMN_CODEC_SELECTABLE);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (codecs_list_fixed_toggled), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Info"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_INFO,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Bandwidth"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_BANDWIDTH,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set_data (G_OBJECT (pw->codecs_list_store), "tree_view",
		     (gpointer) tree_view);


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();
  
  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 2 * GNOMEMEETING_PAD_SMALL);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, 
		      TRUE, TRUE, 2 * GNOMEMEETING_PAD_SMALL);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "up");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "down");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));

  button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codecs_list_info_button_clicked_callback), 
		    GTK_TREE_MODEL (pw->codecs_list_store));  
  gtk_widget_show_all (frame);


  gnomemeeting_codecs_list_build (pw->codecs_list_store,
				  use_quicknet,
				  soft_codecs_supported);


  /* Here we add the audio codecs options */
  subsection = 
    gnome_prefs_subsection_new (window, container,
				_("Audio Codecs Settings"), 2, 1);

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gnome_prefs_range_new (subsection, _("Automatically adjust _jitter buffer between"), NULL, _("and"), NULL, _("ms"), AUDIO_CODECS_KEY "minimum_jitter_buffer", AUDIO_CODECS_KEY "maximum_jitter_buffer", _("The minimum jitter buffer size for audio reception (in ms)."), _("The maximum jitter buffer size for audio reception (in ms)."), 20.0, 20.0, 1000.0, 1000.0, 1.0, 0);
  
  gnome_prefs_toggle_new (subsection, _("Enable silence _detection"), AUDIO_CODECS_KEY "enable_silence_detection", _("If enabled, use silence detection with the GSM and G.711 codecs."), 1);
}
                                                                               

/* BEHAVIOR     :  It builds the container for video codecs settings and
 *                 returns it.
 * PRE          :  /
 */
static void
gnomemeeting_init_pref_window_video_codecs (GtkWidget *window,
					    GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GmPrefWindow *pw = NULL;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  subsection = gnome_prefs_subsection_new (window, container,
					   _("General Settings"), 2, 1);

  
  /* Add fields */
  gnome_prefs_toggle_new (subsection, _("Enable video _transmission"), VIDEO_CODECS_KEY "enable_video_transmission", _("If enabled, video is transmitted during a call."), 0);

  gnome_prefs_toggle_new (subsection, _("Enable video _reception"), VIDEO_CODECS_KEY "enable_video_reception", _("If enabled, allows video to be received during a call."), 1);


  /* H.261 Settings */
  subsection = gnome_prefs_subsection_new (window, container,
					   _("Bandwidth Control"), 1, 1);

  /* Translators: the full sentence is Maximum video bandwidth of X kB/s */
  gnome_prefs_spin_new (subsection, _("Maximum video _bandwidth of"), VIDEO_CODECS_KEY "maximum_video_bandwidth", _("The maximum video bandwidth in kbytes/s. The video quality and the number of transmitted frames per second will be dynamically adjusted above their minimum during calls to try to minimize the bandwidth to the given value."), 2.0, 100.0, 1.0, 0, _("kB/s"), true);
  

  /* Advanced quality settings */
  subsection =
    gnome_prefs_subsection_new (window, container,
				_("Advanced Quality Settings"), 3, 1);
  
  /* Translators: the full sentence is Keep a minimum video quality of X % */
  gnome_prefs_spin_new (subsection, _("Keep a minimum video _quality of"), VIDEO_CODECS_KEY "transmitted_video_quality", _("The minimum transmitted video quality to keep when trying to minimize the used bandwidth:  choose 100% on a LAN for the best quality, 1% being the worst quality"), 1.0, 100.0, 1.0, 0, _("%"), true);

  /* Translators: the full sentence is Transmit at least X frames per second */
  gnome_prefs_spin_new (subsection, _("Transmit at least"), VIDEO_CODECS_KEY "transmitted_fps", _("The minimum number of video frames to transmit each second when trying to minimize the bandwidth"), 1.0, 30.0, 1.0, 1, _("_frames per second"), true);
				 
  /* Translators: the full sentence is Transmit X background blocks with each
     frame */
  gnome_prefs_spin_new (subsection, _("Transmit"), VIDEO_CODECS_KEY "transmitted_background_blocks", _("Choose the number of blocks (that have not changed) transmitted with each frame. These blocks fill in the background"), 1.0, 99.0, 1.0, 2, _("background _blocks with each frame"), true);
}


void 
gnomemeeting_pref_window_update_devices_list ()
{
  GmPrefWindow *pw = NULL;

  gchar **array = NULL;
  
  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  /* The player */
  array = gw->audio_player_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_player,
					 array,
					 AUDIO_DEVICES_KEY "output_device");
  free (array);
  
  /* The recorder */
  array = gw->audio_recorder_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_recorder,
					 array,
					 AUDIO_DEVICES_KEY "input_device");
  free (array);
  
  
  /* The Video player */
  array = gw->video_devices.ToCharArray ();

  gnome_prefs_string_option_menu_update (pw->video_device,
					 array,
					 VIDEO_DEVICES_KEY "input_device");
  free (array);
}


GtkWidget *
gnomemeeting_pref_window_new (GmPrefWindow *pw)
{
  GtkWidget *window = NULL;
  GtkWidget *container = NULL;
  
  window = 
    gnome_prefs_window_new (GNOMEMEETING_IMAGES "/gnomemeeting-logo.png");
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("preferences_window"), g_free);
  gtk_window_set_title (GTK_WINDOW (window), "");
  
  gnome_prefs_window_section_new (window, _("General"));
  container = gnome_prefs_window_subsection_new (window, _("Personal Data"));
  gnomemeeting_init_pref_window_general (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));
		       
  container = gnome_prefs_window_subsection_new (window,
						 _("General Settings"));
  gnomemeeting_init_pref_window_interface (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));
  
  container = gnome_prefs_window_subsection_new (window,
						 _("Directory Settings"));
  gnomemeeting_init_pref_window_directories (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));
  
  container = gnome_prefs_window_subsection_new (window, _("Call Options"));
  gnomemeeting_init_pref_window_call_options (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("NAT Settings"));
  gnomemeeting_init_pref_window_nat (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Sound Events"));
  gnomemeeting_init_pref_window_sound_events (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("H.323 Settings"));
  container = gnome_prefs_window_subsection_new (window,
						 _("Advanced Settings"));
  gnomemeeting_init_pref_window_h323_advanced (window, container);          
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Call Forwarding"));
  gnomemeeting_init_pref_window_call_forwarding (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Gatekeeper Settings"));
  gnomemeeting_init_pref_window_gatekeeper (window, container);          
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Gateway / Proxy Settings"));
  gnomemeeting_init_pref_window_gateway (window, container);          
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Codecs"));

  container = gnome_prefs_window_subsection_new (window, _("Audio Codecs"));
  gnomemeeting_init_pref_window_audio_codecs (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));
  
  container = gnome_prefs_window_subsection_new (window, _("Video Codecs"));
  gnomemeeting_init_pref_window_video_codecs (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Devices"));
  container = gnome_prefs_window_subsection_new (window, _("Audio Devices"));
  gnomemeeting_init_pref_window_audio_devices (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));
  
  container = gnome_prefs_window_subsection_new (window, _("Video Devices"));
  gnomemeeting_init_pref_window_video_devices (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), 
                    "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);

  return window;
}

