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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v 1.2 1989-12-22 16:03:04 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

extern int OLC;

ERRCODE
OInitialize()
{
  int uid;
  struct passwd *pwent;
  char hostname[LINE_LENGTH];  /* Name of local machine. */
  struct hostent *host;

#ifdef LAVIN
  char type[BUF_SIZE];
#endif LAVIN

#ifdef HESIOD
  char **hp;

#ifdef OLZ
  if ((hp = hes_resolve("olz",OLC_SERV_NAME)) == NULL)
#else
  if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
#endif
    {	

      fprintf(stderr,
	      "Unable to get name of OLC server host from the Hesiod nameserver.\n");
      fprintf(stderr, 
	      "This means that you cannot use OLC at this time. Any problems \n");
      fprintf(stderr,
	      "you may be experiencing with your workstation may be the result of this\n");
      fprintf(stderr,
	      "problem. \n");
      
      exit(ERROR);
    }
  else 
    (void) strcpy(DaemonHost, *hp);

#endif HESIOD

#ifdef LAVIN
  if(OLC)
    printf("Please choose which type of question you wish to ask:\n\n");
  else
    printf("Please choose which queue you would like to enter:\n\n");

  while(1)
    {
      if(OLC)
        printf("\ttype\t description\n\t----\t -----------\n");
      else
        printf("\tqueue\t description\n\t-----\t -----------\n");
      printf("\tathena\t questions concerning general athena topics\n");
      printf("\t\t (topics of scribe, emacs, unix, etc...)\n");
      printf("\tcourse\t questions for a specific course\n");
      printf("\t\t (topics of 6.170, 1.00, etc...)\n\n");
      type[0] = '\0';
      if(OLC)
         get_prompted_input("type: ", type);
      else
         get_prompted_input("queue: ", type);
      if(*type == '\0')
	exit(0);
      if(string_equiv(type,"course",max(strlen(type),1)))
	{
	  strcpy(DaemonHost,"nemesis.mit.edu");
	  break;
	}
      if(string_equiv(type,"athena",max(strlen(type),1)))
	break;
    }
#endif LAVIN
#ifdef COURSE
  strcpy(DaemonHost,"nemesis.mit.edu");
#endif

  uid = getuid();
  pwent = getpwuid(uid);
  if(pwent == (struct passwd *) NULL)
    {
       fprintf(stderr,"warning: unable to get passwd information\n");
       if(OLC)
          printf("Try logging out and in again.\n");
       strcpy(User.username,"nobody");
       strcpy(User.realname,"No passwd info"); 
     }
  else
     {
       (void) strcpy(User.username, pwent->pw_name);
       (void) strcpy(User.realname, pwent->pw_gecos);
     }

  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';
  
  gethostname(hostname, LINE_LENGTH);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));     
  User.uid = uid;
 
  expand_hostname(DaemonHost,INSTANCE,REALM);
  return(SUCCESS);
}
