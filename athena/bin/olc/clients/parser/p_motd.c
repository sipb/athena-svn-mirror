/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *	$Id: p_motd.c,v 1.23 1999-07-30 18:29:08 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_motd.c,v 1.23 1999-07-30 18:29:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_motd(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  ERRCODE status = SUCCESS;
  int save_file = 0;
  int type=0;
  int change_flag = 0;
  int clear_flag = 0;
  char editor[NAME_SIZE];

  file[0] = '\0';
  editor[0] = '\0';

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

      if (is_flag(*arguments, "-editor",2))
	{
          arguments++;
          if((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      strcpy(editor, *arguments);
	      arguments++;
	    }
          else
	    {
	      strcpy(editor, NO_EDITOR);
	    }
	  continue;
	}

      if(is_flag(*arguments,"-change", 2))
	{
          arguments++;
	  change_flag = TRUE;
	  continue;
	}

      if (is_flag(*arguments,"-clear", 2))
	{
	  arguments++;
	  clear_flag = TRUE;
	  continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
	
    }

  if(status != SUCCESS)
    {
      fprintf(stderr,"Usage is: \tmotd [-file <filename>] [-change] "
	      "[-editor <editor>] [-clear]\n");
      return ERROR;
    }

  if(!(change_flag || clear_flag))
    {
      Request.request_type = OLC_MOTD;
      status = t_get_file(&Request,type,file,!save_file);
    }
  else
    {
      Request.request_type = OLC_CHANGE_MOTD;
      status = t_change_file(&Request,type,file,editor,
			     (!save_file ? OLC_MOTD : 0), clear_flag); 
    }

  if(!save_file)
    unlink(file);

  return status;
}

ERRCODE
do_olc_hours(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  ERRCODE status = SUCCESS;
  int save_file = 0;
  int type=0;
  int change_flag = 0;
  char editor[NAME_SIZE];

  file[0] = '\0';
  editor[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return ERROR;  

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

      if (is_flag(*arguments, "-editor", 2))
	{
          arguments++;
          if((*arguments != NULL) || (*arguments[0] != '-'))
	    {
	      strcpy(editor, *arguments);
	      arguments++;
	    }
          else
	    {
	      strcpy(editor, NO_EDITOR);
	    }
	  continue;
	}

      if(is_flag(*arguments,"-change", 2))
	{
          arguments++;
	  change_flag = TRUE;
	  continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)
    {
      fprintf(stderr,
	      "Usage is: \thours  [-file <filename>] [-change] "
	      "[-editor <editor>]\n");
      return ERROR;
    }

  if(!change_flag)
    {
      Request.request_type = OLC_GET_HOURS;
      status = t_get_file(&Request,type,file,!save_file);
    }
  else
    {
      Request.request_type = OLC_CHANGE_HOURS;
      status = t_change_file(&Request,type,file,editor,
			     (!save_file ? OLC_GET_HOURS : 0),0);
    }

  if(!save_file)
    unlink(file);

  return status;
}
