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
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: requests_admin.c,v 1.30 1999-06-28 22:52:42 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: requests_admin.c,v 1.30 1999-06-28 22:52:42 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olcd.h>

ERRCODE
olc_load_user(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester,*target;
  USER *user;
  int status;

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));

  if(!isme(request)) {
    status = find_knuckle(&(request->target), &target);
    if(status)
      return(send_response(fd,status));
    user = target->user;
  }
  else 
    user = requester->user;

  if (!is_allowed(requester->user,ADMIN_ACL)
      && !isme (request))
    return(send_response(fd,PERMISSION_DENIED));

  load_user(user);
  
  send_response(fd,SUCCESS);
  return(SUCCESS);
}


ERRCODE
olc_dump(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;
  char file[NAME_SIZE];

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));
   
  if(!is_allowed(requester->user, ADMIN_ACL))
    return(send_response(fd,PERMISSION_DENIED));
  
  send_response(fd,SUCCESS);
  make_temp_name(file);
  dump_data(file);
  write_file_to_fd(fd,file);
  unlink(file);
  return(SUCCESS);
}

ERRCODE
olc_change_motd(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));
   
  if(!is_allowed(requester->user, MOTD_ACL))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  if(is_option(request->options, VERIFY))
    return(status);

  status = send_response(fd,read_text_into_file(fd,MOTD_FILE));

  log_motd(requester->user->username);
  set_motd_timeout(requester);

  return(status);
}


ERRCODE
olc_change_hours(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int status;

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));
   
  if(!is_allowed(requester->user, ADMIN_ACL))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  if(is_option(request->options, VERIFY))
    return(status);

  status = send_response(fd,read_text_into_file(fd,HOURS_FILE));
  return(status);
}

  
ERRCODE
olc_change_acl(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  USER *user = (USER *) NULL;
  ACL *a_ptr;
  char tmpfile[MAXPATHLEN];
  char *acl;
  char *name;
  int status;

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));
  
  status = get_user(&(request->target), &user);
  if(status)
      name = request->target.username;
  else
    name = user->username;

  if(!is_allowed(requester->user,ADMIN_ACL))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);

  acl = read_text_from_fd(fd);
  if(acl == (char *) NULL)
    return(send_response(fd,ERROR));

#ifdef OLCD_LOG_ACTIONS
  log_admin("%s changing %s acl for %s", request->requester.username, 
	    acl, name);
#endif /* OLCD_LOG_ACTIONS */

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(string_eq(a_ptr->name,acl))
	{
	  if(is_option(request->options, ADD_OPT))
	    {
	      if(acl_check(a_ptr->file, name))   /* acl_add won't tell us */
		{  
		  send_response(fd,USER_EXISTS);
		  goto RELOAD;
		}
	      if ((acl_add(a_ptr->file, name) <0)    /* try again */
		&& (acl_add(a_ptr->file, name) <0))  /* a bug in acl lib */
		  send_response(fd,ERROR);
	      else
		send_response(fd,SUCCESS);
	      goto RELOAD;
	    }
	  else       /* remove acl */
	    {
	      if(!acl_check(a_ptr->file, name)) /*it adds it if doesn't exist*/
		{
		  send_response(fd,USER_NOT_FOUND);
		  goto RELOAD;
		}
	      if ((acl_delete(a_ptr->file, name) <0)
		&& (acl_delete(a_ptr->file, name) <0))
		  send_response(fd,ERROR);
	      else
		send_response(fd,SUCCESS);
	      goto RELOAD;
	    }
	}
    }
  send_response(fd,UNKNOWN_ACL);

 RELOAD:    /* no matter what reload the permissions from file */

  if (user != (USER *) NULL)
    {
      user->permissions = 0;
      get_acls(user);

      /* If a user has NO permissions (not even ask), that's probably a bug. */
      if (user->permissions == 0)
	{
	  char *realm = user->realm;

	  if (!realm || !realm[0])
	    realm = "???";

	  log_error("alert: all permissions (including ask) "
		    "for user '%s@%s' have been removed via 'olcr acl'!",
		    user->username, realm);

	  strcpy(tmpfile, OLXX_SPOOL_DIR "/perm_corrupt_change_XXXX");
	  mktemp(tmpfile);
	  dump_data(tmpfile);
	}
    }
  return 1;  /* is the error code ever used ? */
}

