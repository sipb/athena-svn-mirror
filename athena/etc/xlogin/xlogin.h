/* $Id: xlogin.h,v 1.2 2000-04-11 13:38:42 rbasch Exp $ */

/* Copyright 1999 by the Massachusetts Institute of Technology.
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

/* from StringToPixel.c */
void add_converter(void);

/* from verify.c */
pid_t fork_and_store(pid_t *var);
int psetenv(const char *name, const char *value, int overwrite);
int punsetenv(const char *name);
char *dologin(char *user, char *passwd, int option, char *script,
	      char *tty, char *session, char *display);
void cleanup(char *user);

/* from xlogin.c */
void prompt_user(char *msg, void (*abort_proc)(void *), void *abort_arg);
char *lose(char *msg);
int set_uid_and_caps(struct passwd *pw);
