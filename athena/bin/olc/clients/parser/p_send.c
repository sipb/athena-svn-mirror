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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_send.c,v 1.5 1989-11-17 14:08:39 tjcoppet Exp $";
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
	  editorP = editor;
	  if(*arguments != (char *) NULL)
	    (void) strcpy(editorP, *arguments);
	  else
	    (void) strcpy(editorP, NO_EDITOR);
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
	  editorP = editor;
	  if(*arguments != (char *) NULL)
	    (void) strcpy(editorP, *arguments);
	  else
	    (void) strcpy(editorP, NO_EDITOR);
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
	  printf("Usage is: \tcomment  [<username> <instance id>] ");
	  printf("[-editor <editor>]\n\t\t[-file <file name>] ");
	  printf("[-instance <instance id>]\n");
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
  char smargs[NAME_LENGTH][NAME_LENGTH];
  char *smargsP[NAME_LENGTH];
  int status;
  int temp = FALSE;
  int checkhub = 0;
  int i = 0;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  smargsP[0] = (char *) NULL;

  for(++arguments; *arguments != (char *) NULL; arguments++)
    {
      if (string_equiv(*arguments, "-editor", max(strlen(*arguments),2)))
	{
	  ++arguments;
	  editorP = editor;
	  if(*arguments != (char *) NULL)
	    (void) strcpy(editorP, *arguments);
	  else
	    (void) strcpy(editorP,NO_EDITOR);
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

      if (string_equiv(*arguments, "-checkhub", max(strlen(*arguments),2)))
	{
	  checkhub = TRUE;
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
			*(*arguments)++;
		      
		      if(i >= NAME_LENGTH-1)
			{
			  fprintf(stderr,"Too many options...\n");
			  break;
			}
		      
		      if(strlen(*arguments) >= (NAME_LENGTH))
			fprintf(stderr, "Name too long. Continuing...\n");
		      else
			{
			  strncpy(smargs[i], *arguments, NAME_LENGTH-1);
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

      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tmail  [<username> <instance id>] ");
	  printf("[-editor <editor>]\n\t\t[-file <file name>] ");
	  printf("[-smopt <[\\-]sendmail options>] [-checkhub]\n");
	  printf("\t\t[-instance <instance id>]\n");
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

  status = t_mail(&Request,fileP, editorP, smargsP, checkhub);
  if(temp)
    (void) unlink(file);
  
  return(status);
}


