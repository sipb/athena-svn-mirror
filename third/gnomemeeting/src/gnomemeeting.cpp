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
 *                         gnomemeeting.cpp  -  description
 *                         --------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#include "../config.h"

#include "gnomemeeting.h"
#include "callbacks.h"
#include "sound_handling.h"
#include "ils.h"
#include "urlhandler.h"
#include "ldap_window.h"
#include "pref_window.h"
#include "chat_window.h"
#include "druid.h"
#include "tools.h"
#include "tray.h"
#include "main_window.h"
#include "toolbar.h"
#include "misc.h"

#include "history-combo.h"
#include "dialog.h"
#include "e-splash.h"
#include "stock-icons.h"
#include "gconf_widgets_extensions.h"

#ifndef WIN32
#include <esd.h>
#include <signal.h>
#endif


static gint
gnomemeeting_tray_hack (gpointer);

GnomeMeeting *GnomeMeeting::GM = NULL;

GtkWidget *gm;


static gint
gnomemeeting_tray_hack (gpointer data)
{
  GmWindow *gw = NULL;

  gdk_threads_enter ();

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (!gnomemeeting_tray_is_embedded (gw->docklet)) {

    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Notification area not detected"), _("You have chosen to start GnomeMeeting hidden, however the notification area is not present in your panel, GnomeMeeting can thus not start hidden."));
    gnomemeeting_window_show (gm);
  }
  
  gdk_threads_leave ();

  return FALSE;
}


/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)

{
  /* no endpoint for the moment */
  endpoint = NULL;
  url_handler = NULL;

  /* Init the different structures */
  gw = new GmWindow ();
  pw = new GmPrefWindow ();
  lw = new GmLdapWindow ();
  dw = new GmDruidWindow ();
  chat = new GmTextChat ();
  chw = new GmCallsHistoryWindow ();
  rtp = new GmRtpData ();

  memset ((void *) rtp, 0, sizeof (struct _GmRtpData));
  gw->docklet = gw->ldap_window = gw->pref_window = gw->calls_history_window =
    gw->splash_win = gw->incoming_call_popup = gw->transfer_call_popup =
    gw->log_window = gw->audio_transmission_popup = gw->audio_reception_popup =
#ifndef DISABLE_GNOME
    gw->druid_window =
#endif
    NULL;

  GM = this;
  
  endpoint = new GMH323EndPoint ();
  
  call_number = 0;
}


GnomeMeeting::~GnomeMeeting()
{
  if (endpoint) {

    endpoint->RemoveVideoGrabber ();
#ifdef HAS_IXJ
    endpoint->RemoveLid ();
#endif
  }
  
  RemoveEndpoint ();

  if (gw->ldap_window) {
    gnomemeeting_ldap_window_destroy_notebook_pages ();
    gtk_widget_destroy (gw->ldap_window);
  }
  
  if (gw->pref_window)
    gtk_widget_destroy (gw->pref_window);
  if (gw->log_window)
    gtk_widget_destroy (gw->log_window);
  if (gw->calls_history_window)
    gtk_widget_destroy (gw->calls_history_window);
  if (gm)
    gtk_widget_destroy (gm);
#ifndef DISABLE_GNOME
  if (gw->druid_window)
    gtk_widget_destroy (gw->druid_window);
#endif
  
  delete (gw);
  delete (pw);
  delete (lw);
  delete (dw);
  delete (chat);
  delete (chw);
  delete (rtp);
}


void 
GnomeMeeting::Connect()
{
  PString call_address;
  
  gnomemeeting_threads_enter ();
  gnomemeeting_statusbar_push  (gw->statusbar, NULL);
  call_address = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry));
  gnomemeeting_threads_leave ();


  /* If incoming connection, then answer it */
 if (endpoint->GetCallingState () == 3) {

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (_("Answering incoming call"));
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
    gnomemeeting_threads_leave ();

    url_handler = new GMURLHandler ();
  }
  else 
  {
    gnomemeeting_threads_enter ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			call_address);
    
    if (!call_address.IsEmpty ())
      gm_history_combo_add_entry (GM_HISTORY_COMBO (gw->combo), 
				  USER_INTERFACE_KEY "main_window/urls_history",
				  call_address);
    gnomemeeting_threads_leave ();


    /* if we call somebody, and if the URL is not empty */
    if (!GMURL (call_address).IsEmpty ()) {
      call_number++;

      url_handler = new GMURLHandler (call_address);
    }
    else  /* We untoggle the connect button in the case it was toggled */
      {

	gnomemeeting_threads_enter ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      FALSE);
	gnomemeeting_threads_leave ();
      }
  }
}


void
GnomeMeeting::Disconnect (H323Connection::CallEndReason reason)
{
  gnomemeeting_threads_enter ();
  gnomemeeting_statusbar_push (gw->statusbar, NULL);
  gnomemeeting_threads_leave ();


  /* if we are trying to call somebody */
  if (endpoint->GetCallingState () == 1) {

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (_("Trying to stop calling"));
    gnomemeeting_threads_leave ();

    endpoint->ClearCall (endpoint->GetCurrentCallToken (), reason);
  }
  else {

    /* if we are in call with somebody */
    if (endpoint->GetCallingState () == 2) {

      gnomemeeting_threads_enter ();	
      gnomemeeting_log_insert (_("Stopping current call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				    0);
      gnomemeeting_threads_leave ();

      endpoint->ClearAllCalls (reason, FALSE);
    }
    else if (endpoint->GetCallingState () == 3) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Refusing Incoming call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				    0);
      gnomemeeting_threads_leave ();

      endpoint->ClearCall (endpoint->GetCurrentCallToken (),
			   H323Connection::EndedByLocalUser);
    }
  }
}


