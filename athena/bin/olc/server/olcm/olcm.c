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
 *      $Id: olcm.c,v 1.16 2000-11-16 16:31:51 zacheiss Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: olcm.c,v 1.16 2000-11-16 16:31:51 zacheiss Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olcm.h>
#include <olc/olc.h>

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netdb.h>
#include <sys/file.h>
#include <ctype.h>

#ifdef HAVE_KRB4
#include <krb.h>
#endif

#ifdef   HAVE_SYSLOG_H
#include   <syslog.h>
#ifndef    LOG_CONS
#define      LOG_CONS 0  /* if LOG_CONS isn't defined, just ignore it */
#endif     /* LOG_CONS */
#endif /* HAVE_SYSLOG_H */


/* timeout to link with client library */
int select_timeout=300;


#define N_VALID_REALMS 2
char *valid_realms[] = {
  "athena.mit.edu",
  "mit.edu"
  };

#ifdef HAVE_KRB4
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
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"get_krb_tkt: error parsing %s",ident);
#endif /* HAVE_SYSLOG */
    fprintf(stderr,"System error: Could not parse %s\n",ident);
    exit(1);
  }

  if (realm[0] == '\0')
    strncpy(realm, DFLT_SERVER_REALM, REALM_SZ);
  strcpy(tkt_filename,"/tmp/olcm_tkt_XXXXXX");
  mktemp(tkt_filename);
  set_env_var("KRBTKFILE",tkt_filename);

  dest_tkt();
  ret = krb_get_svc_in_tkt(serv, inst, realm, "krbtgt", realm, 1,
			   SRVTAB_LOC);
  if (ret != KSUCCESS) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"get_krb_tkt: %s",krb_err_txt[ret]);
#endif /* HAVE_SYSLOG */
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
#ifdef HAVE_KRB4
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
  int instance,send_default;
  int error,do_send;

#ifdef HAVE_SYSLOG
  openlog("olcm", LOG_CONS | LOG_PID, SYSLOG_FACILITY);
#endif /* HAVE_SYSLOG */

  username[0] = '\0';
  strcpy(topic,DFLT_TOPIC);
  strcpy(server,DFLT_SERVER);
  send_default = 1;

  while ((c = getopt(argc, argv, "s:t:k:n")) != EOF)
    switch (c) {
    case 'n':
      send_default = 0;
    case 's':
      strncpy(server,optarg,MAXHOSTNAMELEN);
      server[MAXHOSTNAMELEN-1] = '\0';
      break;
    case 't':
      strncpy(topic,optarg,BUFSIZ);
      topic[BUFSIZ-1] = '\0';
      break;
#ifdef HAVE_KRB4
    case 'k':
      strncpy(ident,optarg,SNAME_SZ+INST_SZ+REALM_SZ+4);
      break;
#endif
    default:
#ifdef HAVE_KRB4
      fprintf(stderr,"Usage: %s [-t topic] [-s server] [-k service.instance@realm]\n", argv[0]);
      fprintf(stderr,"\t[-n]\n");
#else
      fprintf(stderr,"Usage: %s [-t topic] [-s server]\n",argv[0]);
      fprintf(stderr,"\t[-n]\n");
#endif
      exit(1);
    }

  strcpy(filename,"/tmp/olcm_XXXXXX");
  mktemp(filename);

  f = fopen(filename,"w+");
  if (f == NULL) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"olcm: fopening file %s: %m",filename);
#endif /* HAVE_SYSLOG */
    exit(1);
  }

  /* Incarnate polld with minimum data, so we won't need a config file */
  if (incarnate_hardcoded("olcm", OLXX_SERVICE, DaemonHost) != SUCCESS) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR, "Program incarnation failed???");
