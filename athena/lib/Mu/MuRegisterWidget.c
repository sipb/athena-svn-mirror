
/*
 *
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * MotifUtils:   Utilities for use with Motif and UIL
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuRegisterWidget.c,v $
 * $Author: cfields $
 * $Log: not supported by cvs2svn $
 * Revision 1.4  1990/09/07  18:03:29  vanharen
 * Changed MuGetWidget to MuLookupWidget since the include file "Mu.h"
 * defines MuGetWidget(w) as MuLookupWidget(w).  I think the intent was to
 * rename the function, but provide the macro for backwards compatibility.
 *
 * Also, modified MuLookupWidget to return NULL if the hash table hasn't
 * been initialized yet.
 *
 * Revision 1.3  90/02/13  14:26:54  djf
 * fixed bad buffer declaration in MuRegisterWidget()
 * 
 * Revision 1.2  90/02/13  14:21:08  djf
 * 
 * Revision 1.1  89/12/09  15:14:44  djf
 * Initial revision
 * 
 *
 * This file contains the functions MuRegisterWidgetCallback (which is
 * called from uil as MuRegisterWidget -- see register.c) and MuGetWidget.
 *
 * MuRegisterWidget is called from uil as a create callback procedure. 
 * It is passed a char *string which it converts to a quark using 
 * XrmStringToQuark.  It stores the widget ID of the created widget
 * in a hash table using the quark as an integer key (see hash.c).
 *
 * MuGetWidget is called from C, and it returns the widget ID associated  
 * with the char *string passed.  It calls the routine hash_lookup in 
 * hash.c , passing it the hash table created in MuRegisterWidget and  
 * the quark of the string passed.
 */

#include "Mu.h"
#include <stdio.h>
#include <stdlib.h>

/*
 *
 * Generic hash table routines.  Uses integer keys to store Widget values.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 *
 *  This file contains the hash table functions create_hash, hash_lookup,
 *  hash_store, hash_update, hash_search, hash_step, and hash_destroy.
 *
 *  Since only the functions create_hash, hash_lookup, and hash_store are
 *  used, the others have been commented out.  They should be uncommented
 *  if needed.
 *
 *  The general code for these hash table routines were copied from 
 *  another project (smsdev).  The routines have been modified for 
 *  MotifUtilities use.  In hash_store and hash_lookup the argument key 
 *  is a quark of a char *string, and the argument value in hash_store is 
 *  a widget (see widgets.c).  These functions are only called from 
 *  MuRegisterWidget and MuGetWidget.
 */

/* #include <mit-copyright.h> */



/* Hash table declarations */

struct bucket {
    struct bucket *next;
    int key;
    Widget data;
};
struct hash {
    int size;
    struct bucket **data;
};

#define hash_func(h,key) (key>=0 ? (key % h->size) : (-key % h->size))


/* Create a hash table.  The size is just a hint, not a maximum. */

struct hash *create_hash(size)
int size;
{
    struct hash *h;

    h = (struct hash *) malloc(sizeof(struct hash));
    h->size = size;
    h->data = (struct bucket **) malloc(size * sizeof(struct bucket *));
    bzero(h->data, size * sizeof(struct bucket *));
    return(h);
}

/* Lookup an object in the hash table.  Returns the value associated with
 * the key, or NULL (thus NULL is not a very good value to store...)
 */

Widget hash_lookup(h, key)
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
Widget value;
{
    register struct bucket *b, **p;

    p = &(h->data[hash_func(h,key)]);
    if (*p == NULL) {
	b = *p = (struct bucket *) malloc(sizeof(struct bucket));
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
    b = *p = (struct bucket *) malloc(sizeof(struct bucket));
    b->next = NULL;
    b->key = key;
    b->data = value;
    return(0);
}

/****************************************************************************/
/* Update an existing object in the hash table.  Returns 1 if the object
 * existed, or 0 if not.
 */

/*  int hash_update(h, key, value)
struct hash *h;
register int key;
Widget value;
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
}  */

/* Search through the hash table for a given value.  For each piece of
 * data with that value, call the callback proc with the corresponding key.
 */

/* hash_search(h, value, callback)
struct hash *h;
register Widget value;
void (*callback)();
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    if (b->data == value)
	      (*callback)(b->key);
	}
    }
} */


/* Step through the hash table, calling the callback proc with each key.
 */

/* hash_step(h, callback, hint)
struct hash *h;
void (*callback)();
Widget hint;
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    (*callback)(b->key, b->data, hint);
	}
    }
} */


/* Deallocate all of the memory associated with a table */

/* hash_destroy(h)
struct hash *h;
{
    register struct bucket *b, **p, *b1;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b1) {
	    b1 = b->next;
	    free(b);
	}
    }
} */



static struct hash *h = NULL;
#define HASHSIZE 67

void MuRegisterWidget(w, name)
Widget w;
char *name;
{
    XrmQuark quark;
    int i;
    char err_buf[BUFSIZ];
  
    quark = XrmStringToQuark(name);
    
    if ( h == NULL ) 
         h = (struct hash *)(create_hash(HASHSIZE));

    i = hash_store(h, quark, w);
  
    if (i == 1) {/* if a widget was already stored by that name, warn */
	sprintf(err_buf,
"MuRegisterName: reuse of name %s.  Previously registered value overwritten",
		name);
	XtWarning(err_buf);
    }
}


Widget MuLookupWidget(string)
      char *string;
{
   XrmQuark quark;
   Widget w;

   quark = XrmStringToQuark(string);
   
   if (h == NULL)		/* if hash table hasn't been initialized */
     return(NULL);		/* yet, return null. */

   w = (Widget)(hash_lookup(h, quark));
   return(w);
 }
   
