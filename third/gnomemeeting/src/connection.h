
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
 *                         connection.h  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains connection related functions.
 *
 */


#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "common.h"
#include "endpoint.h"


/* GMH323Connection */

class GMH323Connection : public H323Connection
{
  PCLASSINFO(GMH323Connection, H323Connection);

  
  public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Setups the connection parameters.
   * PRE          :  /
   */
  GMH323Connection (GMH323EndPoint &, unsigned);


  /* DESCRIPTION  :  Callback called when OpenH323 opens a new logical channel
   * BEHAVIOR     :  Updates the log window with information about it, returns
   *                 FALSE if error, TRUE if OK
   * PRE          :  /
   */
  virtual BOOL OnStartLogicalChannel (H323Channel &);


  /* DESCRIPTION  :  Callback called when OpenH323 closes a new logical channel
   * BEHAVIOR     :  Close the channel.
   * PRE          :  /
   */
  virtual void OnClosedLogicalChannel (const H323Channel &);


  BOOL OpenLogicalChannel(
      const H323Capability & capability,  /// Capability to open channel with
      unsigned sessionID,                 /// Session for the channel
      H323Channel::Directions dir         /// Direction of channel
    );

  
  /* DESCRIPTION  :  This callback is called to give the opportunity
   *                 to take an action on an incoming call
   * BEHAVIOR     :  Behavior is the following :
   *                 - returns AnswerCallDenied if user is in DND mode
   *                   connection aborted and a Release Complete PDU is sent
   *                 - returns AnswerCallNow if user is in Auto Answer mode
   *                   H323 protocol proceeds and the call is answered
   *                 - return AnswerCallPending (default)
   *                   pause until AnsweringCall is called (if the user
   *                   clicks on connect or disconnect)
   * PRE          :  /
   */
  virtual H323Connection::AnswerCallResponse 
    OnAnswerCall (const PString &, const H323SignalPDU &, H323SignalPDU &);


  /* DESCRIPTION  :  This is called when we received User Input String.
   * BEHAVIOR     :  Displays the string in the chat window.
   * PRE          :  /
   */
  virtual void OnUserInputString(const PString &);


  /* DESCRIPTION  :  This is called when the current call is transferred but
   *                 that it fails.
   * BEHAVIOR     :  Displays an error message.
   * PRE          :  /
   */
  virtual void HandleCallTransferFailure(const int);

  
  /* DESCRIPTION  :  This is called when we received Q.931 Facility
   * BEHAVIOR     :  Detect if it's an H.245 reverting message
   * PRE          :  /
   */
  virtual BOOL OnReceivedFacility(const H323SignalPDU &);

  virtual BOOL OnRequestModeChange (const H245_RequestMode &,
				    H245_RequestModeAck &,
				    H245_RequestModeReject &,
				    PINDEX &);
  
 protected:

  BOOL OnLogicalChannel (H323Channel *, BOOL);
  
  GmWindow *gw;
  
  int opened_video_channels;
  int opened_audio_channels;  

  BOOL is_transmitting_video;
  BOOL is_transmitting_audio;
  BOOL is_receiving_video;
  BOOL is_receiving_audio;  

  PMutex channels;
};


#endif
