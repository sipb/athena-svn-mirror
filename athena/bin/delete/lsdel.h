/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.h,v 1.5 1991-02-28 18:43:08 jik Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */
#include "mit-copying.h"

#define ERROR_MASK 1
#define NO_DELETE_MASK 2

int get_the_files();
