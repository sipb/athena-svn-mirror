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
 *      $Id: olcm.c,v 1.7 1991-09-22 11:44:37 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcm/olcm.c,v 1.7 1991-09-22 11:44:37 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcm.h>
#include <olc/olc.h>

#include <stdio.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <ctype.h>
#include <syslog.h>
#ifdef KERBEROS
#include <krb.h>
#endif

/* Need to set timeout to link with client library */
int select_timeout=300;


#ifdef NEEDS_ERRNO_DEFS
extern int	errno;
extern char	*sys_errlist[];
extern int	sys_nerr;
#endif

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
  char orig_address[BUFSIZ];
  char buf[BUFSIZ];
  char mailbuf[BUFSIZ];
  char *p,*end;
  ERRCODE errcode;
  int status;
  int fd;
  FILE *f, *mail;
  int c;
  extern char *optarg;
  extern int optind;
  REQUEST Request;
  int instance;

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
	strcpy(orig_address,p);
	end = index(orig_address,'>');
	if (end != NULL) *end = '\0';
	strncpy(username,p,8);
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

#ifdef PUTENV
  sprintf(buf, "OLCD_HOST=%s", server);
  putenv(buf);
#else
  (void)setenv("OLCD_HOST", server, 1);
#endif

  OInitialize();

  if (fill_request(&Request) != SUCCESS) {
    syslog(LOG_ERR,"fill_request unsuccessful");
    exit(1);
  }

  (void) strcpy(Request.target.username, username);

  instance = Request.requester.instance;
  set_option(Request.options,VERIFY);
  status = OAsk(&Request,topic,NULL);
  unset_option(Request.options, VERIFY);

  switch(status)
    {
    case SUCCESS:
      break;

    case INVALID_TOPIC:
      syslog(LOG_ERR,"topic %s is invalid", topic);
      fprintf(stderr,"unable to enter a question via mail\n");
      exit(1);
      break;

    case ERROR:
      syslog(LOG_ERR,"error contacting server %s", server);
      fprintf(stderr,
         "An error has occurred while contacting server.  Please try again.\n");
      exit(1);
      break;

    case CONNECTED:
      syslog(LOG_ERR,"user %s already connected", username);
      fprintf(stderr, "You are already connected to olc.\n");
      exit(1);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,"You are not allowed to ask OLC questions.\n");
      fprintf(stderr,"Does defeat the purpose of things, doesn't it?\n");
      syslog(LOG_ERR,"user %s: permission denied", username);
      exit(1);
      break;

    case MAX_ASK:
    case ALREADY_HAVE_QUESTION:
      syslog(LOG_ERR,"user %s already asking a question", username);
      fprintf(stderr, "You are already asking a question. \n");
      exit(1);
      break;

    case HAS_QUESTION:
#if 0
      syslog(LOG_ERR,"user %s already asking a question...splitting", username);
      fprintf(stderr,
	"Your current instance is busy, creating another one for you.\n");      
      set_option(Request.options, SPLIT_OPT);
      t_ask(Request,topic,filename);
#else
      exit(1);
#endif
      break;

    case ALREADY_SIGNED_ON:
      syslog(LOG_ERR,"user %s already a consultant on this instance", username);
      fprintf(stderr,
              "You cannot be a user and consult in the same instance.\n");
      exit(1);
      break;

    default:
#if 0
      if((status = handle_response(status, &Request))!=SUCCESS)
        {
          if(OLC)
            exit(1);
          else
            return(ERROR);
        }
#else
      syslog(LOG_ERR,"OAsk (VERIFY) Error status %d\n", status);
      fprintf(stderr,"Error status %d\n", status);
      exit(1);
#endif
      break;
    }

    status = OAsk(&Request,topic,filename);
    (void) unlink(filename);
  switch(status)
    {
    case NOT_CONNECTED:
      sprintf(mailbuf, "Your question has been received and will be forwarded to the first\navailable consultant.");
      break;
    case CONNECTED:
      sprintf(mailbuf, "Your question has been received and a consultant is reviewing it now.");
      break;
    default:
      syslog(LOG_ERR,"OAsk Error status %d\n", status);
      fprintf(stderr,"Error status %d\n", status);
      exit(1);
      break;
    }

#if 0
  sprintf(buf, "-topic %s %s -file %s", topic, username, filename);
  printf("buf is %s\n", buf);
  errcode = do_olc_ask(&buf);
  if (errcode != SUCCESS) {
    syslog(LOG_ERR,"\"%s\" exited with status %d",buf,status);
    exit(1);
  }
#endif

#ifdef KERBEROS
  dest_tkt();
#endif

  sprintf(buf, "/usr/lib/sendmail -t");
  mail = popen(buf, "w");
  if (mail) {
    fprintf(mail, "To: %s\n",orig_address);
    fprintf(mail, "From: \"OLC Server\" <olc-test@matisse.local>\n");
    fprintf(mail, "Subject: Your OLC question\n");

    fprintf(mail, "%s\n", mailbuf);
    fprintf(mail, "You will receive mail from a consultant when it has been answered.\nYou may also continue this question by using olc or xolc.\n");
    fprintf(mail, "Do not reply directly to this message; it was automatically generated.\n");
    fprintf(mail, ".\n");
    pclose(mail);
  } else {
    syslog(LOG_ERR,"popen: /usr/lib/sendmail failed: %m");
  }
}


