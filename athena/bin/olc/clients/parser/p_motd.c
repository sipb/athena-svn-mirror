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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v $
 *	$Id: p_motd.c,v 1.16 1992-01-10 20:00:23 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v 1.16 1992-01-10 20:00:23 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

ERRCODE
do_olc_motd(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  int status;
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
	  else {
	    (void) strcpy(file,*arguments);
	    arguments++;
	  }
	  
	  save_file = TRUE;
	  continue;
	}

      if (string_equiv(*arguments, "-editor",max(strlen(*arguments),2)))
        {
          ++arguments;
          if(*arguments != (char *) NULL) {
            (void) strcpy(editor, *arguments);
	    arguments++;
	  }
          else
            (void) strcpy(editor, NO_EDITOR);
        }

      if(string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
          ++arguments;
	  change_flag = TRUE;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
	
      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tmotd  [-file <filename>] [-change] ");
	  printf("[-editor <editor>]\n");
	  return(ERROR);
	}
    }

  if(!change_flag) {
    Request.request_type = OLC_MOTD;
    status = t_get_file(&Request,type,file,!save_file);
  }
  else {
    Request.request_type = OLC_CHANGE_MOTD;
    status = t_change_file(&Request,type,file,editor,
			   (!save_file ? OLC_MOTD : 0)); 
  }
  if(!save_file)
    (void) unlink(file);
  return(status);
}

ERRCODE
do_olc_hours(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  int status;
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
	  else {
	    (void) strcpy(file,*arguments);
	    arguments++;
	  }
	  
	  save_file = TRUE;
	  continue;
	}

      if (string_equiv(*arguments, "-editor",max(strlen(*arguments),2)))
        {
          ++arguments;
          if(*arguments != (char *) NULL) {
            (void) strcpy(editor, *arguments);
	    arguments++;
	  }
          else
            (void) strcpy(editor, NO_EDITOR);
        }

      if(string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
          ++arguments;
	  change_flag = TRUE;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
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
			   (!save_file ? OLC_GET_HOURS : 0));
  }
  if(!save_file)
    (void) unlink(file);
  return(status);
}
