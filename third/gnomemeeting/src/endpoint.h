
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_


#include "../config.h"

#include "common.h"

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif


#include "gdkvideoio.h"
#include "videograbber.h"

class GMILSClient;
class GMLid;
class GMH323Gatekeeper;


/**
 * COMMON NOTICE: The Endpoint must be initialized with Init after its
 * creation.
 */

class GMH323EndPoint : public H323EndPoint
{
  PCLASSINFO(GMH323EndPoint, H323EndPoint);

  
 public:

  enum CallingState {Standby, Calling, Connected, Called};

  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the local endpoint and initialises the variables
   * PRE          :  /
   */
  GMH323EndPoint ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323EndPoint ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Makes a call to the given address, and fills in the
   *                 call taken. It returns a locked pointer to the connection
   *                 in case of success.
   * PRE          :  The called address, its call token.
   */
  H323Connection *MakeCallLocked (const PString &, PString &);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the internal audio and video devices for playing
   *                 and recording following the GConf database content.
   *                 If a Quicknet card is used, it will be opened, and if
   *                 a video grabber is used in preview mode, it will also
   *"                be opened.
   * PRE          :  /
   */
  void UpdateDevices ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Creates a video grabber.
   * PRE          :  If TRUE, then the grabber will start
   *                 grabbing after its creation. If TRUE,
   *                 then the opening is done sync.
   */  
  GMVideoGrabber *CreateVideoGrabber (BOOL = TRUE, BOOL = FALSE);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current video grabber, if any.
   * PRE          :  /
   */  
  void RemoveVideoGrabber ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current videograbber, if any.
   *                 The pointer is locked, which means that the device
   *                 can't be deleted until the pointer is unlocked with
   *                 Unlock. This is a protection to always manipulate
   *                 existing objects. Notice that all methods in the
   *                 GMH323EndPoint class related to the GMVideoGrabber
   *                 use internal mutexes so that the pointer cannot be
   *                 returned during a RemoveVideoGrabber/CreateVideoGrabber,
   *                 among others. You should use those functions and not
   *                 manually delete the GMVideoGrabber.
   * PRE          :  /
   */
  GMVideoGrabber *GetVideoGrabber ();

  
  /* DESCRIPTION  :  This callback is called to create an instance of
   *                 H323Gatekeeper for gatekeeper registration.
   * BEHAVIOR     :  Return an instance of H323GatekeeperWithNAT
   *                 that implements the Citron NAT Technology.
   * PRE          :  /
   */
  virtual H323Gatekeeper * CreateGatekeeper (H323Transport *);
     
  
  /* DESCRIPTION  :  This callback is called if we create a connection
   *                 or if somebody calls and we accept the call.
   * BEHAVIOR     :  Create a connection using the call reference
   *                 given as parameter which is given by OpenH323.
   * PRE          :  /
   */
  virtual H323Connection *CreateConnection (unsigned);
  

  /* DESCRIPTION  :  This callback is called on an incoming call.
   * BEHAVIOR     :  If a call is already running, returns FALSE
   *                 -> the incoming call is not accepted, else
   *                 returns TRUE which was the default behavior
   *                 if we had not defined it.
   *
   * PRE          :  /
   */
  virtual BOOL OnIncomingCall (H323Connection &, const H323SignalPDU &,
			       H323SignalPDU &);


  /* DESCRIPTION  :  This callback is called when a call is forwarded.
   * BEHAVIOR     :  Outputs a message in the log and statusbar.
   * PRE          :  /
   */
  virtual BOOL OnConnectionForwarded (H323Connection &,
				      const PString &,
				      const H323SignalPDU &);

  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   *                 It means that a connection to a remote endpoint is ok,
   *                 with one control channel and x >= 0 logical channel(s)
   *                 opened
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters (and updates the GUI)
   * PRE          :  /
   */
  virtual void OnConnectionEstablished (H323Connection &,
				        const PString &);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the remote party name (UTF-8), the
   *                 remote application name (UTF-8), and the best
   *                 guess for the URL to use when calling back the user
   *                 (the IP, or the alias or e164 number if the local
   *                 user is registered to a gatekeeper. Not always accurate,
   *                 for example if you are called by an user with an alias,
   *                 but not registered to the same GK as you.)
   * PRE          :  /
   */
  void GetRemoteConnectionInfo (H323Connection &,
				gchar * &,
				gchar * &,
				gchar * &);

  
  /* DESCRIPTION  :  This callback is called when the connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters and updates the GUI.
   * PRE          :  /
   */
  virtual void OnConnectionCleared (H323Connection &,
				    const PString &);


