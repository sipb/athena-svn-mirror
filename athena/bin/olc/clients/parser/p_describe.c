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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_describe.c,v $
 *	$Id: p_describe.c,v 1.5 1990-07-16 08:20:47 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_describe.c,v 1.5 1990-07-16 08:20:47 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_describe(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  char note[NOTE_SIZE];
  char *noteP = (char *) NULL;
  int status;
  int save_file = 0;
  int cherries=0;
  int oranges=0;
 
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);
  note[0] = '\0';

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_eq(*arguments, ">") || string_equiv(*arguments,"-file",
						    max(strlen(*arguments),2)))
	{
          ++arguments;
	  if(*arguments == NULL)
            {
	      file[0] = '\0';
              get_prompted_input("Enter file name: ",file);
	      if(file[0] == '\0')
		return(ERROR);
            }
	  else
	    (void) strcpy(file,*arguments);

	  save_file = TRUE;
	}
      else
	if(string_equiv(*arguments,"-note",max(strlen(*arguments),2)))
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
	if(string_equiv(*arguments,"-comment",max(strlen(*arguments),2)))
	  oranges = TRUE;
      else
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status)
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

  if(!save_file)
    (void) unlink(file);

  return(status);
}
  
