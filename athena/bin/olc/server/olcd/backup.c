/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for backing up the state of the OLC system.
 *
 *      Win Treese
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/backup.c,v $
 *      $Author: raeburn $
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/backup.c,v 1.6 1990-01-05 06:21:46 raeburn Exp $";
#endif

#include <olc/olc.h>
#include <olcd.h>

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */
#include <sys/file.h>           /* System file defs. */
#include <setjmp.h>             /* For string validation kludge */

static void type_error (int, const char *);


/* 
 * Strings used to separate data chunks in the backup file.
 * Plain text is used so the file can be debugged easily.
 */

static char *DATA_SEP      = "  new data:    \0";
static char *USER_SEP      = "  new user:    \0";
static char *KNUCKLE_SEP   = "  new knuckle: \0";
static char *TEXT_SEP      = "  new text:    \0";
static char *BLANK_SEP     = "  a blank:     \0";
static char *TRANS_SEP     = "  new train:   \0";

static type_buf[BUF_SIZE];
static int skip;

static char *
read_msgs(int fd)			
{
  int length;
  char *return_value = (char *)NULL;
  
  if (read_int_from_fd(fd, &length) == ERROR) 
    return((char *)NULL);

  return_value = malloc((unsigned)length*sizeof(char));
  if (return_value == (char *)NULL)
    return(return_value);

  if (read(fd, return_value, length) != length) 
    return((char *)NULL);
	
  return(return_value);
}

static int
write_msgs(int fd, char *msg)
{
  int length;
  
  length = strlen(msg);

  if(write(fd,TEXT_SEP,sizeof(char) * STRING_LENGTH) != 
     sizeof(char) * STRING_LENGTH)
    {
      perror("write_msgs");
      return(ERROR);
    }

  if (write_int_to_fd(fd, length) != SUCCESS)
    return(ERROR);
  return(write(fd, msg, length) != length);
}

