
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
 *                         lid.h  -  description
 *                         ---------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains LID functions.
 *
 */


#ifndef _LID_H_
#define _LID_H_

#include "../config.h"

#include "common.h"
#include "endpoint.h"


#ifdef HAS_IXJ
#include <ixjlid.h>


class GMLid : public PThread, public OpalIxJDevice
{
  PCLASSINFO(GMLid, PThread);

 public:

  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  Creates the thread and opens the Line Interface Device.
   *                 The path for the LID is given as argument.
   * PRE          :  /
   */
  GMLid (PString);

  
  /* DESCRIPTION  :  Destructor.
   * BEHAVIOR     :  Closes the Line Interface Device.
   * PRE          :  /
   */
  ~GMLid ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the state of the LID following the endpoint
   *                 calling state. For example, called => ring, ...
   * PRE          :  /
   */
  void UpdateState (GMH323EndPoint::CallingState);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Plays a DTMF after having stopped any tone.
   * PRE          :  /
   */
  void PlayDTMF (const char *);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the playing or the recording volume.
   * PRE          :  BOOL = is_encoding or not, 0% < volume < 100%
   */
  void SetVolume (BOOL,
		  int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if software codecs are supported or FALSE
   *                 if the LID is closed or if only hardware codecs are
   *                 supported.
   * PRE          :  /
   */
  BOOL areSoftwareCodecsSupported ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Lock the LID, preventing it to be Opened or Closed.
   * PRE          :  /
   */
  void Lock ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Unlock the LID, permitting it to be Opened or Closed.
   * PRE          :  /
   */
  void Unlock ();
  
 private:

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Opens the Line Interface Device with the options
   *                 stored in the GConf database. Displays success messages 
   *                 in the log and failure messages in popups.
   * PRE          :  /
   */
  BOOL Open ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Closes the currently opened Line Interface Device.
   * PRE          :  /
   */
  BOOL Close ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Monitors the LID status, and take appropriate actions,
   *                 following it is off hook, on hook, and following DTMF
   *                 sequences.
   * PRE          :  /
   */
  void Main ();


  /* Internal variables */
  PMutex device_access_mutex;
  PMutex quit_mutex;
  PSyncPoint thread_sync_point;

  PString dev_name;
  BOOL stop;
  BOOL soft_codecs;
};
#endif

#endif
