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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_hand_resp.c,v $
 *      $Author: vanharen $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_hand_resp.c,v 1.3 1989-10-11 16:21:56 vanharen Exp $";
#endif

#include "xolc.h"

int
handle_response(response, req)
     int response;
     REQUEST *req;
{
  int status;
  char message[BUF_SIZE];
#ifdef KERBEROS
  char *kmessage = "\n\nIf you were having trouble with some other program, problems with your\nkerberos tickets may have been the reason.  Try the other program again\nafter getting new kerberos tickets with `kinit'.\n\nIf you continue to have difficulty, feel free to contact a user consultant\nby phone at 253-4435.\n\nOnce you have gotten new kerberos tickets, you may try to continue with OLC.\nIf you wish to continue, click on the `Try again' button below.\nIf you wish to exit OLC now, click on the `Quit' button.";
#endif KERBEROS

  switch(response)
    {
    case UNKNOWN_REQUEST:
      popup_error("This function cannot be performed by the OLC server.\nWhat you want is down the hall to the left.");
      return(NO_ACTION);

    case SIGNED_OFF:
      if(isme(req))
        popup_error("You have signed off of OLC.");
      else {
	sprintf(message, "%s is singed off of OLC.",req->target.username);
	popup_error(message);
      }
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(isme(req))
	popup_error("You are not signed on to OLC.");
      else {
	sprintf(message, "%s (%d) is not signed on to OLC.",
                req->target.username,req->target.instance);
        popup_error(message);
      }
      return(NO_ACTION);

    case NO_QUESTION:
      if(isme(req)) {
	if(OLC) {
	  popup_error("You do not have a question in OLC.\n\nIf you wish to ask another question, use 'olc' again.");

/* 
 * Something intelligent should be done here...  ask them if they want to
 *  ask a new question in OLC, perhaps, and push them through that loop,
 *  else, have them exit the client...   or something...
 */
	}
	else {
	  popup_error("You do not have a question in OLC.");
	}
      }
      else {
	sprintf(message,"%s (%d) does not have a question.",
		req->target.username, req->target.instance);
	popup_error(message);
      }
      return(NOT_CONNECTED);

    case HAS_QUESTION:
      if(isme(req))
        popup_error("You have a question.");
      else {
        sprintf(message, "%s (%d) does not have a question.",
                req->target.username,req->target.instance);
	popup_error(message);
      }
      return(ERROR);

    case NOT_CONNECTED:
      if(isme(req)) {
	sprintf(message, "You are not connected to a %s.",
                OLC?"consultant":"user");
	popup_error(message);
      }
      else {
        sprintf(message,"%s (%d) is not connected nor is asking a question.",
                req->target.username,req->target.instance);
	popup_error(message);
      }
      return(NO_ACTION);

    case PERMISSION_DENIED:
      popup_error("You are not allowed to do that.");
      return(NO_ACTION);

    case TARGET_NOT_FOUND:
      sprintf(message, "Target user %s (%d) not found.",
	      req->target.username, req->target.instance);
      popup_error(message);
      return(ERROR);

    case REQUESTER_NOT_FOUND:
      sprintf(message, "You [%s (%d)] are unknown.  There is a problem.",
              req->requester.username,
              req->requester.instance);
      popup_error(message);
      return(ERROR);

    case INSTANCE_NOT_FOUND:
      popup_error("Incorrect instance specified.");
      return(ERROR);

    case ERROR:
      popup_error("Error response from daemon.");
      return(ERROR);

    case USER_NOT_FOUND:
      sprintf(message, "User \"%s\" not found.",req->target.username);
      popup_error(message);
      return(ERROR);

    case NAME_NOT_UNIQUE:
      sprintf(message, "The string %s is not unique.",req->target.username);
      popup_error(message);
      return(ERROR);

#ifdef KERBEROS     /* these error codes are < 100 */
    case MK_AP_TGTEXP:

    case RD_AP_EXP:
      strcpy(message, "Your Kerberos ticket has expired.  To renew your Kerberos tickets, type:\n\n        kinit"); 
      if(OLC)
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case NO_TKT_FIL:
      strcpy(message, "You do not have a Kerberos ticket file. To get one, type:\n\n        kinit"); 
      if(OLC)
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case TKT_FIL_ACC:
      strcpy(message, "Cannot access your Kerberos ticket file.  Try:\n\n        setenv   KRBTKFILE  /tmp/random\n        kinit");
      if(OLC)
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

    case RD_AP_TIME:
      strcpy(message, "Kerberos authentication failed: workstation clock is incorrect.\nPlease contact Athena operations and move to another workstation.");
      if(OLC)
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);

#endif KERBEROS

    case SUCCESS:
      return(SUCCESS);

    default:
      sprintf(message, "Unknown response %d (fascinating).", response);
      MuErrorSync(message);
      return(ERROR);
    }
}

popup_error(message)
     char *message;
{
  MuError(message);
}

int
popup_option(message)
     char *message;
{
  Arg arg;

  if (MuGetBoolean(message, "Try again", "Quit", NULL, TRUE, Mu_Popup_Center))
    return(FAILURE);
  else
    return(ERROR);
}
