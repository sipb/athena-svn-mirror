
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
 *                         tools.cpp  -  description
 *                         -------------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the simple
 *                          tools of the tools menu.
 *
 */


#include "../config.h"

#include "tools.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "ldap_window.h"
#include "misc.h"

#include "gconf_widgets_extensions.h"
#include "gnome_prefs_window.h"
#include "stock-icons.h"


extern GtkWidget *gm;

static void pc_to_phone_window_response_cb (GtkWidget *,
					    gint,
					    gpointer);

static void microtelco_consult_cb (GtkWidget *,
				   gpointer);

static void dnd_drag_data_get_cb (GtkWidget *,
				  GdkDragContext *,
				  GtkSelectionData *,
				  guint,
				  guint,
				  gpointer);

static void clear_button_clicked_cb (GtkButton *,
				     gpointer);

static void find_button_clicked_cb (GtkButton *,
				    gpointer);


/* DESCRIPTION  :  This callback is called when the user validates an answer
 *                 to the PC-To-Phone window.
 * BEHAVIOR     :  Hide the window (if not Apply), and apply the settings
 *                 (if not cancel), ie change the settings and register to gk.
 * PRE          :  /
 */
static void
pc_to_phone_window_response_cb (GtkWidget *w,
				gint response,
				gpointer data)
{
  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (response != 1)
    gnomemeeting_window_hide (w);
  

  if (response == 1 || response == 0) {

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data))) {
	
      /* The Username and PIN already are correct, update the other settings */
      gconf_set_bool (H323_ADVANCED_KEY "enable_fast_start", TRUE);
      gconf_set_bool (H323_ADVANCED_KEY "enable_h245_tunneling", TRUE);
      gconf_set_bool (H323_ADVANCED_KEY "enable_early_h245", TRUE);
      gconf_set_string (H323_GATEKEEPER_KEY "host", "gk.microtelco.com");
      gconf_set_int (H323_GATEKEEPER_KEY "registering_method", 1);
    }
    else
      gconf_set_int (H323_GATEKEEPER_KEY "registering_method", 0);
    
    /* Register the current Endpoint to the Gatekeeper */
    ep->GatekeeperRegister ();
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks on the link
 *                 button to consult his account details.
 * BEHAVIOR     :  Builds a filename with autopost html in /tmp/ and opens it
 *                 with the GNOME preferred browser.
 * PRE          :  /
 */
static void
microtelco_consult_cb (GtkWidget *widget,
		       gpointer data)
{
  gchar *tmp_filename = NULL;
  gchar *filename = NULL;
  gchar *account = NULL;
  gchar *pin = NULL;
  gchar *buffer = NULL;
  
  int fd = -1;

  account = gconf_get_string (H323_GATEKEEPER_KEY "alias");
  pin = gconf_get_string (H323_GATEKEEPER_KEY "password");

  if (!account || !pin)
    return;
  
  buffer =
    g_strdup_printf ("<HTML><HEAD><TITLE>MicroTelco Auto-Post</TITLE></HEAD>"
		     "<BODY BGCOLOR=\"#FFFFFF\" "
		     "onLoad=\"Javascript:document.autoform.submit()\">"
		     "<FORM NAME=\"autoform\" "
		     "ACTION=\"https://%s.an.microtelco.com/acct/Controller\" "
		     "METHOD=\"POST\">"
		     "<input type=\"hidden\" name=\"command\" value=\"caller_login\">"
		     "<input type=\"hidden\" name=\"caller_id\" value=\"%s\">"
		     "<input type=\"hidden\" name=\"caller_pin\" value=\"%s\">"
		     "</FORM></BODY></HTML>", account, account, pin);

  fd = g_file_open_tmp ("mktmicro-XXXXXX", &tmp_filename, NULL);
  filename = g_strdup_printf ("file:///%s", tmp_filename);
  
  write (fd, (char *) buffer, strlen (buffer)); 
  close (fd);
  
  gnome_url_show (filename, NULL);

  g_free (tmp_filename);
  g_free (filename);
  g_free (buffer);
  g_free (account);
  g_free (pin);
}


/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Puts the required data into the selection_data, we put
 *                 name and the url fields for now.
 * PRE          :  data = the type of the page from where the drag occured :
 *                 CONTACTS_GROUPS or CONTACTS_SERVERS.
 */
