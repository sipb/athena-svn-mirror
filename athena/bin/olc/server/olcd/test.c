  
ERRCODE
olc_change_acl(fd,request)
     int fd;
     REQUEST *request;
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
		{  
		  send_response(fd,USER_EXISTS);
		  goto RELOAD;
		}
	      if ((acl_add(a_ptr->file, name) <0)    /* try again */
		&& (acl_add(a_ptr->file, name) <0))  /* a bug in acl lib */
		  send_response(fd,ERROR);
	      else
		send_response(fd,SUCCESS);
	      goto RELOAD:
	    }
	  else       /* remove acl */
	    {
	      if(!acl_check(a_ptr->file, name)) /*it adds it if doesn't exist*/
		{
		  send_response(fd,USER_NOT_FOUND);
		  goto RELOAD;
		}
	      if (acl_delete(a_ptr->file, name) <0)
		&& (acl_delete(a_ptr->file, name) <0)
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
    }
  return 1;  /* is the error code ever used ? */
}


