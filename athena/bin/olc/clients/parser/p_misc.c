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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_misc.c,v $
 *	$Id: p_misc.c,v 1.5 1990-07-16 08:21:27 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_misc.c,v 1.5 1990-07-16 08:21:27 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_load_user(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
            if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tload  [username]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  return(t_load_user(&Request));
}


do_olc_dump(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;            
  char file[NAME_SIZE];
  int save_file = 0;
  int type = OLC_DUMP;

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

      if(string_equiv(*arguments,"-requests",max(strlen(*arguments),2)))
	{
	  type = OLC_DUMP_REQ_STATS;
	  continue;
	}

      if(string_equiv(*arguments,"-questions",max(strlen(*arguments),2)))
	{
	  type = OLC_DUMP_QUES_STATS;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tdump  [-requests] [-questions]");
	  printf(" [-file <filename>]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  status = t_dump(&Request,type,file);
    if(!save_file)
    (void) unlink(file);
  return(status);
}





do_olc_dbinfo(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;            
  char file[NAME_SIZE];
  int save_file = 0;
  int change = 0;

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
        }
      else
	if(string_equiv(*arguments,"-change",max(strlen(*arguments),2)))
	  change = TRUE;
	else
	  {
	    arguments = handle_argument(arguments, &Request, &status);
          if(status)
            return(ERROR);
	  }

      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \tdbinfo [-file <filename>] ");
	  printf("[-change]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  if(!change)
    status = t_dbinfo(&Request,file);
  else
    status = t_change_dbinfo(&Request);

  if(!save_file)
    (void) unlink(file);
  return(status);
}




do_olc_happy(arguments)
     char **arguments;
{
  REQUEST Request;
  char *argv0;
  int status;

  argv0 = *arguments;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
       
      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \t%s\n",argv0);
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  printf("%s.\n",happy_message());
  return(SUCCESS);
}

do_olc_home(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
       
      if(arguments == (char **) NULL)   /* error */
        {
          printf("Usage is: \thome [<username>]\n");
          return(ERROR);
        }

      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }


  if(string_eq(Request.target.username,"carla"))
    printf("227-9517\n");
  else
    if(string_eq(Request.target.username,"vanharen"))
      printf("248-0946\n");
    else
      if(string_eq(Request.target.username,"tjcoppet"))
	printf("248-0946\n");
      else
	if(string_eq(Request.target.username,"srz"))
	  printf("577-1685\n");
	else
	  if(string_eq(Request.target.username,"jon"))
	    printf("577-1685\n");
	  else
	    if(string_eq(Request.target.username,"hoffmann"))
	      printf("484-2098\n");
	    else
	      printf("No home information for %s.\n",Request.target.username);

  return(SUCCESS);
}

static int swiss = 0;

do_olc_cheese(arguments)
     char **arguments;
{
  swiss ++;
  switch(random() % swiss)
    {
    case 0:
      printf("You are standing alone.\n");
      break;
    case 1: 
      printf("The window is slightly ajar. You are standing alone.\n");
      break;
    case 2:
      printf("The door slammed shut. There is a lantern on the wall and a magazine\n");
      printf("on the table. Suddenly you hear laughter from upstairs and the lights\n");
      printf("go out. The wind is making eerie noises through the slightly ajar window\n");
      printf("You are standing alone.\n");
      break;
    case 3:
      break;
    }
  if(swiss > 10)
    swiss = 0;
}
