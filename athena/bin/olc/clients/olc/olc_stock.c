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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_stock.c,v $
 *	$Id: olc_stock.c,v 1.16 1991-04-10 21:58:58 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_stock.c,v 1.16 1991-04-10 21:58:58 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define STOCK_DIR "/mit/olc-stock/stock_answers"
#define BROWSER "/mit/olc-stock/browser"
#define MAGIC  "/mit/olc-stock/MAGIC"

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
  char file[NAME_SIZE];		/* Temporary file for text. */
  char bfile[NAME_SIZE];	/* Name of browser file. */
  struct stat statbuf;		/* Ptr. to file status buffer. */
  char *topic;			/* Topic from daemon. */
  char dtopic[TOPIC_SIZE];	/* default topic */
  int status;			/* Status returned by whatnow */
  int find_topic=0;
  
  dtopic[0] = '\0';

  if (stat(MAGIC, &statbuf) < 0) 
    {
      call_program("/bin/athena/attach","olc-stock");
      if (stat(MAGIC, &statbuf) < 0)
	{
	  fprintf(stderr,"Unable to attach olc-stock file system.\n");
	  return(ERROR);
	}
    }
 
  OFillRequest(&Request);

  arguments++;
  while(*arguments != (char *) NULL)
    {
      if(string_equiv(*arguments,"-t",2))
	{
          ++arguments;
	  if(*arguments==(char *) NULL)
            {
	      find_topic=1;
              break;   
            }
	  else
	    {
	      strcpy(dtopic,*arguments);
	      arguments++;
	    }
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(arguments == (char **) NULL)   /* error */
	return(ERROR);
    }

  topic = &dtopic[0];

  if(find_topic)
    {
      status = OGetTopic(&Request,topic);
      if(status != SUCCESS)
	{
	  fprintf(stderr,"Error occurred while trying to get topic.\n");
	  printf("Proceeding with default ... \n");
	  topic = dtopic;
	}
    }

  strcpy(bfile, "/.");
  if (topic[0] != NULL)  {
    strcat(bfile, "/");
    strcat(bfile, topic);
  }

  make_temp_name(file);
  
#ifdef NO_VFORK
  switch(fork()) 
#else
  switch(vfork()) 
#endif
    {
    case -1:                /* error */
      olc_perror("stock: fork");
      fprintf(stderr, "stock: can't fork to execute OLC browser\n");
      return(ERROR);
    case 0:                 /* child */
      if (topic[0] != NULL)
	execlp(BROWSER, BROWSER, "-s", file, "-r", STOCK_DIR, "-f", topic,
	       0);
      else
	execlp(BROWSER, BROWSER, "-s", file, "-r", STOCK_DIR, 0);
      olc_perror("stock: execlp");
      fprintf(stderr, "stock: could not exec OLC browser\n");
      _exit(ERROR);
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

  status = OReply(&Request,file);
  
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
	  printf("%s [%d] is not connected to a consultant but the next one\n",
		 Request.target.username,Request.target.instance);
	  printf("to answer %s's question will receive your message.\n",
		 Request.target.username);
	}
      break;

    case CONNECTED:
      printf("message sent.\n");
      break;
  
    case PERMISSION_DENIED:
      fprintf(stderr, "You are no longer allowed to send to %s [%d].\n",
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
