
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#include "../config.h"

#include "main_window.h"
#include "gnomemeeting.h"
#include "chat_window.h"
#include "config.h"
#include "misc.h"
#include "toolbar.h"
#include "callbacks.h"
#include "tray.h"
#include "lid.h"
#include "sound_handling.h"

#include "dialog.h"
#include "gmentrydialog.h"
#include "stock-icons.h"
#include "gconf_widgets_extensions.h"


#ifndef DISABLE_GNOME
#include <libgnomeui/gnome-window-icon.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-register.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>
#endif

#ifndef WIN32
#include <gdk/gdkx.h>
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include <libxml/parser.h>


#define ACT_IID "OAFIID:GNOME_gnomemeeting_Factory"


/* Declarations */
extern GtkWidget *gm;


static void video_window_shown_cb (GtkWidget *,
				   gpointer);

static gboolean stats_drawing_area_exposed (GtkWidget *,
					    gpointer data);

#ifndef DISABLE_GNOME
static gboolean gnomemeeting_invoke_factory (int, char **);
static void gnomemeeting_new_event (BonoboListener *, const char *, 
				    const CORBA_any *, CORBA_Environment *,
				    gpointer);
static Bonobo_RegistrationResult gnomemeeting_register_as_factory (void);
#endif

static void main_notebook_page_changed (GtkNotebook *, GtkNotebookPage *,
					gint, gpointer);
static void audio_volume_changed       (GtkAdjustment *, gpointer);
static void brightness_changed         (GtkAdjustment *, gpointer);
static void whiteness_changed          (GtkAdjustment *, gpointer);
static void colour_changed             (GtkAdjustment *, gpointer);
static void contrast_changed           (GtkAdjustment *, gpointer);
static void dialpad_button_clicked     (GtkButton *, gpointer);

static gint gm_quit_callback (GtkWidget *, GdkEvent *, gpointer);
static void gnomemeeting_init_main_window_video_settings ();
static void gnomemeeting_init_main_window_audio_settings ();
static void gnomemeeting_init_main_window_stats ();
static void gnomemeeting_init_main_window_dialpad (GtkAccelGroup *);


/* For stress testing */
//int i = 0;

/* GTK Callbacks */
/*
gint StressTest (gpointer data)
 {
   gdk_threads_enter ();


   GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

   if (!GTK_TOGGLE_BUTTON (gw->connect_button)->active) {

     i++;
     cout << "Call " << i << endl << flush;
   }

   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
 				!GTK_TOGGLE_BUTTON (gw->connect_button)->active);

   gdk_threads_leave ();
   return TRUE;
}
*/


/* DESCRIPTION  :  This callback is called when a video window is shown.
 * BEHAVIOR     :  Set the WM HINTS to stay-on-top if the gconf key is set
 *                 to true.
 * PRE          :  /
 */
static void
video_window_shown_cb (GtkWidget *w, gpointer data)
{
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (endpoint && gconf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top")
      && endpoint->GetCallingState () == GMH323EndPoint::Connected)
    gdk_window_set_always_on_top (GDK_WINDOW (w->window), TRUE);
}


