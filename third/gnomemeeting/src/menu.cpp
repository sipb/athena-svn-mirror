
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
 *                         menu.cpp  -  description 
 *                         ------------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Functions to create the menus.
 *
 */


#include "../config.h"

#include "menu.h"
#include "connection.h"
#include "endpoint.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "chat_window.h"
#include "ldap_window.h"

#include "stock-icons.h"
#include "gtk_menu_extensions.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
extern GtkWidget *gm;


/* Static functions */
static void zoom_changed_callback (GtkWidget *,
				   gpointer);

static void fullscreen_changed_callback (GtkWidget *,
					 gpointer);

static void speed_dial_menu_item_selected (GtkWidget *,
					   gpointer);


/* Those 2 callbacks update a gconf key when 
   a menu item is toggled */
static void radio_menu_changed (GtkWidget *, 
				gpointer);

static void toggle_menu_changed (GtkWidget *, 
				 gpointer);



/* GTK Callbacks */
/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu.
 * BEHAVIOR     :  Sets zoom to 1:2 if data == 0, 1:1 if data == 1, 
 *                 2:1 if data == 2. (Updates the gconf key).
 * PRE          :  /
 */
static void 
zoom_changed_callback (GtkWidget *widget,
		       gpointer data)
{
  double zoom = 0.0;
  
  zoom = gconf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");

  switch (GPOINTER_TO_INT (data)) {

  case 0:
    if (zoom > 0.5)
      zoom = zoom / 2.0;
    break;

  case 1:
    zoom = 1.0;
    break;

  case 2:
    if (zoom < 2.00)
      zoom = zoom * 2.0;
  }

  gconf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", zoom);
}


/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles fullscreen.
 * PRE          :  / 
 */
static void 
fullscreen_changed_callback (GtkWidget *widget,
			     gpointer data)
{
  gconf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
}


/* DESCRIPTION  :  This callback is called when the user toggles an 
 *                 item in the speed dials menu.
 * BEHAVIOR     :  Calls the given speed dial.
 * PRE          :  data is the speed dial as a gchar *
 */
static void
speed_dial_menu_item_selected (GtkWidget *w,
			       gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gchar *url = NULL;
    
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (!data)
    return;

  url = g_strdup_printf ("%s#", (gchar *) data);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
		      (gchar *) url);
    
  if (ep->GetCallingState () == GMH323EndPoint::Connected)
    transfer_call_cb (NULL, (gpointer) url);
  else
    connect_cb (NULL, NULL);

  g_free (url);
}


/* DESCRIPTION  :  This callback is called when the user 
 *                 selects a different option in a radio menu.
 * BEHAVIOR     :  Sets the gconf key.
 * PRE          :  data is the gconf key.
 */
