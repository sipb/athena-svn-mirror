/*
 * Generic hash table routines.  Uses integer keys to store objects.
 *
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/hash.c,v $
 * $Author: cfields $ 
 *
 *  (c) Copyright 1988,1991 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  mit-copyright.h.
 *
 *  This file contains the hash table functions create_hash, hash_lookup,
 *  hash_store, hash_update, hash_search, hash_step, and hash_destroy.
 *
 *  Since only the functions create_hash, hash_lookup, and hash_store are
 *  used, the others have been commented out.  They should be uncommented
 *  if needed.
 *
 *  The general code for these hash table routines were copied from 
 *  another project (smsdev).
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/hash.c,v 1.1 1992-04-29 13:55:54 cfields Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h> /* this is annoying... */
#include "Jets.h"
#include "hash.h"


#define hash_func(h,key) (key>=0 ? (key % h->size) : (-key % h->size))


/* Create a hash table.  The size is just a hint, not a maximum. */

struct hash *create_hash(size)
int size;
{
    struct hash *h;

    h = (struct hash *) XjMalloc((unsigned) sizeof(struct hash));
    h->size = size;
    h->data = (struct bucket **) XjMalloc((unsigned) size
					  * sizeof(struct bucket *));
    bzero(h->data, size * sizeof(struct bucket *));
    return(h);
}

/* Lookup an object in the hash table.  Returns the value associated with
 * the key, or NULL (thus NULL is not a very good value to store...)
 */

caddr_t hash_lookup(h, key)
struct hash *h;
register int key;
{
    register struct bucket *b;

    b = h->data[hash_func(h,key)];
    while (b && b->key != key)
      b = b->next;
    if (b && b->key == key)
      return(b->data);
    else
      return(NULL);
}


/* Store an item in the hash table.  Returns 0 if the key was not previously
 * there, or 1 if it was.
 */

int hash_store(h, key, value)
struct hash *h;
register int key;
caddr_t value;
{
    register struct bucket *b, **p;

    p = &(h->data[hash_func(h,key)]);
    if (*p == NULL) {
	b = *p = (struct bucket *) XjMalloc((unsigned) sizeof(struct bucket));
	b->next = NULL;
	b->key = key;
	b->data = value;
	return(0);
    }

    for (b = *p; b && b->key != key; b = *p)
      p = (struct bucket **) *p;
    if (b && b->key == key) {
	b->data = value;
	return(1);
    }
    b = *p = (struct bucket *) XjMalloc((unsigned) sizeof(struct bucket));
    b->next = NULL;
    b->key = key;
    b->data = value;
    return(0);
}


#ifdef notdef

/****************************************************************************/
/* Update an existing object in the hash table.  Returns 1 if the object
 * existed, or 0 if not.
 */

int hash_update(h, key, value)
struct hash *h;
register int key;
caddr_t value;
{
    register struct bucket *b;

    b = h->data[hash_func(h,key)];
    while (b && b->key != key)
      b = b->next;
    if (b && b->key == key) {
	b->data = value;
	return(1);
    } else
      return(0);
}

/* Search through the hash table for a given value.  For each piece of
 * data with that value, call the callback proc with the corresponding key.
 */

hash_search(h, value, callback)
struct hash *h;
register caddr_t value;
void (*callback)();
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    if (b->data == value)
	      (*callback)(b->key);
	}
    }
}


/* Step through the hash table, calling the callback proc with each key.
 */

hash_step(h, callback, hint)
struct hash *h;
void (*callback)();
caddr_t hint;
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    (*callback)(b->key, b->data, hint);
	}
    }
}


/* Deallocate all of the memory associated with a table */

hash_destroy(h)
struct hash *h;
{
    register struct bucket *b, **p, *b1;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b1) {
	    b1 = b->next;
	    free(b);
	}
    }
}

#endif /* notdef */