  /* DESCRIPTION  :  This callback is called when a video device 
   *                 has to be opened.
   * BEHAVIOR     :  Create a GDKVideoOutputDevice for the local and remote
   *                 image display.
   * PRE          :  /
   */
  virtual BOOL OpenVideoChannel (H323Connection &,
				 BOOL, H323VideoCodec &);

  
  /* DESCRIPTION  :  This callback is called when an audio channel has to
   *                 be opened.
   * BEHAVIOR     :  Opens the Audio Channel or warns the user if it was
   *                 impossible.
   * PRE          :  /
   */
  virtual BOOL OpenAudioChannel (H323Connection &, BOOL,
				 unsigned, H323AudioCodec &);

			       
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options.
   *                 returns TRUE if success and FALSE in case of error.
   */
  BOOL StartListener ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the capability corresponding to the PString and
   *                 return the remaining capabilities list.
   * PRE          :  /
   */
  H323Capabilities RemoveCapability (PString);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove all capabilities of the endpoint.
   * PRE          :  /
   */
  void RemoveAllCapabilities (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add all possible capabilities for the endpoint.
   * PRE          :  /
   */
  void AddAllCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add audio capabilities following the user config.
   * PRE          :  /
   */
  void AddAudioCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add video capabilities.
   * PRE          :  /
   */
  void AddVideoCapabilities ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Adds the User Input capabilities following the
   *                 configuration options. Can set: None, All, rfc2833,
   *                 String, Signal.
   * PRE          :  /
   */
  void AddUserInputCapabilities ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Saves the currently displayed picture in a file.
   * PRE          :  / 
   */
  void SavePicture (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current calling state :
   *                   { Standby,
   *                     Calling,
   *                     Connected,
   *                     Called }
   * PRE          :  /
   */
  void SetCallingState (CallingState);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current calling state :
   *                   { Standby,
   *                     Calling,
   *                     Connected,
   *                     Called }
   * PRE          :  /
   */
  CallingState GetCallingState (void);


  /* Overrides from H323Endpoint */
  H323Connection *SetupTransfer (const PString &,
				 const PString &,
				 const PString &,
				 PString &,
				 void * = NULL);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current IP of the endpoint, 
   *                 even if the endpoint is listening on many interfaces
   * PRE          :  /
   */
  PString GetCurrentIP (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Translate to packets to the IP of the gateway.
   * PRE          :  /
   */
  virtual void TranslateTCPAddress(PIPSocket::Address &,
				   const PIPSocket::Address &);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts an audio tester that will play any recorded
   *                 sound to the speakers in real time. Can be used to
   *                 check if the audio volumes are correct before 
   *                 a conference.
   * PRE          :  /
   */
  void StopAudioTester ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stops the current audio tester if any for the given
   *                 audio manager, player and recorder.
   * PRE          :  /
   */
  void StartAudioTester (gchar *,
			 gchar *,
			 gchar *);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current call token.
   * PRE          :  A valid PString for a call token.
   */
  void SetCurrentCallToken (PString);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current call token (empty if
   *                 no call is in progress).
   * PRE          :  /
   */
  PString GetCurrentCallToken (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current gatekeeper.
   * PRE          :  /
   */
  H323Gatekeeper *GetGatekeeper (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to (or unregister from) the gatekeeper 
   *                 given in the options, if any.
   * PRE          :  /
   */
  void GatekeeperRegister (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to (or unregister from) the ILS server
   *                 given in the options, if any.
   * PRE          :  /
   */
  void ILSRegister ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the firstname and last 
   *                 name stored by gconf, set the gatekeeper alias.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();

    
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the RTP, TCP, UDP ports from the GConf database.
   * PRE          :  /
   */
  void SetPorts ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the audio device volume (playing then recording). 
   * PRE          :  /
   */
  BOOL SetDeviceVolume (PSoundChannel *, BOOL, unsigned int);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the audio device volume (playing then recording). 
   * PRE          :  /
   */
  BOOL GetDeviceVolume (PSoundChannel *, BOOL, unsigned int &);
  


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be transmitted
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartTransmitVideo (BOOL a) {autoStartTransmitVideo = a;}


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be received
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartReceiveVideo (BOOL a) {autoStartReceiveVideo = a;}


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Automatically starts transmitting (BOOL = FALSE) or
   *                 receiving (BOOL = TRUE) with the given
   *                 capability, for the given session RTP id and returns
   *                 TRUE if it was successful. Do that only if there is
   *                 a connection.
   * PRE          :  /
   */
  BOOL StartLogicalChannel (const PString &, unsigned int, BOOL);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Automatically stop transmitting (BOOL = FALSE) or
   *                 receiving (BOOL = TRUE) for the given RTP session id
   *                 and returns TRUE if it was successful.
   *                 Do that only if there is a connection.
   * PRE          :  /
   */
  BOOL StopLogicalChannel (unsigned int, BOOL);


#ifdef HAS_IXJ
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current Lid Thread pointer, locked so that
   *                 the object content is protected against deletion. See
   *                 GetVideoGrabber ().
   * PRE          :  /
   */
  GMLid *GetLid ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current Lid.
   * PRE          :  /
   */
  void RemoveLid ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Create a new Lid.
   * PRE          :  Non-empty device name.
   */
  GMLid *CreateLid (PString);
#endif


 protected:

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the last called call token.
   * PRE          :  /
   */
  PString GetLastCallAddress ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current transfer call token.
   * PRE          :  A valid PString for a call token.
   */
  void SetTransferCallToken (PString);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current transfer call token (empty if
   *                 no call is in progress).
   * PRE          :  /
   */
  PString GetTransferCallToken (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns when the primary call of a transfer is terminated.
   * PRE          :  /
   */
  void TransferCallWait ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set (BOOL = TRUE) or get (BOOL = FALSE) the
   *                 audio playing and recording volumes for the
   *                 audio device.
   * PRE          :  /
   */
  BOOL DeviceVolume (PSoundChannel *, BOOL, BOOL, unsigned int &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Check if the listening port is accessible from the
   *                 outside and returns the test result given by seconix.com.
   * PRE          :  /
   */
  PString CheckTCPPorts ();

  
  /* DESCRIPTION  :  Notifier called after 30 seconds of no media.
   * BEHAVIOR     :  Clear the calls.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnNoIncomingMediaTimeout);

  
  /* DESCRIPTION  :  Notifier called periodically to update details on ILS.
   * BEHAVIOR     :  Register, unregister the user from ILS or udpate his
   *                 personal data using the GMILSClient (XDAP).
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnILSTimeout);


  /* DESCRIPTION  :  Notifier called periodically during calls.
   * BEHAVIOR     :  Refresh the statistics window of the Control Panel
   *                 if it is currently shown.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnRTPTimeout);


  /* DESCRIPTION  :  Notifier called periodically to update the gateway IP.
   * BEHAVIOR     :  Update the gateway IP to use for the IP translation
   *                 if IP Checking is enabled in the GConf database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnGatewayIPTimeout);


  /* DESCRIPTION  :  Notifier called when an incoming call
   *                 has not been answered after 15 seconds.
   * BEHAVIOR     :  Reject the call, or forward if forward on no answer is
   *                 enabled in the GConf database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnNoAnswerTimeout);


  /* DESCRIPTION  :  Notifier called every second while waiting for an answer
   *                 for an incoming call.
   * BEHAVIOR     :  Display an animation in the docklet and play a ring
   *                 sound.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnCallPending);


  /* DESCRIPTION  :  Notifier called every 2 seconds while waiting for an
   *                 answer for an outging call.
   * BEHAVIOR     :  Display an animation in the main winow and play a ring
   *                 sound.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnOutgoingCall);

  
  PString called_address;
  PString current_call_token;
  PString transfer_call_token;

  H323Connection *current_connection;  
  H323ListenerTCP *listener;  

  CallingState calling_state; 

  PTimer NoIncomingMediaTimer;
  PTimer ILSTimer;
  PTimer RTPTimer;
  PTimer GatewayIPTimer;
  PTimer NoAnswerTimer;
  PTimer CallPendingTimer;
  PTimer OutgoingCallTimer;
    
  BOOL ils_registered;

  GmWindow *gw; 
  GmLdapWindow *lw;
  GmTextChat *chat;


  /* The encoding video grabber */
  GMVideoGrabber *video_grabber;

  GMH323Gatekeeper *gk;
  
  GMILSClient *ils_client;
  PThread *audio_tester;


  /* Various mutexes to ensure thread safeness around internal
     variables */
  PMutex vg_access_mutex;
  PMutex ils_access_mutex;
  PMutex cs_access_mutex;
  PMutex ct_access_mutex;
  PMutex tct_access_mutex;
  PMutex lid_access_mutex;
  PMutex at_access_mutex;
  PMutex lca_access_mutex;

  PMutex sound_event_mutex;
  
  PMutex audio_channel_mutex;
  PMutex video_channel_mutex;

#ifdef HAS_IXJ
  GMLid *lid;
#endif
};

#endif
