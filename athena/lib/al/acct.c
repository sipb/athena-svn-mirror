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

/* This file is part of the Athena login library.  It implements
 * functions for creating and reverting local accounts.
 */

static const char rcsid[] = "$Id: acct.c,v 1.11 1998-06-10 22:27:34 ghudson Exp $";

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include "al.h"
#include "al_private.h"

/* The al_acct_create() function has the following side-effects:
 *
 * 	* If a login record is not already present for this user:
 * 		- The user is added to the system passwd database if
 * 		  not already present there.  If /etc/nocrack is not
 * 		  present in the filesystem and the value of cryptpw
 * 		  is not NULL, it is substituted for the password
 * 		  field of the Hesiod passwd line.
 * 		- The user is added to zero or more groups in the
 * 		  system group database according to the user's hesiod
 * 		  group list.
 * 		- A login session record is created containing the
 * 		  requisite information for reversal of the above
 * 		  steps by al_acct_revert().
 *	  Otherwise
 *		- If the user was added to the system passwd database
 *		  by a prior login, the value of cryptpw is not NULL,
 *		  and /etc/nocrack is not present in the filesystem,
 *		  the value of cryptpw is substituted for the password
 *		  field of the added passwd line.
 *
 * 	* Unless a login record was already present and indicated that
 * 	  a temporary directory has been created for the user:
 * 		- The user's home directory is attached (by
 * 		  an invocation of "attach").
 * 	  If the value of havecred is true, the user's home directory
 * 	  is attached with authentication; otherwise the "-n" flag is
 * 	  passed to attach to suppress authentication.
 *
 * 	* If the user's home directory is remote and "attach"
 * 	  fails and tmphomedir is true:
 * 		- A temporary home directory is created for the user.
 * 		- The user's passwd entry is modified to point at the
 * 		  temporary home directory.
 * 		- The login session record is modified to indicate
 * 		  that a temporary directory has been created.  The
 * 		  old home directory field of the user's passwd entry
 * 		  is stored in the record for restoration of the
 * 		  passwd entry when the user is no longer logged in.
 *
 * 	* If sessionpid is not already in the login session record's
 *	  list of pids of active login sessions, it is added.
 *
 * The al_acct_create() function may return the following values:
 *
 * 	AL_SUCCESS	Successful completion
 * 	AL_ENOUSER	Unknown user
 * 	AL_EPASSWD	Can't add to passswd file
 * 	AL_ESESSION	Could not lock or modify login session record
 * 	AL_ENOMEM	Ran out of memory
 * 	AL_WARNINGS	Successful completion with some non-optimal
 * 			behavior
 *
 * If al_acct_create() returns AL_WARNINGS, then *warnings is set to a
 * malloc()'d array (which the caller must free) containing a list of
 * warning codes terminated by AL_SUCCESS.
 */


int al_acct_create(const char *username, const char *cryptpw,
		   pid_t sessionpid, int havecred, int tmphomedir,
		   int **warnings)
{
  int retval = AL_SUCCESS, nwarns = 0, warns[6], i, existed;
  struct al_record record;

  /* If the caller wants warnings, initialize them to NULL so that
   * the caller can easily tell if they were set. */
  if (warnings)
    *warnings = NULL;

  if (!al__username_valid(username))
    return AL_ENOUSER;

  /* Get and lock the session record. */
  retval = al__get_session_record(username, &record);
  if (AL_ISWARNING(retval))
    warns[nwarns++] = retval;
  else if (retval != AL_SUCCESS)
    return retval;

  /* Make sure user added to passwd file can be cleaned up later. */
  existed = record.exists;
  record.exists = 1;

  /* Add the user to the passwd file if necessary.  Do this even if
   * the record already existed, in case the user was removed from the
   * passwd file since the last login.
   */
  retval = al__add_to_passwd(username, &record, cryptpw);
  if (AL_ISWARNING(retval))
    warns[nwarns++] = retval;
  else if (retval != AL_SUCCESS)
    goto cleanup;

  if (!existed)			/* We're first interested in this user. */
    {
      /* Add the user's groups to the group file if not already there. */
      retval = al__add_to_group(username, &record);
      if (AL_ISWARNING(retval))
	warns[nwarns++] = retval;
      else if (retval != AL_SUCCESS)
	goto cleanup;

      record.pids = malloc(sizeof(pid_t));
      if (!record.pids)
	{
	  retval = AL_ENOMEM;
	  goto cleanup;
	}
      record.npids = 1;
      record.pids[0] = sessionpid;
    }
  else				/* Other processes also interested in user. */
    {
      /* Update the encrypted password entry if we added a passwd line. */
      al__update_cryptpw(username, &record, cryptpw);

      /* Add pid to record if not already there. */
      for (i = 0; i < record.npids; i++)
	{
	  if (record.pids[i] == sessionpid)
	    break;
	}
      /* al__get_session_record() leaves an extra slot in record.pids. */
      if (i == record.npids)
	record.pids[record.npids++] = sessionpid;
    }

  retval = al__setup_homedir(username, &record, havecred, tmphomedir);
  if (AL_ISWARNING(retval))
    warns[nwarns++] = retval;
  else if (retval != AL_SUCCESS)
    goto cleanup;

  /* Set warnings. */
  if (nwarns > 0)
    {
      retval = AL_WARNINGS;
      if (warnings)
	{
	  warns[nwarns++] = AL_SUCCESS;
	  *warnings = malloc(nwarns * sizeof(int));
	  if (!*warnings)
	    retval = AL_ENOMEM;
	  else
	    memcpy(*warnings, warns, nwarns * sizeof(int));
	}
    }

cleanup:
  al__put_session_record(&record);
  return retval;
}

