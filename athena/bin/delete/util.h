/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.h,v 1.9 1989-12-15 04:37:03 jik Exp $
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
char *strindex();
char *strrindex();
#ifdef DEBUG
char *Malloc();
#else
#define Malloc(a) malloc(a)
extern char *malloc();
#endif

int is_mountpoint(), is_link();

#define is_dotfile(A) ((*A == '.') && \
		       ((*(A + 1) == '\0') || \
			((*(A + 1) == '.') && \
			 (*(A + 2) == '\0'))))

#define is_deleted(A) ((*A == '.') && (*(A + 1) == '#'))

 /* It would be BAD to pass something with a ++ anywhere near it into */
 /* this macro! 						      */
#define Opendir(dir) opendir(*(dir) ? (dir) : ".")