#endif /* HAVE_SYSLOG */
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

      end = strchr(p,'<');
      if (end == NULL)
	end = p;
      else {
	end++;
	p = end;
      }

      while(isalnum(*end) || *end == '_')
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
	end = strchr(orig_address,'>');
	if (end != NULL) *end = '\0';
	end = strchr(orig_address,'\n');
	if (end != NULL) *end = '\0';
	strncpy(username,p,8);
	username[8] = '\0';
      }
    }
  }

  fclose(f);

  valid_realm = 0;
  for(i=0;i<N_VALID_REALMS;i++) {
    /* TODO: should be strcasecmp (think "mit.edu.hr" =) --bert 30jan1997 */
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

#ifdef HAVE_KRB4
  /* Set ticket file correctly */
  get_krb_tkt(ident);
#endif

  set_env_var("OLCD_HOST", server);

  OInitialize();

  if (fill_request(&Request) != SUCCESS) {
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"fill_request unsuccessful");
#endif /* HAVE_SYSLOG */
    exit(1);
  }

  (void) strcpy(Request.target.username, username);

  error = 0;
  do_send = 0;
  if (send_default) {
    mail = popen("/usr/lib/sendmail -t", "w");
    if (mail) {
      char stock_filename[1024];

      sprintf(stock_filename,"%s_%s",STOCK_HEADER,topic);
      fprintf(mail, "To: %s\n",orig_address);
      f = fopen(stock_filename,"r");
      if (f != NULL) {
	while(fgets(buf,BUFSIZ,f) != NULL) {
	  /* Output stock header */
	  fputs(buf,mail);
	}
	fclose(f);
      }
    } else {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR,"popen: /usr/lib/sendmail failed: %m");
#endif /* HAVE_SYSLOG */
      mail = stderr;
    }
  }

  instance = Request.requester.instance;
  set_option(Request.options,VERIFY);
  status = OAsk_buffer(&Request,topic,NULL);
  unset_option(Request.options, VERIFY);

  switch(status) {
  case SUCCESS:
    break;
    
  case INVALID_TOPIC:
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"topic %s is invalid", topic);
#endif /* HAVE_SYSLOG */
    if (send_default) {
      fprintf(mail,"An error in the setup has prevented your question from being asked;\n");
      fprintf(mail,"The topic `%s' which the server tried to use is invalid.\n\n",
	      topic);
    }
    error = 1;
    break;
    
  case ERROR:
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"error contacting server %s", server);
#endif /* HAVE_SYSLOG */
    if (send_default) {
      fprintf(mail,
	      "An error has occurred while contacting the server.  Please try sending\n");
      fprintf(mail,
	      "your question again; if it continues to fail, please contact the consultants\n");
      fprintf(mail, "by other means.\n\n");
    }
    error = 1;
    break;
    
  case PERMISSION_DENIED:
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"user %s: permission denied", username);
#endif /* HAVE_SYSLOG */
    if (send_default) {
      fprintf(mail,"You are not allowed to ask questions on this server.\n");
      fprintf(mail,"Does defeat the purpose of things, doesn't it?\n");
    }
    error = 1;
    break;
    
  case CONNECTED:
  case MAX_ASK:
  case ALREADY_HAVE_QUESTION:
  case HAS_QUESTION:
#ifdef HAVE_SYSLOG
    syslog(LOG_INFO,"user %s sending reply", username);
#endif /* HAVE_SYSLOG */
    if (send_default) {
      fprintf(mail, "You are already asking a question; your message will be \n");
      fprintf(mail, "appended to the text of your existing question.\n");
    }
    do_send = 1;
    break;
    
  default:
#ifdef HAVE_SYSLOG
    syslog(LOG_ERR,"OAsk (VERIFY) Error status %d\n", status);
#endif /* HAVE_SYSLOG */
    if (send_default) {
      fprintf(mail,"Received unexpected error %d from the server.\n", status);
      fprintf(mail,"Please try sending your question again; if it\n");
      fprintf(mail,"continues to fail, please contact the consultants\n");
      fprintf(mail,"by other means.\n\n");
    }
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
    if (send_default) {
      switch(status) {
      case SUCCESS:
      case NOT_CONNECTED:
/*      fprintf(mail, "Your question has been received and will be forwarded */
/*      to the first\navailable consultant.\n");  */
/*      break; */
      case CONNECTED:
/*      fprintf(mail, "Your question has been received and a consultant is */
/*      reviewing it now.\n");  */
	break;
      default:
#ifdef HAVE_SYSLOG
	syslog(LOG_ERR,"OAsk Error status %d\n", status);
#endif /* HAVE_SYSLOG */
	fprintf(mail,"Received unexpected error %d from the server.\n", status);
	fprintf(mail,"Please try sending your question again; if it\n");
	fprintf(mail,"continues to fail, please contact the consultants\n");
	fprintf(mail,"by other means.\n\n");
	error = 1;
	break;
      }
    }
  }

#ifdef HAVE_KRB4
  dest_tkt();
#endif

  if (send_default) {
    char stock_filename[1024];

    sprintf(stock_filename,"%s_%s",STOCK_FILE,topic);
    f = fopen(stock_filename,"r");
    if (f != NULL) {
      while(fgets(buf,BUFSIZ,f) != NULL) {
	/* Output stock reply */
	fputs(buf,mail);
      }
      fclose(f);
    }

    if (mail) {
      fprintf(mail, "Do not reply directly to this message; it was automatically generated.\n");
      fprintf(mail, ".\n");
      pclose(mail);
    }
  }
  exit(0);
}


