
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
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "../config.h"

#include "ldap_window.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "menu.h"
#include "misc.h"
#include "tools.h"
#include "urlhandler.h"

#include "gmentrydialog.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
extern GtkWidget *gm;


/* The callbacks */
void 
hold_call_cb(GtkWidget *widget,
	     gpointer data)
{
  GtkWidget *child = NULL;

  GmWindow *gw = NULL;
  
  H323Connection *connection = NULL;
  GMH323EndPoint *endpoint = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  gdk_threads_leave ();
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  connection =
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
  gdk_threads_enter ();

  if (connection) {

    child = GTK_BIN (gtk_menu_get_widget (gw->main_menu, "hold_call"))->child;
    
    if (!connection->IsCallOnHold ()) {

      if (GTK_IS_LABEL (child))
	gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					  _("_Retrieve Call"));

      gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "suspend_video", FALSE);
      
      connection->HoldCall (TRUE);
    }
    else {
      
      if (GTK_IS_LABEL (child))
	gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					  _("_Hold Call"));

      gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), TRUE);
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "suspend_video", FALSE);

      connection->RetrieveCall ();
    }

    connection->Unlock ();
  }
}


void
transfer_call_cb (GtkWidget* widget,
		  gpointer data)
{
  GMH323EndPoint *endpoint = NULL;
  GmWindow *gw = NULL;
  GMURL url;
  
  char *gconf_forward_value = NULL;
  gint answer = 0;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  gw->transfer_call_popup = gm_entry_dialog_new (_("Transfer call to:"),
						 _("Transfer"));

  if (!data) {

    gtk_window_set_transient_for (GTK_WINDOW (gw->transfer_call_popup),
				  GTK_WINDOW (gm));
    gconf_forward_value =
      gconf_get_string (CALL_FORWARDING_KEY "forward_host");
  }
  else {
    
    gtk_window_set_transient_for (GTK_WINDOW (gw->transfer_call_popup),
				  GTK_WINDOW (gw->ldap_window));
    gconf_forward_value = g_strdup ((gchar *) data);
  }
  
  gtk_dialog_set_default_response (GTK_DIALOG (gw->transfer_call_popup),
				   GTK_RESPONSE_ACCEPT);
  
  if (gconf_forward_value && strcmp (gconf_forward_value, ""))
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (gw->transfer_call_popup),
			      gconf_forward_value);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (gw->transfer_call_popup),
			      (const char *) url.GetDefaultURL ());

  g_free (gconf_forward_value);
  gconf_forward_value = NULL;
  
  gtk_widget_show_all (gw->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (gw->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    gconf_forward_value =
      (gchar *) gm_entry_dialog_get_text (GM_ENTRY_DIALOG (gw->transfer_call_popup));
    new GMURLHandler (gconf_forward_value, TRUE);
      
    break;

  default:
    break;
  }

  gtk_widget_destroy (gw->transfer_call_popup);
  gw->transfer_call_popup = NULL;
}


void save_callback (GtkWidget *widget, gpointer data)
{
  GnomeMeeting::Process ()->Endpoint ()->SavePicture ();
}


