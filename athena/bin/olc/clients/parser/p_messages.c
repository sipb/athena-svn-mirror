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
 *	$Id: p_messages.c,v 1.17 1999-07-30 18:28:22 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_messages.c,v 1.17 1999-07-30 18:28:22 ghudson Exp $";
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
  ERRCODE status = SUCCESS;
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
    return ERROR;

  if(arguments == NULL)
    return ERROR;
  arguments++;

  make_temp_name(file);
  savefile = FALSE;

  while(*arguments != NULL)
    {
      if ((is_flag(*arguments, "-file",2)) ||
	  string_eq(*arguments,">"))
	{
	  arguments++;
	  unlink(file);
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      file[0] = '\0';
	      get_prompted_input("Enter a file name: ",file,NAME_SIZE,0);
	      if(file[0] == '\0')
		{
		  status = ERROR;
		  break;
		}
	    }
	  else
	    { 
	      strcpy(file, *arguments);
	      arguments++;
	    }
	  savefile = TRUE;
	  continue;
	}

      if(is_flag(*arguments,"-queue",2))
	{
	  arguments++;
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      fprintf(stderr,
		      "You must specify a queue after the -queue option.\n");
	      status = ERROR;
	      break;
	    }

	  i = 0;
	  while((*arguments != NULL) && (*arguments[0] != '-'))	 
	    {
	      i += strlen(*arguments) + 1;
	      if(i >= NAME_SIZE)
		{
		  fprintf(stderr,"Too many queues specified.\n");
		  status = ERROR;
		  break;
		}
	      else
		{
		  if(queues[0] != '\0')
		    strcat(queues," ");
		  strncat(queues,*arguments,NAME_SIZE-1);
		  arguments++;
		}
	    }
	  if(status != SUCCESS)
	    break;
	  else
	    continue;
	}

      if(is_flag(*arguments,"-status",2))
	{
	  arguments++;
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      fprintf(stderr, "You must specify something "
		      "after the -status option.\n");
	      status = ERROR;;
	      break;
	    }
	  while((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      OGetStatusCode(*arguments, &mask);
	      arguments++;
	      if(mask == -1)
		{
		  printf("Invalid status label specified. Choose one of...\n");
		  t_pp_stati();
		  status = ERROR;
		  break;
		}
	      else
		stati |= mask;
	    }
	  if(status != SUCCESS)
	    break;
	  else
	    continue;
	}

      if(is_flag(*arguments,"-user",2))
	{
	  arguments++;
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      fprintf(stderr,
		      "You must specify a username after the -user option.\n");
	      status = ERROR;
	      break;
	    }
	  else
	    {
	      strncpy(users,*arguments,NAME_SIZE-1);
	      arguments++;
	      continue;
	    }
	}

      if(is_flag(*arguments,"-topic",2))
	{
	  arguments++;
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      fprintf(stderr,
		      "You must specify a topic after the -topic option.\n");
	      status = ERROR;
	      break;
	    }

	  i = 0;
	  while((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      i += strlen(*arguments) + 1;
	      if(i >= NAME_SIZE)
		{
		  fprintf(stderr,
			  "Too many topics specified.\n");
		  status = ERROR;
		  break;
		}
	      else
		{
		  if(topics[0] != '\0')
		    strcat(topics," ");
		  strncat(topics,*arguments,NAME_SIZE-1);
		  arguments++;
		  continue;
		}
	    }
	  if(status != SUCCESS)
	    break;
	  else
	    continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)
    {
      if(client_is_user_client())
	fprintf(stderr, "Usage is: \treplay [-file <file name>]\n");
      else
	{
	  fprintf(stderr, "Usage is: \treplay [<username> <instance id>] "
		  "[-file <file name>]\n"
		  "\t\t[-instance <instance id>]\n");
	}
      return ERROR;
    }


  status = t_replay(&Request, queues, topics, users, stati, file, !savefile);
  if((savefile == FALSE) || (status != SUCCESS))
    unlink(file);
  return status;
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
  ERRCODE status = SUCCESS;
  int connected = 0;
  int noflush = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  savefile = FALSE;
  make_temp_name(file);

  while(*arguments != NULL)
    {
      if ((is_flag(*arguments, "-file",2)) ||
	  string_eq(*arguments,">"))
	{
	  arguments++;
	  unlink(file);
	  if((*arguments == NULL) || (*arguments[0] == '-'))
	    {
	      file[0] = '\0';
  	      get_prompted_input("Enter a filename: ",file,NAME_SIZE,0);
	      if(file[0] == '\0')
		{
		  status = ERROR;
		  break;
		}
	    }
	  else
	    { 
	      strcpy(file, *arguments);
	      arguments++;
	    }
	  savefile = TRUE;
	  continue;
	}

      if (is_flag(*arguments, "-connected", 2))
	{
	  arguments++;
	  connected = TRUE;
	  continue;
	}

      if (is_flag(*arguments, "-noflush", 2))
	  {
	    arguments++;
	    noflush = TRUE;
	    continue;
	  }

      status = handle_common_arguments(&arguments, &Request);
      if (status != SUCCESS)
	break;
    }

  if(status != SUCCESS)
    {
      if(client_is_user_client())
	{
	  fprintf(stderr,
		  "Usage is: \tshow [-file <filename>] [-connected]"
		  " [-noflush]\n");
	}
      else
	{
	  fprintf(stderr,
		  "Usage is: \tshow [<username> <instance id>] "
		  "[-file <filename>]\n"
		  "\t\t[-connected] [-noflush] "
		  "[-instance <instance id>]\n");
	}
      return ERROR;
    }

  status = t_show_message(&Request,file, !savefile, connected, noflush);

  if((savefile == FALSE) || (status != SUCCESS))
    unlink(file);

  return status;
}
