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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_live.c,v $
 *	$Id: p_live.c,v 1.6 1990-10-18 06:09:49 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_live.c,v 1.6 1990-10-18 06:09:49 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_live(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  int savefile;		 
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  savefile = FALSE;
  make_temp_name(file);
  	
  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      if (string_equiv(*arguments, "-f",2)) 
	{
	  arguments++;
	  if (*arguments == (char *)NULL) 
	    {
	      printf("No filename specified.\n");
	      return(ERROR);
	    }
	  else 
	    {
	      (void) strcpy(file, *arguments);
	      savefile = TRUE;
	    }
	  continue;
	}
     else 
       {
	 arguments = handle_argument(arguments, &Request,&status);
	 if(arguments == (char **) NULL)   /* error */
	   return(ERROR);
	 if(*arguments == (char *) NULL)   /* end of list */
	   break;
       }
    }

  status = t_live(&Request,file);
  if(savefile == FALSE || status != SUCCESS)
    (void) unlink(file);
  return(status);
}
