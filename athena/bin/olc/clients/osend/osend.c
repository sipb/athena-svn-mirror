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
 *	$Id: osend.c,v 1.3 1997-04-30 17:58:38 ghudson Exp $
 *	$Author: ghudson $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/osend/osend.c,v 1.3 1997-04-30 17:58:38 ghudson Exp $";
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
  char *inst = "olc";
  char *buf, *config, *cf_server;
  char username[9];
  struct hostent *hp = NULL;
  extern int optind;
  extern char *optarg;
  int c;

  select_timeout = 600;

  config = getenv("OLXX_CONFIG");
  if (! config)
    config = OLC_CONFIG_PATH;

  while ((c = getopt(argc, argv, "C:f:s:i:")) != EOF)
    switch(c) {
    case 'C':	/* -C path: set path for the configuration file */
      config = optarg;
      break;
    case 'i':	/* -i inst: use service INST (eg. "-i olta") */
      inst = optarg;
      /* Downcase the instance, so we can use it as the cfg file name. */
      for (buf=inst ; *buf ; buf++)
	*buf = tolower(*buf);
      break;
    case 'f':	/* -f file: save output in FILE */
      fd = open(optarg, O_RDONLY, 0);
      if (fd < 0) {
	perror("osend: error opening file");
	exit(1);
      }
      close(0);
      dup2(fd,0);
      break;
    case 's':	/* -s host: use server HOST */
      hp = gethostbyname(optarg);
      if (hp == NULL) {
	fprintf(stderr,"%s: unknown host %s\n",argv[0],optarg);
	exit(1);
      }
      break;
    case '?':	/* unknown option... */
    default:
      usage();
    }

  instance = 0;

  if (((argc - optind) > 2) || (argc == optind)) {
    usage();
  }

  strcpy(username,argv[optind++]);
  if ((argc - optind) == 1)
    instance = atoi(argv[optind]);

  if (incarnate(inst, config) == FATAL) {
    /* Fatal problem.  Messages indicating causes were already displayed... */
    exit(1);
  }

#ifdef HESIOD
  if (hp == NULL) {
    olc_servers = hes_resolve(inst, OLC_SERV_NAME);
    if (olc_servers == NULL) {
      fprintf(stderr,"%s: Unable to get hesiod information for %s.%s\n",
	      inst, argv[0], OLC_SERV_NAME);
      exit(1);
    }

    hp = gethostbyname(olc_servers[0]);
    if (hp == NULL) {
      fprintf(stderr,"%s: Unknown host %s\n",argv[0],olc_servers[0]);
      exit(1);
    }
  }
#endif /* HESIOD */
  if (hp == NULL) {
    cf_server = client_hardcoded_server();
    if (cf_server) {
      hp = gethostbyname(cf_server);
      if (hp == NULL) {
	fprintf(stderr,"%s: Unknown host %s\n",argv[0],cf_server);
	exit(1);
      }
    }
  }
  if (hp == NULL) {
    fprintf(stderr,"%s: no server specified\n",argv[0]);
    exit(1);
  }

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
  fprintf(stderr, "Usage: osend [options] username [instance]\n"
		  "options:\n"
		  "   -f filename   save output in FILENAME\n"
		  "   -i service    use SERVICE (olc/olta/owl)\n"
		  "   -s host       use server HOST\n"
		  "   -C path       use PATH to search for configuration\n");
  exit(1);
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

  memset(req, 0, sizeof(REQUEST));

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  uid = getuid();
  pwd = getpwuid(getuid());
  req->requester.instance = 0;
  req->requester.uid = pwd->pw_uid;
  (void) strncpy(req->requester.username, pwd->pw_name, LOGIN_SIZE);
  gethostname(req->requester.machine, sizeof(req->requester.machine));

#ifdef KERBEROS
  status = krb_get_cred("krbtgt",REALM,REALM, &k_cred);
  if(status == KSUCCESS)
    {
      (void) strncpy(req->target.username, k_cred.pname, LOGIN_SIZE);
      (void) strncpy(req->requester.username, k_cred.pname, LOGIN_SIZE);
    }
  else
    fprintf(stderr,"%s\n",krb_err_txt[status]);
#endif /* KERBEROS */

  return(SUCCESS);
}

