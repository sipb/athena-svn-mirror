/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for logging each conversation.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems / Project Athena
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/log.c,v $
 *      $Author: raeburn $
 */

#ifndef lint
static const char rcsid[] =
    "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/log.c,v 1.10 1990-01-05 06:22:52 raeburn Exp $";
#endif


#include <sys/time.h>		/* System time definitions. */
#include <sys/types.h>		/* System type declarations. */
#include <sys/stat.h>		/* File status definitions. */
#include <sys/file.h>
#include <string.h>		/* Defs. for string functions. */
#include <syslog.h>             /* syslog do hickies */

#include <stdarg.h>

#include <olc/olc.h>
#include <olcd.h>

static FILE *status_log = (FILE *)NULL;
static FILE *error_log = (FILE *)NULL;
static FILE *admin_log = (FILE *) NULL;

static ERRCODE terminate_log_crash (KNUCKLE *);
static ERRCODE dispose_of_log (KNUCKLE *, int);

/*
 * Function:	write_line_to_log() writes a single line of text into a
 *			log file.
 * Arguments:	log:	A FILE pointer to the log file.
 *		line:	A pointer to the text to be written.
 * Returns:	Nothing.
 * Notes:
 *	Write the text into the file, making sure that the line ends with a
 *	newline character.  If it does not, add one.
 */

write_line_to_log(struct _iobuf *log, char *line)
{
	 
	fputs (line, log);
	if (line[strlen(line) - 1] != '\n')
		fputc('\n', log);
}

/*
 * Function:	format_line_to_user_log() writes a single line of text into a
 *			log file.
 * Arguments:	log:	A FILE pointer to the log file.
 *		line:	A pointer to the text to be written.
 * Returns:	Nothing.
 * Notes:
 *	Write the text into the file, making sure that the line ends with a
 *	newline character.  If it does not, add one. This function provides
 *      a hook for better formatting of the user logs.
 */

format_line_to_user_log(struct _iobuf *log, char *line)
{
    fputs (line, log);
}
	
/*
 * Function:	log_message() writes a message into the log of a user's
 *			 conversation.
 * Arguments:	user:		Ptr. to current user structure.
 *		message:	Message to be written into the log.
 *		from:		Flag indicating who sent the message.
 * Returns:	SUCCESS if the message is successfully logged, ERROR otherwise.
 * Notes:
 *	If we can't open the log file, record the error and return ERROR.
 *	Otherwise, find out the time, create a header based on the sender,
 *	and write the time, header, and message into the user's log.
 *	Finally, update the information about the log length in the user
 *	structure.
 */

ERRCODE
log_log(KNUCKLE *knuckle, char *message, char *header)
{
  FILE *log;		
  char error[DB_LINE];

  if ((log = fopen(knuckle->question->logfile, "a")) == (FILE *)NULL) 
    {
      (void) sprintf(error, "log_log: can't open log %s\n",
	      knuckle->question->logfile);
      log_error(error);
      return(ERROR);
    }
  fprintf(log, "\n%s",header);
  format_line_to_user_log(log,message);

  fclose(log);
  return(SUCCESS);
}


char * fmt (const char * format, ...) {
    va_list pvar;
    static char buf[BUFSIZ];
    FILE strbuf;
    int len;

    va_start (pvar, format);
    /* copied from sprintf.c, BSD */
    strbuf._flag = _IOWRT + _IOSTRG;
    strbuf._ptr = buf;
    strbuf._cnt = 32767;
    bzero (buf, sizeof (buf));
    len = _doprnt (format, pvar, &strbuf);
#if 0
    buf[len] = '\0';
#endif
    va_end (pvar);
    return buf;
}
    


void log_daemon(KNUCKLE *knuckle, char *message)
{
  char time[TIME_SIZE];	
  char header[DB_LINE]; 
  
  time_now(time);
  (void) sprintf(header,"--- %s\n    [%s]\n ",message,time);
  (void) log_log(knuckle,"",header);
}


void log_message(KNUCKLE *owner, KNUCKLE *sender, char *message)
{
  char time[TIME_SIZE];	
  char header[DB_LINE];
  
  time_now(time);
  (void) sprintf(header, "*** Reply from %s %s@%s (%d)\n    [%s]\n",
	  sender->title, sender->user->username, 
	  sender->user->machine, sender->instance, time);
  (void) log_log(owner,message,header);
}


void log_mail(KNUCKLE *owner, KNUCKLE *sender, char *message)
{
  char time[TIME_SIZE];	
  char header[DB_LINE];
  
  time_now(time);
  (void) sprintf(header, "*** Mail from %s %s@%s (%d)\n    [%s]\n",
	  sender->title, sender->user->username, 
	  sender->user->machine, sender->instance, time);
  (void) log_log(owner,message,header);
}


