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
 *	$Id: p_describe.c,v 1.14 1999-07-30 18:27:02 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_describe.c,v 1.14 1999-07-30 18:27:02 ghudson Exp $";
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
  char *noteP = NULL;
  ERRCODE status = SUCCESS;
  int save_file = FALSE;
  int dochnote = FALSE;
  int dochcomment = FALSE;

  make_temp_name(file);
  note[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if(is_flag(*arguments,"-file", 2) ||
	 is_flag(*arguments,">", 1))
	{
	  arguments++;
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

      if(is_flag(*arguments,"-note", 2))
	{
	  arguments++;
	  if((*arguments != NULL) && (*arguments[0] != '-'))
	      {
		strncpy(note,*arguments,NOTE_SIZE-1);
		noteP = &note[0];
		arguments++;
	      }
	  dochnote = TRUE;
	  continue;
	}

      if(is_flag(*arguments,"-comment",2))
	{	
	  dochcomment = TRUE;
	  arguments++;
	  continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)   /* error */
    {
      fprintf(stderr,
	      "Usage is: \tdescribe  [user <instance id>] [-file <filename>] "
	      "[-note <note>]\n\t\t[-comment] [-instance <instance id>]\n");
      return ERROR;
    }

  status = t_describe(&Request, file, noteP, dochnote, dochcomment);
  if (status == OK)
    status = SUCCESS;

  if(!save_file)
    unlink(file);

  return status;
}
