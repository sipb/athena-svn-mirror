/*
 * $Id: shell_regexp.h,v 1.4 1999-01-22 23:09:05 ghudson Exp $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */
#include "mit-copying.h"

extern int reg_cmp();

#define REGEXP_MATCH 1
#define REGEXP_NO_MATCH 0
