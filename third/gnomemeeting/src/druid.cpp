
/* GnomeMeeting --  Video-Conferencing application
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
 *                         druid.cpp  -  description
 *                         --------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the druid.
 */


#include "../config.h"

#include "druid.h"
#include "gnomemeeting.h"
#include "pref_window.h"
#include "sound_handling.h"
#include "ils.h"
#include "misc.h"
#include "callbacks.h"

#include "dialog.h"
#include "stock-icons.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
static gint kind_of_net_hack (gpointer);

static void audio_test_button_clicked (GtkWidget *,
				       gpointer);

static void video_test_button_clicked (GtkWidget *,
				       gpointer);

static void gnomemeeting_druid_get_data (gchar *&,
					 gchar *&,
					 gchar *&,
					 gchar *&,
					 gchar *&,
					 gchar *&,
					 gchar *&,
					 gchar *&);

static void gnomemeeting_druid_cancel (GtkWidget *, gpointer);
static void gnomemeeting_druid_quit (GtkWidget *, gpointer);
static void gnomemeeting_druid_destroy (GtkWidget *, GdkEventAny *, gpointer);
static void gnomemeeting_druid_personal_data_check (GnomeDruid *, int);
static void gnomemeeting_druid_entry_changed (GtkWidget *, gpointer);
static void gnomemeeting_druid_page_prepare (GnomeDruidPage *, GnomeDruid *,
					     gpointer);


static void gnomemeeting_init_druid_audio_manager_page (GnomeDruid *, 
							int,
							int);

static void gnomemeeting_init_druid_audio_devices_page (GnomeDruid *, 
							int,
							int);

static void gnomemeeting_init_druid_connection_type_page (GnomeDruid *, 
							  int, int);

extern GtkWidget *gm;


/* GTK Callbacks */
static gint
kind_of_net_hack (gpointer data)
{
  gconf_set_int (GENERAL_KEY "kind_of_net", GPOINTER_TO_INT (data));

  return FALSE;
}


static void
audio_test_button_clicked (GtkWidget *w,
			   gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;

  gnomemeeting_druid_get_data (name, mail, con_type, audio_manager, player,
			       recorder, video_manager, video_recorder);

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (GTK_TOGGLE_BUTTON (w)->active) {

    /* Try to prevent a crossed mutex deadlock */
    gdk_threads_leave ();
    ep->StartAudioTester (audio_manager, player, recorder);
    gdk_threads_enter ();
  }
  else {

    gdk_threads_leave ();
    ep->StopAudioTester ();
    gdk_threads_enter ();
  }
}


static void
video_test_button_clicked (GtkWidget *w,
			   gpointer data)
{
  GMVideoTester *t = NULL;

  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;

  gnomemeeting_druid_get_data (name, mail, con_type, audio_manager, player,
			       recorder, video_manager, video_recorder);

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester (video_manager, video_recorder);
}