ERRCODE
olc_list_acl(fd, request)
     int fd;                    /* File descriptor for socket. */
     REQUEST *request;          /* Request structure from olcr. */
{
  KNUCKLE *requester;
  ACL *a_ptr;
  char *acl;
  int status;

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));

  if(!is_allowed(requester->user, ADMIN_ACL))
     return(send_response(fd,PERMISSION_DENIED));
  
  send_response(fd,SUCCESS);

  acl = read_text_from_fd(fd);
  if(acl == (char *) NULL)
    return(send_response(fd,ERROR));

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    if(string_eq(a_ptr->name,acl))
      {
	send_response(fd,SUCCESS);
	write_file_to_fd(fd,a_ptr->file);
	return(SUCCESS);
      }

   return(send_response(fd,UNKNOWN_ACL));
}

ERRCODE
olc_get_accesses(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  USER *user, u;
  char file[NAME_SIZE];
  FILE *fp;
  ACL *a_ptr;
  int status;

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));

  if(!is_allowed(requester->user, ADMIN_ACL) &&
     !isme(request))
     return(send_response(fd,PERMISSION_DENIED));

  if(!isme(request))
    {
      status = get_user(&(request->target), &user);
      if(status)
	{
	  strcpy(u.username, request->target.username);
#ifdef HAVE_KRB4
	  strcpy(u.realm, request->target.realm);
#else /* not HAVE_KRB4 */
	  strcpy(u.realm, DFLT_SERVER_REALM);
#endif /* not HAVE_KRB4 */
	  user = &u;    
	  init_dbinfo(user);
	}
    }
  else
    user = requester->user;

  send_response(fd,SUCCESS);
  make_temp_name(file);
  fp = fopen(file, "w");
  if(fp == NULL)
    return(send_response(fd,ERROR));

  fprintf(fp,"Accesses: \n");
  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(user->permissions & a_ptr->code)
        fprintf(fp,"\t%s\n",a_ptr->name);
    }
  fclose(fp);

  write_file_to_fd(fd,file);
  (void) unlink(file);

  return(SUCCESS);
}


ERRCODE
olc_get_dbinfo(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester,*target;
  USER *user;
  DBINFO dbinfo;
  int status;

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));

  if(!is_allowed(requester->user, ADMIN_ACL) &&
     !isme(request))
     return(send_response(fd,PERMISSION_DENIED));

  if(!isme(request)) {
    status = find_knuckle(&(request->target), &target);
    if(status)
      return(send_response(fd,status));
    user = target->user;
  }
  else 
    user = requester->user;

  send_response(fd,SUCCESS);

  strcpy(dbinfo.title1, user->title1);
  strcpy(dbinfo.title2, user->title2);
  dbinfo.max_ask = user->max_ask;
  dbinfo.max_answer = user->max_answer;

  send_dbinfo(fd,&dbinfo);
  return(SUCCESS);
}


ERRCODE
olc_change_dbinfo(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  USER *user, u;
  int status;
  DBINFO dbinfo;

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));

  if(!is_allowed(requester->user, ADMIN_ACL) &&
     !isme(request))
     return(send_response(fd,PERMISSION_DENIED));

  if(!isme(request))
    {
      status = get_user(&(request->target), &user);
      if(status)
	{
	  strcpy(u.username, request->target.username);
#ifdef HAVE_KRB4
	  strcpy(u.realm, request->target.realm);
#else /* not HAVE_KRB4 */
	  strcpy(u.realm, DFLT_SERVER_REALM);
#endif /* not HAVE_KRB4 */
	  user = &u;    /* local init for now */
	  init_dbinfo(user);
	}
    }
  else
    user = requester->user;

  send_response(fd,SUCCESS);
  status = read_dbinfo(fd, &dbinfo);
  if(status == SUCCESS)
    {
      user->max_ask = dbinfo.max_ask;
      user->max_answer = dbinfo.max_answer;
      strcpy(user->title1, dbinfo.title1);
      strcpy(user->title2, dbinfo.title2);
      status = save_user_info(user);
    }
  return(send_response(fd,status));
}

