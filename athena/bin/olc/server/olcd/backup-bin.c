/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for backing up the state of the OLC system, in the old binary
 * format.
 *
 *      Win Treese (MIT Project Athena)
 *      Ken Raeburn (MIT Information Systems)
 *      Tom Coppeto, Chris VanHaren, Lucien Van Elsen (MIT Project Athena)
 *
 * Copyright (C) 1988-1997 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: backup-bin.c,v 1.3 1999-06-29 21:30:10 ghudson Exp $
 */

#ifndef SABER
#ifndef lint
static char rcsid[] ="$Id: backup-bin.c,v 1.3 1999-06-29 21:30:10 ghudson Exp $";
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

#include <syslog.h>

/* prototypes for this file */

static int write_knuckle_info (int fd , KNUCKLE *knuckle );
static int read_knuckle_info (int fd , KNUCKLE *knuckle );
static int write_user_info (int fd , USER *user );
static int read_user_info (int fd , USER *user );
static void type_error (int fd , char *string );

/* 
 * Strings used to separate data chunks in the backup file.
 * Plain text is used so the file can be debugged easily.
 */

static char *DATA_SEP      = "  new data:    ";
static char *USER_SEP      = "  new user:    ";
static char *KNUCKLE_SEP   = "  new knuckle: ";
static char *BLANK_SEP     = "  a blank:     ";
static char *TRANS_SEP     = "  new train:   ";

static char type_buf[BUF_SIZE];
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
	  log_error("question malloc: %m");
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
  
#ifdef DEBUG_LOAD
  printf("size read in: %d/%d\n", size, sizeof(USER));
  printf("user: %s %s %s\n", user->username, user->realname, user->nickname);
  printf("uid: %d\n", user->uid);
  printf("machine: %s  #spec: %d   status: %d\n",
	 user->machine, user->no_specialties, user->status); 
#endif /* DEBUG_LOAD */
  
  if (size != sizeof(USER))
    {
      log_error("barf on user; size mismatch (wanted %d bytes, got %d)",
		sizeof(USER), size);
      return(ERROR);
    }
  
  /* restore database info from the database */
  load_user(user);

  return(SUCCESS);
}


int needs_backup = 0;

/*
 * Function:	binary_backup_data() stores the state of the OLC system in
 *		a byte-order-dependent binary-format file.
 * Argments:	none.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 */

void binary_backup_data(void)
{
  KNUCKLE **k_ptr, **k_again; /* Current user. */
  int fd;			   /* Backup file descriptor. */
  int i;
  char buf[BUF_SIZE];
  
  ensure_consistent_state();
  
  fd = open(BINARY_BACKUP_TEMP, O_CREAT | O_WRONLY | O_TRUNC, 0600);
  if (fd < 0)
    {
      log_error("binary_backup_data: unable to open backup file: %m");
      goto PUNT_BACKUP;
    }
  
  for (k_ptr = Knuckle_List; *k_ptr != (KNUCKLE *) NULL; k_ptr++) 
    {
      if(((*k_ptr)->instance == 0) && (((*k_ptr)->user->no_knuckles > 1) ||
				       is_active((*k_ptr))))
	{
	  if (write_user_info(fd, (*k_ptr)->user) != SUCCESS) 
	    {
	      log_error("binary_backup_data: unable to write_user");
	      goto PUNT_BACKUP;
	    }
	  k_again = (*k_ptr)->user->knuckles;
	  for(i=0; i< (*k_ptr)->user->no_knuckles; i++)
	    if (write_knuckle_info(fd, *(k_again+i)) != SUCCESS) 
	      {
		log_error("binary_backup_data: unable to write_knuckle");
		goto PUNT_BACKUP;
	      }
	}
    }
  if(write(fd,DATA_SEP,sizeof(char) * STRING_SIZE) != 
     sizeof(char) * STRING_SIZE)
    {
      log_error("binary_backup_data: unable to write data sep: %m");
      goto PUNT_BACKUP;
    }
  (void) rename(BINARY_BACKUP_TEMP, BINARY_BACKUP_FILE);

 PUNT_BACKUP:
  (void) close(fd); 
}



/*
 * Function:    binary_load_data() loads an OLC binary backup data file.
 *		It is called when the daemon starts up if no ASCII backup
 *		is available.
 * Argments:	None.
 * Returns:	Nothing.
 * Notes:
 */

void binary_load_data(void)
{
  int fd, nk = -1;
  int status;
  int successful = 0;
  KNUCKLE *kptr;
  USER *uptr = (USER *) NULL;
  char type[STRING_SIZE];
  char buf[BUF_SIZE];
  
  skip = FALSE;
  
  log_status("Loading binary data...\n");
  
  fd = open(BINARY_BACKUP_FILE, O_RDONLY, 0);
  if (fd < 0) 
    {
      log_error("binary_load_data: unable to open backup file: %m");
      return;
    }
  
  while (TRUE)
    {
      if(skip == FALSE)
	{
	  status = read(fd,type, STRING_SIZE);
	  if(status != STRING_SIZE)
	    {
	      if(status == -1)
		break;
	      log_error("binary_load_data: unable to read type: %m");
	      break;
	    }
	}
      else
	{
	  memcpy(type, type_buf, STRING_SIZE);
	  skip = FALSE;
	}
      
      if(string_eq(type,USER_SEP))
	{
	  if (uptr && nk != -1 && nk != uptr->no_knuckles) {
	    log_error("No. knuck. mismatch: %d vs. %d",
		      nk, uptr->no_knuckles);
	  }
	  
	  uptr = (USER *) malloc(sizeof(USER));
	  if(uptr == (USER *) NULL)
	    {
	      log_error("binary_load_data: malloc failed: %m");
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
      log_status("Load failed, punting...");
      Knuckle_List = (KNUCKLE **) NULL;
      (void) unlink(BINARY_BACKUP_FILE);
      needs_backup = 1;
    }
  else 
    {
      reconnect_knuckles_binary();
      ensure_consistent_state();
      log_status("Loading complete.");
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
  
  memcpy(my_buf, string, STRING_SIZE);
  skip = TRUE;
  
  while(TRUE)
    {
      /*      write(1,my_buf,STRING_SIZE);
	      write(1,"\n",1);*/
      if(!strncmp(my_buf,USER_SEP,STRING_SIZE))
	{
	  memcpy(type_buf, my_buf, STRING_SIZE);
	  log_error("type_error: recovered");
	  return;
	}
      memcpy(buf2, my_buf, STRING_SIZE);
      memcpy(my_buf, &buf2[1], STRING_SIZE-1);
      cc = read(fd,buf2,1);
      if(cc != 1)
	{
	  memcpy(type_buf, my_buf, STRING_SIZE);
	  if(cc == 0)
	    skip = FALSE;
	  log_error("type_error: brain damage: %m");
	  return;
	}
      my_buf[STRING_SIZE-1] = buf2[0];
      memcpy(type_buf, my_buf, STRING_SIZE);
    }
}
