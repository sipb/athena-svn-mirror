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
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_messages.c,v 1.16 1999-06-28 22:52:08 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_messages.c,v 1.16 1999-06-28 22:52:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:	do_olc_replay() prints the log of the conversation up to
 *			this point.
 * Arguments:	arguments:	Argument array from the parser.
 *		    arguments[1] may be a username or ">".
 *		    arguments[2] may be a filename to save the log in if
 *			arguments[1] is ">".
 *		    The two different arguments may not be used at the
 *			same time.  (I call this a bug)
 * Returns:	An error code.
 * Notes:
 *	If the command has the form "replay > filename", save the
 *	conversation in "filename" for the user.  If the first argument is
 *	a username, replay the conversation of that user.  Otherwise,
 *	read the log from the daemon and display it.
 */



ERRCODE
do_olc_replay(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;
  int  stati = 0;
  char queues[NAME_SIZE];
  char users[NAME_SIZE];
  char topics[NAME_SIZE];
  char file[NAME_SIZE];
  int savefile = FALSE;
  int i, mask;

  queues[0] = '\0';
  users[0] = '\0';
  topics[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  make_temp_name(file);
  savefile = FALSE;

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      if ((is_flag(*arguments, "-file",2)) || string_eq(*arguments,">"))
	{
	  arguments++;
	  unlink(file);
	  if (*arguments == (char *)NULL) 
	    {
	      file[0] = '\0';
	      get_prompted_input("Enter a file name: ",file,NAME_SIZE,0);
	      if(file[0] == '\0')
		return(ERROR);
	    }
	  else 
	    strcpy(file, *arguments);
	
	  savefile = TRUE;
	  continue;
	}
      
      if(is_flag(*arguments,"-queue",2))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      fprintf(stderr,
		      "You must specify a queue after the -queue option.\n");
	      return(ERROR);
	    }
	 
	  for(i=0; *arguments != (char *) NULL; arguments++)
	    {
	      if(strlen(*arguments) >= (NAME_SIZE -i))
		fprintf(stderr,"Too many queues specified. Continuing...\n");
	      else
		{
		  strcat(queues," ");
		  strncat(queues,*arguments,NAME_SIZE-1);
		  break;
		}
	      if(*(arguments+1) && (*(arguments+1)[0] == '-'))
		break;
	      if(arguments[1] == (char *) NULL)
		break;
	    }
	  continue;
	}

      if(is_flag(*arguments,"-status",2))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    for(i=0; *arguments != (char *) NULL; arguments++)
	      {
		OGetStatusCode(*arguments, &mask);
		if(mask == -1)
		  {
		    printf("Invalid status label specified. Choose one of...\n");
		    t_pp_stati();
		    return(ERROR);
		  }
		else
		  stati |= mask;
		if((*(arguments+1)) && (*(arguments+1)[0] == '-'))
		  break;
		if(arguments[1] == (char *) NULL)
		  break;
	      }
	  continue;
	}

      if(is_flag(*arguments,"-user",2))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      fprintf(stderr,
		      "You must specify something after the -user option.\n");
	      return(ERROR);
	    }
	  strncpy(users,*arguments,NAME_SIZE-1);
	  continue;
	}
      
      if(is_flag(*arguments,"-topic",2))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      fprintf(stderr,
		      "You must specify a topic after the -topic option.\n");
	      return(ERROR);
	    }

	  for(i=0; *arguments != (char *) NULL; arguments++)
	    {
	      if(strlen(*arguments) >= (NAME_SIZE -i))
		fprintf(stderr,
			"Too many topics specified. Continuing...\n");
	      else
		{
		  strncpy(topics, *arguments, NAME_SIZE-1);
		  break;
		}
	      if((*(arguments+1)) && (*(arguments+1)[0] == '-'))
		break;
	      if(arguments[1] == (char *) NULL)
		break;
	    }
	  continue;
	}
 
      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);
	
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \treplay [-file <file name>]\n");
	  else
	    {
	      printf("Usage is: \treplay [<username> <instance id>] ");
	      printf("[-file <file name>]\n");
	      printf("\t\t[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }


  status = t_replay(&Request, queues, topics, users, stati, file, !savefile);
  if((savefile == FALSE) || (status != SUCCESS))
    unlink(file);
  return(status);
}






/*
 * Function:	do_olc_show() shows the new part of the log.
 * Arguments:	arguments:	The argument array from the parser.
 * Returns:	An error code.
 * Notes:
 */

ERRCODE
do_olc_show(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  int savefile;		 
  ERRCODE status;
  int connected = 0;
  int noflush = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  savefile = FALSE;
  make_temp_name(file);

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      if ((is_flag(*arguments, "-file",2)) || string_eq(*arguments,">"))
	{
	  arguments++;
	  unlink(file);
	  if (*arguments == (char *)NULL)
	    {
	      file[0] = '\0';
  	      get_prompted_input("Enter a filename: ",file,NAME_SIZE,0);
	      if(file[0] == '\0')
		return(ERROR);
	    }
	  else 
	    strcpy(file, *arguments);
	
	  savefile = TRUE;
	}
      else
	if (is_flag(*arguments, "-connected", 2))
	  {
	    connected = TRUE;
	    continue;
	  }
      else
	if (is_flag(*arguments, "-noflush", 2))
	  {
	    noflush = TRUE;
	    continue;
	  }
	else 
	  {
	    arguments = handle_argument(arguments, &Request, &status);
	    if (status != SUCCESS)
	      return(ERROR);
	  }
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    {
	      printf("Usage is: \tshow [-file <filename>] [-connected]");
	      printf(" [-noflush]\n");
	    }
	  else
	    {
	      printf("Usage is: \tshow [<username> <instance id>] ");
	      printf("[-file <filename>]\n");
	      printf("\t\t[-connected] [-noflush] ");
	      printf("[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }


  status = t_show_message(&Request,file, !savefile, connected, noflush);

  if((savefile == FALSE) || (status != SUCCESS))
    unlink(file);

  return(status);
}
