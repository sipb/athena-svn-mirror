/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for the internal hash tables and free/inuse linked lists
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: data_structs.c,v 1.3 1999-01-22 23:14:23 ghudson Exp $
 */

#ifndef SABER
#ifndef lint
static char rcsid[] ="$Id: data_structs.c,v 1.3 1999-01-22 23:14:23 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <syslog.h>

#include <olcd.h>

/* USER STRUCT FUNCTIONS */


static struct hash_entry *user_buckets[USER_HASHSIZE];

static int
user_hash_func(foo)
     char *foo;
{
  int i = 0;
  
  while(*foo != '\0') {
    i += *foo;
    foo++;
  }

  return(i % USER_HASHSIZE);
}

void
init_user_hash()
{
  int i;

  for(i=0;i<USER_HASHSIZE;i++) {
    user_buckets[i] = NULL;
  }
}


USER *
find_user(name)
     char *name;
{
  struct hash_entry *p;

  p = user_buckets[user_hash_func(name)];

  while (p != NULL) {
    if (strcmp(name,((USER *) p->entry)->username) == 0)
      return((USER *)p->entry);
    p = p->next;
  }
  return(NULL);
}

int
insert_user(u)
     USER *u;
{
  struct hash_entry *new,*p;

  if (find_user(u->username) != NULL) {
    /* Trying to insert duplicate user */
    return(-1);
  }

  if ((new = (struct hash_entry *)malloc(sizeof(struct hash_entry))) == NULL) {
    log_error("Could not malloc for hash_entry struct");
    exit(1);
  }

  new->entry = (void *)u;

  p = user_buckets[user_hash_func(u->username)];

  new->next = p;
  if (p != NULL) 
    p->prev = new;
  new->prev = NULL;
  user_buckets[user_hash_func(u->username)] = new;
  return(0);
}
  
int
remove_user(u)
     USER *u;
{
  struct hash_entry *p, *head;

  p = head = user_buckets[user_hash_func(u->username)];

  while (p != NULL) {
    if (strcmp(u->username,((USER *) p->entry)->username) == 0)
      break;
    p = p->next;
  }

  if (p == NULL)
    return(-1);

  if (p->prev = NULL) {
    head = p->next;
  } else {
    p->prev->next = p->next;
  }

  free(p);
  return(0);
}


USER *
alloc_user()
{
  USER *u;
  int i;

/* get off of free list, expanding as necessary */
  if (User_free != NULL) {
    u = User_free;
    User_free = u->next;
    if (User_free != NULL)
      User_free->prev = NULL;
  } else {
    if ((User_free = (USER *) calloc(USER_ALLOC_SZ,sizeof(USER)))
	== NULL) {
      log_error("olcd: can't allocate User list");
      return(NULL);
    }
    for(i=1;i<USER_ALLOC_SZ-2;i++) {
      User_free[i].next = &User_free[i+1];
      User_free[i].prev = &User_free[i-1];
      User_free[i].in_use = 0;
    }
    User_free[0].next = &User_free[1];
    User_free[0].prev = NULL;
    User_free[0].in_use = 0;
    User_free[USER_ALLOC_SZ-2].next = NULL;
    User_free[USER_ALLOC_SZ-2].prev = &User_free[USER_ALLOC_SZ-3];
    User_free[USER_ALLOC_SZ-2].in_use = 0;

    u = &User_free[KNUC_ALLOC_SZ-1];
  }

  /* add to in-use list */
  if (User_inuse != NULL)
    User_inuse->prev = u;
  u->next = User_inuse;
  u->prev = NULL;
  User_inuse = u;
  u->in_use = 1;
  return(u);
}

void
dealloc_user(u)
     USER *u;
{
  if (u->in_use != 1) {
    log_error("dealloc_user: trying to free user now in use");
    return;
  }

  /* remove from in-use list */
  u->in_use = 0;
  if (u->prev == NULL)
    User_inuse = u->next;
  else
    u->prev->next = u->next;
  if (u->next != NULL)
    u->next->prev = u->prev;

  /* Clear data, just in case */
  memset(u, 0, sizeof(USER));

  /* Add to free list. */
  if (User_free != NULL)
    User_free->prev = u;
  u->next = User_free;
  u->prev = NULL;
  User_free = u;
  return;
}



/* KNUCKLE STRUCT FUNCTIONS */

static struct hash_entry *knuc_buckets[KNUC_HASHSIZE];

static int
knuc_hash_func(foo,bar)
     char *foo;
     int bar;
{
  while(*foo != '\0') {
    bar += *foo;
    foo++;
  }

  return(bar % USER_HASHSIZE);
}

void
init_knuc_hash()
{
  int i;

  for(i=0;i<KNUC_HASHSIZE;i++) {
    knuc_buckets[i] = NULL;
  }
}


