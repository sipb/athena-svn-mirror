/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/hash.h,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

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
