/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for handling requests from the olc
 * client.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/requests_olc.c,v $
 *	$Id: requests_olc.c,v 1.42 1991-04-08 21:11:19 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/requests_olc.c,v 1.42 1991-04-08 21:11:19 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

/*
 * Function:	olc_on() signs a user on to the OLC system.  If
 *			there is a pending question, a message is sent
 *			to the user signing on and a connection between 
 *			both users is established if possible.
 * Arguments:	fd:		File descriptor of the socket.
 *		request:	The request structure from olcr.
 * Returns:	A response code.
 * Notes:
 *	First, we verify that the person is allowed to sign on.  If
 *	she is, check to see if she is already signed on.  If not, add
 *	her to the ring, set up the status flags, and check for users with
 *	pending questions.  As long as there are users with questions,
 *	connect them and notify both users.  Once she is done
 *	with them, log the fact that she is signed on.
 */

ERRCODE
olc_on(fd, request)
     int fd;			/* File descriptor for socket. */
     REQUEST *request;	        /* Request structure from olcr. */
{
  KNUCKLE *requester;
  KNUCKLE *target;
  KNUCKLE **k_ptr;
  int qcount = 0;
  char msgbuf[BUF_SIZE];
  int status;
  char st[10];

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));
      
  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status != SUCCESS)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!((is_me(target,requester)) && 
	is_allowed(requester->user,ON_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(has_question(target) && !(is_option(request->options,SPLIT_OPT)))
    {
      for (k_ptr = target->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{
	  if(is_signed_on((*k_ptr)))
	    return(send_response(fd, ALREADY_SIGNED_ON));
	  if((*k_ptr)->question != NULL)
	    if((*k_ptr)->question-> owner != (*k_ptr))
	      ++qcount;
	}
      if(qcount < target->user->max_answer)
	return(send_response(fd,HAS_QUESTION));
      else
	return(send_response(fd,MAX_ANSWER));
    }
  else
    if(is_option(request->options,SPLIT_OPT))
    {
#ifdef LOG
      sprintf(msgbuf,"olc on: created new knuckle2 for %s",
	      target->user->username);
      log_status(msgbuf);
#endif /* LOG */

      for (k_ptr = target->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{
	  if(is_signed_on((*k_ptr)))
	    return(send_response(fd, ALREADY_SIGNED_ON));
	  if((*k_ptr)->question != NULL)
	    if((*k_ptr)->question-> owner != (*k_ptr))
	      ++qcount;
	}

      if(qcount >= target->user->max_answer)
	return(send_response(fd,MAX_ANSWER));

      target = create_knuckle(target->user);
    }

  /* translate request option */
  if(request->options & ON_FIRST)
    request->options = FIRST;
  else if(request->options & ON_SECOND)
    request->options = SECOND;
  else if(request->options & ON_DUTY)
    request->options = DUTY;
  else if(request->options & ON_URGENT)
    request->options = URGENT;
  else
    request->options = DUTY;

  if(is_signed_on(target))
    {
      sign_off(target);
      sign_on(target,request->options);
      return(send_response(fd, ALREADY_SIGNED_ON));
    }

  target->title = target->user->title2;
  sign_on(target,request->options);
  OGetStatusString(request->options,st);
  sprintf(msgbuf,"%s %s (%s [%d]) signed on (%s).",cap(target->title),
	  target->user->realname, target->user->username, target->instance,
	  st);

#ifdef LOG
  log_status(msgbuf);
#endif /* LOG */

  olc_broadcast_message("nol", fmt("%s\n", msgbuf), "on");

  if(is_connected(target))
    {
      send_response(fd,ALREADY_CONNECTED);
      sprintf(msgbuf,"%s [%d]",target->connected->user->username,
	      target->connected->instance);
      write_text_to_fd(fd,msgbuf);
      return(SUCCESS);
    }
           
  status = match_maker(target);     
  if(status == SUCCESS)
    {
      send_response(fd, CONNECTED);
    }
  else
    send_response(fd,SUCCESS);

  needs_backup = TRUE;
  return(SUCCESS);
}

/*
 * Function:     olc_create_instance()  creates a new knuckle
 * Arguments:    fd:                    File descriptor socket
 *               request:               The request from olc
 * Returns:      PERMISSION_DENIED: 
 *               USER_NOT_FOUND:
 *               ERROR
 *               SUCCESS:
 * Notes:        Creates a new instance of the target user. Upon
 *               successful response to the client. the id of this 
 *               new instance is sent back to the client. Future
 *               References to this knuckle should be made with the
 *               id.   
 */

ERRCODE
olc_create_instance(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  KNUCKLE *knuckle;
  int status;
  
#ifdef LOG
  char buf[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));
      
  if(!isme(request))
    { 
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status != SUCCESS)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_me(target,requester)))
    return(send_response(fd,PERMISSION_DENIED));

  if(is_active(target))
    knuckle = create_knuckle(target->user);
  else
    return(send_response(fd,PERMISSION_DENIED));

  if(knuckle == (KNUCKLE *) NULL)
    return(send_response(fd,ERROR));

#ifdef LOG
  sprintf(buf,"%s [%d] extended to %d", target->user->username,
	  target->instance, knuckle->instance);
  log_status(buf);
#endif /* LOG */

  send_response(fd,SUCCESS);
  write_int_to_fd(fd,knuckle->instance);
  return(SUCCESS);
}

ERRCODE
olc_verify_instance(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  int status;
  int instance;

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));
     
  if(!isme(request))
    {   
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status != SUCCESS)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_me(target,requester)) &&
     !(is_allowed(requester->user,MONITOR_ACL)))
    return(send_response(fd,status));

  if(request->version == VERSION_3)
    {
      send_response(fd,SUCCESS);
      read_int_from_fd(fd,&instance);
      return(send_response(fd,verify_instance(target,instance)));
    }

  if(is_me(target,requester))
    {
      send_response(fd,SUCCESS);
      read_int_from_fd(fd,&instance);
      return(send_response(fd,verify_instance(target,instance)));
    }
  else
    {
      if(!is_me(target->connected, requester))
	return(send_response(fd, NOT_CONNECTED));
      else
	{
	  send_response(fd, OK);
	  write_int_to_fd(fd,target->connected->instance);
	}
    }
  return(SUCCESS);
}


ERRCODE

olc_default_instance(fd,request)
     int fd;
     REQUEST *request;
{
  int status;
  int instance;

  status = get_instance(request->target.username, &instance);
  if(status)
    return(send_response(fd,status));

  send_response(fd,SUCCESS);
  write_int_to_fd(fd,instance);
  
  return(status);
}


ERRCODE