void pause_channel_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = NULL;
  
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  GMH323EndPoint *endpoint = NULL;
  GMVideoGrabber *vg = NULL;
  PString current_call_token;

  GtkToggleButton *b = NULL;
  GtkWidget *child = NULL;
  
  gchar *menu_suspend_msg = NULL;
  gchar *menu_resume_msg = NULL;
  gchar *log_suspend_msg = NULL;
  gchar *log_resume_msg = NULL;

  gdk_threads_leave ();
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  current_call_token = endpoint->GetCurrentCallToken ();

  gw = GnomeMeeting::Process ()->GetMainWindow ();

  if (!current_call_token.IsEmpty ())
    connection =
      endpoint->FindConnectionWithLock (current_call_token);


  if (connection) {

    if (GPOINTER_TO_INT (data) == 0)
      channel = 
	connection->FindChannel (RTP_Session::DefaultAudioSessionID, 
				 FALSE);
    else
      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);
    
    if (channel) {

      if (GPOINTER_TO_INT (data) == 0) {

	menu_suspend_msg = g_strdup (_("Suspend _Audio"));
	menu_resume_msg = g_strdup (_("Resume _Audio"));
	log_suspend_msg = g_strdup (_("Audio transmission: suspended"));
	log_resume_msg = g_strdup (_("Audio transmission: resumed"));

	gdk_threads_enter ();
	b = GTK_TOGGLE_BUTTON (gw->audio_chan_button);
	
	child =
	  GTK_BIN (gtk_menu_get_widget (gw->main_menu, "suspend_audio"))->child;
	gdk_threads_leave ();
      }
      else {
	
	menu_suspend_msg = g_strdup (_("Suspend _Video"));
	menu_resume_msg = g_strdup (_("Resume _Video"));
	log_suspend_msg = g_strdup (_("Video transmission: suspended"));
	log_resume_msg = g_strdup (_("Video transmission: resumed"));

	gdk_threads_enter ();
	b = GTK_TOGGLE_BUTTON (gw->video_chan_button);
	
	child =
	  GTK_BIN (gtk_menu_get_widget (gw->main_menu, "suspend_video"))->child;
	gdk_threads_leave ();
      }
    
      if (channel->IsPaused ()) {

	gdk_threads_enter ();
	if (GTK_IS_LABEL (child)) 
	  gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					    menu_suspend_msg);

	gnomemeeting_log_insert (log_resume_msg);
	gnomemeeting_statusbar_flash (gw->statusbar, log_resume_msg);

	g_signal_handlers_block_by_func (G_OBJECT (b),
					 (gpointer) pause_channel_callback,
					 GINT_TO_POINTER (0));
	gtk_toggle_button_set_active (b, FALSE);
	gtk_widget_queue_draw (GTK_WIDGET (b));
	g_signal_handlers_unblock_by_func (G_OBJECT (b),
					   (gpointer) pause_channel_callback,
					   GINT_TO_POINTER (0));

	channel->SetPause (FALSE);
	gdk_threads_leave ();
      }
      else {

	gdk_threads_enter ();
	if (GTK_IS_LABEL (child)) 
	  gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					    menu_resume_msg);

	gnomemeeting_log_insert (log_suspend_msg);
	gnomemeeting_statusbar_flash (gw->statusbar, log_suspend_msg);

	g_signal_handlers_block_by_func (G_OBJECT (b),
					 (gpointer) pause_channel_callback,
					 GINT_TO_POINTER (1));
	gtk_toggle_button_set_active (b, TRUE);
	gtk_widget_queue_draw (GTK_WIDGET (b));
	g_signal_handlers_unblock_by_func (G_OBJECT (b),
					   (gpointer) pause_channel_callback,
					   GINT_TO_POINTER (1));
	
	channel->SetPause (TRUE);
	gdk_threads_leave ();
      }
    }

    g_free (menu_suspend_msg);
    g_free (menu_resume_msg);
    g_free (log_suspend_msg);
    g_free (log_resume_msg);
    
    connection->Unlock ();
  }
  else {

    vg = endpoint->GetVideoGrabber ();
    if (vg && vg->IsGrabbing ())
      vg->StopGrabbing ();
    else
      vg->StartGrabbing ();
  }
  
  gdk_threads_enter ();
}


gboolean
delete_window_cb (GtkWidget *w,
                  GdkEvent *ev,
                  gpointer data)
{
  gnomemeeting_window_hide (GTK_WIDGET (w));

  return TRUE;
}


void
show_window_cb (GtkWidget *w,
		gpointer data)
{
  if (!gnomemeeting_window_is_visible (GTK_WIDGET (data)))
    gnomemeeting_window_show (GTK_WIDGET (data));
  else
    gnomemeeting_window_hide (GTK_WIDGET (data));
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;

  if ((GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) ||
      (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Called))
    GnomeMeeting::Process ()->Connect ();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;
  
  GnomeMeeting::Process ()->Disconnect ();
}