/* DESCRIPTION  :  This callback is called when the user clicks on Cancel.
 * BEHAVIOR     :  Hides the druid and shows GM.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_cancel (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_hide (gw->druid_window);
  gnomemeeting_window_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Destroys the druid, update gconf settings and update
 *                 the internal structures for devices and the corresponding
 *                 prefs window menus. Displays a welcome message.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_quit (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  GMH323EndPoint *ep = NULL;
  
  GtkWidget *active_item = NULL;
  int item_index = 0;
  int version = 0;

  BOOL has_video_device = FALSE;
  
  gchar *name = NULL;
  gchar **couple = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  
  active_item =
    gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu));
  item_index =
    g_list_index (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu))->children, active_item) + 1;

  gnomemeeting_druid_get_data (name, mail, con_type, audio_manager, player,
			       recorder, video_manager, video_recorder);

  
  /* Set the personal data: firstname, lastname and mail
     and ILS registering
  */
  if (name)
    couple = g_strsplit (name, " ", 2);

  if (couple && couple [0])
    gconf_set_string (PERSONAL_DATA_KEY "firstname", couple [0]);
  if (couple && couple [1])
    gconf_set_string (PERSONAL_DATA_KEY "lastname", couple [1]);

  gconf_set_string (PERSONAL_DATA_KEY "mail", mail);
  
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto))
      && mail) {

    if (!gconf_get_bool (LDAP_KEY "enable_registering"))
      gconf_set_bool (LDAP_KEY "enable_registering", TRUE);
    else
      ep->ILSRegister ();
  }
  else {

    gconf_set_bool (LDAP_KEY "enable_registering", FALSE);
  }
  

  /* Set the right devices and managers */
  if (audio_manager)
    gconf_set_string (AUDIO_DEVICES_KEY "plugin", audio_manager);
  if (player)
    gconf_set_string (AUDIO_DEVICES_KEY "output_device", player);
  if (recorder)
    gconf_set_string (AUDIO_DEVICES_KEY "input_device", recorder);
  if (video_manager)
    gconf_set_string (VIDEO_DEVICES_KEY "plugin", video_manager);
  if (video_recorder) {
    
    gconf_set_string (VIDEO_DEVICES_KEY "input_device", video_recorder);
    if (strcmp (video_recorder, _("No device found")))
      has_video_device = TRUE;
  }
  

  /* Set the connection quality settings */
  /* Dialup */
  if (item_index == 1) {
    
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 1);
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 1);
    gconf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 1);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 2) { /* ISDN */
    
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 1);
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 1);
    gconf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 2);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 3) { /* DSL / CABLE */
    
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 8);
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 60);
    gconf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 8);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		    has_video_device);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }
  else if (item_index == 4) { /* LDAN */
    
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 20);
    gconf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 80);
    gconf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 100);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		    has_video_device);
    gconf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }  

  g_timeout_add (2000, (GtkFunction) kind_of_net_hack,
		 GINT_TO_POINTER (item_index));

  
  /* Set User Name and Alias */
  ep->SetUserNameAndAlias ();
  
  
  /* Hide the druid and show GnomeMeeting */
  gnomemeeting_window_hide (GTK_WIDGET (gw->druid_window));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_show (gm);


  /* Will be done through GConf if the manager changes, but not
     if the manager doesn't change */
  GnomeMeeting::Process ()->DetectDevices ();  
  gnomemeeting_pref_window_update_devices_list ();
  

  /* Displays a welcome message */
  if (gconf_get_int (GENERAL_KEY "version") 
      < MAJOR_VERSION * 1000 + MINOR_VERSION * 10 + BUILD_NUMBER)
    gnomemeeting_message_dialog (GTK_WINDOW (gm), _("Welcome to GnomeMeeting 1.00!"), _("Congratulations, you have just successfully launched GnomeMeeting 1.00 for the first time.\nGnomeMeeting is the leading VoIP, videoconferencing and telephony software for Unix.\n\nThanks to all of you who have helped us along the road to our golden 1.00 release!\n\nThe GnomeMeeting Team."));

  
  /* Update the version number */
  version = MAJOR_VERSION*1000+MINOR_VERSION*10+BUILD_NUMBER;
    
  gconf_set_int (GENERAL_KEY "version", version);
}


/* DESCRIPTION  :  This callback is called when the user destroys the druid.
 * BEHAVIOR     :  Exits. 
 * PRE          :  /
 */
static void 
gnomemeeting_druid_destroy (GtkWidget *w, GdkEventAny *ev, gpointer data)
{
  gnomemeeting_druid_cancel (w, data);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok.
 * PRE          :  The druid and the page number.
 */
static void 
gnomemeeting_druid_personal_data_check (GnomeDruid *druid, int page)
{
  GmDruidWindow *dw = NULL;

  PString mail;
  gchar ** couple = NULL;

  BOOL error = TRUE;

  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  if (page == 2) {
      
    couple = g_strsplit (gtk_entry_get_text (GTK_ENTRY (dw->name)), " ", 2);

    if (couple && couple [0] && couple [1]
	&& !PString (couple [0]).Trim ().IsEmpty ()
	&& !PString (couple [1]).Trim ().IsEmpty ()) 
      error = FALSE;
  } else if (page == 3) {

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)))
      error = FALSE;
    else {

      mail = PString (gtk_entry_get_text (GTK_ENTRY (dw->mail)));
      if (!mail.IsEmpty () && mail.Find ("@") != P_MAX_INDEX)
	error = FALSE;
    }
  }
  
  
  if (!error)
    gnome_druid_set_buttons_sensitive (druid, TRUE, TRUE, TRUE, FALSE);
  else
    gnome_druid_set_buttons_sensitive (druid, TRUE, FALSE, TRUE, FALSE);
}