KNUCKLE *
find_knuc(name,inst)
     char *name;
     int inst;
{
  struct hash_entry *p;

  p = knuc_buckets[knuc_hash_func(name,inst)];

  while (p != NULL) {
    if ((strcmp(name,((KNUCKLE *) p->entry)->user->username) == 0) &&
	(inst == ((KNUCKLE *) p->entry)->instance))
      return((USER *)p->entry);
    p = p->next;
  }
  return(NULL);
}

int
insert_knuc(k)
     KNUCKLE *k;
{
  struct hash_entry *new,*p;

  if (find_knuc(k->user->username,k->instance) != NULL) {
    /* Trying to insert duplicate knuckle */
    return(-1);
  }

  if ((new = (struct hash_entry *)malloc(sizeof(struct hash_entry))) == NULL) {
    log_error("Could not malloc for hash_entry struct");
    exit(1);
  }

  new->entry = (void *)k;

  p = knuc_buckets[knuc_hash_func(k->user->username,k->instance)];

  new->next = p;
  if (p != NULL) 
    p->prev = new;
  new->prev = NULL;
  knuc_buckets[knuc_hash_func(k->user->username,k->instance)] = new;
  return(0);
}
  
int
remove_knuc(k)
     KNUCKLE *k;
{
  struct hash_entry *p, *head;

  p = head = knuc_buckets[knuc_hash_func(k->user->username,k->instance)];

  while (p != NULL) {
    if ((strcmp(k->user->username,((KNUCKLE *) p->entry)->user->username) == 0) &&
	(k->instance == ((KNUCKLE *) p->entry)->instance))
      break;
    p = p->next;
  }

  if (p == NULL)
    return(-1);

  if (p->prev = NULL) {
    head = p->next;
  } else {
    p->prev->next = p->next;
  }

  free(p);
  return(0);
}


KNUCKLE *
alloc_knuc()
{
  KNUCKLE *u;
  int i;

/* get off of free list, expanding as necessary */
  if (Knuckle_free != NULL) {
    u = Knuckle_free;
    Knuckle_free = u->next;
    if (Knuckle_free != NULL)
      Knuckle_free->prev = NULL;
  } else {
    if ((Knuckle_free = (KNUCKLE *) calloc(KNUC_ALLOC_SZ,sizeof(KNUCKLE)))
	== NULL) {
      log_error("olcd: can't allocate Knuc list");
      return(NULL);
    }
    for(i=1;i<KNUC_ALLOC_SZ-2;i++) {
      Knuckle_free[i].next = &Knuckle_free[i+1];
      Knuckle_free[i].prev = &Knuckle_free[i-1];
      Knuckle_free[i].in_use = 0;
    }
    Knuckle_free[0].next = &Knuckle_free[1];
    Knuckle_free[0].prev = NULL;
    Knuckle_free[0].in_use = 0;
    Knuckle_free[KNUC_ALLOC_SZ-2].next = NULL;
    Knuckle_free[KNUC_ALLOC_SZ-2].prev = &Knuckle_free[KNUC_ALLOC_SZ-3];
    Knuckle_free[KNUC_ALLOC_SZ-2].in_use = 0;

    u = &Knuckle_free[KNUC_ALLOC_SZ-1];
  }

  /* add to in-use list */
  if (Knuckle_inuse != NULL)
    Knuckle_inuse->prev = u;
  u->next = Knuckle_inuse;
  u->prev = NULL;
  Knuckle_inuse = u;
  u->in_use = 1;
  return(u);
}

void
dealloc_knuc(u)
     KNUCKLE *u;
{
  /* remove from user list */
  remove_knuc_from_user(u);

  /* remove from in-use list */
  if (u->in_use != 1) {
    log_error("dealloc_knuc: trying to free knuckle not in use");
    return;
  }

  u->in_use = 0;
  if (u->prev == NULL)
    Knuckle_inuse = u->next;
  else
    u->prev->next = u->next;
  if (u->next != NULL)
    u->next->prev = u->prev;

  /* Clear data, just in case */
  memset(u, 0, sizeof(KNUCKLE));

  /* Add to free list. */
  if (Knuckle_free != NULL)
    Knuckle_free->prev = u;
  u->next = Knuckle_free;
  u->prev = NULL;
  Knuckle_free = u;
  return;
}


void
insert_knuc_in_user(u,k)
     USER *u;
     KNUCKLE *k;
{
  if (u->knuckles != NULL)
    u->knuckles->prev_k = k;
  k->next_k = u->knuckles;
  k->prev_k = NULL;
  u->knuckles = k;
  return;
}

void
remove_knuc_from_user(k)
     KNUCKLE *k;
{
  /* should check to see if knuckle really in user */

  if (k->user->knuckles == NULL)
    return;

  if (k->prev_k == NULL)
    k->user->knuckles = k->next_k;
  else
    k->prev_k->next_k = k->next_k;
  if (k->next_k != NULL)
    k->next_k->prev_k = k->prev_k;

  k->prev_k = NULL;
  k->next_k = NULL;
  return;
}
