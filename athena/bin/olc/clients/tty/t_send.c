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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_send.c,v $
 *	$Id: t_send.c,v 1.16 1991-01-21 17:11:32 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_send.c,v 1.16 1991-01-21 17:11:32 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


ERRCODE
t_reply(Request,file,editor)    
     REQUEST *Request;
     char *file, *editor;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to send to %s [%d].\n",
	      Request->target.username,
	      Request->target.instance);
      status = NO_ACTION;
      break;

    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);

  status = enter_message(file,editor);
  if(status)
    return(status);
  
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("Message sent. \n");
      break;

    case NOT_CONNECTED:
      printf("You are not currently connected to a consultant but the next one\n");
      printf("available will receive your message.\n");
      status = ERROR;
      break;

    case CONNECTED:
      printf("Message sent.\n");
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are no longer allowed to send to %s [%d].\n",
	      Request->target.username,
	      Request->target.instance);
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;

    default:
      status =  handle_response(status, Request);
      break;
    }
  
  return(status);
}






ERRCODE
t_comment(Request,file,editor)    
     REQUEST *Request;
     char *file, *editor;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OComment(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,  "You are not allowed to comment in %s [%d]'s log.\n",
	      Request->target.username,
	      Request->target.instance);
      status = NO_ACTION;
      break;

    case ERROR:
      fprintf(stderr, "Unable to log comment.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);
  
  status = enter_message(file,editor);
  if(status)
    return(status);

  status = OComment(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("Comment Logged. \n");
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,
	      "You are no longer allowed to comment in %s [%d]'s log.\n",
	      Request->target.username,
	      Request->target.instance);
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "An error has occurred while logging comment.  Try resending.\n");
      status = ERROR;
      break;

    default:
      status =  handle_response(status, Request);
      break;
    }  

  return(status);
}








ERRCODE
t_mail(Request,file,editor,smargs, check,noedit, header)
     REQUEST *Request;
     char *file, *editor;
     char **smargs;
     int check;
     int noedit;
     int header;
{
  int status;
  LIST list;
  struct stat statbuf;
  char buf[BUF_SIZE];
  char tmp_file[NAME_SIZE];
  char *username;
  char *realname;
  char *message;

  set_option(Request->options, VERIFY);
  status = OMail(Request,file);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case SUCCESS:
      status = OWho(Request,&list);
      if(status != SUCCESS)
	{
	  fprintf(stderr,
		"warning: Unable to get status of conversation... exiting\n");
	  return(ERROR);
	}
  
      if(isme(Request))
	{
	  username = list.connected.username;
	  realname = list.connected.realname;
	}
      else
	{
	  username = Request->target.username;
	  realname = Request->target.realname;
	}

      if(check)
	if(can_receive_mail(username) != SUCCESS)
	  {
	    printf("%s is not registered with local mail server.\n",username);
	    get_prompted_input("continue? ",buf);
	    if(buf[0]!='y')
	      return(ERROR);
	  }

      if(isme(Request))
	set_option(Request->options, CONNECTED_OPT);
      
      status = OShowMessage(Request,&message);
      if ((status != SUCCESS) && (status != NO_MESSAGES))
	{
	  handle_response(status,Request);
	}

      if ((status == NO_MESSAGES) ||
	  (!strncmp(message,"No new messages.", 16)))
	message = (char *) NULL;

      make_temp_name(tmp_file);

      if (header)
	(void) OMailHeader(Request, tmp_file, username, realname,
			   list.topic, DEFAULT_MAILHUB, message);

      if (file[0] != '\0') {
	int fd1, fd2;
	char *tmp;

	fd1 = open(tmp_file,O_WRONLY|O_CREAT,0644);
	fd2 = open(file,O_RDONLY,0644);
	if (fd2 < 0) {
	  fprintf(stderr,"No such file %s. . . aborting\n",file);
	  close(fd1);
	  return(ERROR);
	}
	lseek(fd1,0,L_XTND);
	fstat(fd2,&statbuf);
	tmp = malloc(statbuf.st_size);
	read(fd2,tmp,statbuf.st_size);
	write(fd1,tmp,statbuf.st_size);
	close(fd1);
	close(fd2);
      }
	
      if (!noedit) {
	status = edit_message(tmp_file,editor);
	if(status == ERROR)
	  {
	    fprintf(stderr,"Error editing file. . . aborting\n");
	    return(status);
	  }
	else  
	  if(status)
	    return(status);
      
	status = what_now(tmp_file,FALSE,NULL);
	if(status == ERROR)
	  {
	    fprintf(stderr,"An error occurred while reading your");
	    fprintf(stderr," message; unable to continue.\n");
	    unlink(file);
	    return(ERROR);
	  }
	else
	  if(status)
	    return(status);
      }

      (void) stat(tmp_file, &statbuf);
      if (statbuf.st_size == 0) 
	{
	  printf("No message to send.\n");
	  return(NO_ACTION);
	}
      if (mail_message(username, Request->requester.username,tmp_file, smargs)
	  == SUCCESS)
	printf("Mail message sent.\n");
      else 
	{
	  printf("Error sending mail.\n");
	  return(ERROR);
	}
      break;

    case NOT_CONNECTED:
      printf("You are not connected.\n");
      return(ERROR);
    default:
      status = handle_response(status, Request);
      return(status);
    }

  status = OMail(Request,tmp_file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    default:
      status = handle_response(status, Request);
      return(status);
    }  

  unlink(tmp_file);
  return(status);
}
