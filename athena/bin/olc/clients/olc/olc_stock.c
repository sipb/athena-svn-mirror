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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_stock.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_stock.c,v 1.4 1989-08-07 14:54:41 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define STOCK_DIR "stock_answers"
#define BROWSER "/usr/athena/lib/olc/stock/browser"
#define MAGIC  "/usr/athena/lib/olc/stock/MAGIC"

/*
 * Function:  do_olcr_stock() allows a consultant to examine the
 *            OLC stock answers using the OLC documentation browser.
 * Arguments: arguments:      Arguments array from the parser.
 * Returns:   An error code.
 * Notes:
 */

ERRCODE
do_olc_stock(arguments)
      char **arguments;
{
  REQUEST Request;
  char file[NAME_LENGTH]; /* Temporary file for text. */
  char bfile[NAME_LENGTH];     /* Name of browser file. */
  struct stat statbuf;        /* Ptr. to file status buffer. */
  char *topic;                /* Topic from daemon. */
  char *dtopic = "/";         /* default topic */
  int status;                 /* Status returned by whatnow */
  int find_topic=0;
  

  if (stat(MAGIC, &statbuf) < 0) 
    {
      call_program("/bin/athena/attach","olc-stock");
      if (stat(MAGIC, &statbuf) < 0)
	call_program("/bin/athena/attach","olc-stock_backup");
      if (stat(MAGIC, &statbuf) < 0)
	{
	  fprintf(stderr,"Unable to attach olc-stock file system.\n");
	  return(ERROR);
	}
    }
 
  fill_request(&Request);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_equiv(*arguments,"-t",2))
	{
	  if(*(++arguments)==(char *) NULL)
	    find_topic=1;
	  else
	    strcpy(dtopic,*arguments);
	  continue;
	}
      arguments = handle_argument(arguments, &Request);
      if(arguments == (char **) NULL)   /* error */
	return(ERROR);
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }
    
  topic = dtopic;

  if(find_topic)
    {
      status = t_get_topic(&Request,topic);
      if(status != SUCCESS)
	{
	  fprintf(stderr,"Error occurred while trying to get topic.\n");
	  printf("Proceeding with default ... \n");
	  topic = dtopic;
	}
    }

  strcpy(bfile, STOCK_DIR);
  strcat(bfile, "/");
  strcat(bfile, topic);
  make_temp_name(file);
  
  switch(fork()) 
    {
    case -1:                /* error */
      perror("stock: fork");
      fprintf(stderr, "stock: can't fork to execute OLC browser\n");
      return(ERROR);
    case 0:                 /* child */
      execlp(BROWSER, BROWSER, "-s", file, "-f", bfile, 0);
      perror("stock: execlp");
      fprintf(stderr, "stock: could not exec OLC browser\n");
      exit(ERROR);
    default:                /* parent */
      (void) wait(0);
    }
 
  if(OLC)
     return(SUCCESS);
 
  if (stat(file, &statbuf) < 0) 
    {
      if(!OLC)
	printf("No stock answer to send.\n");
      return(SUCCESS);
    }

  status = what_now(file, FALSE, NULL);
  if (status == NO_ACTION)
    return(SUCCESS);
  else if (status == ERROR)
    return(ERROR);

  status = t_reply(&Request,file,NULL);
  
  switch (status)
    {
    case SUCCESS:
      printf("Message sent. \n");
      break;
    case NOT_CONNECTED:
      if(string_eq(Request.target.username, Request.requester.username))
	{
	  printf("You are not connected to a consultant but the next one\n");
	  printf("to answer your question will receive your message.\n");
	}
      else
	{
	  printf("%s (%d) is not connected to a consultant but the next one\n",
		 Request.target.username,Request.target.instance);
	  printf("to answer %s's question will receive your message.\n",
		 Request.target.username);
	}
      break;

    case CONNECTED:
      printf("message sent.\n");
      break;
  
    case PERMISSION_DENIED:
      fprintf(stderr, "You no longer allowed to send to %s (%d).\n",
	      Request.target.username,
	      Request.target.instance);
      status = NO_ACTION;
      break;
    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;
    default:
      status =  handle_response(status, &Request);
      break;
    }

  return(status);
}


