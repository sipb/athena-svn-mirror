/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains miscellaneous utilties for the olc and olcr programs.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_utils.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_utils.c,v 1.2 1989-07-13 12:11:23 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>


char **
handle_argument(args, req, status)
     char **args;
     REQUEST *req;
     int *status;
{
  
  *status = SUCCESS;

  if (string_equiv(args[0], "-instance",strlen(args[0]))) 
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	req->requester.instance = atoi(*args);
	return(args);
      }
    else
      {
	fprintf(stderr,"You must specify an instance after '-i'.\n");
        *status = ERROR;
	return((char **) NULL);
      }
  else  if (string_equiv(args[0], "-j",2)) 
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	req->target.instance = atoi(*args);
	return(args);
      }
    else
      {
	fprintf(stderr,"You must specify an instance after '-j'.\n");
	*status = ERROR;
	return((char **) NULL);
      }
  else if (*args[0] == '-')
    {
      fprintf(stderr, "Unknown option %s\n", *args);
      return((char **) NULL);
    }
  else if(string_equiv(args[0],"-help",max(strlen(args[0]),2)))
         return((char **) NULL);
  else          
    if(*args)
	{
	  (void) strcpy(req->target.username,*args);
	  if((*(++args) != (char *) NULL) && (*args[0] != '-'))
	    req->target.instance = atoi(*args); 	 
	  else
	    req->target.instance = NO_INSTANCE; 	 
	} 
  return(args);
}



fill_request(req)
     REQUEST *req;
{
  int status;

#ifdef KERBEROS
    int result;
#endif KERBEROS

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  req->requester.instance = User.instance;
  req->requester.uid      = User.uid;
  (void) strncpy(req->requester.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->requester.realname, User.realname, NAME_LENGTH);
  (void) strncpy(req->requester.machine,  User.machine,  NAME_LENGTH);
  
  req->target.instance = User.instance;
  req->target.uid      = User.uid;
  (void) strncpy(req->target.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->target.realname, User.realname, NAME_LENGTH);
  (void) strncpy(req->target.machine,  User.machine,  NAME_LENGTH);

#ifdef KERBEROS
  result = krb_mk_req(&(req->kticket), K_SERVICE, INSTANCE, REALM, 0);

#ifdef TEST
  printf("kerberos request: %d, size %d\n",result, req->kticket.length);
#endif TEST
  
  switch(result)
    {
    case SUCCESS:
      status = SUCCESS;
      break;
    default:
      status = handle_response(result,req);
      break;
    }
  return(status);
#else KERBEROS

status = SUCCESS;
return(status);

#endif KERBEROS

}