/* DESCRIPTION  :  Called when the user changes an info in the Personal
 *                 Information page.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  GPOINTER_TO_INT (data) == page to which the entry belongs.
 */
static void
gnomemeeting_druid_entry_changed (GtkWidget *w, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  gnomemeeting_druid_personal_data_check (dw->druid, GPOINTER_TO_INT (data));
}


/* DESCRIPTION  :  Called when the user changes the registering toggle.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  /
 */
static void
gnomemeeting_druid_ils_register_changed (GtkToggleButton *b, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  gnomemeeting_druid_personal_data_check (dw->druid, GPOINTER_TO_INT (data));
}


static void
option_menu_update (GtkWidget *option_menu,
		    gchar **options,
		    gchar *default_value)
{
  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  int history = -1;
  int cpt = 0;                                                   

  cpt = 0;

  if (!options)
    return;

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (option_menu));
  menu = gtk_menu_new ();

  while (options [cpt]) {

    if (default_value && !strcmp (options [cpt], default_value)) 
      history = cpt;

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    cpt++;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

  if (history != -1)
    gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), history);
}


static void
gnomemeeting_druid_get_data (gchar * &name,
			     gchar * &mail,
			     gchar * &connection_type,
			     gchar * &audio_manager,
			     gchar * &player,
			     gchar * &recorder,
			     gchar * &video_manager,
			     gchar * &video_recorder)
{
  GmDruidWindow *dw = NULL;

  GtkWidget *child = NULL;
  
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  
  name = (gchar *) gtk_entry_get_text (GTK_ENTRY (dw->name));
  mail = (gchar *) gtk_entry_get_text (GTK_ENTRY (dw->mail));
  child = GTK_BIN (dw->kind_of_net)->child;
  if (child)
    connection_type = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    connection_type = "";

  child = GTK_BIN (dw->audio_manager)->child;
  if (child)
    audio_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    audio_manager = "";

  child = GTK_BIN (dw->video_manager)->child;
  if (child)
    video_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    video_manager = "";

  child = GTK_BIN (dw->audio_player)->child;
  if (child)
    player = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    player = "";

  child = GTK_BIN (dw->audio_recorder)->child;
  if (child)
    recorder = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    recorder = "";

  child = GTK_BIN (dw->video_device)->child;
  if (child)
    video_recorder = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    video_recorder = "";
}


/* DESCRIPTION  :  Called when the user switches between the pages 1, 2, and 6.
 * BEHAVIOR     :  Update the Next/Back buttons following the fields and the
 *                 page. Updates the text of the last page.
 * PRE          :  GPOINTER_TO_INT (data) = page number to prepare.
 */
