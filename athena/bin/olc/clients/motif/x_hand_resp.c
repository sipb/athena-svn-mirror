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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_hand_resp.c,v 1.2 1989-07-31 15:11:28 vanharen Exp $";
#endif

#include "xolc.h"

int
handle_response(response, req)
     int response;
     REQUEST *req;
{
  char message[256];
#ifdef KERBEROS
  char *kmessage = "\nYou will have been properly authenticated when you do not see this\nmessage the next time you run olc. If you were having trouble\nwith a program, try again.\n\nIf you continue to have difficulty, feel free to contact a user\nconsultant by phone. Schedules and phone numbers are posted in\nthe clusters.";
#endif KERBEROS

  switch(response)
    {
    case UNKNOWN_REQUEST:
      popup_error("This function cannot be performed by the OLC server.\nWhat you want is down the hall to the left.");
      return(NO_ACTION);

    case SIGNED_OFF:
      popup_error("You have signed off of OLC.");
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(string_eq(req->target.username,req->requester.username))
	popup_error("You are not signed on to OLC.");
      else {
	sprintf(message, "%s (%d) is not signed on to OLC.",
                req->target.username,req->target.instance);
        popup_error(message);
      }
      return(NO_ACTION);

    case NO_QUESTION:
      sprintf(message,"%s (%d) does not have a question.",
	      req->target.username, req->target.instance);
      popup_error(message);
      return(ERROR);

    case HAS_QUESTION:
      popup_error("You have a question.");
      return(ERROR);

    case NOT_CONNECTED:
      if(string_eq(req->target.username,req->requester.username))
	popup_error("You are not connected nor do you seem to have a question.\nPerhaps you should ask one.\n");
      else {
        sprintf(message,"%s (%d) is not connected nor is asking a question.\n",
                req->target.username,req->target.instance);
	popup_error(message);
      }
      return(NO_ACTION);

    case PERMISSION_DENIED:
      popup_error("You are not allowed to do that.");
      return(NO_ACTION);

    case TARGET_NOT_FOUND:
      sprintf(message, "Target user %s (%d) unknown.\n",  req->target.username,
              req->target.instance);
      popup_error(message);
      return(ERROR);

    case REQUESTER_NOT_FOUND:
      sprintf(message, "You [%s (%d)] are unknown.\n",
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
      sprintf(message, "User, %s, not found.\n",req->target.username);
      popup_error(message);
      return(ERROR);

#ifdef KERBEROS     /* these error codes are < 100 */
    case MK_AP_TGTEXP:
    case RD_AP_EXP:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Your Kerberos ticket has expired. ");
      printf("To renew your Kerberos tickets,\n");
      printf("type:    kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case NO_TKT_FIL:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("You do not have a Kerberos ticket file. To ");
      printf("get one, \ntype:    kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case TKT_FIL_ACC:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Cannot access your Kerberos ticket file.\n");
      printf("Try:              setenv   KRBTKFILE  /tmp/random\n");
      printf("                  kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case RD_AP_TIME:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Kerberos authentication failed: workstation clock is");
      printf("incorrect.\nPlease contact Athena operations and move to ");
      printf("another worstation.\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
#endif KERBEROS

    case SUCCESS:
      return(SUCCESS);

    default:
      fprintf(stderr, "Unknown response %d (fascinating)\n", response);
      return(ERROR);
    }
}

popup_error(message)
     char *message;
{
  Arg arg;

  fprintf(stderr, message);
  XtSetArg(arg, XmNmessageString, XmStringLtoRCreate(message, ""));
  XtSetValues(w_error_dlg, &arg, 1);
  XtManageChild(w_error_dlg);
}
