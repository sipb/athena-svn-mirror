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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v $
 *	$Id: init.c,v 1.14 1991-03-28 16:26:09 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v 1.14 1991-03-28 16:26:09 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>
#include <strings.h>

extern int OLC;

ERRCODE
OInitialize()
{
  int uid;
  struct passwd *pwent;
  char hostname[LINE_SIZE];  /* Name of local machine. */
  char *h;
  struct hostent *host;

  h = getenv ("OLCD_HOST");

#ifdef HESIOD
  if (!h) {
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
    }
#else
  if (!h) {
    fprintf (stderr, "Can't find OLC server host!\n");
    exit (ERROR);
  }
#endif /* HESIOD */

  strcpy (DaemonHost, h);

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

  {
    char *cp;
    if ((cp = index(User.realname, ',')) != 0)
      *cp = '\0';
  }
  
  gethostname(hostname, LINE_SIZE);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));     
  User.uid = uid;
 
#ifdef KERBEROS
  expand_hostname(DaemonHost,INSTANCE,REALM);
#endif
  return(SUCCESS);
}
