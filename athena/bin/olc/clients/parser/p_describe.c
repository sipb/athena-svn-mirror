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
 *	$Id: p_describe.c,v 1.12 1999-06-28 22:52:07 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_describe.c,v 1.12 1999-06-28 22:52:07 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_describe(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  char note[NOTE_SIZE];
  char *noteP = (char *) NULL;
  ERRCODE status;
  int save_file = 0;
  int cherries=0;
  int oranges=0;
 
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);
  note[0] = '\0';

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_eq(*arguments, ">") || is_flag(*arguments,"-file", 2))
	{
          ++arguments;
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
	if(is_flag(*arguments,"-note", 2))
	  {
	    if(*(arguments +1) != (char *) NULL)
	      if(*(arguments + 1)[0] != '-')
		{
		  ++arguments;
		  strncpy(note,*arguments,NOTE_SIZE-1);
		  noteP = &note[0];
		}
	    cherries = TRUE;
	  }
       else
	if(is_flag(*arguments,"-comment",2))
	  oranges = TRUE;
      else
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status != SUCCESS)
	    return(ERROR);
	}

      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tdescribe  [user <instance id>] [-file <filename>] [-note <note>]\n\t\t[-comment] ");
	  printf("[-instance <instance id>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_describe(&Request,file,noteP,cherries,oranges);

  if (status == OK)
    status = SUCCESS;

  if(!save_file)
    unlink(file);

  return(status);
}
  