static gboolean 
stats_drawing_area_exposed (GtkWidget *drawing_area, gpointer data)
{
  GdkSegment s [50];
  gboolean success [256];

  static PangoLayout *pango_layout = NULL;
  static PangoContext *pango_context = NULL;
  static GdkGC *gc = NULL;
  static GdkColormap *colormap = NULL;

  gchar *pango_text = NULL;
  
  int x = 0, y = 0;
  int cpt = 0;
  int pos = 0;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();
  GmRtpData *rtp = GnomeMeeting::Process ()->GetRtpData (); 
  GdkPoint points [50];

  int width_step = (int) GTK_WIDGET (drawing_area)->allocation.width / 40;
  x = width_step;
  int allocation_height = GTK_WIDGET (drawing_area)->allocation.height;
  float height_step = allocation_height;

  float max_tr_video = 1;
  float max_tr_audio = 1;
  float max_re_video = 1;
  float max_re_audio = 1;

  if (!gc)
    gc = gdk_gc_new (gw->stats_drawing_area->window);

  if (!colormap) {

    colormap = gdk_drawable_get_colormap (gw->stats_drawing_area->window);
    gdk_colormap_alloc_colors (colormap, gw->colors, 6, FALSE, TRUE, success);
  }

  gdk_gc_set_foreground (gc, &gw->colors [0]);
  gdk_draw_rectangle (drawing_area->window,
		      gc,
		      TRUE, 0, 0, 
		      GTK_WIDGET (drawing_area)->allocation.width,
		      GTK_WIDGET (drawing_area)->allocation.height);

  gdk_gc_set_foreground (gc, &gw->colors [1]);
  gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  while ((y < GTK_WIDGET (drawing_area)->allocation.height)&&(cpt < 50)) {

    s [cpt].x1 = 0;
    s [cpt].x2 = GTK_WIDGET (drawing_area)->allocation.width;
    s [cpt].y1 = y;
    s [cpt].y2 = y;
      
    y = y + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (drawing_area->window), gc, s, cpt);

  cpt = 0;
  while ((x < GTK_WIDGET (drawing_area)->allocation.width)&&(cpt < 50)) {

    s [cpt].x1 = x;
    s [cpt].x2 = x;
    s [cpt].y1 = 0;
    s [cpt].y2 = GTK_WIDGET (drawing_area)->allocation.height;
      
    x = x + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (drawing_area->window), gc, s, cpt);
  gdk_window_set_background (drawing_area->window, &gw->colors [0]);


  /* Compute the height_step */
  if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Connected) {

    for (cpt = 0 ; cpt < 50 ; cpt++) {
    
      if (rtp->tr_audio_speed [cpt] > max_tr_audio)
	max_tr_audio = rtp->tr_audio_speed [cpt];
      if (rtp->re_audio_speed [cpt] > max_re_audio)
	max_re_audio = rtp->re_audio_speed [cpt];
      if (rtp->tr_video_speed [cpt] > max_tr_video)
	max_tr_video = rtp->tr_video_speed [cpt];
      if (rtp->re_video_speed [cpt] > max_re_video)
	max_re_video = rtp->re_video_speed [cpt];    
    }
    if (max_re_video > allocation_height / height_step)
      height_step = allocation_height / max_re_video;
    if (max_re_audio > allocation_height / height_step)
      height_step = allocation_height / max_re_audio;
    if (max_tr_video > allocation_height / height_step)
      height_step = allocation_height /  max_tr_video;
    if (max_tr_audio > allocation_height / height_step)
      height_step = allocation_height / max_tr_audio;

    gdk_gc_set_line_attributes (gc, 2, GDK_LINE_SOLID, 
				GDK_CAP_ROUND, GDK_JOIN_BEVEL);

    /* Transmitted audio */
    gdk_gc_set_foreground (gc, &gw->colors [3]);
    pos = rtp->tr_audio_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->tr_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received audio */
    gdk_gc_set_foreground (gc, &gw->colors [5]);
    pos = rtp->re_audio_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->re_audio_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Transmitted video */
    gdk_gc_set_foreground (gc, &gw->colors [4]);
    pos = rtp->tr_video_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->tr_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Received video */
    gdk_gc_set_foreground (gc, &gw->colors [2]);
    pos = rtp->re_video_pos;
    for (cpt = 0 ; cpt < 50 ; cpt++) {

      points [cpt].x = cpt * width_step;

      points [cpt].y = allocation_height -
	(gint) (rtp->re_video_speed [pos] * height_step);
      pos++;

      if (pos >= 50) pos = 0;
    }
    gdk_draw_lines (GDK_DRAWABLE (drawing_area->window), gc, points, 50);


    /* Text */
    if (!pango_context)
      pango_context = gtk_widget_get_pango_context (GTK_WIDGET (drawing_area));
    if (!pango_layout)
      pango_layout = pango_layout_new (pango_context);

    pango_text =
      g_strdup_printf (_("Total: %.2f MB"),
		       (float) (rtp->tr_video_bytes+rtp->tr_audio_bytes
		       +rtp->re_video_bytes+rtp->re_audio_bytes)
		       / (1024*1024));
    pango_layout_set_text (pango_layout, pango_text, strlen (pango_text));
    gdk_draw_layout_with_colors (GDK_DRAWABLE (drawing_area->window),
				 gc, 5, 2,
				 pango_layout,
				 &gw->colors [5], &gw->colors [0]);
    g_free (pango_text);
  }
  else {

    for (cpt = 0 ; cpt < 50 ; cpt++) {

      rtp->re_audio_speed [pos] = 0;
      rtp->re_video_speed [pos] = 0;
      rtp->tr_audio_speed [pos] = 0;
      rtp->tr_video_speed [pos] = 0;
    }    
  }


  return TRUE;
}


/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Calls the user corresponding to the drag data.
 * PRE          :  /
 */
