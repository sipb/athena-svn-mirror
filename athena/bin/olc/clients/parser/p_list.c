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
 *	$Id: p_list.c,v 1.13 1999-07-30 18:27:48 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_list.c,v 1.13 1999-07-30 18:27:48 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:	do_olc_list() displays the current questions and their
 *			status.
 * Arguments:	arguments:	The argument array from the parser.
 * Returns:	An error code.
 * Notes:
 *	Send an OLC_LIST request to the daemon and read back the text,
 *	if the request is successful.
 */

ERRCODE
do_olc_list(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status = SUCCESS;
  int  stati = 0;
  char queues[NAME_SIZE];
  char users[NAME_SIZE];
  char topics[NAME_SIZE];
  char instances[NAME_SIZE];
  char sort[NAME_SIZE][NAME_SIZE];
  char *sortP[NAME_SIZE];
  char **sortPP = sortP;
  char file[NAME_SIZE];
  int savefile = FALSE;
  int comments = 0;
  int i,mask,display= FALSE;

  queues[0] = '\0';
  users[0] = '\0';
  instances[0] = '\0';
  topics[0] = '\0';
  *sort[0] = '\0';
  sortP[0] = NULL;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  make_temp_name(file);
  savefile = FALSE;

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if((is_flag(*arguments, "-file", 2)) ||
	  string_eq(*arguments,">"))
	{
          arguments++;
          unlink(file);
          if((*arguments == NULL) || (*arguments[0] == '-'))
            {
              file[0] = '\0';
              get_prompted_input("Enter a file name: ",file, NAME_SIZE,0);
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

      if(is_flag(*arguments,"-queue", 2))
	{
	  /* Currently the server ignores this option. I am not actually
	   * sure what it is supposed to do, but I am leaving it in.
	   * - Richard Tibbetts 7-19-99
	   */
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
	      if( i >= NAME_SIZE)
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
	  if(status == ERROR)
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
		  fprintf(stderr, "Invalid status label "
			  "specified. Choose one of...\n");
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
	  strncpy(users,*arguments,NAME_SIZE-1);
	  arguments++;
	  continue;
	}

      if(is_flag(*arguments,"-topic", 2))
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
		    {
		      /* Currently the server doesnt deal with multiple topics,
		       * though it is supposed to. This throws an error if
		       * multiple topics are specified.
		       */
		      /*strcat(topics, " ");	This is the line that would be
		       *			here if it worked.
		       */
		      fprintf(stderr,
			      "Multiple topics are not allowed.\n");
		      status = ERROR;
		      break;
		    }
		  strncat(topics,*arguments,NAME_SIZE-1);
		  arguments++;
		}
	    }
	  if(status != SUCCESS)
	    break;
	  else
	    continue;
	}

      if(is_flag(*arguments,"-comments",2) ||
	 is_flag(*arguments,"-long",2))
	{
	  arguments++;
	  comments = TRUE;
	  continue;
	}

      if(is_flag(*arguments,"-display",2))
	{
	  arguments++;
	  display = TRUE;
	  continue;
	}

      if(*arguments[0] != '-')
	{
	  strncpy(users,*arguments,NAME_SIZE-1);
	  arguments++;
	  continue;
	}

       status = handle_common_arguments(&arguments, &Request);
       if(status != SUCCESS)
	 break;
    }

  if(status != SUCCESS)
    {
      fprintf(stderr, "Usage is: \tlist [-display] [-queue <queues>] "
	      "[-topic <topic>]\n\t\t[-status <statuses>] "
	      "[-comments] [<username pattern>]\n\t\t"
	      "[-file <filename>]\n");
      return ERROR;
    }


  status = t_list_queue(&Request, sortP, queues, topics, users,
			stati, comments, file, display);

  if((savefile == FALSE) || (status != SUCCESS))
    unlink(file);

  return status;
}
