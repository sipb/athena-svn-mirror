/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for resolving questions.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_resolve.c,v $
 *      $Author: vanharen $
 */


#include <olc/olc.h>
#include <olc/olc_tty.h>


ERRCODE
t_done(Request,title)
     REQUEST *Request;
     char *title;
{
  int status;
  char buf[LINE_SIZE];
  int instance; 

  instance = Request->requester.instance;

  if(title == (char *) NULL)
    {
      set_option(Request->options,VERIFY);
      status = ODone(Request,title);
      unset_option(Request->options, VERIFY);

      switch(status)
	{
	case SEND_INFO:
	  if(isme(Request))
	    status = t_check_connected_messages(Request);
	  else
	    status = t_check_messages(Request);

	  if(status == SUCCESS)
	    {
	      *buf = '\0';
	      get_prompted_input("Do you wish to resolve this question? ",buf);
	      if(*buf != 'y')
		return(SUCCESS);
	    }

	  get_prompted_input("Enter a title for this conversation: ", buf);
	  title = &buf[0];
	  break;

	case OK:
	  status = t_check_messages(Request);
	  if(status == SUCCESS)
	    {
	      *buf = '\0';
	      get_prompted_input("Do you wish to mark this question \"done\"? "
				 ,buf);
	      if(*buf != 'y')
		return(SUCCESS);
	    }
	  printf("Using this command means that the consultant has satisfactorly answered\n");
	  printf("your question.  If this is not the case, you can exit using the 'quit' command,\n");
	  printf("and OLC will save your question until a consultant can answer it.  If you\n");
	  printf("wish to withdraw your question, use the 'cancel' command.\n");
	  buf[0] = '\0';
	  get_prompted_input("Really done? [y/n] ", buf);
	  if(buf[0] != 'y')             
	    {
	      printf("OK, your question will remain in the queue.\n");
	      return(NO_ACTION);
	    }
	  break;

	case PERMISSION_DENIED:
	  fprintf(stderr, "You are not allowed to resolve %s's question.\n",
		  Request->target.username);
	  return(ERROR);
	  
	case NO_QUESTION:
	case NOT_CONNECTED:
	  fprintf(stderr,"You do not have a question to resolve.\n");
	  return(ERROR);
	  
	default:
	  status = handle_response(status, Request);
	  if(status != SUCCESS)
	    return(status);
	  break;
	}
    }

  status = ODone(Request,title);
  
  switch(status)
    {
    case SIGNED_OFF:
      printf("Question resolved. ");
      if(is_option(Request->options,OFF_OPT))
	printf("You have signed off OLC.\n");
      else
	printf("You are signed off OLC.\n");

      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("Question resolved.  You have been connected to another user.\n");
      status = SUCCESS;
      break;

    case OK:
      printf("The consultant has been notified that you are finished with your question.\n");
      printf("Thank you for using OLC!\n");
     if(OLC)
        exit(0);
     
      t_set_default_instance(Request);
      break;

    case SUCCESS:
      if(isme(Request))
	printf("Your question is resolved. Thank you for using OLC.\n");
      else
	{
	  printf("%s's [%d] question is resolved. Thank him for using OLC.\n",
		 Request->target.username, Request->target.instance);
	  printf("That tom and his sexist messages!\n");
	}
      if(OLC)
	exit(0);

      t_set_default_instance(Request);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to resolve %s's question.\n",
              Request->target.username);
      return(ERROR);

    case ERROR:
      fprintf(stderr, "An error has occurred.  The question\n");
      fprintf(stderr, "has not been marked resolved.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

    if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are %s [%d].\n",
  	Request->requester.username, instance,
        Request->requester.username, Request->requester.instance);

  return(status);
}

ERRCODE
t_cancel(Request,title)
     REQUEST *Request;
     char *title;
{
  int status;
  char buf[BUFSIZ];
  int instance;

  instance = Request->requester.instance;
  
  if(OLC)
    {
      printf("Using this command means that you want to withdraw your question. If you \n");
      printf("do not, OLC will store your question until a consultant can answer it.\n");
      printf("In that case, exit using the 'quit' command. If the consultant has\n");
      printf("satisfactorily answered your question, use the 'done' command to exit OLC.\n");
    }

  buf[0] = '\0';
  get_prompted_input("Are you sure you wish to cancel this question? ", buf);
  if(buf[0] != 'y')            
    {
      printf("OK, your question will remain in the queue.\n");
      return(NO_ACTION);
    }

  Request->request_type = OLC_CANCEL;
  
  if(title == (char *) NULL)
    status = OCancel(Request,"Cancelled Question");
  else
    status = OCancel(Request,title);

  switch(status)
    {
    case SUCCESS:
      printf("Question cancelled. \n");
      t_set_default_instance(Request);
      if(OLC)
	exit(0);
      status = SUCCESS;
      break;

    case OK:
      printf("Your question has been cancelled.\n");
      if(OLC)
         exit(0);
      
      t_set_default_instance(Request);
      break;

    case SIGNED_OFF:
      printf("Question cancelled.  ");
      if(is_option(Request->options,OFF_OPT))
	printf("You have signed off OLC.\n");
      else
	printf("You are signed off OLC.\n");
      
      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("You have been connected to another user.\n");
      status = SUCCESS;
      break;

    case ERROR:
      fprintf(stderr, "An error has occurred.  The question ");
      fprintf(stderr, "has not been cancelled.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are now %s [%d].\n",
	Request->requester.username, instance,
        Request->requester.username, Request->requester.instance);

  return(status);
}