void about_callback (GtkWidget *widget, gpointer parent_window)
{
#ifndef DISABLE_GNOME
  GtkWidget *abox = NULL;
  GdkPixbuf *pixbuf = NULL;
	
  const gchar *authors [] = {
      "Damien Sandras <damien.sandras@it-optics.com>",
      "",
      N_("Code contributors:"),
      "Kenneth Rohde Christiansen <kenneth@gnu.org>",
      "Julien Puydt <julien.puydt@club-internet.fr>",
      "Miguel Rodríguez Pérez <migrax@terra.es>",
      "Paul <paul@argo.dyndns.org>", 
      "Roger Hardiman <roger@freebsd.org>",
      "Sébastien Josset <Sebastien.Josset@space.alcatel.fr>",
      "Stefan Bruëns <lurch@gmx.li>",
      "Tuan <tuan@info.ucl.ac.be>",
      "",
      N_("Artwork:"),
      "Jakub Steiner <jimmac@ximian.com>",
      "Carlos Pardo <me@m4de.com>",
      "",
      N_("Contributors:"),
      "Alexander Larsson <alexl@redhat.com>",
      "Artur Flinta  <aflinta@at.kernel.pl>",
      "Bob Mroczka <bob@mroczka.com>",
      "Chih-Wei Huang <cwhuang@citron.com.tw>",
      "Christian Rose <menthos@menthos.com>",
      "Christian Strauf <strauf@uni-muenster.de>",
      "Christopher R. Gabriel <cgabriel@cgabriel.org>",
      "Cristiano De Michele <demichel@na.infn.it>",
      "Fabrice Alphonso <fabrice@alphonso.dyndns.org>",
      "Florin Grad <florin@mandrakesoft.com>",
      "Georgi Georgiev <chutz@gg3.net>",
      "Johnny Ström <jonny.strom@netikka.fi>",
      "Kilian Krause <kk@verfaction.de>",
      "Matthias Marks <matthias@marksweb.de>",
      "Rafael Pinilla <r_pinilla@yahoo.com>",
      "Santiago García Mantiñán <manty@manty.net>",
      "Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>",
      "Stéphane Wirtel <stephane.wirtel@belgacom.net>",
      "Vincent Deroo <crossdatabase@aol.com>",
      NULL
  };
	
  authors [2] = gettext (authors [2]);
  authors [12] = gettext (authors [12]);
  authors [16] = gettext (authors [16]);
  
  const char *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Christopher Warner <zanee@kernelcode.com>",
    "Matthias Redlich <m-redlich@t-online.de>",
    NULL
  };

  /* Translators: Please write translator credits here, and
   * seperate names with \n */
  const char *translator_credits = _("translator_credits");
  
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png", NULL);
  

  abox = gnome_about_new ("GnomeMeeting",
			  VERSION,
			  "Copyright © 2000-2004 Damien Sandras",
                          /* Translators: Please test to see if your translation
                           * looks OK and fits within the box */
			  _("GnomeMeeting is full-featured H.323 compatible videoconferencing, VoIP and IP-Telephony application that allows you to make audio and video calls to remote users with H.323 hardware or software."),
			  (const char **) authors,
                          (const char **) documenters,
                          strcmp (translator_credits, 
				  "translator_credits") != 0 ? 
                          translator_credits : "No translators, English by\n"
                          "Damien Sandras <dsandras@seconix.com>",
			  pixbuf);

  g_object_unref (pixbuf);

  gtk_window_set_transient_for (GTK_WINDOW (abox), GTK_WINDOW (parent_window));
  gtk_window_present (GTK_WINDOW (abox));
#endif

  return;
}


void help_cb (GtkWidget *widget,
	       gpointer data)
{
  GError *err = NULL;
#ifndef DISABLE_GNOME
  gnome_help_display ("gnomemeeting.xml", NULL, &err);
#endif
}


void quit_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep =NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  gnomemeeting_window_hide (gm);
  gnomemeeting_window_hide (gw->log_window);
  gnomemeeting_window_hide (gw->calls_history_window);
  gnomemeeting_window_hide (gw->ldap_window);
  gnomemeeting_window_hide (gw->pref_window);
  
  gdk_threads_leave ();
  ep->ClearAllCalls (H323Connection::EndedByLocalUser, TRUE);
  gdk_threads_enter ();

  gtk_widget_hide (gw->docklet);

  gtk_main_quit ();
}  


