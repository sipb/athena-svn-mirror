
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
 *                         connection.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains connection related functions.
 *
 */


#include "../config.h"

#include "connection.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "tools.h"
#include "chat_window.h"
#include "main_window.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"

#include <h323pdu.h>

#define new PNEW


/* Declarations */
extern GtkWidget *gm;


/* The functions */
GMH323Connection::GMH323Connection (GMH323EndPoint & ep, 
				    unsigned callReference)
  :H323Connection(ep, callReference)
{
  int min_jitter = 20;
  int max_jitter = 1000;

  gnomemeeting_threads_enter ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  opened_audio_channels = 0;
  opened_video_channels = 0;

  min_jitter = 
    gconf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");

  max_jitter = 
    gconf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
  gnomemeeting_threads_leave ();

  SetAudioJitterDelay (PMAX (min_jitter, 20), PMIN (max_jitter, 1000));

  is_transmitting_video = FALSE;
  is_transmitting_audio = FALSE;
  is_receiving_video = FALSE;
  is_receiving_audio = FALSE;  
}


void
GMH323Connection::OnClosedLogicalChannel(const H323Channel &channel)
{
  OnLogicalChannel ((H323Channel *) &channel, TRUE);
}


BOOL
GMH323Connection::OnStartLogicalChannel (H323Channel &channel)
{
  return  OnLogicalChannel ((H323Channel *) &channel, FALSE);
}


BOOL
GMH323Connection::OnLogicalChannel (H323Channel *channel,
				    BOOL is_closing)
{
  PString codec_name;
  BOOL is_encoding = FALSE;
  BOOL is_video = FALSE;
  BOOL preview = FALSE;
  
  gchar *msg = NULL;
  
  PWaitAndSignal m(channels);

  is_video = (channel->GetCodec ()->IsDescendant (H323VideoCodec::Class ()));
  is_encoding = (channel->GetDirection () == H323Channel::IsTransmitter);
  codec_name = channel->GetCapability ().GetFormatName ();

  gnomemeeting_threads_enter ();
  preview = gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  gnomemeeting_threads_leave ();

  if (is_video) {
    
    is_closing ?
      (is_encoding ? is_transmitting_video = FALSE:is_receiving_video = FALSE)
      :(is_encoding ? is_transmitting_video = TRUE:is_receiving_video = TRUE);
  }
  else {
    
    is_closing ?
      (is_encoding ? is_transmitting_audio = FALSE:is_receiving_audio = FALSE)
      :(is_encoding ? is_transmitting_audio = TRUE:is_receiving_audio = TRUE);
  }

  
  if (!is_closing) {

    if (!H323Connection::OnStartLogicalChannel (*channel))
      return FALSE;
  }
  else {

    H323Connection::OnClosedLogicalChannel (*channel);
  }

  /* Do not optimize, easier for translators */
  if (is_encoding)
    if (!is_closing)
      msg = g_strdup_printf (_("Opened codec %s for transmission"),
			     (const char *) codec_name);
    else
      msg = g_strdup_printf (_("Closed codec %s which was opened for transmission"),
			     (const char *) codec_name);
  else
    if (!is_closing)
      msg = g_strdup_printf (_("Opened codec %s for reception"),
			     (const char *) codec_name);
    else
      msg = g_strdup_printf (_("Closed codec %s which was opened for reception"),
			     (const char *) codec_name);

  /* Update the GUI and menus wrt opened channels */
  gnomemeeting_threads_enter ();
  gnomemeeting_log_insert (msg);
  gnomemeeting_menu_update_sensitivity (is_video, is_video?is_receiving_video:is_receiving_audio, is_video?is_transmitting_video:is_transmitting_audio);
  gnomemeeting_main_window_update_sensitivity (is_video, is_video?is_receiving_video:is_receiving_audio, is_video?is_transmitting_video:is_transmitting_audio);
  if (!is_receiving_video && !is_transmitting_video && !preview)
    gnomemeeting_init_main_window_logo (gw->main_video_image);
  gnomemeeting_threads_leave ();
  
  g_free (msg);
    
  return TRUE;
}

BOOL
GMH323Connection::OpenLogicalChannel (const H323Capability &capability,
				      unsigned session_id,  
				      H323Channel::Directions dir)
{
  GmWindow *gw = NULL;
  BOOL success = FALSE;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  success =
    H323Connection::OpenLogicalChannel (capability, session_id, dir);

  /* Translators, the full message is "Failure opening XXX for transmission,
     will ..." or "Failure opening XXX for reception, will ..." where
     XXX is for example GSM-06.10 */
  if (!success) {
    
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert ((dir == H323Channel::IsTransmitter)
				? _("Failure opening %s for transmission, will try with next common codec")
				: _("Failure opening %s for reception, will try with next common codec"),
				(const char *) capability.GetFormatName ());
    gnomemeeting_threads_leave ();
  }
  
  return success;
}


