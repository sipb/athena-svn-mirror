/*
 * $Id: lsdel.h,v 1.6 1999-01-22 23:09:02 ghudson Exp $
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
