/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for backing up the state of the OLC system.
 *
 *      Win Treese (MIT Project Athena)
 *      Ken Raeburn (MIT Information Systems)
 *      Tom Coppeto, Chris VanHaren, Lucien Van Elsen (MIT Project Athena)
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: backup.c,v 1.28 1999-03-06 16:48:52 ghudson Exp $
 */

#ifndef SABER
#ifndef lint
static char rcsid[] ="$Id: backup.c,v 1.28 1999-03-06 16:48:52 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "olcd.h"

#include <unistd.h>
#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */
#include <sys/file.h>           /* System file defs. */
#include <setjmp.h>             /* For string validation kludge */
#include <pwd.h>

/*** top-level backup functions ***/

/* Back up the queue in both the binary and the ASCII format.
 * Note: the binary format is byte-order and word-size dependent.
 */

void backup_data(void)
{
  dump_data(ASCII_BACKUP_TEMP);
  if (rename(ASCII_BACKUP_TEMP, ASCII_BACKUP_FILE) < 0)
    log_error("can't rename %s to %s: %m",
	      ASCII_BACKUP_TEMP, ASCII_BACKUP_FILE);

  binary_backup_data();

  needs_backup = 0;  
  log_status("Backup completed.");
}

/* Restore the ASCII backup of the queue.  If it is missing, restore
 * the binary backup instead.
 * Note: the binary format is byte-order and word-size dependent, so
 *       any actual use of the binary backup is discouraged.  Unless
 *       you have a good excuse, at least.
 */

void load_data(void)
{
  if (access(ASCII_BACKUP_FILE, R_OK) == 0)
      {
        if (undump_data(ASCII_BACKUP_FILE) == SUCCESS)
	  return;  /* dump succeeded, we're done. */
      }
  else
    log_status("Can't access the ASCII backup.");

  /* ASCII dump doesn't exist or loading it failed. */
  log_error("reading ASCII backup didn't work, trying binary.");
  binary_load_data();
}


/*** things that cause olcd to not go boom in the night ***/

void ensure_consistent_state(void)
{
  register KNUCKLE **k_ptr, *k;
  KNUCKLE *foo;
  int status, broken;
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      k = *k_ptr;

      if (k->connected == NULL)
	k->cusername[0] = '\0';

      else
	{
	  /* check if connected person exists */
	  broken = 0;
	  status = get_knuckle (k->connected->user->username,
				k->connected->instance, &foo, /* ??? */0);
	  /* specify *exactly* what went wrong */
	  if (status != SUCCESS)
	    {
	      log_error("Inconsistency: can't get user connected to %s[%d]\n",
			k->user->username, k->instance);
	      broken = 1;
	    }
	  else if (k->connected->connected == NULL)
	    {
	      /* check if k->connected->user is printable */
	      if (k->connected->user && k->connected->user->username)
		{
		  log_error("Inconsistency: %s[%d] is connected to %s[%d],"
			    " but this user is connected to NULL\n",
			    k->user->username,
			    k->instance,
			    k->connected->user->username,
			    k->connected->instance);
		}
	      else
		{
		  log_error("Inconsistency: %s[%d] is connected to some user,"
			    " but this user is connected to NULL\n",
			    k->user->username, k->instance);
		}
	      broken = 1;
	    }
	  else if (k != k->connected->connected)
	    {
	      /* check if k->connected->connected->user is printable */
	      if (k->connected->connected->user
		  && k->connected->connected->user->username)
		{
		  log_error("Inconsistency: %s[%d] is connected to %s[%d]"
			    " is connected to %s[%d] ???\n",
			    k->user->username,
			    k->instance,
			    k->connected->user->username,
			    k->connected->instance,
			    k->connected->connected->user->username,
			    k->connected->instance);
		}
	      else
		{
		  log_error("Inconsistency: %s[%d] is connected to %s[%d]"
			    " is connected somewhere else ???\n",
			    k->user->username,
			    k->instance,
			    k->connected->user->username,
			    k->connected->instance);
		}
	      broken = 1;
	    }

	  if (broken) {
	    /* problem: the question data is probably screwed due to
	     * earlier bugs.  don't try to salvage because addresses to
	     * question->owner is in space. Take the loss on the question
	     * data, delete both knuckles, and continue. This should be
	     * looked into.  */
	    k->question = (QUESTION *) NULL;
	    delete_knuckle(k, /*???*/0);
	    /* delete_knuckle(k->connected); */

	    k->connected->cusername[0] = '\0';
	    k->connected->connected = NULL;
	    k->connected->question = NULL;
	    k->cusername[0] = '\0';
	    k->connected = NULL;
	    k->question = NULL;

	    continue;
#if 0
	    if (k->question != (QUESTION *) NULL)
	      {
		if(k->status & QUESTION_STATUS)
		  k->question->owner = k;
		else
		  k->question->owner = (KNUCKLE *) NULL;
		
		if(k->question->owner != (KNUCKLE *) NULL)
		  {
		    if(k->question->owner->connected != (KNUCKLE *) NULL)
		      {
			k->question->owner->connected->connected = 
			  (KNUCKLE *) NULL;  
			write_message_to_user(k->question->owner->connected,
					      "Daemon error -- please reconnect.\n",
					      NO_RESPOND);
		      }
		    k->question->owner->connected = (KNUCKLE *) NULL;
		  }
	      }
#endif
	  }
	}
    }
}