ERRCODE
olc_set_user_status(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester,*target;
  USER *user;
  int status;
  char message[BUF_SIZE];

  status = find_knuckle(&(request->requester), &requester);
  if (status != SUCCESS)
    return(send_response(fd,status));
  if (!is_allowed(requester->user, ADMIN_ACL))
    return(send_response(fd,PERMISSION_DENIED));

  status = get_knuckle(request->target.username, request->target.instance,
		       &target,0);
  if (status != SUCCESS)
    return(send_response(fd,status));

  user = target->user;
  if (target->question != NULL) {
    /* make sure they still have a question- */
    switch(request->options) {
    case ACTIVE:
      if (!(user->status & ACTIVE)) {
	sprintf(message,"%s %s has logged back in.",
		cap(target->title), user->username);
	log_daemon(target,message);
#ifdef OLCD_LOG_ACTIONS
	log_status(message);
#endif /* OLCD_LOG_ACTIONS */
	
	strcat(message, "\n");
	olc_broadcast_message("resurrection",message,
			      target->question->topic);
	set_status(user,ACTIVE);
      }
      break;
    case LOGGED_OUT:
      if (!(user->status & LOGGED_OUT)) {
	sprintf(message,"%s %s has logged out.",
		cap(target->title), user->username);
	log_daemon(target,message);
#ifdef OLCD_LOG_ACTIONS
	log_status(message);
#endif /* OLCD_LOG_ACTIONS */
	
	strcat(message, "\n");
	olc_broadcast_message("aloha",message, target->question->topic);
	set_status(user,LOGGED_OUT);
      }
      break;
    case MACHINE_DOWN:
      if (!(user->status & MACHINE_DOWN)) {
	sprintf(message,"%s %s machine is down.",
		cap(target->title), user->username);
	log_daemon(target,message);
#ifdef OLCD_LOG_ACTIONS
	log_status(message);
#endif /* OLCD_LOG_ACTIONS */
	
	set_status(user,MACHINE_DOWN);
      }
      break;
    default:
      send_response(fd,UNKNOWN_REQUEST);
      return(PERMISSION_DENIED);
    }
  }
  send_response(fd,SUCCESS);
  needs_backup = TRUE;
  return(SUCCESS);
}

#ifdef HAVE_ZEPHYR
ERRCODE
olc_toggle_zephyr(fd,request)
     int fd;
     REQUEST *request;
{
  KNUCKLE *requester;
  int punt_time,status;
  char buf[BUFSIZ];

  status = find_knuckle(&(request->requester), &requester);
  if(status != SUCCESS)
    return(send_response(fd,status));
  
  if(!is_allowed(requester->user, ADMIN_ACL))
    return(send_response(fd,PERMISSION_DENIED));

  send_response(fd,SUCCESS);
  if (request->options & OFF_OPT) {
    read_int_from_fd(fd,&punt_time);
    if (punt_time == -1)
      punt_time = ZEPHYR_PUNT_TIME;
    if (punt_time == 0) {
      sprintf(buf,"Disabling zephyr indefinitely");
    } else {
      sprintf(buf,"Disabling zephyr for %d minutes",punt_time);
    }
    log_status(buf);
    olc_broadcast_message("syslog",buf, "system");
    toggle_zephyr(1,punt_time);
  } else {
    sprintf(buf,"Attempting to enable zephyr");
    log_status(buf);
    toggle_zephyr(0,0);
    olc_broadcast_message("syslog",buf, "system");
  }
  needs_backup = FALSE;
  return(SUCCESS);
}
#endif /* HAVE_ZEPHYR */