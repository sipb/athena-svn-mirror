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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_acl.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_acl.c,v 1.1 1989-11-17 14:09:46 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

ERRCODE
t_set_acl(Request,acl,flag)     
     REQUEST *Request;
     char *acl;
     int flag;                  /* flag == TRUE, add */
{
  int status;

  if(flag)
    set_option(Request->options,ADD_OPT);
  else
    set_option(Request->options,DEL_OPT);

  status = OSetAcl(Request,acl);

  switch(status)
    {
    case SUCCESS:
      printf("Access change successful.\n");
      break; 

    case UNKNOWN_ACL:
      fprintf(stderr, 
	      "No such access list, \"%s\"\n",acl);
      return(ERROR);
      break;

    case ERROR:
      fprintf(stderr, "An error has occurred while setting acl.\n");
      return(ERROR);
      break;

    default:
      if((status = handle_response(status, Request))!=SUCCESS)
	return(ERROR);
      break;
    }

  return(SUCCESS);
}


t_list_acl(Request, acl, file)
     REQUEST *Request;
     char *acl;
     char *file;
{
  int status;

  status = OListAcl(Request, acl, file);
  switch(status)
    {
    case SUCCESS:
      display_file(file,TRUE);
      break;

    case ERROR:
      fprintf(stderr,"Cannot list acl \"%s\".\n", acl);
      status = ERROR;
      break;

   case  UNKNOWN_ACL:
      fprintf(stderr, "Unknown acl \"%s\".\n", acl);
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
   }
  return(status);
}


t_get_accesses(Request,file)
     REQUEST *Request;
     char *file;
{
  int status;

  status = OGetAccesses(Request,file);
  switch(status)
    {
    case SUCCESS:
      display_file(file,TRUE);
      break;

    case ERROR:
      fprintf(stderr,"Cannot list acccess for \"%s\".\n", 
	      Request->target.username);
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
    }

  return(status);
}
