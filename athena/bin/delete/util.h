/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.h,v 1.4 1989-03-27 12:08:41 jik Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include "mit-copyright.h"

char *append();
char *convert_to_user_name();
char *firstpart();
char *lastpart();
char *reg_firstpart();
char *strindex();
char *strrindex();
