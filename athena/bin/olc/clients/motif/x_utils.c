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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_utils.c,v $
 *	$Id: x_utils.c,v 1.3 1991-03-24 14:36:03 lwvanels Exp $
 *      $Author: lwvanels $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_utils.c,v 1.3 1991-03-24 14:36:03 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include "xolc.h"

ERRCODE
handle_response(response, req)
     int response;
     REQUEST *req;
{
  int status;
  char message[BUF_SIZE];
#ifdef KERBEROS
  char kmessage[BUF_SIZE];

  strcpy(kmessage,"\n\nIf you were having trouble with some other program, problems with your\nkerberos tickets may have been the reason.  Try the other program again\nafter getting new kerberos tickets with `kinit'.\n\n");

#ifdef ATHENA
  strcat(kmessage, "If you continue to have difficulty, feel free to contact a user consultant\nby phone at 253-4435.\n");
#else
  strcat(kmessage, "If you continue to have difficulty, contact a user consultant.\n");
#endif
  strcat(kmessage, "\nOnce you have gotten new kerberos tickets, you may try to continue with OLC.\nIf you wish to continue, click on the `Try again' button below.\nIf you wish to exit OLC now, click on the `Quit' button.");
#endif

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
	if(OLC) {
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
                OLC?"consultant":"user");
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
#ifdef ATHENA
      strcpy(message, "Kerberos authentication failed: workstation clock is incorrect.\nPlease contact Athena operations and move to another workstation.");
#else
      strcpy(message, "Kerberos authentication failed; the clock on this workstation is incorrect.\nPlease contact the maintainer of this workstation to update it.");
#endif
      if(OLC)
	strcat(message, kmessage);
      status = popup_option(message);
      return(status);
#endif

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
  Arg arg;

  if (MuGetBoolean(message, "Try again", "Quit", NULL, TRUE))
    return(FAILURE);
  else
    return(ERROR);
}
