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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_acl.c,v $
 *	$Id: p_acl.c,v 1.5 1990-11-14 12:22:30 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_acl.c,v 1.5 1990-11-14 12:22:30 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
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
  int status;

  *acl = '\0';
  make_temp_name(file);

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_equiv(*arguments,"-add",max(strlen(*arguments),2)))
	  {
	    ++arguments;
	    acl_flag = TRUE;
	    change_flag = TRUE;	    

	    if(*arguments != (char *) NULL)
	      strncpy(acl,*arguments,NAME_SIZE);
	    continue;
	  }

       if(string_equiv(*arguments,"-delete",max(strlen(*arguments),2)))
	  {
	    ++arguments;
	    acl_flag = 0;
	    change_flag = TRUE;
	    
	    if(*arguments != (char *) NULL)
	      strncpy(acl,*arguments,NAME_SIZE);
	    continue;
	  }

      if(string_equiv(*arguments,"-list",max(strlen(*arguments),2)))
	{
	  list_flag = TRUE;
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    strncpy(acl,*arguments,NAME_SIZE);
	    
	  continue;
	}

       if(!strcmp(*arguments, ">") ||
         string_equiv(*arguments,"-file",max(strlen(*arguments),2)))
        {
          ++arguments;
          unlink(file);
          if (*arguments == (char *)NULL)
            {
              file[0] = '\0';
              get_prompted_input("Enter a file name: ",file);
              if(file[0] == '\0')
                return(ERROR);
            }
          else
            (void) strcpy(file, *arguments);

          save_file = TRUE;
          continue;
        }

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      else
	list_flag = TRUE;
    
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tacl [-add <list>] [-del <list>] [<username>] ");
	  printf(" [-list <list>]\n\t\t[-list <username>] [-file <filename>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  if((*acl == '\0') && !list_flag)
    {
      get_prompted_input("Enter access control list: ",acl);
      if(acl[0] == '\0')
	return(ERROR);
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


