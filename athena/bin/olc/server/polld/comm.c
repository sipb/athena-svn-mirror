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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/comm.c,v $
 *	$Id: comm.c,v 1.1 1991-01-08 16:51:30 lwvanels Exp $
 *	$Author: lwvanels $
 */


#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/comm.c,v 1.1 1991-01-08 16:51:30 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <polld.h>
#include <olcd.h>

char INSTANCE[INST_SZ];
char REALM[REALM_SZ];

void
tell_main_daemon(user)
     PTF user;
{
  static REQUEST request;
  int k_errno;
  int status, fd;

  if (request.version != CURRENT_VERSION) {
    char kname[ANAME_SZ], kinst[INST_SZ];

    request.options = NO_OPT;
    request.version = CURRENT_VERSION;
    request.request_type = OLC_SET_USER_STATUS;
    request.code = 0;
#ifdef KERBEROS
    if (k_errno = tf_init(TICKET_FILE, R_TKT_FIL)) {
     syslog(LOG_ERR,"tf_init: %m", krb_err_txt[k_errno]);
      return;
    }
    if ((k_errno = tf_get_pname(kname)) ||
	(k_errno = tf_get_pinst(kinst))) {
      tf_close();
      syslog(LOG_ERR,"tf_get: %m", krb_err_txt[k_errno]);
      return;
    }
    tf_close();
    strcpy(request.requester.username, kname);
    strcat(request.requester.username,".");
    strcat(request.requester.username,kinst);
    strcpy(INSTANCE,kinst);
    strcpy(REALM,DFLT_SERVER_REALM);
#else
    /* Well, if you don't have kerberos, you might as well be anyone.. */
    strcpy(request.requester.username,"olc.poller");
#endif
    request.requester.instance = 0;
  }

  strcpy(request.target.username, user.username);
  strcpy(request.target.machine, user.machine);
  request.options = user.status;
  
  if ((status = open_connection_to_daemon(&request, &fd)) != 0) {
    syslog(LOG_ERR,"tell_daemon: open_connection: Error %d", status);
    return;
  }

  if ((status = send_request(fd, &request)) != 0) {
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
  
