/* $Id: al.h,v 1.3 1997-11-13 21:59:30 ghudson Exp $ */

/* Copyright 1997 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef AL__H
#define AL__H

#include <sys/types.h>

/* Exported path name. */
#define AL_PATH_SESSIONS	"/var/athena/sessions"

/* Error values */
#define AL_SUCCESS		0
#define AL_WARNINGS		1
#define AL_EBADHES		2
#define AL_ENOUSER		3
#define AL_ENOCREATE		4
#define AL_ENOLOGIN		5
#define AL_ENOREMOTE		6
#define AL_EPASSWD		7
#define AL_ESESSION		8
#define AL_EPERM		9
#define AL_ENOMEM		10

/* Warning values */
#define AL_ISWARNING(n) 	(AL_WBADSESSION <= (n) && (n) <= AL_WNOATTACH)
#define AL_WBADSESSION		11
#define AL_WGROUP		12
#define AL_WXTMPDIR		13
#define AL_WTMPDIR		14
#define AL_WNOHOMEDIR		15
#define AL_WNOATTACH		16

/* Public functions */
int al_login_allowed(const char *username, int isremote, char **filetext);
int al_acct_create(const char *username, const char *cryptpw,
		   pid_t sessionpid, int havecred, int tmphomedir,
		   int **warnings);
int al_acct_revert(const char *username, pid_t sessionpid);
int al_acct_cleanup(const char *username);
const char *al_strerror(int code, char **mem);
void al_free_errmem(char *mem);

#endif
