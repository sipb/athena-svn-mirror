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
 *	$Id: db.c,v 1.23 1999-03-06 16:48:54 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: db.c,v 1.23 1999-03-06 16:48:54 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <ctype.h>		/* Standard type definitions. */
#include <pwd.h>
#include <olcd.h>

extern ACL  Acl_List[];

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int get_user_info P((USER *user ));

#undef P


/*
 * Function:	get_specialties() searches the OLC database to find a
 *			user's list of specialties.
 * Arguments:	user:	Ptr. to desired user
 * Returns:	nothing
 * Notes:
 */


get_specialties(user)
     USER *user;
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
get_acls(user)
     USER *user;
{
  ACL *a_ptr;
  char buf[BUFSIZ];

  if ((user->realm != NULL) && (user->realm[0] != '\0'))
    sprintf(buf,"%s@%s",user->username,user->realm);
  else
    strcpy(buf,user->username);

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(acl_check(a_ptr->file,buf))
	user->permissions |= a_ptr->code;
    }
}


int
load_db()
{
  FILE *fp;
  TOPIC *t;
  ACL *a;
  char *db;
  char db_line[DB_LINE];
  char msgbuf[BUFSIZ];
  char buf[BUFSIZ];
  int i=0;

  fp = fopen(TOPIC_FILE,"r");
  if (fp == NULL)
    {
      log_error("load_db: can't open OLC database %s: %m", 
		TOPIC_FILE);
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
	  log_error("load_db: topic malloc: %m");
	  fclose(fp);
	  return(ERROR);
	}
      db = get_next_word(db, t->name, NotWhiteSpace);
      sprintf(t->acl,"%s/%s.acl",SPECIALTY_DIR,t->name);
      t->value = i;
      if (insert_topic(t) != SUCCESS) {
	log_error("load_db: insert_topic failed");
	fclose(fp);
	return(ERROR);
      }

      ++i;
    } 

  
  for(a = Acl_List; a->file != (char *) NULL; a++)
    {
      sprintf(buf,"%s/%s",ACL_DIR,a->file);
      a->file = (char *) malloc((strlen(buf)+1) * sizeof(char));
      strcpy(a->file,buf);
    }

  fclose(fp);

#ifdef HAVE_ZEPHYR
  unlink (ZEPHYR_DOWN_FILE);
#endif
  return(SUCCESS);
}

void
load_user(user)
     USER *user;
{
  get_specialties(user);
  get_acls(user);
  get_user_info(user);

  /* If a user has NO permissions (not even ask), that's probably a bug. */
  if (user->permissions == 0)
    {
      int external_realm;
      char *realm = user->realm;

      if (realm && realm[0])
	{
	  external_realm = strcasecmp(realm, DFLT_SERVER_REALM);
	}
      else
	{
	  realm = "???";
	  external_realm = 1;
	}

      if (external_realm)
	log_admin("user '%s@%s' has no permissions (including ask)!",
		  user->username, realm);
      else
	log_error("alert: user '%s@%s' has no permissions (including ask)!",
		  user->username, realm);

      strcpy(tmpfile, OLXX_SPOOL_DIR "/perm_corrupt_load_XXXX");
      mktemp(tmpfile);
      dump_data(tmpfile);
    }

}

static int
get_user_info(user)
     USER *user;
{
  FILE *fp;
  char db_line[DB_LINE];  
  char msgbuf[BUFSIZ];
  char *db, buf[BUFSIZ];            
  char canon[BUFSIZ];
  struct passwd *pwd;
  char *p;

#ifdef HAVE_KRB4
  sprintf(canon,"%s@%s",user->username,user->realm);
#else
  sprintf(canon,"%s@%s",user->username,DFLT_SERVER_REALM);
#endif

#ifdef HAVE_HESIOD
  pwd = hes_getpwnam(user->username);
#else
  pwd = getpwnam(user->username);
#endif
  if (pwd != (struct passwd *) NULL) {
    strncpy(user->realname,pwd->pw_gecos,NAME_SIZE);
    p = strchr(user->realname,',');
    if (p != NULL)
      *p = '\0';
  }

  fp = fopen(DATABASE_FILE,"r");
  if (fp == NULL)
    {
      log_error("get_user_info: can't open OLC database %s: %m",
		DATABASE_FILE);
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
	      fclose(fp);
	      return(SUCCESS);
	    }
	  
	}
    }
  fclose(fp);
  return(ERROR);
}


int
save_user_info(user)
     USER *user;
{
  /* This needs to be written */

  user = user;
  return(ERROR);
}
