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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/log.c,v $
 *	$Id: log.c,v 1.44 1992-07-16 15:34:55 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/log.c,v 1.44 1992-07-16 15:34:55 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/time.h>		/* System time definitions. */
#include <sys/types.h>		/* System type declarations. */
#include <sys/stat.h>		/* File status definitions. */
#include <sys/file.h>
#include <strings.h>		/* Defs. for string functions. */
#ifdef DISCUSS
#include <lumberjack.h>
#endif
#include <olcd.h>

extern int errno;

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

static ERRCODE log_log P((KNUCKLE *knuckle , char *message , char *header , int is_private ));
static ERRCODE terminate_log_crash P((KNUCKLE *knuckle ));
static ERRCODE dispose_of_log P((KNUCKLE *knuckle ));
static char *trans_m_i P((char *os ));

#undef P

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

void
write_line_to_log(log, line)
     FILE *log;
     char *line;
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

void
format_line_to_user_log (log, line)
     FILE *log;
     char *line;
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

static ERRCODE
log_log (knuckle, message, header, is_private)
     KNUCKLE *knuckle;
     char *message;
     char *header;
     int is_private;
{
  FILE *log;
  char error[DB_LINE];
  char censored_filename[NAME_SIZE];

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

  if (!is_private) {
    sprintf(censored_filename,"%s.censored",knuckle->question->logfile);
    if ((log = fopen(censored_filename, "a")) == (FILE *)NULL) 
      {
	(void) sprintf(error, "log_log: can't open log %s\n",
		       knuckle->question->logfile);
	log_error(error);
	return(ERROR);
      }
    fprintf(log, "\n%s",header);
    format_line_to_user_log(log,message);
    fclose(log);
  }

  return(SUCCESS);
}


void
log_daemon(knuckle, message)
     KNUCKLE *knuckle;
     char *message;
{
  char time[TIME_SIZE];	
  char header[DB_LINE]; 
  
  time_now(time);
  (void) sprintf(header,"--- %s\n    [%s]\n ",message,time);
  (void) log_log(knuckle,"",header,0);
}


void
log_message (owner, sender, message)
     KNUCKLE *owner, *sender;
     char *message;
{
  char time[TIME_SIZE];	
  char header[DB_LINE];
  
  time_now(time);
  (void) sprintf(header, "*** Reply from %s %s@%s [%d].\n    [%s]\n",
	  sender->title, sender->user->username, 
	  sender->user->machine, sender->instance, time);
  (void) log_log(owner,message,header,0);
}


void
log_mail(owner,sender,message)
     KNUCKLE *owner,*sender;
     char *message;
{
  char time[TIME_SIZE];	
  char header[DB_LINE];
  
  time_now(time);
  (void) sprintf(header, "*** Mail from %s %s@%s [%d].\n    [%s]\n",
	  sender->title, sender->user->username, 
	  sender->user->machine, sender->instance, time);
  (void) log_log(owner,message,header,0);
}


void
log_comment(owner,sender,message,is_private)
     KNUCKLE *owner,*sender;
     char *message;
     int is_private;
{
  char time[TIME_SIZE];
  char header[DB_LINE];

  time_now(time);
  (void) sprintf(header, "--- %sComment by %s %s@%s [%d].\n    [%s]\n",
		 (is_private ? "Private " : ""),
		 sender->title, sender->user->username,
		 sender->user->machine, sender->instance, time);

  (void) log_log(owner,message,header,is_private);
}


void
log_description(owner,sender,message)
     KNUCKLE *owner,*sender;
     char *message;
{
  char time[TIME_SIZE];
  char header[DB_LINE];
  char msgbuf[NOTE_SIZE+1];

  time_now(time);
  (void) sprintf(header,
		 "--- Description changed by %s %s@%s [%d].\n    [%s]\n",
		 sender->title, sender->user->username,
		 sender->user->machine, sender->instance, time);

  sprintf(msgbuf, "%s\n", message);
  (void) log_log(owner,msgbuf,header,1);
}


