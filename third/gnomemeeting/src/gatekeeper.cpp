
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
 *                         gatekeeper.cpp  -  description
 *                         ------------------------------
 *   begin                : Wed Sep 19 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to register to gatekeepers
 *                          given the options in gconf.
 *
 */


#include "../config.h" 

#include "gatekeeper.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "tools.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"

#include <h323pdu.h>


/* Declarations */
extern GtkWidget *gm;

/* The class */
GMH323Gatekeeper::GMH323Gatekeeper ()
  :PThread (1000, NoAutoDeleteThread)
{
  gchar *gconf_string = NULL;
    
  /* Query the gconf database for options */
  gnomemeeting_threads_enter ();
  registering_method =
    gconf_get_int (H323_GATEKEEPER_KEY "registering_method");

  /* Gatekeeper password */
  gconf_string = gconf_get_string (H323_GATEKEEPER_KEY "password");
  if (gconf_string) {
    
    gk_password = PString (gconf_string);
    g_free (gconf_string);
  }

  /* Gatekeeper host */
  if (registering_method == 1) {
    
    gconf_string = gconf_get_string (H323_GATEKEEPER_KEY "host");
    if (gconf_string) {
      
      gk_host = PString (gconf_string);
      g_free (gconf_string);
    }
  }

  /* Gatekeeper ID */
  if (registering_method == 2) {
    
    gconf_string = gconf_get_string (H323_GATEKEEPER_KEY "id");
    if (gconf_string) {
      
      gk_id = PString (gconf_string);
      g_free (gconf_string);
    }
  }
  gnomemeeting_threads_leave ();
  
  this->Resume ();
}


GMH323Gatekeeper::~GMH323Gatekeeper ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);
}


void GMH323Gatekeeper::Main ()
{
  PString gk_name;
  gchar *msg = NULL;

  GMH323EndPoint *endpoint = NULL;
  H323Gatekeeper *gatekeeper = NULL;
  
  BOOL no_error = TRUE;
  GmWindow *gw = NULL;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  PWaitAndSignal m(quit_mutex);
  
  /* Remove the current Gatekeeper */
  gnomemeeting_threads_enter ();
  gatekeeper = endpoint->GetGatekeeper ();
  if (gatekeeper) {

    gk_name = gatekeeper->GetName ();
    msg = g_strdup_printf (_("Unregistered from gatekeeper %s"),
			   (const char *) gk_name);
    gnomemeeting_log_insert (msg);
    gnomemeeting_statusbar_flash (gw->statusbar, msg);
    g_free (msg);
  }
  gnomemeeting_threads_leave ();
  endpoint->RemoveGatekeeper (0);  
  endpoint->SetUserNameAndAlias (); 

  
  /* Check if we have all the needed information, if so we register */
  if (registering_method == 1 && gk_host.IsEmpty ()) {
  
    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid gatekeeper hostname"), _("Please provide a hostname to use for the gatekeeper."));
    gnomemeeting_threads_leave ();

    no_error = FALSE;
  }
  else if (registering_method == 2 && gk_id.IsEmpty ()) {

    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid gatekeeper ID"), _("Please provide a valid ID for the gatekeeper."));
    gnomemeeting_threads_leave ();

    no_error = FALSE;
  }
  else {

    /* Set the gatekeeper password */
    endpoint->SetGatekeeperPassword ("");
    if (!gk_password.IsEmpty ())
      endpoint->SetGatekeeperPassword (gk_password);

    
    /* Registers to the gk */
    if (registering_method == 3) {
    
      if (!endpoint->UseGatekeeper ())
	no_error = FALSE;
    }
    else if (registering_method == 1) {
      
      if (!endpoint->UseGatekeeper (gk_host, PString ()))
	no_error = FALSE;
    }
    else if (registering_method == 2) {
      
      if (!endpoint->UseGatekeeper (PString (), gk_id))
	no_error = FALSE;
    }
  }

  
  /* There was an error (missing parameter or registration failed)
     or the user chose to not register */
  if (!no_error || registering_method == 0) {
      
    /* Registering failed */
    gnomemeeting_threads_enter ();

    if (!no_error) {

      gatekeeper = endpoint->GetGatekeeper ();
      if (gatekeeper) {

	switch (gatekeeper->GetRegistrationFailReason()) {

	case H323Gatekeeper::DuplicateAlias :
	  msg = g_strdup (_("Another user already exists with the same alias, please use another alias."));
	  break;
	case H323Gatekeeper::SecurityDenied :
	  msg = g_strdup (_("You are not allowed to register to the gatekeeper. Please check your login, password and firewall."));
	  break;
	case H323Gatekeeper::TransportError :
	  msg = g_strdup (_("There was a transport error."));
	  break;
	default :
	  msg = g_strdup (_("No gatekeeper corresponding to your options has been found."));
	  break;
	}
      }
      else
	msg = g_strdup (_("No gatekeeper corresponding to your options has been found."));

      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Error while registering with gatekeeper"), msg);
      gnomemeeting_log_insert (msg);
      g_free (msg);
    }
    
    gconf_set_int (H323_GATEKEEPER_KEY "registering_method", 0);
    gnomemeeting_threads_leave ();
  }
  /* Registering is ok */
  else {

    gatekeeper = endpoint->GetGatekeeper ();
    if (gatekeeper)
      gk_name = gatekeeper->GetName ();
    msg =
      g_strdup_printf (_("Gatekeeper set to %s"), (const char *) gk_name);
    
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (msg);
    gnomemeeting_statusbar_flash (gw->statusbar, msg);
    gnomemeeting_threads_leave ();
      
    g_free (msg);
  } 
}