void
GnomeMeeting::Init ()
{
#ifndef WIN32
  /* Ignore SIGPIPE */
  signal (SIGPIPE, SIG_IGN);
#endif

  endpoint->Init ();
}


BOOL
GnomeMeeting::DetectDevices ()
{
  gchar *audio_plugin = NULL;
  gchar *video_plugin = NULL;

  PINDEX fake_idx;

  audio_plugin = gconf_get_string (AUDIO_DEVICES_KEY "plugin");
  video_plugin = gconf_get_string (VIDEO_DEVICES_KEY "plugin");
 

  /* Detect the devices */
  gnomemeeting_sound_daemons_suspend ();

  /* Detect the plugins */
  gw->audio_managers = PSoundChannel::GetDriverNames ();
  gw->video_managers = PVideoInputDevice::GetDriverNames ();

  /* Detect the devices */
   gw->video_devices = PVideoInputDevice::GetDriversDeviceNames (video_plugin);
 
  if (PString ("Quicknet") == audio_plugin) {

    gw->audio_recorder_devices = OpalIxJDevice::GetDeviceNames ();
    gw->audio_player_devices = gw->audio_recorder_devices;
  }
  else {
    
    gw->audio_recorder_devices = 
      PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Recorder);
    gw->audio_player_devices = 
      PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Player);
  }
    
  if (gw->audio_recorder_devices.GetSize () == 0) 
    gw->audio_recorder_devices += PString (_("No device found"));
  if (gw->audio_player_devices.GetSize () == 0)
    gw->audio_player_devices += PString (_("No device found"));
  if (gw->video_devices.GetSize () == 0)
    gw->video_devices += PString (_("No device found"));

  g_free (audio_plugin);
  g_free (video_plugin);

  if (gw->audio_managers.GetSize () == 0)
    return FALSE;

  fake_idx = gw->video_managers.GetValuesIndex (PString ("FakeVideo"));
  if (fake_idx != P_MAX_INDEX)
    gw->video_managers.RemoveAt (fake_idx);
  
  gw->audio_managers += PString ("Quicknet");
  gnomemeeting_sound_daemons_resume ();

  return TRUE;
}


GMH323EndPoint *
GnomeMeeting::Endpoint ()
{
  GMH323EndPoint *ep = NULL;
  PWaitAndSignal m(ep_var_mutex);

  ep = endpoint;
  
  return ep;
}


GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


GmWindow *
GnomeMeeting::GetMainWindow ()
{
  return gw;
}


GmPrefWindow *
GnomeMeeting::GetPrefWindow ()
{
  return pw;
}


GmLdapWindow *
GnomeMeeting::GetLdapWindow ()
{
  return lw;
}


GmDruidWindow *
GnomeMeeting::GetDruidWindow ()
{
  return dw;
}


GmCallsHistoryWindow *
GnomeMeeting::GetCallsHistoryWindow ()
{
  return chw;
}


GmTextChat *
GnomeMeeting::GetTextChat ()
{
  return chat;
}


GmRtpData *
GnomeMeeting::GetRtpData ()
{
  return rtp;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::BuildGUI ()
{
  bool show_splash = TRUE;

  
  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  g_signal_connect (G_OBJECT (gw->splash_win), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  show_splash = gconf_get_bool (USER_INTERFACE_KEY "show_splash_screen");
  if (show_splash) 
  {
    /* We show the splash screen */
    gtk_widget_show_all (gw->splash_win);

    while (gtk_events_pending ())
      gtk_main_iteration ();
  }

  
  /* Build the GUI */
  gnomemeeting_stock_icons_init ();
  gw->tips = gtk_tooltips_new ();
  gw->log_window = gnomemeeting_log_window_new ();
  gw->calls_history_window = gnomemeeting_calls_history_window_new (chw);
  gw->pc_to_phone_window = gnomemeeting_pc_to_phone_window_new ();  
  gw->pref_window = gnomemeeting_pref_window_new (pw);  
  gw->ldap_window = gnomemeeting_ldap_window_new (lw);
#ifndef DISABLE_GNOME
  gw->druid_window = gnomemeeting_druid_window_new (dw);
#endif
#ifndef WIN32
  gw->docklet = gnomemeeting_init_tray ();
  gw->tray_popup_menu = gnomemeeting_tray_init_menu (gw->docklet);
#endif
  gnomemeeting_main_window_new (gw);


#ifndef DISABLE_GNOME
 if (gconf_get_int (GENERAL_KEY "version") 
      < 1000 * MAJOR_VERSION + 10 * MINOR_VERSION + BUILD_NUMBER) {

   gtk_widget_show_all (GTK_WIDGET (gw->druid_window));
  }
  else {
#endif
    /* Show the main window */
#ifndef WIN32
    if (!gconf_get_bool (USER_INTERFACE_KEY "start_hidden")) 
#endif
      gnomemeeting_window_show (gm);
#ifndef WIN32
    else
      gtk_timeout_add (15000, (GtkFunction) gnomemeeting_tray_hack, NULL);
#endif
#ifndef DISABLE_GNOME
  }
#endif

 
  /* Destroy the splash */
 if (gw->splash_win) {
   
   gtk_widget_destroy (gw->splash_win);
   gw->splash_win = NULL;
 }

  
  /* GM is started */
  gnomemeeting_log_insert (_("Started GnomeMeeting V%d.%d.%d for %s\n"), 
			   MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
                           g_get_user_name ());
}


void GnomeMeeting::RemoveEndpoint ()
{
  PWaitAndSignal m(ep_var_mutex);

  if (endpoint)
    delete (endpoint);
  
  endpoint = NULL;
}