static int
write_knuckle_info(int fd, KNUCKLE *knuckle)
{
  int size;
  QUESTION q;

  if(write(fd,KNUCKLE_SEP,sizeof(char) * STRING_LENGTH) != 
     sizeof(char) * STRING_LENGTH)
    {
      perror("write_knuckle_info");
      return(ERROR);
    }

  size = write(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    return(ERROR);

  if(knuckle->question != (QUESTION *) NULL && 
     knuckle->question->owner == knuckle) 
    {
       if(write(fd,TRANS_SEP,sizeof(char) * STRING_LENGTH) != 
	  sizeof(char) * STRING_LENGTH)
	 {
	   perror("write_knuckle_info");
	   return(ERROR);
	 }
      size = write(fd, (char *) knuckle->question, sizeof(QUESTION));
      if(size != sizeof(QUESTION))
	{
	  perror("write_knuckle_info");
	  return(ERROR);
	}
    }
  else
    if(write(fd,BLANK_SEP,sizeof(char) * STRING_LENGTH) !=  
       sizeof(char) * STRING_LENGTH)
      {
	perror("write_knuckle_info");
	return(ERROR);
      }

  if (knuckle->new_messages != (char *) NULL)
    return(write_msgs(fd, knuckle->new_messages));
  else
    if(write(fd,BLANK_SEP,sizeof(char) * STRING_LENGTH) != 
       sizeof(char) * STRING_LENGTH)
      {
	perror("write_knuckle_info");
	return(ERROR);
      }

  return(SUCCESS);
}

static int
read_knuckle_info(int fd, KNUCKLE *knuckle)
{
  QUESTION q;
  int size;
  char type[STRING_LENGTH];

  size = read(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    {
      log_error("read_knuckle_info: cannot read knuckle");
      return(ERROR);
    }

  if(read(fd, (char *) type, sizeof(char) * STRING_LENGTH) != 
     sizeof(char) * STRING_LENGTH)
    {
      log_error("read_kncukle_info: cannot read type");
      perror("read_knuckle");
      type_error(fd,type);
      return(ERROR);
    }

  if(string_eq(type,TRANS_SEP))
    {
      size = read(fd,(char *) &q, sizeof(QUESTION));
      if(size != sizeof(QUESTION))
	{
	  log_error("read_knuckle_info: cannot read transaction");
	  return(ERROR);
	}	  
      knuckle->question = (QUESTION *) malloc(sizeof(QUESTION));
      if(knuckle->question == (QUESTION *) NULL)
	{
	  perror("question malloc");
	  return(ERROR);
	}
      bcopy((char *) &q, (char *) knuckle->question, sizeof(QUESTION));
      knuckle->question->owner = knuckle;
    }
  else
    knuckle->question = (QUESTION *) NULL;

  if(read(fd, (char *) type, sizeof(char) * STRING_LENGTH) != 
     sizeof(char) * STRING_LENGTH)
    {
      log_error("read_kncukle_info: cannot read second type");
      perror("read_knuckle");
      type_error(fd,type);
      return(ERROR);
    }

  if (string_eq(type,TEXT_SEP))
    {
      knuckle->new_messages = read_msgs(fd);
      return(knuckle->new_messages == (char *) NULL);
    }
  else
    knuckle->new_messages = (char *) NULL;

  return(SUCCESS);
}


static int
write_user_info(int fd, USER *user)
{
  int size;

  if(write(fd,USER_SEP,sizeof(char) * STRING_LENGTH) 
     != sizeof(char) * STRING_LENGTH)
    return(ERROR);

  size = write(fd, (char *) user, sizeof(USER));
  if (size != sizeof(USER))
    return(ERROR);
  
  return(SUCCESS);
}

static int
read_user_info(int fd, USER *user)
{
  int size;
  
  size = read(fd, (char *) user, sizeof(USER));

#ifdef TEST
  printf("size read in: %d/%d\n",size,sizeof(USER));
  printf("user: %s %s %s\n",user->username,user->realname,user->nickname);
  printf("uid: %d   \n",user->uid);
  printf("machine: %s  ns: %d   status: %d  \n",user->machine, user->no_specialties, user->status);
#endif TEST

  if (size != sizeof(USER))
    {
      perror("barf on user ");
      return(ERROR);
    }

  return(SUCCESS);
}


static jmp_buf trap;

oops(void)
{
  longjmp(trap, 1);
}

static int
verify_string(char *str)
{
  register int string_is_bogus;

  (void) signal(SIGSEGV, oops);
  (void) signal(SIGBUS,  oops);
  (void) signal(SIGILL,  oops);
  if (!setjmp(trap)) 
    {
      (void) strlen(str);
      string_is_bogus = SUCCESS;
    }
  else
    string_is_bogus = FAILURE;
  
  (void) signal(SIGSEGV, SIG_DFL);
  (void) signal(SIGBUS, SIG_DFL);
  (void) signal(SIGILL, SIG_DFL);

  return (string_is_bogus);
}

static void
ensure_consistent_state(void)
{
  register KNUCKLE **k_ptr, *k;
  KNUCKLE *foo;
  char msgbuf[BUFSIZ];
  int status;

  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      k = *k_ptr;
      if (k->connected != (KNUCKLE *) NULL) 
	{
	  /* check if connected person exists */
	    status = get_knuckle (k->connected->user->username,
				  k->connected->instance, &foo, /* ??? */0);
	    if (status != SUCCESS || k != k->connected->connected) {
		(void)sprintf(msgbuf,
			      "Inconsistency: user connected to %s (%d)\n",
			      k->user->username, k->instance);
		log_error(msgbuf);
/* problem: the question data is probably screwed due to earlier bugs.
 * don't try to salvage because addresses to question->owner is in
 * space. Take the loss on the question data, delete both knuckles,
 * and continue. This should be looked into.
 */
		k->question = (QUESTION *) NULL;
		delete_knuckle(k, /*???*/0);
		k->connected->question = (QUESTION *) NULL;
		k->connected->connected = (KNUCKLE *) NULL;
/* eat this too- chances are that if the connected person is invalid, 
 * it is total garbage.
 */

		/* delete_knuckle(k->connected); */
		continue;

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
						  "Daemon error -- please reconnect.",
						  NO_RESPOND);
			}
			k->question->owner->connected = (KNUCKLE *) NULL;
		    }
		}
#ifdef TEST
		if (!fork())
		    abort();
		else 
		{
		    printf("Inconsistencies found; creating core file");
		    wait(0);
		}
#endif	TEST  
	    }
	}
      if (k->new_messages != (char *) NULL) 
	{
	  if (verify_string(k->new_messages) != SUCCESS) 
	    {
	      k->new_messages = (char *) NULL;
	      (void) sprintf(msgbuf, "Inconsistency: user %s's messages\n",
			     k->user->username);
	      log_error(msgbuf);
	    }
	}
    }
}


reconnect_knuckles(void)
{
  KNUCKLE **k_ptr;
  KNUCKLE *k;
  int status; 
  
  for (k_ptr = Knuckle_List; (*k_ptr) != (KNUCKLE *) NULL; k_ptr++)
    if((*k_ptr)->connected != (KNUCKLE *) NULL)
      {
	status = get_knuckle((*k_ptr)->cusername,(*k_ptr)->cinstance, &k, /*???*/0);
	if(status == SUCCESS)
	  {
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
	    *((*k_ptr)->cusername) = '\0';
	    (*k_ptr)->connected = (KNUCKLE *) NULL;
	    if((*k_ptr)->question != (QUESTION *) NULL)
	      if((*k_ptr)->status  & QUESTION_STATUS)
		    (*k_ptr)->question->owner = (*k_ptr);		    
	  }
      }
}

