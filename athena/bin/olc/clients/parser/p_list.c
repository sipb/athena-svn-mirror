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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_list.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_list.c,v 1.4 1990-02-14 14:43:15 vanharen Exp $";
#endif


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
  int  status;
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
  sortP[0] = (char *) NULL;
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  make_temp_name(file);
  savefile = FALSE;

  for (++arguments; *arguments != (char *) NULL; ++arguments) 
    {

       if ((string_equiv(*arguments, "-file",max(2,strlen(*arguments)))) ||
          string_eq(*arguments,">"))
        {
          arguments++;
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

          savefile = TRUE;
          continue;
        }

      if(string_equiv(*arguments,"-queue",max(strlen(*arguments),2)))
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

      if(string_equiv(*arguments,"-status",max(strlen(*arguments),2)))
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

      if(string_equiv(*arguments,"-user",max(strlen(*arguments),2)))
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
      
      if(string_equiv(*arguments,"-topic",max(strlen(*arguments),2)))
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
      
      /* 
       * some strange way is going to have to be devised to specify
       * multiple targets
       */
	 
      if(string_equiv(*arguments,"-comments",max(strlen(*arguments),2))||
	 string_equiv(*arguments,"-long",max(strlen(*arguments),2)))
	{
	  comments = TRUE;
	  continue;
	}
	 
      if(string_equiv(*arguments,"-display",max(strlen(*arguments),2)))
	{
	  display = TRUE;
	  continue;
	}
	
#if 0
       
       if(string_equiv(*arguments,"-sort",max(strlen(*arguments),3)))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      return(ERROR);
	    }
	  for(i=0; *arguments != (char *) NULL; arguments++)
	    {
	      if(i >= NAME_SIZE-1)
		{
		  fprintf(stderr,"Too many rules...\n");
		  break;
		}
	      if(strlen(*arguments) >= (NAME_SIZE))
		fprintf(stderr,
			"Name too long. Continuing...\n");
	      else
		{
		  strncpy(sort[i], *arguments, NAME_SIZE-1);
		  *sort[i+1] = '\0';
		  sortP[i] = &sort[i][0];
		}
	      if((*(arguments+1)) && (*(arguments+1)[0] == '-'))
		break;
		
	      if(arguments[1] == (char *) NULL)
		break;
	    }
	  sortP[i] = &sort[i][0];
	  continue;
	}

/* #else
   Sorry, Ken, we gotta rip sorting out of the CLI for now...
*/
       if (!strcmp (*arguments, "-sort")) {
	   ++arguments;
	   if (*arguments == (char *) NULL)
	       return ERROR;
	   if (*arguments[0] == '-')
	       return ERROR;
	   *sortPP++ = *arguments++;
	   *sortPP = NULL;
       }
#endif


      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
	    
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tlist [-display] [-queue <queues>] ");
	  printf("[-topic <topic>]\n\t\t[-status <statuses>] ");
	  printf("[-comments] [<username pattern>]\n\t\t");
#if 0
	  printf("[-sort <something>] ");
#endif
	  printf("[-file <filename>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_list_queue(&Request,sortP,queues,topics,users,stati,comments,file,display);
  if((savefile == FALSE) || (status != SUCCESS))
    (void) unlink(file);

  return(status);
}
