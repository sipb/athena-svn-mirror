/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v 1.4 1989-10-23 13:32:29 jik Exp $
 * 
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include "mit-copyright.h"

int add_str();
int add_arrays();
int find_contents();
int find_deleted_contents();
int find_deleted_contents_recurs();
int find_matches();
int find_deleted_matches();
int find_recurses();
int find_deleted_recurses();
