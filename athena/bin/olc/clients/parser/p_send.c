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
 *	$Id: p_send.c,v 1.18 1999-03-06 16:48:03 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_send.c,v 1.18 1999-03-06 16:48:03 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

ERRCODE
do_olc_send(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  char file[NAME_SIZE];
  char editor[NAME_SIZE];
  int temp = FALSE;

  file[0] = '\0';
  editor[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  arguments++;
  while(*arguments != (char *) NULL)
    {
      if (string_equiv(*arguments, "-editor",max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL) {
	    (void) strcpy(editor, *arguments);
	    arguments++;
	  }
	  else
	    (void) strcpy(editor, NO_EDITOR);
	  continue;
	}

      if(string_equiv(*arguments, "-file",max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL)
	    {
	      (void) strcpy(file, *arguments);
	      arguments++;
	    }
	  continue;
	}
      
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    {
	      printf("Usage is: \tsend [-editor <editor>] ");
	      printf("[-file <file name>]\n");
	    }
	  else
	    {
	      printf("Usage is: \tsend  [<username> <instance id>] ");
	      printf("[-editor <editor>]\n\t\t[-file <file name>] ");
	      printf("[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
    }
  
  if(string_eq(file, ""))
    {
      make_temp_name(file);
      temp = TRUE;
    }

  status = t_reply(&Request,file,editor);
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
  char file[NAME_SIZE];
  char editor[NAME_SIZE];
  int temp = FALSE;

  file[0] = '\0';
  editor[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  arguments++;
  while(*arguments != (char *) NULL)
    {
      if (string_equiv(*arguments, "-editor", max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL) {
	    (void) strcpy(editor, *arguments);
	    arguments++;
	  }
	  else
	    (void) strcpy(editor, NO_EDITOR);
	  continue;
	}
   
      if(string_equiv(*arguments, "-file", max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL)
	    {
	      (void) strcpy(file, *arguments);
	      arguments++;
	    }
	  continue;
	}
   
      if(string_equiv(*arguments, "-private", max(strlen(*arguments),2)))
	{
	  set_option(Request.options,PRIV_COMMENT_OPT);
	  arguments++;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tcomment  [<username> <instance id>] ");
	  printf("[-editor <editor>]\n\t\t[-file <file name>] ");
	  printf("[-instance <instance id>]\n");
	  printf("\t\t[-private]\n");
	  return(ERROR);
	}
    }
  
  if(string_eq(file, ""))
    {
      make_temp_name(file);
      temp = TRUE;
    }

  status = t_comment(&Request,file,editor);
  if(temp)
    (void) unlink(file);

  return(status);
}



ERRCODE 
do_olc_mail(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  char editor[NAME_SIZE];
  char smargs[NAME_SIZE][NAME_SIZE];
  char *smargsP[NAME_SIZE];
  int status;
  int checkhub = 0;
  int noedit = 0;
  int header = TRUE;
  int i = 0;

  file[0] = '\0';
  strcpy(editor, NO_EDITOR);

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  smargsP[0] = (char *) NULL;

  arguments++;
  while(*arguments != (char *) NULL)
    {
      if (string_equiv(*arguments, "-editor", max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL) {
	    (void) strcpy(editor, *arguments);
	    arguments++;
	  }
	  else
	    (void) strcpy(editor,NO_EDITOR);
	  continue;
	}

      if (string_equiv(*arguments, "-file", max(strlen(*arguments),2)))
	{
	  arguments++;
	  if(*arguments != (char *) NULL)
	    {
	      (void) strcpy(file, *arguments);
	      arguments++;
	    }
	  continue;
	}

      if (string_equiv(*arguments, "-checkhub", max(strlen(*arguments),2)))
	{
	  checkhub = TRUE;
	  arguments++;
	  continue;
	}

      if (string_equiv(*arguments, "-noedit", max(strlen(*arguments),2)))
	{
	  noedit = TRUE;
	  arguments++;
	  continue;
	}

      if (string_equiv(*arguments, "-noheader", max(strlen(*arguments),2)))
	{
	  header = FALSE;
	  arguments++;
	  continue;
	}

      if(string_equiv(*arguments, "-smopt", max(strlen(*arguments),2)))
	{
	  if(arguments[1] && (*arguments[1] == '\\'))
	    {
	      ++arguments;
	      if(*arguments != (char *) NULL)
		{
		  for(i=0; *arguments != (char *) NULL; arguments++)
		    {
		      if(*arguments[0] == '\\')
			(*arguments)++;
		      
		      if(i >= NAME_SIZE-1)
			{
			  fprintf(stderr,"Too many options...\n");
			  break;
			}
		      
		      if(strlen(*arguments) >= (NAME_SIZE))
			fprintf(stderr, "Name too long. Continuing...\n");
		      else
			{
			  strncpy(smargs[i], *arguments, NAME_SIZE-1);
			  *smargs[i+1] = '\0';
			  smargsP[i] = &smargs[i][0];
			}
		      
		      if((*(arguments+1)) && (*arguments[1] == '-'))
			break;
		      if(!(*arguments+1))
			break;
		      
		      ++i;
		    }
		  smargsP[i] = &smargs[i][0];
		  continue;
		}
	    }
	  else
	    {
	      if((arguments[1] != (char *) NULL) && (*arguments[1] == '-'))
		continue;
	      else
		if(arguments[1] != (char *) NULL)
		  ++arguments;
	    }
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      arguments += num_of_args;		/* HACKHACKHACK */
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tmail  [<username> <instance id>] ");
	  printf("[-editor <editor>]\n\t\t[-file <file name>] ");
	  printf("[-smopt <[\\-]sendmail options>] [-checkhub]\n");
	  printf("\t\t[-noedit] [-instance <instance id>] ");
	  printf(" [-noheader]\n");
	  return(ERROR);
	}
    }
  
  status = t_mail(&Request, file, editor, smargsP, checkhub,noedit,header);
  
  return(status);
}