static void
dnd_drag_data_get_cb (GtkWidget *tree_view,
		      GdkDragContext *dc,
		      GtkSelectionData *selection_data,
		      guint info,
		      guint t,
		      gpointer data)
{
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;
  gchar *drag_data = NULL;

        
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			1, &contact_name,
			2, &contact_url, -1);

    if (contact_name && contact_url) {
      
      drag_data = g_strdup_printf ("%s|%s", contact_name, contact_url);
    
      gtk_selection_data_set (selection_data,
			      selection_data->target, 
			      8,
			      (const guchar *) drag_data,
			      strlen (drag_data));
      g_free (drag_data);
    }
  }
  
  g_free (contact_name);
  g_free (contact_url);
}


/* DESCRIPTION  :  This callback is called when the user has clicked the clear
 *                 button.
 * BEHAVIOR     :  Clears the corresponding calls list using the GConf DB.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void
clear_button_clicked_cb (GtkButton *b, gpointer data)
{
  if (!data)
    return;
  
  switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (data))) {

  case RECEIVED_CALL:
    gconf_set_string_list (USER_INTERFACE_KEY "calls_history_window/received_calls_history", NULL);
    break;
  case PLACED_CALL:
    gconf_set_string_list (USER_INTERFACE_KEY "calls_history_window/placed_calls_history", NULL);
    break;
  case MISSED_CALL:
    gconf_set_string_list (USER_INTERFACE_KEY "calls_history_window/missed_calls_history", NULL);
    break;
  }
}


/* DESCRIPTION  :  This callback is called when the user has clicked the find
 *                 button.
 * BEHAVIOR     :  Hides the rows that do not contain the text in the search
 *                 entry.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void
find_button_clicked_cb (GtkButton *b, gpointer data)
{ 
  GmCallsHistoryWindow *chw = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeIter iter;
  const char *entry_text = NULL;
  gchar *date = NULL;
  gchar *software = NULL;
  gchar *remote_user = NULL;
  gchar *end_reason = NULL;

  BOOL removed = FALSE;
  BOOL ok = FALSE;
  
  if (!data)
    return;

  /* Fill in the window */
  gnomemeeting_calls_history_window_populate ();
  
  chw = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  entry_text = gtk_entry_get_text (GTK_ENTRY (chw->search_entry));

  switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (data))) {
    
  case RECEIVED_CALL:
    list_store = chw->received_calls_list_store;
    break;
  case PLACED_CALL:
    list_store = chw->given_calls_list_store;
    break;
  case MISSED_CALL:
    list_store = chw->missed_calls_list_store;
    break;
  }

  if (strcmp (entry_text, "")
      && gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) {

    do {

      ok = FALSE;
      removed = FALSE;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store),
			  &iter,
			  0, &date,
			  1, &remote_user,
			  4, &end_reason,
			  5, &software,
			  -1);

      if (!(PString (date).Find (entry_text) != P_MAX_INDEX
	  || PString (remote_user).Find (entry_text) != P_MAX_INDEX
	  || PString (end_reason).Find (entry_text) != P_MAX_INDEX
	    || PString (software).Find (entry_text) != P_MAX_INDEX)) {
	
	ok = gtk_list_store_remove (GTK_LIST_STORE (list_store), &iter);
	removed = TRUE;
      }
      
      g_free (date);
      g_free (remote_user);
      g_free (end_reason);
      g_free (software);

      if (!removed)
	ok = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
	
    } while (ok);
  }
}


/* The functions */
void
gnomemeeting_calls_history_window_populate ()
{
  GtkTreeIter iter;
  GtkListStore *list_store = NULL;

  gchar *gconf_key = NULL;
  gchar **call_data = NULL;
  
  GSList *calls_list = NULL;

  GmCallsHistoryWindow *chw = NULL;

  chw = GnomeMeeting::Process ()->GetCallsHistoryWindow ();

  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {
    
    switch (i) {

    case RECEIVED_CALL:
      list_store = chw->received_calls_list_store;
      gconf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/received_calls_history");
      break;
    case PLACED_CALL:
      list_store = chw->given_calls_list_store;
      gconf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/placed_calls_history");
      break;
    case MISSED_CALL:
      list_store = chw->missed_calls_list_store;
      gconf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/missed_calls_history");
      break;
    }

    gtk_list_store_clear (list_store);
    
    calls_list = gconf_get_string_list (gconf_key);

    while (calls_list && calls_list->data) {
      
      call_data = g_strsplit ((char *) calls_list->data, "|", 0);
      
      if (call_data) {
	
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store,
			    &iter,
			    0, call_data [0],
			    1, call_data [1],
			    2, call_data [2],
			    3, call_data [3],
			    4, call_data [4],
			    5, call_data [5],
			    -1);
      }
      
      g_strfreev (call_data);

      calls_list = g_slist_next (calls_list);
    }
    
    g_free (gconf_key);
    g_slist_free (calls_list);
  }
}


