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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v $
 *	$Id: db.c,v 1.9 1990-05-26 11:04:24 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v 1.9 1990-05-26 11:04:24 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <olcd.h>
#include <ctype.h>		/* Standard type definitions. */
#ifdef NDBM
#include <ndbm.h>
#endif /* NDBM */

#ifdef NDBM
static DBM *dbp = (DBM *) NULL;
#endif /* NDBM */

extern ACL  Acl_List[];
extern int NotWhiteSpace();

#ifdef __STDC__
static int get_user_info (USER *);
#else
static int get_user_info ();
#endif /* STDC */

/*
 * Function:	get_specialties() searches the OLC database to find a
 *			user's list of specialties.
 * Arguments:	user:	Ptr. to desired user
 * Returns:	nothing
 * Notes:
 */


#ifdef __STDC__
get_specialties(USER *user)
#else
get_specialties(user)
     USER *user;
#endif /* STDC */
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


void
#ifdef __STDC__
get_acls(USER *user)
#else
get_acls(user)
     USER *user;
#endif /* STDC */
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


int
#ifdef __STDC__
load_db(void)
#else
load_db()
#endif /* STDC */
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
      db = get_next_word(db, t->name, NotWhiteSpace);
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

void
#ifdef __STDC__
load_user(USER *user)
#else
load_user(user)
     USER *user;
#endif /* STDC */
{
  get_specialties(user);
  get_acls(user);
  get_user_info(user);
}

#ifdef NDBM

static int
#ifdef __STDC__
get_user_info(USER *user)
#else
get_user_info(user)
     USER *user;
#endif /* STDC */
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
      ptr = get_next_word(ptr, user->title1, NotWhiteSpace);
      ptr = get_next_word(ptr, buf, NotWhiteSpace);
      user->max_ask = atoi(buf);
      ptr = get_next_word(ptr, user->title2, NotWhiteSpace);
      ptr = get_next_word(ptr, buf, NotWhiteSpace);
      user->max_answer = atoi(buf);
    }
  
  (void) dbm_close(dbp);
  dbp = (DBM *) NULL;
  return(SUCCESS);
}

int
#ifdef __STDC__
save_user_info(USER *user)
#else
save_user_info(user)
#endif /* STDC */
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

static int
#ifdef __STDC__
get_user_info(USER *user)
#else
get_user_info(user)
     USER *user;
#endif /* STDC */
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
	  db = get_next_word(db, buf, NotWhiteSpace);
	  if(string_eq(canon,buf))
	    {
	      db = get_next_word(db, user->title1, NotWhiteSpace);
	      db = get_next_word(db, buf, NotWhiteSpace);
	      user->max_ask = atoi(buf);
	      db = get_next_word(db, user->title2, NotWhiteSpace);
	      db = get_next_word(db, buf, NotWhiteSpace);
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


int
#ifdef __STDC__
save_user_info(USER *user)
#else
save_user_info(user)
     USER *user;
#endif /* STDC */
{
  return(ERROR);
}

#endif /* NDBM */
