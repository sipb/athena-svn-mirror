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
 *	$Id: p_motd.c,v 1.22 1999-06-28 22:52:08 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_motd.c,v 1.22 1999-06-28 22:52:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

ERRCODE
do_olc_motd(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  ERRCODE status;
  int save_file = 0;
  int type=0;
  int change_flag = 0;
  int clear_flag = 0;
  char editor[NAME_SIZE];

  strcpy(file, "");
  strcpy(editor, "");

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);

  arguments++;
  while(*arguments != (char *) NULL)
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
	  else {
	    strcpy(file,*arguments);
	    arguments++;
	  }
	  
	  save_file = TRUE;
	  continue;
	}

      if (is_flag(*arguments, "-editor",2))
        {
          ++arguments;
          if(*arguments != (char *) NULL) {
            strcpy(editor, *arguments);
	    arguments++;
	  }
          else
            strcpy(editor, NO_EDITOR);
	  continue;
        }

      if(is_flag(*arguments,"-change", 2))
	{
          ++arguments;
	  change_flag = TRUE;
	  continue;
	}

      if (is_flag(*arguments,"-clear", 2))
	{
	  ++arguments;
	  clear_flag = TRUE;
	  continue;
	}
      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);
	
      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tmotd  [-file <filename>] [-change] ");
	  printf("[-editor <editor>] [-clear]\n");
	  return(ERROR);
	}
    }

  if(!(change_flag || clear_flag)) {
    Request.request_type = OLC_MOTD;
    status = t_get_file(&Request,type,file,!save_file);
  }
  else {
    Request.request_type = OLC_CHANGE_MOTD;
    status = t_change_file(&Request,type,file,editor,
			   (!save_file ? OLC_MOTD : 0), clear_flag); 
  }
  if(!save_file)
    unlink(file);
  return(status);
}

ERRCODE
do_olc_hours(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  ERRCODE status;
  int save_file = 0;
  int type=0;
  int change_flag = 0;
  char editor[NAME_SIZE];

  strcpy(file, "");
  strcpy(editor, "");

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);

  arguments++;
  while(*arguments != (char *) NULL)
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
	  else {
	    strcpy(file,*arguments);
	    arguments++;
	  }
	  
	  save_file = TRUE;
	  continue;
	}

      if (is_flag(*arguments, "-editor", 2))
        {
          ++arguments;
          if(*arguments != (char *) NULL) {
            strcpy(editor, *arguments);
	    arguments++;
	  }
          else
            strcpy(editor, NO_EDITOR);
	  continue;
        }

      if(is_flag(*arguments,"-change", 2))
	{
          ++arguments;
	  change_flag = TRUE;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);
	
      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \thours  [-file <filename>] [-change] ");
	  printf("[-editor <editor>]\n");
	  return(ERROR);
	}
    }

  if(!change_flag) {
    Request.request_type = OLC_GET_HOURS;
    status = t_get_file(&Request,type,file,!save_file);
  }
  else {
    Request.request_type = OLC_CHANGE_HOURS;
    status = t_change_file(&Request,type,file,editor,
			   (!save_file ? OLC_GET_HOURS : 0),0);
  }
  if(!save_file)
    unlink(file);
  return(status);
}