static void 
radio_menu_changed (GtkWidget *widget,
		    gpointer data)
{
  GSList *group = NULL;

  int group_last_pos = 0;
  int active = 0;

  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, last pos is 0 */

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    while (group) {

      if (group->data == widget) 
	break;
      
      active++;
      group = g_slist_next (group);
    }

    gconf_set_int ((gchar *) data, group_last_pos - active);
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles an 
 *                 option in a menu.
 * BEHAVIOR     :  Updates the gconf key given as parameter.
 * PRE          :  data is the key.
 */
static void 
toggle_menu_changed (GtkWidget *widget, gpointer data)
{
  gconf_set_bool ((gchar *) data, GTK_CHECK_MENU_ITEM (widget)->active);
}


/* The functions */
GtkWidget *
gnomemeeting_init_menu (GtkAccelGroup *accel)
{
  GmWindow *gw = NULL;
  GmTextChat *chat = NULL;

  GtkWidget *menubar = NULL;

  IncomingCallMode icm = AVAILABLE;
  ControlPanelSection cps = CLOSED;
  bool show_status_bar = false;
  bool show_chat_window = false;

  chat = GnomeMeeting::Process ()->GetTextChat ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  menubar = gtk_menu_bar_new ();

  /* Default values */
  icm = (IncomingCallMode) 
    gconf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 
  cps = (ControlPanelSection)
    gconf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section"); 
  show_status_bar =
    gconf_get_bool (USER_INTERFACE_KEY "main_window/show_status_bar"); 
  show_chat_window =
    gconf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"); 
  
  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("C_all")),

      GTK_MENU_ENTRY("connect", _("C_onnect"), _("Create a new connection"), 
		     GM_STOCK_CONNECT_16, 'o',
		     GTK_SIGNAL_FUNC (connect_cb), NULL, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Disconnect"),
		     _("Close the current connection"), 
		     GM_STOCK_DISCONNECT_16, 'd',
		     GTK_SIGNAL_FUNC (disconnect_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("available", _("_Available"),
			   _("Display a popup to accept the call"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("free_for_chat", _("Free for Cha_t"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FREE_FOR_CHAT), TRUE),
      GTK_MENU_RADIO_ENTRY("busy", _("_Busy"), _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == BUSY), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FORWARD), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_SUBMENU_NEW("speed_dials", _("Speed dials")),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("hold_call", _("_Hold Call"), _("Hold the current call"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (hold_call_cb), (gpointer) gw, FALSE),
      GTK_MENU_ENTRY("transfer_call", _("_Transfer Call"),
		     _("Transfer the current call"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (transfer_call_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("suspend_audio", _("Suspend _Audio"),
		     _("Suspend or resume the audio transmission"),
		     NULL, 0,
		     GTK_SIGNAL_FUNC (pause_channel_callback),
		     GINT_TO_POINTER (0), FALSE),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (pause_channel_callback),
		     GINT_TO_POINTER (1), FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("save_picture", _("_Save Current Picture"), 
		     _("Save a snapshot of the current video"),
		     GTK_STOCK_SAVE, 'S',
		     GTK_SIGNAL_FUNC (save_callback), NULL, FALSE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("close", _("_Close"), _("Close the GnomeMeeting window"),
		     GTK_STOCK_CLOSE, 'W', 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gm, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("quit", _("_Quit"), _("Quit GnomeMeeting"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback), (gpointer) gw, TRUE),

      GTK_MENU_NEW (_("_Edit")),

#ifndef DISABLE_GNOME
      GTK_MENU_ENTRY("configuration_druid", _("Configuration Druid"),
		     _("Run the configuration druid"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->druid_window, TRUE),
#else
      GTK_MENU_ENTRY("configuration_druid", _("Configuration Druid"),
		     _("Run the configuration druid"),
		     NULL, 0, 
		     NULL, NULL, FALSE),
#endif

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("preferences", _("_Preferences"),
		     _("Change your preferences"), 
		     GTK_STOCK_PREFERENCES, 'P',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->pref_window, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_MENU_TOGGLE_ENTRY("text_chat", _("Text Chat"),
			    _("View/Hide the text chat window"), 
			    NULL, 0,
			    GTK_SIGNAL_FUNC (toggle_menu_changed),
			    (gpointer) USER_INTERFACE_KEY "main_window/show_chat_window",
			     show_chat_window, TRUE),
      GTK_MENU_TOGGLE_ENTRY("status_bar", _("Status Bar"),
			    _("View/Hide the status bar"), 
			    NULL, 0, 
			    GTK_SIGNAL_FUNC (toggle_menu_changed),
			    (gpointer) USER_INTERFACE_KEY "main_window/show_status_bar",
			    show_status_bar, TRUE),

      GTK_SUBMENU_NEW("control_panel", _("Control Panel")),

      GTK_MENU_RADIO_ENTRY("statistics", _("Statistics"), 
			   _("View audio/video transmission and reception statistics"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 0), TRUE),
      GTK_MENU_RADIO_ENTRY("dialpad", _("_Dialpad"), _("View the dialpad"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 1), TRUE),
      GTK_MENU_RADIO_ENTRY("audio_settings", _("_Audio Settings"),
			   _("View audio settings"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 2), TRUE),
      GTK_MENU_RADIO_ENTRY("video_settings", _("_Video Settings"),
			   _("View video settings"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 3), TRUE),
      GTK_MENU_RADIO_ENTRY("off", _("Off"), _("Hide the control panel"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 4), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("clear_text_chat", _("_Clear Text Chat"),
		     _("Clear the text chat"), 
		     GTK_STOCK_CLEAR, 'L',
		     GTK_SIGNAL_FUNC (gnomemeeting_text_chat_clear),
		     (gpointer) chat, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("local_video", _("Local Video"),
			   _("Local video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   TRUE, FALSE),
      GTK_MENU_RADIO_ENTRY("remote_video", _("Remote Video"),
			   _("Remote video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("Both (Local Video Incrusted)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_window",
			   _("Both (Local Video in New Window)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_windows",
			   _("Both (Both in New Windows)"), 
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", _("Zoom In"), _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (2), FALSE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (0), FALSE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (1), FALSE),

      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_callback),
		     NULL, FALSE),

      GTK_MENU_NEW(_("_Tools")),

      GTK_MENU_ENTRY("address_book", _("Address _Book"),
		     _("Open the address book"),
		     GM_STOCK_ADDRESSBOOK_16, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->ldap_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("log", _("General History"),
		     _("View the operations history"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->log_window, TRUE),
      GTK_MENU_ENTRY("calls_history", _("Calls History"),
		     _("View the calls history"),
		     GM_STOCK_CALLS_HISTORY, 'h',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->calls_history_window, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("pc-to-phone", _("PC-To-Phone Account"),
		     _("Manage your PC-To-Phone account"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->pc_to_phone_window, TRUE),

      GTK_MENU_NEW(_("_Help")),

#ifndef DISABLE_GNOME
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),
#else
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     NULL, NULL, TRUE),
#endif
       
#ifndef DISABLE_GNOME
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     GNOME_STOCK_ABOUT, 0, 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) gm,
		     TRUE),
#else
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) gm,
		     FALSE),
#endif

      GTK_MENU_END
    };

  gtk_build_menu (menubar, gnomemeeting_menu, accel, gw->statusbar);
  gnomemeeting_speed_dials_menu_update (menubar);
  gtk_widget_show_all (GTK_WIDGET (menubar));
  
  return menubar;
}


GtkWidget *
gnomemeeting_video_popup_init_menu (GtkWidget *widget, GtkAccelGroup *accel)
{
  GtkWidget *popup_menu_widget = NULL;
  
  static MenuEntry video_menu [] =
    {
      GTK_MENU_RADIO_ENTRY("local_video", _("Local Video"),
			   _("Local video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   TRUE, FALSE),
      GTK_MENU_RADIO_ENTRY("remote_video", _("Remote Video"),
			   _("Remote video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("Both (Local Video Incrusted)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_window",
			   _("Both (Local Video in New Window)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_windows",
			   _("Both (Both in New Windows)"), 
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", _("Zoom In"), _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (2), FALSE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (0), FALSE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_changed_callback),
		     GINT_TO_POINTER (1), FALSE),

      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_callback), NULL,
		     FALSE),

      GTK_MENU_END
    };

  popup_menu_widget = gtk_build_popup_menu (widget, video_menu, accel);
  
  return popup_menu_widget;
}


GtkWidget *
gnomemeeting_tray_init_menu (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget = NULL;
  IncomingCallMode icm = AVAILABLE;
  GmWindow *gw = NULL;

  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  icm = (IncomingCallMode)gconf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 

  static MenuEntry tray_menu [] =
    {
      GTK_MENU_ENTRY("connect", _("_Connect"), _("Create a new connection"), 
		     GM_STOCK_CONNECT_16, 'c', 
		     GTK_SIGNAL_FUNC (connect_cb), gw, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Disconnect"),
		     _("Close the current connection"), 
		     GM_STOCK_DISCONNECT_16, 'd',
		     GTK_SIGNAL_FUNC (disconnect_cb), gw, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("available", _("_Available"),
			   _("Display a popup to accept the call"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("free_for_chat", _("Free for Cha_t"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FREE_FOR_CHAT), TRUE),
      GTK_MENU_RADIO_ENTRY("busy", _("_Busy"), _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == BUSY), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FORWARD), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("address_book", _("Address _Book"),
		     _("Open the address book"),
		     GM_STOCK_ADDRESSBOOK_16, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->ldap_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("calls_history", _("Calls History"),
		     _("View the calls history"),
		     GM_STOCK_CALLS_HISTORY, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->calls_history_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("preferences", _("_Preferences"),
		     _("Change your preferences"),
		     GTK_STOCK_PREFERENCES, 'P', 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gw->pref_window, TRUE),

      GTK_MENU_SEPARATOR,
     
#ifndef DISABLE_GNOME
      GTK_MENU_ENTRY("about", _("_About GnomeMeeting"),
		     _("View information about GnomeMeeting"),
		     GNOME_STOCK_ABOUT, 'a', 
		     GTK_SIGNAL_FUNC (about_callback),
		     (gpointer) gm, TRUE),
#else
      GTK_MENU_ENTRY("about", _("_About GnomeMeeting"),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback),
		     (gpointer) gm, FALSE),
#endif

      GTK_MENU_ENTRY("quit", _("_Quit"), _("Quit GnomeMeeting"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback),
		     (gpointer) gw, TRUE),

      GTK_MENU_END
    };

  
  popup_menu_widget = gtk_build_popup_menu (widget, tray_menu, NULL);

  return popup_menu_widget;
}


void
gnomemeeting_speed_dials_menu_update (GtkWidget *menubar)
{
  GtkWidget *item = NULL;
  GtkWidget *menu = NULL;
  
  GSList *glist = NULL;
  GList *old_glist_iter = NULL;
  
  gchar *ml = NULL;  
  gchar **couple = NULL;

  glist = gnomemeeting_addressbook_get_speed_dials ();
  menu = gtk_menu_get_widget (menubar, "speed_dials");

  while ((old_glist_iter = GTK_MENU_SHELL (menu)->children)) 
    gtk_container_remove (GTK_CONTAINER (menu),
			  GTK_WIDGET (old_glist_iter->data));

  
  while (glist && glist->data) {

    couple = g_strsplit ((gchar *) glist->data, "|", 0);

    if (couple [0] && couple [1]) {
      
      ml = g_strdup_printf ("<b>%s#</b>   <i>%s</i>", couple [1], couple [0]);
      item = gtk_menu_item_new_with_label (ml);
      gtk_label_set_markup (GTK_LABEL (gtk_bin_get_child (GTK_BIN (item))),
			    ml);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      gtk_widget_show (item);

      g_signal_connect_data (G_OBJECT (item), "activate",
			     GTK_SIGNAL_FUNC (speed_dial_menu_item_selected),
			     (gpointer) g_strdup (couple [1]),
			     (GClosureNotify) g_free, (GConnectFlags) 0);
    }
    
    glist = g_slist_next (glist);

    g_free (ml);
    g_strfreev (couple);
  }
  
  g_slist_free (glist);
}


void
gnomemeeting_menu_update_sensitivity (unsigned calling_state)
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  switch (calling_state)
    {
    case GMH323EndPoint::Standby:

      gtk_menu_set_sensitive (gw->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", TRUE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (gw->main_menu, "hold_call", FALSE);
      break;


    case GMH323EndPoint::Calling:

      gtk_menu_set_sensitive (gw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      break;


    case GMH323EndPoint::Connected:
      gtk_menu_set_sensitive (gw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (gw->main_menu, "hold_call", TRUE);
      break;


    case GMH323EndPoint::Called:
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      break;
    }
}


void
gnomemeeting_menu_update_sensitivity (BOOL is_video,
				      BOOL is_receiving,
				      BOOL is_transmitting)
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (is_video) {

    if (is_transmitting) {
      
      gtk_menu_section_set_sensitive (gw->main_menu, "local_video", TRUE);
      gtk_menu_section_set_sensitive (gw->main_menu, "suspend_video", TRUE);
    }
    else {
      
      gtk_menu_section_set_sensitive (gw->main_menu, "local_video", FALSE);
      gtk_menu_section_set_sensitive (gw->main_menu, "suspend_video", FALSE);
    }
    
    if (is_receiving && is_transmitting) {
    
      gtk_menu_section_set_sensitive (gw->main_menu,
				      "local_video", TRUE);
      gtk_menu_section_set_sensitive (gw->video_popup_menu,
				      "local_video", TRUE);
    }
    else {
      
      gtk_menu_section_set_sensitive (gw->main_menu,
				      "local_video", FALSE);
      gtk_menu_section_set_sensitive (gw->video_popup_menu,
				      "local_video", FALSE);
      if (is_transmitting) {
	
	gtk_menu_set_sensitive (gw->main_menu,
				"local_video", TRUE);
	gtk_menu_set_sensitive (gw->video_popup_menu,
				"local_video", TRUE);
      }
      else if (is_receiving) {

	gtk_menu_set_sensitive (gw->main_menu,
				"remote_video", TRUE);
	gtk_menu_set_sensitive (gw->video_popup_menu,
				"remote_video", TRUE);
      }

      if (!is_receiving && !is_transmitting) {

	gtk_menu_section_set_sensitive (gw->main_menu,
					"zoom_in", FALSE);
	gtk_menu_section_set_sensitive (gw->video_popup_menu,
					"zoom_in", FALSE);
	gtk_menu_set_sensitive (gw->main_menu, "save_picture", FALSE);
      }
      else {

	gtk_menu_section_set_sensitive (gw->main_menu,
					"zoom_in", TRUE);
	gtk_menu_section_set_sensitive (gw->video_popup_menu,
					"zoom_in", TRUE);
	gtk_menu_set_sensitive (gw->main_menu, "save_picture", TRUE);
      }
    }
  }
  else {

    if (is_transmitting)
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", TRUE);
    else
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", FALSE);
  }
}
