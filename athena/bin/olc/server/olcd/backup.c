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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/backup.c,v $
 *	$Id: backup.c,v 1.25 1993-08-12 17:27:48 cfields Exp $
 *	$Author: cfields $
 */

#ifndef SABER
#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/backup.c,v 1.25 1993-08-12 17:27:48 cfields Exp $";
#endif
#endif

#include <mit-copyright.h>


#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */
#include <sys/file.h>           /* System file defs. */
#include <setjmp.h>             /* For string validation kludge */
#include <pwd.h>
#include <olcd.h>
#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int write_knuckle_info P((int fd , KNUCKLE *knuckle ));
static int read_knuckle_info P((int fd , KNUCKLE *knuckle ));
static int write_user_info P((int fd , USER *user ));
static int read_user_info P((int fd , USER *user ));

static void ensure_consistent_state P((void ));
static void type_error P((int fd , char *string ));

static int ascii_write_knuckle_info P((FILE *fd , KNUCKLE *knuckle ));
static int ascii_write_user_info P((FILE *fd , USER *user ));
#ifdef notdone
static int ascii_read_knuckle_info P((int fd , KNUCKLE *knuckle ));
static int ascii_read_user_info P((int fd , USER *user ));
#endif

#undef P

/* 
 * Strings used to separate data chunks in the backup file.
 * Plain text is used so the file can be debugged easily.
 */

static char *DATA_SEP      = "  new data:    ";
static char *USER_SEP      = "  new user:    ";
static char *KNUCKLE_SEP   = "  new knuckle: ";
static char *BLANK_SEP     = "  a blank:     ";
static char *TRANS_SEP     = "  new train:   ";

static type_buf[BUF_SIZE];
static int skip;

static int
  write_knuckle_info(fd, knuckle)
