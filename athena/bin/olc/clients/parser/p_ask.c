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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v 1.1 1989-07-06 22:08:27 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_ask(arguments)
     char **arguments;
{
  REQUEST  Request;
  int status = 0;
  char topic[TOPIC_SIZE];

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  topic[0]= '\0';

  if(arguments != (char **) NULL)
    {
      for (arguments++; *arguments != (char *) NULL; arguments++)
        {
          if(string_equiv(*arguments, "-t", 2))
            {
              if(*++arguments != (char *) NULL)
                {
                  (void) strcpy(topic,*++arguments);
                  status = 1;
                }
            }
          else
            {
              arguments = handle_argument(arguments, &Request);
              if(arguments == (char **) NULL)   /* error */
                return(ERROR);
              if(*arguments == (char *) NULL)   /* end of list */
                break;
            }
        }
    }

  if(!status)
    t_input_topic(&Request,topic,TRUE);

  status = t_ask(&Request,topic);
  return(status);
}
