/*
 * This file is part of the OLC On-Line Consulting System.  It
 * contains functions for backing up the state of the OLC system in
 * the "dump" ASCII format.
 *
 *      Win Treese (MIT Project Athena)
 *      Ken Raeburn (MIT Information Systems)
 *      Tom Coppeto, Chris VanHaren, Lucien Van Elsen (MIT Project Athena)
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: backup-dump.c,v 1.2 1999-06-28 22:52:38 ghudson Exp $
 */

#ifndef SABER
#ifndef lint
static char rcsid[] ="$Id: backup-dump.c,v 1.2 1999-06-28 22:52:38 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "olcd.h"

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */
#include <sys/file.h>           /* System file defs. */
#include <setjmp.h>             /* For string validation kludge */
#include <pwd.h>

static ERRCODE ascii_write_knuckle_info (FILE *fd , KNUCKLE *knuckle );
static ERRCODE ascii_write_user_info (FILE *fd , USER *user );
static ERRCODE ascii_read_knuckle_info (FILE *fp , KNUCKLE *knuck,
					  char **buf, size_t *size);
static ERRCODE ascii_read_user_info (FILE *fp, USER *user,
				       char **buf, size_t *size);

/* NOTE: This file contains a LOT of hardcoded strings.  We could #define
 *   them all or something, but you probably don't want to change them
 *   anyway, since it makes migration from older formats painful.  If you
 *   really want to change the format, make sure you have a functional
 *   binary backup, shut down the server, install the binary with the new
 *   format, REMOVE the old-format ASCII backup and start the daemon on the
 *   same platform (which will read the binary backup).
 */

/*** functions for writing data ***/

static ERRCODE ascii_write_user_info (FILE *fp, USER *user)
{
  int i;

  fprintf(fp,"\n\nuser...");
  fprintf(fp,"\nuid:      %d", user->uid);
  fprintf(fp,"\nusername: %s", user->username);
  fprintf(fp,"\nrealname: %s", user->realname);
  fprintf(fp,"\nnickname: %s", user->nickname);
  fprintf(fp,"\ntitle1:   %s", user->title1);
  fprintf(fp,"\ntitle2:   %s", user->title2);
  fprintf(fp,"\nmachine:  %s", user->machine);
  fprintf(fp,"\nrealm:    %s", user->realm);
  fprintf(fp,"\nspeclts:  ");
  for (i=0; i < user->no_specialties; i++)
    fprintf(fp,"%d ", user->specialties[i]);
  fprintf(fp,"\nnum_spec: %d", user->no_specialties);
  fprintf(fp,"\npermssns: %d", user->permissions);
  fprintf(fp,"\nstatus:   %d", user->status);
  fprintf(fp,"\nnumknuck: %d", user->no_knuckles);
  fprintf(fp,"\nmax_ask:  %d", user->max_ask);
  fprintf(fp,"\nmax_ans:  %d", user->max_answer);
  fprintf(fp,"\n");

  return(SUCCESS);
}


