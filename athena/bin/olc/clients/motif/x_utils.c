/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Id: x_utils.c,v 1.9 1999-06-28 22:51:58 ghudson Exp $
 */


#ifndef lint
static char rcsid[]= "$Id: x_utils.c,v 1.9 1999-06-28 22:51:58 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include "xolc.h"

ERRCODE
handle_response(response, req)
     int response;
     REQUEST *req;
{
  ERRCODE status;
  char message[BUF_SIZE];
#ifdef HAVE_KRB4
  char kmessage[BUF_SIZE];

  strcpy(kmessage,"\n\nIf you were having trouble with some other program, problems with your\nkerberos tickets may have been the reason.  Try the other program again\nafter getting new kerberos tickets with `renew'.\n\n");

  strcat(kmessage, "If you continue to have difficulty, "
#ifdef CONSULT_PHONE_NUMBER
	 "you can contact a user consultant\nby phone at "
	 CONSULT_PHONE_NUMBER ".\n"
#else /* no CONSULT_PHONE_NUMBER */
	 "contact a user consultant.\n"
#endif /* no CONSULT_PHONE_NUMBER */
	 );

  strcat(kmessage, "\nOnce you have gotten new kerberos tickets, you may try to continue with OLC.\nIf you wish to continue, click on the `Try again' button below.\nIf you wish to exit OLC now, click on the `Quit' button.");
#endif /* HAVE_KRB4 */

  switch(response)
    {
    case UNKNOWN_REQUEST:
      MuError("This function cannot be performed by the OLC server.");
      return(NO_ACTION);

    case SIGNED_OFF:
      if(isme(req))
        MuError("You have signed off of OLC.");
      else {
	sprintf(message, "%s is singed off of OLC.",req->target.username);
	MuError(message);
      }
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(isme(req))
	MuError("You are not signed on to OLC.");
      else {
	sprintf(message, "%s (%d) is not signed on to OLC.",
                req->target.username,req->target.instance);
        MuError(message);
      }
      return(NO_ACTION);

    case NO_QUESTION:
      if(isme(req)) {
	if(client_is_user_client()) {
	  MuError("You do not have a question in OLC.\n\nIf you wish to ask another question, use 'olc' again.");

/* 
 * Something intelligent should be done here...  ask them if they want to
 *  ask a new question in OLC, perhaps, and push them through that loop,
 *  else, have them exit the client...   or something...
 */
	}
	else {
	  MuError("You do not have a question in OLC.");
	}
      }
      else {
	sprintf(message,"%s (%d) does not have a question.",
		req->target.username, req->target.instance);
	MuError(message);
      }
      return(NOT_CONNECTED);

    case HAS_QUESTION:
      if(isme(req))
        MuError("You have a question.");
      else {
        sprintf(message, "%s (%d) does not have a question.",
                req->target.username,req->target.instance);
	MuError(message);
      }
      return(ERROR);

    case NOT_CONNECTED:
      if(isme(req)) {
	sprintf(message, "You are not connected to a %s.",
		client_is_user_client()?"consultant":"user");
	MuError(message);
      }
      else {
        sprintf(message,"%s (%d) is not connected nor is asking a question.",
                req->target.username,req->target.instance);
	MuError(message);
      }
      return(NO_ACTION);

    case PERMISSION_DENIED:
      MuError("You are not allowed to do that.");
      return(NO_ACTION);

    case TARGET_NOT_FOUND:
      sprintf(message, "Target user %s (%d) not found.",
	      req->target.username, req->target.instance);
      MuError(message);
      return(ERROR);

    case REQUESTER_NOT_FOUND:
      sprintf(message, "You [%s (%d)] are unknown.  There is a problem.",
              req->requester.username,
              req->requester.instance);
      MuError(message);
      return(ERROR);

    case INSTANCE_NOT_FOUND:
      MuError("Incorrect instance specified.");
      return(ERROR);

    case ERROR:
      MuError("Error response from daemon.");
      return(ERROR);

    case USER_NOT_FOUND:
      sprintf(message, "User \"%s\" not found.",req->target.username);
      MuError(message);
      return(ERROR);

    case NAME_NOT_UNIQUE:
      sprintf(message, "The string %s is not unique.",req->target.username);
      MuError(message);
      return(ERROR);

#ifdef HAVE_KRB4     /* these error codes are < 100 */
    case MK_AP_TGTEXP:
    case RD_AP_EXP:
      strcpy(message, "Your Kerberos ticket has expired.  To renew your Kerberos tickets, type:\n\n        renew"); 
      if(client_is_user_client())
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case NO_TKT_FIL:
      strcpy(message, "You do not have a Kerberos ticket file. To get one, type:\n\n        renew"); 
      if(client_is_user_client())
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case TKT_FIL_ACC:
      strcpy(message, "Cannot access your Kerberos ticket file.  Try:\n\n        setenv   KRBTKFILE  /tmp/random\n        renew");
      if(client_is_user_client())
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case RD_AP_TIME:
      strcpy(message, "Kerberos authentication failed: this workstation's "
	     "clock is set incorrectly.\nPlease move to another workstation "
	     "and notify "
#ifdef HARDWARE_MAINTAINER
             HARDWARE_MAINTAINER
#else /* no HARDWARE_MAINTAINER */
             "the workstation's maintainer"
#endif /* no HARDWARE_MAINTAINER */
             " of this problem.");  /* was ATHENA */
      if(client_is_user_client())
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);
#endif /* HAVE_KRB4 */

    case SUCCESS:
      return(SUCCESS);

    default:
      sprintf(message, "Unknown response %d.", response);
      MuErrorSync(message);
      return(ERROR);
    }
}

int
popup_option(message)
     char *message;
{
  if (MuGetBoolean(message, "Try again", "Quit", NULL, TRUE))
    return(FAILURE);
  else
    return(ERROR);
}