void
gnomemeeting_calls_history_window_add_call (int i,
					    const char *remote_user,
					    const char *ip,
					    const char *duration,
					    const char *reason,
					    const char *software)
{
  PString time;

  gchar *gconf_key = NULL;
  gchar *call_data = NULL;
  
  GSList *calls_list = NULL;
  GSList *tmp = NULL;
  
  time = PTime ().AsString ("yyyy/MM/dd hh:mm:ss");
  
  switch (i) {

  case RECEIVED_CALL:
    gconf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/received_calls_history");
    break;
  case PLACED_CALL:
    gconf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/placed_calls_history");
    break;
  case MISSED_CALL:
    gconf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/missed_calls_history");
    break;
  }

  
  call_data =
    g_strdup_printf ("%s|%s|%s|%s|%s|%s",
		     (const char *) time ? (const char *) time : "",
		     remote_user ? remote_user : "",
		     ip ? ip : "",
		     duration ? duration : "",
		     reason ? reason : "",
		     software ? software : "");
  
  calls_list = gconf_get_string_list (gconf_key);
  calls_list = g_slist_append (calls_list, (gpointer) call_data);

  while (g_slist_length (calls_list) > 100) {

    tmp = g_slist_nth (calls_list, 0);
    calls_list = g_slist_remove_link (calls_list, tmp);

    g_slist_free_1 (tmp);
  }
  
  gconf_set_string_list (gconf_key, calls_list);
  
  g_free (gconf_key);
  g_slist_free (calls_list);
}


GtkWidget *
gnomemeeting_calls_history_window_new (GmCallsHistoryWindow *chw)
{
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;
  GtkWidget *window = NULL;
  GtkWidget *notebook = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GtkWidget *tree_view = NULL;
  GdkPixbuf *icon = NULL;
  
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkListStore *list_store [3];
  
  gchar *label_text [3] =
    {N_("Received Calls"), N_("Placed Calls"), N_("Unanswered Calls")};
  label_text [0] = gettext (label_text [0]);
  label_text [1] = gettext (label_text [1]);
  label_text [2] = gettext (label_text [2]);

  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };

  
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("calls_history_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("Calls History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_CALLS_HISTORY,
				 GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  g_object_unref (icon);

  
  /* The notebook containing the 3 lists of calls */
  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), notebook,
		      TRUE, TRUE, 0);


  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {

    label = gtk_label_new (N_(label_text [i]));
    
    scr = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), 
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    list_store [i] = 
      gtk_list_store_new (6,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING);
    
    tree_view = 
      gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store [i]));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Date"),
						       renderer,
						       "text", 
						       0,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Remote User"),
						       renderer,
						       "text", 
						       1,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);
        
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       2,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    gtk_tree_view_column_set_visible (column, false);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call Duration"),
						       renderer,
						       "text", 
						       3,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    if (i == 2)
      gtk_tree_view_column_set_visible (column, false);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call End Reason"),
						       renderer,
						       "text", 
						       4,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Software"),
						       renderer,
						       "text", 
						       5,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
	
    
    gtk_container_add (GTK_CONTAINER (scr), tree_view);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scr, label);


    /* Signal to call the person on the double-clicked row */
    g_signal_connect (G_OBJECT (tree_view), "row_activated", 
		      G_CALLBACK (contact_activated_cb), GINT_TO_POINTER (3));

    /* The drag and drop information */
    gtk_drag_source_set (GTK_WIDGET (tree_view),
			 GDK_BUTTON1_MASK, dnd_targets, 1,
			 GDK_ACTION_COPY);
    g_signal_connect (G_OBJECT (tree_view), "drag_data_get",
		      G_CALLBACK (dnd_drag_data_get_cb), NULL);

    /* Right-click on a contact */
    g_signal_connect (G_OBJECT (tree_view), "event_after",
		    G_CALLBACK (contact_clicked_cb), GINT_TO_POINTER (1));
  }

  chw->received_calls_list_store = list_store [0];
  chw->given_calls_list_store = list_store [1];
  chw->missed_calls_list_store = list_store [2];


  /* The hbox added below the notebook that contains the Search field,
     and the search and clear buttons */
  hbox = gtk_hbox_new (FALSE, 0);  
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  chw->search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), chw->search_entry, TRUE, TRUE, 2);
  g_signal_connect (G_OBJECT (chw->search_entry), "activate",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) notebook);  
  
  button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) notebook);

  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (clear_button_clicked_cb),
		    (gpointer) notebook);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), hbox,
		      FALSE, FALSE, 0); 

  
  /* Generic signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);

  
  /* Fill in the window with old calls */
  gnomemeeting_calls_history_window_populate ();

  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}