static int revert(const char *username, struct al_record *record)
{
  int retval, reterr = AL_SUCCESS;

  retval = al__revert_homedir(username, record);
  if (AL_SUCCESS != retval)
    reterr = retval;
  retval = al__remove_from_group(username, record);
  if (AL_SUCCESS != retval)
    reterr = retval;
  retval = al__remove_from_passwd(username, record);
  if (AL_SUCCESS != retval)
    reterr = retval;

  record->exists = 0;
  return reterr;
}

/* The al_acct_revert() function has the following side effects:
 *
 * 	* If a login record for the user exists and sessionpid is in
 * 	  the record's list of pids of active login sessions:
 * 	  - sessionpid is removed from the user's login record's list
 * 	    of pids of active login sessions.
 *
 * 	* If sessionpid was successfully removed from the list, and
 * 	  the list of pids is now empty:
 * 	  - All modifications to the passwd and group database
 * 	    effected by calls to al_acct_create() are reverted.
 * 	  - The user's home directory is detached.
 */

int al_acct_revert(const char *username, pid_t sessionpid)
{
  int retval, i, j;
  struct al_record record;

  if (!al__username_valid(username))
    return AL_EPERM;

  /* Don't create a session record if there isn't one. */
  if (!al__record_exists(username))
    return AL_SUCCESS;

  retval = al__get_session_record(username, &record);
  if (AL_SUCCESS != retval)
    return retval;

  if (record.exists)
    {
      /* Copy record.pids to itself, eliminating occurances of sessionpid. */
      j = 0;
      for (i = 0; i < record.npids; i++)
	{
	  if (record.pids[i] != sessionpid)
	    record.pids[j++] = record.pids[i];
	}
      record.npids = j;

      /* Revert the account if we emptied out the pid list. */
      if (record.npids == 0)
	revert(username, &record);
    }

  return al__put_session_record(&record);
}

/* The al_acct_cleanup() function has the following side effects:
 *
 * 	* If a login record for the user exists:
 * 	  - All pids in the login record are checked for existence and
 * 	    the ones which don't exist are removed from the list.
 *
 * 	* If the list of pids was emptied by the above operation:
 * 	  - All modifications to the passwd and group database
 * 	    effected by calls to al_acct_create() are reverted.
 * 	  - The user's home directory is detached.
 */

int al_acct_cleanup(const char *username)
{
  int retval, i, j;
  struct al_record record;

  if (!al__username_valid(username))
    return AL_EPERM;

  /* Don't create a session record if there isn't one. */
  if (!al__record_exists(username))
    return AL_SUCCESS;

  retval = al__get_session_record(username, &record);
  if (AL_SUCCESS != retval)
    return retval;

  if (record.exists)
    {
      /* Copy record.pids to itself, eliminating pids which don't exist. */
      j = 0;
      for (i = 0; i < record.npids; i++)
	{
	  if (kill(record.pids[i], 0) == 0)
	    record.pids[j++] = record.pids[i];
	}
      record.npids = j;

      /* Revert the account if we emptied out the pid list. */
      if (record.npids == 0)
	revert(username, &record);
    }

  return al__put_session_record(&record);
}
