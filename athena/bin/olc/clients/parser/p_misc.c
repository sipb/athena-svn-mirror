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
 *	$Id: p_misc.c,v 1.13 1999-07-30 18:28:47 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_misc.c,v 1.13 1999-07-30 18:28:47 ghudson Exp $";
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
  ERRCODE status = SUCCESS;

  if (fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      status = handle_common_arguments(&arguments, &Request);
      if (status != SUCCESS)
	break;
    }
  if(status != SUCCESS)
    {
      printf("Usage is: \tload [username]\n");
      return ERROR;
    }

  return t_load_user(&Request);
}

ERRCODE
do_olc_dbinfo(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status = SUCCESS;
  char file[NAME_SIZE];
  int save_file = 0;
  int change = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  make_temp_name(file);

  while(*arguments != NULL)
    {
      if(is_flag(*arguments,"-file", 2) ||
	 string_eq(*arguments, ">"))
	{
          arguments++;
          unlink(file);
          if((*arguments == NULL) || (*arguments[0] == '-'))
            {
              file[0] = '\0';
              get_prompted_input("Enter file name: ",file,NAME_SIZE,0);
              if(file[0] == '\0')
		{
		  status = ERROR;
		  break;
		}
            }
	  else
	    {
	      strcpy(file,*arguments);
	      arguments++;
	    }

          save_file = TRUE;
	  continue;
	}
      if(is_flag(*arguments,"-change",2))
	{
	  arguments++;
	  change = TRUE;
	  continue;
	}
      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)
    {
      fprintf(stderr, "Usage is: \tdbinfo [-file <filename>] "
	      "[-change]\n");
      return ERROR;
    }

  if(!change)
    status = t_dbinfo(&Request,file);
  else
    status = t_change_dbinfo(&Request);

  if(!save_file)
    unlink(file);

  return status;
}