/* Reconnect the knuckles that ought to be connected.
 * Note: since the ASCII backup doesn't contain "connected" data, we
 *       have to do more convoluted things.
 */

void reconnect_knuckles(void)
{
  KNUCKLE **k_ptr;
  KNUCKLE *k;
  int status; 
  int questions;

  for (k_ptr = Knuckle_List; (*k_ptr) != (KNUCKLE *) NULL; k_ptr++)
    {
      if ((*k_ptr)->cusername[0] == '\0') {
	/* no connected username; check question status and go on. */
	(*k_ptr)->connected = (KNUCKLE *) NULL;

	continue;
      }

      /* Most knuckles at this point don't have ->connected set right.
       * Therefore, we can't use a non-zero value for "active" (4th arg).
       */
      status = get_knuckle((*k_ptr)->cusername,(*k_ptr)->cinstance, &k, 0);
      if ((status == SUCCESS)
	  && !strcmp((*k_ptr)->user->username, k->cusername)
	  && ( (*k_ptr)->instance == k->cinstance ))
	{
	  /* Additional sanity check: exactly one of the the two knuckles
	   * should have a question attached.
	   */
	  questions = ((*k_ptr)->question != NULL) + (k->question != NULL)
                      - ((*k_ptr)->question == k->question);
	  /* "questions" is now equal to -1 if both ->question fields are NULL,
	   * 1 if one is NULL or if both are equal and non-NULL,
	   * 2 if they are different and both non-NULL.
	   */
	  if (questions == 1)
	    {
	      log_status("connecting %s[%d] -> %s[%d]",
			 (*k_ptr)->user->username, (*k_ptr)->instance,
			 (*k_ptr)->cusername, (*k_ptr)->cinstance);

	      (*k_ptr)->connected = k;

	      if ((*k_ptr)->question == NULL)
		(*k_ptr)->question = (*k_ptr)->connected->question;

	      continue;  /* done; go to the next knuckle in Knuckle_List */
	    }
	  else
	    log_status("Can't connect %s[%d] -> %s[%d],"
		       " the connected knuckles would have %d questions",
		       (*k_ptr)->user->username, (*k_ptr)->instance,
		       (*k_ptr)->cusername, (*k_ptr)->cinstance,
		       (questions < 0) ? 0 : questions);
	}

      /* if control reaches here, it means we didn't connect knuckles. */

      log_status("un-connecting %s[%d] -> %s[%d]",
		 (*k_ptr)->user->username, (*k_ptr)->instance,
		 (*k_ptr)->cusername, (*k_ptr)->cinstance);
      ((*k_ptr)->cusername)[0] = '\0';
      (*k_ptr)->connected = (KNUCKLE *) NULL;
      if ( ((*k_ptr)->question != NULL)
	   && ! ((*k_ptr)->status & QUESTION_STATUS) )
	log_error("%s[%d] has a question, but no status bits are set",
		  (*k_ptr)->user->username, (*k_ptr)->instance);
    }
}

/* Reconnect the knuckles that ought to be connected.
 * Note: This version is suitable only for data loaded from binary
 *       backup!  It relies on the fact that knuckle->connected will
 *       be non-zero (although meaningless as a pointer) if the
 *       knuckle should be connected to another knuckle.
 */
void reconnect_knuckles_binary(void)
{
  KNUCKLE **k_ptr;
  KNUCKLE *k;
  int status; 
  
  for (k_ptr = Knuckle_List; (*k_ptr) != (KNUCKLE *) NULL; k_ptr++)
    if((*k_ptr)->connected != (KNUCKLE *) NULL)
      {
	status = get_knuckle((*k_ptr)->cusername,(*k_ptr)->cinstance, &k,
			     /*???*/0);
	if(status == SUCCESS)
	  {
	    log_status("connecting %s[%d] -> %s[%d]",
		       (*k_ptr)->user->username, (*k_ptr)->instance,
		       (*k_ptr)->cusername, (*k_ptr)->cinstance);
	    (*k_ptr)->connected = k;
	    if((*k_ptr)->question != (QUESTION *) NULL)
	      {
		(*k_ptr)->connected->question = (*k_ptr)->question;
		
		/* not good enough- should have some physical tag */
		if((*k_ptr)->status  & QUESTION_STATUS)
		  (*k_ptr)->question->owner = (*k_ptr);		    
	      }
	  }
	else
	  {
	    log_status("un-connecting %s[%d] -> %s[%d]",
		       (*k_ptr)->user->username, (*k_ptr)->instance,
		       (*k_ptr)->cusername, (*k_ptr)->cinstance);
	    *((*k_ptr)->cusername) = '\0';
	    (*k_ptr)->connected = (KNUCKLE *) NULL;
	    if((*k_ptr)->question != (QUESTION *) NULL)
	      if((*k_ptr)->status  & QUESTION_STATUS)
		(*k_ptr)->question->owner = (*k_ptr);		    
	  }
      }
}