olc_who(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  LIST list;
  char message[BUF_SIZE];
  int status;
  
/* don't want to cache info with this command */

  status = get_knuckle(request->requester.username, 
		       request->requester.instance, &requester,0);
  if(status != SUCCESS)
    return(send_response(fd,status));
      
  if(!isme(request))
    {
      status = match_knuckle(request->target.username,
			     request->target.instance,
			     &target);
      if(status != SUCCESS)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_allowed(requester->user,MONITOR_ACL) || is_me(target,requester)))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  get_list_info(target, &list);
  send_list(fd, request, &list);

  if(is_logout(requester) && is_active(requester))
    {
      if(has_new_messages(requester))
	{
	  if (owns_question(requester))
	    {
	      sprintf(message,"A %s has sent you a response in %s.\nTo see it, type 'show' at the %s prompt.\n",
		      DEFAULT_TITLE2,DaemonInst,DaemonInst);
	    }
	  else
	    sprintf(message, "You have new messages in %s.\n",DaemonInst);
	}
      else
	{
	  if (owns_question(requester))
	    {
	      sprintf(message,"Your question is still pending in %s.\nCheck your mail for any responses.\n",
		      DaemonInst);
	    }
	  else
	    sprintf(message,"You are still connected in %s.\n",DaemonInst);
	}
      
      if (write_message_to_user(requester,message,NULL_FLAG) == SUCCESS)
	{
	  sprintf(message, "%s %s has logged back in.",
		  cap(requester->title), requester->user->username);
	  log_daemon(requester,message);

#ifdef LOG
	  log_status(message);
#endif /* LOG */

	  strcat(message, "\n");
	  if (requester->connected != (KNUCKLE *) NULL)
	    {
	      if ((write_message_to_user(requester->connected,
					 message,NULL_FLAG)
		   != SUCCESS)
		  && (owns_question(requester)))
		{
		  free_new_messages(requester->connected);
		  deactivate(requester->connected);
		  disconnect_knuckles(requester, requester->connected);
		  if (!(requester->status & REFERRED))
		    set_status(requester, PENDING);
		  olc_broadcast_message("resurrection",message, 
					requester->question->topic);
		}
	    }
	  else
/*	    if (!(requester->status & REFERRED))
	      set_status(requester, PENDING); */
	    olc_broadcast_message("resurrection",message, 
				  requester->question->topic);
	  needs_backup = TRUE;
	}
    }

  return(SUCCESS);
}

 
/*
 * Function:	olc_done()      marks a question resolved.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olc.
 * Returns:	NOT_SIGNED_ON:	Consultant is not signed on to OLC.
 *		NOT_CONNECTED:	Consultant is not connected to a user.
 *		SUCCESS:	Question successfully resolved.
 * Notes:
 *	The request type is OLC_DONE.
 *	First, look up the consultant in the ring.  If she is connected
 *	to a user, log the fact that the question is done, and terminate
 *	the log. Then notify the user that the question is done and remove
 *	her from the user ring.  Finally, connect the consultant to a
 *	pending user if there is one, unless the OFF_OPT is set.  In that
 *	case, sign the consultant off of OLC.
 */

ERRCODE
olc_done(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *target;
  KNUCKLE *requester, *consultant;
  char msgbuf[BUF_SIZE];	      /* Message buffer. */
  int status;
  char *text;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if (status)
	return(send_response(fd, status));
    }
  else
    target = requester;

  if(!(is_me(target,requester)) && 
     !((is_connected_to(target,requester)) && 
       (is_allowed(requester->user, CONSULT_ACL)) &&
       (owns_question(target))) &&
     !(is_allowed(requester->user, GRESOLVE_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

  if((is_option(request->options,VERIFY)) && 
     owns_question(target) && (is_me(target,requester)))
    return(send_response(fd,OK));
  else
    if(is_option(request->options,VERIFY))
	return(send_response(fd,SEND_INFO));

  if(owns_question(target) && (is_me(requester,target)))
    {
      if(!is_connected(target))
	{
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] is done with question",
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */
	  sprintf(target->question->title,
		  target->question->note[0]
		  ? target->question->note
		  : "No consultant present.");
	  log_daemon(target, "User is done with question.");
	  terminate_log_answered(target);
	  free((char *) target->question);
	  target->question = (QUESTION *) NULL;
	  free_new_messages(target);
	  deactivate(target);
          needs_backup = TRUE;
	  return(send_response(fd, SUCCESS));
	}
      else
	{
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] signals done",
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */
	  set_status(target,DONE);
	  sprintf(msgbuf,"%s %s is done with question.", cap(target->title),
		  target->user->username);
	  log_daemon(target,msgbuf);
	  if (write_message_to_user(target->connected, fmt("%s\n", msgbuf),
				NULL_FLAG)
	      != SUCCESS)
	    {
	      sprintf(target->question->title,
		      target->question->note[0]
		      ? target->question->note
		      : "No consultant present.");
	      log_daemon(target, "User is done with question.");
	      terminate_log_answered(target);
	      free_new_messages(target);
	      free_new_messages(target->connected);
	      deactivate(target);
	      deactivate(target->connected);
	      disconnect_knuckles(target, target->connected);
	      free((char *) target->question);
	      target->question = (QUESTION *) NULL;
	    }
          needs_backup = TRUE;
	  return(send_response(fd, OK));
	}
    }
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] dones %s [%d]'s question",
		  requester->user->username,requester->instance,
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */

  send_response(fd,SEND_INFO);
  if((target == requester) && !(owns_question(requester)))
      target = requester->connected;

  text = read_text_from_fd(fd);

  (void) strncpy(target->question->title, text,
		 sizeof(target->question->title) - 1);
  target->question->title[sizeof(target->question->title) - 1] = '\0';
  (void) sprintf(msgbuf, "Resolved by %s@%s.", 
		 requester->user->username,
		 requester->user->machine);
  log_daemon(target,msgbuf);

  if ( ! ((target->status == DONE) || (target->status == CANCEL)))
    {
      (void) sprintf(msgbuf,"%s %s has resolved your question about \"%s\".\n",
		     cap(requester->title), requester->user->username, 
		     target->question->topic);
      
      (void) write_message_to_user(target, msgbuf, NO_RESPOND);
    }
  
  terminate_log_answered(target);
  
  if (is_option(request->options, OFF_OPT))
    deactivate(target->connected);

/*  if(requester->instance > 0 && target->connected == requester)
    deactivate(requester);*/
  
  consultant = target->connected;
  free_new_messages(target);
  if (target->connected != NULL)
    free_new_messages(target->connected);
  deactivate(target);
  disconnect_knuckles(target, target->connected);
  free((char *) target->question);
  target->question = (QUESTION *) NULL;

/*  sub_status(target->connected,BUSY);*/

  if (consultant) {
      if (is_signed_on(consultant)) {
	  status = match_maker(consultant);
	  if(status == SUCCESS)
	      send_response(fd,CONNECTED);
	  else
	      send_response(fd,SUCCESS);
      }
      else {
	  send_response(fd, SIGNED_OFF);
	  deactivate(consultant);
      }
  }
  else
    send_response(fd,SUCCESS);

  needs_backup = TRUE;
  return(SUCCESS);
}





