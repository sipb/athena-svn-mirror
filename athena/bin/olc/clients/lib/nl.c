/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for communication between the user programs
 * and the non-locking daemon.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/nl.c,v $
 *	$Id: nl.c,v 1.4 1991-04-08 20:48:24 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/nl.c,v 1.4 1991-04-08 20:48:24 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#ifdef KERBEROS
#include <krb.h>
#endif

#include <olc/olc.h>
#include <nl_requests.h>

#define        MAX(a,b) (((a)>(b))?(a):(b))

#if defined(__STDC__)
# define P_(s) s
#else
# define P_(s) ()
#endif

#ifdef KERBEROS
static ERRCODE get_k_auth P_((KTEXT_ST *my_auth));
#endif /* KERBEROS */
#undef P_


ERRCODE
nl_get_qlist(fd,buf,buflen,outlen)
     int fd;
     char **buf;
     int *buflen;
     int *outlen;
{
  static char username[9];

  if (username[0] == '\0')
    strncpy(username,LIST_NAME,8);

  return(nl_get_log(fd,buf,buflen,username,LIST_INSTANCE,outlen));
}

ERRCODE
nl_get_log(fd,buf,buflen,username,instance,outlen)
     int fd;
     char **buf;
     int *buflen;
     char *username;
     int instance;
     int *outlen;
{
  long i,len,total_read;
  ERRCODE retcode;
#ifdef KERBEROS
KTEXT_ST my_auth;
#endif

  if (username[0] == '\0')
    strncpy(username,LIST_NAME,8);

/* Send request number */
  i = htonl((u_long) VERSION);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }

/* Send request number */
  i = htonl((u_long) LIST_REQ);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }

/* Send username */
  if ((retcode = swrite(fd,username,9)) == -1) {
    close(fd);
    return(errno);
  }

/* Send instance */
  i = htonl((u_long) instance);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == 1) {
    close(fd);
    return(errno);
  }
  
#ifdef KERBEROS
  retcode = get_k_auth(&my_auth);
  if (retcode != SUCCESS) {
    close(fd);
    return(retcode);
  }
  i = htonl((u_long) my_auth.length);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }
  if ((retcode = swrite(fd,(char *) my_auth.dat,my_auth.length)) == -1) {
    close(fd);
    return(errno);
  }
#else
  i = htonl((u_long) 0);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }
#endif
  
  /* Get length of text to recieve, or error code */
  if ((retcode = sread(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }
  len = ntohl(i);
  if (len < 0)
    return(len);

  /* +1 for trailing \0 */
  if (*buflen < len+1) {
/* Need to re-allocate and resize buffer */
    *buflen = MAX(len+1, 2*(*buflen));
    free(*buf);
    *buf = malloc(*buflen);
    if (*buf == NULL) {
      close(fd);
      return(ERR_MEM);
    }
  }

  total_read = 0;
  while (total_read < len) {
    i = sread(fd,(*buf+total_read),(int)len);
    if (i == -1) {
      close(fd);
      return(errno);
    }
    total_read += i;
  }
  close(fd);
  (*buf)[len] = '\0';
  *outlen = len+1;
  return(SUCCESS);
}

ERRCODE
nl_get_nm(fd, buf, buflen, username, instance, nuke, outlen)
     int fd;
     char **buf;
     int *buflen;
     char *username;
     int instance;
     int nuke;
     int *outlen;
{
  long i,len,total_read;
  ERRCODE retcode;
#ifdef KERBEROS
KTEXT_ST my_auth;
#endif

  i = htonl((u_long) VERSION);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }

/* send request # */
  if (nuke)
    i = htonl((u_long) SHOW_KILL_REQ);
  else
    i = htonl((u_long) SHOW_NO_KILL_REQ);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }

/* Send username */ 
  if ((retcode = swrite(fd,username,9)) == -1) {
    close(fd);
    return(errno);
  }

/* Send instance */
  i = htonl((u_long) instance);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == 1) {
    close(fd);
    return(errno);
  }

#ifdef KERBEROS
  retcode = get_k_auth(&my_auth);
  if (retcode != SUCCESS) {
    close(fd);
    return(retcode);
  }
  i = htonl((u_long) my_auth.length);
  if ((retcode = swrite(fd,(char *) &i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }
  if ((retcode = swrite(fd,(char *) my_auth.dat,my_auth.length)) == -1) {
    close(fd);
    return(errno);
  }
#endif
  
  /* Get length of text to recieve, or error code */
  if ((retcode = sread(fd,(char *)&i,sizeof(i))) == -1) {
    close(fd);
    return(errno);
  }
  len = ntohl(i);
  if (len < 0)
    return(len);

  if (*buflen < len) {
/* Need to re-allocate and resize buffer */
    *buflen = MAX(len, 2*(*buflen));
    free(*buf);
    *buf = malloc(*buflen);
    if (*buf == NULL) {
      close(fd);
      return(ERR_MEM);
    }
  }

  total_read = 0;
  while (total_read < len) {
    i = sread(fd,(*buf+total_read),(int)len);
    if (i == -1) {
      close(fd);
      return(errno);
    }
    total_read += i;
  }
  close(fd);
  *outlen = len;
  return(SUCCESS);
}

#ifdef KERBEROS
static ERRCODE
get_k_auth(my_auth)
     KTEXT_ST *my_auth;
{
  static char server_instance[INST_SZ];
  static char server_realm[REALM_SZ];
  int auth_result;

  expand_hostname(DaemonHost, server_instance, server_realm);
  auth_result = krb_mk_req(my_auth,K_SERVICE,server_instance,server_realm,0);
  if (auth_result != MK_AP_OK)
    return(auth_result);
  else
    return(SUCCESS);
}
#endif
