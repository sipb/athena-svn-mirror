
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
 *                          urlhandler.cpp  -  description
 *                          ------------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to call a given URL or to
 *                          answer a call.
 *
 */


#include "../config.h" 


#include "urlhandler.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "ldap_window.h"
#include "main_window.h"
#include "toolbar.h"
#include "menu.h"
#include "tools.h"

#include "dialog.h"
#include "gtk_menu_extensions.h"
#include "gconf_widgets_extensions.h"

/* Declarations */
extern GtkWidget *gm;

static gint
TransferTimeOut (gpointer data)
{
  PString transfer_call_token;
  PString call_token;

  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gdk_threads_enter ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  if (ep)
    call_token = ep->GetCurrentCallToken ();

  if (!call_token.IsEmpty ()) {

    gw = GnomeMeeting::Process ()->GetMainWindow ();
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Call transfer failed"), _("The call transfer failed, the user was either unreachable, or simply busy when he received the call transfer request."));
    gnomemeeting_log_insert (_("Call transfer failed"));
  }
  gdk_threads_leave ();

  
  return FALSE;
}



GMURL::GMURL ()
{
  is_supported = false;
}


GMURL::GMURL (PString c)
{
  c.Replace ("//", "");
  url = c.Trim ();
  
  if (url.Find ('#') == url.GetLength () - 1) {

    url.Replace ("callto:", "");
    url.Replace ("h323:", "");
    type = PString ("shortcut");
    is_supported = true;
  }
  else if (url.Find ("callto:") == 0) {

    url.Replace ("callto:", "");
    type = PString ("callto");
    is_supported = true;
  }
  else if (url.Find ("h323:") == 0) {

    url.Replace ("h323:", "");
    type = PString ("h323");
    is_supported = true;
  }
  else if (url.Find ("h323:") == P_MAX_INDEX
	   && url.Find ("callto:") == P_MAX_INDEX) {

    if (url.Find ("/") != P_MAX_INDEX) {
      
      type = PString ("callto");
      is_supported = true;
    }
    else {
      
      type = PString ("h323");
      is_supported = true;
    }
  }
  else
    is_supported = false;
}


GMURL::GMURL (const GMURL & u)
{
  is_supported = u.is_supported;
  type = u.type;
  url = u.url;
}


BOOL GMURL::IsEmpty ()
{
  return url.IsEmpty ();
}


BOOL GMURL::IsSupported ()
{
  return is_supported;
}


PString GMURL::GetType ()
{
  return type;
}


PString GMURL::GetValidURL ()
{
  PString valid_url;

  if (type == "shortcut")
    valid_url = url.Left (url.GetLength () -1);
  else if (type == "callto"
	   && url.Find ('/') != P_MAX_INDEX
	   && url.Find ("type") == P_MAX_INDEX) 
    valid_url = "callto:" + url + "+type=directory";
  else if (is_supported)
    valid_url = type + ":" + url;
    
  return valid_url;
}


PString GMURL::GetDefaultURL ()
{
  return PString ("h323:");
}


BOOL GMURL::operator == (GMURL u) 
{
  return (this->GetValidURL () *= u.GetValidURL ());
}


BOOL GMURL::operator != (GMURL u) 
{
  return !(this->GetValidURL () *= u.GetValidURL ());
}


/* The class */
GMURLHandler::GMURLHandler (PString c, BOOL transfer)
  :PThread (1000, AutoDeleteThread)
{
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  url = GMURL (c);

  answer_call = FALSE;
  transfer_call = transfer;
  
  this->Resume ();
}


GMURLHandler::GMURLHandler ()
  :PThread (1000, AutoDeleteThread)
{
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  answer_call = TRUE;
  
  this->Resume ();
}


GMURLHandler::~GMURLHandler ()
{
  PWaitAndSignal m(quit_mutex);
  /* Nothing to do here except waiting that LDAP at worse
     cancels the search */
}