ERRCODE
olc_cancel(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *target;
  KNUCKLE *requester, *consultant;
  char msgbuf[BUF_SIZE];	      /* Message buffer. */
  int status;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {        
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if (status)
	return(send_response(fd, status));
    }
  else
    target = requester;

  if(!(is_me(requester,target)) && 
     !(is_connected_to(requester,target) && 
       (is_allowed(requester->user, CONSULT_ACL)) &&
       (owns_question(target))) &&
     !(is_allowed(requester->user, GRESOLVE_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(is_option(request->options,VERIFY))
    if (has_question(target))
      return(send_response(fd, OK));
    else
      return(send_response(fd, NO_QUESTION));

  if(!is_connected(target) && !has_question(target))
    return(send_response(fd, NOT_CONNECTED));

  if(owns_question(target) && (is_me(requester,target)))
    {
      if(!is_connected(target))
	{
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] has cancelled question",
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */
	  (void) strcpy(target->question->title,
			target->question->note[0]
			? target->question->note
			: "Cancelled question/No consultant present.");
	  log_daemon(target,"User cancelled question.");
	  terminate_log_answered(target);
	  free((char *) target->question);
	  target->question = (QUESTION *) NULL;
	  free_new_messages(target);
	  deactivate(target);
          needs_backup = TRUE;
	  return(send_response(fd, OK));
	}
      else
	{
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] signals cancel",
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */
	  sprintf(msgbuf,"%s %s cancelled question.",cap(target->title),
		  target->user->username);
	  set_status(target,CANCEL);
	  log_daemon(target,msgbuf);
	  sprintf(msgbuf,"Question cancelled by %s %s.\n",
		  target->title, target->user->username);
	  consultant = target->connected;
	  if (write_message_to_user(consultant,msgbuf, NULL_FLAG) != SUCCESS)
	    {
	      (void) strcpy(target->question->title,
			    target->question->note[0]
			    ? target->question->note
			    : "Cancelled question/No consultant present.");
	      log_daemon(target,"User cancelled question.");
	      terminate_log_answered(target);
	      free_new_messages(target);
	      free_new_messages(consultant);
	      deactivate(target);
	      deactivate(consultant);
	      disconnect_knuckles(target, consultant);
	      free((char *) target->question);
	      target->question = (QUESTION *) NULL;
	    }
          needs_backup = TRUE;
	  return(send_response(fd, SUCCESS));
	}
    }

  if((target == requester) && !(owns_question(requester)))
    target = requester->connected;
  consultant = target->connected;

  (void) strcpy(target->question->title,
		target->question->note[0]
		? target->question->note
		: "Cancelled question.");
#ifdef LOG
	  sprintf(msgbuf,"%s [%d] cancels %s [%d]'s question",
		  requester->user->username,requester->instance,
		  target->user->username,target->instance);
	  log_status(msgbuf);
#endif /* LOG */

  (void) sprintf(msgbuf, "Cancelled by %s@%s.", 
		 requester->user->username,
		 requester->user->machine);
  log_daemon(target,msgbuf);

  if ( ! ((target->status == DONE) || (target->status == CANCEL)))
    {
      (void) sprintf(msgbuf,
		     "%s %s has cancelled your question about \"%s\".\n",
		     cap(requester->title), requester->user->username,
		     target->question->topic);
  
      (void) write_message_to_user(target, msgbuf, NULL_FLAG);
    }

  terminate_log_answered(target);
  
  if (is_option(request->options, OFF_OPT))
    deactivate(target->connected);

/*  if(requester->instance > 0 && target->connected == requester)
    deactivate(requester);*/

  if (target->connected != NULL)
    free_new_messages(target->connected);
  free_new_messages(target);
  deactivate(target);
  if (consultant != NULL)
    disconnect_knuckles(target, consultant);
  free((char *) target->question);
  target->question = (QUESTION *) NULL;

/*  sub_status(target->connected,BUSY);*/

  if (consultant)
    if (is_signed_on(consultant))
      {
	status = match_maker(consultant);
	if(status == SUCCESS)
	  send_response(fd,CONNECTED);
	else
	  send_response(fd,SUCCESS);
      }
    else
      {
	send_response(fd, SIGNED_OFF);
	deactivate(consultant);
      }

  needs_backup = TRUE;
  return(SUCCESS);
}


/*
 *
 */

ERRCODE
olc_ask(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *target;
  KNUCKLE *requester;
  KNUCKLE **k_ptr;
  char msgbuf[BUF_SIZE];	      /* Message buffer. */
  char *question, *machinfo;
  int status;
  int question_len;
  char topic[TOPIC_SIZE], *text;
  int qcount = 0;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));
        
  if(!isme(request))
    {
      status = find_knuckle(&(request->target), &target);
      if (status)
	return(send_response(fd, status));
    }
  else
    target = requester;

#ifdef LOG
  (void) sprintf(msgbuf,"Attempted question from  %s [%d]",
		 target->user->username,target->instance);
  log_status(msgbuf);
