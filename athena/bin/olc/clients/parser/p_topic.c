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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_topic.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_topic.c,v 1.7 1989-08-22 13:50:41 tjcoppet Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:	do_olc_topic() queries or changes the topic of a question.
 * Arguments:	arguments:	The argument array from the parser.
 *		    arguments[1] is the new topic.
 *		    arguments[1] may be "?" to get a list of allowed topics.
 * Returns:	An error code.
 * Notes:
 *	If the first argument is "?", print the list of allowed topics.  If
 *	there is no argument, ask the daemon for the current topic.  Otherwise,
 *	check to see that the given topic is valid, and request the daemon to
 *	change to topic to the one specified. 
 */

ERRCODE
do_olc_topic(arguments)
     char **arguments;
{
  REQUEST Request;
  char topic[TOPIC_SIZE];	
  char file[NAME_LENGTH];
  int status;
  int function  = 0;
  int save_file = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  make_temp_name(file);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_equiv(*arguments,"-list",max(strlen(*arguments),2)) ||
	 string_equiv(*arguments,"?",max(strlen(*arguments),1)))
	{
	  function = 1;
	  continue;
	}

      if(string_equiv(*arguments,"-topic",max(strlen(*arguments),2)))
	{
	  if(*(arguments+1) != (char *) NULL) 
	    {
              ++arguments;
	      strcpy(topic,*arguments);
            }
          else
              topic[0]='\0';

	  t_input_topic(&Request,topic,FALSE);
	  function = 2;
	  continue;
	}
      
      if(!strcmp(*arguments, ">") || 
	 string_equiv(*arguments,"-file",max(strlen(*arguments),2)))
	{
          ++arguments;
	  unlink(file);
	  if (*arguments == (char *)NULL)
            {
              file[0] = '\0';
              get_prompted_input("Enter a file name: ",file);
              if(file[0] == '\0')
                return(ERROR);
            }
          else
            (void) strcpy(file, *arguments);

	  save_file = TRUE;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(OLC)
	    fprintf(stderr,
		  "Usage is: \ttopic [-list] [-file <file name>] \n");
	  else
	    {
	      fprintf(stderr,
		      "Usage is: \ttopic  [<username> <instance id>] ");
	      fprintf(stderr,
		      "[-topic <topic>] [-list]\n\t\t[-file <file name>]\n");
	    }
	  return(ERROR);
	}
      if(arguments == (char **) NULL)   /* error */
	return(ERROR);
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  switch(function)
    {
    case 0:   
      status = t_get_topic(&Request,topic);
      break;

    case 1:   
      status = t_list_topics(&Request,file,!save_file);
      if((status != SUCCESS) || (save_file == FALSE))
	unlink(file);
      break;

    case 2:   
      status = t_change_topic(&Request,topic);
      break;
    }

  return(status);
}