/* This class implements the Citron NAT Technology
   Any question or comment, redirect to Chih-Wei Huang <cwhuang@citron.com.tw>
   This permits to GnomeMeeting clients to register to public GNU GK while
   being behind a non-configured NAT gateway and to still work.
*/
static bool
SendTPKT (PTCPSocket *sender,
	  const PBYTEArray &buf)
{
  WORD len = buf.GetSize(), tlen = len + 4;
  PBYTEArray tbuf(tlen);
  BYTE *bptr = tbuf.GetPointer();
  bptr[0] = 3, bptr[1] = 0;

  *(reinterpret_cast<WORD *>(bptr + 2)) = PIPSocket::Host2Net(WORD(len + 4));
  memcpy(bptr + 4, buf, len);

  return sender->Write(bptr, tlen);
}


static bool
ForwardMesg (PTCPSocket *receiver,
	     PTCPSocket *sender)
{
  BYTE tpkt[4];
  if (!receiver->ReadBlock(tpkt, sizeof(tpkt)))
    return false;
  if (tpkt[0] != 3) {
    PTRACE(2, "NAT\tNot a TPKT packet? " << tpkt[0]);
    return true; // continue
  }

  int bSize = ((tpkt[2] << 8)|tpkt[3]) - 4;
  PBYTEArray buffer (bSize);
  
  if (!receiver->ReadBlock(buffer.GetPointer(), bSize)) {
    PTRACE(3, "NAT\tRead Error? size=" << bSize);

    return false;
  }
  
  return SendTPKT(sender, buffer);
}


static void
DoForwarding (PTCPSocket *incomingTCP,
	      PTCPSocket *outgoingTCP)
{
  while (incomingTCP->IsOpen() && outgoingTCP->IsOpen()) {
    
    int Selection = PSocket::Select(*incomingTCP, *outgoingTCP);

    switch (Selection)
      {
      case -3 :   // Data available on both
	if (!ForwardMesg(outgoingTCP, incomingTCP))
	  return;
      case -1 :   // Data available on incomingTCP
	if (!ForwardMesg(incomingTCP, outgoingTCP))
	  return;
	break;
      case -2 :   // Data available on outgoingTCP
	if (!ForwardMesg(outgoingTCP, incomingTCP))
	  return;
	break;
      case 0  :   // Timeout
	PTRACE(1, "NAT\tTimeout " << Selection << " on select");
	break;
      default :
	PTRACE(3, "NAT\tError " << Selection << " on select");
	break;
      }
  }
}