void
dnd_drag_data_received_cb (GtkWidget *widget,
			   GdkDragContext *context,
			   int x,
			   int y,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
			   gpointer data)
{
  GmWindow *gw = NULL;
  gchar **data_split = NULL;


  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (selection_data && selection_data->data) {

    data_split = g_strsplit ((char *) selection_data->data, "|", 0);

    if (data_split && data_split [1]) {

      if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {
      
	/* this function will store a copy of text */
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			    PString (data_split [1]));
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button),
				      true);
      }
    }
  } 
}


/* Factory stuff */

#ifndef DISABLE_GNOME
/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoked remotely to instantiate GnomeMeeting 
 *                 with the given URL.
 * PRE          :  /
 */
static void
gnomemeeting_new_event (BonoboListener    *listener,
			const char        *event_name, 
			const CORBA_any   *any,
			CORBA_Environment *ev,
			gpointer           user_data)
{
  int i;
  int argc;
  char **argv;
  CORBA_sequence_CORBA_string *args;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  args = (CORBA_sequence_CORBA_string *) any->_value;
  argc = args->_length;
  argv = args->_buffer;

  if (strcmp (event_name, "new_gnomemeeting")) {

      g_warning ("Unknown event '%s' on GnomeMeeting", event_name);
      return;
  }

  
  for (i = 1; i < argc; i++) {
    if (!strcmp (argv [i], "-c") || !strcmp (argv [i], "--callto")) 
      break;
  } 


  if ((i < argc) && (i + 1 < argc) && (argv [i+1])) {
    
     /* this function will store a copy of text */
    if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {

      gdk_threads_enter ();
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  argv [i + 1]);
      gdk_threads_leave ();
    }

    gdk_threads_enter ();
    connect_cb (NULL, NULL);
    gdk_threads_leave ();
  }
  else {

    gdk_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (gm), _("Cannot run GnomeMeeting"), _("GnomeMeeting is already running, if you want it to call a given callto or h323 URL, please use \"gnomemeeting -c URL\"."));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Registers GnomeMeeting as a factory.
 * PRE          :  Returns the registration result.
 */
static Bonobo_RegistrationResult
gnomemeeting_register_as_factory (void)
{
  char *per_display_iid;
  BonoboListener *listener;
  Bonobo_RegistrationResult result;

  listener = bonobo_listener_new (gnomemeeting_new_event, NULL);

  per_display_iid = 
    bonobo_activation_make_registration_id (ACT_IID, 
					    DisplayString (gdk_display));

  result = 
    bonobo_activation_active_server_register (per_display_iid, 
					      BONOBO_OBJREF (listener));

  if (result != Bonobo_ACTIVATION_REG_SUCCESS)
    bonobo_object_unref (BONOBO_OBJECT (listener));

  g_free (per_display_iid);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Invoke the factory.
 * PRE          :  Registers the new factory, or use the already registered 
 *                 factory, or displays an error in the terminal.
 */
static gboolean
gnomemeeting_invoke_factory (int argc, char **argv)
{
  Bonobo_Listener listener;

  switch (gnomemeeting_register_as_factory ())
    {
    case Bonobo_ACTIVATION_REG_SUCCESS:
      /* we were the first GnomeMeeting to register */
      return FALSE;

    case Bonobo_ACTIVATION_REG_NOT_LISTED:
      g_printerr (_("It appears that you do not have gnomemeeting.server installed in a valid location. Factory mode disabled.\n"));
      return FALSE;
      
    case Bonobo_ACTIVATION_REG_ERROR:
      g_printerr (_("Error registering GnomeMeeting with the activation service; factory mode disabled.\n"));
      return FALSE;

    case Bonobo_ACTIVATION_REG_ALREADY_ACTIVE:
      /* lets use it then */
      break;
    }


  listener = 
    bonobo_activation_activate_from_id (ACT_IID, 
					Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, 
					NULL, NULL);
  
  if (listener != CORBA_OBJECT_NIL) {

    int i;
    CORBA_any any;
    CORBA_sequence_CORBA_string args;
    CORBA_Environment ev;
    
    CORBA_exception_init (&ev);

    any._type = TC_CORBA_sequence_CORBA_string;
    any._value = &args;

    args._length = argc;
    args._buffer = g_newa (CORBA_char *, args._length);
    for (i = 0; i < (signed) (args._length); i++)
      args._buffer [i] = argv [i];
      
    Bonobo_Listener_event (listener, "new_gnomemeeting", &any, &ev);
    CORBA_Object_release (listener, &ev);

    if (!BONOBO_EX (&ev))
      return TRUE;

    CORBA_exception_free (&ev);
  } 
  else {    
    g_printerr (_("Failed to retrieve gnomemeeting server from activation server\n"));
  }
  
  return FALSE;
}
#endif


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the gconf key accordingly.
 * PRE          :  /
 **/
static void 
main_notebook_page_changed (GtkNotebook *notebook, GtkNotebookPage *page,
			    gint page_num, gpointer user_data) 
{
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  gconf_set_int (USER_INTERFACE_KEY "main_window/control_panel_section",
		 gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)));
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume of the choosen mixers or of the lid.
 * PRE          :  /
 **/
