
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
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#include "../config.h"

#include "chat_window.h"
#include "ldap_window.h"
#include "gnomemeeting.h"
#include "ldap_window.h"
#include "callbacks.h"
#include "misc.h"
#include "menu.h"
#include "callbacks.h"

#include "gtk-text-tag-addon.h"
#include "gtk-text-buffer-addon.h"
#include "gtk-text-view-addon.h"
#include "gtk_menu_extensions.h"

extern GtkWidget *gm;


#ifndef DISABLE_GNOME
/* DESCRIPTION  :  Called when an URL is clicked.
 * BEHAVIOR     :  Displays it with gnome_url_show.
 * PRE          :  /
 */
static void
open_uri_callback (const gchar *uri)
{
  if (uri)
    gnome_url_show (uri, NULL);
}
#endif


/* DESCRIPTION  :  Called when an URL is clicked.
 * BEHAVIOR     :  Set the text in the clipboard.
 * PRE          :  /
 */
static void
copy_uri_callback (const gchar *uri)
{
  if (uri) {
    gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY),
			    uri, -1);
    gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
			    uri, -1);
  }
}


/* DESCRIPTION  :  Called when an URL is clicked.
 * BEHAVIOR     :  Connect to the given URL or transfer the call to that URL.
 * PRE          :  /
 */
static void
connect_uri_callback (const gchar *uri)
{
  GMH323EndPoint *ep = NULL;
  GmWindow *gw = NULL;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
    
  if (uri) {

    if (ep->GetCallingState () == GMH323EndPoint::Standby) {

      gw = GnomeMeeting::Process ()->GetMainWindow ();
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			  uri);
      
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button),
				    true);
    }
    else if (ep->GetCallingState () == GMH323EndPoint::Connected)
      transfer_call_cb (NULL, (gpointer) uri);
  }
}


/* DESCRIPTION  :  Called when an URL has to be added to the addressbook.
 * BEHAVIOR     :  Displays the popup.
 * PRE          :  /
 */
static void
add_uri_callback (const gchar *uri)
{
  if (uri)
    gnomemeeting_addressbook_edit_contact_dialog ((gchar *) uri);
}  


/* DESCRIPTION  :  Called when the chat entry is activated.
 * BEHAVIOR     :  Send the given message to the remote user.
 * PRE          :  /
 */
static void
chat_entry_activate (GtkEditable *w,
		     gpointer data)
{
  GMH323EndPoint *endpoint = GnomeMeeting::Process ()->Endpoint ();
  PString s;
    
  if (endpoint) {
        
    PString local = endpoint->GetLocalUserName ();
    /* The local party name has to be converted to UTF-8, but not
       the text */
    gchar *utf8_local = NULL;

    s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
    
    if (endpoint->GetCallingState () == GMH323EndPoint::Connected
	&& !s.IsEmpty ()) {
            
      /* If the GDK lock is taken, the connection will never get a
	 chance to be established if we lock its mutex */
      gdk_threads_leave ();
      H323Connection *connection = 
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
      gdk_threads_enter ();

      if (connection != NULL)  {
                
	connection->SendUserInput ("MSG"+s);

	if (g_utf8_validate ((gchar *) (const unsigned char*) local, -1, NULL))
	  utf8_local = g_strdup ((char *) (const char *) (local));
	else
	  utf8_local = gnomemeeting_from_iso88591_to_utf8 (local);

	if (utf8_local)
	  gnomemeeting_text_chat_insert (utf8_local, s, 0);
	g_free (utf8_local);
                
	gtk_entry_set_text (GTK_ENTRY (w), "");

	connection->Unlock ();
      }
    }
  }
}

void gnomemeeting_text_chat_clear (GtkWidget *w,
				   GmTextChat *chat)
{
  GmWindow *gw = NULL;
  GtkTextIter start_iter, end_iter;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  gtk_text_buffer_get_start_iter (chat->text_buffer, &start_iter);
  gtk_text_buffer_get_end_iter (chat->text_buffer, &end_iter);

  gtk_text_buffer_delete (chat->text_buffer, &start_iter, &end_iter);

  gtk_menu_set_sensitive (gw->main_menu, "clear_text_chat", FALSE);
}

void
gnomemeeting_text_chat_call_start_notification (void)
{
  GmTextChat *chat = NULL;
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  chat = GnomeMeeting::Process ()->GetTextChat ();
	
  // find the time at which the event occured
  time_t *timeptr;
  char *time_str;
  
  time_str = (char *) malloc (21);
  timeptr = new (time_t);
 
  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  // prepare the message
  if (chat->begin_msg) g_free (chat->begin_msg); // shouldn't happen...
  chat->begin_msg = g_strdup_printf ("---- Call begins at %s\n", time_str);

  // we are ready to trigger the message display
  chat->something_typed = FALSE;
  
  //  free what we should
  g_free (time_str);
}

void
gnomemeeting_text_chat_call_stop_notification (void)
{
  GtkTextIter iter;
  GmTextChat *chat = NULL;
  GmWindow *gw = NULL;
  gchar *text = NULL;

  // find the time at which the event occured	
  time_t *timeptr;
  char *time_str;
  
  time_str = (char *) malloc (21);
  timeptr = new (time_t);
 
  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  // prepare the message to be displayed
  text = g_strdup_printf ("---- Call ends at %s\n", time_str);

  // displays the message
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  chat = GnomeMeeting::Process ()->GetTextChat ();
  gtk_text_buffer_get_end_iter (chat->text_buffer, &iter);

  if (chat->something_typed == TRUE)
    gtk_text_buffer_insert(chat->text_buffer, &iter, text, -1);

  // freeing what we need to
  free (time_str);
  g_free (text);
}

