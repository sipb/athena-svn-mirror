
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
 *                         lid.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains LID functions.
 *
 */


#include "../config.h"

#include "lid.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "urlhandler.h"
#include "misc.h"
#include "tools.h"
#include "main_window.h"
#include "pref_window.h"
#include "callbacks.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"


static gboolean transfer_callback_cb (gpointer data);

/* Declarations */
extern GtkWidget *gm;


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Hack to be able to call the transfer_call_cb in as a g_idle
 *                 from a thread.
 * PRE          :  /
 */
static gboolean
transfer_callback_cb (gpointer data)
{
  gdk_threads_enter ();
  transfer_call_cb (NULL, NULL);
  gdk_threads_leave ();

  return false;
}


#ifdef HAS_IXJ
GMLid::GMLid (PString d)
  :PThread (1000, NoAutoDeleteThread)
{
  stop = FALSE;
  soft_codecs = FALSE;
  
  dev_name = d;

  /* Open the device */
  Open ();

  this->Resume ();
  thread_sync_point.Wait ();
} 


GMLid::~GMLid ()
{
  stop = TRUE;
  
  PWaitAndSignal m(quit_mutex);
  
  Close ();
}


BOOL
GMLid::Open ()
{
  GMH323EndPoint *ep = NULL;
  GmWindow *gw = NULL;

  gchar *lid_country = NULL;

  int lid_aec = 0;
  int lid_odt = 0;

  BOOL return_val = FALSE;

  PWaitAndSignal m(device_access_mutex);
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (!IsOpen ()) {

    /* We use the default settings, but the device name provided as argument */
    gnomemeeting_threads_enter ();
    lid_country = gconf_get_string (AUDIO_DEVICES_KEY "lid_country_code");
    lid_aec = gconf_get_int (AUDIO_DEVICES_KEY "lid_echo_cancellation_level");
    lid_odt = gconf_get_int (AUDIO_DEVICES_KEY "lid_output_device_type");
    gnomemeeting_threads_leave ();

    if (OpalIxJDevice::Open (dev_name)) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Opened Quicknet device %s"), 
				(const char *) GetName ()); // FIXME: is it thread-safe!?
      gnomemeeting_threads_leave ();
      
      if (lid_country)
	SetCountryCodeName(lid_country);
      
      SetAEC (0, (OpalLineInterfaceDevice::AECLevels) lid_aec);
      StopTone (0);
      SetLineToLineDirect (0, 1, FALSE);
      if (lid_odt == 0) // POTS
	EnableAudio (0, TRUE);
      else 
	EnableAudio (0, FALSE);

      soft_codecs =
	GetMediaFormats ().GetValuesIndex (OpalMediaFormat (OPAL_PCM16)) 
	!= P_MAX_INDEX;
      return_val = TRUE;
    }
    else {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Error while opening the Quicknet device."), _("Please check that your driver is correctly installed and that the device is working correctly."));
      gnomemeeting_threads_leave ();

      return_val = FALSE;
    }
  }

  g_free (lid_country);
  
  return return_val;
}


BOOL
GMLid::Close ()
{
  GmWindow *gw = NULL;
  
  PWaitAndSignal m(device_access_mutex);

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (IsOpen ()) {

    OpalIxJDevice::Close ();

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (_("Closed Quicknet device %s"), 
			     (const char *) GetName ()); // FIXME: is it thread-safe?!
    gnomemeeting_threads_leave ();
  }

  return TRUE;
}