void 
audio_volume_changed (GtkAdjustment *adjustment, gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  
  H323Connection *con = NULL;
  H323Codec *raw_codec = NULL;
  H323Channel *channel = NULL;

  PSoundChannel *sound_channel = NULL;

  unsigned int play_vol =  0, rec_vol = 0;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  play_vol = (unsigned int) (GTK_ADJUSTMENT (gw->adj_play)->value);
  rec_vol = (unsigned int) (GTK_ADJUSTMENT (gw->adj_rec)->value);

  gdk_threads_leave ();
 
  con = ep->FindConnectionWithLock (ep->GetCurrentCallToken ());

  if (con) {

    for (int cpt = 0 ; cpt < 2 ; cpt++) {

      channel = 
        con->FindChannel (RTP_Session::DefaultAudioSessionID, (cpt == 0));         
      if (channel) {

        raw_codec = channel->GetCodec();

        if (raw_codec) {

          sound_channel = (PSoundChannel *) raw_codec->GetRawDataChannel ();

          if (sound_channel)
            ep->SetDeviceVolume (sound_channel, 
                                 (cpt == 1), 
                                 (cpt == 1) ? rec_vol : play_vol);
        }
      }
    }
    con->Unlock ();
  }

  gdk_threads_enter ();
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video brightness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMH323EndPoint *ep = NULL;
  GMVideoGrabber *video_grabber = NULL;

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);


  /* Notice about mutexes:
     The GDK lock is taken in the callback. We need to release it, because
     if CreateVideoGrabber is called in another thread, it will only
     release its internal mutex (also used by GetVideoGrabber) after it 
     returns, but it will return only if it is opened, and it can't open 
     if the GDK lock is held as it will wait on the GDK lock before 
     updating the GUI */
  gdk_threads_leave ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetBrightness (brightness << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video whiteness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = NULL;
  GMH323EndPoint *ep = NULL;
  
  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);

  gdk_threads_leave ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetWhiteness (whiteness << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video colour slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 */
void 
colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = NULL;
  GMH323EndPoint *ep = NULL;
  
  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);

  gdk_threads_leave ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetColour (colour << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video contrast slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GmWindow
 **/
void 
contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = NULL;
  GMH323EndPoint *ep = NULL;
  
  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);

  gdk_threads_leave ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetContrast (contrast << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


/**
 * DESCRIPTION  :  This callback is called when the user 
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Puts it in the URL at the right place, and also sends 
 *                 the corresponding UserInput if we are in a connection.
 * PRE          :  gpointer is a valid pointer containing the button pressed
 *                 symbol (char *).
 **/
static void 
dialpad_button_clicked (GtkButton *button, gpointer data)
{
  GtkWidget *label = NULL;
  const char *button_text = NULL;

  label = gtk_bin_get_child (GTK_BIN (button));
  button_text = gtk_label_get_text (GTK_LABEL (label));

  if (button_text
      && strcmp (button_text, "")
      && strlen (button_text) > 1
      && button_text [0])
    gnomemeeting_dialpad_event (button_text [0]);
}


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is 
 *                 not shown else hide GM.
 * PRE          :  /
 */
static gint 
gm_quit_callback (GtkWidget *widget, GdkEvent *event, 
			      gpointer data)
{
  gboolean b;
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  b = gnomemeeting_tray_is_embedded (gw->docklet);

  if (!b)
    quit_callback (NULL, data);
  else 
    gnomemeeting_window_hide (GTK_WIDGET (gm));

  return (TRUE);
}  


/* The functions */
void gnomemeeting_dialpad_event (const char d)
{
  GMH323EndPoint *endpoint = NULL;
  H323Connection *connection = NULL;

#ifdef HAS_IXJ
  GMLid *lid = NULL;
#endif
  
  PString url;
  PString new_url;

  char dtmf = d;
  gchar *msg = NULL;
  
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (gw->transfer_call_popup)
    url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (gw->transfer_call_popup));
  else
    url = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)); 

  
  if (endpoint->GetCallingState () == GMH323EndPoint::Standby) {

    /* Replace the * by a . */
    if (dtmf == '*') 
      dtmf = '.';
  }
      
  new_url = PString (url) + dtmf;

  if (gw->transfer_call_popup)
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (gw->transfer_call_popup),
			      new_url);
  else if (endpoint->GetCallingState () == GMH323EndPoint::Standby)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), new_url);

  if (dtmf == '#' && gw->transfer_call_popup)
    gtk_dialog_response (GTK_DIALOG (gw->transfer_call_popup),
			 GTK_RESPONSE_ACCEPT);
  
  if (endpoint->GetCallingState () == GMH323EndPoint::Connected
      && !gw->transfer_call_popup) {

    gdk_threads_leave ();
    connection = 
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
            
    if (connection) {

      msg = g_strdup_printf (_("Sent dtmf %c"), dtmf);
      
      connection->SendUserInput (dtmf);
      connection->Unlock ();
    }
    gdk_threads_enter ();

    if (msg) {

      gnomemeeting_statusbar_flash (gw->statusbar, msg);
      g_free (msg);
    }
  }