#endif /* LOG */
  
  if(!(is_allowed(requester->user,OLC_ACL) && (is_me(target,requester))) &&
     !(is_allowed(requester->user,GASK_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if((has_question(target) || is_signed_on(target)) && 
     !(is_option(request->options,SPLIT_OPT)))
    {
      for (k_ptr = target->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	if((*k_ptr)->question != NULL)
	  if((*k_ptr)->question-> owner == (*k_ptr))
	    ++qcount;

      if(qcount < requester->user->max_ask)
	return(send_response(fd,HAS_QUESTION));
      else
	return(send_response(fd,MAX_ASK));
    }
  else
    if(is_option(request->options,SPLIT_OPT))
    {
#ifdef LOG
      sprintf(msgbuf,"olc ask: created new knuckle2 for %s",
              target->user->username);
      log_status(msgbuf);
#endif /* LOG */
      for (k_ptr = target->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
        if((*k_ptr)->question != NULL)
          if((*k_ptr)->question-> owner == (*k_ptr))
            ++qcount;

      if(qcount >= target->user->max_ask)
        return(send_response(fd,MAX_ASK));

      target = create_knuckle(target->user);
    }

  send_response(fd,SUCCESS);
  text =  read_text_from_fd(fd);
  if(verify_topic(text) == FAILURE)
    return(send_response(fd, INVALID_TOPIC));
  else
    if(is_option(request->options, VERIFY))
      return(send_response(fd, SUCCESS));
    else
      send_response(fd, SUCCESS);
       
  
  /* 
   * text will be overwritten on the next read
   * we need to save it so the question doesn't become the 
   * topic 
   */
  
  
  strncpy(topic,text,TOPIC_SIZE);

  text = read_text_from_fd(fd);
  if(text == (char *) NULL)
    return(send_response(fd,ERROR));
  else
    if (request->version >= VERSION_5)
      send_response(fd, SUCCESS);

  /*
   * question will be overwritten on the next read
   * Need to save away info so the question doesn't get overwritten by the
   * machine type information
   */
  
  if (request->version >= VERSION_5) {
    question_len = strlen(text);
    question = malloc(question_len+1);
    if (!question) {
      log_error("Couldn't malloc %d bytes for question");
      return(send_response(fd,ERROR));
    }
    question[question_len] = '\0';
    strcpy(question,text);
    machinfo = read_text_from_fd(fd);
  }
  else {
    question = text;
    machinfo = NULL;
  }
  init_question(target,topic,question,machinfo);
  if (request->version >= VERSION_5)
    free(question);
  set_status(target, NOT_SEEN);
  target->title = target->user->title1;

  if (!isme(request)) {
    sprintf(msgbuf,"Question asked on users behalf by %s@%s [%d].",
	    requester->user->username, requester->user->machine,
	    requester->instance);
    log_daemon(target,msgbuf);
  }

  status = match_maker(target);
  switch(status)
    {
    case CONNECTED:
      send_response(fd,CONNECTED);
      break;
    default:      /* NOT_CONNECTED */
      send_response(fd, NOT_CONNECTED);
      sprintf(msgbuf,"New \"%s\" question from %s %s [%d].",
	      topic, target->title, 
	      target->user->username,
	      target->instance);
      olc_broadcast_message("new_question",msgbuf, topic);
      break;
    }

  if (request->version >= VERSION_4)
    write_int_to_fd(fd, target->instance);

#ifdef LOG
  (void) sprintf(msgbuf,"Successful question from  %s [%d]",
		 target->user->username,target->instance);
  log_status(msgbuf);
#endif /* LOG */

  needs_backup = TRUE;
  return(SUCCESS);
}

/*
 * Function:	olc_forward() forwards a question to another consultant.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olcr.
 * Returns:	NO_SIGNED_ON:	Consultant is not signed on.
 *		NOT_CONNECTED:	Consultant is not connected to a user.
 *		SUCCESS:	Question forwarded successfully.
 * Notes:
 *	Request type is OLCR_FORWARD.  First, find the consultant and
 *	check to make sure that she is signed on and connected.  Next,
 *	if the consultant used the "unanswered" option, terminate this
 *	question as unanswerd.  Otherwise, log the fact that she is
 *	forwarding the question.  If the user has seen MAX_SEEN consultants,
 *	consider his question unanswered and dispose of it properly.
 *	Otherwise, try to find another consultant, notifying the user of
 *	any connection.  Finally, if the consultant set OFF_OPT in the
 *	request, sign her off of OLC.
 */

ERRCODE
olc_forward(fd, request)
     int fd;
     REQUEST *request;
{
    KNUCKLE *target, *requester, *consultant;
    char msgbuf[BUF_SIZE];	         /* Message buffer. */
    int  status;	                 /* Status flag */
    char *dest_q;
    
    status = find_knuckle(&(request->requester), &requester);
    if (status)
	return(send_response(fd, status));
    
    if(!isme(request)) {
	status = match_knuckle(request->target.username,
			       request->target.instance,
			       &target);
	if(status)
	    return(send_response(fd,status));
    }
    else
	target = requester;
    
    if(!(is_allowed(requester->user,GRESOLVE_ACL)) &&  
       !((is_connected_to(target, requester) && 
	  (owns_question(target)))) &&
       !(is_me(target,requester) && is_connected(target) && 
	 owns_question(target->connected)))
	return(send_response(fd, PERMISSION_DENIED));
    
    if(!is_connected(target))
	return(send_response(fd, NOT_CONNECTED));
    
    if(!has_question(target))
	return(send_response(fd, NO_QUESTION));
    
    if(target == requester) {
	if(!is_connected(requester))
	    return(send_response(fd,NOT_CONNECTED));
	target = requester->connected;
    }
    
    if(owns_question(requester))  /* cannot for own question */
	return(send_response(fd,HAS_QUESTION));
    
    needs_backup = TRUE;

    consultant = target->connected;
    if (is_option(request->options, FORWARD_UNANSWERED) ||
	target->question->nseen >= MAX_SEEN) {
	(void) sprintf(msgbuf,
		       "%s forwarding question of %s to unanswered questions log.",
		       requester->user->username, target->user->username);
	log_daemon(target, msgbuf);
	log_status(msgbuf);
	
	(void) write_message_to_user(target,
				     "You will receive a reply by mail.\n",
				     NULL_FLAG);
	(void) sprintf(target->question->title, "%s (unanswered)",
		       target->question->note[0]
		       ? target->question->note
		       : target->question->topic);
	(void) sprintf(target->question->topic, "oga");
	terminate_log_unanswered(target);
	free_new_messages(target);
	free_new_messages(consultant);
	deactivate(target);
	disconnect_knuckles(target, consultant);
	free((char *) target->question);
	target->question = (QUESTION *) NULL;
	goto handle_off;
    }
    else {	
	(void) sprintf(msgbuf, "Your question is being forwarded to another %s...\n",
		       DEFAULT_TITLE2);
	(void) write_message_to_user(target,
				     msgbuf,
				     NULL_FLAG);
    /* Writing to user may leave user status in an inactive state. */
    /* If so, don't bother trying to do match_maker. */
	if (target->user->status == ACTIVE) {
	  status = match_maker(target);
	  if(status != CONNECTED) {
	    (void) sprintf(msgbuf, "There is no %s available right now.\n",
			   DEFAULT_TITLE2);
	    (void) write_message_to_user(target,
					 msgbuf,
					 NULL_FLAG);
	  }
	}
      }
    
    set_status(target, PENDING);
    dest_q = "pending";

    /* handle status recommendations (pending/active by default)*/
    if(is_option(request->options,STATUS_REFERRED)) {
	set_status(target,REFERRED);
	dest_q = "refer";
      }
    if(is_option(request->options,STATUS_PICKUP)) {
	set_status(target,PICKUP);
	dest_q = "pickup";
      }

    sprintf(msgbuf,
	    "%s %s [%d]'s \"%s\" question forwarded to %s.",
	    target->title, 
	    target->user->username,
	    target->instance,
	    target->question->topic,
	    dest_q);
    olc_broadcast_message("forward",msgbuf,target->question->topic);

    (void) sprintf(msgbuf, "Question forwarded by %s to %s.",
		   requester->user->username, dest_q);
    log_daemon(target,msgbuf);

#ifdef LOG
    sprintf(msgbuf,"%s [%d] forwards %s [%d]",
	    requester->user->username,requester->instance,
	    target->user->username,target->instance);
    log_status(msgbuf);
#endif /* LOG */
    
    if (consultant) {
	free_new_messages(consultant);
	disconnect_knuckles(target, consultant);
    }

handle_off:
    if (!consultant)
	return SUCCESS;

    if (is_option(request->options,OFF_OPT)) {
	sign_off(consultant);
	return send_response(fd, SIGNED_OFF);
    }
    else if (is_signed_on (consultant)) {
	status = match_maker (consultant);
	return send_response (fd, status == SUCCESS ? CONNECTED : SUCCESS);
    }	
    else
	return send_response(fd, SUCCESS);
}



/*
 * Function:	olc_off() signs a consultant off of OLC.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olcr.
 * Returns:	NOT_SIGNED_ON:		Consultant is not signed on.
 *		QUESTION_PENDING:	Consultant has a pending question.
 *		SUCCESS:		Consultant successfully logged off.
 * Notes:
 *	Request type is OLCR_OFF.
 *	Look up the consultant in the list.  If she is connected to a user,
 *	don't sign her off.  Otherwise, log the sign-off and return.
 */

ERRCODE
olc_off(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *target;
  KNUCKLE *requester;
  char msgbuf[BUF_SIZE];	        /* Message buffer. */
  int status;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd, status));

  if(!isme(request))
    {        
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd, status));
    }
  else
    target = requester;

  if(!(is_me(target,requester)) &&
     !(is_allowed(requester->user,GRESOLVE_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(owns_question(target))
    return(send_response(fd, NOT_SIGNED_ON));

  if(!is_signed_on(target))
    {
      sign_off(target); /* just in case */
      return(send_response(fd, NOT_SIGNED_ON));
    }

  sign_off(target);
  if(is_connected(target))
    {
      send_response(fd, CONNECTED);
      sprintf(msgbuf, "%s %s [%d]",
	      target->connected->title,
	      target->connected->user->username,
	      target->connected->instance);
      write_text_to_fd(fd,msgbuf);
    }
  else
    if(!is_connected(target))
      {
	send_response(fd,NOT_CONNECTED);
	deactivate(target);
      }
    else
      send_response(fd,SUCCESS);

  sprintf(msgbuf,"%s %s (%s [%d]) signed off.", cap(target->title),
	  target->user->realname, target->user->username, target->instance);
  olc_broadcast_message("nol",msgbuf,"off");

#ifdef LOG
  log_status(msgbuf);
#endif /* LOG */

  needs_backup = TRUE;
  return(SUCCESS);
}

/*
* Function:	olc_send() sends a message to the user.
 * Arguments:	fd:		   File descriptor of socket.
 *		request:	   The request structure from olcr.
 * Returns:	NOT_SIGNED_ON:	   Consultant is not signed on to OLC.
 *		USER_NOT_FOUND:    Consultant is not connected to a user.
 *		SUCCESS:	   Message successfully sent.
 *              PERMISSION_DENIED: Not on access control list
 *
 * Notes:       Request type is:  OLC_SEND
 *              The target (person receiving the message) and me (the sender)
 *              are specified in the request. From the target's id the node is
 *              found which contains a pointer back to the target. The target
 *              must be the same as the sender or have THIRD_PARTY_SEND access.
 *              
 *              If all's well up to this point the message read and logged and 
 *              both connected users are notified.
 */

ERRCODE
olc_send(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE    *requester;        /* user making request */
  KNUCKLE    *target;           /* target intermediate connection */
  char       *msg;		/* Message from consultant. */
  char       mesg[BUFSIZ];
  int        status; 

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {     
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_me(requester,target)) && !(is_connected_to(requester,target)) && 
     !(is_allowed(requester->user, GMESSAGE_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if((target == requester) && is_connected(requester))
    target = requester->connected;

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

  if(is_me(requester,target) && 
     !owns_question(target) && 
     !is_connected(target))
    return(send_response(fd,NOT_CONNECTED));

  send_response(fd,SUCCESS);

  if(is_option(request->options,VERIFY))
    return(SUCCESS);

  if ((msg = read_text_from_fd(fd)) == (char *) NULL)
    return(send_response(fd, ERROR));
  
  if (target != requester)
    new_message(target, requester,  msg);

  log_message(target,requester,msg);

  if(is_me(target,requester) && !is_connected(target))
    send_response(fd, NOT_CONNECTED);
  else 
    if(is_me(target,requester))
      {
	send_response(fd, CONNECTED);
      }
    else
      send_response(fd, SUCCESS);
  
  if(owns_question(target))
    sprintf(mesg,
	    "New message from %s %s.\nTo see it, type 'show' within %s.\n",
	    requester->title, requester->user->username,DaemonInst);
  else
    sprintf(mesg,"New message from %s %s.\n",
	    requester->title, requester->user->username);

  if(target != requester)
    if (write_message_to_user(target,mesg, NULL_FLAG) != SUCCESS)
      if (owns_question(requester)) {
	  char *msg = fmt ("Unable to contact %s %s [%d].  Forwarding.",
			   target->title, target->user->username,
			   target->instance);
	  log_daemon(requester, msg);
	  free_new_messages(target);
	  set_status(requester, PENDING);
	  disconnect_knuckles(requester, target);
	  needs_backup = TRUE;
	  (void) sprintf(mesg,"The %s server could not contact the %s you were connected to.\nLooking for another %s for you....\n",
			 DaemonInst,DEFAULT_TITLE2,DEFAULT_TITLE2);
	  (void) write_message_to_user (requester,
					mesg,
					NO_RESPOND);
	  status = match_maker (requester);
	  if (status != CONNECTED) {
	      char *msg = fmt ("Question from %s %s [%d] on \"%s\" has been forwarded.",
			       requester->title,
			       requester->user->username, requester->instance,
			       requester->question->topic);
	      olc_broadcast_message ("forwarded", msg,
				     requester->question->topic);
	  }
      }


#ifdef LOG
  sprintf(mesg,"%s [%d] sends message to %s [%d]",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(mesg);
#endif /* LOG */

  if (owns_question(requester))
    {
      if (requester->status == PICKUP)
	set_status(requester, PENDING);
      if (! is_connected(target))
	{
	  sprintf(mesg,"%s %s [%d] has sent a message.\n",target->title,
		  target->user->username, target->instance);
	  olc_broadcast_message("lonely_hearts",mesg, target->question->topic);
	}
    }

  if ( (!owns_question(target) && is_connected(target) &&
	is_me(target,requester))
      ||  (owns_question(target) && is_connected_to(target,requester))
		/** ||  (is_me(target,requester) && owns_question(target)) **/
      )
    set_status(target->question->owner, SERVICED);

  return(SUCCESS);
}


/*
 * Function:	olc_comment() places a comment from the consultant into
 *			the log.
 * Arguments:	fd:		   File descriptor of socket.
 *		request:	   The request structure from olcr.
 * Returns:	NOT_SIGNED_ON:	   The consultant is not signed on to OLC.
 *		NOT_CONNECTED:	   The consultant is not connected to a user.
 *		SUCCESS:	   Comment successfully inserted into the log.
 *              PERMISSION_DENIED: Not on access controllist
 * Notes:
 *	Request type is OLC_COMMENT.
 *	Just like OLC_SEND except that it is logged as a consultant and only
 *      the overrided user is notified.
 */

ERRCODE
olc_comment(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE   *requester;	        /* user making request */
  KNUCKLE   *target;	        /* target user */
  char      *msg;		/* Message from consultant. */
  int        status;

#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!((is_me(requester,target)) &&
       (is_allowed(requester->user, CONSULT_ACL))) &&
     !(is_connected_to(requester,target)) && 
     !(is_allowed(requester->user, GCOMMENT_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

  if(is_me(requester,target) && 
     !owns_question(target) && 
     !is_connected(target))
    return(send_response(fd,NOT_CONNECTED));

  send_response(fd,SUCCESS);

  if(is_option(request->options,VERIFY))
    return(SUCCESS);

  if ((msg = read_text_from_fd(fd)) == (char *)NULL)
    return(send_response(fd, ERROR));
  
  send_response(fd, SUCCESS);

#ifdef LOG
  if(is_option(request->options,PRIV_COMMENT_OPT))
    sprintf(mesg,"%s [%d] comments privately in %s [%d]'s log",
	    requester->user->username, requester->instance,
	    target->user->username,target->instance);
  else
    sprintf(mesg,"%s [%d] comments in %s [%d]'s log",
	    requester->user->username, requester->instance,
	    target->user->username,target->instance);
  log_status(mesg);
#endif /* LOG */

  log_comment(target,requester,msg,(request->options &PRIV_COMMENT_OPT));
  return(SUCCESS);
}






ERRCODE
olc_describe(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE   *requester;	        /* user making request */
  KNUCKLE   *target;	        /* target user */
  LIST      list;
  char      *mesg;		/* Message from consultant. */
  int       status;
  
  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));
       
  if(!isme(request))
    { 
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_allowed(requester->user, MONITOR_ACL)))
    return(send_response(fd,PERMISSION_DENIED));
  
  if(is_option(request->options,(CHANGE_COMMENT_OPT | CHANGE_NOTE_OPT)))
    if(!(is_allowed(requester->user, GCOMMENT_ACL)) &&
       !((is_allowed(requester->user, CONSULT_ACL)) &&
	 is_connected_to(requester,target)) &&
       !((is_allowed(requester->user, CONSULT_ACL)) &&
	 (is_me(requester,target)) &&
	 is_connected(target)))
      return(send_response(fd,PERMISSION_DENIED));

  if(!has_question(target))
    return(send_response(fd,NO_QUESTION));

  if(!(is_option(request->options,(CHANGE_COMMENT_OPT | CHANGE_NOTE_OPT))))
    {
      send_response(fd,OK);
      get_list_info(target, &list);
      send_list(fd,request,&list);
      write_text_to_fd(fd,target->question->comment);
    }

  if(is_option(request->options,CHANGE_NOTE_OPT))
    {
      send_response(fd,SUCCESS);
      mesg = read_text_from_fd(fd);
      if(mesg != (char *) NULL)
	{
	  strncpy(target->question->note,mesg,NOTE_SIZE-1);
          target->question->note[NOTE_SIZE-1] = '\0';
	  log_description(target,requester, target->question->note);
	}
      else
	target->question->note[0] = '\0';
    }

  if(is_option(request->options,CHANGE_COMMENT_OPT))
    {
      send_response(fd,SUCCESS);
      mesg = read_text_from_fd(fd);
      if(mesg != (char *) NULL)
	{
	  strncpy(target->question->comment,mesg,COMMENT_SIZE-1);
          target->question->comment[COMMENT_SIZE-1] = '\0';
	  log_long_description(target,requester, target->question->comment);
	}
      else
	target->question->comment[0] = '\0';
    }

  write_question_info(target->question);
  return(send_response(fd,SUCCESS));
}
	




/*
 * Function:	olc_replay() replays the entire conversation.
 * Arguments:	fd:		   File descriptor of socket.
 *		request:	   The request structure from olcr.
 * Returns:	USER_NOT_FOUND:    The user does not have a pending question.
 *		NOT_SIGNED_ON:	   Consultant is not signed on to OLC.
 *		SUCCESS:	   Conversation successfully replayed.
 *              PERMISSION_DENIED: does not have access
 * Notes:
 *	Request type is OLC_REPLAY.
 *      If the requestor wishes to play a log other than his own, he must have
 *      OLC_REPLAY access. If the log is his own, his new_message buffer is zeroed.
 */

ERRCODE
olc_replay(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;	       /* Current user  making request */
  KNUCKLE *target;             /* target user */
  int status;
  int instance;
  int can_monitor;
  char censored_filename[NAME_SIZE];

#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd, status));

  if(!isme(request))
    {     
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd, status));
    }
  else
    target = requester;

  can_monitor = is_allowed(requester->user,MONITOR_ACL);

  if(!(is_me(target,requester)) &&
     !(is_connected_to(target,requester)) &&
     !(can_monitor))
    return(send_response(fd,PERMISSION_DENIED));

  if (has_question(target))
    status = SUCCESS;
  else {
    status = NO_QUESTION;
    for(instance=0;instance<target->user->no_knuckles;instance++)
      if (has_question(target->user->knuckles[instance])) {
	target = target->user->knuckles[instance];
	status = SUCCESS;
	break;
      }
  }

  send_response(fd, status);

/* If no question, return now. */
  if (status == NO_QUESTION)
    return(SUCCESS);

  if (can_monitor)
    write_file_to_fd(fd, target->question->logfile);
  else {
/* Can't monitor, so send censored version of the log */
    sprintf(censored_filename,"%s.censored",target->question->logfile);
    write_file_to_fd(fd, censored_filename);
  }    

  if((is_me(target,requester)) && 
     !(is_option(request->options, NOFLUSH_OPT)))
    {
      free_new_messages(target);
    }

#ifdef LOG
  sprintf(mesg,"%s [%d] replays %s [%d]'s log",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(mesg);
#endif /* LOG */

  return(SUCCESS);
}

/*
 * Function:	olcr_show() shows any new message from the user.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olcr.
 * Returns:	NOT_SIGNED_ON:	The consultant is not signed on to OLC.
 *		NOT_CONNECTED:	The consultant is not connected to a user.
 *		SUCCESS:	User's name successfully sent.
 * Notes:
 */

ERRCODE
olc_show(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;	            /* Current user  making request */
  KNUCKLE *target;
  int status;

#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {  
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_me(target,requester)) &&
     !(is_connected_to(target,requester)) &&
     !(is_allowed(requester->user,MONITOR_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(is_option(request->options,CONNECTED_OPT))
    {
      target = target->connected;
      if(target == (KNUCKLE *) NULL)
	return(send_response(fd,NOT_CONNECTED));
    }


  if (has_new_messages(target)) 
    {
      send_response(fd, SUCCESS);
      write_file_to_fd(fd, target->nm_file);
      if((is_me(target,requester)) && 
	 !(is_option(request->options, NOFLUSH_OPT)))
	{
	  free_new_messages(target);
	}
#ifdef LOG
      if((owns_question(requester)) && (is_me(target,requester)))
	{
	  sprintf(mesg,"%s %s read reply.", cap(requester->title),
		  requester->user->username);
	  log_daemon(requester, mesg);
	}
#endif
    }
  else
    {
      if(request->version == VERSION_3)
	{
	  send_response(fd, SUCCESS);
	  write_text_to_fd(fd, "No new messages.\n");
	}
      else
	send_response(fd,NO_MESSAGES);
    }
  
#ifdef LOG
  sprintf(mesg,"%s [%d] showing new messages of %s [%d]",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(mesg);
#endif
  return(SUCCESS);
}


/*
 * Function:	olc_list() lists active conversations.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olcr.
 * Returns:	SUCCESS:	List successfully sent.
 *		ERROR:		Unable to list conversations.
 * Notes:
 *	Loop through all of the users, putting their status information into
 *	a temporary file.  Then loop through the consultants, recording
 *	status information about the ones who are not connected (the ones
 *	who are connected would be listed with the users).  Finally, ship
 *	the list off to the consultant.  If there are no users or consultants,
 *	send back a message saying that.
 */


ERRCODE
olc_list(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  LIST *list;
  char queues[NAME_SIZE];
  char topics[NAME_SIZE];
  char name[NAME_SIZE];
  char *buf_ptr;
  int  *topicP;
  int stati;
  int topic_codes[SPEC_SIZE];
  int status;
  int n,i;

#ifdef LOG
  char mesg[BUFSIZ];
#endif

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));
  
  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      
      if(is_option(request->options,LIST_PERSONAL))
	if(status)
	  return(send_response(fd,status));
    }
  else
    target = requester;

  if(!(is_allowed(requester->user,MONITOR_ACL)) && 
     !(is_option(request->options, LIST_PERSONAL)))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  if(is_option(request->options,LIST_PERSONAL))
    status = list_user_knuckles(target,&list,&n);
  else
    {
      buf_ptr = read_text_from_fd(fd);
      if(buf_ptr != (char *) NULL)
	strncpy(queues,buf_ptr,NAME_SIZE-1);
      buf_ptr = read_text_from_fd(fd);
       if(buf_ptr != (char *) NULL)
	strncpy(topics,buf_ptr,NAME_SIZE-1);
      buf_ptr = read_text_from_fd(fd);
       if(buf_ptr != (char *) NULL)
	strncpy(name,buf_ptr,NAME_SIZE-1);
      read_int_from_fd(fd,&stati);
      send_response(fd,SUCCESS);

      if(*name == '\0')
	if(requester != target)
	  strcpy(name,request->target.username);

      topicP = (int *) NULL;      
      if(*topics != '\0')
	{
	  topic_codes[0] = verify_topic(topics);
	  topic_codes[1] = -1;
	  topicP = &topic_codes[0];
	}
      	
      status = list_queue(&list,topicP,stati,name,&n);
    }

  if(status != SUCCESS)
    n = 0;

  write_int_to_fd(fd,n);
  
  for(i=0;i<=n;i++)
    {
      status =  send_list(fd,request,&list[i]);
      if(status == ERROR)
	{
	  sprintf(mesg,
		  "Error in sending list to %s@%s.  %d of %d packets sent.",
		  requester->user->username, requester->user->machine, i, n);
	  log_error(mesg);
	  break;
	}
    }

  free((char *) list);
  return(SUCCESS);
}





/*
 * Function:	olc_topic() queries or changes the topic of a conversation.
 * Arguments:	fd:		File descriptor of socket.
 *		request:	The request structure from olcr.
 * Returns:	NOT_SIGNED_ON:	The consultant is not signed on to OLC.
 *		NOT_CONNECTED:	The consultant is not connected to a user.
 *		SUCCESS:	Topic was successfully changed.
 * Notes:
 *	Look up the consultant.  If she is connected to a user, copy
 *	a maximum of TOPIC_SIZE characters of the new topic into the
 *	user structure.
 */

ERRCODE
olc_topic(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  int status;
#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {     
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));  
    }
  else
    target = requester;

  if(!(is_me(target,requester)) && 
     !(is_connected_to(target,requester)) &&
     !(is_allowed(requester->user,MONITOR_ACL)))
    return(send_response(fd,PERMISSION_DENIED));
  
  if(!(has_question(target)))
    return(send_response(fd, NO_QUESTION));
