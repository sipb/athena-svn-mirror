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
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_utils.c,v 1.18 1999-07-08 22:56:53 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_utils.c,v 1.18 1999-07-08 22:56:53 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <ctype.h>

#include <olc/olc.h>
#include <olc/olc_parser.h>


/*
 * Function: handle_common_arguments() handles arguments common to all
 *		olc commands, including -help, -instance, -j (specify
 *		target instance), and usernames.
 * Arguments: args: pointer to the pointer to the array of arguments.
 * 		handle_common_arguments() will modify this to reflect
 *		the arguments that are handled. Don't pass a pointer
 *		a NULL pointer please.
 *		req:	REQUEST structure to modify.
 * Returns:	ERRCODE:
 *		SUCCESS means...success (one or more arguments were
 *			popped and handled.)
 *		ABORT means the function lost, and that the
 *			calling function should return the user
 *			back to command loop. (An error message
 *			will already have been printed.)
 * Notes: handle_common_arguments() will look at the first argument in
 *		args, and if it knows how to handle the argument, it
 *		will pop it off and take care of it. The function may
 *		pop off more than one argument, for instance if the arg
 *		takes a modifier. In general, call the function after
 *		checking all function specific options. If the function
 *		returns SUCCESS, continue checking the rest of args. If
 *		it returns ABORT, the command call was bad (args
 *		invalid arguments), and error message has been printed
 *		and the calling function should abort.
 */

ERRCODE
handle_common_arguments(args, req)
     char ***args;
     REQUEST *req;
{
  if((is_flag((*args)[0], "-instance",2)) ||
     (is_flag((*args)[0], "-j",2)))
    {
      if((*args)[1] != NULL)
	{
	  if((*args)[1][0] == '-')
	    {
	      printf("You must specify and instance after %s\n", (*args)[0]);
	      (*args)++;
	      req->target.instance = NO_INSTANCE;
	      return ABORT;
	    }
	  else if(isnumber((*args)[1]) != SUCCESS)
	    {
	      printf("Specified instance id \"%s\" is not a number.\n",
		     (*args)[1]);
	      (*args) += 2;
	      req->target.instance = NO_INSTANCE;
	      return ABORT;
	    }
	  else	/* (*args)[1] is a valid instance id. */
	    {
	      if(is_flag((*args)[0], "-instance", 2))
		req->requester.instance = atoi((*args)[1]);
	      else /* -j flag */
		req->target.instance = atoi((*args)[1]);
	      (*args) += 2;
	      return SUCCESS;
	    }
	}
    }
  else if(is_flag((*args)[0], "-help",2))
    {
      printf("Use \"help <command name>\" to get help.-n");
      (*args)++;
      return ABORT; /* Don't do the command, they just wanted help. */
    }
  else if((*args)[0][0] == '-')
    {
      /* Some flag that the calling command doesn't take, and
       * neither do we.
       */
      printf("%s is not a valid command option.\n",
	     (*args)[0]);
      (*args)++;
      return ABORT;
    }
  else
    {
      /* Assume the argument is supposed to be a username. */
      if(!isalnum((*args)[0][0]))
	{
	  printf("%s is not a valid username\n", (*args)[0]);
	  (*args)++;
	  return ABORT;
	}

      /* Hopefully this is a valid username, possibly a more robust
       * check should be done (although the server will barf back a
       * username that doesn't match a real person).
       */
      strncpy(req->target.username,(*args)[0],LOGIN_SIZE);

      /* Check for an instance on the username.*/
      if(((*args)[1] != NULL) && ((*args)[1][0] != '-') &&
	 ((*args)[1][0] != '>'))
	{
	  if(isnumber((*args)[1]) != SUCCESS)
	    {
	      printf("Specified instance id \"%s\" is not a number.\n", 
		     (*args)[1]);
	      (*args) += 2;
	      return ABORT;
	    }
	  req->target.instance = atoi((*args)[1]);
	  (*args) += 2;
	  return SUCCESS;
	}
      else
	{
	  /* No instance. */
	  req->target.instance = NO_INSTANCE;
	  (*args) += 1;
	  return SUCCESS;
	}
    }
}
      

 /* handle_argument() used to do what handle_common_arguments() does now.
  * It sucks though. */

int num_of_args;		/* Serious hack.  Global var to indicate
				 * number of args popped off of arg
				 * array by handle_argument().
				 */

char **
handle_argument(args, req, status)
     char **args;
     REQUEST *req;
     ERRCODE *status;
{
  num_of_args = 0;
  *status = SUCCESS;

  if(string_eq(args[0],"-help"))
    return((char **) NULL);
  
  if (is_flag(args[0], "-instance",2))
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	if(isnumber(*args) != SUCCESS)
	  {
	    printf("Specified instance id \"%s\" is not a number.\n", *args);
	    *status = ERROR;
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
  else if (is_flag(args[0], "-j",2))
    if((*(++args) != (char *) NULL) && (*args[0] != '-'))
      {
	if(isnumber(*args) != SUCCESS)
	  {
	    printf("Specified instance id \"%s\" is not a number.\n", *args);
	    *status = ERROR;
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
      *status = ERROR;
      return((char **) NULL);
    }
  else if(is_flag(args[0],"-help",2))
      return((char **) NULL);
  else if(*args)
    {
      if (!isalnum(*args[0])) {
	fprintf(stderr,"The username \"%s\" is invalid.\n", *args);
	*status = ERROR;
	return((char **) NULL);
      }
      strncpy(req->target.username,*args,LOGIN_SIZE);
      num_of_args++;
      if((*(args+1) != (char *) NULL) && (*args[1] != '-') &&
	 (*args[1] != '>'))
	{
	  ++args;
	  if(isnumber(*args) != SUCCESS)
	    {
	      printf("Specified instance id \"%s\" is not a number.\n", 
		     *args);
	      *status = ERROR;
	      return((char **) NULL);
	    }
	  req->target.instance = atoi(*args); 	 
	}
      else
	req->target.instance = NO_INSTANCE; 	 
    } 
  return(args);
}