GtkWidget *
gnomemeeting_pc_to_phone_window_new ()
{
  GtkWidget *window = NULL;
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;
  GtkWidget *use_service_button = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *href = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;

  gchar *txt = NULL;
  
  window = gtk_dialog_new ();
  gtk_dialog_add_buttons (GTK_DIALOG (window),
			  GTK_STOCK_APPLY,  1,
			  GTK_STOCK_CANCEL, 2,
			  GTK_STOCK_OK, 0,
			  NULL);
  
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("pc_to_phone_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("PC-To-Phone Settings"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  label = gtk_label_new (_("You can make calls to regular phones and cell numbers worldwide using GnomeMeeting and the MicroTelco service from Quicknet Technologies. To enable this you need to enter your MicroTelco Account number and PIN below, then enable registering to the MicroTelco service. Please visit the GnomeMeeting website for more information."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label,
		      FALSE, FALSE, 20);


  vbox = GTK_DIALOG (window)->vbox;
  
  subsection =
    gnome_prefs_subsection_new (window, vbox,
				_("PC-To-Phone Settings"), 2, 1);

  gnome_prefs_entry_new (subsection, _("Account _number:"), H323_GATEKEEPER_KEY "alias", _("Use your MicroTelco account number"), 1, false);

  entry =
    gnome_prefs_entry_new (subsection, _("_Pin:"), H323_GATEKEEPER_KEY "password", _("Use your MicroTelco PIN"), 2, false);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);

  use_service_button =
    gtk_check_button_new_with_label (_("Use PC-To-Phone service"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (use_service_button),
		      FALSE, TRUE, 0);
  
  label =
    gtk_label_new (_("Click on one of the following links to get more information about your existing MicroTelco account, or to create a new account."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 20);
  href = gnome_href_new ("http://www.linuxjack.com", _("Get an account"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  href = gnome_href_new ("http://www.linuxjack.com", _("Buy a card"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult my account details"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (microtelco_consult_cb), NULL);
				
  g_signal_connect (GTK_OBJECT (window), 
		    "response", 
		    G_CALLBACK (pc_to_phone_window_response_cb),
		    (gpointer) use_service_button);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));

  return window;
}

GtkWidget *
gnomemeeting_log_window_new ()
{
  GtkWidget *window = NULL;
  GtkWidget *scr = NULL;
  GtkTextMark *mark = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter end;

  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  /* Fix me, create a structure for that so that we don't use
     gw here */
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("log_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("General History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  
  gw->log_text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->log_text_view), 
			      FALSE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->log_text_view), 
			      FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (gw->log_text_view),
			       GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (gw->log_text_view), 
				    FALSE);

  buffer = 
    gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw->log_text_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 6);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), gw->log_text_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), scr,
		      TRUE, TRUE, 0);
  
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}

void 
gnomemeeting_log_insert (const char *format,
			 ...)
{
  va_list args;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();
  GtkWidget *text_view = gw->log_text_view;
  GtkTextIter end;
  GtkTextMark *mark;
  GtkTextBuffer *buffer;
	
  time_t *timeptr;
  char *time_str;
  gchar *text_buffer = NULL;
  char buf [1025];

  if (!text_view || !format)
    return;
      
  va_start (args, format);

  vsnprintf (buf, 1024, format, args);

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  text_buffer = g_strdup_printf ("%s %s\n", time_str, buf);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &end, -1);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, text_buffer, -1);

  mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (buffer), 
				   "current-position");

  if (mark)
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_view), mark, 
 				  0.0, FALSE, 0,0);
  
  g_free (text_buffer);
  free (time_str);
  delete (timeptr);
}
