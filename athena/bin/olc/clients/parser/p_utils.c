/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains miscellaneous utilties for the olc and olcr programs.
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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_utils.c,v $
 *	$Id: p_utils.c,v 1.9 1990-05-26 12:12:43 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_utils.c,v 1.9 1990-05-26 12:12:43 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

int num_of_args;		/* Serious hack.  Global var to indicate */
				/* number of args popped off of arg array. */

char **
handle_argument(args, req, status)
     char **args;
     REQUEST *req;
     int *status;
{
  num_of_args = 0;
  *status = SUCCESS;

  if(string_eq(args[0],"-help"))
    return((char **) NULL);
  
  if (string_equiv(args[0], "-instance",max(strlen(args[0]),2))) 
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	if(isnumber(*args) != SUCCESS)
	  {
	    printf("Specified instance id \"%s\" is not a number.\n", *args);
	    return((char **) NULL);
	  }
	num_of_args++;
	req->requester.instance = atoi(*args);
	return(args);
      }
    else
      {
	fprintf(stderr,"You must specify an instance after '-i'.\n");
        *status = ERROR;
	return((char **) NULL);
      }
  else  if (string_equiv(args[0], "-j",max(strlen(args[0]),2))) 
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	if(isnumber(*args) != SUCCESS)
	  {
	    printf("Specified instance id \"%s\" is not a number.\n", *args);
	    return((char **) NULL);
	  }
	num_of_args++;
	req->target.instance = atoi(*args);
	return(args);
      }
    else
      {
	fprintf(stderr,"You must specify an instance after '-j'.\n");
	*status = ERROR;
	return((char **) NULL);
      }
  else if (*args[0] == '-')
    {
      fprintf(stderr, "The argument, \"%s\", is invalid.\n", *args);
      return((char **) NULL);
    }
  else if(string_equiv(args[0],"-help",max(strlen(args[0]),2)))
         return((char **) NULL);
  else          
    if(*args)
	{
	  (void) strcpy(req->target.username,*args);
	  num_of_args++;
	  if((*(args+1) != (char *) NULL) && (*args[1] != '-') &&
	     (*args[1] != '>'))
	    {
	      ++args;
	      if(isnumber(*args) != SUCCESS)
		{
		  printf("Specified instance id \"%s\" is not a number.\n", 
			 *args);
		  return((char **) NULL);
		}
	      req->target.instance = atoi(*args); 	 
	    }
	  else
	    req->target.instance = NO_INSTANCE; 	 
	} 
  return(args);
}




