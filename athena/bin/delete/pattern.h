/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v 1.5 1989-11-06 19:54:32 jik Exp $
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

#define FIND_DELETED		(1<<0)
#define FIND_UNDELETED		(1<<1)
#define RECURS_FIND_DELETED	(1<<2)
#define RECURS_FIND_UNDELETED	(1<<3)
#define RECURS_DELETED		(1<<4)
#define FOLLW_LINKS		(1<<5)
#define FOLLW_MOUNTPOINTS	(1<<6)
#define FIND_DOTFILES		(1<<7)
#define FIND_CONTENTS		(1<<8)
#define SUPPRESS_WARNINGS	(1<<9)
#define RECURS			(RECURS_FIND_DELETED | RECURS_FIND_UNDELETED |\
				 RECURS_DELETED | FIND_CONTENTS)