void
log_long_description(owner,sender,message)
     KNUCKLE *owner,*sender;
     char *message;
{
  char time[TIME_SIZE];
  char header[DB_LINE];

  time_now(time);
  (void) sprintf(header,
		 "--- Long description changed by %s %s@%s [%d].\n    [%s]\n",
		 sender->title, sender->user->username,
		 sender->user->machine, sender->instance, time);

  (void) log_log(owner,message,header,0);
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
init_log(knuckle, question, machinfo)
     KNUCKLE *knuckle;
     char *question;
     char *machinfo;
{
  FILE *logfile;		/* Ptr. to user's log file. */
  FILE *clogfile;		/* Ptr. to censored user's log file. */
  char censored_filename[NAME_SIZE]; /* Censored user log */
  char error[ERROR_SIZE];	        /* Error message. */
  char current_time[32];	/* Current time value. */
  char topic[TOPIC_SIZE];	/* Real topic. */

  (void) sprintf(knuckle->question->logfile, "%s/%s_%d.log", LOG_DIR, 
	  knuckle->user->username,knuckle->instance);
  (void) sprintf(censored_filename,"%s.censored",knuckle->question->logfile);
  if (access(knuckle->question->logfile,F_OK) == 0)
    {
      (void) sprintf(error, 
		     "init_log: already a log file %s, moving it to log.",
			knuckle->question->logfile);
      log_error(error);
      (void) strcpy(topic, knuckle->question->topic);
      (void) strcpy(knuckle->question->topic, "crash");
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

  if ((clogfile = fopen(censored_filename, "w+")) == (FILE *)NULL) 
    {
      (void) sprintf(error, "init_log: can't open log file %s",
	      knuckle->question->logfile);
      log_error(error);
      return(ERROR);
    }

  time_now(current_time);
  fprintf(logfile, "Log Initiated for %s %s (%s@%s [%d]).\n    [%s]\n\n",
	  knuckle->title,
	  knuckle->user->realname, 
	  knuckle->user->username,
	  knuckle->user->machine,
	  knuckle->instance,
	  current_time);
  fprintf(clogfile, "Log Initiated for %s %s (%s@%s [%d]).\n    [%s]\n\n",
	  knuckle->title,
	  knuckle->user->realname, 
	  knuckle->user->username,
	  knuckle->user->machine,
	  knuckle->instance,
	  current_time);

  fprintf(logfile, "Topic:\t\t%s\n\n", knuckle->question->topic);
  fprintf(logfile, "Question:\n");
  fprintf(clogfile, "Topic:\t\t%s\n\n", knuckle->question->topic);
  fprintf(clogfile, "Question:\n");
  write_line_to_log(logfile, question);
  write_line_to_log(clogfile, question);
  if (machinfo != NULL) 
#ifdef ATHENA
    fprintf(logfile, "\nMachine info:%s\n", trans_m_i(machinfo));
#else
    fprintf(logfile, "\nMachine info:%s\n", machinfo);
#endif
  write_line_to_log(logfile,"___________________________________________________________\n\n");
  write_line_to_log(clogfile,"___________________________________________________________\n\n");
  fprintf(logfile, "\n");
  fprintf(clogfile, "\n");
  (void) fclose(logfile);
  (void) fclose(clogfile);
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
terminate_log_answered(knuckle)
     KNUCKLE *knuckle;
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERROR_SIZE];	        /* Error message. */
  char time_buf[32];	        /* Current time. */
	
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
      (void) sprintf(error,
		     "terminate_log_answered: can't open temporary log %s: %m",
		     question->logfile);
      log_error(error);
      return(ERROR);
    }

  time_now(time_buf);
  fprintf(logfile, "\n--- Conversation terminated at %.24s\n", time_buf);
  fprintf(logfile, "\n--- Title: %s\n", question->title);

  (void) fclose(logfile);
  if (dispose_of_log(knuckle) == ERROR)
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
terminate_log_unanswered(knuckle)
     KNUCKLE *knuckle;
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERROR_SIZE];	/* Error message. */
  char current_time[32];	/* Current time. */
  
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
    (void) sprintf(error,
		   "terminate_log_unanswered: can't open temp. log %s: %m",
		   question->logfile);
    log_error(error);
    return(ERROR);
  }
  time_now(current_time);
  fprintf(logfile, 
	  "\n--- Session terminated without answer at %s\n",
	  current_time);
  (void) fclose(logfile);
  if (dispose_of_log(knuckle) == ERROR)
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
terminate_log_crash(knuckle)
     KNUCKLE *knuckle;
{
  QUESTION *question;
  FILE *logfile;		/* Ptr. to user's log file. */
  char error[ERROR_SIZE];	        /* Error message. */
  char current_time[32];	/* Current time. */
  
  question = knuckle->question;
  if ((logfile = fopen(question->logfile, "a")) == (FILE *)NULL) 
    {
      (void) sprintf(error,
		     "terminate_log_crash: can't open temp log %s", 
		     question->logfile);
      log_error(error);
      return(ERROR);
    }
  time_now(current_time);
  fprintf(logfile, "\n--- Log file '%s' saved after daemon crash.\n[%s]",
	  index(question->logfile, '/')+1, current_time);
  (void) fclose(logfile);
  if (dispose_of_log(knuckle) == ERROR)
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
dispose_of_log(knuckle)
     KNUCKLE *knuckle;
{
  QUESTION *question;		/* Pointer to the question. */
  char newfile[NAME_SIZE];	/* New file name. */
  char ctrlfile[NAME_SIZE];	/* Control file name. */
  FILE *fp;			/* Stream for control file. */
  int pid;			/* Process ID for fork. */
  char msgbuf[BUF_SIZE];	/* Construct messages to be logged. */
  long time_now;		/* time now, in seconds (a unique number) */
  char filename[NAME_SIZE];
  char *p;
  
  sprintf(filename,"%s.censored",knuckle->question->logfile);
  unlink(filename);
  unlink(knuckle->question->infofile);

  time_now = NOW;

  question = knuckle->question;

#ifdef DISCUSS
  /* Spool log off to discuss.... */
  (void) sprintf(ctrlfile, "%s/ctrl%d", DONE_DIR, time_now);
  if (access(ctrlfile,F_OK) == 0) {
    /* Whups, processed that last done too fast... */
    time_now++;
    (void) sprintf(ctrlfile, "%s/ctrl%d", DONE_DIR, time_now);
  }
  (void) sprintf(newfile, "%s/log%d", DONE_DIR, time_now);

  if (rename(question->logfile, newfile) == -1) 
    {
      log_error("Can't rename user log: %m");
      return(ERROR);
    }
  if ((fp = fopen(ctrlfile, "w")) == NULL)
    {
      log_error("Can't create control file: %m");
      return(ERROR);
    }
  
/* Make sure there's no newlines in the title */

  while ((p = index(question->title,'\n')) != NULL)
    *p = ' ';

  fprintf(fp, "%s\n%s\n%s\n%s\n", newfile, question->title,
	  question->topic, question->owner->user->username);
  fclose(fp);
      
#ifdef NO_VFORK
  if ((pid = fork()) == -1) 
#else
  if ((pid = vfork()) == -1) 
#endif
    {
      log_error("Can't fork to dispose of log: %m");
      return(ERROR);
    }
  else if (pid == 0) 
    {
      (void) sprintf(msgbuf, "%s to %s logs",
		     question->logfile, question->topic);
      log_status(msgbuf);

      execl(LUMBERJACK_LOC, "lumberjack", 0);
      sprintf(msgbuf,"dispose_of_log: cannot exec %s: %%m",LUMBERJACK_LOC);
      log_error(msgbuf);
      _exit(0);
    }
#else
  /* Well, get rid of it- if you have some alternative to discuss, you */
  /* should include a method for getting the log into it here. */
  unlink(question->logfile);
#endif

  return(SUCCESS);
}

#ifdef ATHENA
/* Translates obsure stuff from machtype -v in os to "english" */

static char *
  trans_m_i(os)
char *os;
{
  static TRANS *mach, *disp;	/* machine and display translations */
  static int n_mach = -1;	/* number of translations loaded */
  static int n_disp = -1;
  char *memory, *o_mach, *o_disp,*p,*q;
  FILE *trans_file;
  int i,size;
  static char stuff[BUF_SIZE];
  char tmp_buf[BUF_SIZE];

  strcpy(stuff,os);
  if (n_mach == -1) {
    n_mach = n_disp = 0;

    /* Load translations */
    trans_file = fopen(MACH_TRANS_FILE,"r");
    if (trans_file == NULL)
      log_error("trans_m_i: could not open translation file %m");
    else {
      fscanf(trans_file,"%d\n",&n_mach);
      mach = (TRANS *) calloc(n_mach,sizeof(TRANS));
      if (mach == NULL) {
	log_error("trans_m_i: calloc failed");
	return(stuff);
      }
      for (i=0;i<n_mach;i++) {
	fgets(mach[i].orig,80,trans_file);
	fgets(mach[i].trans,80,trans_file);
	size = strlen(mach[i].trans);
	mach[i].trans[size-1] = '\0';
      }
      fscanf(trans_file,"%d\n",&n_disp);
      disp = (TRANS *) calloc(n_disp,sizeof(TRANS));
      for (i=0;i<n_disp;i++) {
	fgets(disp[i].orig,80,trans_file);
	fgets(disp[i].trans,80,trans_file);
	size = strlen(disp[i].trans);
	disp[i].trans[size-1] = '\0';
      }
      fclose(trans_file);
    }
  }

  /* This assumes all machines have a processor and memory (not a bad */
  /* assumption, I hope... */

  /* look for processor field */
  p = index(os,',');
  if (p == NULL)
    return(stuff);
  *p = '\0';
  o_mach = os;
  
  p = p+1;
  /* Need to special case for RS/6000 entry; it is formatted
     processor, display, xxxxxx K
   */
  if (strcmp(o_mach,"POWER") == 0) {
    memory = rindex(p,',');
    if (memory == NULL)
      return(stuff);
    *memory = '\0';
    memory++;
    o_disp = p+1;
  } else {
    /* Look backwards for second comma to get memory */
    memory = rindex(p,',');
    if (memory==NULL)
      return(stuff);
    *memory = ';';
    memory = rindex(p,',');
    if (memory==NULL) {
      memory = p;
      o_disp = "none";
    }
    else {
      *memory = '\0';
      memory++;
      o_disp = p+1;
    }
  }

  /* strip whitespace off of front of machine name */
  while (*o_mach == ' ')
    o_mach++;
  size = strlen(o_mach);
  for(i=0;i<n_mach;i++)
    if (strncmp(o_mach,mach[i].orig,size) == 0) {
      o_mach = mach[i].trans;
      break;
    }
  sprintf(stuff,"\nProcessor: %s\n",o_mach);
  
  while (o_disp != NULL) {
    p = index(o_disp,',');
    if (p != NULL) {
      *p = '\0';
      p++;
    }
    while (*o_disp == ' ')
      o_disp++;
    q = index(o_disp,' ');
    if (q != NULL)
      *q = '\0';

    size = strlen(o_disp);
    for(i=0;i<n_disp;i++)
      if (strncmp(o_disp,disp[i].orig,size) == 0) {
	o_disp = disp[i].trans;
	break;
      }
    sprintf(tmp_buf,"Display  : %s\n",o_disp);
    strcat(stuff,tmp_buf);
    o_disp = p;
  }

  while (*memory == ' ')
    memory++;

  sprintf(tmp_buf,"Memory   : %s",memory);
  strcat(stuff,tmp_buf);
  return(stuff);
}
#endif