void log_comment(KNUCKLE *owner, KNUCKLE *sender, char *message)
{
  char time[TIME_SIZE];
  char header[DB_LINE];

  time_now(time);
  (void) sprintf(header, "--- Comment by %s %s@%s (%d)\n    [%s]\n",
		 sender->title, sender->user->username,
		 sender->user->machine, sender->instance, time);

  (void) log_log(owner,message,header);
}


void log_description(KNUCKLE *owner, KNUCKLE *sender, char *message)
{
  char time[TIME_SIZE];
  char header[DB_LINE];

  time_now(time);
  (void) sprintf(header,
		 "--- Description changed by %s %s@%s (%d)\n    [%s]\n",
		 sender->title, sender->user->username,
		 sender->user->machine, sender->instance, time);

  (void) log_log(owner,message,header);
}


/*
 * Function:	log_error() writes an error message into the daemon's error
 *			log file.
 * Arguments:	message:	Error message to be written.
 * Returns:	nothing
 * Notes:
 *	First, open the error log file, printing an error message and dying 
 *	if an error occurs here.  Then, get the system time and print a
 *	formatted version in the error log, followed by the error message.  
 *	Finally, close the file and return.
 */

void log_error(char *message)
{
  char time_buf[32];
  char *time_string = &time_buf[0];


#ifdef TEST
  
  /*
   * print error to stderr for debugging
   */ 

  error_log = stderr;
  time_now(time_string);
  fprintf(error_log, "%s ", time_string);
  write_line_to_log(error_log, message);
  return;
#endif TEST


#ifdef SYSLOG

  /*
   * log errors via syslog
   */

  syslog(LOG_ERR,message);
  return;
#endif SYSLOG

#ifndef TEST

  /* 
   * oh well, use homegrown logging mechanism
   */

  (void) fflush(stderr);
  if (error_log == (FILE *)NULL) 
    {
      error_log = fopen(ERROR_LOG, "a");
      if (error_log == (FILE *)NULL) 
	{
	  printf("OLC log_error: Error occurred trying to log error.\n");
	  printf("Unable to open file %s\n", ERROR_LOG);
	  olc_broadcast_message("syserror", "Unable to open log file (fatal)","system");
	  olc_broadcast_message("syserror", "log errors... exitting!","system");
	  exit(1);
	}
    }

  time_now(time_string);
  fprintf(error_log, "%s ", time_string);
  write_line_to_log(error_log, message);
  olc_broadcast_message("syserror",message);
  (void) fflush(error_log);
#endif

}

/*
 * Function:	log_status() writes a message to the olcd status log.
 * Arguments:	message:	Message to be written.
 * Returns:	nothing
 * Notes:
 *	First, open the status log file, recording an error and exiting 
 *	if one occurs.  Then, write the formatted current time and the
 *	status message into the status log.  Finally, close the log file
 *	and return.
 */

void log_status(char *message)
{
  char time_buf[32];

#ifdef TEST

  /*
   * print to stderr, for debugging
   */

  status_log = stderr;
  time_now(time_buf);
  fprintf(status_log, "%s ", time_buf);
  write_line_to_log(status_log, message);
  return;
#endif

#ifdef SYSLOG

  /*
   * log errors via syslog
   */

  syslog(LOG_INFO,message);
  return;
#endif

#ifndef TEST

  /* 
   * oh well, use homegrown logging mechanism
   */

  if (status_log == (FILE *) NULL)
    {
      status_log = fopen(STATUS_LOG, "a");
      if (status_log == (FILE *)NULL) 
	{
	  log_error("log_status: can't append to status log");
	  exit(1);
	}
    }

  time_now(time_buf);
  fprintf(status_log, "%s ", time_buf);
  write_line_to_log(status_log, message);
  (void) fflush(status_log);

  return;
#endif
}



void
log_admin(char *message)
{
  char time_buf[32];

#ifdef TEST

  /*
   * print to stderr, for debugging
   */

  status_log = stderr;
  time_now(time_buf);
  fprintf(status_log, "%s ", time_buf);
  write_line_to_log(admin_log, message);
  return;
#endif

#ifdef SYSLOG

  /*
   * log errors via syslog
   */

  syslog(LOG_INFO,message);
  return;
#endif

#ifndef TEST

  /* 
   * oh well, use homegrown logging mechanism
   */

  if (admin_log == (FILE *) NULL)
    {
      admin_log = fopen(ADMIN_LOG, "a");
      if (admin_log == (FILE *)NULL) 
	{
	  log_error("log_status: can't append to status log");
	  exit(1);
	}
    }

  time_now(time_buf);
  fprintf(admin_log, "%s ", time_buf);
  write_line_to_log(admin_log, message);
  (void) fflush(admin_log);

  return;
#endif
}

