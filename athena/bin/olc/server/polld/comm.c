/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the fuctions to communcate with the main daemon from polld
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: comm.c,v 1.6 1999-06-28 22:52:48 ghudson Exp $
 */


#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: comm.c,v 1.6 1999-06-28 22:52:48 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <polld.h>
#include <olcd.h>

#ifdef HAVE_KRB4
char INSTANCE[INST_SZ];
char REALM[REALM_SZ];
#endif

void
tell_main_daemon(user)
     PTF user;
{
  static REQUEST request;
  int k_errno;
  ERRCODE status;
  int fd;

  if (request.version != CURRENT_VERSION) {
#ifdef HAVE_KRB4
    char kname[ANAME_SZ], kinst[INST_SZ];
#endif
    request.options = NO_OPT;
    request.version = CURRENT_VERSION;
    request.request_type = OLC_SET_USER_STATUS;
    request.code = 0;
#ifdef HAVE_KRB4
    k_errno = tf_init(TICKET_FILE, R_TKT_FIL);
    if (k_errno) {
      syslog(LOG_ERR,"tf_init: %s", krb_err_txt[k_errno]);
      return;
    }
    k_errno = tf_get_pname(kname);
    if (k_errno) {
      tf_close();
      syslog(LOG_ERR,"tf_get_pname: %s", krb_err_txt[k_errno]);
      return;
    }
    k_errno = tf_get_pinst(kinst);
    if (k_errno) {
      tf_close();
      syslog(LOG_ERR,"tf_get_pinst: %s", krb_err_txt[k_errno]);
      return;
    }

    tf_close();
    strcpy(request.requester.username, kname);
#ifdef HAVE_KRB4
    /* requester.username is narrow! */
    strcpy(request.requester.inst, kinst);
#endif
    strcpy(INSTANCE,kinst);
    strcpy(REALM,DFLT_SERVER_REALM);
#else
    /* Well, if you don't have kerberos, you might as well be anyone.. */
    strcpy(request.requester.username,"olc");
#endif
    request.requester.instance = 0;
  }

  strcpy(request.target.username, user.username);
  strcpy(request.target.machine, "matisse");  /* the hostname isn't used =) */
  request.options = user.status;
  
  status = open_connection_to_daemon(&request, &fd);
  if (status != SUCCESS)
    {
      syslog(LOG_ERR,"tell_daemon: open_connection: Error %d", status);
      return;
    }

  status = send_request(fd, &request);
  if (status != SUCCESS)
    {
      syslog(LOG_ERR,"tell_daemon: send_request: Error %d", status);
      close(fd);
      return;
    }

  read_response(fd, &status);
  if (status != SUCCESS)
    syslog(LOG_ERR,"tell_daemon: send_request: bad return val %d",
	   status);

  close(fd);
  return;
}
  
