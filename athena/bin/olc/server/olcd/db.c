/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains routines for manipulating the OLC database.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v $
 *      $Author: raeburn $
 */

#ifndef lint
static char rcsid[] =
    "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v 1.6 1990-01-05 06:22:44 raeburn Exp $";
#endif


#include <stdio.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <olcd.h>
#include <ctype.h>		/* Standard type definitions. */
#ifdef NDBM
#include <ndbm.h>
#endif NDBM

#ifdef NDBM
static DBM *dbp = (DBM *) NULL;
#endif

extern ACL  Acl_List[];
static int get_user_info (USER *);

/*
 * Function:	get_specialties() searches the OLC database to find a
 *			user's list of specialties.
 * Arguments:	user:	Ptr. to desired user
 * Returns:	nothing
 * Notes:
 */


get_specialties(USER *user)
{
  TOPIC **t_ptr;
  int *s = user->specialties;
  int i =0;
  char buf[BUFSIZ];

  sprintf(buf,"%s@%s",user->username,user->realm);
  for(t_ptr = Topic_List; *t_ptr != (TOPIC *) NULL; t_ptr++)
    {
      if(acl_check((*t_ptr)->acl,buf))
	{
	  *(s+i) = (*t_ptr)->value;	
	  ++i;
	}

      if(i==SPEC_SIZE)
	break;
    }
  *(s+i) = UNKNOWN_TOPIC;
  user->no_specialties = i;
}


get_acls(USER *user)
{
  ACL *a_ptr;
  char buf[BUFSIZ];

  sprintf(buf,"%s@%s",user->username,user->realm);

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(acl_check(a_ptr->file,buf))
	user->permissions |= a_ptr->code;
    }
}


int load_db(void)
{
  FILE *fp;
  TOPIC *t;
  ACL *a;
  char *db;
  char db_line[DB_LINE];
  char msgbuf[BUFSIZ];
  char buf[BUFSIZ];
  int i=0;

  if((fp = fopen(TOPIC_FILE,"r")) == (FILE *) NULL)
    {
      (void) sprintf(msgbuf, "load_db: can't open OLC database %s", 
		     TOPIC_FILE);
      log_error(msgbuf);
      perror("load_db");
      return(ERROR);
    }

  while (fgets(db_line, DB_LINE, fp) != (char *)NULL)
    {
      db = db_line;
      if(*db == '#')       /* comment */
	continue;
      t = (TOPIC *) malloc(sizeof(TOPIC));
      if(t == (TOPIC *) NULL)
	{
	  perror("load_db");
	  fclose(fp);
	  return(ERROR);
	}
      db = get_next_word(db,t->name);
      sprintf(t->acl,"%s/%s.acl",SPECIALTY_DIR,t->name);
      t->value = i;
      insert_topic(t);

#ifdef TEST
      log_status (fmt ("load_db: %s %d %s\n",t->name,i, t->acl));
#endif TEST
      ++i;
    } 

  
  for(a = Acl_List; a->file != (char *) NULL; a++)
    {
      sprintf(buf,"%s/%s",ACL_DIR,a->file);
      a->file = malloc((strlen(buf)+1) * sizeof(char));
      strcpy(a->file,buf);
    }

  fclose(fp);
  return(SUCCESS);
}

void load_user(USER *user)
{
  get_specialties(user);
  get_acls(user);
  get_user_info(user);
}

#ifdef NDBM
static int get_user_info(user)
     USER *user;
{
  datum key,d;
  char canon[BUF_SIZE];
  char buf[BUF_SIZE];
  char *ptr;

  get_specialties(user);
  get_acls(user);
  sprintf(canon,"%s@%s",user->username,user->realm);

  if(dbp == (DBM *) NULL)
    {
      dbp = dbm_open(DATABASE_FILE, O_RDWR | O_CREAT, 0600);
      if(dbp == (DBM *) NULL)
	{
	  perror("load_user (dbm_open)");
	  return(ERROR);
	}
    }
  
  key.dptr = &canon[0];
  key.dsize = strlen(canon);

  d = dbm_fetch(dbp,key);
  if(d.dptr != (char *) NULL)
    {
      ptr = d.dptr;
      ptr = get_next_word(ptr, user->title1);
      ptr = get_next_word(ptr, buf);
      user->max_ask = atoi(buf);
      ptr = get_next_word(ptr, user->title2);
      ptr = get_next_word(ptr, buf);
      user->max_answer = atoi(buf);
    }
  
  (void) dbm_close(dbp);
  dbp = (DBM *) NULL;
  return(SUCCESS);
}

int save_user_info(user)
     USER *user;
{
  char buf[BUF_SIZE];
  char canon[BUF_SIZE];
  datum key, d;

  if(dbp == (DBM *) NULL)
    {
      dbp = dbm_open(DATABASE_FILE, O_RDWR | O_CREAT, 0600);
      if(dbp == (DBM *) NULL)
	{
	  perror("load_user (dbm_open)");
	  return(ERROR);
	}
    }

  sprintf(buf,"%s %d %s %d",user->title1,
	  user->max_ask,user->title2,
	  user->max_answer); 

  sprintf(canon,"%s@%s",user->username,user->realm);

  d.dptr = &buf[0];
  d.dsize = strlen(buf)+1;
  key.dptr = &canon[0];
  key.dsize = strlen(canon)+1;
  
  if(dbm_store(dbp,key,d,DBM_REPLACE))
    {
      perror("save_user_info (dbm_store)");
      (void) dbm_close(dbp);
      dbp = (DBM *) NULL;
      return(ERROR);
    }
  else
    {
      (void) dbm_close(dbp);
      dbp = (DBM *) NULL;
      return(SUCCESS);
    }
}

delete_user_info(user)
     USER *user;
{
  char buf[BUF_SIZE];
  char canon[BUF_SIZE];
  datum key;

  sprintf(canon,"%s@%s",user->username,user->realm);
  key.dptr = &canon[0];
  key.dsize = strlen(canon)+1;

  if(dbm_delete(dbp,key))
    {
      perror("save_user_info (dbm_store)");
      return(ERROR);
    }
  else
    return(SUCCESS);
}

#else /* ! NDBM */

get_user_info(USER *user)
{
  FILE *fp;
  char db_line[DB_LINE];  
  char msgbuf[BUFSIZ];
  char *db, buf[BUFSIZ];            
  char canon[BUFSIZ];

  sprintf(canon,"%s@%s",user->username,user->realm);

  if((fp = fopen(DATABASE_FILE,"r")) == (FILE *) NULL)
    {
      (void) sprintf(msgbuf, "load_user: can't open OLC database %s", 
		     DATABASE_FILE);
      log_error(msgbuf);
      perror("load_user");
      return(ERROR);
    }

  while (fgets(db_line, DB_LINE, fp) != (char *) NULL)
    {
      db = db_line;
      if(*db_line == '#')       /* comment */
	continue;
      else
	{
	  db = get_next_word(db,buf);	  
	  if(string_eq(canon,buf))
	    {
	      db = get_next_word(db, user->title1);
	      db = get_next_word(db, buf);
	      user->max_ask = atoi(buf);
	      db = get_next_word(db, user->title2);
	      db = get_next_word(db, buf);
	      user->max_answer = atoi(buf);
#ifdef TEST
	      printf("%s %s %d %s %d\n",canon, user->title1, user->max_ask,
		user->title2,user->max_answer);
#endif TEST
	      fclose(fp);
	      return(SUCCESS);
	    }
	  
	}
    }
  fclose(fp);
  return(ERROR);
}


save_user_info(USER *user)
{
  return(ERROR);
}

#endif
