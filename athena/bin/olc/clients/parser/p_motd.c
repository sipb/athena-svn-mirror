/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v $
 *      $Author: vanharen $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_motd.c,v 1.7 1990-01-17 02:54:12 vanharen Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_motd(arguments)
     char **arguments;
{
  REQUEST Request;
  char file[NAME_SIZE];
  int status;
  int save_file = 0;
  int type=0;
  int change_flag = 0;
  char editor[NAME_SIZE];
  char *editorP = (char *) NULL;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  make_temp_name(file);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_eq(*arguments, ">") || string_equiv(*arguments,"-file",
						    max(strlen(*arguments),2)))
	{
          ++arguments;
	  unlink(file);
	  if(*arguments == NULL)
            {
	      file[0] = '\0';
              get_prompted_input("Enter file name: ",file);
	      if(file[0] == '\0')
		return(ERROR);
            }
	  else
	    (void) strcpy(file,*arguments);
	  
	  save_file = TRUE;
	  continue;
	}

       if (string_equiv(*arguments, "-editor",max(strlen(*arguments),2)))
        {
          ++arguments;
          editorP = editor;
          if(*arguments != (char *) NULL)
            (void) strcpy(editorP, *arguments);
          else
            (void) strcpy(editorP, NO_EDITOR);
        }

      if(string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
	  change_flag = TRUE;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
	
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tmotd  [-file <filename>] [-change] ");
	  printf("[-editor <editor>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  if(!change_flag)
    status = t_get_motd(&Request,type,file,!save_file);
  else
    status = t_change_motd(&Request,type,file,editorP, !save_file);

  if(!save_file)
    (void) unlink(file);
  return(status);
}
  
