
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
 *                         gatekeeper.h  -  description
 *                         ----------------------------
 *   begin                : Wed Sep 19 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to register to gatekeepers
 *                          given the options in gconf.
 *
 */


#ifndef _GATEKEEPER_H_
#define _GATEKEEPER_H_

#include "common.h"


class GMH323Gatekeeper : public PThread
{
  PCLASSINFO(GMH323Gatekeeper, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  GMH323Gatekeeper ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323Gatekeeper ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Register to the gatekeeper using the method and the 
   *                 parameters in gconf. This is done in a separate thread.
   *                 This class is auto-deleted on termination. A popup
   *                 is displayed if registration fails or if options are
   *                 missing.
   * PRE          :  /
   */
  void Main ();

protected:

  int registering_method;
  
  PString gk_host;
  PString gk_password;
  PString gk_id;

  PMutex quit_mutex;
};


/* This class implements the Citron NAT Technology
   Any question or comment, redirect to Chih-Wei Huang <cwhuang@citron.com.tw>
   This permits to GnomeMeeting clients to register to public GNU GK while
   being behind a non-configured NAT gateway and to still work.
*/
class H323GatekeeperWithNAT : public H323Gatekeeper
{
  PCLASSINFO(H323GatekeeperWithNAT, H323Gatekeeper);
    
 public:

  H323GatekeeperWithNAT(H323EndPoint & ep, H323Transport * trans);
  ~H323GatekeeperWithNAT();

  // overrides from H323Gatekeeper
  virtual BOOL OnReceiveRegistrationConfirm (const H225_RegistrationConfirm & rcf);
  virtual BOOL OnReceiveUnregistrationRequest (const H225_UnregistrationRequest &);
  virtual void OnSendRegistrationRequest (H225_RegistrationRequest &);
  virtual void OnSendUnregistrationRequest (H225_UnregistrationRequest &);

  virtual BOOL MakeRequest (Request &);

  virtual void DetectIncomingCall ();
 protected:

  virtual void StopDetecting ();

  bool SendInfo (int state);

  class DetectIncomingCallThread : public PThread
    {
      PCLASSINFO(DetectIncomingCallThread, PThread);
    public:
      DetectIncomingCallThread(H323GatekeeperWithNAT * gk)
	: PThread(1000, NoAutoDeleteThread), gatekeeper(gk) { Resume(); }
      void Main() { gatekeeper->DetectIncomingCall(); }

    private:
      H323GatekeeperWithNAT *gatekeeper;
    };

  bool isMakeRequestCalled;

  DetectIncomingCallThread *detectorThread;
  PTCPSocket *incomingTCP, *outgoingTCP;
  PMutex threadMutex, socketMutex;
  PIPSocket::Address gkip;
  WORD gkport;
  bool isDetecting;
};

#endif
