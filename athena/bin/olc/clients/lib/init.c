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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v $
 *	$Id: init.c,v 1.7 1990-07-16 08:15:26 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v 1.7 1990-07-16 08:15:26 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

extern int OLC;

#ifdef OLZ
#undef OLC_SERVICE
#define OLC_SERVICE "olz"
#endif

ERRCODE
OInitialize()
{
  int uid;
  struct passwd *pwent;
  char hostname[LINE_SIZE];  /* Name of local machine. */
  char *h;
  struct hostent *host;

#ifdef LAVIN
  char type[BUF_SIZE];
#endif LAVIN

  h = getenv ("OLCD_HOST");

  if (!h) {

#ifdef HESIOD
      char **hp;

      if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL) {	

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
	  h = *hp;

#endif HESIOD

      if (!h) {
	  fprintf (stderr, "Can't find OLC server host!\n");
	  exit (ERROR);
      }
  }

  strcpy (DaemonHost, h);

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
#if defined(COURSE) && !defined(OLZ)
  strcpy(DaemonHost,"nemesis.mit.edu");
#endif

  uid = getuid();
  pwent = getpwuid(uid);
  if(pwent == (struct passwd *) NULL)
    {
       fprintf(stderr, "Warning: unable to get passwd information.\n");
       fprintf(stderr, "Try logging out and in again.\n");
       strcpy(User.username, "nobody");
       strcpy(User.realname, "No passwd info"); 
     }
  else
     {
       (void) strcpy(User.username, pwent->pw_name);
       (void) strcpy(User.realname, pwent->pw_gecos);
     }

  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';
  
  gethostname(hostname, LINE_SIZE);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));     
  User.uid = uid;
 
  expand_hostname(DaemonHost,INSTANCE,REALM);
  return(SUCCESS);
}
