/* Copyright 1997, 1998 by the Massachusetts Institute of Technology.
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

/* This file is part of the Athena login library.  It implements the
 * function to check if a user is allowed to log in.
 */

static const char rcsid[] = "$Id: allowed.c,v 1.7 1998-05-05 21:56:02 ghudson Exp $";

#include <errno.h>
#include <hesiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "al.h"
#include "al_private.h"

static int try_access(const char *username, int isremote, int haslocalpwd,
		      int *local_acct, char **text, int *retval);
static int good_hesiod(const char *username, int *retval);
static int first_field_match(const char *line, const char *s);

/* The al_login_allowed() function determines whether a user is allowed
 * to log in.  The calling process provides an indication of whether the
 * login session is from a remote host.  The al_login_allowed() function
 * has no side-effects.
 * 
 * The al_login_allowed() function may return the following values:
 * 
 * 	AL_SUCCESS	The user may log in
 * 	AL_ENOUSER	Unknown user
 * 	AL_EBADHES	Illegal hesiod entry for user
 * 	AL_ENOLOGIN	Login denied because logins are disabled
 * 	AL_ENOREMOTE	Login denied because this user is not allowed
 *			to log in remotely
 * 	AL_ENOCREATE	Login denied because this user is not allowed
 *			to log in
 * 	AL_ENOMEM	Ran out of memory
 * 
 * If al_login_allowed() returns AL_ENOLOGIN, AL_ENOREMOTE, or
 * AL_ENOCREATE and text is not NULL, then *text is set to a malloc()'d
 * string (which the caller must free) containing the text of the file
 * which caused the login to be denied.  Otherwise, *text is set to NULL.
 */

int al_login_allowed(const char *username, int isremote, int *local_acct,
		     char **text)
{
  struct passwd *local_pwd;
  int retval = AL_SUCCESS;
  char *retfname = NULL;
  FILE *retfile;

  /* Make sure *text gets set to NULL if we don't give it a value
   * later.  Also, assume account is non-local for now.
   */
  if (text)
    *text = NULL;
  *local_acct = 0;

  if (!al__username_valid(username))
    return AL_ENOUSER;

  /* root is always authorized to log in and is always a local account. */
  local_pwd = al__getpwnam(username);
  if (local_pwd && local_pwd->pw_uid == 0)
    {
      *local_acct = 1;
      goto cleanup;
    }

  /* For all non-root users, honor the /etc/nologin file. */
  if (!access(PATH_NOLOGIN, F_OK))
    {
      retval = AL_ENOLOGIN;
      retfname = PATH_NOLOGIN;
      goto cleanup;
    }

  /* Those without local passwd information must have Hesiod passwd
   * information or they don't exist.
   */
  if (!local_pwd && !good_hesiod(username, &retval))
    goto cleanup;

  /* Try the access control file. */
  if (try_access(username, isremote, (local_pwd != NULL), local_acct, text,
		 &retval))
    goto cleanup;

  /* There is no access control file.  Look at the nocreate and noremote
   * files if the user has no local passwd information.
   */
  if (!local_pwd)
    {
      if (!access(PATH_NOCREATE, F_OK))
	{
	  retval = AL_ENOCREATE;
	  retfname = PATH_NOCREATE;
	  goto cleanup;
	}
      if (isremote && !access(PATH_NOREMOTE, F_OK))
	{
	  retval = AL_ENOREMOTE;
	  retfname = PATH_NOREMOTE;
	  goto cleanup;
	}
    }

cleanup:
  if (local_pwd)
    al__free_passwd(local_pwd);
  if (retfname && text)
    {
      retfile = fopen(retfname, "r");
      if (retfile)
	{
	  struct stat st;

	  if (!fstat(fileno(retfile), &st) && st.st_size > 0)
	    {
	      *text = malloc(1 + st.st_size);
	      if (*text)
		{
		  /* Zero all in case fewer chars read than expected. */
		  memset(*text, 0, 1 + st.st_size);
		  fread(*text, sizeof(char), st.st_size, retfile);
		}
	    }
	  fclose(retfile);
	}
    }
  return retval;
}

static int try_access(const char *username, int isremote, int haslocalpwd,
		      int *local_acct, char **text, int *retval)
{
  FILE *fp;
  int linesize;
  char *line = NULL;
  const char *p;

  fp = fopen(PATH_ACCESS, "r");
  if (!fp)
    return 0;

  /* Lines in the access file are of the form:
   *
   *	username	access-bits	text
   *
   * Where "*" matches any username, "*inpasswd" matches any username with
   * local password information, the access bits 'l' and 'r' set local and
   * remote access, and text (if specified) gives a message to return if
   * the user is denied access.
   */
  *retval = AL_ENOCREATE;
  while (al__read_line(fp, &line, &linesize) == 0)
    {
      p = line;

      /* Ignore comment lines and lines which don't match the username. */
      if (*p == '#' || (!first_field_match(p, username)
			&& !(first_field_match(p, "*inpasswd") && haslocalpwd)
			&& !first_field_match(p, "*")))
	continue;

      while (*p && !isspace(*p))
	p++;
      while (isspace(*p))
	p++;

      /* Read the access bits to determine if this user is allowed to
       * log in.
       */
      for (; *p && !isspace(*p); p++)
	{
	  if ((*p == 'l' && !isremote) || (*p == 'r' && isremote))
	    *retval = AL_SUCCESS;
	  if (*p == 'l' && isremote && *retval == AL_ENOCREATE)
	    *retval = AL_ENOREMOTE;
	  if (*p == 'L')
	    *local_acct = 1;
	}

      /* If the user is not allowed to log in and there is text specified,
       * set text.  Add a newline so that it's consistent with text one
       * might read from a file.
       */
      if (*retval != AL_SUCCESS && text)
	{
	  while (isspace(*p))
	    p++;
	  if (*p)
	    {
	      *text = malloc(strlen(p) + 2);
	      if (*text)
		{
		  strcpy(*text, p);
		  strcat(*text, "\n");
		}
	    }
	}
      break;
    }
  free(line);
  fclose(fp);
  return 1;
}

/* Check whether a user has Hesiod information which doesn't conflict
 * with a local uid.
 */
static int good_hesiod(const char *username, int *retval)
{
  void *hescontext = NULL;
  struct passwd *local_pwd, *hes_pwd;
  int ok = 0;

  if (hesiod_init(&hescontext) == 0)
    {
      hes_pwd = hesiod_getpwnam(hescontext, username);
      if (hes_pwd)
	{
	  local_pwd = al__getpwuid(hes_pwd->pw_uid);
	  if (local_pwd)
	    {
	      al__free_passwd(local_pwd);
	      *retval = AL_EBADHES;
	    }
	  else
	    ok = 1;
	  hesiod_free_passwd(hescontext, hes_pwd);
	}
      else
	*retval = AL_ENOUSER;
      hesiod_end(hescontext);
    }
  else
    *retval = (errno == ENOMEM) ? AL_ENOMEM : AL_ENOUSER;
  return ok;
}

/* Return true if the first field of line (terminated by whitespace or the
 * end of the string) matches s.
 */
static int first_field_match(const char *line, const char *s)
{
  int len = strlen(s);

  return (strncmp(line, s, len) == 0 && (isspace(line[len]) || !line[len]));
}
