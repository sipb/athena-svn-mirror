/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.h,v 1.5 1991-02-28 18:44:06 jik Exp $
 * 
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */
#include "mit-copying.h"

#define DELETEPREFIX ".#"
#define DELETEREPREFIX "\\.#"

typedef struct {
     char *user_name;
     char *real_name;
} listrec;

int sort_files();
int unique();

int get_the_files();