void 
gnomemeeting_text_chat_insert (PString local,
			       PString str,
			       int user)
{
  gchar *msg = NULL;
  GtkTextIter iter;
  GtkTextMark *mark;
  
  GmTextChat *chat = NULL;
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  chat = GnomeMeeting::Process ()->GetTextChat ();

  gtk_text_buffer_get_end_iter (chat->text_buffer, &iter);

  // delayed call begin notification first!
  if (chat->something_typed == FALSE) {
    chat->something_typed = TRUE;
    if (chat->begin_msg) { // should always be true
      gtk_text_buffer_insert(chat->text_buffer, &iter, chat->begin_msg, -1);
      g_free (chat->begin_msg);
      chat->begin_msg = NULL;
    }
  }

  msg = g_strdup_printf ("%s: ", (const char *) local);

  if (user == 1)
    gtk_text_buffer_insert_with_tags_by_name (chat->text_buffer, &iter, msg, 
					      -1, "primary-user", NULL);
  else
    gtk_text_buffer_insert_with_tags_by_name (chat->text_buffer, &iter, msg, 
					      -1, "secondary-user", NULL);
  
  g_free (msg);
  
  gtk_text_buffer_insert_with_regex (chat->text_buffer, &iter, 
					 (const char *) str);

  gtk_text_buffer_insert (chat->text_buffer, &iter, "\n", -1);

  mark = gtk_text_buffer_get_mark (chat->text_buffer, "current-position");

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (chat->text_view), mark, 
				0.0, FALSE, 0,0);

  gtk_menu_set_sensitive (gw->main_menu, "clear_text_chat", TRUE);
}


GtkWidget *
gnomemeeting_text_chat_new (GmTextChat *chat)
{
  GtkWidget *entry = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GtkWidget *table = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *chat_window = NULL;

  GtkTextIter  iter;
  GtkTextMark *mark = NULL;
  GtkTextTag *regex_tag = NULL;


  /* Get the structs from the application */
  chat_window = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (chat_window), GTK_SHADOW_NONE);
  table = gtk_table_new (1, 3, FALSE);
  
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_container_add (GTK_CONTAINER (chat_window), table);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  chat->begin_msg = NULL;
  chat->something_typed = FALSE;

  chat->text_view = gtk_text_view_new_with_regex ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (chat->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (chat->text_view),
			       GTK_WRAP_WORD);

  chat->text_buffer = 
    gtk_text_view_get_buffer (GTK_TEXT_VIEW (chat->text_view));

  gtk_text_buffer_get_end_iter (chat->text_buffer, &iter);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (chat->text_view), false);

  mark = gtk_text_buffer_create_mark (chat->text_buffer, 
				      "current-position", &iter, FALSE);

  gtk_text_buffer_create_tag (chat->text_buffer, "primary-user",
			      "foreground", "red", 
			      "weight", 900, NULL);

  gtk_text_buffer_create_tag (chat->text_buffer,
			      "secondary-user",
			      "foreground", "darkblue", 
			      "weight", 900, NULL);

  
  /* Create the various tags for the different urls types */
  regex_tag = gtk_text_buffer_create_tag (chat->text_buffer,
					  "uri-http",
					  "foreground", "blue",
					  NULL);
  if (gtk_text_tag_set_regex (regex_tag,
			      "\\<([s]?(ht|f)tp://[^[:blank:]]+)\\>")) {
    gtk_text_tag_add_actions_to_regex (regex_tag,
#ifndef DISABLE_GNOME
				       _("Open URI"),
				       open_uri_callback,
#endif
				       _("Copy Link Location"),
				       copy_uri_callback,
				       NULL);
  }

  
  regex_tag = gtk_text_buffer_create_tag (chat->text_buffer, "uri-h323",
					  "foreground", "pink",
					  NULL);
  if (gtk_text_tag_set_regex (regex_tag,
			      "\\<(h323:[^[:blank:]]+)\\>")) {
    gtk_text_tag_add_actions_to_regex (regex_tag,
				       _("Connect to"),
				       connect_uri_callback,
				       _("Add to Address Book"),
				       add_uri_callback,
				       _("Copy Link Location"),
				       copy_uri_callback,
				       NULL);
  }

  regex_tag = gtk_text_buffer_create_tag (chat->text_buffer, "smileys",
					  "foreground", "grey",
					  NULL);
  if (gtk_text_tag_set_regex (regex_tag,
			      "(:[-]?(\\)|\\(|o|O|p|P|D|\\||/)|\\}:(\\(|\\))|\\|[-]?(\\(|\\))|:'\\(|:\\[|:-(\\.|\\*|x)|;[-]?\\)|(8|B)[-]?\\)|X(\\(|\\||\\))|\\((\\.|\\|)\\)|x\\*O)"))
    gtk_text_tag_set_regex_display (regex_tag, gtk_text_buffer_insert_smiley);

  regex_tag = gtk_text_buffer_create_tag (chat->text_buffer, "latex",
					  "foreground", "grey",
					  NULL);
  if (gtk_text_tag_set_regex (regex_tag,
			      "(\\$[^$]*\\$|\\$\\$[^$]*\\$\\$)"))
    gtk_text_tag_add_actions_to_regex (regex_tag,
				       _("Copy Equation"),
				       copy_uri_callback, NULL);

  /* */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (scr), chat->text_view);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  label = gtk_label_new (_("Send message:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label), 
		    0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  entry = gtk_entry_new ();
  hbox = gtk_hbox_new (FALSE, 0);

  gtk_widget_set_size_request (GTK_WIDGET (entry), 245, -1);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (hbox), 
		    0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  g_signal_connect (GTK_OBJECT (entry), "activate",
		    G_CALLBACK (chat_entry_activate), chat->text_view);

  return chat_window;
}
