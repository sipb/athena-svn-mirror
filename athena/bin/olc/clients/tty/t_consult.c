/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_consult.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_consult.c,v 1.3 1989-08-15 00:24:48 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>


ERRCODE
t_sign_on(Request)
     REQUEST *Request;
{
  int status;

  status = OSignOn(Request);
  switch (status)
    { 
    case SUCCESS: 
      if(isme(Request))
	printf("You have signed on.\n");
      else
	printf("%s (%d) is signed on. I hope you told him.\n",
	       Request->target.username,Request->target.instance);
      status = SUCCESS; 
      break; 

    case ALREADY_SIGNED_ON:
      if(isme(Request))
	fprintf(stderr,"You are already signed on at this level.\n");
      else
	printf("%s (%d) is already signed on at this level.\n",
	       Request->target.username,Request->target.instance);
      status = NO_ACTION;
      break;

    case HAS_QUESTION:
      if(isme(Request))
	fprintf(stderr,
	       "You can't ask a question and sign on in the same instance.\n");
      else
	printf("%s (%d) is already asking a question in that instance.\n",
	       Request->target.username,Request->target.instance);
      break;

    case ALREADY_CONNECTED: 
      if(isme(Request))
	printf("You have signed on to OLC. You are already connected.\n");
      else
	printf("%s (%d) is signed on and connected.\n",
	        Request->target.username,Request->target.instance);
      status = SUCCESS;
      break; 	

    case CONNECTED: 
      if(isme(Request))
	printf("Congratulations, you have been connected.\n");
      else
        printf("%s (%d) has just been connected and it's your fault.\n",
               Request->target.username,Request->target.instance);
      status = SUCCESS;
      break;

    case PERMISSION_DENIED:
      if(isme(Request))
        fprintf(stderr, "You are not allowed to sign on at that level.\n");
      else
        fprintf(stderr, "%s is not allowed to sign on at that level.\n",
                Request->target.username);
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }
    
  return(status);
}
	    



ERRCODE
t_olc_off(Request)
     REQUEST *Request;
{
  int status;
  
  status = OSignOff(Request);

  switch (status)
    {
    case SUCCESS:
      printf("You have signed off of OLC.\n");
      break;

    case NOT_SIGNED_ON:
      fprintf(stderr, "You are not signed on to OLC.\n");
      status = NO_ACTION;
      break;

    case CONNECTED:
      fprintf(stderr,
              "You have signed off of OLC but you are still connected.\n");
      status = NO_ACTION;
      break;

    case ERROR:
     fprintf(stderr,
              "An error has occurred. You are not signed off.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}




