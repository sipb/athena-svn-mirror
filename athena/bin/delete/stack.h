/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/stack.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/stack.h,v 1.3 1991-02-28 18:43:52 jik Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */
#include "mit-copying.h"

#define STACK_PUSH 	0
#define STACK_POP	1
#define EMPTY_STACK	2

#define push(data, size)	dostack((caddr_t) data, STACK_PUSH, size)
#define pop(data, size)		dostack((caddr_t) data, STACK_POP, size)
#define popall()		dostack((caddr_t) NULL, EMPTY_STACK, 0)
     