#ifdef LOG
  sprintf(mesg,"%s [%d] topics %s [%d]",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(mesg);
#endif /* LOG */
  send_response(fd,SUCCESS);
  write_text_to_fd(fd,target->question->topic);
  return(SUCCESS);
}



ERRCODE
olc_chtopic(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  char msg_buf[BUFSIZ];
  char *text;
  int status;
  int code;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester->connected;

  if(!((is_me(requester,target) || is_connected_to(requester,target)) &&
       (is_allowed(requester->user, CONSULT_ACL))) &&
     !(is_connected_to(requester,target)) && 
     !(is_allowed(requester->user, GCHTOPIC_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

#ifdef LOG
  sprintf(msg_buf,"%s [%d] changes topic of  %s [%d]'s question",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(msg_buf);
#endif /* LOG */

  send_response(fd,SUCCESS);
  text = read_text_from_fd(fd);

  if((code = verify_topic(text)) != FAILURE)
    {
      send_response(fd,SUCCESS);
      if((owns_question(target)) && !(is_me(target,requester)))
	{
	  (void) sprintf(msg_buf,
		"%s %s has changed your question from topic '%s' to '%s'.\n",
		cap(requester->title), requester->user->username,
		target->question->topic, text);
	  (void) write_message_to_user(target,msg_buf, NULL_FLAG);
	}
      (void) sprintf(msg_buf, "Topic changed from '%s' to '%s' by %s %s.", 
		     target->question->topic, text,
		     requester->title, requester->user->username);

      (void) strncpy(target->question->topic, text, TOPIC_SIZE);
      target->question->topic_code = code;
      log_daemon(target,msg_buf);
      return(SUCCESS);
    }
  
  send_response(fd,ERROR);
  return(ERROR);
}
  


ERRCODE
olc_verify_topic(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;
  char *text;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!(is_allowed(requester->user,OLC_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  text = read_text_from_fd(fd);

  if(verify_topic(text) != FAILURE)
    return(send_response(fd,SUCCESS));
  else
    return(send_response(fd,INVALID_TOPIC));
}


        
ERRCODE
olc_list_topics(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;

#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!is_allowed(requester->user,OLC_ACL))
    return(send_response(fd,PERMISSION_DENIED));

#ifdef LOG
  sprintf(mesg,"%s [%d] lists topics",
	  requester->user->username, requester->instance);
  log_status(mesg);
#endif /* LOG */

  send_response(fd,SUCCESS);
  status = write_file_to_fd(fd,TOPIC_FILE);    
  return(status);
}



ERRCODE
olc_motd(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;
  
#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!is_allowed(requester->user,OLC_ACL))
    return(send_response(fd,PERMISSION_DENIED));

#ifdef LOG
  sprintf(mesg,"%s [%d] gets motd",
	  requester->user->username, requester->instance);
  log_status(mesg);
#endif /* LOG */

  send_response(fd,SUCCESS);
  check_motd_timeout();
  status = write_file_to_fd(fd,MOTD_FILE);    
  return(status);
}


ERRCODE
olc_get_hours(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;
  
#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!is_allowed(requester->user,OLC_ACL))
    return(send_response(fd,PERMISSION_DENIED));

#ifdef LOG
  sprintf(mesg,"%s [%d] gets hours",
	  requester->user->username, requester->instance);
  log_status(mesg);
#endif /* LOG */

  send_response(fd,SUCCESS);
  status = write_file_to_fd(fd,HOURS_FILE);
  return(status);
}


/*
 * Function:	olcr_mail() sends mail to a user.  It is called twice;
 *			once to set up a mail message and once to
 *			copy a mail message into the log.
 * Arguments:	fd:		File descriptor of a socket.
 *		request:	The request structure from olcr.
 * Returns:	An error code.
 * Notes:
 *	If NO_OPT is set, construct a mail header from the user structure
 *	and send it back to the consultant.  Otherwise, read the mail
 *	message and save it in the log.
 */

ERRCODE
olc_mail(fd, request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  KNUCKLE *target;
  char    *msgbuf;		        /* Ptr. to mail message. */
  int status;
#ifdef LOG
  char mesg[BUFSIZ];
#endif /* LOG */
  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {      
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if ( (!(is_me(requester,target)
	  || is_connected_to(requester,target)
	  || is_allowed(requester->user, GMESSAGE_ACL)))
      || owns_question(requester))
    return(send_response(fd,PERMISSION_DENIED));

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

  if(requester == target)
    target = requester->connected;

  if(is_me(requester,target) && 
     !owns_question(target) && 
     !is_connected(target))
    return(send_response(fd,NOT_CONNECTED));

  if (!(is_option(request->options, VERIFY)))
    {
      send_response(fd, SUCCESS);
      if ((msgbuf = read_text_from_fd(fd)) != (char *)NULL) 
	{
	  send_response(fd, SUCCESS);
	  log_mail(target,requester, msgbuf);
	  return(SUCCESS);
	}
      else 
	return(send_response(fd, ERROR));
    }
  else
    if(is_option(request->options,VERIFY))
      {
#ifdef LOG
  sprintf(mesg,"%s [%d] mails message to %s [%d]",
	  requester->user->username, requester->instance,
	  target->user->username,target->instance);
  log_status(mesg);
#endif /* LOG */

	send_response(fd, SUCCESS);
	return(SUCCESS);
      }
    else 
      {
	send_response(fd,ERROR);
	return(ERROR);
      }
}


/*
 * Function:	olc_startup() checks to see if a person is a person.
 *			If so, we check for a
 *			current question.
 * Arguments:	fd:		File descriptor of the socket.
 *		request:	The request structure from olc.
 * Returns:	A response code.
 * Notes:
 */

/*RESPONSE*/ ERRCODE
olc_startup(fd, request)
     int fd;			/* File descriptor for socket. */
     REQUEST *request;	        /* Request structure from olc. */
{
  KNUCKLE *requester;
  char msgbuf[BUF_SIZE];	
  int status,i,entries=0;
    
  status = find_knuckle(&(request->requester), &requester);

#ifdef LOG
  sprintf(msgbuf,"hello from %s [%d], status: %d\n", 
	 request->requester.username, request->requester.instance, status);
  log_status(msgbuf);
#endif /* LOG */

  if(status == INSTANCE_NOT_FOUND)
    return(send_response(fd,status));

  if(!(is_allowed(requester->user,OLC_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(status == SUCCESS)
    {
      if(is_connected(requester) && 0) /* this is not good */
	 {
	   send_response(fd,CONNECTED);
	   sprintf(msgbuf,"%s@%s",
		   requester->connected->user->username,
		   requester->connected->user->machine);
	   write_text_to_fd(fd,msgbuf);
	   return(SUCCESS);
	 }
      else
	{
	  for(i=0;i<requester->user->no_knuckles;i++)
	    if(is_active(requester->user->knuckles[i]))
	      entries++;
	  if(entries > 0) 
	    {
	      send_response(fd,SUCCESS);
	      write_int_to_fd(fd,entries);
	      return(SUCCESS);
	    }
	}
    }
  return(send_response(fd,USER_NOT_FOUND));
}
 

/*
 * Function:	olc_grab() grabs a pending or unseen question.
 * Arguments:	fd:		File descriptor of the socket.
 *		request:	The request structure from olcr.
 * Returns:	A response code.
 * Notes:
 */

ERRCODE
olc_grab(fd, request)
     int fd;			 /* File descriptor for socket. */
     REQUEST *request;	         /* Request structure from olc */
{
  KNUCKLE    *target;            /* User being grabbed. */
  KNUCKLE    *requester;	 
  KNUCKLE    **k_ptr;
  char msgbuf[BUF_SIZE];	         /* Message buffer. */
  int status; 
  int qcount = 0;

  status = find_knuckle(&(request->requester), &requester);
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {
      status = match_knuckle(request->target.username, 
			     request->target.instance,
			     &target);
      if(status)
	return(send_response(fd,status));
    }
  else
    target = requester;

  if(!has_question(target))
    return(send_response(fd,NO_QUESTION));

  if(!(is_allowed(requester->user,CONSULT_ACL) && 
       is_specialty(requester->user,target->question->topic_code)) &&
     !(is_allowed(requester->user,GRAB_ACL)))
    return(send_response(fd,PERMISSION_DENIED));

  if(is_connected(target))
    return(send_response(fd,ALREADY_CONNECTED));
  
  if(has_question(requester) && !(is_option(request->options,SPLIT_OPT)))
    {
      for (k_ptr = requester->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	{
	  if((*k_ptr)->question != NULL)
	    if((*k_ptr)->question->owner != (*k_ptr))
	      ++qcount;
	  else
	    if(is_signed_on((*k_ptr)))
	      ++qcount;
	}

      if(qcount < requester->user->max_answer)
	return(send_response(fd,HAS_QUESTION));
      else
	return(send_response(fd,MAX_ANSWER));
    }
  else
    if(is_option(request->options,SPLIT_OPT))
    {
#ifdef LOG
      sprintf(msgbuf,"olc grab: attempting to create new knuckle for %s",
	      requester->user->username);
      log_status(msgbuf);
#endif /* LOG */

      for (k_ptr = requester->user->knuckles; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
	if((*k_ptr)->question != NULL)
	  if((*k_ptr)->question->owner != (*k_ptr))
	    ++qcount;

      if(qcount >= requester->user->max_answer)
	return(send_response(fd,MAX_ANSWER));

      requester = create_knuckle(requester->user);
    }

  if(is_connected(requester) && !(is_option(request->options,SPLIT_OPT)))
     return(send_response(fd,CONNECTED));
  else
    if(is_option(request->options,SPLIT_OPT))
      {
#ifdef LOG
	sprintf(msgbuf,"olc grab: created new knuckle for %s",
		requester->user->username);
	log_status(msgbuf);
#endif /* LOG */
	requester = create_knuckle(requester->user);
    }

  if(target == requester)
    return(send_response(fd,GRAB_ME));

  if(!has_question(target))
    return(send_response(fd, NO_QUESTION));

  status = connect_knuckles(target,requester);
  if(status!=SUCCESS)
    return(send_response(fd,status));

  send_response(fd, SUCCESS);
  write_int_to_fd(fd,requester->instance);
  needs_backup = TRUE;

  (void) sprintf(msgbuf, "Question grabbed by %s %s@%s [%d].",
		 requester->title,requester->user->username,
		 requester->user->machine, requester->instance);
  log_daemon(target,msgbuf);
#ifdef LOG
  (void) sprintf(msgbuf, "%s [%d] grabbing user %s [%d]",
		 requester->user->username,requester->instance,
		 target->user->username, target->instance);
  log_status(msgbuf);
#endif /* LOG */

  if(target->status != NOT_SEEN)
    set_status(target, SERVICED);

  return(SUCCESS);
}