H323Connection::AnswerCallResponse
GMH323Connection::OnAnswerCall (const PString & caller,
				const H323SignalPDU &,
				H323SignalPDU &)
{
  IncomingCallMode icm;
  GMH323EndPoint *ep = NULL;

  icm = AVAILABLE;
  ep = GnomeMeeting::Process ()->Endpoint ();

  ep->SetCurrentCallToken (GetCallToken());

  gnomemeeting_threads_enter ();  
  icm = (IncomingCallMode)
    gconf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  gnomemeeting_threads_leave ();

  if (icm == FREE_FOR_CHAT) 
    return AnswerCallNow; 
  
  return AnswerCallPending;
}


void GMH323Connection::OnUserInputString(const PString & value)
{
  PString val;
  PString remote = GetRemotePartyName ();
  PINDEX bracket;

  /* The remote party name has to be converted to UTF-8, but not
     the text */
  gchar *utf8_remote = NULL;

  /* The MCU sends MSG[remote] value as message, 
     check if we are not using the MCU */
  bracket = value.Find("[");

  if ((bracket != P_MAX_INDEX) && (bracket == 3)) {
    
    remote = value.Mid (bracket + 1, value.Find ("] ") - 4);
    bracket = value.Find ("] ");
    val = value.Mid (bracket + 1);
  }
  else {

    if (value.Find ("MSG") != P_MAX_INDEX)
      val = value.Mid (3);
    else
      return;
  }

  /* If the remote name can be converted, use the conversion,
     else (Netmeeting), suppose it is ISO-8859-1 */  
  remote = gnomemeeting_pstring_cut (remote);
  if (g_utf8_validate ((gchar *) (const unsigned char*) remote, -1, NULL))
    utf8_remote = g_strdup ((char *) (const unsigned char *) (remote));
  else
    utf8_remote = gnomemeeting_from_iso88591_to_utf8 (remote);

  gnomemeeting_threads_enter ();
  if (utf8_remote && strcmp (utf8_remote, "")) 
    gnomemeeting_text_chat_insert (utf8_remote, val, 1);
  
  if (!GTK_WIDGET_VISIBLE (gw->chat_window))
    gconf_set_bool (USER_INTERFACE_KEY "main_window/show_chat_window", true);

  g_free (utf8_remote);
  gnomemeeting_threads_leave ();
}


BOOL GMH323Connection::OnReceivedFacility(const H323SignalPDU & pdu)
{
  if (pdu.m_h323_uu_pdu.m_h323_message_body.GetTag() == H225_H323_UU_PDU_h323_message_body::e_empty)
    return TRUE;

  if (pdu.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_facility)
    return FALSE;
  const H225_Facility_UUIE & fac = pdu.m_h323_uu_pdu.m_h323_message_body;

  // Check that it has the H.245 channel connection info
  if (fac.m_reason.GetTag() == H225_FacilityReason::e_startH245 &&
		  fac.HasOptionalField(H225_Facility_UUIE::e_h245Address) &&
			fac.m_protocolIdentifier.GetValue().IsEmpty()) {
    if (controlChannel != NULL && !controlChannel->IsOpen()) {
        PTRACE(2, "H225\tSimultaneous start of H.245 channel, connecting to remote.");
        controlChannel->CleanUpOnTermination();
        delete controlChannel;
        controlChannel = NULL;
    }
  }
  return H323Connection::OnReceivedFacility(pdu);
}


void 
GMH323Connection::HandleCallTransferFailure (const int returnError)
{
  GmWindow *gw = NULL;
  
  gnomemeeting_threads_enter ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Call transfer failed"), _("The remote user tried to transfer your call to another user, but it failed."));
  gnomemeeting_log_insert (_("Call transfer failed"));
  gnomemeeting_threads_leave ();
}


BOOL
GMH323Connection::OnRequestModeChange (const H245_RequestMode & pdu,
				       H245_RequestModeAck & ack,
				       H245_RequestModeReject & reject,
				       PINDEX & mode)
{
  H323Channel *channel = NULL;

  channel =
    this->FindChannel (RTP_Session::DefaultAudioSessionID,
					  FALSE);

  channel->GetCodec ()->GetRawDataChannel ()->Close ();


  return  H323Connection::OnRequestModeChange (pdu, ack, reject, mode);
}
  
