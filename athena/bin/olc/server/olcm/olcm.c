/*
 * This file is part of the OLC On-Line Consulting System.
 * It receives incoming mail to ship off to OLC
 *
 *      Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcm/olcm.c,v $
 *      $Id: olcm.c,v 1.4 1991-04-10 00:43:09 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcm/olcm.c,v 1.4 1991-04-10 00:43:09 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcm.h>

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <ctype.h>
#include <syslog.h>
#ifdef KERBEROS
#include <krb.h>
#endif

extern int errno;
extern char *sys_errlist[];

#ifdef KERBEROS
void
get_krb_tkt(ident)
     char *ident;
{
  char serv[SNAME_SZ];
  char inst[INST_SZ];
  char realm[REALM_SZ];
  char tkt_filename[MAXPATHLEN];
  int ret;

  serv[0] = '\0';
  inst[0] = '\0';
  realm[0] = '\0';

  if (kname_parse(serv,inst,realm,ident) != KSUCCESS) {
    syslog(LOG_ERR,"get_krb_tkt: error parsing %s",ident);
    fprintf(stderr,"System error: Could not parse %s\n",ident);
    exit(1);
  }

  if (realm[0] == '\0')
    strncpy(realm,DFLT_REALM,REALM_SZ);
  strcpy(tkt_filename,"/tmp/olcm_tkt_XXXXXX");
  mktemp(tkt_filename);
  setenv("KRBTKFILE",tkt_filename,1);

  dest_tkt();
  ret = krb_get_svc_in_tkt(serv, inst, realm, "krbtgt", realm, 1,
			   SRVTAB_FILE);
  if (ret != KSUCCESS) {
    syslog(LOG_ERR,"get_krb_tkt: %s",krb_err_txt[ret]);
    fprintf(stderr,"Could not get tickets for operation: %s\n",
	    krb_err_txt[ret]);
    exit(1);
  }
}
#endif

main(argc,argv)
     int argc;
     char **argv;
{
  char filename[MAXPATHLEN];
  char server[MAXHOSTNAMELEN];
  char topic[BUFSIZ];
#ifdef KERBEROS
  char ident[SNAME_SZ+INST_SZ+REALM_SZ+4];
#endif
  char username[9];
  char buf[BUFSIZ];
  char *p,*end;
  int status;
  int fd;
  FILE *f;
  char c;
  extern char *optarg;
  extern int optind;

#if defined(ultrix)
#ifdef LOG_CONS
  openlog ("olcm", LOG_CONS | LOG_PID);
#else
  openlog ("olcm", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
  openlog ("olcm", LOG_CONS | LOG_PID,SYSLOG_FACILITY);
#else
  openlog ("olcm", LOG_PID, SYSLOG_FACILITY);
#endif /* LOG_CONS */
#endif /* ultrix */

  username[0] = '\0';
  strcpy(topic,DFLT_TOPIC);
  strcpy(server,DFLT_SERVER);

  while ((c = getopt(argc, argv, "s:t:k:")) != EOF)
    switch (c) {
    case 's':
      strncpy(server,optarg,MAXHOSTNAMELEN);
      server[MAXHOSTNAMELEN-1] = '\0';
      break;
    case 't':
      strncpy(topic,optarg,BUFSIZ);
      topic[BUFSIZ-1] = '\0';
      break;
#ifdef KERBEROS
    case 'k':
      strncpy(ident,optarg,SNAME_SZ+INST_SZ+REALM_SZ+4);
      break;
#endif
    default:
#ifdef KERBEROS
      fprintf(stderr,"Usage: %s [-t topic] [-s server] [-k service.instance@realm]\n",
	      argv[0]);
#else
      fprintf(stderr,"Usage: %s [-t topic] [-s server]\n",argv[0]);
#endif
      exit(1);
    }

  strcpy(filename,"/tmp/olcm_XXXXXX");
  mktemp(filename);

  if ((fd = open(filename,O_CREAT|O_WRONLY|O_EXCL,0600)) < 0) {
    syslog(LOG_ERR,"olcm: opening file %s: %m",filename);
    exit(1);
  }

  if ((f = fdopen(fd,"w+")) == NULL) {
    syslog(LOG_ERR,"olcm: fdopening file %s: %m",filename);
    exit(1);
  }

  while(fgets(buf,BUFSIZ,stdin) != NULL) {
    /* Output to new file */
    fputs(buf,f);
    
    /* find the username 
     * Handles usernames of the form    user@foo.bar.baz
     * or				"John Q. User" <user@foo.bar.baz>
     * This is a gross hack, but to do it right would involve writing a real
     * parser...
     */

    if ((username[0] == '\0') && (strncmp(buf,"From:",5) == 0)) {
      /* Found from line, get username */

      p = &buf[5];
      while(isascii(*p) && isspace(*p))
	p++;

      end = index(p,'<');
      if (end == NULL)
	end = p;
      else {
	end++;
	p = end;
      }

      while(isascii(*end) && isalnum(*end))
	end++;

      if (end != p) {
	*end = '\0';
	strcpy(username,p,8);
	username[8] = '\0';
      }
    }
  }

  fclose(f);

  if (username[0] == '\0') {
    /* didn't find one, make up one by default */
    strncpy(username,DFLT_USERNAME,8);
    username[8] = '\0';
  }

  /* invoke olcr to deal with entering question */

#ifdef KERBEROS
  /* Set ticket file correctly */
  get_krb_tkt(ident);
#endif
  sprintf(buf,"%s -server %s ask -topic %s -file %s %s", OLCR_PATH, server,
	  topic, filename, username);
  syslog(LOG_DEBUG,"Mailed in question from \"%s\" on \"%s\"",topic,username);
  status = system(buf);
  if (status != 0) {
    syslog(LOG_ERR,"\"%s\" exited with status %d",buf,status);
    exit(1);
  }
#ifdef KERBEROS
  dest_tkt();
#endif

  unlink(filename);
  exit(1);
}