static void
gnomemeeting_druid_page_prepare (GnomeDruidPage *page,
				 GnomeDruid *druid,
				 gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  GMH323EndPoint *ep = NULL;
  
  gchar *name = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *mail = NULL;
  gchar *text = NULL;
  gchar *connection_type = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_recorder = NULL;
  gchar *video_manager = NULL;
  gchar *audio_manager = NULL;
  gchar *callto_url = NULL;
  BOOL ils_register = FALSE;
  
  int kind_of_net = 0;
  
  char **array = NULL;
  char *options [] =
    {_("56k Modem"),
     _("ISDN"),
     _("xDSL/Cable"),
     _("T1/LAN"),
     _("Keep current settings"), NULL};

  GtkWidget *child = NULL;

  GdkCursor *cursor = NULL;
  
  PStringArray devices;
  
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  if (GPOINTER_TO_INT (data) == 1) {
    
    gnome_druid_set_buttons_sensitive (druid, FALSE, TRUE, TRUE, FALSE);
  }
  else if (GPOINTER_TO_INT (data) == 2) {

  /* When the first page is displayed, and only during that time,
     we update the firstname, lastname, the audio / video managers menus,
     the connection type menu to the default GConf values */

    firstname = gconf_get_string (PERSONAL_DATA_KEY "firstname");
    lastname = gconf_get_string (PERSONAL_DATA_KEY "lastname");
    mail = gconf_get_string (PERSONAL_DATA_KEY "mail");
    kind_of_net = gconf_get_int (GENERAL_KEY "kind_of_net");
    ils_register = gconf_get_bool (LDAP_KEY "enable_registering");

    
    if (!strcmp (gtk_entry_get_text (GTK_ENTRY (dw->name)), "")) {

      if (firstname && lastname
	  && strcmp (firstname, "") && strcmp (lastname, "")) {

	text = g_strdup_printf ("%s %s", firstname, lastname);
	gtk_entry_set_text (GTK_ENTRY (dw->name), text);
	g_free (text);
      }
    }


    if (!strcmp (gtk_entry_get_text (GTK_ENTRY (dw->mail)), "")
	&& mail)
      gtk_entry_set_text (GTK_ENTRY (dw->mail), mail);

    
    option_menu_update (dw->kind_of_net, options, NULL);
    gtk_option_menu_set_history (GTK_OPTION_MENU (dw->kind_of_net),
				 kind_of_net - 1);

    array = gw->audio_managers.ToCharArray ();
    audio_manager = gconf_get_string (AUDIO_DEVICES_KEY "plugin");
    option_menu_update (dw->audio_manager, array, audio_manager);
    free (array);
    
    array = gw->video_managers.ToCharArray ();
    video_manager = gconf_get_string (VIDEO_DEVICES_KEY "plugin");
    option_menu_update (dw->video_manager, array, video_manager);
    free (array);

    GTK_TOGGLE_BUTTON (dw->use_callto)->active = !ils_register;
      
    g_free (video_manager);
    g_free (audio_manager);    
    g_free (mail);
    g_free (firstname);
    g_free (lastname);
  }


  if (GPOINTER_TO_INT (data) == 2
	   || GPOINTER_TO_INT (data) == 3) {	   	   

    gnomemeeting_druid_personal_data_check (druid, GPOINTER_TO_INT (data));
  }
  else if (GPOINTER_TO_INT (data) == 6) {

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				  FALSE);
    
    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, cursor);
    gdk_cursor_unref (cursor);

    child = GTK_BIN (dw->audio_manager)->child;

    if (child)
      audio_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
    else
      audio_manager = "";
	
    player = gconf_get_string (AUDIO_DEVICES_KEY "output_device");
    recorder = gconf_get_string (AUDIO_DEVICES_KEY "input_device");
    
    gnomemeeting_sound_daemons_suspend ();
    if (PString ("Quicknet") == audio_manager)
      devices = OpalIxJDevice::GetDeviceNames ();
    else
      devices = PSoundChannel::GetDeviceNames (audio_manager,
					       PSoundChannel::Player);
    if (devices.GetSize () == 0) {

      devices += PString (_("No device found"));
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
    }
    else
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), TRUE);

    array = devices.ToCharArray ();
    option_menu_update (dw->audio_player, array, player);
    free (array);

    if (PString ("Quicknet") == audio_manager)
      devices = OpalIxJDevice::GetDeviceNames ();
    else
      devices = PSoundChannel::GetDeviceNames (audio_manager,
					       PSoundChannel::Recorder);
    if (devices.GetSize () == 0) {

      devices += PString (_("No device found"));
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
    }
    else 
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), TRUE);

    array = devices.ToCharArray ();
    option_menu_update (dw->audio_recorder, array, recorder);
    free (array);
    gnomemeeting_sound_daemons_resume ();

    gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, NULL);
    
    g_free (player);
    g_free (recorder);

    if (ep->GetCallingState () != GMH323EndPoint::Standby)
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
  }
  else if (GPOINTER_TO_INT (data) == 8) {

    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, cursor);
    gdk_cursor_unref (cursor);

    child = GTK_BIN (dw->video_manager)->child;

    if (child)
      video_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
    else
      video_manager = "";
	
    video_recorder = gconf_get_string (VIDEO_DEVICES_KEY "input_device");
    
    devices = PVideoInputDevice::GetDriversDeviceNames (video_manager);

    if (devices.GetSize () == 0) {

      devices += PString (_("No device found"));
      gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
    }
    else 
      gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), TRUE);

    array = devices.ToCharArray ();
    option_menu_update (dw->video_device, array, video_recorder);
    free (array);

    gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, NULL);
   
    g_free (recorder);

    if (ep->GetCallingState () != GMH323EndPoint::Standby)
      gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
  }
  else if (GPOINTER_TO_INT (data) == 9) {

    gnomemeeting_druid_get_data (name, mail, connection_type, audio_manager,
				 player, recorder, video_manager,
				 video_recorder);
    callto_url = g_strdup_printf ("callto:ils.seconix.com/%s",
				  mail ? mail : "");
    
    text = g_strdup_printf (_("You have now finished the GnomeMeeting configuration. All the settings can be changed in the GnomeMeeting preferences. Enjoy!\n\n\nConfiguration summary:\n\nUsername:  %s\nConnection type:  %s\nAudio manager: %s\nAudio player:  %s\nAudio recorder:  %s\nVideo manager: %s\nVideo player: %s\nCallto URL: %s\n"), name, connection_type, audio_manager, player, recorder, video_manager, video_recorder, !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)) ? callto_url : _("None"));
    gnome_druid_page_edge_set_text (GNOME_DRUID_PAGE_EDGE (page), text);

    g_free (callto_url);
    g_free (text);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Personal Information.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_personal_data_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GmDruidWindow *dw = NULL;
  GtkWidget *page = NULL;


  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Personal Information - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
  
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page));

  
  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  
  /* The user fields */
  label = gtk_label_new (_("Please enter your first name and your surname:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  dw->name = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->name, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your first name and surname will be used when connecting to other VoIP and videoconferencing software."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  g_signal_connect (G_OBJECT (dw->name), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed),
		    GINT_TO_POINTER (p));

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (p));
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for ILS registering and callto.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_callto_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *align = NULL;
  
  gchar *title = NULL;
  gchar *text = NULL;
  
  GmDruidWindow *dw = NULL;
  GtkWidget *page = NULL;


  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Callto URL - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
				
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page));

  
  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  label = gtk_label_new (_("Please enter your e-mail address:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  dw->mail = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->mail, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your e-mail address is used when registering to the GnomeMeeting users directory. It is used to create a callto address permitting your contacts to easily call you wherever you are."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  dw->use_callto = gtk_check_button_new ();
  label = gtk_label_new (_("I don't want to register to the GnomeMeeting users directory and get a callto address"));
  gtk_container_add (GTK_CONTAINER (dw->use_callto), label);
  align = gtk_alignment_new (0, 1.0, 0, 0);
  gtk_container_add (GTK_CONTAINER (align), dw->use_callto);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (dw->mail), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed),
		    GINT_TO_POINTER (p));
  g_signal_connect (G_OBJECT (dw->use_callto), "toggled",
		    G_CALLBACK (gnomemeeting_druid_ils_register_changed),
		    GINT_TO_POINTER (p));

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (p));
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Connection page.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_connection_type_page (GnomeDruid *druid,
					      int p,
					      int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Connection Type - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The connection type */
  label = gtk_label_new (_("Please choose your connection type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->kind_of_net = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->kind_of_net, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The connection type will permit determining the best quality settings that GnomeMeeting will use during calls. You can later change the settings individually in the preferences window."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio manager configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_audio_manager_page (GnomeDruid *druid,
					    int p,
					    int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose your audio manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_manager = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio manager is the plugin that will manage your audio devices, ALSA is probably the best choice when available."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio devices configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_audio_devices_page (GnomeDruid *druid,
					    int p,
					    int t)
{
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose the audio output device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_player = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_player, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio output device is the device managed by the audio manager that will be used to play audio."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please choose the audio input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_recorder = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_recorder, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio input device is the device managed by the audio manager that will be used to record your voice."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (1.0, 0, 0, 0);
  dw->audio_test_button =
    gtk_toggle_button_new_with_label (_("Test Settings"));
  gtk_container_add (GTK_CONTAINER (align), dw->audio_test_button);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);
  
  g_signal_connect (G_OBJECT (dw->audio_test_button), "clicked",
		    GTK_SIGNAL_FUNC (audio_test_button_clicked),
		    (gpointer) druid);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (p));


  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the video manager configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_video_manager_page (GnomeDruid *druid,
					    int p,
					    int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose your video manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->video_manager = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->video_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video manager is the plugin that will manage your video devices, Video4Linux is the most common choice if you own a webcam."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the video devices configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_video_devices_page (GnomeDruid *druid,
					    int p,
					    int t)
{
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  dw = GnomeMeeting::Process ()->GetDruidWindow ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Video devices */
  label = gtk_label_new (_("Please choose the video input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->video_device = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->video_device, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video input device is the device managed by the video manager that will be used to capture video."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (1.0, 0, 0, 0);
  dw->video_test_button =
    gtk_toggle_button_new_with_label (_("Test Settings"));
  gtk_container_add (GTK_CONTAINER (align), dw->video_test_button);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);
  
  g_signal_connect (G_OBJECT (dw->video_test_button), "clicked",
		    GTK_SIGNAL_FUNC (video_test_button_clicked),
		    (gpointer) druid);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (p));

  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}



/* Functions */
GtkWidget *
gnomemeeting_druid_window_new (GmDruidWindow *dw)
{
  GtkWidget *window = NULL;
  
#ifndef DISABLE_GNOME
  gchar *title = NULL;

  GnomeDruidPageEdge *page_final = NULL;
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("druid_window"), g_free); 
  
  gtk_window_set_title (GTK_WINDOW (window), 
			_("First Time Configuration Druid"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  dw->druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (dw->druid));


  title = g_strdup_printf (_("Configuration Druid - page %d/%d"), 1, 9);
  static const gchar text[] =
    N_
    ("This is the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions.\n\nOnce you have completed "
     "these steps, you can always change them later by "
     "selecting Preferences in the Edit menu.");


  /* Create the first page */
  dw->page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new_aa (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (dw->page_edge, title);
  g_free (title);
			   
  gnome_druid_page_edge_set_text (dw->page_edge, _(text));

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  
  g_signal_connect_after (G_OBJECT (dw->page_edge), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (1));


  /* Create the different pages */
  gnomemeeting_init_druid_personal_data_page (dw->druid, 2, 9);
  gnomemeeting_init_druid_callto_page (dw->druid, 3, 9);
  gnomemeeting_init_druid_connection_type_page (dw->druid, 4, 9);
  gnomemeeting_init_druid_audio_manager_page (dw->druid, 5, 9);
  gnomemeeting_init_druid_audio_devices_page (dw->druid, 6, 9);
  gnomemeeting_init_druid_video_manager_page (dw->druid, 7, 9);
  gnomemeeting_init_druid_video_devices_page (dw->druid, 8, 9);

  /*
  gnomemeeting_init_druid_ixj_device_page (dw->druid, 6, 7);
  */

  /* Create final page */
  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  title = g_strdup_printf (_("Configuration complete - page %d/%d"), 9, 9);
  gnome_druid_page_edge_set_title (page_final, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_final));

  g_signal_connect_after (G_OBJECT (page_final), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  GINT_TO_POINTER (9));  

  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (gnomemeeting_druid_quit), dw->druid);

  g_signal_connect (G_OBJECT (dw->druid), "cancel",
		    G_CALLBACK (gnomemeeting_druid_cancel), NULL);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gnomemeeting_druid_destroy), NULL);

  gtk_widget_show_all (GTK_WIDGET (dw->druid));
#endif

  return window;
}