/*
 * Function:	init_log() initializes a log file for a user.
 * Arguments:	user:		A pointer to the user's user structure.
 *		question:	User's initial question.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, construct the name of the user's log file.  Then attempt to
 *	open the file for reading.  If the open does not fail, then a file by
 *	that name already exists.  In this case, record an error message and
 *	terminate the log appropriately.  Otherwise, open the
 *	file for writing and print the initial information into the file.
 *	Finally, close the file, update the length of the log, and return.
 */

ERRCODE
init_log(KNUCKLE *knuckle, char *question)
{
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERRSIZE];	        /* Error message. */
  char current_time[32];	/* Current time value. */
  char topic[TOPIC_SIZE];	/* Real topic. */
	
#ifdef TEST
  log_status (fmt ("init_log: %s (%d)\n",knuckle->user->username,
		   knuckle->instance));
#endif

  (void) sprintf(knuckle->question->logfile, "%s/%s_%d.log", LOG_DIR, 
	  knuckle->user->username,knuckle->instance);
  if ((logfile = fopen(knuckle->question->logfile, "r")) != (FILE *) NULL) 
    {
      (void) sprintf(error, 
		     "init_log: already a log file %s, moving it to log.",
			knuckle->question->logfile);
      log_error(error);
      (void) fclose(logfile);
      (void) strcpy(topic, knuckle->question->topic);
      (void) sprintf(knuckle->question->topic, "crash");
      terminate_log_crash(knuckle);
      (void) strcpy(knuckle->question->topic, topic);
    }

  if ((logfile = fopen(knuckle->question->logfile, "w")) == (FILE *)NULL) 
    {
      (void) sprintf(error, "init_log: can't open log file %s",
	      knuckle->question->logfile);
      log_error(error);
      return(ERROR);
    }

  time_now(current_time);
  fprintf(logfile, "Log Initiated for %s %s [%d] (%s@%s)\n[%s]\n\n",
	  knuckle->title,
	  knuckle->user->realname, 
	  knuckle->instance,
	  knuckle->user->username,
	  knuckle->user->machine,
	  current_time);

  fprintf(logfile, "Topic:\t\t%s\n\n", knuckle->question->topic);
  fprintf(logfile, "Question:\n");
  write_line_to_log(logfile, question);
  write_line_to_log(logfile,"___________________________________________________________\n\n");
  fprintf(logfile, "\n");
  (void) fclose(logfile);
  return(SUCCESS);
}

/*
 * Function:	terminate_log_answered() ends an answered question.
 * Arguments:	user:		Name of the user whose log we are closing.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, construct the name of the user's log file.  Then attempt to open
 *	the file, recording an error if one occurs.  If the open is successful,
 *	note that the conversation is over, dispose of the log, and return.
 */

ERRCODE
terminate_log_answered(KNUCKLE *knuckle)
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERRSIZE];	        /* Error message. */
  char time_buf[32];	        /* Current time. */
	
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
      perror("terminate_log_answered: fopen");
      perror(question->logfile);
      (void) sprintf(error,
		     "terminate_log_answered: can't open temporary log %s",
		     question->logfile);
      log_error(error);
      return(ERROR);
    }

  time_now(time_buf);
  fprintf(logfile, "\n--- Conversation terminated at %.24s\n", time_buf);
  fprintf(logfile, "\n--- Title: %s\n", question->title);

  (void) fclose(logfile);
  if (dispose_of_log(knuckle, ANSWERED) == ERROR)
    return(ERROR);
  return(SUCCESS);
}

/*
 * Function:	terminate_log_unanswered() closes a user's log when his
 *			question remains unanswered.
 * Arguments:	user:	Ptr. to current user structure.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, open the user's log file.  If we can't, log an error message
 *	and return.  Otherwise, write a message about the termination into
 *	the log and dispose of it. This should be separate in case we want
 *      to put these elsewhere.
 */

ERRCODE
terminate_log_unanswered(KNUCKLE *knuckle)
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERRSIZE];	/* Error message. */
  char current_time[32];	/* Current time. */
  
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
    perror(question->logfile);
    (void) sprintf(error,
		   "terminate_log_unanswered: can't open temp. log %s",
		   question->logfile);
    log_error(error);
    return(ERROR);
  }
  time_now(current_time);
  fprintf(logfile, 
	  "\n--- Session terminated without answer at %s\n",
	  current_time);
  (void) fclose(logfile);
  if (dispose_of_log(knuckle, UNANSWERED) == ERROR)
    return(ERROR);
  return(SUCCESS);
}

