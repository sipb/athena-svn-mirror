/*
 * $Source: /afs/dev.mit.edu/source/repository/third/cns/src/kadmin/kadm_server.h,v $
 * $Author: ghudson $
 * $Header: /afs/dev.mit.edu/source/repository/third/cns/src/kadmin/kadm_server.h,v 1.1.1.1 1996-09-06 00:47:17 ghudson Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Definitions for Kerberos administration server & client
 */

#ifndef KADM_SERVER_DEFS
#define KADM_SERVER_DEFS

#include <mit-copyright.h>
/*
 * kadm_server.h
 * Header file for the fourth attempt at an admin server
 * Doug Church, December 28, 1989, MIT Project Athena
 *    ps. Yes that means this code belongs to athena etc...
 *        as part of our ongoing attempt to copyright all greek names
 */

#include <sys/types.h>
#include <krb.h>
#include <des.h>

typedef struct {
  struct sockaddr_in admin_addr;
  struct sockaddr_in recv_addr;
  int recv_addr_len;
  int admin_fd;			/* our link to clients */
  char sname[ANAME_SZ];
  char sinst[INST_SZ];
  char krbrlm[REALM_SZ];
  C_Block master_key;
  C_Block session_key;
  Key_schedule master_key_schedule;
  long master_key_version;
} Kadm_Server;

/* the default syslog file */
#define KADM_SYSLOG  "/kerberos/admin_server.syslog"

/* where to find the bad password table */
#define PW_CHECK_FILE "/kerberos/bad_passwd"

#define DEFAULT_ACL_DIR	"/kerberos"
#define	ADD_ACL_FILE	"/admin_acl.add"
#define	GET_ACL_FILE	"/admin_acl.get"
#define	MOD_ACL_FILE	"/admin_acl.mod"

#endif /* KADM_SERVER_DEFS */