#ifdef HAS_IXJ
  lid = endpoint->GetLid ();
  if (lid) {

    lid->StopTone (0);
    lid->Unlock ();
  }
#endif
}


void
gnomemeeting_main_window_update_sensitivity (unsigned calling_state)
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  switch (calling_state)
    {
    case GMH323EndPoint::Standby:

      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);
      break;


    case GMH323EndPoint::Calling:

      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
      break;


    case GMH323EndPoint::Connected:

      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
      break;


    case GMH323EndPoint::Called:

      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      break;
    }
}


void
gnomemeeting_main_window_update_sensitivity (BOOL is_video,
					     BOOL is_receiving,
					     BOOL is_transmitting)
{
  GmWindow *gw = NULL;
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (is_video) {

    frame = gw->video_settings_frame;
    button = gw->video_chan_button;
  }
  else {

    frame = gw->audio_settings_frame;
    button = gw->audio_chan_button;
  }
  
  gtk_widget_set_sensitive (GTK_WIDGET (button), is_transmitting);
  gtk_widget_set_sensitive (GTK_WIDGET (frame), is_transmitting);
}


GtkWidget *
gnomemeeting_main_window_new (GmWindow *gw)
{
  GtkWidget *window = gm;
  GtkWidget *table = NULL;	
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkAccelGroup *accel = NULL;
#ifdef DISABLE_GNOME
  GtkWidget *window_vbox = NULL;
  GtkWidget *window_hbox = NULL;
#endif
  GtkWidget *event_box = NULL;
  GtkWidget *main_toolbar = NULL;
  GtkWidget *left_toolbar = NULL;

  int main_notebook_section = 0;

  GmTextChat *chat = NULL;

  static GtkTargetEntry dnd_targets [] =
  {
    {"text/plain", GTK_TARGET_SAME_APP, 0}
  };

  chat = GnomeMeeting::Process ()->GetTextChat ();
  
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);

#ifdef DISABLE_GNOME
  window_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), window_vbox);
  gtk_widget_show (window_vbox);
#endif

    /* The statusbar and the progressbar */
  hbox = gtk_hbox_new (0, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (window_vbox), hbox, 
		      FALSE, FALSE, 0);
#else
  gnome_app_add_docked (GNOME_APP (window), hbox, "statusbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_BOTTOM, 3, 0, 0);
#endif
  gtk_widget_show (hbox);

  
  gw->statusbar = gtk_statusbar_new ();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (gw->statusbar), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), gw->statusbar, 
		      TRUE, TRUE, 0);

  if (gconf_get_bool (USER_INTERFACE_KEY "main_window/show_status_bar"))
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  
  gw->main_menu = gnomemeeting_init_menu (accel);
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (window), 
			gw->main_menu,
			"menubar",
			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 0, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (window_vbox), gw->main_menu,
		      FALSE, FALSE, 0);
#endif

  main_toolbar = gnomemeeting_init_main_toolbar ();
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (window), main_toolbar, "main_toolbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 1, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (window_vbox), main_toolbar, 
		      FALSE, FALSE, 0);
#endif

  left_toolbar = gnomemeeting_init_left_toolbar ();
#ifndef DISABLE_GNOME
  gnome_app_add_toolbar (GNOME_APP (window), GTK_TOOLBAR (left_toolbar),
 			 "left_toolbar", BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			 BONOBO_DOCK_LEFT, 2, 0, 0);
#else
  window_hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (window_vbox), window_hbox, 
		      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (window_hbox), left_toolbar, 
		      FALSE, FALSE, 0);
  gtk_widget_show (window_hbox);