/*
 * Function:	terminate_log_crash() closes a user's log if it is left over
 *			by the daemon crashing.
 * Arguments:	user:	Ptr. to current user structure.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, open the user's log file.  If we can't, log an error message
 *	and return.  Otherwise, write a message about the termination into
 *	the log and dispose of it.
 */

static ERRCODE
terminate_log_crash(KNUCKLE *knuckle)
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERRSIZE];	        /* Error message. */
  char current_time[32];	/* Current time. */
  
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
      (void) sprintf(error,
		     "terminate_log_unanswered: can't open temp log %s", 
		     question->logfile);
      log_error(error);
      return(ERROR);
    }
  time_now(current_time);
  fprintf(logfile, "\n--- Log file '%s' saved at after daemon crash\n[%s]",
	  index(question->logfile, '/')+1, current_time);
  (void) fclose(logfile);
  if (dispose_of_log(knuckle, UNANSWERED) == ERROR)
    return(ERROR);
  return(SUCCESS);
}

/*
 * Function:	dispose_of_log() sends a user's log to the appropriate
 *			notesfile after	it has been closed.
 * Arguments:	user:		Ptr. to current user structure.
 *		answered:	Flag indicating whether or not the question
 *				has been answered. 
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, construct the name of the user's log file.  Then use stat() to
 *	find out how big it is, allocate some memory for it (plus a byte for a
 *	NULL character at the end), and read it in, checking for errors.
 *	Next, put a NULL at the end of the string and write the buffer to the
 *	notesfile with the specified title.   Finally, close the file, delete
 *	it, free the memory space, and return.
 */

static ERRCODE
dispose_of_log(KNUCKLE *knuckle, int answered)
{
  QUESTION *question;
  char error[ERRSIZE];	        /* Error message. */
  char notesfile[NAME_LENGTH];  /* Name of notesfile. */
  char newfile[NAME_LENGTH];    /* New file name. */
  char topic[NAME_LENGTH];
  int fd;			/* File descriptor of log. */
  int pid, pid2;		/* Process ID for fork. */
  char msgbuf[BUFSIZ];
  
#ifdef TEST
  printf("dispose title: %s\n",knuckle->question->title);
#endif TEST

  question = knuckle->question;

  sprintf(topic,"%s: %s",question->owner->user->username,question->title);
  (void) strcpy(newfile, question->logfile);
  *(rindex(newfile, '/') + 1) = '\0';
  (void) strcat(newfile, "#");
  (void) strcat(newfile, rindex(question->logfile, '/') + 1);

  if (rename(question->logfile, newfile) == -1) 
    {
      perror("dispose_of_log: rename");
      log_error("Can't rename user log.");
      return(ERROR);
    }
  if ((pid = fork()) == -1) 
    {
      perror("dispose_of_log: fork");
      log_error("Can't fork to write to notesfile.");
      return(ERROR);
    }
  else if (pid == 0) 
    {
      (void) sprintf(notesfile, "%s%s", NF_PREFIX, question->topic);
      (void) sprintf(msgbuf, "%s to crash",
		     question->logfile);
      log_status(msgbuf);
      
      if ((fd = open(newfile, O_RDONLY, 0)) == -1) 
	{
	  perror("dispose_of_log: open");
	  (void) sprintf(error, 
			 "dispose_of_log: unable to open %s",
			 question->logfile);
	  log_error(error);
	  exit(ERROR);
	}
      if (dup2(fd, 0) == -1) {
	perror("dispose_of_log: dup2");
	log_error("dispose_of_log: unable to duplicate file descriptor");
	exit(ERROR);
      }
      execl("/usr/sipb/bin/dspipe", "dspipe", notesfile, "-t",topic, 0);
      perror("dispose_of_log: /usr/sipb/bin/dspipe");
      (void) sprintf(error,
		     "dispose_of_log: cannot exec /usr/sipb/bin/dspipe.\n");
      log_error(error);
      execl("/usr/local/dspipe", "dspipe", notesfile, "-t", topic, 0);
      perror("dispose_of_log: /usr/local/dspipe");
      (void) sprintf(error, 
		     "dispose_of_log: cannot exec /usr/local/dspipe, giving up.\n");
      log_error(error);
      exit(0);
    }
  while ((pid2 = wait(0)) != pid) 
    {
      if (pid2 == -1) 
	{
	  perror("dispose_of_log: wait");
	  return(ERROR);
	}
    }
  if (unlink(newfile) == -1) 
    {
      perror("dispose_of_log: unlink");
      (void) sprintf(error, "dispose_of_log: Unable to remove %s",newfile);
      log_error(error);
    }
  return(SUCCESS);
}


		
	     