H323GatekeeperWithNAT::H323GatekeeperWithNAT (H323EndPoint &ep, H323Transport *trans) : H323Gatekeeper(ep, trans)
{
  isMakeRequestCalled = false;
  detectorThread = 0;
  incomingTCP = outgoingTCP = 0;
  gkport = 0;
}


H323GatekeeperWithNAT::~H323GatekeeperWithNAT()
{
  StopDetecting();
}


BOOL H323GatekeeperWithNAT::MakeRequest (Request &request)
{
  if (request.requestPDU.GetAuthenticators().IsEmpty())
    request.requestPDU.SetAuthenticators(authenticators);

  if (isMakeRequestCalled) {
    
    isMakeRequestCalled = false;
    PTRACE(2, "GK\tRecursively call detected");

    return H225_RAS::MakeRequest(request);
  }

  PWaitAndSignal lock(requestMutex);
  while (!H225_RAS::MakeRequest(request)) {
    
    if (request.responseResult != Request::NoResponseReceived && request.responseResult != Request::TryAlternate)
      return FALSE;

    PINDEX alt, altsize = alternates.GetSize();
    if (altsize == 0)
      return FALSE;

    H323TransportAddress tempAddr = transport->GetRemoteAddress();
    PString tempIdentifier = gatekeeperIdentifier;

    PTRACE(2, "GK\tTry alt GK");
    StopDetecting();
    PSortedList<AlternateInfo> altGKs = alternates;

    for (alt = 0; alt < altsize; ++alt) {
      
      AlternateInfo *altInfo = &altGKs[alt];
      transport->SetRemoteAddress(altInfo->rasAddress);
      transport->Connect();
      PTRACE(2, "GK\tTry " << altInfo->rasAddress);

      gatekeeperIdentifier = altInfo->gatekeeperIdentifier;

      H323RasPDU pdu;
      Request req(SetupGatekeeperRequest(pdu), pdu);

      if (H225_RAS::MakeRequest(req)) {

	endpointIdentifier = "";
	isMakeRequestCalled = true;

	if (!RegistrationRequest(autoReregister))
	  continue;

	PTRACE(1, "GK\tSwitch to altGK " << gatekeeperIdentifier << " with id " << endpointIdentifier << " successful");
	if (request.requestPDU.GetChoice ().GetTag() == H323RasPDU::e_admissionRequest) {

          H225_AdmissionRequest & arq = (H225_AdmissionRequest &) request.requestPDU.GetChoice ();

	  arq.m_gatekeeperIdentifier = gatekeeperIdentifier;
	  arq.m_endpointIdentifier = endpointIdentifier;
	} else if (request.requestPDU.GetChoice ().GetTag() == H323RasPDU::e_registrationRequest)
	  return TRUE;
	break;
      }
    }
    
    if (alt >= altsize) {

      // switch failed, use primary GK
      transport->SetRemoteAddress(tempAddr);
      transport->Connect();
      gatekeeperIdentifier = tempIdentifier;
      PTRACE(2, "GK\tFall back to " << tempAddr);
      return FALSE;
    }
  }
  
  return TRUE;
}


BOOL H323GatekeeperWithNAT::OnReceiveRegistrationConfirm (const H225_RegistrationConfirm & rcf)
{
  if (rcf.HasOptionalField(H225_RegistrationConfirm::e_nonStandardData))
    if (rcf.m_nonStandardData.m_data.AsString().Find("NAT=") == 0) {
      
      PWaitAndSignal lock(threadMutex);
      // OK, I'm behind an NAT
      
      if (!detectorThread) {
	if (rcf.m_callSignalAddress.GetSize() > 0) {
          const H225_TransportAddress & addr = rcf.m_callSignalAddress[0];
	  if (addr.GetTag() == H225_TransportAddress::e_ipAddress) {
            const H225_TransportAddress_ipAddress & ip = addr;
            gkip = PIPSocket::Address(ip.m_ip[0], ip.m_ip[1], ip.m_ip[2], ip.m_ip[3]);
            gkport = ip.m_port;
          }
        }
        detectorThread = new DetectIncomingCallThread(this);
      }
    }

  return H323Gatekeeper::OnReceiveRegistrationConfirm(rcf);
}