#endif

  gtk_widget_show (main_toolbar);
  gtk_widget_show (left_toolbar);

  
  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (window_hbox), table, FALSE, FALSE, 0);
#else
  gnome_app_set_contents (GNOME_APP (window), table);
#endif
  gtk_widget_show (table);

  /* The Notebook */
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gw->main_notebook), TRUE);

  gnomemeeting_init_main_window_stats ();
  gnomemeeting_init_main_window_dialpad (accel);
  gnomemeeting_init_main_window_audio_settings ();
  gnomemeeting_init_main_window_video_settings ();

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    6, 6); 

  main_notebook_section = 
    gconf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section");

  if (main_notebook_section != GM_MAIN_NOTEBOOK_HIDDEN) {

    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK ((gw->main_notebook)), 
				   main_notebook_section);
  }


  /* The drawing area that will display the webcam images */

  /* The frame that contains video and remote name display */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

  /* The frame that contains the video */
  gw->video_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_frame), GTK_SHADOW_IN);
  
  event_box = gtk_event_box_new ();
  gw->video_popup_menu = gnomemeeting_video_popup_init_menu (event_box, accel);

  vbox = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), gw->video_frame, TRUE, TRUE, 0);

  gw->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_frame), 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->main_video_image);

  gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame), 
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, 
			       GM_QCIF_HEIGHT + GM_FRAME_SIZE); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    (GtkAttachOptions) GTK_EXPAND,
		    (GtkAttachOptions) GTK_EXPAND,
		    6, 6);

  gtk_widget_show_all (GTK_WIDGET (frame));

  
  /* The 2 video window popups */
  gw->local_video_window =
    gnomemeeting_video_window_new (_("Local Video"),
				   gw->local_video_image,
				   "local_video_window");
  gw->remote_video_window =
    gnomemeeting_video_window_new (_("Remote Video"),
				   gw->remote_video_image,
				   "remote_video_window");
  
  gnomemeeting_init_main_window_logo (gw->main_video_image);

  g_signal_connect (G_OBJECT (gw->local_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);
  g_signal_connect (G_OBJECT (gw->remote_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);


  /* The remote name */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  gw->remote_name = gtk_label_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (gw->remote_name), 
			       GM_QCIF_WIDTH, -1);

  gtk_container_add (GTK_CONTAINER (frame), gw->remote_name);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  gw->chat_window = gnomemeeting_text_chat_new (chat);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    6, 6);
  if (gconf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"))
    gtk_widget_show_all (GTK_WIDGET (gw->chat_window));
  
  gtk_widget_set_size_request (GTK_WIDGET (gw->main_notebook),
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, -1);
  gtk_widget_set_size_request (GTK_WIDGET (window), -1, -1);

  
  /* Add the window icon and title */
  gtk_window_set_title (GTK_WINDOW (window), _("GnomeMeeting"));
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
			      "/gnomemeeting-logo-icon.png", NULL);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (window);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_window_set_resizable (GTK_WINDOW (window), false);

  g_signal_connect_after (G_OBJECT (gw->main_notebook), "switch-page",
			  G_CALLBACK (main_notebook_page_changed), NULL);


  /* Init the Drag and drop features */
  gtk_drag_dest_set (GTK_WIDGET (window), GTK_DEST_DEFAULT_ALL,
		     dnd_targets, 1,
		     GDK_ACTION_COPY);

  g_signal_connect (G_OBJECT (window), "drag_data_received",
		    G_CALLBACK (dnd_drag_data_received_cb), 0);

  /* if the user tries to close the window : delete_event */
  g_signal_connect (G_OBJECT (gm), "delete_event",
		    G_CALLBACK (gm_quit_callback), (gpointer) gw);

  
  return window;
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the statistics part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_stats ()
{
  GtkWidget *frame2 = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  /* The first frame with statistics display */
  frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);

  vbox = gtk_vbox_new (FALSE, 6);
  gw->stats_drawing_area = gtk_drawing_area_new ();

  gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (frame2), gw->stats_drawing_area);

  gtk_widget_set_size_request (GTK_WIDGET (frame2), GM_QCIF_WIDTH, 47);
  gw->colors [0].red = 0;
  gw->colors [0].green = 0;
  gw->colors [0].blue = 0;

  gw->colors [1].red = 169;
  gw->colors [1].green = 38809;
  gw->colors [1].blue = 52441;

  gw->colors [2].red = 0;
  gw->colors [2].green = 65535;
  gw->colors [2].blue = 0;
  
  gw->colors [3].red = 65535;
  gw->colors [3].green = 0;
  gw->colors [3].blue = 0;

  gw->colors [4].red = 0;
  gw->colors [4].green = 0;
  gw->colors [4].blue = 65535;

  gw->colors [5].red = 65535;
  gw->colors [5].green = 54756;
  gw->colors [5].blue = 0;

  g_signal_connect (G_OBJECT (gw->stats_drawing_area), "expose_event",
		    G_CALLBACK (stats_drawing_area_exposed), 
		    NULL);

  gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);


  /* The second one with some labels */
  gw->stats_label =
    gtk_label_new (_("Lost packets:\nLate packets:\nRound-trip delay:\nJitter buffer:"));
  gtk_misc_set_alignment (GTK_MISC (gw->stats_label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gw->stats_label, FALSE, TRUE,
		      GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (_("Statistics"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), vbox, label);
}


/*
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the dialpad part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_dialpad (GtkAccelGroup *accel)
{
  GtkWidget *label = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;

  int i = 0;

  GmWindow *gw = NULL;

  char *key_n [] = { "1", "2", "3", "4", "5", "6", "7", "8", "9",
		     "*", "0", "#"};
  char *key_a []= { "  ", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv",
		   "wxyz", "  ", "  ", "  "};

  gchar *text_label = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  table = gtk_table_new (4, 3, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  
  for (i = 0 ; i < 12 ; i++) {

    label = gtk_label_new (NULL);
    text_label =
      g_strdup_printf ("%s<sub><span size=\"small\">%s</span></sub>",
		       key_n [i], key_a [i]);
    gtk_label_set_markup (GTK_LABEL (label), text_label); 
    button = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button), 0);
    gtk_container_add (GTK_CONTAINER (button), label);
    
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (button), 
		      i%3, i%3+1, i/3, i/3+1,
		      (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL),
		      1, 1);
    
    g_signal_connect (G_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialpad_button_clicked), NULL);

    g_free (text_label);
  }
  
  label = gtk_label_new (_("Dialpad"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook),
			    table, label);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the video settings part of the main window. This
 *                 part is made unsensitive while the grabber is not enabled.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_video_settings ()
{
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;

  GtkWidget *hscale_brightness, *hscale_colour, 
    *hscale_contrast, *hscale_whiteness;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  
  /* Webcam Control Frame, we need it to disable controls */		
  gw->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_settings_frame), 0);
  
  /* Category */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->video_settings_frame), vbox);

  
  /* Brightness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_BRIGHTNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  gw->adj_brightness = gtk_adjustment_new (brightness, 0.0, 
					   255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_brightness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_brightness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_brightness,
			_("Adjust brightness"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_brightness), "value-changed",
		    G_CALLBACK (brightness_changed), (gpointer) gw);


  /* Whiteness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_WHITENESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  gw->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
					  255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_whiteness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_whiteness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_whiteness,
			_("Adjust whiteness"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_whiteness), "value-changed",
		    G_CALLBACK (whiteness_changed), (gpointer) gw);


  /* Colour */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_COLOURNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  gw->adj_colour = gtk_adjustment_new (colour, 0.0, 
				       255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_colour));
  gtk_range_set_update_policy (GTK_RANGE (hscale_colour),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_colour,
			_("Adjust color"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_colour), "value-changed",
		    G_CALLBACK (colour_changed), (gpointer) gw);


  /* Contrast */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_CONTRAST, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  
  gw->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					 255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_contrast));
  gtk_range_set_update_policy (GTK_RANGE (hscale_contrast),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (gw->tips, hscale_contrast,
			_("Adjust contrast"), NULL);

  g_signal_connect (G_OBJECT (gw->adj_contrast), "value-changed",
		    G_CALLBACK (contrast_changed), (gpointer) gw);
  

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(gw->main_notebook), 
			    gw->video_settings_frame, label);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the audio setting part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_audio_settings ()
{
  GtkWidget *label = NULL;
  GtkWidget *hscale_play = NULL, *hscale_rec = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;

  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();


  /* Webcam Control Frame, we need it to disable controls */		
  gw->audio_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->audio_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (gw->audio_settings_frame), 0);


  /* The vbox */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->audio_settings_frame), vbox);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_settings_frame), FALSE);
  

  /* Audio volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_VOLUME, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);
  
  gw->adj_play = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_play));
  gtk_range_set_update_policy (GTK_RANGE (hscale_play),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_play, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  /* Recording volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_MICROPHONE, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);

  gw->adj_rec = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_rec));
  gtk_range_set_update_policy (GTK_RANGE (hscale_rec),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_rec, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  g_signal_connect (G_OBJECT (gw->adj_play), "value-changed",
		    G_CALLBACK (audio_volume_changed), NULL);

  g_signal_connect (G_OBJECT (gw->adj_rec), "value-changed",
		    G_CALLBACK (audio_volume_changed), NULL);

		    
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook),
			    gw->audio_settings_frame, label);
}


/* The main () */

