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
 *	$Id: init.c,v 1.22 1997-04-30 17:35:07 ghudson Exp $
 *	$Author: ghudson $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/init.c,v 1.22 1997-04-30 17:35:07 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <netdb.h>
#include <string.h>

ERRCODE
OInitialize()
{
  int uid;
  struct passwd *pwent;
  char hostname[MAXHOSTNAMELEN];  /* Name of local machine. */
  char *h;
  char *inst;
  struct hostent *host;
#ifdef HESIOD
  int need_to_free = 0;
#endif

  h = (char *) getenv ("OLCD_HOST");
  inst = (char *) getenv("OLCD_INST");
  if (!inst) {
    inst = client_service_name();
  }

#ifdef HESIOD
  if (!h) {
      char **hp;

      hp = hes_resolve(inst,OLC_SERV_NAME);
      if (hp == NULL) {	

	  fprintf(stderr,
		  "Unable to get name of %s server host from the Hesiod nameserver.\n",inst);
	  fprintf(stderr, 
		  "This means that you may be unable to use %s at this time.  Any problems\n",inst);
	  fprintf(stderr,
		  "you may be experiencing with your workstation may be the result of this\n");
	  fprintf(stderr,
		  "problem. \n");
      
	  h = client_hardcoded_server();
      }
      else
	  need_to_free = 1;
	  h = *hp;
    }
#else
  if (!h) {
    h = client_hardcoded_server();
  }
#endif /* HESIOD */

  strcpy (DaemonHost, h);
#ifdef HESIOD
  if (need_to_free)
    free(h);
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
      
  {
    char *cp = strchr(User.realname, ',');
    if (cp != NULL)
      *cp = '\0';
  }
      
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    fprintf(stderr,"Unable to get host name of this machine; this may be the cause\n");
    fprintf(stderr,"of the problems you may currently be having.\n");
    strcpy(hostname,"unknown-host");
  } else {
    host = gethostbyname(hostname);
    if (host == (struct hostent *)NULL) {
      fprintf(stderr,"Unable to get host by name for this host, `%s'\n",
	      hostname);
    }
    (void) strcpy(User.machine, (host ? host->h_name : hostname));
  }
  User.uid = uid;
 
#ifdef KERBEROS
  expand_hostname(DaemonHost,INSTANCE,REALM);
#endif
  return(SUCCESS);
}