BOOL H323GatekeeperWithNAT::OnReceiveUnregistrationRequest(const H225_UnregistrationRequest &urq)
{
  StopDetecting();
  return H323Gatekeeper::OnReceiveUnregistrationRequest(urq);
}


void H323GatekeeperWithNAT::OnSendRegistrationRequest(H225_RegistrationRequest &rrq)
{
  H323Gatekeeper::OnSendRegistrationRequest(rrq);
}


void H323GatekeeperWithNAT::OnSendUnregistrationRequest(H225_UnregistrationRequest &urq)
{
  StopDetecting();
  H323Gatekeeper::OnSendUnregistrationRequest(urq);
}


void H323GatekeeperWithNAT::DetectIncomingCall()
{
  const H323ListenerList & listeners = endpoint.GetListeners();

  if (listeners.IsEmpty()) {

    PTRACE(1, "NAT\tNo listener!");
    return;
  }
  H323TransportAddress localAddr = listeners[0].GetTransportAddress();

  isDetecting = true;
  while (isDetecting) {

    if (!gkport) {
      H323TransportAddress gksig;

      if (!LocationRequest(endpoint.GetAliasNames(), gksig)) {
	PTRACE(1, "NAT\tUnable to get GK signalling address");
	PProcess::Current().Sleep(60 * 1000); // TODO...
	continue;
      }
      gksig.GetIpAndPort(gkip, gkport);
    }
    socketMutex.Wait();
    incomingTCP = new PTCPSocket(gkport);
    socketMutex.Signal();

    if (!incomingTCP->Connect(gkip)) {

      socketMutex.Wait();
      delete incomingTCP;
      incomingTCP = 0;
      socketMutex.Signal();
      PTRACE(1, "NAT\tUnable to connect to GK");
      PProcess::Current().Sleep(60 * 1000); // TODO...
      continue;
    }

    PTRACE(2, "NAT\tConnect to GK, wait for incoming call");
    while (incomingTCP->IsOpen()) {

      SendInfo(Q931::CallState_IncomingCallProceeding);
      PSocket::SelectList rlist;
      rlist.Append(incomingTCP);
      if ((PSocket::Select(rlist, 86400 * 1000) == PSocket::NoError) && !rlist.IsEmpty()) {

	// incoming call detected
	PIPSocket::Address ip;
	WORD port;
	localAddr.GetIpAndPort(ip, port);
	socketMutex.Wait();
	outgoingTCP = new PTCPSocket(port);
	socketMutex.Signal();

	if (!outgoingTCP->Connect(ip)) {
	  PTRACE(1, "NAT\tUnable to connect to listener");
	  break;
	}

	DoForwarding(incomingTCP, outgoingTCP);
	break;
      }
    }
    
    PWaitAndSignal socketlock(socketMutex);
    delete incomingTCP;
    incomingTCP = 0;
    delete outgoingTCP;
    outgoingTCP = 0;
  }
}


void H323GatekeeperWithNAT::StopDetecting()
{
  PWaitAndSignal lock(threadMutex);
  if (detectorThread) {
    isDetecting = false;
    socketMutex.Wait();
    if (incomingTCP) {
      if (outgoingTCP)
        outgoingTCP->Close();
      else // no incoming call
	SendInfo(Q931::CallState_DisconnectRequest);
      incomingTCP->Close();
    }

    socketMutex.Signal();
    detectorThread->Terminate();
    detectorThread->WaitForTermination();
    delete detectorThread;
    detectorThread = 0;
  }
}


bool H323GatekeeperWithNAT::SendInfo(int state)
{
  Q931 information;
  information.BuildInformation(0, false);
  PBYTEArray buf, epid(endpointIdentifier, endpointIdentifier.GetLength(), false);
  information.SetIE(Q931::FacilityIE, epid);
  information.SetCallState(Q931::CallStates(state));
  information.Encode(buf);
  return SendTPKT(incomingTCP, buf);
}