static ERRCODE ascii_write_knuckle_info (FILE *fp, KNUCKLE *knuckle)
{
  int i;

  fprintf(fp,"\n\nknuckle...");
  fprintf(fp,"\ntitle:        %s", knuckle->title);
  fprintf(fp,"\ninstance:     %d", knuckle->instance);
  fprintf(fp,"\ntimestamp:    %ld", knuckle->timestamp);
  fprintf(fp,"\nstatus:       %d", knuckle->status);
  fprintf(fp,"\ncusername:    %s", ((knuckle->cusername &&
				     knuckle->cusername[0])
				    ? knuckle->cusername : "(nobody)"));
  fprintf(fp,"\ncinstance:    %d", knuckle->cinstance);
  fprintf(fp,"\nnm_file:      %s", knuckle->nm_file);
  fprintf(fp,"\nnew_messages: %d", knuckle->new_messages);

  if(knuckle->question != (QUESTION *) NULL && 
     knuckle->question->owner == knuckle) 
    {
      fprintf(fp,"\n\nquestion...");
      fprintf(fp,"\nlogfile:      %s", knuckle->question->logfile);
      fprintf(fp,"\ninfofile:     %s", knuckle->question->infofile);
      fprintf(fp,"\nseen:         ");
      for (i=0; i< knuckle->question->nseen; i++)
	fprintf(fp,"%d ", knuckle->question->seen[i]);
      fprintf(fp,"\nnseen:        %d", knuckle->question->nseen);
      fprintf(fp,"\ntopic:        %s", knuckle->question->topic);
      fprintf(fp,"\ntopic_code:   %d", knuckle->question->topic_code);
      fprintf(fp,"\ntitle:        %s", knuckle->question->title);
      fprintf(fp,"\nnote:         %s", knuckle->question->note);
      fprintf(fp,"\ncomment:      %s", knuckle->question->comment);
      fprintf(fp,"\nstats...");
      fprintf(fp,"\nnum_crepl     %d", knuckle->question->stats.n_crepl);
      fprintf(fp,"\nnum_cmail     %d", knuckle->question->stats.n_cmail);
      fprintf(fp,"\nnum_urepl     %d", knuckle->question->stats.n_urepl);
      fprintf(fp,"\ntime_to_fr    %d", knuckle->question->stats.time_to_fr);
    }
  fprintf(fp, "\n");

  return(SUCCESS);
}

/*** dump the queue ***/