void
GMLid::Main ()
{
  GMH323EndPoint *endpoint = NULL;

  GmWindow *gw = NULL;
  GmPrefWindow *pw = NULL;

  BOOL off_hook = TRUE;
  BOOL last_off_hook = TRUE;
  BOOL do_not_connect = TRUE;

  char c = 0;
  char old_c = 0;
  const char *url = NULL;
  
  PTime now;
  PTime last_key_press;

  unsigned int vol = 0;
  int lid_odt = 0;
  GMH323EndPoint::CallingState calling_state = GMH323EndPoint::Standby;
  
  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  
  /* Check the initial hook status. */
  off_hook = last_off_hook = IsLineOffHook (OpalIxJDevice::POTSLine);

  gnomemeeting_threads_enter ();
  lid_odt = gconf_get_int (AUDIO_DEVICES_KEY "lid_output_device_type");
  gnomemeeting_threads_leave ();
  

  gnomemeeting_threads_enter ();
  /* Update the mixers if the lid is used */
  GetPlayVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_play)->value = (int) (vol);
  GetRecordVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_rec)->value = (int) (vol);
  gtk_widget_queue_draw (GTK_WIDGET (gw->audio_settings_frame));

  /* Update the codecs list */
  gnomemeeting_codecs_list_build (pw->codecs_list_store, TRUE, soft_codecs);
  gnomemeeting_threads_leave ();

  endpoint->AddAllCapabilities ();

  while (IsOpen() && !stop) {

    calling_state = endpoint->GetCallingState ();

    off_hook = IsLineOffHook (0);
    now = PTime ();


    /* If there is a DTMF, trigger the dialpad event */
    c = ReadDTMF (0);
    if (c) {

      gnomemeeting_threads_enter ();
      gnomemeeting_dialpad_event (c);
      gnomemeeting_threads_leave ();

      last_key_press = PTime ();
      do_not_connect = FALSE;
    }


    /* If there is a state change: the phone is put off hook */
    if (off_hook == TRUE && last_off_hook == FALSE) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Phone is off hook"));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Phone is off hook"));
      gnomemeeting_threads_leave ();


      /* We answer the call */
      if (calling_state == GMH323EndPoint::Called)
	GnomeMeeting::Process ()->Connect ();


      /* We are in standby */
      if (calling_state == GMH323EndPoint::Standby) {

	if (lid_odt == 0) { /* POTS: Play a dial tone */

	  PlayTone (0, OpalLineInterfaceDevice::DialTone);
	  EnableAudio (0, TRUE);
	}
	else
	  EnableAudio (0, FALSE);

	gnomemeeting_threads_enter ();
	url = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)); 
	gnomemeeting_threads_leave ();

	/* Automatically connect */
	if (url && !GMURL (url).IsEmpty ())
	  GnomeMeeting::Process ()->Connect ();
      }
    }

    
    /* if phone is put on hook */
    if (off_hook == FALSE && last_off_hook == TRUE) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Phone is on hook"));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Phone is on hook"));
  
      /* Remove the current called number */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  GMURL ().GetDefaultURL ());
      gnomemeeting_threads_leave ();

      RingLine (0, 0);

      /* If we are in a call, or calling, we disconnect */
      if (calling_state == GMH323EndPoint::Connected
	  || calling_state == GMH323EndPoint::Calling)
	GnomeMeeting::Process ()->Disconnect ();
    }


    /* Examine the DTMF history for special actions */
    if (off_hook == TRUE) {

      PTimeInterval t = now - last_key_press;

      /* Trigger the call after 3 seconds, or on # pressed if
       we are in standby */
      if ((t.GetSeconds () > 3 && !do_not_connect)
	  || c == '#') {

	if (calling_state == GMH323EndPoint::Standby)
	  GnomeMeeting::Process ()->Connect ();

	do_not_connect = TRUE;
      }


      /* *1 to hold a call */
      if (old_c == '*' && c == '1'
	  && calling_state == GMH323EndPoint::Connected) {

	gnomemeeting_threads_enter ();
	hold_call_cb (NULL, NULL);
	gnomemeeting_threads_leave ();
      }


      /* *2 to transfer a call */
      if (old_c == '*' && c == '2'
	  && calling_state == GMH323EndPoint::Connected) {

	gnomemeeting_threads_enter ();
	g_idle_add (transfer_callback_cb, NULL);
	gnomemeeting_threads_leave ();
      }
    }


    /* Save some history */
    last_off_hook = off_hook;
    if (c)
      old_c = c;

    
    /* We must poll to read the hook state */
    PThread::Sleep (50);
  }

  /* Update the codecs list */
  gnomemeeting_threads_enter ();
  gnomemeeting_codecs_list_build (pw->codecs_list_store, FALSE, FALSE);
  gnomemeeting_threads_leave ();
}


void
GMLid::UpdateState (GMH323EndPoint::CallingState i)
{
  int lid_odt = 0;

  lid_odt = gconf_get_int (AUDIO_DEVICES_KEY "lid_output_device_type");
  
  if (IsOpen ()) {

    switch (i) {

    case GMH323EndPoint::Calling:
      RingLine (0, 0);
      PlayTone (0, OpalLineInterfaceDevice::RingTone);
      SetRemoveDTMF (0, TRUE);
      
      break;

    case GMH323EndPoint::Called: 

      if (lid_odt == 0)
	RingLine (OpalIxJDevice::POTSLine, 0x33);
      else
	RingLine (0, 0);
      break;

    case GMH323EndPoint::Standby: /* Busy */
      RingLine (0, 0);
      PlayTone (0, OpalLineInterfaceDevice::BusyTone);
      if (lid_odt == 1) {
	
	PThread::Current ()->Sleep (2800);
	StopTone (0);
      }
      break;

    case GMH323EndPoint::Connected:

      RingLine (0, 0);
      StopTone (0);
    }    

    
    if (lid_odt == 0) // POTS
      EnableAudio (0, TRUE);
    else 
      EnableAudio (0, FALSE);
  }
}


void
GMLid::SetVolume (BOOL is_encoding, 
                  int x)
{
  if (IsOpen ()) {

    if (!is_encoding)
      SetPlayVolume (0, x);
    else
      SetRecordVolume (0, x);
  }
}


void
GMLid::PlayDTMF (const char *digits)
{
  if (IsOpen () && digits) {

    StopTone (0);
    OpalIxJDevice::PlayDTMF (0, digits);
  }
}


BOOL
GMLid::areSoftwareCodecsSupported ()
{
  if (IsOpen ())
    return soft_codecs;
  else
    return FALSE;
}


void
GMLid::Lock ()
{
  device_access_mutex.Wait ();
}


void
GMLid::Unlock ()
{
  device_access_mutex.Signal ();
}
#endif