int main (int argc, char ** argv, char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  GtkWidget *dialog = NULL;
  
  GmWindow *gw = NULL;

  gchar *url = NULL;
  gchar *key_name = NULL;
  gchar *msg = NULL;

  int debug_level = 0;

  
#ifndef WIN32
  setenv ("ESD_NO_SPAWN", "1", 1);
#endif
  

  /* Threads + Locale Init + Gconf */
  g_thread_init (NULL);
  gdk_threads_init ();
  
#ifndef WIN32
  gtk_init (&argc, &argv);
#else
  gtk_init (NULL, NULL);
#endif

  xmlInitParser ();
  
#ifndef DISABLE_GNOME
  /* Cope with command line options */
  struct poptOption arguments [] =
    {
      {"debug", 'd', POPT_ARG_INT, &debug_level, 
       1, N_("Prints debug messages in the console (level between 1 and 6)"), 
       NULL},
      {"call", 'c', POPT_ARG_STRING, &url,
       1, N_("Makes GnomeMeeting call the given URL"), NULL},
      {NULL, '\0', 0, NULL, 0, NULL, NULL}
    };
  

  /* Initialize gettext */
  textdomain (GETTEXT_PACKAGE);
#ifndef WIN32
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
#endif
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  
  /* Select the Mic as default source for OSS. Will be removed when
   * ALSA will be everywhere 
   */
  gnomemeeting_mixers_mic_select ();


  /* GnomeMeeting Initialisation */
  gnome_program_init ("gnomemeeting", VERSION,
		      LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PARAM_POPT_TABLE, arguments,
		      GNOME_PARAM_HUMAN_READABLE_NAME,
		      "gnomemeeting",
		      GNOME_PARAM_APP_DATADIR, DATADIR,
		      (void *) NULL);

  gm = gnome_app_new ("gnomemeeting", NULL);
#else
  gm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
  g_object_set_data_full (G_OBJECT (gm), "window_name",
			  g_strdup ("main_window"), g_free);
  
  g_signal_connect (G_OBJECT (gm), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);
  
  gdk_threads_enter ();
  gconf_init (argc, argv, 0);
 
  /* The factory */
#ifndef DISABLE_GNOME
  if (gnomemeeting_invoke_factory (argc, argv))
    exit (1);
#endif

  
  /* Upgrade the preferences */
  gnomemeeting_gconf_upgrade ();

  
  /* GnomeMeeting main initialisation */
  static GnomeMeeting instance;

  /* Debug */
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (4, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);

  
  /* Detect the devices, exit if it fails */
  if (!GnomeMeeting::Process ()->DetectDevices ()) {

    dialog = gnomemeeting_error_dialog (NULL, _("No usable audio manager detected"), _("GnomeMeeting didn't find any usable sound manager. Make sure that your installation is correct."));
    
    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));

    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }


  /* Init the process and build the GUI */
  GnomeMeeting::Process ()->BuildGUI ();
  GnomeMeeting::Process ()->Init ();


  /* Init the GConf DB, exit if it fails */
  if (!gnomemeeting_init_gconf (gconf_client_get_default ())) {

    key_name = g_strdup ("\"/apps/gnomemeeting/general/gconf_test_age\"");
    msg = g_strdup_printf (_("GnomeMeeting got an invalid value for the GConf key %s.\n\nIt probably means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org) about this problem."), key_name);
    
    dialog = gnomemeeting_error_dialog (GTK_WINDOW (gm),
					_("Gconf key error"), msg);

    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));


    g_free (msg);
    g_free (key_name);
    
    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }

  
  /* Call the given host if needed */
  if (url) {

    gw = GnomeMeeting::Process ()->GetMainWindow ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), url);
    connect_cb (NULL, NULL);
  }

  //  gtk_timeout_add (15000, (GtkFunction) StressTest, 
  //		   NULL);
  
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  //  delete (GnomeMeeting::Process ());
  
#ifdef DISABLE_GCONF
  gconf_save_content_to_file ();
#endif

  return 0;
}


#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR     lpCmdLine,
		     int       nCmdShow)
{
  return main (0, NULL, NULL);
}
#endif
