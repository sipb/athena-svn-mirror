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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/requests_admin.c,v $
 *	$Id: requests_admin.c,v 1.13 1990-12-09 16:59:37 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/requests_admin.c,v 1.13 1990-12-09 16:59:37 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

extern ACL  Acl_List[];

ERRCODE
olc_load_user(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
{
  KNUCKLE *requester;
  USER *user;
  int status;

  status = find_knuckle(&(request->requester), &requester);	
  if(status)
    return(send_response(fd,status));

  if(!isme(request))
    {
      status = get_user(&(request->target),&user);	
      if(status)
	return(send_response(fd,status));
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
olc_dump(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
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
olc_dump_req_stats(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
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
  dump_request_stats(file);
  write_file_to_fd(fd,file);
  unlink(file);
  return(SUCCESS);
}


ERRCODE
olc_dump_ques_stats(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
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
  dump_question_stats(file);
  write_file_to_fd(fd,file);
  unlink(file);
  return(SUCCESS);
}


ERRCODE
olc_change_motd(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
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
  set_motd_timeout(requester);
  return(status);
}


ERRCODE
olc_change_acl(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
{
  KNUCKLE *requester;
  USER *user = (USER *) NULL;
  ACL *a_ptr;
  char *acl;
  char *name;
  int status;

#ifdef LOG
  char mesg[BUF_SIZE];
#endif /* LOG */

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

#ifdef LOG
  sprintf(mesg,"%s changing %s acl for %s", request->requester.username, 
	  acl, name);
  log_admin(mesg);
#endif /* LOG */

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(string_eq(a_ptr->name,acl))
	{
	  if(is_option(request->options, ADD_OPT))
	    {
	      if(acl_check(a_ptr->file, name))   /* acl_add won't tell us */
		  return(send_response(fd,USER_EXISTS));
	      if(user != (USER *) NULL)
		user->permissions |= a_ptr->code;
	      if(acl_add(a_ptr->file, name) <0)    /* try again */
		if(acl_add(a_ptr->file, name) <0)  /* a bug in acl lib */
		  return(send_response(fd,ERROR));
	      return(send_response(fd,SUCCESS));
	    }
	  else
	    {
	      if(!acl_check(a_ptr->file, name)) /*it adds it if doesn't exist*/
		return(send_response(fd,USER_NOT_FOUND));
	      if(user != (USER *) NULL)
		if(user->permissions & a_ptr->code)
		  user->permissions |= ~(a_ptr->code);
	      if(acl_delete(a_ptr->file, name) <0)
		if(acl_add(a_ptr->file, name) <0)
		  return(send_response(fd,ERROR));
	      return(send_response(fd,SUCCESS));
	    }
	}
    }
  return(send_response(fd,UNKNOWN_ACL));
}

  

ERRCODE
olc_list_acl(fd, request, auth)
     int fd;                    /* File descriptor for socket. */
     REQUEST *request;          /* Request structure from olcr. */
     int auth;                  /* indicates if requestor was authenticated */
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
olc_get_accesses(fd,request,auth)  
     int fd;
     REQUEST *request;
     int auth;
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
	  strcpy(u.realm, request->target.realm);
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
olc_get_dbinfo(fd,request,auth)  
     int fd;
     REQUEST *request;
     int auth;
{
  KNUCKLE *requester;
  USER *user, u;
  DBINFO dbinfo;
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
	  strcpy(u.realm, request->target.realm);
	  user = &u;    /* local init for now */
	  init_dbinfo(user);
	}
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
olc_change_dbinfo(fd,request,auth)
     int fd;
     REQUEST *request;
     int auth;
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
	  strcpy(u.realm, request->target.realm);
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
