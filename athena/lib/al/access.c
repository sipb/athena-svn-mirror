/* Copyright 1998 by the Massachusetts Institute of Technology.
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
 * function to get the access string and text strings from the access
 * file for a user.
 */

static const char rcsid[] = "$Id: access.c,v 1.2 1998-05-13 01:23:15 danw Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include "al.h"
#include "al_private.h"

static int first_field_match(const char *line, const char *s);

/* The al_get_access() function reads the access bits and explanatory
 * text from the access file.  The calling program may specify NULL
 * for either the access or text parameter if it is not interested
 * in that piece of information.  On successful return, *access is
 * set to the access bits for the user and *text is set to the
 * explanatory text given for that user, or NULL if there is no text.
 *
 * The al_get_access() function may return the following values:
 *
 *	AL_SUCCESS	Access information successfully read
 *	AL_ENOENT	Access file does not exist
 *	AL_EPERM	Permissions error reading access file
 *	AL_ENOUSER	User not found in access file
 *	AL_ENOMEM	Ran out of memory
 */

int al_get_access(const char *username, char **access, char **text)
{
  FILE *fp;
  int linesize, haslocal, retval;
  char *line = NULL, *match = NULL;
  const char *p, *q;
  struct passwd *pwd;

  pwd = al__getpwnam(username);
  if (pwd)
    {
      haslocal = 1;
      al__free_passwd(pwd);
    }
  else
    haslocal = 0;

  fp = fopen(PATH_ACCESS, "r");
  if (!fp)
    return (errno == ENOENT) ? AL_ENOENT : AL_EPERM;

  /* Lines in the access file are of the form:
   *
   *	username	access-bits	text
   *
   * Where "*" matches any username, "*inpasswd" matches any username with
   * local password information, the access bits 'l' and 'r' set local and
   * remote access, and text (if specified) gives a message to return if
   * the user is denied access.
   */
  retval = AL_ENOUSER;
  while (al__read_line(fp, &line, &linesize) == 0)
    {
      if (*line == '#')
        continue;
      if (first_field_match(line, username))
	{
	  match = username;
	  break;
	}
      else if (first_field_match(line, "*inpasswd") && haslocal)
        match = "*inpasswd";
      else if (first_field_match(line, "*") && !match)
        match = "*";
    }  

  if (match)
    {
      rewind(fp);
      while (al__read_line(fp, &line, &linesize) == 0)
	{
	  p = line;

	  if (!first_field_match(p, match))
	    continue;

	  /* Skip to the access bits. */
	  while (*p && !isspace(*p))
	    p++;
	  while (isspace(*p))
	    p++;

	  q = p;
	  while (*q && !isspace(*q))
	    q++;
	  if (access)
	    {
	      *access = malloc(q - p + 1);
	      if (!*access)
		{
		  retval = AL_ENOMEM;
		  break;
		}
	      memcpy(*access, p, q - p);
	      (*access)[q - p] = 0;
	    }

	  if (text)
	    {
	      p = q;
	      while (isspace(*p))
		p++;
	      if (*p)
		{
		  *text = malloc(strlen(p) + 1);
		  if (!*text)
		    {
		      if (access)
			free(*access);
		      retval = AL_ENOMEM;
		      break;
		    }
		  strcpy(*text, p);
		}
	    }

	  retval = AL_SUCCESS;
	  break;
	}
    }

  free(line);
  fclose(fp);
  return retval;
}

/* The al_is_local_acct() function determines whether a username is
 * listed as having a local account in the access file.  Returns
 * 1 if the user is listed as having a local accounts, 0 if not, and
 * -1 if there was an out of memory error or a permissions error
 * reading the access file.
 */
int al_is_local_acct(const char *username)
{
  int status;
  char *bits;
  const char *p;

  status = al_get_access(username, &bits, NULL);
  if (status == AL_ENOENT || status == AL_ENOUSER)
    return 0;
  if (status != AL_SUCCESS)
    return -1;
  for (p = bits; *p; p++)
    {
      if (*p == 'L')
	break;
    }
  status = (*p == 'L');
  free(bits);
  return status;
}

/* Return true if the first field of line (terminated by whitespace or the
 * end of the string) matches s.
 */
static int first_field_match(const char *line, const char *s)
{
  int len = strlen(s);

  return (strncmp(line, s, len) == 0 && (isspace(line[len]) || !line[len]));
}