/*** I/O helpers ***/

#define LINE_CHUNK 1024

/* Read a line of arbitrary length from an I/O stream.
 * Arguments:   buf:  Address of a char* variable used as a buffer.  The
 *                    variable should be initialized to NULL or a malloc'd
 *                    buffer before the first call to this function, and
 *                    can simply be re-used later, but the buffer will be
 *                    overwritten and reallocated at will.  The value
 *                    should be free()'d after the last call to fget_line.
 *              size: Address of a size_t variable containing the size of
 *                    *buf.  The variable should be initialized to 0 if
 *                    *buf is NULL, or whatever the allocation size of *buf
 *                    is, otherwise.  It will be modified whenever buf is
 *                    grown.
 *              fp:   the stream to read.  (Must be open for reading!)
 * Returns: A pointer to the data read (same as *buf), or NULL if an error
 *          or EOF occurs and no data is available.
 * Note: trailing newlines are stripped from data.
 */
char *fget_line (char **buf, size_t *size, FILE *fp)
{
  size_t len;

  /* If we don't have a buffer yet, get one. */
  if (*size == 0)
    {
      *size = LINE_CHUNK;
      *buf = malloc(*size);
      if (*buf == NULL)
	{
	  log_error("fget_line: malloc failed: %m");
	  return NULL;
	}
    }

  /* Read some data, returning NULL if we fail. */
  if (fgets(*buf, *size, fp) == NULL)
    return NULL;

  while (1)
    {
      len = strlen(*buf);
      /* If the string is empty, return NULL.
       * (This shouldn't ever happen unless fp contains '\0' characters.)
       */
      if (len == 0)
	return NULL;
      /* If the string contains a full line, return that. */
      if ((*buf)[len-1] == '\n') {
	(*buf)[len-1] = '\0';
	return *buf;
      }
      /* If the line is shorter than *buf is, we ran into EOF; return data. */
      if (len+1 < *size)
	return *buf;

      /* OK, the only other option is that our line was too short... */
      *size += LINE_CHUNK;
      *buf = realloc(*buf, *size);
      if (*buf == NULL)
	{
	  log_error("fget_line: realloc failed: %m");
	  return NULL;
	}

      /* Try reading more data and see if we fare better. */
      if (fgets((*buf)+len, (*size)-len, fp) == NULL)
	return *buf;
    }
}

/* Read a line containing a specified fixed string followed by the
 * ASCII representation of a long value, and extract the value.  If
 * the line read does not start with the fixed string, report an error.
 *
 * Arguments:   val:  pointer to a long variable to store the result in.
 *              lead: String expected to appear at the start of the line.
 *              buf:  Address of a char* variable used as a buffer, as
 *                    for fget_line [ie, initialize to NULL, free afterwards].
 *              size: Address of a size_t variable containing the size of
 *                    *buf, as for fget_line [ie, initialize to 0].
 *              fp:   the stream to read.  (Must be open for reading!)
 * Returns: SUCCESS on success, ERROR on failure
 */
ERRCODE expect_long (long *val, char *lead, char **buf, size_t *size, FILE *fp)
{
  char *end;
  size_t llen = strlen(lead);

  if (! fget_line(buf, size, fp)) {
    log_error("can't read a line while expecting '%s'", lead);
    return ERROR;
  }
  if (strncmp(*buf, lead, llen)) {
    log_error("oops, I was expecting '%s' but I got '%s'", lead, *buf);
    return ERROR;
  }

  *val = strtol((*buf)+llen, &end, 10);
  if ((end == (*buf)+llen) || (*end != '\0')) {
    log_error("line doesn't end after expected long at '%s'", lead);
    return ERROR;
  }
  return SUCCESS;
}

/* Read a line containing a specified fixed string followed by a
 * string value with fixed manimum width.  If the line read does not
 * start with the fixed string, report an error.
 *
 * Arguments:   dst:    pointer to a char[] variable to store the result in.
 *		dstlen: size of the dst array.
 *              lead:   String expected to appear at the start of the line.
 *              buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 *              fp:     the stream to read.  (Must be open for reading!)
 * Returns: SUCCESS on success, ERROR on failure
 */
ERRCODE expect_str_fixwid (char *dst, size_t dstlen, char *lead,
			   char **buf, size_t *size, FILE *fp)
{
  size_t llen = strlen(lead);

  if (! fget_line(buf, size, fp)) {
    log_error("can't read a line while expecting '%s'", lead);
    return ERROR;
  }
  if (strncmp(*buf, lead, llen)) {
    log_error("oops, I was expecting '%s' but I got '%s'", lead, *buf);
    return ERROR;
  }

  strncpy(dst, (*buf)+llen, dstlen);
  dst[dstlen-1] = '\0';
  return SUCCESS;
}
