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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_send.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_send.c,v 1.3 1989-07-16 17:06:06 tjcoppet Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_send(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  char file[NAME_LENGTH];
  char editor[NAME_LENGTH];
  char *editorP = (char *) NULL;
  char *fileP = (char *) NULL;
  int temp = FALSE;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if (string_equiv(*arguments, "-editor",max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      editorP = editor;
	      (void) strcpy(editorP, *arguments);
	    }
	  continue;
	}

      if(string_equiv(*arguments, "-file",max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      fileP = file;
	      (void) strcpy(fileP, *arguments);
	    }
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(OLC)
	    {
	      fprintf(stderr,"Usage is: \tsend [-editor <editor>] ");
	      fprintf(stderr,"[-file <file name>]\n");
	    }
	  else
	    {
	      fprintf(stderr,
		      "Usage is: \tsend  [<username> <instance id>] ");
	      fprintf(stderr,"[-editor <editor>]\n\t\t[-file <file name>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }
  
  if(fileP == (char *) NULL)
    {
      make_temp_name(file);
      fileP = file;
      temp = TRUE;
    }

  status = t_reply(&Request,fileP,editorP);
  if(temp)
    (void) unlink(file);

  return(status);
}





ERRCODE
do_olc_comment(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  char file[NAME_LENGTH];
  char editor[NAME_LENGTH];
  char *editorP = (char *) NULL;
  char *fileP = (char *) NULL;
  int temp = FALSE;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if (string_equiv(*arguments, "-editor", max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      editorP = editor;
	      (void) strcpy(editorP, *arguments);
	    }
	  continue;
	}
      if(string_equiv(*arguments, "-file", max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      fileP = file;
	      (void) strcpy(fileP, *arguments);
	    }
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  fprintf(stderr,
		  "Usage is: \tcomment  [<username> <instance id>] ");
	  fprintf(stderr,"[-editor <editor>]\n\t\t[-file <file name>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }
  
  if(fileP == (char *) NULL)
    {
      make_temp_name(file);
      fileP = file;
      temp = TRUE;
    }

  status = t_comment(&Request,fileP,editorP);
  if(temp)
    (void) unlink(file);

  return(status);
}



ERRCODE 
do_olc_mail(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_LENGTH];
  char editor[NAME_LENGTH];
  char *editorP = (char *) NULL;
  char *fileP = (char *) NULL;
  int status;
  int temp = FALSE;
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for(++arguments; *arguments != (char *) NULL; arguments++)
    {
      if (string_equiv(*arguments, "-editor", max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      editorP = editor;
	      (void) strcpy(editorP, *arguments);
	    }
	  continue;
	}
      
      if (string_equiv(*arguments, "-file", max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments != (char *) NULL)
	    {
	      fileP = file;
	      (void) strcpy(fileP, *arguments);
	    }
	  continue;
	}
      
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  fprintf(stderr,
		  "Usage is: \tmail  [<username> <instance id>]");
	  fprintf(stderr,"[-editor <editor>]\n\t\t[-file <file name>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }
  
  if(fileP == (char *) NULL)
    {
      make_temp_name(file);
      fileP = file;
      temp = TRUE;
    }

  status = t_mail(&Request,fileP, editorP);
  if(temp)
    (void) unlink(file);
  
  return(status);
}


