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
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/backup.c,v 1.1 1989-07-16 17:14:45 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olcd.h>

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */
#include <sys/file.h>           /* System file defs. */
#include <setjmp.h>             /* For string validation kludge */


static char *
read_msgs(fd)
     int fd;			/* file descriptor */
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
write_msgs(fd, msg)
     int fd;
     char *msg;
{
  int length;
  
  length = strlen(msg);
  if (write_int_to_fd(fd, length) != SUCCESS)
    return(ERROR);
  return(write(fd, msg, length) != length);
}

static int
write_knuckle_info(fd, knuckle)
     int fd;
     KNUCKLE *knuckle;
{
  int size;
  QUESTION q;

  q.owner = (KNUCKLE *) 0;
  size = write(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    return(ERROR);

  if(knuckle->question != (QUESTION *) NULL && 
     knuckle->question->owner == knuckle)
    {
      size = write(fd, (char *) knuckle->question, sizeof(QUESTION));
      if(size != sizeof(QUESTION))
	return(ERROR);
    }
  else
    size = write(fd, (char *) &q, sizeof(QUESTION));

  if(size != sizeof(QUESTION))
    return(ERROR);

  if (knuckle->new_messages != (char *) NULL)
    return(write_msgs(fd, knuckle->new_messages));
  
  return(SUCCESS);
}

static int
read_knuckle_info(fd, knuckle)
     int fd;
     KNUCKLE *knuckle;
{
  int size;
  QUESTION q;

  size = read(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    {
      log_error("cannot read knuckle");
      return(ERROR);
    }

  size = read(fd, (char *) &q, sizeof(QUESTION));
  if(size != sizeof(QUESTION))
    return(ERROR);

  if(q.owner != (KNUCKLE *) 0)
    {
      knuckle->question = (QUESTION *) malloc(sizeof(QUESTION));
      if(knuckle->question == (QUESTION *) NULL)
	{
	  perror("question malloc");
	  return(ERROR);
	}
      bcopy((char *) &q, (char *) knuckle->question, sizeof(QUESTION));
      knuckle->question->owner = knuckle;
    }

  if (knuckle->new_messages != (char *) NULL) 
    {
      knuckle->new_messages = read_msgs(fd);
      return(knuckle->new_messages == (char *) NULL);
    }

  return(SUCCESS);
}


static int
write_user_info(fd, user)
     int fd;
     USER *user;
{
  int size;

  size = write(fd, (char *) user, sizeof(USER));
  if (size != sizeof(USER))
    return(ERROR);
  
  return(SUCCESS);
}

static int
read_user_info(fd, user)
     int fd;
     USER *user;
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

oops()
{
  longjmp(trap, 1);
}

static int
verify_string(str)
     char *str;
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
ensure_consistent_state()
{
  register KNUCKLE **k_ptr, *k;
  KNUCKLE *foo;
  char msgbuf[BUFSIZ];
      
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      k = *k_ptr;
      if (k->connected != (KNUCKLE *) NULL) 
	{
	  /* check if connected person exists */
		  
	  if (get_knuckle(k->connected->user->username, 
			  k->connected->instance, &foo) != SUCCESS ||
	      k != k->connected->connected)
	    {
	     (void)sprintf(msgbuf,"Inconsistency: user connected to %s (%d)\n",
			     k->user->username, k->instance);
	      log_error(msgbuf);
	      k->question->owner->connected->connected = (KNUCKLE *) NULL;
	      k->question->owner->connected = (KNUCKLE *) NULL;

	      write_message_to_user(k->question->owner->connected,
				    "Daemon error -- please reconnect.",
				    NULL);
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

reconnect_knuckles()
{
  KNUCKLE **k_ptr;
  KNUCKLE *k;
  int status; 
  
  for (k_ptr = Knuckle_List; (*k_ptr) != (KNUCKLE *) NULL; k_ptr++)
    if((*k_ptr)->connected != (KNUCKLE *) NULL)
      {
	status = get_knuckle((*k_ptr)->cusername,(*k_ptr)->cinstance, &k);
	if(status == SUCCESS)
	  {
	    (*k_ptr)->connected = k;
	    if((*k_ptr)->question != (QUESTION *) NULL)
	      (*k_ptr)->connected->question = (*k_ptr)->question;
	  }
	else
	  {
	    *((*k_ptr)->cusername) = '\0';
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
backup_data()
{
  KNUCKLE **k_ptr, **k_again;	           /* Current user. */
  int no_knuckles=0;
  int fd;			   /* Backup file descriptor. */
  int i;

  ensure_consistent_state();
  return;

  if ((fd = open(BACKUP_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0) 
    {
      perror("backup_data");
      log_error("backup_data: Unable to open backup file.");
      goto PUNT;
    }
	
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++)
     no_knuckles++;

/*  if (write(fd, (char *) & no_knuckles, sizeof (int)) != sizeof (int)) 
    {
      perror("backup_data");
      log_error("backup_data: Unable to write knuckle length.");
      goto PUNT;
    }
	
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if((*k_ptr)->instance == 0)
	{
	  if (write_user_info(fd, (*k_ptr)->user) != SUCCESS) 
	    {
	      perror("backup_data: write_user");
	      goto PUNT;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (write_knuckle_info(fd, *(k_again+i)) != SUCCESS) 
	      {
		perror("backup_data: write_knuckle");
		goto PUNT;
	      }
	}
    }
  needs_backup = 0;
  */
 PUNT:
  (void) close(fd); 
  log_status("Backup completed.");
}

/*
 * Function:	load_data() loads an OLC backup data file.  It is called
 *			when the daemon starts up.
 * Argments:	None.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 */

load_data()
{
  int no_knuckles=0;
  int fd, i,j,nk;		
  int status;
  int successful = 0;
  KNUCKLE *kptr;
  USER *uptr;

  i = j = 0;

  log_status("Loading....\n");
  return(SUCCESS);

  if ((fd = open(BACKUP_FILE, O_RDONLY, 0)) < 0) 
    {
      perror("load_data");
      log_error("load_data: Unable to open backup file.");
      return;
    }
  
/*
  if (read(fd, (char *) &no_knuckles, sizeof (int)) != sizeof (int)) 
    {
      perror("load_data");
      log_error("load_data: Unable to read status information.");
      goto PUNT;
    }
	
#ifdef TEST
  printf("number of knuckles: %d\n",no_knuckles);
#endif TEST

  i=0;
  while (i < no_knuckles)
    {

#ifdef TEST
      printf("reading user: %d\n",i);
#endif TEST

      uptr = (USER *) malloc(sizeof(USER));
      status = read_user_info(fd,uptr);
      if(status != SUCCESS)
	goto PUNT;
      nk = uptr->no_knuckles;
      uptr->no_knuckles = 0;
      uptr->knuckles = (KNUCKLE **) NULL;
      for(j=0; j < nk; j++)
	{
	  ++i;
	  kptr = (KNUCKLE *) malloc(sizeof(KNUCKLE));
	  status = read_knuckle_info(fd,kptr);
	  if(status != SUCCESS)
	    goto PUNT;
	  insert_knuckle(kptr);
	  kptr->user = uptr;
	  insert_knuckle_in_user(kptr,uptr);
	  kptr->user->no_knuckles++;
	}
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
  needs_backup = 0; */
}




void
dump_data(file)
     char *file;
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