int needs_backup = 0;

/*
 * Function:	backup_data() stores the state of the OLC system in a file.
 * Argments:	none.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 */

void
backup_data(void)
{
  KNUCKLE **k_ptr, **k_again;	           /* Current user. */
  int no_knuckles=0;
  int fd;			   /* Backup file descriptor. */
  int i;

  ensure_consistent_state();

  if ((fd = open(BACKUP_TEMP, O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0) 
    {
      perror("backup_data");
      log_error("backup_data: unable to open backup file.");
      goto PUNT;
    }
	
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if(((*k_ptr)->instance == 0) && (((*k_ptr)->user->no_knuckles > 1) ||
	 is_active((*k_ptr))))
	{
	  if (write_user_info(fd, (*k_ptr)->user) != SUCCESS) 
	    {
	      log_error("backup_data: unable to write_user");
	      goto PUNT;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (write_knuckle_info(fd, *(k_again+i)) != SUCCESS) 
	      {
		log_error("backup_data: unable to write_knuckle");
		goto PUNT;
	      }
	}
    }
  if(write(fd,DATA_SEP,sizeof(char) * STRING_LENGTH) != 
     sizeof(char) * STRING_LENGTH)
    {
      log_error("backup_data: unable to write data sep");
      goto PUNT;
    }
  needs_backup = 0;

  (void) rename(BACKUP_TEMP, BACKUP_FILE);

 PUNT:
  (void) close(fd); 
  log_status("Backup completed.");
}



/*
 * Function:	load_data() loads an OLC backup data file.  It is called
 *			when the daemon starts up.
 * Argments:	None.
 * Returns:	Nothing.
 * Notes:
 */

void load_data(void)
{
  int no_knuckles=0;
  int fd, i,j,nk;		
  int status;
  int successful = 0;
  KNUCKLE *kptr;
  USER *uptr = (USER *) NULL;
  char type[STRING_LENGTH];
  char buf[BUF_SIZE];

  i = j = 0;
  skip = FALSE;

  log_status("Loading....(wish me luck)\n");

  if ((fd = open(BACKUP_FILE, O_RDONLY, 0)) < 0) 
    {
      perror("load_data");
      log_error("load_data: unable to open backup file");
      return;
    }
  
  while (TRUE)
    {
      if(skip == FALSE)
	{
	  if((status = read(fd,type, sizeof(char) * STRING_LENGTH)) != 
	     sizeof(char) * STRING_LENGTH)
	    {
	      if(status == -1)
		break;
	      log_error("load_data: unable to read type");
	      perror("read_knuckle");
	      break;
	    }
	}
      else
	{
	  bcopy(type_buf,type,STRING_LENGTH);
	  skip = FALSE;
	}

      if(string_eq(type,USER_SEP))
	{
#ifdef TEST
	  log_status("load_data: reading user data\n");
#endif TEST

	  if(uptr != (USER *) NULL)
	    if(nk != uptr->no_knuckles)
	      {
		sprintf(buf,"No. knuck. mismatch: %d vs. %d",
			nk, uptr->no_knuckles);
		log_error(buf);
	      }

	  uptr = (USER *) malloc(sizeof(USER));
	  if(uptr == (USER *) NULL)
	    {
	      perror("load_data (failed malloc)");
	      goto PUNT;
	    }
	  status = read_user_info(fd,uptr);
	  if(status != SUCCESS)
	    goto PUNT;

	  nk = uptr->no_knuckles;
	  uptr->no_knuckles = 0;
	  uptr->knuckles = (KNUCKLE **) NULL;
	  continue;
	}

      if(string_eq(type,KNUCKLE_SEP))
	{
#ifdef TEST
	  log_status("load_data: reading user data\n");
#endif
	  kptr = (KNUCKLE *) malloc(sizeof(KNUCKLE));
	  status = read_knuckle_info(fd,kptr);
	  if(skip)
	    continue;
	  if(status != SUCCESS)
	    goto PUNT;
	  insert_knuckle(kptr);
	  kptr->user = uptr;
	  insert_knuckle_in_user(kptr,uptr);
	  kptr->user->no_knuckles++;
	  continue;
	}
      
      if(string_eq(type,TEXT_SEP))
	{
	  log_error("load_data: text found in data");
	  read_msgs(fd);
	  continue;
	}
      
      if(string_eq(type,BLANK_SEP))
	{
	  log_error("load_data: blank in data");
	  continue;
	}

      if(string_eq(type,DATA_SEP))
	break;

      type_error(fd,type); 
    }
  
  successful = 1;
  
  PUNT:
  if (!successful) 
    {
      log_status("Load failed, punting...\n");
      Knuckle_List = (KNUCKLE **) NULL;
      (void) unlink(BACKUP_FILE);
      needs_backup = 1;
    }
  else 
    {
      reconnect_knuckles();
      ensure_consistent_state();
      log_status("Loading complete.\n");
    }

  (void) close(fd);
  needs_backup = 0; 
}


/* code like this can only be written the day after an all nighter */

static void type_error(int fd, char *string)
{
  char buf[BUF_SIZE];
  char buf2[BUF_SIZE];
  int cc;
 
  bcopy(string,buf,STRING_LENGTH);
  skip = TRUE;

  while(TRUE)
    {
/*      write(1,buf,STRING_LENGTH);
      write(1,"\n",1);*/
      if(!strncmp(buf,USER_SEP,STRING_LENGTH))
	{
	  bcopy(buf,type_buf,STRING_LENGTH);
	  log_error("type_error: recovered\n");
	  return;
	}
      bcopy(buf,buf2,STRING_LENGTH);
      bcopy(&buf2[1],buf, STRING_LENGTH-1);
      if((cc = read(fd,buf2,sizeof(char))) != sizeof(char))
	{
	  bcopy(buf,type_buf,STRING_LENGTH);
	  if(cc == 0)
	    skip = FALSE;
	  log_error("type_error: brain damage\n");
	  return;
	}
      buf[STRING_LENGTH-1] = buf2[0];
      bcopy(buf,type_buf,STRING_LENGTH);
    }
}

void
dump_data(char *file)
{
  FILE *fp;
  KNUCKLE **k_ptr, **k_again;	           /* Current user. */
  int no_knuckles=0;
  int i;

  ensure_consistent_state();
  fp = fopen(file,"w");
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
     no_knuckles++;

  fprintf(fp,"\nNumber of knuckles:: %d\n",no_knuckles);
	
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if((*k_ptr)->instance == 0)
	{
	  fprintf(fp,"\n\nuser...");
	  fprintf(fp,"\nuser:     %s",(*k_ptr)->user->username);
	  fprintf(fp,"\nuid:      %d",(*k_ptr)->user->uid);
	  fprintf(fp,"\nrealname: %s",(*k_ptr)->user->realname);
	  fprintf(fp,"\ntitle:    %s",(*k_ptr)->title);
	  fprintf(fp,"\nmachine:  %s",(*k_ptr)->user->machine);
	  fprintf(fp,"\nstatus:   %d",(*k_ptr)->user->status);
	  fprintf(fp,"\nmax_ask:  %d",(*k_ptr)->user->max_ask);
	  fprintf(fp,"\nmax_ans:  %d",(*k_ptr)->user->max_answer);
	  fprintf(fp,"\n");
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    {
	      fprintf(fp,"\n\nknuckle...");
	      fprintf(fp,"\ninstance:     %d", (*(k_again+i))->instance);
	      fprintf(fp,"\npriority:     %d", (*(k_again+i))->priority);
	      fprintf(fp,"\nqueue:        %d", (*(k_again+i))->queue);
	      fprintf(fp,"\ntimestamp:    %d", (*(k_again+i))->timestamp);
	      fprintf(fp,"\nstatus:       %d", (*(k_again+i))->status);
	      fprintf(fp,"\ncusername:    %d", (*(k_again+i))->cusername);
	      fprintf(fp,"\ncinstance:    %d", (*(k_again+i))->cinstance);
	      if(is_connected((*(k_again+i))))
		 {
		   fprintf(fp,"\n\nconnected...");
		   fprintf(fp,"\nrcusername:   %d", (*(k_again+i))->connected->user->username);
		   fprintf(fp,"\nrcinstance:   %d", (*(k_again+i))->connected->instance);
		 }
	      if(has_question((*(k_again+i))))
		{
		  fprintf(fp,"\n\nquestion...");
		  fprintf(fp,"\nlogfile:    %s",(*(k_again+i))->question->logfile);
		  fprintf(fp,"\nlogtime:    %d",(*(k_again+i))->question->logfile_timestamp);
		  fprintf(fp,"\nnseen:      %d",(*(k_again+i))->question->nseen);
		  fprintf(fp,"\ntopic:      %s",(*(k_again+i))->question->topic);
		  fprintf(fp,"\ntcode:      %d",(*(k_again+i))->question->topic_code);
		  fprintf(fp,"\ntitle:      %s",(*(k_again+i))->question->title);
		  fprintf(fp,"\nowner:      %s",(*(k_again+i))->question->owner->user->username);
		  fprintf(fp,"\nownerin:    %d",(*(k_again+i))->question->owner->instance);
		}
	    }
	}
    }
  fclose(fp);
}

