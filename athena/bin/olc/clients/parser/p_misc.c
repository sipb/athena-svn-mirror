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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_misc.c,v $
 *	$Id: p_misc.c,v 1.8 1992-01-10 20:00:23 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_misc.c,v 1.8 1992-01-10 20:00:23 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_load_user(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
            if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tload  [username]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  return(t_load_user(&Request));
}

ERRCODE
do_olc_dump(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;            
  char file[NAME_SIZE];
  int save_file = 0;
  int type = OLC_DUMP;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if(string_eq(*arguments, ">") || string_equiv(*arguments,"-file",
                                                    max(strlen(*arguments),2)))
        {
          ++arguments;
          unlink(file);
          if(*arguments == NULL)
            {
              file[0] = '\0';
              get_prompted_input("Enter file name: ",file,NAME_SIZE,0);
              if(file[0] == '\0')
                return(ERROR);
            }
	  else
	    (void) strcpy(file,*arguments);

          save_file = TRUE;
	  continue;
        }

      if(string_equiv(*arguments,"-requests",max(strlen(*arguments),2)))
	{
	  type = OLC_DUMP_REQ_STATS;
	  continue;
	}

      if(string_equiv(*arguments,"-questions",max(strlen(*arguments),2)))
	{
	  type = OLC_DUMP_QUES_STATS;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tdump  [-requests] [-questions]");
	  printf(" [-file <filename>]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  status = t_dump(&Request,type,file);
    if(!save_file)
    (void) unlink(file);
  return(status);
}

ERRCODE
do_olc_dbinfo(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;            
  char file[NAME_SIZE];
  int save_file = 0;
  int change = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if(string_eq(*arguments, ">") || string_equiv(*arguments,"-file",
                                                    max(strlen(*arguments),2)))
        {
          ++arguments;
          unlink(file);
          if(*arguments == NULL)
            {
              file[0] = '\0';
              get_prompted_input("Enter file name: ",file,NAME_SIZE,0);
              if(file[0] == '\0')
                return(ERROR);
            }
	  else
	    (void) strcpy(file,*arguments);

          save_file = TRUE;
        }
      else
	if(string_equiv(*arguments,"-change",max(strlen(*arguments),2)))
	  change = TRUE;
	else
	  {
	    arguments = handle_argument(arguments, &Request, &status);
          if(status)
            return(ERROR);
	  }

      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tdbinfo [-file <filename>] ");
	  printf("[-change]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  if(!change)
    status = t_dbinfo(&Request,file);
  else
    status = t_change_dbinfo(&Request);

  if(!save_file)
    (void) unlink(file);
  return(status);
}
