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
 *      $Author: vanharen $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_consult.c,v 1.7 1990-02-15 18:13:42 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>


ERRCODE
t_sign_on(Request,flag,hold)
     REQUEST *Request;
     int flag;
     int hold;
{
  int status;
  int instance;
  
  if(flag)
    set_option(Request->options,SPLIT_OPT);

  instance = Request->requester.instance;

  status = OSignOn(Request);
  switch (status)
    { 
    case SUCCESS: 
      if(isme(Request))
	printf("You have signed on to OLC.\n");
      else
	printf("%s [%d] is signed on.  I hope you told him.\n",
	       Request->target.username,Request->target.instance);
      status = SUCCESS; 
      break; 

    case ALREADY_SIGNED_ON:
      if(isme(Request))
	fprintf(stderr,"You are already signed on.\n");
      else
	printf("%s [%d] is already signed on.\n",
	       Request->target.username,Request->target.instance);
      status = NO_ACTION;
      break;

    case HAS_QUESTION:
      if(isme(Request))
	{
	  printf("Your current instance is busy, creating another one for you.\n");
	  return(t_sign_on(Request,TRUE,hold));
	  break;
	}
      else
	printf("%s [%d] is already asking a question in that instance.\n",
	       Request->target.username,Request->target.instance);
      break;

    case MAX_ANSWER:
      printf("You cannot answer any more questions simultaneously\n");
      printf("without forwarding one of your existing connections.\n");
      status = NO_ACTION;
      break;

    case ALREADY_CONNECTED: 
      if(isme(Request))
	printf("You have signed on to OLC. You are already connected.\n");
      else
	printf("%s [%d] is signed on and connected.\n",
	        Request->target.username,Request->target.instance);
      status = SUCCESS;
      break; 	

    case CONNECTED: 
      if(isme(Request))
	printf("Congratulations, you have been connected.\n");
      else
        printf("%s [%d] has just been connected and it's your fault.\n",
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

  if((instance != Request->requester.instance) && !hold)
    {
      printf("You are now %s [%d].\n",Request->requester.username,
	     Request->requester.instance);
      User.instance =  Request->requester.instance;
    }

  return(status);
}
	    



ERRCODE
t_olc_off(Request)
     REQUEST *Request;
{
  int status;
  int instance;

  instance = Request->requester.instance;
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

    case NOT_CONNECTED:
      printf("You have signed off.  This instance has been terminated.\n");
      status = SUCCESS;
      
      t_set_default_instance(Request);
      break;

    case ERROR:
     fprintf(stderr,
              "An error has occurred.  You are not signed off.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are now %s [%d]. %s\n",
  	Request->requester.username, instance,
        Request->requester.username, Request->requester.instance, 
	happy_message());

  return(status);
}




