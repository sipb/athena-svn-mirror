/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/hash.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _hash_h
#define _hash_h

#include <sys/types.h>

/* Hash table declarations */

struct bucket {
    struct bucket *next;
    int key;
    caddr_t data;
};

struct hash {
    int size;
    struct bucket **data;
};

#define HASHSIZE 67

extern struct hash *create_hash();
extern caddr_t hash_lookup();
extern int hash_store();

#endif /* _hash_h */