ERRCODE dump_data(char *file)
{
  FILE *fp;
  int no_knuckles;
  KNUCKLE **k_ptr, **k_again;	/* Current user. */
  int i=0;
  extern int request_count;
  extern int request_counts[OLC_NUM_REQUESTS];
  extern PROC *proc_list;
  
  ensure_consistent_state();
  fp = fopen(file, "w");

  if (fp == NULL)
    {
      log_error("dump_data: unable to open ascii dump file: %m");
      return ERROR;
    }

  fprintf(fp, "\nRequest count data...\n");
  fprintf(fp, "Total number of requests: %d\n", request_count);
  while (proc_list[i].proc_code != UNKNOWN_REQUEST)
    {
      fprintf(fp, " %d\t%s\n", request_counts[i], proc_list[i].description);
      i++;
    }

  for (k_ptr = Knuckle_List, no_knuckles = 0; *k_ptr; k_ptr++)
    no_knuckles++;
  fprintf(fp,"\nNumber of knuckles: %d\n",no_knuckles);
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if(((*k_ptr)->instance == 0) && (((*k_ptr)->user->no_knuckles > 1) ||
				       is_active((*k_ptr))))
	{
	  if (ascii_write_user_info(fp, (*k_ptr)->user) != SUCCESS) 
	    {
	      log_error("dump_data: unable to write_user");
	      fclose(fp);
	      return ERROR;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for (i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (ascii_write_knuckle_info(fp, k_again[i]) != SUCCESS) 
	      {
		log_error("dump_data: unable to write_knuckle");
		fclose(fp);
		return ERROR;
	      }
	}
    }
  fclose(fp);

  return SUCCESS;
}

/*** data input helpers ***/

/* Skip the header (1 blank line + stats + 1 bl + knuckles + >=2 bl) and
 * read the following line.
 *
 * Arguments:   buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 *              fp:     the stream to read.  (Must be open for reading!)
 * Returns: SUCCESS on success, ERROR on failure
 *          Running into the end of file in header is considered failure.
 * Note: leaves the delimiter line of the first chunk of data in *buf.
 */
static ERRCODE read_header (char **buf, size_t *size, FILE *fp)
{
  /* read a blank line */
  if (! fget_line(buf, size, fp) || ((*buf)[0] != '\0'))
    {
      log_error("ASCII backup file is empty or broken!");
      return ERROR;
    }
  /* read a non-blank line */
  if (! fget_line(buf, size, fp) || ((*buf)[0] == '\0'))
    {
      log_error("ASCII backup file is broken!  [stats expected]");
      return ERROR;
    }
  /* read until the next blank line */
  while ((*buf)[0] != '\0')
    {
      if (! fget_line(buf, size, fp))
	{
	  log_error("ASCII backup file is broken!  [error reading stats]");
	  return ERROR;
	}
    }
  /* read a non-blank line */
  if (! fget_line(buf, size, fp) || ((*buf)[0] == '\0'))
    {
      log_error("ASCII backup file is broken!  [# knuckles expected]");
      return ERROR;
    }
  /* read a blank line */
  if (! fget_line(buf, size, fp) || ((*buf)[0] != '\0'))
    {
      /* EOF here isn't failure, it just means no knuckles at all */
      if (feof(fp))
	return SUCCESS;
      log_error("ASCII backup file is broken!  [after # knuckles]");
      return ERROR;
    }
  /* read until the next non-blank line */
  while ((*buf)[0] == '\0')
    {
      if (! fget_line(buf, size, fp))
	{
	  /* EOF here isn't failure, it just means no knuckles at all */
	  if (feof(fp))
	    return SUCCESS;
	  log_error("ASCII backup file is broken!  [reading whitespace]");
	  return ERROR;
	}
    }

  return SUCCESS;
}

/* Read at least one blank line followrd by a non-blank line, store into buf.
 *
 * Arguments:   buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 *              fp:     the stream to read.  (Must be open for reading!)
 * Returns: SUCCESS on success, ERROR on failure.
 *          Running into the end of file is *not* considered failure!
 * Note: leaves the delimiter line of the next chunk of data in *buf.
 */
static ERRCODE read_delim (char **buf, size_t *size, FILE *fp)
{
  if (! fget_line(buf, size, fp))
    {
      if (feof(fp))
	{
	  (*buf)[0] = '\0';
	  return SUCCESS;  /* EOF is not a failure */
	}
      else
	{
	  log_error("ASCII undump: error reading [blank] line: %m");
	  return ERROR;
	}
    }
  if ((*buf)[0] != '\0')
    {
      log_error("ASCII undump: no blank line preceding the delimiter");
      return ERROR;
    }

  while ((*buf)[0] == '\0')
    {
      if (! fget_line(buf, size, fp))
	{
	  if (feof(fp))
	    {
	      (*buf)[0] = '\0';
	      return SUCCESS;  /* EOF is still not a failure */
	    }
	  else
	    {
	      log_error("ASCII undump: error reading [delim] line: %m");
	      return ERROR;
	    }
	}
    }

  return SUCCESS;
}

/*** functions for reading data ***/

/* Read user data in ASCII format from the input stream.
 *
 * Arguments:   fp:     the stream to read.  (Must be open for reading!)
 *              user:   Pointer to the structure to be filled with the data
 *              buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 * Returns: SUCCESS on success, ERROR on failure
 * Note: leaves the delimiter line of the next chunk of data in *buf.
 */
static ERRCODE ascii_read_user_info(FILE *fp, USER *user,
				    char **buf, size_t *size)
{
  long num;

  /* being here means that *buf contains "user...\0" */

  if (expect_long(&num, "uid:      ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->uid = num;  /* this ensures sanity if user->uid changes type */

  if (expect_str_fixwid(user->username, sizeof(user->username),
			"username: ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->realname, sizeof(user->realname),
			"realname: ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->nickname, sizeof(user->nickname),
			"nickname: ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->title1, sizeof(user->title1),
			"title1:   ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->title2, sizeof(user->title2),
			"title2:   ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->machine, sizeof(user->machine),
			"machine:  ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(user->realm, sizeof(user->realm),
			"realm:    ", buf, size, fp) != SUCCESS)
    return ERROR;

  /* Old specialties-writing code is broken, and the new uses numbers,
   * which will break if the topic list changes while the server is down.
   * So don't bother reading data.  We load_user() at the end, anyway.
   */
  if (! fget_line(buf, size, fp) || strncmp(*buf, "speclts:  ", 10))
    {
      log_error("ASCII undump: no specialties line");
      return ERROR;
    }

  if (! fget_line(buf, size, fp) || strncmp(*buf, "num_spec: ", 10))
    {
      log_error("ASCII undump: no num_specialties line");
      return ERROR;
    }

  if (expect_long(&num, "permssns: ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->permissions = num;

  if (expect_long(&num, "status:   ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->status = num;

  if (expect_long(&num, "numknuck: ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->no_knuckles = num;

  if (expect_long(&num, "max_ask:  ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->max_ask = num;

  if (expect_long(&num, "max_ans:  ", buf, size, fp) != SUCCESS)
    return ERROR;
  user->max_answer = num;

  /* read a blank line, then read a section delimiter into *buf */
  if (read_delim(buf, size, fp) != SUCCESS)
    return ERROR;

  /* restore database info from the database */
  load_user(user);

  return SUCCESS;
}

/* Read knuckle data in ASCII format from the input stream.
 * Does *not* read data for the question associated with knuckle, if any.
 *
 * Arguments:   fp:     the stream to read.  (Must be open for reading!)
 *              knuck:  Pointer to the structure to be filled with the data
 *              buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 * Returns: SUCCESS on success, ERROR on failure
 * Note: leaves the delimiter line of the next chunk of data in *buf.
 */
static ERRCODE ascii_read_knuckle_only(FILE *fp, KNUCKLE *knuck,
				       char **buf, size_t *size)
{
  long num;

  /* being here means that *buf contains "knuckle...\0" */

  /* knuck->title just points to ->title1 or ->title2 of some user.
   * We can just leave it alone here and it'll get fixed by undump_data().
   */
  if (! fget_line(buf, size, fp) || strncmp(*buf, "title:        ", 14))
    {
      log_error("ASCII undump: no title line for knuckle");
      return ERROR;
    }

  if (expect_long(&num, "instance:     ", buf, size, fp) != SUCCESS)
    return ERROR;
  knuck->instance = num;

  if (expect_long(&num, "timestamp:    ", buf, size, fp) != SUCCESS)
    return ERROR;
  knuck->timestamp = num;

  if (expect_long(&num, "status:       ", buf, size, fp) != SUCCESS)
    return ERROR;
  knuck->status = num;

  if (expect_str_fixwid(knuck->cusername, sizeof(knuck->cusername),
			"cusername:    ", buf, size, fp) != SUCCESS)
    return ERROR;
  /* If username is empty, "(nobody)" is written to file.  Deal. */
  if (! strcmp(knuck->cusername, "(nobody)"))
    {
      /* cusername is currently a fixed-size array */
      knuck->cusername[0] = '\0';
    }

  if (expect_long(&num, "cinstance:    ", buf, size, fp) != SUCCESS)
    return ERROR;
  knuck->cinstance = num;

  if (expect_str_fixwid(knuck->nm_file, sizeof(knuck->nm_file),
			"nm_file:      ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_long(&num, "new_messages: ", buf, size, fp) != SUCCESS)
    return ERROR;
  knuck->new_messages = num;

  /* read a blank line, then read a section delimiter into *buf */
  if (read_delim(buf, size, fp) != SUCCESS)
    return ERROR;

  return SUCCESS;
}


/* Read question data in ASCII format from the input stream.
 *
 * Arguments:   fp:     the stream to read.  (Must be open for reading!)
 *              quest:  Pointer to the structure to be filled with the data
 *              buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 * Returns: SUCCESS on success, ERROR on failure
 * Note: leaves the delimiter line of the next chunk of data in *buf.
 */
static ERRCODE ascii_read_question(FILE *fp, QUESTION *quest,
				   char **buf, size_t *size)
{
  char *next_num, *end_num;
  int index;
  long num;

  /* being here means that *buf contains "question...\0" */

  if (expect_str_fixwid(quest->logfile, sizeof(quest->logfile),
			"logfile:      ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(quest->infofile, sizeof(quest->infofile),
			"infofile:     ", buf, size, fp) != SUCCESS)
    return ERROR;

  /* 'seen:' is a space-separated array of integers */
  if (! fget_line(buf, size, fp) || strncmp(*buf, "seen:         ", 14))
    {
      log_error("ASCII undump: error reading 'seen:' header");
      return ERROR;
    }
  index = 0;
  next_num = (*buf) + 14;
  while (*next_num && (index < MAX_SEEN))
    {
      quest->seen[index] = strtol(next_num, &end_num, 10);
      index++;

      if ((end_num == next_num)
	  || ((*end_num != '\0') && (*end_num != ' ')))
	{
	  log_error("ASCII undump: not a number while reading list for 'seen:'");
	  return ERROR;
	}

      next_num = end_num+1;
    }
  quest->nseen = index;

  if (expect_long(&num, "nseen:        ", buf, size, fp) != SUCCESS)
    return ERROR;
  if (quest->nseen != num)
    {
      log_error("ASCII undump: nseen doesn't match the size of seen, ignored.");
    }

  if (expect_str_fixwid(quest->topic, sizeof(quest->topic),
			"topic:        ", buf, size, fp) != SUCCESS)
    return ERROR;
  quest->topic_code = get_topic_code(quest->topic);

  if (expect_long(&num, "topic_code:   ", buf, size, fp) != SUCCESS)
    return ERROR;
  if (quest->topic_code != num)
    {
      log_status("topic number for '%s' changed (%d -> %d)",
		 quest->topic, num, quest->topic_code);
    }
  
  if (expect_str_fixwid(quest->title, sizeof(quest->title),
			"title:        ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(quest->note, sizeof(quest->note),
			"note:         ", buf, size, fp) != SUCCESS)
    return ERROR;

  if (expect_str_fixwid(quest->comment, sizeof(quest->comment),
			"comment:      ", buf, size, fp) != SUCCESS)
    return ERROR;

  /* plus "stats..." and the question statistics */

  if (! fget_line(buf, size, fp) || strcmp(*buf, "stats..."))
    {
      log_error("error reading 'stats...' delimiter");
      return ERROR;
    }

  if (expect_long(&num, "num_crepl     ", buf, size, fp) != SUCCESS)
    return ERROR;
  quest->stats.n_crepl = num;

  if (expect_long(&num, "num_cmail     ", buf, size, fp) != SUCCESS)
    return ERROR;
  quest->stats.n_cmail = num;

  if (expect_long(&num, "num_urepl     ", buf, size, fp) != SUCCESS)
    return ERROR;
  quest->stats.n_urepl = num;

  if (expect_long(&num, "time_to_fr    ", buf, size, fp) != SUCCESS)
    return ERROR;
  quest->stats.time_to_fr = num;

  /* read a blank line, then read a section delimiter into *buf */
  if (read_delim(buf, size, fp) != SUCCESS)
    return ERROR;

  return(SUCCESS);
}

/* Read knuckle data in ASCII format, plus associated question data if
 * any, from the input stream.
 *
 * Arguments:   fp:     the stream to read.  (Must be open for reading!)
 *              knuck:  Pointer to the structure to be filled with the data
 *              buf:    Address of a char* variable used as a buffer, as
 *                      for fget_line [ie, initialize to NULL, free afterwards]
 *              size:   Address of a size_t variable containing the size of
 *                      *buf, as for fget_line [ie, initialize to 0].
 * Returns: SUCCESS on success, ERROR on failure
 * Note: leaves the delimiter line of the next chunk of data in *buf.
 */
static ERRCODE ascii_read_knuckle_info(FILE *fp, KNUCKLE *knuck,
				       char **buf, size_t *size)
{
  /* being here means that *buf contains "knuckle...\0" */

  if (ascii_read_knuckle_only(fp, knuck, buf, size) != SUCCESS)
    return ERROR;

  /* Check the delimiter in *buf and see if we'll be loading a question */
  if (feof(fp) || strcmp(*buf, "question..."))
    knuck->question = NULL;
  else
    {
      knuck->question = malloc(sizeof(QUESTION));
      if (knuck->question == NULL)
	{
	  log_error("question malloc failed: %m");
	  return ERROR;
	}

      if (ascii_read_question(fp, knuck->question, buf, size) != SUCCESS)
	return ERROR;
      knuck->question->owner = knuck;
    }
  
  /* at this point *buf contains the start of the next section (unless EOF) */
  return SUCCESS;
}

/*** undump the queue ***/

ERRCODE undump_data(char *file)
{
  int nk, status;
  KNUCKLE *kptr;
  USER *uptr;
  
  FILE* fp;
  char *buf = NULL;
  size_t size = 0;

  log_status("Loading ascii data...");

  fp = fopen(file, "r");
  if (fp == NULL) 
    {
      log_error("undump_data: unable to open backup file: %m");
      return ERROR;
    }
  
  /* Skip the header (1 blank line + stats + 1 bl + knuckles + >=2 bl) and
   * read the following line.
   */
  if (read_header(&buf, &size, fp) != SUCCESS)
    return ERROR;
  
  /* We have the first section header in buf; go! */

  while (! feof(fp))
    {
      /* User header should be in the buffer; read the data. */

      if (strcmp(buf, "user..."))
	{
	  log_error("undump_data: failed to find user, saw '%s'", buf);
	  return ERROR;
	}

#ifdef DEBUG_UNDUMP
      log_debug("undump_data: reading user data");
#endif /* DEBUG_UNDUMP */
	  
      uptr = malloc(sizeof(USER));
      if(uptr == NULL)
	{
	  log_error("load_data: user malloc: %m");
	  return ERROR;
	}
      status = ascii_read_user_info(fp, uptr, &buf, &size);
      if(status != SUCCESS)
	return ERROR;	/* error message was already printed */
	  
      nk = uptr->no_knuckles;
      uptr->no_knuckles = 0;
      uptr->knuckles = (KNUCKLE **) NULL;
      

      /* Read in data for all of users' knuckles. */
#ifdef DEBUG_UNDUMP
      log_debug("undump_data: reading knuckle data");
#endif /* DEBUG_UNDUMP */

      while (!feof(fp) && !strcmp(buf, "knuckle...")) {
	kptr = malloc(sizeof(KNUCKLE));
	if(kptr == NULL)
	  {
	    log_error("load_data: knuckle malloc: %m");
	    return ERROR;
	  }
	status = ascii_read_knuckle_info(fp, kptr, &buf, &size);
	if(status != SUCCESS)
	  return ERROR;	/* error message was already printed */

	status = insert_knuckle(kptr);
	if (status != SUCCESS) {
	  log_error("Failed to insert a new knuckle!");
	  /* give up on the current knuckle and go to the next */
	  continue;
	}

	kptr->user = uptr;
	if (kptr->question == NULL)
	  kptr->title = uptr->title2;
	else
	  kptr->title = uptr->title1;
	insert_knuckle_in_user(kptr,uptr);

#ifdef NEED_TO_RESET_NM_FILE
	sprintf(kptr->nm_file,"%s/%s_%d.nm", LOG_DIR, kptr->user->username,
		kptr->instance);
#endif /* NEED_TO_RESET_NM_FILE */

	kptr->user->no_knuckles++;
      }
      
      /* Done with knuckles; complain if needed. */

      if (nk != uptr->no_knuckles)
	log_error("Mismatch: %d knuckles expected, %d found for %s",
		  nk, uptr->no_knuckles, uptr->username);
    }
  
  reconnect_knuckles();
  ensure_consistent_state();
  log_status("Un-dumping complete.");
  
  fclose(fp);
  needs_backup = 0; 

  return SUCCESS;
}
