/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "osend".
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1992 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/osend/osend.c,v $
 *	$Id: osend.c,v 1.1 1994-09-18 05:07:40 cfields Exp $
 *	$Author: cfields $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/osend/osend.c,v 1.1 1994-09-18 05:07:40 cfields Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>

#include <olc/olc.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static void usage P((void ));
static int init_request P((REQUEST *req ));
#undef P

int select_timeout;
extern char DaemonHost[];

main(argc,argv)
     int argc;
     char **argv;
{
  REQUEST Request;
  int fd;
  int instance;
  char **olc_servers;
  char *inst = "OLC";
  char username[9];
  struct hostent *hp = NULL;
  extern int optind;
  extern char *optarg;
  int c;

  select_timeout = 600;

  while ((c = getopt(argc, argv, "f:s:i:")) != EOF)
    switch(c) {
    case 'i':
      inst = optarg;
      break;
    case 'f':
      if ((fd = open(optarg, O_RDONLY, 0)) < 0) {
	perror("osend: error opening file");
	exit(1);
      }
      close(0);
      dup2(fd,0);
      break;
    case 's':
      if ((hp = gethostbyname(optarg)) == NULL) {
	fprintf(stderr,"%s: unknown host %s\n",argv[0],optarg);
	exit(1);
      }
      break;
    case '?':
    default:
      usage();
      exit(1);
    }

  instance = 0;

  if (((argc - optind) > 2) || (argc == optind)) {
    usage();
    exit(1);
  }

  strcpy(username,argv[optind++]);
  if ((argc - optind) == 1)
    instance = atoi(argv[optind]);

  if (hp == NULL) {
#ifdef HESIOD
    if ((olc_servers = hes_resolve(inst,"SLOC")) == NULL) {
      fprintf(stderr,"%s: Unable to get hesiod information for %s/SLOC\n",
	      inst,argv[0]);
      exit(1);
    }
    
    if ((hp = gethostbyname(olc_servers[0])) == NULL) {
      fprintf(stderr,"%s: Unknown host %s\n",argv[0],olc_servers[0]);
      exit(1);
    }
    strncpy(DaemonHost,hp->h_name,MAXHOSTNAMELEN);
#else /* ! HESIOD */
    fprintf(stderr,"%s: no server specified\n",argv[0]);
    exit(1);
#endif /* HESIOD */
  }
  else
    strncpy(DaemonHost,hp->h_name,MAXHOSTNAMELEN);
    
  if (init_request(&Request) != SUCCESS) {
    fprintf(stderr,"Error filling request\n");
    exit(1);
  }

  strcpy(Request.target.username,username);
  Request.target.instance = instance;
  OSend(&Request,OLC_SEND,"/tmp/foo");
  exit(0);
}


static void usage()
{
  fprintf(stderr,"Usage: osend [-f filename] [-s server] username [instance]\n");
}

static int
init_request(req)
     REQUEST *req;
{
#ifdef KERBEROS
  CREDENTIALS k_cred;
  int status;
  int uid;
#endif /* KERBEROS */
  struct passwd *pwd;

  bzero(req, sizeof(REQUEST));

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  uid = getuid();
  pwd = getpwuid(getuid());
  req->requester.instance = 0;
  req->requester.uid = pwd->pw_uid;
  (void) strncpy(req->requester.username, pwd->pw_name, LOGIN_SIZE);
  gethostname(req->requester.machine, sizeof(req->requester.machine));

#ifdef KERBEROS
  if((status = krb_get_cred("krbtgt",REALM,REALM, &k_cred)) == KSUCCESS)
    {
      (void) strncpy(req->target.username, k_cred.pname, LOGIN_SIZE);
      (void) strncpy(req->requester.username, k_cred.pname, LOGIN_SIZE);
    }
  else
    fprintf(stderr,"%s\n",krb_err_txt[status]);
#endif /* KERBEROS */

  return(SUCCESS);
}

