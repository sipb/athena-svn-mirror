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
 *	$Id: p_misc.c,v 1.12 1999-06-28 22:52:08 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_misc.c,v 1.12 1999-06-28 22:52:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_load_user(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;
  
  if (fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      arguments = handle_argument(arguments, &Request, &status);
      if (status != SUCCESS)
	return(ERROR);
            if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tload  [username]\n");
          return(ERROR);
        }

      if (*arguments == (char *) NULL)   /* end of list */
        break;
    }

  return(t_load_user(&Request));
}

ERRCODE
do_olc_dbinfo(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;            
  char file[NAME_SIZE];
  int save_file = 0;
  int change = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if(string_eq(*arguments, ">") || is_flag(*arguments,"-file", 2))
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
	    strcpy(file,*arguments);

          save_file = TRUE;
        }
      else
	if(is_flag(*arguments,"-change",2))
	  change = TRUE;
	else
	  {
	    arguments = handle_argument(arguments, &Request, &status);
          if(status != SUCCESS)
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
    unlink(file);
  return(status);
}
