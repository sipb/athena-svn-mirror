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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_acl.c,v 1.10 1999-07-08 22:56:50 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_acl.c,v 1.10 1999-07-08 22:56:50 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_acl(arguments)
     char **arguments;
{
  REQUEST Request;
  char acl[NAME_SIZE];
  char file[NAME_SIZE];
  int save_file = 0;
  int list_flag = 0;
  int acl_flag  = 0;
  int change_flag = 0;
  ERRCODE status = SUCCESS;

  *acl = '\0';
  make_temp_name(file);

  if(fill_request(&Request) != SUCCESS)
    return ERROR;

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if(is_flag(*arguments,"-add",2))
	{
	  arguments++;
	  acl_flag = TRUE;
	  change_flag = TRUE;	    

	  if(*arguments != NULL)
	    {
	      strncpy(acl,*arguments,NAME_SIZE);
	      arguments++;
	    }
	  continue;
	}

      if(is_flag(*arguments,"-delete",2))
	{
	  arguments++;
	  acl_flag = 0;
	  change_flag = TRUE;
	    
	  if(*arguments != NULL)
	    {
	      strncpy(acl,*arguments,NAME_SIZE);
	      arguments++;
	    }
	  continue;
	}

      if(is_flag(*arguments,"-list",2))
	{
	  list_flag = TRUE;
	  arguments++;
	  if(*arguments != NULL)
	    {
	      arguments++;
	      strncpy(acl,*arguments,NAME_SIZE);
	    }
	  continue;
	}
      
      if(!strcmp(*arguments, ">") || is_flag(*arguments,"-file",2))
	{
	  arguments++;
	  unlink(file);
          if (*arguments != NULL)
            {
	      strcpy(file, *arguments);
	      arguments++;
            }
          else
	    {
              file[0] = '\0';
              get_prompted_input("Enter a file name: ",file, NAME_SIZE,0);
              if(file[0] == '\0')
                return ERROR;
	    }
	  
          save_file = TRUE;
          continue;
        }

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
      else
	continue;
    }
  if(status != SUCCESS)   /* error */
    {
      fprintf(stderr,
	      "Usage is: \tacl [-add <list>] [-del <list>] [<username>] "
	      " [-list <list>]\n\t\t[-list <username>] [-file <filename>]\n");
      return ERROR;
    }
    
  if((*acl == '\0') && !list_flag)
    {
      get_prompted_input("Enter access control list: ",acl, NAME_SIZE,0);
      if(acl[0] == '\0')
	return ERROR;
    }
    
  if(list_flag && !change_flag)
    {
      if(acl[0] != '\0')
	status = t_list_acl(&Request, acl, file);
      else
	status = t_get_accesses(&Request,file);
    }
  else
    status = t_set_acl(&Request,acl,acl_flag);

  if((status != SUCCESS) || (!save_file))
    unlink(file);

  return(status);
}