void GMURLHandler::Main ()
{
  GmWindow *gw = NULL;

  BOOL use_gateway = FALSE;
  
  PString gateway;
  PString call_address;
  PString current_call_token;

  GMURL old_url;
  
  gchar *msg = NULL;
  
  GMH323EndPoint *endpoint = NULL;
  H323Connection *con = NULL;

  PWaitAndSignal m(quit_mutex);
  
  gnomemeeting_threads_enter ();
  use_gateway = gconf_get_bool (H323_GATEWAY_KEY "use_gateway");
  gateway = gconf_get_string (H323_GATEWAY_KEY "host");
  gnomemeeting_threads_leave ();

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  endpoint = GnomeMeeting::Process ()->Endpoint ();


  /* Answer the current call in a separate thread if required */
  if (answer_call && endpoint && endpoint->GetCallingState () == GMH323EndPoint::Called) {

    con = 
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (con) {

      con->AnsweringCall (H323Connection::AnswerCallNow);
      con->Unlock ();
    }

    return;
  }


  if (url.IsEmpty ())
    return;

  old_url = url;

  /* If it is a shortcut (# at the end of the URL), then we use it */
  if (url.GetType () == "shortcut") {

    url =
      gnomemeeting_addressbook_get_url_from_speed_dial (url.GetValidURL ());

    if (url.IsEmpty ()) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("No contact with speed dial %s# found, will call number %s instead"), (const char *) old_url.GetValidURL (), (const char *) (GMURL ().GetDefaultURL () + old_url.GetValidURL ()));
      gnomemeeting_threads_leave ();
      
      url = GMURL (GMURL ().GetDefaultURL () + old_url.GetValidURL ());
    }
  } 


  if (!url.IsEmpty ()) {

    call_address = url.GetValidURL ();

    if (!url.IsSupported ()) {

      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid URL handler"), _("Please specify a valid URL handler. Currently both h323: and callto: are supported."));
      gnomemeeting_threads_leave ();
    }
    else {

      gnomemeeting_threads_enter ();
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			  call_address);
      gnomemeeting_threads_leave ();
    }
  
    gnomemeeting_threads_enter ();
    if (!call_address.IsEmpty ()) {

      gnomemeeting_main_window_update_sensitivity (GMH323EndPoint::Calling);
      gnomemeeting_menu_update_sensitivity (GMH323EndPoint::Calling);
      endpoint->SetCallingState (GMH323EndPoint::Calling);
      
      if (!transfer_call) {

	msg = g_strdup_printf (_("Calling %s"), 
			       (const char *) call_address);
      }
      else
	msg = g_strdup_printf (_("Transferring call to %s"), 
			       (const char *) call_address);
      gnomemeeting_log_insert (msg);
      gnomemeeting_statusbar_push (gw->statusbar, msg);
    }
    g_free (msg);

    gnomemeeting_threads_leave ();
  
    if (use_gateway && !gateway.IsEmpty ()) 	
      call_address = call_address + "@" + gateway;

    /* Connect to the URL */
    if (!transfer_call) {

      con = 
	endpoint->MakeCallLocked (call_address, current_call_token);
    }
    else
      if (!url.IsEmpty () && url.IsSupported ()) {
	
	endpoint->TransferCall (endpoint->GetCurrentCallToken (),
				call_address);
	g_timeout_add (11000, (GtkFunction) TransferTimeOut, NULL);
      }
  }


  if (!transfer_call) {
    
    /* If we have a valid URL, we a have a valid connection, if not
       we put things back in the initial state */
    if (con) {

      endpoint->SetCurrentCallToken (current_call_token);
      con->Unlock ();
    }
    else {

      gnomemeeting_threads_enter ();
      gnomemeeting_menu_update_sensitivity (GMH323EndPoint::Standby);
      gnomemeeting_main_window_update_sensitivity (GMH323EndPoint::Standby);

      if (call_address.Find ("+type=directory") != P_MAX_INDEX) {
	
	gnomemeeting_statusbar_flash (gw->statusbar, _("User not found"));
	if (!transfer_call)
	  gnomemeeting_calls_history_window_add_call (1,
						      NULL,
						      call_address, 
						      "0.00",
						      _("User not found"),
						      NULL);
      }
      
      gnomemeeting_threads_leave ();

      endpoint->SetCallingState (GMH323EndPoint::Standby);
    }
  }
}
