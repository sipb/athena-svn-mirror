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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v 1.1 1989-07-06 22:08:58 tjcoppet Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_motd(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_LENGTH];
  int status;
  int save_file;
  int type=0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_eq(*arguments, ">") || string_equiv(*arguments,"-file",
						    max(strlen(*arguments),2)))
	{
	  (void) strcpy(file,*arguments);
	  save_file = TRUE;
	  continue;
	}
      arguments = handle_argument(arguments, &Request, &status);
      if(!status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  fprintf(stderr,"Usage is: \tmotd  [-file <filename>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_get_motd(&Request,type,file,TRUE);
  if(!save_file)
    (void) unlink(file);
  return(status);
}
  
