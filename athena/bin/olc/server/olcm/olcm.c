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
 *      $Id: olcm.c,v 1.9 1991-11-06 15:46:40 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcm/olcm.c,v 1.9 1991-11-06 15:46:40 lwvanels Exp $";
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


#define N_VALID_REALMS 2
char *valid_realms[] = {
  "athena.mit.edu",
  "mit.edu"
  };

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
			   NULL);
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
  char realm[BUFSIZ];
  char buf[BUFSIZ];
  char *p,*end;
  int status;
  int valid_realm;
  FILE *f, *mail;
  int c,i;
  extern char *optarg;
  extern int optind;
  REQUEST Request;
  int instance;
  int error,do_send;

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

  if ((f = fopen(filename,"w+")) == NULL) {
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

/* look for correct realm;
   if not @athena.mit.edu or @mit.edu, give the username "nobody"
   eventually, real mail address will go in nickname.
 */

      if (*end == '@') {
	strncpy(realm,end+1,BUFSIZ);
      }

      if (end != p) {
	strcpy(orig_address,p);
	*end = '\0';
	end = index(orig_address,'>');
	if (end != NULL) *end = '\0';
	end = index(orig_address,'\n');
	if (end != NULL) *end = '\0';
	strncpy(username,p,8);
	username[8] = '\0';
      }
    }
  }

  fclose(f);

  valid_realm = 0;
  for(i=0;i<N_VALID_REALMS;i++) {
    if (strncasecmp(realm,valid_realms[i],strlen(valid_realms[i])) == 0) {
      valid_realm = 1;
      break;
    }
  }

  if ((username[0] == '\0') || (!valid_realm)) {
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

  error = 0;
  do_send = 0;
  mail = popen("/usr/lib/sendmail -t", "w");
  if (mail) {
    fprintf(mail, "To: %s\n",orig_address);
    fprintf(mail, "From: \"OLC Server\" <olc-test@matisse.local>\n");
    fprintf(mail, "Subject: Your OLC question\n");

  } else {
    syslog(LOG_ERR,"popen: /usr/lib/sendmail failed: %m");
    mail = stderr;
  }

  instance = Request.requester.instance;
  set_option(Request.options,VERIFY);
  status = OAsk_buffer(&Request,topic,NULL);
  unset_option(Request.options, VERIFY);

  switch(status) {
  case SUCCESS:
    break;
    
  case INVALID_TOPIC:
    syslog(LOG_ERR,"topic %s is invalid", topic);
    fprintf(mail,"An error in the setup has prevented your question from being asked;\n");
    fprintf(mail,"The topic `%s' which the server tried to use is invalid.\n\n",
	      topic);
    error = 1;
    break;
    
  case ERROR:
    syslog(LOG_ERR,"error contacting server %s", server);
    fprintf(mail,
	    "An error has occurred while contacting the server.  Please try sending\n");
    fprintf(mail,
	    "your question again; if it continues to fail, please contact the consultants\n");
    fprintf(mail, "by other means.\n\n");
    error = 1;
    break;
    
  case PERMISSION_DENIED:
    fprintf(mail,"You are not allowed to ask questions on this server.\n");
    fprintf(mail,"Does defeat the purpose of things, doesn't it?\n");
    syslog(LOG_ERR,"user %s: permission denied", username);
    error = 1;
    break;
    
  case CONNECTED:
  case MAX_ASK:
  case ALREADY_HAVE_QUESTION:
  case HAS_QUESTION:
    syslog(LOG_INFO,"user %s sending reply", username);
    fprintf(mail, "You are already asking a question; this message will be \n");
    fprintf(mail, "appended to the text of your existing question.\n");
    do_send = 1;
    break;
    
  default:
    syslog(LOG_ERR,"OAsk (VERIFY) Error status %d\n", status);
    fprintf(mail,"Received unexpected error %d from the server.\n", status);
    fprintf(mail,"Please try sending your question again; if it\n");
    fprintf(mail,"continues to fail, please contact the consultants\n");
    fprintf(mail,"by other means.\n\n");
    error = 1;
    break;
  }
  
  if (!error) {
    if (do_send) {
      status = OReply(&Request,filename);
    } else {
      status = OAsk_file(&Request,topic,filename);
    }

    (void) unlink(filename);
    switch(status) {
    case SUCCESS:
    case NOT_CONNECTED:
      fprintf(mail, "Your question has been received and will be forwarded to the first\navailable consultant.\n");
      break;
    case CONNECTED:
      fprintf(mail, "Your question has been received and a consultant is reviewing it now.\n");
      break;
    default:
      syslog(LOG_ERR,"OAsk Error status %d\n", status);
      fprintf(mail,"Received unexpected error %d from the server.\n", status);
      fprintf(mail,"Please try sending your question again; if it\n");
      fprintf(mail,"continues to fail, please contact the consultants\n");
      fprintf(mail,"by other means.\n\n");
      error = 1;
      break;
    }
  }

#ifdef KERBEROS
  dest_tkt();
#endif

  if (mail) {
    fprintf(mail, "Do not reply directly to this message; it was automatically generated.\n");
    fprintf(mail, ".\n");
    pclose(mail);
  }

}