int fd;
KNUCKLE *knuckle;
{
  int size;
  
  if(write(fd,KNUCKLE_SEP,sizeof(char) * STRING_SIZE) != 
     sizeof(char) * STRING_SIZE)
    {
      log_error("write_knuckle_info: %m");
      return(ERROR);
    }
  
  size = write(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    return(ERROR);
  
  if(knuckle->question != (QUESTION *) NULL && 
     knuckle->question->owner == knuckle) 
    {
      if(write(fd,TRANS_SEP,sizeof(char) * STRING_SIZE) != 
	 sizeof(char) * STRING_SIZE)
	{
	  log_error("write_knuckle_info: %m");
	  return(ERROR);
	}
      size = write(fd, (char *) knuckle->question, sizeof(QUESTION));
      if(size != sizeof(QUESTION))
	{
	  log_error("write_knuckle_info: %m");
	  return(ERROR);
	}
    }
  else
    if(write(fd,BLANK_SEP,sizeof(char) * STRING_SIZE) !=  
       sizeof(char) * STRING_SIZE)
      {
	log_error("write_knuckle_info: %m");
	return(ERROR);
      }
  
  return(SUCCESS);
}

static int
  read_knuckle_info(fd, knuckle)
int fd;
KNUCKLE *knuckle;
{
  int size;
  char type[STRING_SIZE];
  
  size = read(fd, (char *) knuckle, sizeof(KNUCKLE));
  if (size != sizeof(KNUCKLE))
    {
      log_error("read_knuckle_info: cannot read knuckle: %m");
      return(ERROR);
    }
  
  if(read(fd, (char *) type, sizeof(char) * STRING_SIZE) != 
     sizeof(char) * STRING_SIZE)
    {
      log_error("read_kncukle_info: cannot read type: %m");
      type_error(fd,type);
      return(ERROR);
    }
  
  if(string_eq(type,TRANS_SEP))
    {
      knuckle->question = (QUESTION *) malloc(sizeof(QUESTION));
      if(knuckle->question == (QUESTION *) NULL)
	{
	  log_error("question malloc");
	  return(ERROR);
	}
      size = read(fd,(char *) knuckle->question, sizeof(QUESTION));
      if(size != sizeof(QUESTION))
	{
	  log_error("read_knuckle_info: cannot read transaction: %m");
	  return(ERROR);
	}	  
      knuckle->question->owner = knuckle;
      knuckle->question->topic_code =
	get_topic_code(knuckle->question->topic);
    }

  else
    knuckle->question = (QUESTION *) NULL;
  return(SUCCESS);
}


static int
  write_user_info(fd, user)
int fd;
USER *user;
{
  int size;
  
  if(write(fd,USER_SEP,sizeof(char) * STRING_SIZE) 
     != sizeof(char) * STRING_SIZE)
    return(ERROR);
  
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
  struct passwd *pwd;
  
  size = read(fd, (char *) user, sizeof(USER));
  
#ifdef HESIOD
  pwd = hes_getpwnam(user->username);
#else
  pwd = getpwnam(user->username);
#endif
  if (pwd != (struct passwd *) NULL) {
    strncpy(user->realname,pwd->pw_name,NAME_SIZE);
  }

#ifdef TEST
  printf("size read in: %d/%d\n",size,sizeof(USER));
  printf("user: %s %s %s\n",user->username,user->realname,user->nickname);
  printf("uid: %d   \n",user->uid);
  printf("machine: %s  ns: %d   status: %d  \n",user->machine,
	 user->no_specialties, user->status); 
#endif /* TEST */
  
  if (size != sizeof(USER))
    {
      log_error("barf on user; size mismatch ");
      return(ERROR);
    }
  
  return(SUCCESS);
}


static void
  ensure_consistent_state()
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
	    /* eat this too- chances are that if the connected person is */
	    /* invalid, it is total garbage. */
	    
	    /* delete_knuckle(k->connected); */
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

void
  reconnect_knuckles()
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
  backup_data()
{
  KNUCKLE **k_ptr, **k_again; /* Current user. */
  int fd;			   /* Backup file descriptor. */
  int i;
  char buf[BUF_SIZE];
  
  ensure_consistent_state();
  
  if ((fd = open(BACKUP_TEMP, O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0) 
    {
      log_error("backup_data: unable to open backup file: %m");
      goto PUNT_BACKUP;
    }
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if(((*k_ptr)->instance == 0) && (((*k_ptr)->user->no_knuckles > 1) ||
				       is_active((*k_ptr))))
	{
	  if (write_user_info(fd, (*k_ptr)->user) != SUCCESS) 
	    {
	      log_error("backup_data: unable to write_user");
	      goto PUNT_BACKUP;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (write_knuckle_info(fd, *(k_again+i)) != SUCCESS) 
	      {
		log_error("backup_data: unable to write_knuckle");
		goto PUNT_BACKUP;
	      }
	}
    }
  if(write(fd,DATA_SEP,sizeof(char) * STRING_SIZE) != 
     sizeof(char) * STRING_SIZE)
    {
      log_error("backup_data: unable to write data sep: %m");
      goto PUNT_BACKUP;
    }
  needs_backup = 0;
  
  (void) rename(BACKUP_TEMP, BACKUP_FILE);
  (void) sprintf(buf, "%s.ascii", BACKUP_TEMP);
  dump_data(buf);
  
 PUNT_BACKUP:
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

void
  load_data()
{
  int fd, nk = -1;
  int status;
  int successful = 0;
  KNUCKLE *kptr;
  USER *uptr = (USER *) NULL;
  char type[STRING_SIZE];
  char buf[BUF_SIZE];
  
  skip = FALSE;
  
  log_status("Loading....\n");
  
  if ((fd = open(BACKUP_FILE, O_RDONLY, 0)) < 0) 
    {
      log_error("load_data: unable to open backup file: %m");
      return;
    }
  
  while (TRUE)
    {
      if(skip == FALSE)
	{
	  if((status = read(fd,type, sizeof(char) * STRING_SIZE)) != 
	     sizeof(char) * STRING_SIZE)
	    {
	      if(status == -1)
		break;
	      log_error("load_data: unable to read type: %m");
	      break;
	    }
	}
      else
	{
	  bcopy(type_buf,type,STRING_SIZE);
	  skip = FALSE;
	}
      
      if(string_eq(type,USER_SEP))
	{
#ifdef TEST
	  log_status("load_data: reading user data\n");
#endif /* TEST */
	  
	  if (uptr && nk != -1 && nk != uptr->no_knuckles) {
	    sprintf(buf,"No. knuck. mismatch: %d vs. %d",
		    nk, uptr->no_knuckles);
	    log_error(buf);
	  }
	  
	  uptr = (USER *) malloc(sizeof(USER));
	  if(uptr == (USER *) NULL)
	    {
	      log_error("load_data (failed malloc)");
	      goto PUNT_LOAD;
	    }
	  status = read_user_info(fd,uptr);
	  if(status != SUCCESS)
	    goto PUNT_LOAD;
	  
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
	    goto PUNT_LOAD;
	  status = insert_knuckle(kptr);
	  if (status != SUCCESS)
	    goto PUNT_LOAD; 
	  kptr->user = uptr;
	  if (kptr->question == NULL)
	    kptr->title = uptr->title2;
	  else
	    kptr->title = uptr->title1;
	  insert_knuckle_in_user(kptr,uptr);
	  sprintf(kptr->nm_file,"%s/%s_%d.nm", LOG_DIR, kptr->user->username,
		  kptr->instance);

	  kptr->user->no_knuckles++;
	  continue;
	}
      
      if(string_eq(type,DATA_SEP))
	break;
      
      type_error(fd,type); 
    }
  
  successful = 1;
  
 PUNT_LOAD:
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

static void
  type_error(fd,string)
int fd;
char *string;
{
  char my_buf[BUF_SIZE];
  char buf2[BUF_SIZE];
  int cc;
  
  bcopy(string,my_buf,STRING_SIZE);
  skip = TRUE;
  
  while(TRUE)
    {
      /*      write(1,my_buf,STRING_SIZE);
	      write(1,"\n",1);*/
      if(!strncmp(my_buf,USER_SEP,STRING_SIZE))
	{
	  bcopy(my_buf,type_buf,STRING_SIZE);
	  log_error("type_error: recovered\n");
	  return;
	}
      bcopy(my_buf,buf2,STRING_SIZE);
      bcopy(&buf2[1],my_buf, STRING_SIZE-1);
      if((cc = read(fd,buf2,sizeof(char))) != sizeof(char))
	{
	  bcopy(my_buf,type_buf,STRING_SIZE);
	  if(cc == 0)
	    skip = FALSE;
	  log_error("type_error: brain damage");
	  return;
	}
      my_buf[STRING_SIZE-1] = buf2[0];
      bcopy(my_buf,type_buf,STRING_SIZE);
    }
}

void
dump_data(file)
     char *file;
{
  FILE *fp;
  int no_knuckles;
  KNUCKLE **k_ptr, **k_again;	/* Current user. */
  int i=0;
  extern int request_count;
  extern int request_counts[OLC_NUM_REQUESTS];
  extern PROC *proc_list;
  
  ensure_consistent_state();
  fp = fopen(file,"w");

  if (fp == NULL)
    {
      log_error("dump_data: unable to open ascii dump file: %m");
      return;
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
	      goto PUNT_DUMP;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (ascii_write_knuckle_info(fp, *(k_again+i)) != SUCCESS) 
	      {
		log_error("dump_data: unable to write_knuckle");
		goto PUNT_DUMP;
	      }
	}
    }

 PUNT_DUMP:
  fclose(fp);
}

static int
ascii_write_user_info(fp, user)
     FILE *fp;
     USER *user;
{
  fprintf(fp,"\n\nuser...");
  fprintf(fp,"\nuid:      %d", user->uid);
  fprintf(fp,"\nusername: %s", user->username);
  fprintf(fp,"\nrealname: %s", user->realname);
  fprintf(fp,"\nnickname: %s", user->nickname);
  fprintf(fp,"\ntitle1:   %s", user->title1);
  fprintf(fp,"\ntitle2:   %s", user->title2);
  fprintf(fp,"\nmachine:  %s", user->machine);
  fprintf(fp,"\nrealm:    %s", user->realm);
  fprintf(fp,"\nspeclts:  %d", user->specialties);
  fprintf(fp,"\nnum_spec: %d", user->no_specialties);
  fprintf(fp,"\npermssns: %d", user->permissions);
  fprintf(fp,"\nstatus:   %d", user->status);
  fprintf(fp,"\nnumknuck: %d", user->no_knuckles);
  fprintf(fp,"\nmax_ask:  %d", user->max_ask);
  fprintf(fp,"\nmax_ans:  %d", user->max_answer);
  fprintf(fp,"\n");

  return(SUCCESS);
}




static int
ascii_write_knuckle_info(fp, knuckle)
     FILE *fp;
     KNUCKLE *knuckle;
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

