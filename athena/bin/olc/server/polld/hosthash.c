/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains procedures for caching gethostbyname
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/hosthash.c,v $
 *	$Id: hosthash.c,v 1.2 1991-01-27 22:05:39 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/polld/hosthash.c,v 1.2 1991-01-27 22:05:39 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <netdb.h>

#include <polld.h>

struct entry {
  char hostname[128];   /* Host name */
  struct in_addr ent;    /* Host address */
  short int use;        /* mark for the clock hand */
  struct entry *next;   /* next entry in the chain */
  struct entry *prev;   /* prev entry in the chain */
};

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int get_bucket_index P((char *machname ));
static int allocate_entry P((void ));
static void delete_entry P((struct entry *ent ));

#undef P

/* Note: cachesize must be a power of two. */

#define CACHESIZE 128
#define inc_hand (clock_hand = (++clock_hand)&(CACHESIZE-1));

extern int errno;

static struct entry cache[CACHESIZE];
static struct entry *buckets[CACHESIZE];
int clock_hand = 0;

/*
 * init_cache
 *
 * Sets up cache for use
 *
 * Returns: nothing
 */

void
init_cache()
{
  int i;

  for(i=0;i<CACHESIZE;i++) {
    bzero((char *) &(cache[i]), sizeof(cache[i]));
    cache[i].use = 1;
  }
}

struct hostent *
c_gethostbyname(name)
     char *name;
{
  int hash, found, new;
  struct entry *head, *ptr;
  static struct hostent host;
  static char *addr_list[1];
  struct hostent *host_p;


  cache[clock_hand].use = 1;
  inc_hand;

  hash = get_bucket_index(name);
  head = buckets[hash];

  found = 0;
  ptr = head;

  while (ptr != NULL) {
    if (strcmp(name,ptr->hostname) == 0) {
      found = 1;
      break;
    }
    ptr = ptr->next;
  }

  if (found == 0) {
    host_p = gethostbyname(name);
    if (host_p == NULL)
      return(NULL);

    /* Get a free cache table entry, clearing if necessary */
    new = allocate_entry();

    /* copy information over */
    strcpy(cache[new].hostname,name);
    bcopy(host_p->h_addr_list[0],&cache[new].ent,host_p->h_length);

    /* Add to the head of the linked list */
    /* Need to re-set head, since we may have deleted initial bucket when we */
    /* allocated a new cache entry */

    head = buckets[hash];
    cache[new].next = head;
    if (head != NULL)
      head->prev = &cache[new];
    buckets[hash] = &cache[new];

    if (host.h_length == 0) {
      host.h_aliases = NULL;
      host.h_addrtype = host_p->h_addrtype;
      host.h_length = host_p->h_length;
      host.h_addr_list = addr_list;
    }
    host.h_addr_list[0] = ((char *) &(cache[new].ent));
  }
  else
    host.h_addr_list[0] = (char *) &(ptr->ent);

  host.h_name = name;
  return(&host);
}

static int
get_bucket_index(machname)
     char *machname;
{
  int i;

  i = 0;

  while (*machname != '\0') {
    i += *machname;
    machname++;
  }
  return(i&(CACHESIZE-1));
}

static int
allocate_entry()
{
  int new;

  while (cache[clock_hand].use == 0) {
    cache[clock_hand].use = 1;
    inc_hand;
  }

  /* found an entry; mark it as touched, and increment hand past it */

  new = clock_hand;
  inc_hand;

  /* If it's in use, clean it up */
  delete_entry(&cache[new]);

  /* Now mark it as used */
  cache[new].use = 0;
  return(new);
}

static void
delete_entry(ent)
  struct entry *ent;
{

  ent->use = 1;
  if (ent->hostname != '\0') {
    if (ent->next != NULL)
      ent->next->prev = ent->prev;
    if (ent->prev == NULL)
      /* If it has a null prev pointer, it's the one that buckets points to */
      /* for that hash index */
      buckets[get_bucket_index(ent->hostname)] = ent->next;
    else
      ent->prev->next = ent->next;
  }
  ent->hostname[0] = '\0';
  ent->prev = NULL;
  ent->next = NULL;
}

